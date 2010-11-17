#pragma once

#include <wtl/atlapp.h>
#include <wtl/atlgdi.h>

// �öԻ�����ı��а�ɫ��������XP�¶Ի����Ժÿ�һЩ��

template <typename T>
class CWhiteBkDlgImpl
{
public:
	BEGIN_MSG_MAP(CWhiteBkDlgImpl)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORSCROLLBAR, OnCtlColorScrollbar)
	END_MSG_MAP()

	void SetBkColor(COLORREF cr)
	{
		if (!m_brs.IsNull())
		{
			m_brs.DeleteObject();
		}
		m_brs.CreateSolidBrush(cr);
	}

protected:
	LRESULT OnCtlColorDlg(UINT, WPARAM, LPARAM, BOOL&)
	{
		if (!m_brs.IsNull())
		{
			return (LRESULT)(HBRUSH)m_brs;
		}
		else
		{
			return (LRESULT)::GetStockObject(WHITE_BRUSH);
		}
	}
	LRESULT OnCtlColorStatic(UINT, WPARAM, LPARAM, BOOL&)
	{
		if (!m_brs.IsNull())
		{
			return (LRESULT)(HBRUSH)m_brs;
		}
		else
		{
			return (LRESULT)::GetStockObject(WHITE_BRUSH);
		}
	}
	LRESULT OnCtlColorScrollbar(UINT, WPARAM, LPARAM, BOOL&)
	{
		if (!m_brs.IsNull())
		{
			return (LRESULT)(HBRUSH)m_brs;
		}
		else
		{
			return (LRESULT)::GetStockObject(WHITE_BRUSH);
		}
	}

	CBrush m_brs;
};
