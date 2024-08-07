
// ShanghaiMjDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "group_ctrl.h"

#include "../../comm_files/ipuzzle.h"
#include "../include/smj_ctrl.h"
#include "afxcmn.h"

#define MODE_NUM	4

class CShanghaiMjDlg;

class CSmjListener : public IPlayerListener
{
public:
	CSmjListener(void) { m_dlg = NULL; }
public:
	virtual void OnMove(int play, const CMovement & move);
	virtual void OnResult(int result);
public:
	CShanghaiMjDlg * m_dlg;
};



// CShanghaiMjDlg dialog
class CShanghaiMjDlg : public CDialog
{
// Construction
public:
	CShanghaiMjDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CShanghaiMjDlg(void);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHANGHAIMJ_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnChangeShowLayer();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLoadLayer();
	afx_msg void OnGenerateGame();
	afx_msg void OnChangeModeSelect(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClickedShowTileId();
	afx_msg void OnClickedResetBoard();
	//afx_msg void OnBnClickedAiNext();
	DECLARE_MESSAGE_MAP()

	// control variables
	int m_seed;
	int m_top_layer;

	// 用于在前台运行时驱动消息循环，更新画面
	void PassAllMessage(void);

public:
	void DoubleClickPlay(void);
	void AutoPlay(void);
	void OnAiNext(void);
	void OnNewBoard();
	void OnSaveLayout();
	void Evaluate();
	void PlayUndo(void);
	void SelectTile(BYTE cidx);
	// Ai走下一步，返回true: 可继续走; 返回false: 无牌可走，清空或者死局
	bool AiNextStep(void);
	void OnListenerMove(int play, const CMovement & move);
	void OnListenerResult(int ir);

	bool LoadLayout(const std::wstring & fn);
	void AppendLog(const std::wstring & str);

protected:
	CShanghaiMjBoard * m_board;
	CShanghaiMjCtrl m_board_ctrl;
	CCardStatesCtrl m_cardstates_ctrl;
	CMjPlayer		* m_ai_player;
	CMovement		m_move;

protected:
	typedef std::vector<UINT> CTRL_GROUP;
	CTRL_GROUP		m_ctrl_grp[MODE_NUM];
	CPoint			m_grp_org[MODE_NUM];
	UINT			m_mode;
	CSmjListener	m_listener;

	CString					m_template_fn;

public:
	CTabCtrl m_mode_select;
	CDialog * m_modectrl[MODE_NUM];
	CModeCtrlEdit		m_modectrl_edit;
	CModeCtrlPlay		m_modectrl_play;
	CModeCtrlAiPlayer	m_modectrl_ai;

public:
	BOOL m_show_tile_id;
	CEdit m_logout;
	CFont m_log_font;
	int m_movable;
	int m_remained;
	CString m_status;
	afx_msg void OnClose();
	afx_msg void OnClickedClearLog();
	CComboBox m_tile_char;
	afx_msg void OnCbnSelchangeTileChar();
};
