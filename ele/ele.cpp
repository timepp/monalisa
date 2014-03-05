#include <windows.h>

#define SMNAME L"ELE{8A7A7823-C4B2-4F5A-84DA-11C1B6451AD7}"
#define PROGNAME L"ELE"

struct SharedData
{
    WCHAR szCommandLine[4096];
    WCHAR szWorkingDir[MAX_PATH];
    WCHAR szEnvironmentBlock[65536];
};

void Check(BOOL bCondition)
{
	if (!bCondition)
	{
		DWORD dwErr = ::GetLastError();
		LPCWSTR pMsg;
		::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErr, 0, (LPWSTR)&pMsg, 0, NULL);
		::MessageBoxW(NULL, pMsg, PROGNAME, MB_OK|MB_ICONWARNING|MB_TOPMOST);
		ExitProcess(1);
	}
}

LPWSTR GetParamPos(LPWSTR lpCmdLine)
{
	BOOL inDoubleQuote = FALSE;
	while (*lpCmdLine > L' ' || (*lpCmdLine && inDoubleQuote))
	{
		if (*lpCmdLine == L'\"') inDoubleQuote = !inDoubleQuote;
		++lpCmdLine;
	}
	while (*lpCmdLine && (*lpCmdLine <= L' ')) 
	{
		lpCmdLine++;
	}

	return lpCmdLine;
}

void ExecuteAndWait(LPCWSTR pVerb, LPCWSTR pPath, LPCWSTR pParam, LPCWSTR pDir, BOOL bWaitExit)
{
	SHELLEXECUTEINFOW sei;
	sei.hwnd = NULL;
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.lpVerb = pVerb;
	sei.lpFile = pPath;
	sei.lpParameters = pParam;
	sei.lpDirectory = pDir;
	sei.nShow = SW_SHOW;
	BOOL bRet = ::ShellExecuteExW(&sei);

	if (bWaitExit && sei.hProcess != NULL)
	{
		::WaitForSingleObject(sei.hProcess, INFINITE);
	}
	//::CloseHandle(sei.hProcess);
}


int Ele()
{
	LPWSTR lpCmd = GetParamPos(::GetCommandLineW());
	if (lstrcmpW(lpCmd, L"UAC") == 0)
	{
		// get CD and command from shared memory
		HANDLE hMapping = ::OpenFileMappingW(FILE_MAP_READ|FILE_MAP_WRITE, FALSE, SMNAME);
		Check(hMapping != NULL);
		LPVOID pAddr = ::MapViewOfFile(hMapping, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, sizeof(SharedData));
		Check(pAddr != NULL);

        SharedData *pData = static_cast<SharedData*>(pAddr);

        STARTUPINFO si = {};
        PROCESS_INFORMATION pi = {};
        si.cb = sizeof(si);
        ::CreateProcessW(
            NULL, 
            pData->szCommandLine, 
            NULL, 
            NULL, 
            FALSE, 
            CREATE_UNICODE_ENVIRONMENT, 
            pData->szEnvironmentBlock, 
            pData->szWorkingDir,
            &si,
            &pi
            );
	}
	else if (!*lpCmd)
	{
		::MessageBoxW(NULL, 
			L"以管理员身份运行程序(弹出UAC框以确认)。以ele提升权限的程序会保持启动ele时的当前目录。\n"
			L"\n"
			L"用法：\n"
			L"ele <要运行的程序名> <程序参数>\n"
			L"\n"
			L"VERSION: 2.0\n" 
			,PROGNAME, MB_OK|MB_ICONINFORMATION);
	}
	else
	{
		// prepare shared memory
		WCHAR szPath[MAX_PATH + 100];
		DWORD dwPathLen = ::GetTempPathW(MAX_PATH, szPath);
		lstrcpyW(szPath + dwPathLen, L"\\ele59.dat");
		HANDLE hFile = ::CreateFileW(szPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		Check(hFile != INVALID_HANDLE_VALUE);
		HANDLE hMapping = ::CreateFileMappingW(hFile, NULL, PAGE_READWRITE, 0, sizeof(SharedData), SMNAME);
		Check(hMapping != NULL);
		LPVOID pAddr = ::MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, sizeof(SharedData));
		Check(pAddr != NULL);

        SharedData *pData = static_cast<SharedData*>(pAddr);
        ::GetCurrentDirectoryW(_countof(pData->szWorkingDir), pData->szWorkingDir);
		lstrcpyW(pData->szCommandLine, lpCmd);

        PCWSTR pszEnv = GetEnvironmentStringsW();
        for (size_t i = 0; i < _countof(pData->szEnvironmentBlock); i++)
        {
            if (pszEnv[i] == L'\0' && pszEnv[i + 1] == L'\0')
            {
                pData->szEnvironmentBlock[i] = L'\0';
                pData->szEnvironmentBlock[i+1] = L'\0';
                break;
            }
            else
            {
                pData->szEnvironmentBlock[i] = pszEnv[i];
            }
        }

		// restart in UAC
		::GetModuleFileNameW(NULL, szPath, MAX_PATH);
        ExecuteAndWait(L"runas", szPath, L"UAC", NULL, TRUE);
/*
		::UnmapViewOfFile(pAddr);
		::CloseHandle(hMapping);
		::CloseHandle(hFile);
*/
	}

	ExitProcess(0);
	return 0;
}

