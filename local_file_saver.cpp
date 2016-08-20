#include "stdafx.h"
#include "local_file_saver.h"
#include "working_parameters.h"
#include "working_parameters_inl.h"
#include "bitmap.h"
#include "common.h"
#include "run_cash_monitor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SINGLETON(LocalFileSaver)

LocalFileSaver::LocalFileSaver()
{
	cashSavePath_[0] = _T('\0');
	CISAdjSavePath_[0] = _T('\0');
}

LocalFileSaver::~LocalFileSaver()
{
	// 销毁时清理保存路径 lux
	VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();

	CString savePath = config->GetStringParameter(_T("LocalSavePath"), _T(""));
	if (savePath == _T(""))
	{
		return;
	}
	int saveTime = config->GetIntParameter(_T("LocalSaveTime"), 3);

	// 获取当前时间
	SYSTEMTIME curSysTime;
	GetLocalTime(&curSysTime);
	ULARGE_INTEGER curFileTime;
	SystemTimeToFileTime(&curSysTime, (FILETIME*)&curFileTime);

	// 遍历保存路径下的文件夹
	CFileFind find;
	CString findPath = savePath + _T("\\*.*");
	BOOL bFound = find.FindFile(findPath);
	while (bFound)
	{
		bFound = find.FindNextFile();
		if (find.IsDots())
		{
			continue;
		}

		if (find.IsDirectory())
		{
			CString strDate = find.GetFileName();
			SYSTEMTIME folderDateSysTime;
			if (swscanf(strDate, _T("%04d-%02d-%02d"), &folderDateSysTime.wYear, &folderDateSysTime.wMonth, &folderDateSysTime.wDay) != 3)
			{
				continue;
			}
			folderDateSysTime.wHour = curSysTime.wHour;
			folderDateSysTime.wMinute = curSysTime.wMinute;
			folderDateSysTime.wSecond = curSysTime.wSecond;
			folderDateSysTime.wMilliseconds = curSysTime.wMilliseconds;
			ULARGE_INTEGER folderDataFileTime;
			SystemTimeToFileTime(&folderDateSysTime, (FILETIME*)&folderDataFileTime);

			unsigned __int64 diff = curFileTime.QuadPart - folderDataFileTime.QuadPart;
			int diffDay = (int)(diff / 10000 / 1000 / 60 / 60 / 24);
			if (diffDay > saveTime)
			{
				// 删除大于保留天数的文件夹
				CString delPath = find.GetFilePath();
				DeleteDirectory(delPath);
			}
		}
	}

	find.Close();
}

void LocalFileSaver::DeleteDirectory(LPCTSTR path)
{
	TCHAR findPath[MAX_PATH] = { 0 };
	_tcscpy(findPath, path);
	PathAddBackslash(findPath);
	PathAppend(findPath, _T("*.*"));
	CFileFind find;

	BOOL bFound = find.FindFile(findPath);
	while (bFound)
	{
		bFound = find.FindNextFile();
		if (find.IsDots())
		{
			continue;
		}

		if (find.IsDirectory())
		{
			DeleteDirectory(find.GetFilePath());
		}
		else
		{
			DeleteFile(find.GetFilePath());
		}
	}

	find.Close();
	RemoveDirectory(path);
}


bool LocalFileSaver::SaveADCData(Cash *data)
{
	if (_tcscmp(cashSavePath_, _T("")) == 0)
	{
		return true;
	}

	return SaveADCData(data, cashSavePath_);
}

bool LocalFileSaver::SaveADCData(Cash *data, LPCTSTR folder)
{
	if (_tcscmp(folder, _T("")) == 0)
	{
		return false;
	}

	TCHAR path[MAX_PATH] = { 0 };
	TCHAR fileName[128] = { 0 };
	char sn[24] = { 0 };
	strcpy(sn, data->GetSN());
	if (sn[0] == '\0')
	{
		sprintf(sn, "0");
	}
	wsprintf(fileName, _T("%04d%02d%02d_%02d%02d%02d_%03d_Err%03d_LastErr0x0_Den%d_Dir%d_Ver%02d_SN%s_adc.raw"),
		runCashStartTime.wYear, runCashStartTime.wMonth, runCashStartTime.wDay,
		runCashStartTime.wHour, runCashStartTime.wMinute, runCashStartTime.wSecond,
		data->Id(),
		data->GetError(), data->GetDenomination(), data->GetDirection(), data->GetVersion(),
		CA2T(sn));
	GenerateSavePath(fileName, folder, path);

	CFile file;
	if (!file.Open(path, CFile::modeCreate | CFile::modeWrite))
	{
		return false;
	}

	short count = data->GetADCSampleCount(0); //得到波形的长度
	int dataLen = count * 8 * 20;  //循环采用8次，每次20个字节
	char *writeData = new char[dataLen];
	memset(writeData, 0, dataLen);
	char *tmp = writeData;

	const short *code = data->GetADCSampleCodes(ADC_CHANNEL_BM);
	const short *value[ADC_CHANNEL_COUNT] = { 0 };  //指针数组，22行数据，每行数据长count

	for (int i = 0; i < ADC_CHANNEL_COUNT; i++)
	{
		value[i] = data->GetADCSampleValues(i);   //22种磁信号的电压值
	}

	int channelMap[24] = { 
		ADC_CHANNEL_BM, ADC_CHANNEL_IR1, ADC_CHANNEL_EIR5,   //0 12 9
		ADC_CHANNEL_RSM, ADC_CHANNEL_IR2, ADC_CHANNEL_EIR6,  //2 13 10
		ADC_CHANNEL_HD, ADC_CHANNEL_IR3, ADC_CHANNEL_UV,     //21 14 18
		ADC_CHANNEL_LSM, ADC_CHANNEL_IR4, ADC_CHANNEL_UVL,   //1 15 19

		ADC_CHANNEL_BM, ADC_CHANNEL_IR6, ADC_CHANNEL_EIR4,   //0  17 8
		ADC_CHANNEL_RM, ADC_CHANNEL_IR5, ADC_CHANNEL_EIR1,   //4  16 5
		ADC_CHANNEL_HD, ADC_CHANNEL_COUNT, ADC_CHANNEL_EIR3, //21  22 7
		ADC_CHANNEL_LM, ADC_CHANNEL_COUNT, ADC_CHANNEL_EIR2  //3   22 6      //0 22 
	}; 

	for (int i = 0; i < count; i++)  //每个通道的数据长度
	{
		for (int j = 0; j < 8; j++)   //每个点八次采样，每次20个字节 //对应选择通道
		{
			*(short *)tmp = *(value[channelMap[j * 3]] + i);
			if (channelMap[j * 3 + 1] != ADC_CHANNEL_COUNT) //channelMap[22] j=7 or channelMap[19] j=6
			{
				*(short *)(tmp + 4) = *(value[channelMap[j * 3 + 1]] + i);
			}
			*(short *)(tmp + 6) = 2;
			*(short *)(tmp + 8) = *(value[channelMap[j * 3 + 2]] + i);
			*(short *)(tmp + 10) = 4;
			*(short *)(tmp + 12) = *(code + i);
			*(char *)(tmp + 16) = j;  //
			tmp = tmp + 20;
		}
	}

	file.Write(writeData, dataLen);
	file.Close();

	delete[] writeData;
	return true;
}

bool LocalFileSaver::SaveCISData(Cash *data)
{
	if (_tcscmp(cashSavePath_, _T("")) == 0)
	{
		return true;
	}

	return SaveCISData(data, cashSavePath_);
}

bool LocalFileSaver::SaveCISData(Cash *data, LPCTSTR folder)
{
	if (_tcscmp(folder, _T("")) == 0)
	{
		return false;
	}

	TCHAR path[MAX_PATH] = { 0 };
	TCHAR fileName[128] = { 0 };
	char sn[24] = { 0 };
	strcpy(sn, data->GetSN());
	if (sn[0] == '\0')
	{
		sprintf(sn, "0");
	}
	wsprintf(fileName, _T("%04d%02d%02d_%02d%02d%02d_%03d_Err%03d_LastErr0x0_Den%d_Dir%d_Ver%02d_SN%s_cis_top.bmp"),
		runCashStartTime.wYear, runCashStartTime.wMonth, runCashStartTime.wDay,
		runCashStartTime.wHour, runCashStartTime.wMinute, runCashStartTime.wSecond,
		data->Id(),
		data->GetError(), data->GetDenomination(), data->GetDirection(), data->GetVersion(),
		CA2T(sn));
	GenerateSavePath(fileName, folder, path);
	if (data->GetTopImage()->SaveToFile(path))
	{
		wsprintf(fileName, _T("%04d%02d%02d_%02d%02d%02d_%03d_Err%03d_LastErr0x0_Den%d_Dir%d_Ver%02d_SN%s_cis_bottom.bmp"),
			runCashStartTime.wYear, runCashStartTime.wMonth, runCashStartTime.wDay,
			runCashStartTime.wHour, runCashStartTime.wMinute, runCashStartTime.wSecond,
			data->Id(),
			data->GetError(), data->GetDenomination(), data->GetDirection(), data->GetVersion(),
			CA2T(sn));
		GenerateSavePath(fileName, cashSavePath_, path);
		return data->GetBottomImage()->SaveToFile(path);
	}
	else
	{
		return false;
	}
}

bool LocalFileSaver::SaveCISAdjustData(LPCTSTR name, const void *data, int length)
{
	//if (_tcscmp(CISAdjSavePath_, _T("")) == 0)
	//{
	//	return true;
	//}

	//Bitmap *image = (Bitmap *)data;
	//TCHAR path[MAX_PATH] = { 0 };
	//GenerateSavePath(name, CISAdjSavePath_, path);
	//return image->SaveToFile(path);

	return true;
}

bool LocalFileSaver::SaveCashInfo(Cash *data)
{
	if (_tcscmp(cashSavePath_, _T("")) == 0)
	{
		return true;
	}

	TCHAR path[MAX_PATH] = { 0 };
	TCHAR fileName[128] = { 0 };
	wsprintf(fileName, _T("CashInfo.txt"));
	GenerateSavePath(fileName, cashSavePath_, path);

	CStdioFile cashInfoFile;
	if (!cashInfoFile.Open(path, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
	{
		return false;
	}

	CString strData;
	strData.Format(_T("%d\t%d\t%d\t0x%03X\t%d\t%s\r\n"),
		data->Id(),
		data->GetDenomination(),
		data->GetVersion(),
		data->GetError(),
		data->GetDirection(),
		(LPCTSTR)CA2T(data->GetSN()));
	cashInfoFile.SeekToEnd();
	cashInfoFile.WriteString(strData);
	cashInfoFile.Close();

	return true;
}

void LocalFileSaver::GenerateSavePath(LPCTSTR name, LPCTSTR folder, TCHAR *path)
{
	_tcscpy(path, folder);
	PathAddBackslash(path);
	_tcscat(path, name);
}

void LocalFileSaver::CreateCISAdjustSavePath()
{
	// 从可变配置文件中获取指定保存地址
	VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();
	int localSave = config->GetIntParameter(_T("LocalSave"), 0);
	if (localSave == 0)
	{
		CISAdjSavePath_[0] = _T('\0');
		return;
	}

	CString savePath = config->GetStringParameter(_T("LocalSavePath"), _T(""));
	if (savePath == _T(""))
	{
		CISAdjSavePath_[0] = _T('\0');
		return;
	}

	_tcscpy(CISAdjSavePath_, savePath);
	PathAddBackslash(CISAdjSavePath_);
	SYSTEMTIME curTime;
	GetLocalTime(&curTime);
	CString date;
	date.Format(_T("%04d-%02d-%02d"), curTime.wYear, curTime.wMonth, curTime.wDay);
	_tcscat(CISAdjSavePath_, date);
	if (!CreateDirectoryRecusively(CISAdjSavePath_)) {
		CISAdjSavePath_[0] = _T('\0');
		return;
	}

	PathAddBackslash(CISAdjSavePath_);
	CString time;
	time.Format(_T("CISAdjust_%02d-%02d-%02d"), curTime.wHour, curTime.wMinute, curTime.wSecond);
	_tcscat(CISAdjSavePath_, time);
	if (!CreateDirectoryRecusively(CISAdjSavePath_)) {
		CISAdjSavePath_[0] = _T('\0');
		return;
	}
	PathAddBackslash(CISAdjSavePath_);
}

void LocalFileSaver::CreateCashSavePath()
{
	// 从可变配置文件中获取指定保存地址
	GetLocalTime(&runCashStartTime);

	VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();
	int localSave = config->GetIntParameter(_T("LocalSave"), 0);
	if (localSave == 0)
	{
		cashSavePath_[0] = _T('\0');
		return;
	}

	CString savePath = config->GetStringParameter(_T("LocalSavePath"), _T(""));
	if (savePath == _T(""))
	{
		cashSavePath_[0] = _T('\0');
		return;
	}

	_tcscpy(cashSavePath_, savePath);
	PathAddBackslash(cashSavePath_);
	CString date;
	date.Format(_T("%04d-%02d-%02d"), runCashStartTime.wYear, runCashStartTime.wMonth, runCashStartTime.wDay);
	_tcscat(cashSavePath_, date);
	if (!CreateDirectoryRecusively(cashSavePath_)) {
		cashSavePath_[0] = _T('\0');
		return;
	}
	PathAddBackslash(cashSavePath_);
	CString time;
	time.Format(_T("WaveData_%02d-%02d-%02d"), runCashStartTime.wHour, runCashStartTime.wMinute, runCashStartTime.wSecond);
	_tcscat(cashSavePath_, time);
	if (!CreateDirectoryRecusively(cashSavePath_)) {
		cashSavePath_[0] = _T('\0');
		return;
	}
	PathAddBackslash(cashSavePath_);
}