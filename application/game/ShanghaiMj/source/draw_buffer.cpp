#include "stdafx.h"

#include "../include/smj_ctrl.h"

CDrawBuffer::CDrawBuffer(void)
{

}

CDrawBuffer::~CDrawBuffer(void)
{
	Destory();
}

void CDrawBuffer::Create(CDC * dc, size_t w, size_t h)
{
	m_width = w, m_height = h;
	CreateCompatibleDC(dc);
	m_bmp.CreateCompatibleBitmap(dc, w, h);
	SelectObject(&m_bmp);
}

void CDrawBuffer::Destory(void)
{
	if (m_hDC)
	{
		DeleteDC();
		m_bmp.DeleteObject();
	}
}

void CDrawBuffer::CopyTransporent(CDrawBuffer & buf, size_t offset_x, size_t offset_y, UINT backgnd)
{
	size_t w, h;
	buf.GetSize(w, h);
	w -= offset_x, h -= offset_y;
	if (w > m_width) w = m_width;
	if (h > m_height) h = m_height;
	TransparentBlt(0, 0, w, h,	static_cast<CDC*>(&buf),
		offset_x, offset_y, w, h, backgnd);
}

void CDrawBuffer::Clear(UINT color)
{
	SetBkColor(color);
	FillSolidRect(CRect(0, 0, m_width-1, m_height-1), color);
}


