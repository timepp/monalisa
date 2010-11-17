#include "resource.h"

#include <atlbase.h>
#include <atlwin.h>
#include <wtl/atlcrack.h>
#include "whitebkdlg.h"

class CMainDlg : public CDialogImpl<CMainDlg>, public CWhiteBkDlgImpl<CMainDlg>
{
public:
	enum {IDD = IDD_MAINDLG};

private:
	BEGIN_MSG_MAP(CMainDlg)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(CWhiteBkDlgImpl<CMainDlg>)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDC_BUTTON_EXIT, BN_CLICKED, OnBnClickedButtonExit)
	END_MSG_MAP()

private:
	void OnClose();
	LRESULT OnInitDialog(HWND, LPARAM);
public:
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
