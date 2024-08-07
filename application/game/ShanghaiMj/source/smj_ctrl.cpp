#include "stdafx.h"

#include "../include/smj_ctrl.h"
#include "ShanghaiMjDlg.h"

LOCAL_LOGGER_ENABLE(_T("shmj.ctrl"), LOGGER_LEVEL_DEBUGINFO);

#define CELL_SIZE (16)
#define CARD_WIDTH (2*CELL_SIZE)
#define CARD_HEIGHT_2	(24)
#define CARD_HEIGHT (2*CARD_HEIGHT_2)

static CDrawTile g_drawtile;


//const wchar_t TILE_CHAR[36] = {
//	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
//	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
//	'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
//	'e', 'f', 'g', 'h', 'i', 'j'
//};
const wchar_t TILE_CHAR_KANJI[] = L"赵钱孙李周吴郑王冯陈褚卫蒋沈韩杨朱秦尤许何吕施张孔曹严华金魏陶姜戚谢邹喻柏水窦章云苏潘葛奚范彭郎";
const wchar_t TILE_CHAR_KANA[] =  L"あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよをん";
const wchar_t TILE_CHAR_ABCD[] =  L"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺあｂｃｄｆｇｈｊｋｌｍｎおｐｑえあいう";

///////////////////////////////////////////////////////////////////////////////
// -- Help functions

void CDrawTile::Initialzie(void)
{
	//m_show_tile_id = false;
	m_font_group.CreateFont(
		2*CELL_SIZE, 0, 0, 0,  // nHeight, nWidth, nEscapement, nOrientation
		FW_BOLD, FALSE, FALSE,	// nWeight, bItalic, bUnderline
		0, DEFAULT_CHARSET,  // cStrikeOut, nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Arial"));                 // lpszFacename

	m_font_tile.CreateFont(
		CELL_SIZE, 0, 0, 0,  // nHeight, nWidth, nEscapement, nOrientation
		FW_NORMAL, FALSE, FALSE,	// nWeight, bItalic, bUnderline
		0, ANSI_CHARSET,  // cStrikeOut, nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Arial"));                 // lpszFacename

	m_tile_char = TILE_CHAR_ABCD;
}

void CDrawTile::Draw(CDC & dc, int xx, int yy, CTile & tile, UINT color, UINT mode)
{
	// 显示牌（组号）
	int grp_id = tile.GetGroupId();
	int tile_id = tile.GetTileId() & 0xFF;
	CString str_group, str_tile;
	if (tile.GetTileId() == PLACEHOLDER)
	{
		str_group = "?";
	}
	else
	{
		str_group.Format(L"%c", m_tile_char[grp_id]);
		str_tile.Format(L"%d", tile_id);
	}

	CRect rect_card(xx, yy, xx + CARD_WIDTH, yy + CARD_HEIGHT);
	dc.FillSolidRect(rect_card, color);

	dc.SelectObject(&m_font_group);
	dc.DrawText(str_group, rect_card, DT_CENTER);
	//if (m_show_tile_id)
	if (mode & (SM_SHOW_ID | SM_STATE) )
	{	// 显示牌的id
		CRect r2(xx, yy + CARD_HEIGHT_2+8, xx + CARD_WIDTH, yy + CARD_HEIGHT);
		dc.SelectObject(&m_font_tile);
		if (mode & SM_STATE)	str_tile.Format(L"%02d", grp_id);
		dc.DrawText(str_tile, r2, DT_CENTER);
	}

	dc.DrawEdge(rect_card, EDGE_RAISED, BF_RECT);

	if (mode & SM_SELECTED)
	{
		CBrush *pBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
		CBrush *old_brush = dc.SelectObject(pBrush);
		CPen pen;
		pen.CreatePen(PS_SOLID | PS_COSMETIC, 2, RGB(255, 0, 0));
		CPen * old_pen = dc.SelectObject(&pen);

		dc.RoundRect(rect_card, CPoint(CARD_WIDTH + 5, CARD_HEIGHT + 5));

		dc.SelectObject(old_brush);
		dc.SelectObject(*old_pen);
	}
}

///////////////////////////////////////////////////////////////////////////////
// -- Board Control

IMPLEMENT_DYNAMIC(CShanghaiMjCtrl, CStatic)

#define BKGND_CLR (RGB(255, 255, 255))

CShanghaiMjCtrl::CShanghaiMjCtrl()
	: m_board(NULL)
	, m_layer(NULL), m_grid(NULL), m_mark_layer(NULL)
	, m_layers(0)
	, m_mode(MODE_EDIT)
	, m_sel_index(-1), m_sel_group(-1), m_highlight(true)
	, m_show_tile_id(false)
{
}

CShanghaiMjCtrl::~CShanghaiMjCtrl()
{
	RELEASE(m_board);
	delete[] m_layer;
	delete m_grid;
	delete m_mark_layer;
}

void CShanghaiMjCtrl::CleanMemDc(void)
{
	for (size_t ii = 0; ii < m_layers; ++ii)
	{
		m_layer[ii].Destory();
	}
	delete[] m_layer;
	m_layer = NULL;

	if (m_grid)
	{
		m_grid->Destory();
		delete m_grid;
		m_grid = NULL;
	}
	if (m_mark_layer)
	{
		m_mark_layer->Destory();
		delete m_mark_layer;
		m_mark_layer = NULL;
	}
}

void CShanghaiMjCtrl::SetBoard(IPuzzleBoard * board)
{
	CleanMemDc();
	RELEASE(m_board);

	CShanghaiMjBoard * _board = dynamic_cast<CShanghaiMjBoard*>(board);
	JCASSERT(_board);
	m_board = _board;
	m_board->AddRef();

	m_board->GetSize(m_rows, m_cols, m_layers);
	m_dc_width = (m_cols-1) * CELL_SIZE + 2 * CELL_SIZE;
	m_dc_height = m_rows * CARD_HEIGHT_2 + 2 * CELL_SIZE;

	CPaintDC dc(this);
	m_layer = new CDrawBuffer[m_layers];
	m_grid = new CDrawBuffer;
	for (size_t ii = 0; ii < m_layers; ++ii)
	{
		m_layer[ii].Create(&dc, m_dc_width, m_dc_height);
		DrawLayer(ii);
	}
	m_grid->Create(&dc, m_dc_width, m_dc_height);
	DrawGrid();
}

void CShanghaiMjCtrl::SetPlayMode(PLAY_MODE mode)
{
	m_mode = mode;
	RedrawWindow();
}

void CShanghaiMjCtrl::Initialize(CCardStatesCtrl * ctrl)
{
	JCASSERT(ctrl);
	CRect rect;
	GetClientRect(rect);
	m_wnd_width = rect.Width();
	m_wnd_height = rect.Height();

	CPaintDC dc(this);
	m_client.Create(&dc, m_wnd_width, m_wnd_height);
	m_state_ctrl = ctrl;
}

void CShanghaiMjCtrl::DrawLayer(size_t layer)
{
	LOG_DEBUG_(1, L"DrawLayer %d", layer);
	JCASSERT(m_board && m_layer && layer<m_layers);
	m_layer[layer].Clear(BKGND_CLR);

	CPen pen;
	pen.CreatePen(PS_SOLID | PS_COSMETIC, 2, RGB(0, 255, 0));
	m_layer[layer].SelectObject(&pen);

	// layer offset, 显示立体感
	size_t tile_num = m_board->GetTileNum();
	int layer_xx = layer * 5, layer_yy = layer * 5;
	UINT show_mode = 0;
	if (m_show_tile_id) show_mode |= CDrawTile::SM_SHOW_ID;
	for (size_t rr = 0; rr < m_rows; ++rr)
	{
		for (size_t cc = 1; cc < m_cols; ++cc)	//	最左边是guard
		{
			MJPOS pos = m_board->Pos2Index(layer, rr, cc);
			BYTE cidx = m_board->GetCidx(pos);
			if (cidx != PLACEHOLDER_ID && cidx != INVISIBLE_TILE && cidx >= tile_num) continue;
			CTile * tile = m_board->GetTileByPos(pos);	JCASSERT(tile);
			int yy = rr * CARD_HEIGHT_2 + CELL_SIZE - layer_yy;
			int xx = (cc-1) * CELL_SIZE + CELL_SIZE - layer_xx;
			
			//bool sel = false;
			UINT sel = 0;
			if (m_sel_index >= 0)
			{
				if (m_sel_index == cidx) sel = CDrawTile::SM_SELECTED;
			}
			if (m_marks.find(tile->GetTileId()) != m_marks.end()) sel = CDrawTile::SM_SELECTED;

			UINT txt_clr;
			CTile::STATE ss = tile->GetState();
			switch (ss)
			{
			case CTile::CST_INVISIBLE:	txt_clr = RGB(192, 192, 192);	break;
			case CTile::CST_VISIBLE:	
				if (tile->GetGroupId() == m_sel_group)	txt_clr = RGB(128, 0, 192);
				else									txt_clr = RGB(128, 0, 0);		
				break;
			case CTile::CST_MOVABLE:	
				if (tile->GetGroupId() == m_sel_group)	txt_clr = RGB(0, 0, 192);
				else 									txt_clr = RGB(0, 0, 0);			
				break;
			default:
				txt_clr = RGB(128, 128, 0); 
				break;
			}
			m_layer[layer].SetTextColor(txt_clr);
			g_drawtile.Draw(m_layer[layer], xx, yy, *tile, 
				RGB(0, 100 + layer * 25, 100 + layer * 25), show_mode | sel);
		}
	}				
}

void CShanghaiMjCtrl::DrawGrid()
{
	m_grid->Clear(BKGND_CLR);
	// draw grid
	CPen pen;
	pen.CreatePen(PS_SOLID | PS_COSMETIC, 1, RGB(128, 128, 128));
	m_grid->SelectObject(&pen);

	for (size_t rr = 0; rr <= m_rows; ++rr)
	{
		int yy = rr * CARD_HEIGHT_2 + CELL_SIZE;
		m_grid->MoveTo(CELL_SIZE, yy);
		m_grid->LineTo(m_dc_width - CELL_SIZE, yy);
	}

	for (size_t cc = 0; cc < m_cols; ++cc)	// m_cols包含guard
	{
		int xx = cc * CELL_SIZE + CELL_SIZE;
		m_grid->MoveTo(xx, CELL_SIZE);
		m_grid->LineTo(xx, m_dc_height - CELL_SIZE);
	}
}

bool CShanghaiMjCtrl::HitTest(const CPoint & p, size_t & col, size_t & row)
{
	int x0 = p.x - CELL_SIZE;
	int y0 = p.y - CELL_SIZE;
	if (x0 > 0 && y0 > 0)
	{
		size_t c = x0 / (CELL_SIZE);
		size_t r = y0 / (CARD_HEIGHT_2);
		if (c < (m_cols-1) && r < m_rows)
		{
			col = c+1;
			row = r;
			return true;
		}
	}
	return false;
}

MJPOS CShanghaiMjCtrl::HitVisibleCard(const CPoint & p)
{
	size_t row, col;
	bool br = HitTest(p, col, row);
	if (!br) return NULL;

	// 从最高层开始往下找可间的牌
	for (int tt = m_top_layer; tt >= 0; --tt)
	{
		MJPOS pos = m_board->Pos2Index(tt, row, col);
		CTile * tile = m_board->GetTileByPos(pos);
		if (tile) return pos;
	}
	return -1;
}

void CShanghaiMjCtrl::PutSelectCard(size_t col, size_t row)
{
	JCASSERT(m_state_ctrl && m_board);

	int grp = m_state_ctrl->GetSelectedGroup();
	MJPOS pos = m_board->Pos2Index(m_top_layer, row, col);
	if (col >= (m_cols - 1) || row >= (m_rows - 1)) return;

	bool updated = false;
	if (grp < 0)
	{	// 放置占位符，用于layout的编辑
		updated = m_board->PutPlaceholder(pos);
	}
	else
	{	// 放置指定的牌
		updated = m_board->PutTile(grp, pos);
	}
	if (updated)
	{
		m_state_ctrl->UpdateWnd();
		DrawLayer(m_top_layer);
		RedrawWindow();
	}
}

void CShanghaiMjCtrl::UpdateWnd(UINT layermask)
{
	UINT mask = 1;
	for (size_t ii = 0; ii < m_layers; ++ii, mask <<=1)
	{
		if (layermask & mask) DrawLayer(ii);
	}
	RedrawWindow();
}

void CShanghaiMjCtrl::AddMark(BYTE cidx)
{
	CTile * tile = m_board->GetTile(cidx);
	m_marks.insert(tile->GetTileId());
}

void CShanghaiMjCtrl::OnPaint()
{
	CPaintDC dc(this);
	if (m_client.m_hDC)
	{
		m_client.Clear(BKGND_CLR);
		UINT mask = 1;
		if (m_layer)
		{
			for (size_t ii = 0; ii <= m_top_layer; ++ii, mask >>= 1)
			{
				m_client.CopyTransporent(m_layer[ii], 0, 0, BKGND_CLR);
			}
		}
		if (m_grid && m_mode == MODE_EDIT) m_client.CopyTransporent(*m_grid, 0, 0, BKGND_CLR);
		dc.BitBlt(0, 0, m_wnd_width, m_wnd_height,
			static_cast<CDC*>(&m_client), 0, 0, SRCCOPY);
	}
}

void CShanghaiMjCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_mode == MODE_EDIT)
	{
		size_t row, col;
		bool hit = HitTest(point, col, row);
		if (!hit) return;
		PutSelectCard(col, row);
	}
	else if (m_mode == MODE_PLAY || m_mode == MODE_AI)
	{
		MJPOS pos = HitVisibleCard(point);
		if (pos < 0)
		{	// 该位置没有牌，取消选择
			m_sel_group = -1;
		}
		else
		{
			CTile * tile = m_board->GetTileByPos(pos);
			if (tile == NULL) return;
			if (tile->GetState() != CTile::CST_MOVABLE) return;
			BYTE cidx = m_board->GetTileIndex(pos);
			RemoveMarks();

			if (m_sel_index >= 0)
			{	// 准备消除		
				CMovement move(m_sel_index, cidx);
				bool br = m_board->PlayCard(move);
				if (br)
				{
					m_sel_index = -1;
					m_parent->OnListenerMove(4, move);
				}
				else		m_sel_index = cidx;
			}
			else			m_sel_index = cidx;
			m_parent->SelectTile(m_sel_index);
			m_state_ctrl->UpdateWnd();
			if (m_sel_index >= 0)	m_sel_group = tile->GetGroupId();
			else					m_sel_group = -1;
		}
		UpdateWnd(0xFF);
	}
}

void CShanghaiMjCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_mode == MODE_EDIT)
	{
		MJPOS pos = HitVisibleCard(point);
		CTile * tile = m_board->GetTileByPos(pos);

		if (!tile) return;
		if (tile->GetTileId() == PLACEHOLDER)
		{
			m_board->RemovePlaceHolder(pos);
			DrawLayer(m_top_layer);
			RedrawWindow();
			return;
		}

		bool br = m_board->RemoveTile(tile);
		if (!br) return;
		m_state_ctrl->UpdateWnd();
		DrawLayer(m_top_layer);
		RedrawWindow();
	}
}

void CShanghaiMjCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	m_parent->DoubleClickPlay();
}

void CShanghaiMjCtrl::ShowTileId(bool sh)
{
	//g_drawtile.m_show_tile_id = sh; 
	m_show_tile_id = sh;
	UpdateWnd(0xFF);
	m_state_ctrl->UpdateWnd();
}

void CShanghaiMjCtrl::SelectTileChar(CDrawTile::TILE_CHAR tile_char)
{
	switch (tile_char)
	{
	case CDrawTile::KANA: g_drawtile.m_tile_char = TILE_CHAR_KANA; break;
	case CDrawTile::KANJI:g_drawtile.m_tile_char = TILE_CHAR_KANJI; break;
	case CDrawTile::ABCD: g_drawtile.m_tile_char = TILE_CHAR_ABCD; break;
	}
}

BEGIN_MESSAGE_MAP(CShanghaiMjCtrl, CStatic)
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// -- Card States Control
IMPLEMENT_DYNAMIC(CCardStatesCtrl, CWnd)

#define _SCROLL

CCardStatesCtrl::CCardStatesCtrl()
	: m_board(NULL)
	,m_draw_buf(NULL),	m_group_num(0)
{

}

CCardStatesCtrl::~CCardStatesCtrl()
{
	RELEASE(m_board);
	delete m_draw_buf;
}

BOOL CCardStatesCtrl::Create(LPCTSTR lpszText, DWORD dwStyle, const RECT & rect, CWnd * pParentWnd, UINT nID)
{
//	return CWnd::Create(NULL, lpszText, dwStyle | WS_HSCROLL, rect, pParentWnd, nID);
//	return CStatic::Create(lpszText, dwStyle | WS_HSCROLL, rect, pParentWnd, nID);
	return CWnd::CreateEx(0, L"", lpszText, /*WS_HSCROLL |*/ dwStyle, rect, pParentWnd, nID);
}

void CCardStatesCtrl::Initialize(void)
{
	CRect rect;
	GetClientRect(&rect);
	m_wnd_width = rect.Width();
	m_wnd_height = rect.Height();

#ifdef _SCROLL
	m_scroll.Create(SBS_HORZ | SBS_BOTTOMALIGN | WS_CHILD | WS_VISIBLE,
		CRect(rect.left, rect.bottom - 20, rect.right, rect.bottom), this, 1100);
#else
//	EnableScrollBarCtrl(1101);
	EnableScrollBar(SB_HORZ);
	SetScrollRange(SB_HORZ, 0, 99);
#endif
//	m_scroll.SetScrollRange(0, 100);


	m_brush_card.CreateSolidBrush(RGB(255, 255, 255));

	m_font_info.CreateFont(
		20, 0, 0, 0,  // nHeight, nWidth, nEscapement, nOrientation
		FW_NORMAL, FALSE, FALSE,	// nWeight, bItalic, bUnderline
		0, ANSI_CHARSET,  // cStrikeOut, nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Arial"));                 // lpszFacename

	g_drawtile.Initialzie();
}

void CCardStatesCtrl::SetBoard(CShanghaiMjBoard * board)
{
	if (m_group_num)
	{
		m_draw_buf->Destory();
		delete m_draw_buf;
		m_draw_buf = NULL;

		m_group_num = 0;
	}
	RELEASE(m_board);

	m_board = board;	JCASSERT(m_board);
	m_board->AddRef();
	// 初始化memory dc，计算大小
	m_group_num = m_board->GetGroupNum();
	m_dc_width = m_group_num * (CARD_WIDTH + CELL_SIZE);
	m_dc_height = CARD_HEIGHT + 4 * CELL_SIZE;
	m_selected = -1;

	CRect rect1, rect2;
	this->GetWindowRect(rect1);
	this->GetClientRect(rect2);

	CPaintDC dc(this);

	m_draw_buf = new CDrawBuffer;
	m_draw_buf->Create(&dc, m_dc_width, m_dc_height);

	int range = m_dc_width - m_wnd_width;

	if (range > 0)	
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_ALL;
		si.nMax = range;
		si.nMin = 0;
		si.nPage = range / 10;
		si.nPos = 0;

#ifdef _SCROLL
		m_scroll.EnableScrollBar(ESB_ENABLE_BOTH);
		m_scroll.SetScrollRange(0, range);
#else
		SetScrollInfo(SB_HORZ, &si);
//		::SetScrollRange(this->GetSafeHwnd(), SB_HORZ, 0, range, FALSE);
//		SetScrollRange(SB_HORZ, 0, range);

#endif
	}

	DrawCardState(*m_draw_buf);
	RedrawWindow();
}

void CCardStatesCtrl::DrawCardState(CDC & dc)
{
	int x0, y0, y1;
	y0 = CELL_SIZE;
	y1 = y0 + CARD_HEIGHT + CELL_SIZE;

	if (m_draw_buf)		m_draw_buf->Clear(BKGND_CLR);

	UINT sel = 0;
	for (size_t ii = 0; ii < m_group_num; ++ii)
	{
		CTileGroup & grp = m_board->GetTileGroup(ii);
		x0 = (CARD_WIDTH + CELL_SIZE) * ii + CELL_SIZE;
		dc.SetTextColor(RGB(0, 0, 0));
		CTile * tile = m_board->GetTile(grp.m_card_idx);
		if (m_selected == ii)	sel = CDrawTile::SM_SELECTED;
		else					sel = 0;
		g_drawtile.Draw(dc, x0, y0, *tile, RGB(0, 128, 128), sel | CDrawTile::SM_STATE);
		
		CRect rect_info(x0, y1, x0 + CARD_WIDTH, y1 + CELL_SIZE);
		dc.FillRect(rect_info, &m_brush_card);
		dc.SelectObject(&m_font_info);
		dc.SetTextColor(RGB(255, 0, 0));
		CString str_info;
		str_info.Format(L"%d/%d", grp.m_tile_remain, grp.m_tile_num);
		dc.DrawText(str_info, rect_info, DT_CENTER);
	}
}

void CCardStatesCtrl::UpdateWnd(void)
{
	DrawCardState(*m_draw_buf);
	RedrawWindow();
}


void CCardStatesCtrl::OnPaint()
{
	CPaintDC dc(this);
	if (m_draw_buf)
	{
		dc.BitBlt(0, 0, m_wnd_width, m_wnd_height, 
			static_cast<CDC*>(m_draw_buf), m_offset, 0, SRCCOPY);
		//	dc.BitBlt(0, 0, m_client_rect.Width(), m_client_rect.Height(), &m_memdc_1, 0, 0, SRCAND);
	}
}

void CCardStatesCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar)
{
	int minpos;
	int maxpos;

#ifdef _SCROLL
	m_scroll.GetScrollRange(&minpos, &maxpos);
	int curpos = m_scroll.GetScrollPos();
#else
//	GetScrollRange(SB_HORZ, &minpos, &maxpos);
	pScrollBar->GetScrollRange(&minpos, &maxpos);
//	maxpos = GetScrollLimit(SB_HORZ);
	maxpos = pScrollBar->GetScrollLimit();
	// Get the current position of scroll box.
//	int curpos = GetScrollPos(SB_HORZ);
	int curpos = pScrollBar->GetScrollPos();
#endif
	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:      // Scroll to far left.
		curpos = minpos;
		break;

	case SB_RIGHT:      // Scroll to far right.
		curpos = maxpos;
		break;

	case SB_ENDSCROLL:   // End scroll.
		break;

	case SB_LINELEFT:      // Scroll left.
		if (curpos > minpos)
			curpos--;
		break;

	case SB_LINERIGHT:   // Scroll right.
		if (curpos < maxpos)
			curpos++;
		break;

	case SB_PAGELEFT:    // Scroll one page left.
	{
		// Get the page size. 
		SCROLLINFO   info;
//		GetScrollInfo(SB_HORZ, &info, SIF_ALL);
		pScrollBar->GetScrollInfo(&info, SIF_ALL);

		if (curpos > minpos)
			curpos = max(minpos, curpos - (int)info.nPage);
	}
	break;

	case SB_PAGERIGHT:      // Scroll one page right.
	{
		// Get the page size. 
		SCROLLINFO   info;
//		GetScrollInfo(SB_HORZ, &info, SIF_ALL);
		pScrollBar->GetScrollInfo(&info, SIF_ALL);

		if (curpos < maxpos)
			curpos = min(maxpos, curpos + (int)info.nPage);
	}
	break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		curpos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		curpos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
#ifdef _SCROLL
	m_scroll.SetScrollPos(curpos);
#else
//	SetScrollPos(SB_HORZ, curpos);
	pScrollBar->SetScrollPos(curpos);
#endif
	m_offset = curpos;
	RedrawWindow();
}

void CCardStatesCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	int x0 = point.x + m_offset;
	int y0 = point.y;

	if ((CELL_SIZE <= y0) && (y0 < (CELL_SIZE + CARD_HEIGHT)))
	{
		size_t cc = (size_t)x0 / (CARD_WIDTH + CELL_SIZE);
		if (cc < m_group_num)
		{
			if (m_selected == cc)	m_selected = -1;	// 取消选中
			else					m_selected = cc;
			DrawCardState(*m_draw_buf);
			RedrawWindow();
		}
	}
}

BEGIN_MESSAGE_MAP(CCardStatesCtrl, CWnd)
	ON_WM_HSCROLL()
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()
