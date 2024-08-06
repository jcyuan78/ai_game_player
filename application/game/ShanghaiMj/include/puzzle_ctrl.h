#pragma once

class CPuzzleCtrl : public CStatic
{
	DECLARE_DYNAMIC(CPuzzleCtrl)
public:
	CPuzzleCtrl(void);
	virtual ~CPuzzleCtrl(void);

	enum OPTION
	{
		HSCROLL = 1,
		VSCROLL = 2,
	};

// interface
public:
//	virtual void Initialize(void);

// support functions
public:
	void InitializeCtrl(int scroll);
	void CreateMemDc(int w, int h);


	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point) {};
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point) {};
	afx_msg void OnMouseMove(UINT nFlags, CPoint point) {};
	DECLARE_MESSAGE_MAP()


protected:
	// 绘图缓存
	CDC				* m_memdc;
	CBitmap			* m_membmp;
	// 缓存大小，
	int m_dc_width, m_dc_height;
	// 窗口大小
	int m_wnd_width, m_wnd_height;

	// scrolls
	int	m_scroll_support;
	CScrollBar m_hscroll, m_vscroll;
	size_t m_card_num;
	// 水平滚动条的移动位置
	int m_hoffset, m_voffset;


};
