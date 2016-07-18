#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <Shlwapi.h>
#include <Wininet.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Wininet.lib")

#define SVCNAME TEXT("ZMVision AK47 Upload Service")

SERVICE_STATUS          gSvcStatus; 
SERVICE_STATUS_HANDLE   gSvcStatusHandle; 
HANDLE                  ghSvcStopEvent = NULL;

VOID SvcInstall();
VOID SvcUninstall();
VOID WINAPI SvcCtrlHandler( DWORD ); 
VOID WINAPI SvcMain( DWORD, LPTSTR * ); 

VOID ReportSvcStatus( DWORD, DWORD, DWORD );
VOID SvcInit( DWORD, LPTSTR * ); 
VOID SvcReportEvent( LPTSTR );


//
// Purpose: 
//   Entry point for the process
//
// Parameters:
//   None
// 
// Return value:
//   None
//
void __cdecl _tmain(int argc, TCHAR *argv[]) 
{
    // If command-line parameter is "install", install the service. 
    // Otherwise, the service is probably being started by the SCM.
    if( argc >= 2)
    {
      if (lstrcmpi( argv[1], TEXT("install")) == 0) {
        SvcUninstall();
        SvcInstall();
        return;
      } else if (lstrcmpi( argv[1], TEXT("uninstall")) == 0) {
        SvcUninstall();
        return;
      } 
    }

    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { SVCNAME, (LPSERVICE_MAIN_FUNCTION) SvcMain }, 
        { NULL, NULL } 
    }; 
 
    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.

    if (!StartServiceCtrlDispatcher( DispatchTable )) 
    { 
        SvcReportEvent(TEXT("StartServiceCtrlDispatcher")); 
    } 
} 

BOOL ContainSpace(LPCTSTR str) {
  while (*str) {
    if (*str == ' ') {
      return TRUE;
    }
    str++;
  }
  return FALSE;
}

void BuildCommand(LPCTSTR szPath, int argc, TCHAR *argv[], TCHAR *szCommand) {
  szCommand[0] = '\0';

  if (ContainSpace(szPath)) {
    _tcscat(szCommand, TEXT("\""));
    _tcscat(szCommand, szPath);
    _tcscat(szCommand, TEXT("\""));
  } else {
    _tcscat(szCommand, szPath);
  }
  for (int i = 0; i < argc; i++) {
    _tcscat(szCommand, TEXT(" "));
    if (ContainSpace(argv[i])) {
      _tcscat(szCommand, TEXT("\""));
      _tcscat(szCommand, argv[i]);
      _tcscat(szCommand, TEXT("\""));
    } else {
      _tcscat(szCommand, argv[i]);
    }
  }
}

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID SvcInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szPath[MAX_PATH];
    //TCHAR szCommand[16 * 1024];

    if( !GetModuleFileName( NULL, szPath, MAX_PATH ) )
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return;
    }

    //BuildCommand(szPath, argc, argv, szCommand);

    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Create the service

    schService = CreateService( 
        schSCManager,              // SCM database 
        SVCNAME,                   // name of service 
        SVCNAME,                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_AUTO_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 
 
    if (schService == NULL) 
    {
        printf("CreateService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        return;
    }
    else printf("Service installed successfully\n"); 

    if (!StartService(schService, 0, NULL)) {
      printf("StartService failed (%d)\n", GetLastError()); 
      CloseServiceHandle(schService); 
      CloseServiceHandle(schSCManager);
      return;
    }
    else printf("Service start successfully\n"); 

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}

//
// Purpose: 
//   Uninstalls a service from the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID SvcUninstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS status;
    
    // Get a handle to the SCM database. 
 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    schService = OpenService(schSCManager, SVCNAME, SC_MANAGER_ALL_ACCESS);
    if (NULL == schService) {
      CloseServiceHandle(schSCManager);
      printf("OpenService failed (%d)\n", GetLastError());
      return;
    }

    ControlService(schService, SERVICE_CONTROL_STOP, &status);

    if (!DeleteService(schService)) {
      CloseServiceHandle(schService);
      CloseServiceHandle(schSCManager);
      printf("DeleteService failed (%d)\n", GetLastError());
      return;
    } else {
      printf("Service uninstalled successfully\n"); 
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI SvcMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
    // Register the handler function for the service

    gSvcStatusHandle = RegisterServiceCtrlHandler( 
        SVCNAME, 
        SvcCtrlHandler);

    if( !gSvcStatusHandle )
    { 
        SvcReportEvent(TEXT("RegisterServiceCtrlHandler")); 
        return; 
    } 

    // These SERVICE_STATUS members remain as set here

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    gSvcStatus.dwServiceSpecificExitCode = 0;    

    // Report initial status to the SCM

    ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

    // Perform service-specific initialization and work.

    SvcInit( dwArgc, lpszArgv );
}

static unsigned char CodeBook[256] = {
	0x48, 0x1f, 0xb4, 0xf1, 0xe8, 0x94, 0xb2, 0xf2, 0xd9, 0xb8, 0x17, 0x46, 0xa9, 0x00, 0x3a, 0x56, 
	0x7a, 0x30, 0x66, 0xc9, 0xc1, 0xdd, 0x01, 0x75, 0x49, 0x14, 0x0d, 0xa8, 0xc7, 0x8e, 0x5d, 0x7b, 
	0x7d, 0xa7, 0xc4, 0x16, 0xbe, 0x61, 0x91, 0x31, 0xa0, 0xf0, 0x35, 0xfe, 0xc0, 0x60, 0x6e, 0x2a, 
	0xf6, 0x24, 0x2c, 0xce, 0x6a, 0xc8, 0x4a, 0xad, 0x52, 0x8f, 0xe1, 0x2b, 0x6b, 0x25, 0x41, 0xf7, 
	0xa3, 0x1d, 0xd6, 0x0e, 0xfa, 0x81, 0x62, 0xc6, 0x9b, 0x2f, 0x28, 0x34, 0xbc, 0x47, 0xba, 0xb9, 
	0xeb, 0xcf, 0x9d, 0x32, 0xfb, 0x58, 0x07, 0x7c, 0x1a, 0xdc, 0x69, 0x4d, 0xcd, 0x9f, 0x86, 0xe2, 
	0x38, 0x98, 0x9e, 0xfd, 0x88, 0x1c, 0xa6, 0x12, 0xed, 0x9c, 0x20, 0xb5, 0xdf, 0x99, 0xb6, 0x54, 
	0x13, 0xf8, 0x80, 0xaa, 0x21, 0x5a, 0xd4, 0x55, 0x44, 0x06, 0xe0, 0x76, 0x9a, 0x05, 0xf5, 0x5e, 
	0xda, 0xec, 0xbd, 0xd2, 0xa5, 0xde, 0x40, 0xff, 0xfc, 0x42, 0x39, 0x3b, 0x71, 0x6d, 0x4f, 0x10, 
	0x78, 0x92, 0xf9, 0xd3, 0xd1, 0x96, 0x1b, 0x90, 0x7f, 0x5c, 0x3e, 0xb1, 0x0a, 0x45, 0x2e, 0x19, 
	0xa1, 0x0c, 0x6c, 0xe6, 0x26, 0xca, 0x85, 0x4b, 0x23, 0x4e, 0xcb, 0x8a, 0xf4, 0x7e, 0x15, 0x08, 
	0x3f, 0x8c, 0x0f, 0xb0, 0x33, 0x57, 0x04, 0xc3, 0xaf, 0xa2, 0x4c, 0x3d, 0x93, 0xc2, 0x09, 0x29, 
	0x84, 0x36, 0xbf, 0x79, 0xee, 0x11, 0x72, 0x65, 0xef, 0xe4, 0x8b, 0x22, 0xe9, 0x3c, 0x18, 0xb7, 
	0xcc, 0x5f, 0x5b, 0x1e, 0x27, 0xe5, 0x87, 0x50, 0x6f, 0x97, 0x43, 0x51, 0xdb, 0xe3, 0xb3, 0xab, 
	0x73, 0xa4, 0xd8, 0xd0, 0x64, 0xbb, 0x02, 0xac, 0x77, 0x37, 0x8d, 0xea, 0xe7, 0x68, 0x0b, 0x70, 
	0x89, 0x74, 0x59, 0xc5, 0x95, 0xd7, 0xf3, 0xd5, 0xae, 0x67, 0x63, 0x82, 0x2d, 0x53, 0x03, 0x83, 
};

inline int Hex(TCHAR ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  } else if (ch >= 'a' && ch <= 'f') {
    return ch - 'a' + 10;
  } else if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  }
  return 0;
}

int FtpDecryptPassword(const TCHAR *crypted, TCHAR *decrypted) {
  int length = 0;
  const TCHAR *p = crypted;
  while (*p != '\0' && *(p + 1) != '\0') {
    int code = (Hex(*p) << 4) | Hex(*(p + 1));
    decrypted[length++] = CodeBook[code];
    p += 2;
  }
  decrypted[length] = 0;
  return length;
}

//
// Purpose: 
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None
//
VOID SvcInit( DWORD dwArgc, LPTSTR *lpszArgv)
{
    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with 
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.
    TCHAR szLocalPath[MAX_PATH] = { 0 };
    TCHAR szConfigFilePath[MAX_PATH];
    TCHAR szLogFilePath[MAX_PATH] = { 0 };
    TCHAR szServerIP[256] = { 0 };
    TCHAR szUserName[256] = { 0 };
    TCHAR szPassword[256] = { 0 };
    TCHAR szRemotePath[MAX_PATH] = { 0 };
    int nServerPort = 21;
    TCHAR szFilter[512] = { 0 };
    //HANDLE hLogFile = INVALID_HANDLE_VALUE;
    HINTERNET hInternet = NULL;
    HINTERNET hSession = NULL;  
    /*DWORD i = 0;
    while (i + 1 < dwArgc) {
      if (lstrcmpi( lpszArgv[i], TEXT("-l")) == 0) {
        _tcscpy(szLogFilePath, lpszArgv[i + 1]);
      } else if (lstrcmpi( lpszArgv[i], TEXT("-d")) == 0) {
        _tcscpy(szDirectory, lpszArgv[i + 1]);
      }
      i++;
    }*/
    GetModuleFileName( NULL, szConfigFilePath, MAX_PATH );
    PathRemoveFileSpec(szConfigFilePath);
    PathAddBackslash(szConfigFilePath);
    _tcscat(szConfigFilePath, TEXT("FTPCONFIG.ini"));
    GetPrivateProfileString(TEXT("Upload"), TEXT("ServerIP"), NULL, 
        szServerIP, _countof(szServerIP), szConfigFilePath);
    nServerPort = GetPrivateProfileInt(TEXT("Upload"), TEXT("ServerPort"), 
        21, szConfigFilePath);;
    GetPrivateProfileString(TEXT("Upload"), TEXT("User"), NULL, 
        szUserName, _countof(szUserName), szConfigFilePath);
    FtpDecryptPassword(szUserName, szUserName);
    GetPrivateProfileString(TEXT("Upload"), TEXT("Password"), NULL, 
        szPassword, _countof(szPassword), szConfigFilePath);
    FtpDecryptPassword(szPassword, szPassword);
    GetPrivateProfileString(TEXT("Upload"), TEXT("Filter"), NULL, 
        szFilter, _countof(szFilter), szConfigFilePath);
    GetPrivateProfileString(TEXT("Upload"), TEXT("LocalPath"), NULL, 
        szLocalPath, _countof(szLocalPath), szConfigFilePath);
    GetPrivateProfileString(TEXT("Upload"), TEXT("RemotePath"), NULL, 
        szRemotePath, _countof(szRemotePath), szConfigFilePath);
    if (_tcslen(szLocalPath) == 0) {
      ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
      return;
    }
    PathAddBackslash(szLocalPath);

    /*hLogFile = CreateFile(szLogFilePath, 
        GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hLogFile == INVALID_HANDLE_VALUE) {
      ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
      return;
    }*/

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.
    ghSvcStopEvent = CreateEvent(
                         NULL,    // default security attributes
                         TRUE,    // manual reset event
                         FALSE,   // not signaled
                         NULL);   // no name

    if ( ghSvcStopEvent == NULL)
    {
      //CloseHandle(hLogFile);
      ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
      return;
    }

    // Report running status when initialization is complete.

    ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );

    // TO_DO: Perform work until service stops.

    while(1)
    {
        // Check whether to stop the service.
        if (WaitForSingleObject(ghSvcStopEvent, 500) != WAIT_TIMEOUT) {
          break;
        }
        if (hInternet == NULL) {
          hInternet = InternetOpen(SVCNAME, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
          if (hInternet == NULL) {
            continue;
          }
        }
        if (hSession == NULL) {
          hSession = InternetConnect(hInternet, 
              szServerIP, nServerPort, 
              szUserName, szPassword, 
              INTERNET_SERVICE_FTP,
              INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_PASSIVE, 
              NULL);
          if (hSession == NULL) {
            InternetCloseHandle(hInternet);
            hInternet = NULL;
            continue;
          }
        }
        TCHAR szFindMask[MAX_PATH];
        _tcscpy(szFindMask, szLocalPath);      
        _tcscat(szFindMask, TEXT("*"));
        WIN32_FIND_DATA wfd;
        HANDLE hFind = FindFirstFile(szFindMask, &wfd);
        if (hFind != INVALID_HANDLE_VALUE) {
          do {
            if (lstrcmpi(wfd.cFileName, TEXT(".")) != 0 &&
                lstrcmpi(wfd.cFileName, TEXT("..")) != 0 &&
                (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
              BOOL matched = TRUE;
              if (_tcslen(szFilter) > 0) {
                matched = FALSE;
                TCHAR *extension = PathFindExtension(wfd.cFileName);
                if (extension != NULL) {
                  TCHAR *part = _tcsstr(szFilter, extension + 1);
                  if (part != NULL) {
                    int pos = _tcslen(extension) - 1;
                    matched = (part[pos] == '|' || part[pos] == '\0');
                  }
                }
              }
              if (matched) {
                TCHAR szLocalFilePath[MAX_PATH], szRemoteFilePath[MAX_PATH];
                _tcscpy(szLocalFilePath, szLocalPath);      
                _tcscat(szLocalFilePath, wfd.cFileName);
                _tcscpy(szRemoteFilePath, szRemotePath);
                _tcscat(szRemoteFilePath, wfd.cFileName);
                if (FtpPutFile(hSession, szLocalFilePath, szRemoteFilePath, FTP_TRANSFER_TYPE_BINARY, NULL)) {
                  for (int retry = 0; retry < 5; retry++) {
                    if (DeleteFile(szLocalFilePath)) {
                      break;
                    }
                    Sleep(500);
                  }
                } else {
                  InternetCloseHandle(hSession);
                  InternetCloseHandle(hInternet);
                  hInternet = NULL;
                  hSession = NULL;
                  break;
                }
              }
            }
          } while (FindNextFile(hFind, &wfd));

          FindClose(hFind);
        }
    }

    //CloseHandle(hLogFile);

    if (hSession != NULL) {
      InternetCloseHandle(hSession);
    }
    if (hInternet != NULL) {
      InternetCloseHandle(hInternet);
    }

    ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
}

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
VOID ReportSvcStatus( DWORD dwCurrentState,
                      DWORD dwWin32ExitCode,
                      DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler( DWORD dwCtrl )
{
   // Handle the requested control code. 

   switch(dwCtrl) 
   {  
      case SERVICE_CONTROL_STOP: 
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

         // Signal the service to stop.

         SetEvent(ghSvcStopEvent);
         ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
         
         return;
 
      case SERVICE_CONTROL_INTERROGATE: 
         break; 
 
      default: 
         break;
   } 
   
}

//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID SvcReportEvent(LPTSTR szFunction) 
{ 
    //HANDLE hEventSource;
    //LPCTSTR lpszStrings[2];
    //TCHAR Buffer[80];

    //hEventSource = RegisterEventSource(NULL, SVCNAME);

    //if( NULL != hEventSource )
    //{
    //    StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

    //    lpszStrings[0] = SVCNAME;
    //    lpszStrings[1] = Buffer;

    //    ReportEvent(hEventSource,        // event log handle
    //                EVENTLOG_ERROR_TYPE, // event type
    //                0,                   // event category
    //                SVC_ERROR,           // event identifier
    //                NULL,                // no security identifier
    //                2,                   // size of lpszStrings array
    //                0,                   // no binary data
    //                lpszStrings,         // array of strings
    //                NULL);               // no binary data

    //    DeregisterEventSource(hEventSource);
    //}
}