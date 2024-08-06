#pragma once

#include <lib_shmj.h>

#include "puzzle_ctrl.h"
#include <set>

class CShanghaiMjDlg;

class CDrawBuffer : public CDC
{
public:
	CDrawBuffer(void);
	virtual ~CDrawBuffer(void);
public:
	void Create(CDC * dc, size_t w, size_t h);
	void Destory(void);
	void CopyTransporent(CDrawBuffer & buf, size_t offset_x, size_t offset_y, UINT backgnd);
	void Clear(UINT color);
	void GetSize(size_t & w, size_t & h) { w = m_width, h = m_height; };

protected:
	CBitmap m_bmp;
//	CRect   m_rect;
	size_t m_width, m_height;
};

class CCardStatesCtrl;

class CDrawTile
{
public:
	enum SHOW_MODE {SM_NORMAL = 0, SM_SELECTED = 1, SM_STATE = 2, SM_SHOW_ID = 4};
	enum TILE_CHAR {KANA=0, KANJI=1, ABCD=2,};
	void Initialzie(void);
	void Draw(CDC & dc, int xx, int yy, CTile & card, UINT color, UINT mode);

public:
	//bool m_show_tile_id;
	CBrush	m_brush_card;
	CFont	m_font_group;	// ���������
	CFont	m_font_tile;	// �Ʊ�ŵ�����
	const wchar_t * m_tile_char;
};

///////////////////////////////////////////////////////////////////////////////
// -- Board Control
//	�������̵���ʾ
class CShanghaiMjCtrl : public CStatic
{
	DECLARE_DYNAMIC(CShanghaiMjCtrl)
public:
	CShanghaiMjCtrl();
	virtual ~CShanghaiMjCtrl();

public:
	virtual void SetBoard(IPuzzleBoard * board);
	virtual void SetPlayMode(PLAY_MODE mode);
	
	void Initialize(CCardStatesCtrl * ctrl);
	void SetViewLayers(UINT mask)
	{
		m_layer_mask = mask;
		RedrawWindow();
	}
	void SetTopLayer(size_t layer)
	{
		JCASSERT(layer < m_layers);
		m_top_layer = layer;
		RedrawWindow();
	}
	void DrawLayer(size_t layer);
	// ���¶�㣬��bitmap��ʾҪ���µĲ㡣
	//void DrawLayers(UINT layers);
	void DrawGrid(void);

	bool HitTest(const CPoint & p, size_t &col, size_t &row);
	MJPOS HitVisibleCard(const CPoint & p);
	void PutSelectCard(size_t col, size_t row);

	void UpdateWnd(UINT layermask);

	void AddMark(BYTE cidx);
	void RemoveMarks(void) { m_marks.clear(); }
	void ShowTileId(bool sh);
	void SelectTileChar(CDrawTile::TILE_CHAR tile_char);

protected:
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point) {};
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

protected:
	void CleanMemDc(void);

public:
	CShanghaiMjDlg		* m_parent;

protected:
	CShanghaiMjBoard	* m_board;
	CCardStatesCtrl		* m_state_ctrl;
	PLAY_MODE m_mode;

	size_t m_rows, m_cols, m_layers;

	CDrawBuffer * m_layer;
	CDrawBuffer * m_grid;
	CDrawBuffer * m_mark_layer;
	CDrawBuffer m_client;
//	CFont	m_font_card;

	int	m_dc_width, m_dc_height;
	int m_wnd_width, m_wnd_height;
	std::set<int> m_marks;
	bool m_highlight;

	UINT m_layer_mask;
	// ��������Կ����Ĳ�
	size_t	m_top_layer;

	// ѡ������play mode����
//	CTile * m_selected;
	int m_sel_index;	// m_selected��index
	int m_sel_group;	// ��ѡ���Ƶ�group id��high light������ͬgroup����
	bool m_show_tile_id;
};

///////////////////////////////////////////////////////////////////////////////
// -- Card States Control
//	������״̬����ʾ����ʾ�����Ƶ������ͻ�δȡ�ߵ�����

class CCardStatesCtrl : public CWnd
{
	DECLARE_DYNAMIC(CCardStatesCtrl)
public:
	CCardStatesCtrl();
	virtual ~CCardStatesCtrl();

public:
	virtual BOOL Create(LPCTSTR lpszText, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID = 0xffff);

	void Initialize(void);
	virtual void SetBoard(CShanghaiMjBoard * board);
	void DrawCardState(CDC & dc);
	int GetSelectedGroup(void) { return m_selected; }
	virtual void UpdateWnd(void);

protected:
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point) {};
	afx_msg void OnMouseMove(UINT nFlags, CPoint point) {};
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	DECLARE_MESSAGE_MAP()

protected:
	CDrawBuffer * m_draw_buf;
	CShanghaiMjBoard * m_board;

	int m_wnd_width, m_wnd_height;
	size_t m_group_num;
	int m_dc_width, m_dc_height;
	// ˮƽ���������ƶ�λ��
	int m_offset;

	CBrush	m_brush_card;
//	CFont	m_font_card;
	CFont	m_font_info;

	CScrollBar m_scroll;

	int m_selected;
};
