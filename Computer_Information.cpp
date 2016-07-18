#include "stdafx.h"
#include "Computer_Information.h"

bool GetTerminalID(TCHAR strID[200])
{
	strID[0] = _T('\0');

	TCHAR MACAddr[14] = { 0 };
	TCHAR CPUID[18] = { 0 };
	TCHAR HardDiskID[10] = { 0 };
	if (GetMACadress(MACAddr))
	{
		_tcscat(strID, MACAddr);
		_tcscat(strID, _T("-"));
	}
	if (GetCPU_ID(CPUID))
	{
		_tcscat(strID, CPUID);
		_tcscat(strID, _T("-"));
	}
	if (GetHdiskID(HardDiskID))
	{
		_tcscat(strID, HardDiskID);
	}

	if (strID[0] == _T('\0'))
	{
		wsprintf(strID, _T("?????"));
		return false;
	}
	else
	{
		return true;
	}
}

///////////获取MAC地址函数///////////////////
bool GetMACadress(TCHAR ptrMACAdress[14])
{
	int j = 0;

	DWORD index = 0;
	PGAINFO pGAInfo;//函数指针

	HINSTANCE hInst;//实例句柄
	hInst = LoadLibrary(L"iphlpapi.dll");//加载IP Helper API 所需的库文件

	if (!hInst)  return false; //不成功，返回FALSE

	pGAInfo = (PGAINFO)GetProcAddress(hInst, "GetAdaptersInfo");
	PIP_ADAPTER_INFO pInfo = NULL ;
	ULONG ulSize = 0;
	pGAInfo(pInfo, &ulSize);//第一次调用，获取缓冲区大小
	
	pInfo = (PIP_ADAPTER_INFO)(new char[ulSize]);
	
	pGAInfo(pInfo, &ulSize);

	wsprintf(ptrMACAdress, _T("%x%x%x%x%x%x"),
	pInfo->Address[0],pInfo->Address[1],pInfo->Address[2],
	pInfo->Address[3],pInfo->Address[4],pInfo->Address[5]);

	char *pTmp = (char *)pInfo;
	delete [] pTmp;
	pInfo = NULL;
	
	return true;                //成功，返回TRUE
}


bool GetCPU_ID(TCHAR CPU_ID[18])
{
	unsigned long s1, s2;
	_asm
	{
		mov eax, 03h
		xor ecx, ecx
		xor edx, edx
		cpuid
		mov s1, edx
		mov s2, ecx

		mov eax, 01h
		xor edx, edx
		cpuid
		mov s1, eax
	}

	wsprintf(CPU_ID, _T("%08x-%08X"), s2, s1);

	return true;
}


bool GetHdiskID(TCHAR DiskID[10])
{
	DWORD Serial;
	DWORD Length;
	BOOL success = GetVolumeInformation(_T("C:\\"), NULL, MAX_PATH, &Serial, &Length, NULL, NULL, MAX_PATH);
	if (!success)
	{
		return false;
	}

	wsprintf(DiskID, _T("%x"), Serial);
	return true;
} 
