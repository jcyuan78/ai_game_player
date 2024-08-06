#pragma once

#include "afxwin.h"
#include "afxcmn.h"
#include <lib_shmj.h>


class CShanghaiMjDlg;

class CModeCtrlAiPlayer;

class CModeCtrlEdit : public CDialog
{
public:
	CModeCtrlEdit(CWnd * parent = NULL);
	virtual ~CModeCtrlEdit(void);

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MODE_EDIT };
#endif

public:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();

public:
	CShanghaiMjDlg * m_parent;
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedNewBoard();
	afx_msg void OnClickedSavelayout();

public:
	int m_layer_num;
	int m_col_num;
	int m_row_num;
	int m_grp_num;
};

class CModeCtrlAiPlayer : public CDialog
{
public:
	CModeCtrlAiPlayer(CWnd * pParent = NULL);
	virtual ~CModeCtrlAiPlayer(void);

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MODE_AIPLAY };
#endif

public:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

public:
	CShanghaiMjDlg * m_parent;
														// Implementation
protected:
	// Generated message map functions
	DECLARE_MESSAGE_MAP()
//	HICON m_hIcon;
	afx_msg void OnClickedAutoPlay();
	afx_msg void OnClickedNext();
	afx_msg void OnClickedEvaluate();
	afx_msg void OnAiUndo();

public:
	bool SelectSampling(CMjPlayer * player);
	bool SelectEvaluation(CMjPlayer * player);
	bool IsEnablePeek(void);

public:
	int m_search_depth;
	//BOOL m_enable_peek;
	// 对于next操作，仅标出AI的选项，但不实际移动
	BOOL m_mark_only;
	// 蒙特卡洛算法测试次数
	int m_evaluation_param;
	//int m_ai_playcost;

protected:
	CComboBox m_sampling_algorithm;
	CComboBox m_evaluation_algorithm;
	int m_sampling_param;
};

class CModeCtrlPlay : public CDialog
{
public:
	CModeCtrlPlay(CWnd * pParent = NULL);
	virtual ~CModeCtrlPlay(void);

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MODE_PLAY };
#endif

public:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
														//virtual BOOL Create(LPCTSTR lpszText, DWORD dwStyle,
														//	const RECT& rect, CWnd* pParentWnd, UINT nID = 0xffff);
	virtual BOOL OnInitDialog();

public:
	CShanghaiMjDlg * m_parent;
	// Implementation
protected:
	//	HICON m_hIcon;

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
public:
	int m_search_depth;
	BOOL m_enable_peek;
	//BOOL m_random_play;
	//int m_random_num;
	afx_msg void OnPlaySuggest();
	afx_msg void OnPlayUndo();
};
