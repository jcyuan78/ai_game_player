
// ShanghaiMjDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ShanghaiMj.h"
#include "ShanghaiMjDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

LOCAL_LOGGER_ENABLE(L"gui", LOGGER_LEVEL_DEBUGINFO);


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CShanghaiMjDlg dialog

CShanghaiMjDlg::CShanghaiMjDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SHANGHAIMJ_DIALOG, pParent)
	, m_board(NULL)
	, m_top_layer(0)
	, m_seed(123)
	, m_mode(0xFF)
	, m_show_tile_id(TRUE)
	, m_ai_player(NULL)
	, m_movable(0)
	, m_remained(0)
	, m_status(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	memset(m_modectrl, 0, sizeof(CDialog*)*MODE_NUM);
	m_modectrl[MODE_EDIT] = &m_modectrl_edit;
	m_modectrl[MODE_PLAY] = &m_modectrl_play;
	m_modectrl[MODE_AI] = &m_modectrl_ai;
}

CShanghaiMjDlg::~CShanghaiMjDlg(void)
{
	RELEASE(m_board);
	delete m_ai_player;
}

void CShanghaiMjDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BOARD, m_board_ctrl);
	DDX_Control(pDX, IDC_CARD_STATES, m_cardstates_ctrl);
	DDX_Text(pDX, IDC_SHOW_LAYER, m_top_layer);
	DDV_MinMaxInt(pDX, m_top_layer, 0, 100);
	DDX_Text(pDX, IDC_SEED, m_seed);
	DDX_Control(pDX, IDC_MODE_SELECT, m_mode_select);
	DDX_Check(pDX, IDC_SHOW_TILE_ID, m_show_tile_id);
	DDX_Control(pDX, IDC_LOGOUT, m_logout);
	DDX_Text(pDX, IDC_MOVABLE, m_movable);
	DDX_Text(pDX, IDC_REMAINED, m_remained);
	DDX_Text(pDX, IDC_STATUS, m_status);
	DDX_Control(pDX, IDC_TILE_CHAR, m_tile_char);
}

BEGIN_MESSAGE_MAP(CShanghaiMjDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_SHOW_LAYER, &CShanghaiMjDlg::OnChangeShowLayer)
	ON_BN_CLICKED(IDC_LOAD_LAYOUT, &CShanghaiMjDlg::OnLoadLayer)
	ON_BN_CLICKED(IDC_GENERATE_GAME, &CShanghaiMjDlg::OnGenerateGame)
	ON_NOTIFY(TCN_SELCHANGE, IDC_MODE_SELECT, &CShanghaiMjDlg::OnChangeModeSelect)
	ON_BN_CLICKED(IDC_SHOW_TILE_ID, &CShanghaiMjDlg::OnClickedShowTileId)
	ON_BN_CLICKED(IDC_RESET_BOARD, &CShanghaiMjDlg::OnClickedResetBoard)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CLEAR_LOG, &CShanghaiMjDlg::OnClickedClearLog)
	ON_CBN_SELCHANGE(IDC_TILE_CHAR, &CShanghaiMjDlg::OnCbnSelchangeTileChar)
END_MESSAGE_MAP()


// CShanghaiMjDlg message handlers

BOOL CShanghaiMjDlg::OnInitDialog()
{
	// tempral default value
	//m_rows = 10, m_cols = 10, m_layer_num = 3;

	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_mode_select.InsertItem(MODE_EDIT, L"Edit");
	m_mode_select.InsertItem(MODE_PLAY, L"Play");
	m_mode_select.InsertItem(MODE_AI, L"AI");
	m_mode_select.SetCurSel(MODE_EDIT);


	m_modectrl_edit.m_parent = this;
	m_modectrl_edit.Create(IDD_MODE_EDIT, this);
	m_modectrl_play.m_parent = this;
	m_modectrl_play.Create(IDD_MODE_PLAY, this);
	m_modectrl_ai.m_parent = this;
	m_modectrl_ai.Create(IDD_MODE_AIPLAY, this);


	CRect rect;
	m_mode_select.GetWindowRect(rect);		// screen 
	ScreenToClient(rect);
	for (size_t ii=0; ii<MODE_NUM; ++ii)	if (m_modectrl[ii])
	{
//		m_modectrl[ii]->m_parent = this;
		m_modectrl[ii]->ShowWindow(SW_HIDE);
		m_modectrl[ii]->EnableWindow(FALSE);
		m_modectrl[ii]->SetWindowPos(&(CWnd::wndTop), 
			rect.left + 20, rect.top + 35, 0, 0, SWP_NOSIZE);
	}
	m_modectrl_edit.ShowWindow(SW_SHOW);
	m_modectrl_edit.EnableWindow(TRUE);

	m_cardstates_ctrl.Initialize();
	m_board_ctrl.Initialize(&m_cardstates_ctrl);

	// 初始化listner
	m_board_ctrl.ShowTileId(m_show_tile_id);
	m_board_ctrl.m_parent = this;
	m_listener.m_dlg = this;

	// 调整log为等宽字体
	LOGFONT font;
	CFont * pfont = m_logout.GetFont();
	pfont->GetLogFont(&font);
	wcscpy_s(font.lfFaceName, L"Consolas");
	m_log_font.CreateFontIndirect(&font);
	m_logout.SetFont(&m_log_font);

	m_tile_char.AddString(L"かな");
	m_tile_char.AddString(L"百家姓");
	m_tile_char.AddString(L"ABCD");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CShanghaiMjDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CShanghaiMjDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CShanghaiMjDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CShanghaiMjDlg::OnNewBoard()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_modectrl_edit.UpdateData(TRUE);
	RELEASE(m_board);
	m_board = new jcvos::CDynamicInstance<CShanghaiMjBoard>;
	GlobalGame * game = GlobalGame::Instance();
	game->Clear();
	game->CreateLayout(m_modectrl_edit.m_row_num,
		m_modectrl_edit.m_col_num + 1, m_modectrl_edit.m_layer_num,
		m_modectrl_edit.m_grp_num, 4);
	game->CloneBoard(m_board);
	//m_board->InitializeTiles(m_modectrl_edit.m_grp_num, 4);
	//// 增加一個guard
	//m_board->InitializeBoard(m_modectrl_edit.m_row_num, 
	//	m_modectrl_edit.m_col_num+1, m_modectrl_edit.m_layer_num);
	m_cardstates_ctrl.SetBoard(m_board);
	m_cardstates_ctrl.RedrawWindow();

	m_board_ctrl.SetBoard(m_board);
	m_board_ctrl.RedrawWindow();
	m_top_layer = 0;
	UpdateData(FALSE);
}

void CShanghaiMjDlg::OnChangeShowLayer()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	if (m_board)
	{
		UpdateData(TRUE);
		if (m_top_layer >= m_modectrl_edit.m_layer_num)
		{
			m_top_layer = m_modectrl_edit.m_layer_num - 1;
			UpdateData(FALSE);
		}
		m_board_ctrl.SetTopLayer(m_top_layer);
	}
}


void CShanghaiMjDlg::OnSaveLayout()
{
	// TODO: Add your control notification handler code here
	JCASSERT(m_board);
	CFileDialog fdlg(FALSE, L"xml");
	if (fdlg.DoModal() != IDOK) return;

	CString fn = fdlg.GetPathName();
	m_board->SaveLayout((LPCTSTR)fn);
}

bool CShanghaiMjDlg::LoadLayout(const std::wstring & fn)
{
	bool br = true;
	RELEASE(m_board);
	m_board = new jcvos::CDynamicInstance<CShanghaiMjBoard>;
	GlobalGame * game = GlobalGame::Instance();
	br = game->LoadLayout(fn);
//	bool br = m_board->LoadLayout(fn);
	if (!br) return false;
	game->CloneBoard(m_board);

	size_t cc, rr, ll;
	m_board->GetSize(rr, cc, ll);
	m_modectrl_edit.m_col_num = cc-1;	// 減去guard
	m_modectrl_edit.m_row_num = rr;
	m_modectrl_edit.m_layer_num = ll;
	m_modectrl_edit.m_grp_num = m_board->GetGroupNum();
	m_modectrl_edit.UpdateData(FALSE);

	m_cardstates_ctrl.SetBoard(m_board);
	m_cardstates_ctrl.RedrawWindow();

	m_board_ctrl.SetBoard(m_board);
	m_board_ctrl.RedrawWindow();
	return true;
}

void CShanghaiMjDlg::OnLoadLayer()
{
	UpdateData(TRUE);
	m_modectrl_edit.UpdateData(TRUE);
	// TODO: Add your control notification handler code here
	//JCASSERT(m_board);
	CFileDialog fdlg(TRUE, L"xml");
	if (fdlg.DoModal() != IDOK) return;

	m_template_fn = fdlg.GetPathName();
	LoadLayout((LPCTSTR)m_template_fn);
}


void CShanghaiMjDlg::OnGenerateGame()
{
	UpdateData(TRUE);
	m_board->ResetLayout();
	m_board->GenerateGame(m_seed);
	m_cardstates_ctrl.UpdateWnd();
	m_board_ctrl.UpdateWnd(0xFFFF);
	m_move = CMovement();
}


void CShanghaiMjDlg::OnChangeModeSelect(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	
	UpdateData(TRUE);
	int sel = m_mode_select.GetCurSel();
	// disable all other mode ctrls
	for (size_t ii = 0; ii < MODE_NUM; ++ii)
	{
		if (m_modectrl[ii])
		{
			m_modectrl[ii]->UpdateData(TRUE);
			m_modectrl[ii]->ShowWindow(SW_HIDE);
			m_modectrl[ii]->EnableWindow(FALSE);
		}
	}
	switch (sel)
	{
	case MODE_EDIT:
		break;

	case MODE_PLAY:
		m_top_layer = m_modectrl_edit.m_layer_num-1;
		m_board_ctrl.SetTopLayer(m_top_layer);
//		m_undo_list.clear();
		break;

	case MODE_AI:
		m_top_layer = m_modectrl_edit.m_layer_num-1;
		m_board_ctrl.SetTopLayer(m_top_layer);
		m_board->EnablePeek(m_modectrl_ai.IsEnablePeek());
		LOG_DEBUG(L"ai play: depth=%d, board=%d", m_modectrl_ai.m_search_depth, m_seed);
		if (m_ai_player)	delete m_ai_player;
		m_ai_player = NULL;

		break;
	}
	if (m_modectrl[sel])
	{
		m_modectrl[sel]->ShowWindow(SW_SHOW);
		m_modectrl[sel]->EnableWindow(TRUE);
	}

	m_board_ctrl.SetPlayMode((PLAY_MODE)sel);

	*pResult = 0;
	UpdateData(FALSE);
}

void CShanghaiMjDlg::AutoPlay(void)
{
	UpdateData(TRUE);
	m_modectrl_ai.UpdateData(TRUE);
	wchar_t str[64];

	delete m_ai_player;
	m_ai_player = new CMjPlayer;
	m_ai_player->SetListener(&m_listener);

	m_board->EnablePeek(m_modectrl_ai.IsEnablePeek());
	m_ai_player->SetSearchDepth(m_modectrl_ai.m_search_depth);
	m_modectrl_ai.SelectSampling(m_ai_player);
	m_modectrl_ai.SelectEvaluation(m_ai_player);

	swprintf_s(str, L"auto play: depth = %d, board = %d", 
		m_modectrl_ai.m_search_depth, m_seed);
	AppendLog(str);
	LOG_DEBUG(str);

	// reset board
	m_move = CMovement();
	while (AiNextStep()) {};
//	int remain = m_ai_player->AutoPlay(m_board);
//	LOG_DEBUG(L"result: remain=%d", remain);

	delete m_ai_player;
	m_ai_player = NULL;
}

void CShanghaiMjDlg::Evaluate(void)
{
	UpdateData(TRUE);
	m_modectrl_ai.UpdateData(TRUE);

	if (m_template_fn.IsEmpty()) return;

	FILE * file = NULL;
	file = _wfsopen(L"test-result.csv", L"w+", _SH_DENYNO);
	if (file == NULL) return;
	fwprintf_s(file, L"depth, board_id, remain\n");

	delete m_ai_player;
	m_ai_player = NULL;

	wchar_t str[64];
	for (int dd = m_modectrl_ai.m_search_depth; dd <= 15; dd += 2)
	{
		//swprintf_s(str, L"test for depth %d\n", dd);
		//AppendLog(str);
		m_ai_player = new CMjPlayer;
		m_ai_player->SetSearchDepth(dd);
		m_ai_player->SetListener(&m_listener);
		for (int ii = 1; ii < m_seed; ++ii)
		{
			swprintf_s(str, L"play: depth = %d, board = %d", dd, ii);
			AppendLog(str);
			LOG_DEBUG(str);

			// reset board
//			if (!LoadLayout((LPCTSTR)m_template_fn)) return;
			m_board->ResetLayout();
			m_board->GenerateGame(ii);
			m_board->EnablePeek(m_modectrl_ai.IsEnablePeek());
			m_cardstates_ctrl.UpdateWnd();
			m_board_ctrl.UpdateWnd(0xFFFF);
			PassAllMessage();
			m_modectrl_ai.SelectSampling(m_ai_player);
			m_modectrl_ai.SelectEvaluation(m_ai_player);

			//m_ai_player->SetRandomPlay(m_modectrl_ai.m_random_num);

			//m_ai_player->SetBoard(m_board);
			int remain = m_ai_player->AutoPlay(m_board/*, m_modectrl_ai.m_random_num*/);
			LOG_DEBUG(L"result: remain=%d", remain);

			fwprintf_s(file, L"%d, %d, %d\n", dd, ii, remain);
			fflush(file);
		}	
		delete m_ai_player;
		m_ai_player = NULL;
	}
	fclose(file);

//	m_ai_player.SetListener(&m_listener);
}

void CShanghaiMjDlg::PlayUndo(void)
{
	wchar_t str[64];
	CMovement move = m_board->PlayUndo();
	if (move.Valid())
	{
		swprintf_s(str, L"clear move: %s", move.GetString(m_board).c_str());
		AppendLog(str);
		m_board_ctrl.UpdateWnd(0xFF);
		m_cardstates_ctrl.UpdateWnd();
		m_remained = m_board->GetRemainedNum();
		UpdateData(FALSE);
	}

}
void CShanghaiMjDlg::SelectTile(BYTE cidx)
{
	//m_move = CMovement();
}

void CShanghaiMjDlg::DoubleClickPlay(void)
{
	if (m_move.Valid())
	{
		m_board_ctrl.RemoveMarks();
		bool br = m_board->PlayCard(m_move);
		m_remained = m_board->GetRemainedNum();
		m_board_ctrl.UpdateWnd(0xFF);
		m_cardstates_ctrl.UpdateWnd();
		UpdateData(FALSE);
		PassAllMessage();

		if (!br) m_move = CMovement();
		if (br && (m_board->GetRemainedNum() == 0) )
		{
			AfxMessageBox(L"Clear!");
			delete m_ai_player;
			m_ai_player = NULL;
			return;
		}
	}
}


void CShanghaiMjDlg::OnAiNext(void)
{
	UpdateData(TRUE);
	m_modectrl_ai.UpdateData(TRUE);

	if (m_ai_player == NULL)
	{	// 初始化AI
		m_ai_player = new CMjPlayer;
		LOG_DEBUG(L"ai play: depth=%d, board=%d", m_modectrl_ai.m_search_depth, m_seed);
		m_move = CMovement();
	}
	m_board->EnablePeek(m_modectrl_ai.IsEnablePeek());

	m_ai_player->SetSearchDepth(m_modectrl_ai.m_search_depth);
	m_modectrl_ai.SelectSampling(m_ai_player);
	m_modectrl_ai.SelectEvaluation(m_ai_player);

	m_status = L"thinking ... ";
	UpdateData(FALSE);
	PassAllMessage();

	bool br = AiNextStep();
	if (!br)
	{
		delete m_ai_player;
		m_ai_player = NULL;
	}
	m_status = L"idle.";
	UpdateData(FALSE);
}

void CShanghaiMjDlg::OnClickedShowTileId()
{
	UpdateData(TRUE);
	// TODO: Add your control notification handler code here
	m_board_ctrl.ShowTileId(m_show_tile_id != FALSE);
}

void CShanghaiMjDlg::OnClickedClearLog()
{
	m_logout.SetSel(0, -1);
	m_logout.ReplaceSel(L"");
}


void CShanghaiMjDlg::AppendLog(const std::wstring & str)
{
	int input_len = str.size();

	int buf_size = m_logout.GetLimitText();
	int len = m_logout.GetWindowTextLengthW();
	while ( (len+input_len+10) > buf_size)
	{
		m_logout.SetSel(0, input_len + 10);
		m_logout.ReplaceSel(L"");
		len = m_logout.GetWindowTextLengthW();
	}

	m_logout.SetSel(len, len);
	m_logout.ReplaceSel(str.c_str());
	m_logout.SetSel(len+input_len, len+input_len);
	m_logout.ReplaceSel(L"\n");
}

bool CShanghaiMjDlg::AiNextStep(void)
{
	JCASSERT(m_ai_player);
	// 走前一步
	wchar_t str[64];
	if (m_move.Valid())
	{
		m_board_ctrl.RemoveMarks();
		bool br = m_board->PlayCard(m_move);
		m_remained = m_board->GetRemainedNum();
		m_board_ctrl.UpdateWnd(0xFF);
		m_cardstates_ctrl.UpdateWnd();
		UpdateData(FALSE);
		PassAllMessage();

		if (!br) m_move = CMovement();
		if (br && (m_board->GetRemainedNum() == 0))
		{
			AfxMessageBox(L"Clear!");
//			delete m_ai_player;
//			m_ai_player = NULL;
			return false;
		}
	}
	// 获取下一步
	int type = 0;
	bool br = m_ai_player->OneStepPlay(m_move, type, m_board);
	size_t playout = m_ai_player->GetPlayoutNum();
	if (br && m_move.Valid())
	{
		m_board_ctrl.AddMark(m_move.m_cidx[0]);
		m_board_ctrl.AddMark(m_move.m_cidx[1]);
		m_board_ctrl.UpdateWnd(0xFF);
		m_cardstates_ctrl.UpdateWnd();

		CTile * t1 = m_board->GetTile(m_move.m_cidx[0]);	JCASSERT(t1);
		CTile * t2 = m_board->GetTile(m_move.m_cidx[1]);	JCASSERT(t2);
		swprintf_s(str, L"%s move: %s p=%d\n", (type == 2) ? L"clean" : L"estim",
			m_move.GetString(m_board).c_str(), playout);
		AppendLog(str);
	}
	else
	{
		m_board_ctrl.UpdateWnd(0xFF);
		m_cardstates_ctrl.UpdateWnd();
		AfxMessageBox(L"Cannot play!");
//		delete m_ai_player;
//		m_ai_player = NULL;
		return false;
	}
	return true;
}

void CShanghaiMjDlg::OnListenerMove(int player, const CMovement & move)
{
	m_board_ctrl.RemoveMarks();
	// 显示log
	wchar_t str[64];
	CTile * t1 = m_board->GetTile(move.m_cidx[0]); JCASSERT(t1);
	CTile * t2 = m_board->GetTile(move.m_cidx[1]);	JCASSERT(t2);
	std::wstring str_player;
	switch (player)
	{
	case IPlayerListener::PLAY_CLEAN:	str_player = L"clean"; break;
	case IPlayerListener::PLAY_ESTI:	str_player = L"estim"; break;
	case IPlayerListener::PLAY_USER:	str_player = L"user "; break;

	}
	swprintf_s(str, L"%s move: %s, %s", str_player.c_str(),
		t1->GetString().c_str(), t2->GetString().c_str());
	LOG_DEBUG(str);
	AppendLog(str);
//	UpdateData(FALSE);
	m_board_ctrl.AddMark(move.m_cidx[0]);
	m_board_ctrl.AddMark(move.m_cidx[1]);
	m_board_ctrl.UpdateWnd(0xFF);
	m_cardstates_ctrl.UpdateWnd();
	m_remained = m_board->GetRemainedNum();

	if (player == IPlayerListener::PLAY_USER)
	{
		if (m_ai_player == NULL)	
		{	// 初始化AI
			m_ai_player = new CMjPlayer;
		}

		m_move = m_ai_player->FindCleanPlay(static_cast<const CBoardBase*>(m_board));
		if (m_move.Valid())
		{
			m_board_ctrl.AddMark(m_move.m_cidx[0]);
			m_board_ctrl.AddMark(m_move.m_cidx[1]);
			m_board_ctrl.UpdateWnd(0xFF);
			m_cardstates_ctrl.UpdateWnd();
			swprintf_s(str, L"clear move: %s", m_move.GetString(m_board).c_str());
			AppendLog(str);
		}
	}
	UpdateData(FALSE);
	// 驱动消息循环更新画面
	PassAllMessage();
}

void CShanghaiMjDlg::OnListenerResult(int ir)
{
	//wchar_t str[64];
	if (ir == IPlayerListener::RES_CLEANED)		AppendLog(L"cleaned");
	else AppendLog(L"failed");
	PassAllMessage();
}

void CShanghaiMjDlg::PassAllMessage(void)
{	// 用于在前台运行时驱动消息循环，更新画面
	///////////////////////////////////////////
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);      /* Translates virtual key codes.  */
		DispatchMessage(&msg);       /* Dispatches message to window.  */
	}
	//////////////Simon///////////////////////
}


///////////////////////////////////////////////////////////////////////////////
// -- listener
void CSmjListener::OnMove(int play, const CMovement & move)
{
	JCASSERT(m_dlg);
	m_dlg->OnListenerMove(play, move);

}

void CSmjListener::OnResult(int result)
{
	JCASSERT(m_dlg);
	m_dlg->OnListenerResult(result);

}


void CShanghaiMjDlg::OnClickedResetBoard()
{
	// TODO: Add your control notification handler code here
	m_board->ResetLayout();
	m_cardstates_ctrl.SetBoard(m_board);
	m_cardstates_ctrl.RedrawWindow();

	m_board_ctrl.SetBoard(m_board);
	m_board_ctrl.RedrawWindow();
	m_move = CMovement();
}


void CShanghaiMjDlg::OnClose()
{
	GlobalGame::Instance()->Clear();
	CDialog::OnClose();
}




void CShanghaiMjDlg::OnCbnSelchangeTileChar()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(FALSE);
	m_board_ctrl.SelectTileChar((CDrawTile::TILE_CHAR)m_tile_char.GetCurSel());
	m_board_ctrl.RedrawWindow();
	m_cardstates_ctrl.RedrawWindow();
}
