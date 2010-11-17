#include "maindlg.h"

void CMainDlg::OnClose()
{
	EndDialog(IDOK);
}

LRESULT CMainDlg::OnInitDialog(HWND, LPARAM)
{
	CenterWindow();
	return 0;
}

LRESULT CMainDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CMainDlg::OnBnClickedButtonExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	OnClose();
	return 0;
}
