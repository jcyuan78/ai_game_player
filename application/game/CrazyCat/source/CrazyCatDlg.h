// CrazyCatDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#include "ChessBoard.h"
#include "background_search.h"
#include "afxcmn.h"

#include "test_hook.h"

class CTestHook;

#define TIMER_ID_PROG		100
#define TIMER_INT_PROG		1000

// CCrazyCatDlg 对话框
class CCrazyCatDlg : public CDialog
	, public IRefereeListen
{
// 构造
public:
	CCrazyCatDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CCrazyCatDlg(void);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CRAZYCAT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	enum PLAY_STATUS
	{	
		PS_SETTING = 0,	PS_PLAYING = 0x10, PS_WIN = 0x20,
	};

	enum AI_TYPE
	{
		AI_HUMEN = 0, AI_NO_SORT = 1, AI_SORT = 2,
	};


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnCellRadiusChanged(void);
	afx_msg void OnClickedPlay();
	afx_msg void OnClickedSearch();
	afx_msg void OnClickedShowPath();
	afx_msg LRESULT OnChessClicked(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnCompleteMove(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnRobotMove(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnMoveChess(WPARAM wp, LPARAM lp);
	afx_msg void OnClickedStop();
	afx_msg void OnClickedReset();
	afx_msg void OnClickUndo();
	afx_msg void OnClickedSave();
	afx_msg void OnClickedLoad();
	afx_msg void OnClickedRedo();
	afx_msg void OnClickedMakeGame();
	afx_msg void OnClickedClearSeed();
	afx_msg void OnTimer(UINT_PTR id);

	// for debug only
	afx_msg void OnClidkMakeRndTab();
	afx_msg void OnStartTest();


public:
	bool m_init;

protected:
	//virtual void SendTextMsg(const CJCStringT & txt);
	void SendTextMsg(LPCTSTR fmt, ...);
	virtual void SearchCompleted(MOVEMENT * move);
	void LogBoard(const CChessBoard * board);

protected:
	// 棋盘及AI
	CChessBoard * m_board;
	int m_search_depth;
	ISearchWrapper	* m_search[3];
	int m_move_count;
	CCrazyCatEvaluator	m_eval;
	// 游戏计数器，用于LOG输出
	int m_index;

	//界面变量
	CButton m_btn_play;
	CButton m_btn_stop;
	CButton m_ctrl_turn;
	CEdit m_message_wnd;
	CChessBoardUi m_board_ctrl;
	CString m_txt_location;		// 鼠标位置
	int m_player[3];			// 各角色的玩家类型，TRUE：AI，FALSE：用户
	CComboBox m_ctrl_player_catcher;
	CButton m_btn_makegame;
	// 随机产生棋局是的随机数种子，棋子个数
	int m_seed, m_preset_chess;
	// 棋子间隔，棋子大小
	int	m_cell_size, m_cell_radius;

	BOOL m_show_path, m_show_search;
	CString m_checksum;		// 棋局的HASH
	PLAYER m_turn;			// 轮流标志
	PLAY_STATUS	m_status;
	
	// timer for updating progress
	UINT_PTR	m_prog_timer;

	CTestHook * m_test_hook;

public:
	void InstallTestHook(CTestHook * hook)	{m_test_hook = hook;}
	CProgressCtrl m_ai_progress;
	CButton m_btn_reset;
	CButton m_btn_load;
	CButton m_btn_save;
	CButton m_btn_redo;
	CButton m_btn_undo;

	PLAY_INFO m_play_info;



protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};
