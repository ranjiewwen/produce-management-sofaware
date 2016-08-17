#include "stdafx.h"
#include "device_proxy.h"
#include <memory>
#include "error.h"
#include "progress_tracker.h"
#include "progress_tracker_inl.h"
#include "observer_inl.h"
#include "bitmap.h"
#include "working_parameters.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// device control command IDs
#define COMMAND_GET_SN                      0x0001
#define COMMAND_GET_DEVICE_INFO             0x0002
#define COMMAND_UPDATE_DEBUG_STATE          0x0003
#define COMMAND_UPGRADE                     0x0004
#define COMMAND_UPGRADE_DATA                0x0005
#define COMMAND_RESTART                     0x0006
#define COMMAND_ECHO                        0x8181

//#define COMMAND_GET_IR_PARAMETERS           0x0003
#define COMMAND_GET_IR_VALUES               0x0007
#define COMMAND_SET_IR_PARAMETERS           0x0008
#define COMMAND_UPDATE_IR_PARAMETERS        0x0009

#define COMMAND_START_MASTER_SIGNAL_DETECT  0x0006
#define COMMAND_GET_CIS_PARAMETER           0x0009
#define COMMAND_TAKE_CIS_IMAGE              0x000a
#define COMMAND_SET_CIS_PARAMETER           0x000b
#define COMMAND_UPDATE_CIS_PARAMETER        0x000c
#define COMMAND_GET_CIS_CORRECTION_TABLE    0x000d
#define COMMAND_UPDATE_CIS_CORRECTION_TABLE 0x000e
#define COMMAND_GET_MAC						0x0011
#define COMMAND_GET_STUDY_COMPLETED_STATE   0x0012
#define COMMAND_SET_AGING_TIME              0x0013
#define COMMAND_START_TAPE_STUDY            0x0014

#define COMMAND_START_MOTOR                 0x0015
#define COMMAND_START_RUN_CASH_DETECT       0x8004
#define COMMAND_START_SIGNAL_COLLECT        0x0016
#define COMMAND_DISABLE_DEBUG               0x0017

#define COMMAND_SET_TIME                    0x0018
#define COMMAND_GET_TIME                    0x0019

#define COMMAND_LIGHT_CIS                   0x0020
#define COMMAND_SET_SN                      0x0021
#define COMMAND_TAPE_LEARNING				0x0022

// PacketHeader
struct PacketHeader {
	unsigned char signatures[2];
	union {
		unsigned short id;
		unsigned short status;
	};
	unsigned int count;
	unsigned int length;
};

// CommandResult
CommandResult::CommandResult()
: dataLength_(0)
, status_(0) {
	dataBuffer_ = staticBuffer_;
}

CommandResult::~CommandResult() {
	if (dataBuffer_ != staticBuffer_) {
		delete[] dataBuffer_;
	}
}

bool CommandResult::IsOk() const {
	return status_ == 0;
}

int CommandResult::GetStatus() const {
	return status_;
}

int CommandResult::GetDataLength() const {
	return dataLength_;
}

const void *CommandResult::GetData() const {
	return dataBuffer_;
}

int CommandResult::GetData(void *buffer, int size) {
	if (size > dataLength_) {
		size = dataLength_;
	}
	if (size > 0) {
		memcpy(buffer, dataBuffer_, size);
	}
	return size;
}

void *CommandResult::GetDataBuffer(int length) {
	if (dataBuffer_ != staticBuffer_) {
		delete[] dataBuffer_;
	}
	if (length > STATIC_BUFFER_SIZE) {
		dataBuffer_ = new char[length];
	}
	else {
		dataBuffer_ = staticBuffer_;
	}
	dataLength_ = length;

	return dataBuffer_;
}

void CommandResult::SetStatus(int code) {
	status_ = code;
}

// DataConnection
DataConnection::DataConnection() {
}

DataConnection::~DataConnection() {
	Close();
}

bool DataConnection::Open(LPCTSTR address, int port) {
	if (listenSocket_.IsOpened()) {
		SetLastError(ERROR_ALREADY_INITIALIZED);
		return false;
	}
	if (!listenSocket_.Listen(address, port)) {
		return false;
	}
	return true;
}

bool DataConnection::WaitConnect(int waitMilliseconds) {
	SOCKET s = listenSocket_.Accept(waitMilliseconds);
	if (s == INVALID_SOCKET) {
		return false;
	}
	dataSocket_.Attach(s);
	dataSocket_.SetRecvBufferSize(20 * 1024 * 1024);

	return true;
}

void DataConnection::Close() {
	dataSocket_.Close();
	listenSocket_.Close();
}

bool DataConnection::ReadPacket(DataPacket *packet) {
	PacketHeader hd;
	if (!dataSocket_.ReceiveFully(&hd, sizeof(hd))) {
		return false;
	}
	if (hd.signatures[0] != 'D' || hd.signatures[1] != 'T') {
		SetLastError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}
	packet->Create(hd.id, hd.count, hd.length);
	if (hd.length > 0) {
		if (!dataSocket_.ReceiveFully(packet->GetData(), hd.length)) {
			return false;
		}
	}
	return true;
}

// DeviceProxy
IMPLEMENT_SINGLETON(DeviceProxy)

DeviceProxy::DeviceProxy()
: connected_(false) {
	InitCISCorrectionTable();
}

DeviceProxy::~DeviceProxy() {
	Disconnect();
}

bool DeviceProxy::Connect(LPCTSTR address, int port) {
	LastErrorHolder errorHolder;

	CriticalSection::ScopedLocker locker(criSec_);

	if (connected_) {
		errorHolder.SetError(ERROR_DEVICE_ALREADY_CONNECTED);
		return false;
	}
	if (!connection_.Connect(address, port)) {
		errorHolder.SaveLastError();
		return false;
	}
	connection_.SetTimeout(18000);
	CommandResult result;
	if (!SendCommand(COMMAND_GET_DEVICE_INFO, NULL, 0, &result)) {
		errorHolder.SaveLastError();
		connection_.Close();
		return false;
	}
	//ASSERT(sizeof(DeviceInfo) == 132);  
	if (!result.IsOk() || result.GetData(&deviceInfo_, sizeof(DeviceInfo)) != sizeof(DeviceInfo)) {
		connection_.Close();
		errorHolder.SetError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}

	// sync clock.
	if (!SyncClock()) {
		errorHolder.SaveLastError();
		connection_.Close();
		return false;
	}

	// get CIS correction table.
	if (!SendCommand(COMMAND_GET_CIS_CORRECTION_TABLE, NULL, 0, &result)) {
		errorHolder.SaveLastError();
		connection_.Close();
		return false;
	}
	if (!result.IsOk() || result.GetDataLength() < sizeof(CISCorrectionTable)) {
		connection_.Close();
		errorHolder.SetError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}
	CopyMemory(&cisCorrectionTable_, result.GetData(), sizeof(CISCorrectionTable));

	/*CFile file(_T("d:\\correction_get.bin"), CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
	file.Write(&cisCorrectionTable_, sizeof(cisCorrectionTable_));
	file.Close();*/

	connected_ = true;

	NotifyUpdate(SUBJECT_CONNECTED);

	return true;
}

bool DeviceProxy::IsConnected() const {
	return connection_.IsOpened();
}

void DeviceProxy::Disconnect() {
	CriticalSection::ScopedLocker locker(criSec_);

	if (connected_) {
		connection_.Close();
		connected_ = false;

		NotifyUpdate(SUBJECT_DISCONNECTED);
	}
}

bool DeviceProxy::GetDeviceInfo(DeviceInfo *info) {
	ASSERT(info != NULL);

	if (!connected_) {
		SetLastError(ERROR_DEVICE_NOT_CONNECT);
		return false;
	}

	*info = deviceInfo_;

	return true;
}

bool DeviceProxy::GetMacAddr(char *macBuf, unsigned int bufLen)
{
	CriticalSection::ScopedLocker locker(criSec_);

	CommandResult result;
	if (!SendCommand(COMMAND_GET_MAC, NULL, 0, &result))
	{
		return false;
	}
	if (!result.IsOk())
	{
		return false;
	}
	if (bufLen < strlen((char *)result.GetData()))
	{
		return false;
	}

	CopyMemory(macBuf, result.GetData(), strlen((char *)result.GetData()));

	return true;
}

bool DeviceProxy::Upgrade(LPCTSTR firmwareFile, LPCTSTR newVersion, ProgressTracker *tracker) {
	CriticalSection::ScopedLocker locker(criSec_);

	CString message;

	try {
		CFile file(firmwareFile, CFile::typeBinary | CFile::modeRead);
		int length = (int)file.GetLength();
		struct UpgradeRequest {
			char newVersion[12];
			int length;
		};
		// send start upgrade command.
		UpgradeRequest request;
		ZeroMemory(&request, sizeof(request));
		strncpy(request.newVersion, CT2A(newVersion), _countof(request.newVersion));
		request.length = length;
		CommandResult result;
		if (!SendCommand(COMMAND_UPGRADE, &request, sizeof(request), &result)) {
			return false;
		}
		int blockSize = 0;
		if (!result.GetData(&blockSize, sizeof(blockSize)) || blockSize <= 0) {
			SetLastError(ERROR_DEVICE_RESULT_ERROR);
			return false;
		}
		// send upgrade data.
		int totalTransferred = 0;
		char *buffer = new char[blockSize];
		int blockIndex = 0;
		for (;;) {
			UINT ret = file.Read(buffer, blockSize);
			if (ret == (UINT)-1 || !SendCommand(COMMAND_UPGRADE_DATA, blockIndex, buffer, ret)) {
				delete[] buffer;
				return false;
			}
			if (ret == 0) {
				break;
			}
			if (tracker != NULL) {
				totalTransferred += ret;
				tracker->UpdateProgress(totalTransferred, length);
			}
			blockIndex++;
		}
		delete[] buffer;
	}
	catch (CFileException *e) {
		SetLastError(e->m_lOsError);
		e->Delete();
		return false;
	}
	return true;
}

bool DeviceProxy::Restart() {
	CriticalSection::ScopedLocker locker(criSec_);

	return SendCommand(COMMAND_RESTART);
}

bool DeviceProxy::UpdateDebugState(int state, int mask) {
	CriticalSection::ScopedLocker locker(criSec_);

	int states[16] = { 0 };
	states[0] = (deviceInfo_.debugState[0] & ~mask) | state;
	if (SendCommand(COMMAND_UPDATE_DEBUG_STATE, states, sizeof(states))) {
		deviceInfo_.debugState[0] = states[0];
		return true;
	}
	return false;
}

bool DeviceProxy::Echo() {
	CriticalSection::ScopedLocker locker(criSec_);

	connection_.SetTimeout(3000);

	bool ret = SendCommand(COMMAND_ECHO);

	connection_.SetTimeout(18000);

	return ret;
}

bool DeviceProxy::SetIRParameters(const int *resistances) {
	CriticalSection::ScopedLocker locker(criSec_);

	int irCount = deviceInfo_.numberOfIR;
	CommandResult result;
	if (!SendCommand(COMMAND_SET_IR_PARAMETERS, resistances, irCount * sizeof(int), &result)) {
		return false;
	}
	if (!result.IsOk()) {
		SetLastError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}

	return true;
}

bool DeviceProxy::SetAgingTestTime(const int _hourTime)
{
	CriticalSection::ScopedLocker locker(criSec_);
	CommandResult result;
	if (!SendCommand(COMMAND_SET_AGING_TIME, &_hourTime, 4, &result))
	{
		return false;
	}
	if (!result.IsOk())
	{
		TRACE("SetAgingTestTime error!");
		return false;
	}
	return true;
}

bool DeviceProxy::GetIRParameters(int *resistances) {
	ASSERT(FALSE);
	CriticalSection::ScopedLocker locker(criSec_);
	SetLastError(E_NOTIMPL);
	return false;
}

bool DeviceProxy::GetIRValues(int *values) {
	CriticalSection::ScopedLocker locker(criSec_);

	int irCount = deviceInfo_.numberOfIR;
	CommandResult result;
	if (!SendCommand(COMMAND_GET_IR_VALUES, NULL, 0, &result)) {
		return false;
	}
	if (!result.IsOk() || result.GetDataLength() < (int)(irCount * sizeof(int))) {
		SetLastError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}
	CopyMemory(values, result.GetData(), irCount * sizeof(int));

	return true;
}

bool DeviceProxy::GetIRValues(const int *resistances, int *values) {
	CriticalSection::ScopedLocker locker(criSec_);

	if (!SetIRParameters(resistances)) {
		return false;
	}
	return GetIRValues(values);
}

bool DeviceProxy::UpdateIRParameters(const int *resistances) {
	CriticalSection::ScopedLocker locker(criSec_);

	int irCount = deviceInfo_.numberOfIR;
	CommandResult result;
	if (!SendCommand(COMMAND_UPDATE_IR_PARAMETERS, resistances, irCount * sizeof(int), &result)) {
		return false;
	}
	if (!result.IsOk()) {
		SetLastError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}

	return true;
}

DataConnection *DeviceProxy::StartMasterSignalDetect() {
	CriticalSection::ScopedLocker locker(criSec_);

	return NULL;
}

bool DeviceProxy::LightCIS(int side, int colorMask) {
	CriticalSection::ScopedLocker locker(criSec_);

	int command = 0;
	if (side == CIS_TOP) {
		command = 0x200;
	}
	else {
		command = 0x100;
	}
	command |= colorMask;
	CommandResult result;
	if (!SendCommand(COMMAND_LIGHT_CIS, &command, sizeof(int), &result)) {
		return false;
	}
	if (!result.IsOk()) {
		SetLastError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}
	return true;
}

bool DeviceProxy::UnlightCIS(int side) {
	return LightCIS(side, CIS_COLOR_MASK_NONE);
}

bool DeviceProxy::SetCISParameter(const CISParameter parameter[CIS_COUNT]) {
	LastErrorHolder errorHolder;

	CriticalSection::ScopedLocker locker(criSec_);
#pragma pack(push)
#pragma pack(1)
	struct NetParam {
		int exposureTime[4];
		int gain[3];
		int offset[3];
	};
#pragma pack(pop)
	NetParam netParam[CIS_COUNT];
	for (int i = 0; i < CIS_COUNT; i++) {
		netParam[i].exposureTime[0] = parameter[i].exposureTime[CIS_COLOR_IR]; // ir
		netParam[i].exposureTime[1] = parameter[i].exposureTime[CIS_COLOR_RED]; // red
		netParam[i].exposureTime[2] = parameter[i].exposureTime[CIS_COLOR_GREEN]; // green
		netParam[i].exposureTime[3] = parameter[i].exposureTime[CIS_COLOR_BLUE]; // blue
		netParam[i].offset[0] = parameter[i].offset[0];
		netParam[i].offset[1] = parameter[i].offset[1];
		netParam[i].offset[2] = parameter[i].offset[2];
		netParam[i].gain[0] = parameter[i].gain[0];
		netParam[i].gain[1] = parameter[i].gain[1];
		netParam[i].gain[2] = parameter[i].gain[2];
	}
	CommandResult result;
	if (!SendCommand(COMMAND_SET_CIS_PARAMETER, netParam, sizeof(netParam), &result)) {
		errorHolder.SaveLastError();
		return false;
	}
	if (!result.IsOk()) {
		errorHolder.SetError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}
	return true;
}

bool DeviceProxy::TakeCISImage(int id, Bitmap *image) {
	LastErrorHolder errorHolder;

	CriticalSection::ScopedLocker locker(criSec_);

	CommandResult result;
	if (!SendCommand(COMMAND_TAKE_CIS_IMAGE, &id, 4, &result)) {
		errorHolder.SaveLastError();
		return false;
	}
	if (!result.IsOk() || result.GetDataLength() < CIS_IMAGE_WIDTH * CIS_IMAGE_HEIGHT) {
		errorHolder.SetError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}
	image->CreateGrayBitmap(CIS_IMAGE_WIDTH, CIS_IMAGE_HEIGHT);
	if (image->IsValid()) {
		CopyMemory(image->GetBits(), result.GetData(), CIS_IMAGE_WIDTH * CIS_IMAGE_HEIGHT);
	}

	return true;
}

bool DeviceProxy::TakeCISImage(const CISParameter parameter[CIS_COUNT], int id, Bitmap *image) {
	LastErrorHolder errorHolder;

	CriticalSection::ScopedLocker locker(criSec_);

	if (!SetCISParameter(parameter)) {
		errorHolder.SaveLastError();
		return false;
	}

	if (!TakeCISImage(id, image)) {
		errorHolder.SaveLastError();
		return false;
	}

	return true;
}

bool DeviceProxy::UpdateCISParameter(const CISParameter parameter[CIS_COUNT]) {
	LastErrorHolder errorHolder;

	CriticalSection::ScopedLocker locker(criSec_);
#pragma pack(push)
#pragma pack(1)
	struct NetParam {
		int exposureTime[4];
		int gain[3];
		int offset[3];
	};
#pragma pack(pop)
	NetParam netParam[CIS_COUNT];
	for (int i = 0; i < CIS_COUNT; i++) {
		netParam[i].exposureTime[0] = parameter[i].exposureTime[CIS_COLOR_IR]; // ir
		netParam[i].exposureTime[1] = parameter[i].exposureTime[CIS_COLOR_RED]; // red
		netParam[i].exposureTime[2] = parameter[i].exposureTime[CIS_COLOR_GREEN]; // green
		netParam[i].exposureTime[3] = parameter[i].exposureTime[CIS_COLOR_BLUE]; // blue
		netParam[i].offset[0] = parameter[i].offset[0];
		netParam[i].offset[1] = parameter[i].offset[1];
		netParam[i].offset[2] = parameter[i].offset[2];
		netParam[i].gain[0] = parameter[i].gain[0];
		netParam[i].gain[1] = parameter[i].gain[1];
		netParam[i].gain[2] = parameter[i].gain[2];
	}
	CommandResult result;
	if (!SendCommand(COMMAND_UPDATE_CIS_PARAMETER, netParam, sizeof(netParam), &result)) {
		errorHolder.SaveLastError();
		return false;
	}
	if (!result.IsOk()) {
		errorHolder.SetError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}

	return true;
}

bool DeviceProxy::UpdateCISCorrectionTable(const unsigned char *table, int size) {
	ASSERT(size == sizeof(CISCorrectionTable));

	CopyMemory((char *)&cisCorrectionTable_, table, size);

	/*CFile file(_T("d:\\correction_set.bin"), CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
	file.Write(&cisCorrectionTable_, sizeof(cisCorrectionTable_));
	file.Close();*/

	CommandResult result;
	if (!SendCommand(COMMAND_UPDATE_CIS_CORRECTION_TABLE, table, size, &result)) {
		return false;
	}
	if (!result.IsOk()) {
		SetLastError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}


	return true;
}

CISCorrectionTable *DeviceProxy::GetCISCorrectionTable() {
	CriticalSection::ScopedLocker locker(criSec_);

	return &cisCorrectionTable_;
}

bool DeviceProxy::GetStudyCompletedState(StudyCompletedState *state) {
	CriticalSection::ScopedLocker locker(criSec_);

	return true;
}

DataConnection *DeviceProxy::StartOVIStudy() {
	CriticalSection::ScopedLocker locker(criSec_);

	return NULL;
}

DataConnection *DeviceProxy::StartTapeStudy() {
	CriticalSection::ScopedLocker locker(criSec_);

	return NULL;
}

DataConnection *DeviceProxy::StartRunCashDetect() {
	CriticalSection::ScopedLocker locker(criSec_);

	return OpenDataConnection(COMMAND_START_RUN_CASH_DETECT, NULL, 0);
}

DataConnection *DeviceProxy::StartSignalCollect() {
	CriticalSection::ScopedLocker locker(criSec_);

	return NULL;
}

bool DeviceProxy::TapeStudy(const void *data, int dataLength)
{
	CriticalSection::ScopedLocker locker(criSec_);

	CommandResult result;
	if (!SendCommand(COMMAND_TAPE_LEARNING, data, dataLength, &result)) {
		return false;
	}
	if (!result.IsOk()) {
		SetLastError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}
	return true;
}

bool DeviceProxy::StartMotor(int milliseconds) {
	CriticalSection::ScopedLocker locker(criSec_);

	CommandResult result;
	if (!SendCommand(COMMAND_START_MOTOR, &milliseconds, sizeof(int), &result)) {
		return false;
	}
	if (!result.IsOk()) {
		SetLastError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}
	return true;
}

bool DeviceProxy::DisableDebug() {
	CriticalSection::ScopedLocker locker(criSec_);

	return SendCommand(COMMAND_DISABLE_DEBUG);
}

bool DeviceProxy::SyncClock() {
#pragma pack(push)
#pragma pack(1)
	struct TIME {
		WORD year;
		WORD month;
		WORD day;
		WORD hour;
		WORD minute;
		WORD second;
		WORD millisecond;
	};
#pragma pack(pop)

	static const int RETRY = 1;

	LastErrorHolder errorHolder;

	CriticalSection::ScopedLocker locker(criSec_);

	for (;;) {
		SYSTEMTIME st;
		GetLocalTime(&st);

		TIME t;
		t.year = st.wYear;
		t.month = st.wMonth;
		t.day = st.wDay;
		t.hour = st.wHour;
		t.minute = st.wMinute;
		t.second = st.wSecond;
		t.millisecond = st.wMilliseconds;

		CommandResult result;
		if (!SendCommand(COMMAND_SET_TIME, &t, sizeof(t), &result)) {
			errorHolder.SaveLastError();
			return false;
		}
		int status = result.GetStatus();
		if (status == 0) {
			break;
		}
		if (status != RETRY) {
			errorHolder.SetError(ERROR_DEVICE_RESULT_ERROR);
			return false;
		}
	}
	return true;
}

bool DeviceProxy::SetSerialNumber(LPCTSTR sn) {
#pragma pack(push)
#pragma pack(1)
	struct COMMAND {
		char  sn[SERIAL_NUMBER_LENGTH];
	};
#pragma pack(pop)

	LastErrorHolder errorHolder;

	CriticalSection::ScopedLocker locker(criSec_);

	COMMAND command;
	ZeroMemory(&command, sizeof(command));
	strncpy(command.sn, CT2A(sn), _countof(command.sn));

	CommandResult result;
	if (!SendCommand(COMMAND_SET_SN, &command, sizeof(command), &result)) {
		errorHolder.SaveLastError();
		return false;
	}
	if (result.GetStatus() != 0) {
		errorHolder.SetError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}

	strncpy(deviceInfo_.sn, command.sn, _countof(deviceInfo_.sn));

	return true;
}

DataConnection *DeviceProxy::OpenDataConnection(int commandId, const void *data, int dataLength) {
	DataLevel level;
	memset(&(level.hostAddr), 0, sizeof(level.hostAddr));
	int addrLen = sizeof(level.hostAddr);
	if (!connection_.GetAddress((sockaddr *)&(level.hostAddr), &addrLen)) {
		return NULL;
	}
	u_short port = 4321;
	std::auto_ptr<DataConnection> connection(new DataConnection());
	if (!connection->Open(CA2T(inet_ntoa(level.hostAddr.sin_addr)), port)) {
		return NULL;
	}
	level.hostAddr.sin_port = htons(port);
	ASSERT(sizeof(level.hostAddr) == 16);

	// 从可变配置文件中读取点钞机的数据上传等级
	VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();
	int waveLevel = config->GetIntParameter(_T("WaveDataLevel"), 1);
	if ((waveLevel < 0) || (waveLevel > 1))
	{
		waveLevel = 1;
	}
	level.waveDataLevel = (WaveDataLevel)waveLevel;
	int imageLevel = config->GetIntParameter(_T("ImageDataLevel"), 2);
	if ((imageLevel < 0) || (imageLevel > 2))
	{
		imageLevel = 2;
	}
	level.imageDataLevel = (ImageDataLevel)imageLevel;

	if (!SendCommandNoResult(commandId, 0, &level, sizeof(level))) {
		return NULL;
	}

	if (!connection->WaitConnect(10000)) {
		return NULL;
	}
	CommandResult result;
	if (!ReadResult(&result)) {
		return NULL;
	}
	if (!result.IsOk()) {
		SetLastError(ERROR_DEVICE_RESULT_ERROR);
		return NULL;
	}

	return connection.release();
}
//发送一个指令 Echo
bool DeviceProxy::SendCommand(int id) {
	return SendCommand(id, NULL, 0);
}
//发送带有负载的指令
bool DeviceProxy::SendCommand(int id, const void *data, int dataLength) {
	return SendCommand(id, 0, data, dataLength);
}
//发送有计数，带负载的指令
bool DeviceProxy::SendCommand(int id, int count, const void *data, int dataLength) {
	CommandResult result;
	if (!SendCommand(id, data, dataLength, &result)) {
		return false;
	}
	if (!result.IsOk()) {
		SetLastError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}
	return true;
}
//发送指令，并检验result
bool DeviceProxy::SendCommand(int id, const void *data, int dataLength, CommandResult *result) {
	return SendCommand(id, 0, data, dataLength, result);
}
//发送指令，计数，检验result
bool DeviceProxy::SendCommand(int id, int count, const void *data, int dataLength, CommandResult *result) {
	ASSERT(result != NULL);

	if (!SendCommandNoResult(id, count, data, dataLength)) {
		return false;
	}
	return ReadResult(result);
}
//发送包头和数据
bool DeviceProxy::SendCommandNoResult(int id, int count, const void *data, int dataLength) {
	PacketHeader hd;
	hd.signatures[0] = 'R';
	hd.signatures[1] = 'Q';
	hd.id = (unsigned short)id;
	hd.count = 0;
	hd.length = dataLength;
	if (!connection_.Send(&hd, sizeof(hd))) {
		return false;
	}
	if (dataLength > 0) {
		if (!connection_.Send(data, dataLength)) {
			return false;
		}
	}
	return true;
}

bool DeviceProxy::ReadResult(CommandResult *result) {
	ASSERT(result != NULL);

	PacketHeader hd;
	if (!connection_.ReceiveFully(&hd, sizeof(hd))) {
		return false;
	}
	if (hd.signatures[0] != 'R' || hd.signatures[1] != 'P') {
		SetLastError(ERROR_DEVICE_RESULT_ERROR);
		return false;
	}
	result->SetStatus(hd.status);
	if (hd.length > 0) {
		if (!connection_.ReceiveFully(result->GetDataBuffer(hd.length), hd.length)) {
			return false;
		}
	}
	return true;
}

bool DeviceProxy::CheckConnection() {
	CriticalSection::ScopedLocker locker(criSec_);

	if (!IsConnected()) {
		return false;
	}
	if (!Echo()) {
		Disconnect();
		return false;
	}

	return true;
}

void DeviceProxy::InitCISCorrectionTable() {
	for (int side = 0; side < CIS_COUNT; side++) {
		for (int color = 0; color < COLOR_COUNT; color++) {
			for (int x = 0; x < CIS_IMAGE_WIDTH; x++) {
				for (int level = 0; level < 256; level++) {
					cisCorrectionTable_.data[side][color][x][level] = level;
				}
			}
		}
	}
}