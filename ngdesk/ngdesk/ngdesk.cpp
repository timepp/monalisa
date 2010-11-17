#include <atlbase.h>
#include <wtl/atlapp.h>
#include <auto_release.h>
#include <tplib.h>
#include "wallpapermgr.h"

#include "maindlg.h"

CAppModule _Module;
CMainDlg _dlg;
HANDLE _event;

void WakeThread(void *)
{
	for (;;)
	{
		WaitForSingleObject(_event, INFINITE);
		tp::log(L"收到激活窗口通知");
		_dlg.ShowWindow(SW_SHOWNORMAL);
		::SetForegroundWindow(_dlg.m_hWnd);
	}
}

int WINAPI wWinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPWSTR lpCmdLine, __in int nShowCmd )
{
	_Module.Init(NULL, hInstance);

#ifdef DEBUG
	//tp::sc::log_default_console_config();
#endif

	_event = CreateEventW(NULL, FALSE, FALSE, L"nangua_desk");
	ON_LEAVE_1(CloseHandle(_event), HANDLE, _event);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		SetEvent(_event);
		return 0;
	}

	_beginthread(&WakeThread, 0, NULL);

	CWallPaperMgr::Instance()->Run();

	_dlg.DoModal();

	_Module.Term();
	return 0;
}
