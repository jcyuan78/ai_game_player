#include "stdafx.h"
#include "../include/puzzle_ctrl.h"

#define HSCROLL_ID	(10000)

IMPLEMENT_DYNAMIC(CPuzzleCtrl, CStatic)

CPuzzleCtrl::CPuzzleCtrl(void)
	: m_hoffset(0), m_voffset(0)
	, m_memdc(NULL), m_membmp(NULL)
{

}

CPuzzleCtrl::~CPuzzleCtrl(void)
{
	delete m_memdc;
	delete m_membmp;
}

BEGIN_MESSAGE_MAP(CPuzzleCtrl, CStatic)
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CPuzzleCtrl::OnPaint()
{
	CPaintDC dc(this);
	if (m_memdc && m_memdc->m_hDC)
	{
		dc.BitBlt(0, 0, m_wnd_width, m_wnd_height, 
			m_memdc, m_hoffset, m_voffset, SRCCOPY);
	}
}


void CPuzzleCtrl::InitializeCtrl(int scroll)
{
	CRect rect;
	GetClientRect(&rect);
	m_wnd_width = rect.Width();
	m_wnd_height = rect.Height();

	m_scroll_support = scroll;
	if (m_scroll_support & HSCROLL)
	{
		UINT this_id = GetDlgCtrlID();
		m_hscroll.Create(SBS_HORZ | SBS_BOTTOMALIGN | WS_CHILD | WS_VISIBLE,
			CRect(rect.left, rect.bottom - 20, rect.right, rect.bottom), this, 
			this_id + HSCROLL_ID);
	}

}

void CPuzzleCtrl::CreateMemDc(int w, int h)
{
	m_dc_width = w, m_dc_height = h;
	CPaintDC dc(this);

	if (m_membmp)
	{
		m_membmp->DeleteObject();
		delete m_membmp;
	}	

	if (m_memdc)
	{
		m_memdc->DeleteDC();
		delete m_memdc;
	}

	m_memdc = new CDC;
	m_membmp = new CBitmap;

	m_memdc->CreateCompatibleDC(&dc);
	m_membmp->CreateCompatibleBitmap(&dc, m_dc_width, m_dc_height);
	m_memdc->SelectObject(m_membmp);

	int hdelta = m_dc_width - m_wnd_width;
	if (m_scroll_support & HSCROLL)
	{
		if (hdelta > 0)
		{
			m_hscroll.EnableScrollBar(ESB_ENABLE_BOTH);
			m_hscroll.SetScrollRange(0, hdelta);
		}
		else
		{
			m_hscroll.EnableScrollBar(ESB_DISABLE_BOTH);
		}
	}
}


