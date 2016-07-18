#pragma once

#include "singleton.h"

class Cash;
//class Bitmap;

class LocalFileSaver
{
public:
	bool SaveADCData(Cash *data);
	bool SaveADCData(Cash *data, LPCTSTR folder);
	bool SaveCISData(Cash *data);
	bool SaveCISData(Cash *data, LPCTSTR folder);
	bool SaveCashInfo(Cash *data);
	bool SaveCISAdjustData(LPCTSTR name, const void *data, int length);
	void CreateCashSavePath();
	void CreateCISAdjustSavePath();

protected:
	void GenerateSavePath(LPCTSTR name, LPCTSTR folder, TCHAR *path);
	void DeleteDirectory(LPCTSTR path);

private:
	TCHAR	cashSavePath_[MAX_PATH];
	TCHAR   CISAdjSavePath_[MAX_PATH];
	SYSTEMTIME runCashStartTime;

	LocalFileSaver();
	virtual ~LocalFileSaver();

	DECLARE_SINGLETON(LocalFileSaver);
};