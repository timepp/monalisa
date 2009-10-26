#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlctrls.h>
#include <string>
#include "resource.h"

class CIconDlg : public CDialogImpl<CIconDlg>
{
public:
	enum {IDD = IDD_ICON};
	CIconDlg(const std::wstring& icon_file, int icon_index);
	int GetSelectedIndex() const;

	BEGIN_MSG_MAP(CProgDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	std::wstring m_icon_file;
	int m_icon_index;
	CListViewCtrl m_list;
	void SetListIcon(int select_index);
};
