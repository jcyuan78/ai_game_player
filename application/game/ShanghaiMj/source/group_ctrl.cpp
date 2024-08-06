#include "stdafx.h"
#include "../resource.h"
#include "group_ctrl.h"
#include "afxdialogex.h"
#include "ShanghaiMjDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

LOCAL_LOGGER_ENABLE(_T("group"), LOGGER_LEVEL_DEBUGINFO);


/*
BEGIN_MESSAGE_MAP(CTabSelect, CStatic)

END_MESSAGE_MAP()


IMPLEMENT_DYNAMIC(CTabSelect, CStatic)

CTabSelect::CTabSelect()
	: m_sub(NULL)
{
	
}

CTabSelect::~CTabSelect()
{

}

LRESULT CTabSelect::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_sub) m_sub->WindowProc(message, wParam, lParam);
	LRESULT ir = CStatic::WindowProc(message, wParam, lParam);
	return ir;
}
*/

///////////////////////////////////////////////////////////////////////////////
// -- CModeCtrlEdit
CModeCtrlEdit::CModeCtrlEdit(CWnd * parent)
	: CDialog(IDD_MODE_EDIT, parent)
	, m_parent(NULL)
	, m_layer_num(0)
	, m_col_num(0)
	, m_row_num(0)
	, m_grp_num(1)
{
}

CModeCtrlEdit::~CModeCtrlEdit(void)
{
}


BEGIN_MESSAGE_MAP(CModeCtrlEdit, CDialog)
	ON_BN_CLICKED(IDC_EDIT_NEWBOARD, &CModeCtrlEdit::OnClickedNewBoard)
	ON_BN_CLICKED(IDC_EDIT_SAVELAYOUT, &CModeCtrlEdit::OnClickedSavelayout)
END_MESSAGE_MAP()

void CModeCtrlEdit::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_LAYERNUM, m_layer_num);
	DDV_MinMaxInt(pDX, m_layer_num, 0, 255);
	DDX_Text(pDX, IDC_EDIT_COLNUM, m_col_num);
	DDV_MinMaxInt(pDX, m_col_num, 0, 255);
	DDX_Text(pDX, IDC_EDIT_ROWNUM, m_row_num);
	DDV_MinMaxInt(pDX, m_row_num, 0, 255);
	DDX_Text(pDX, IDC_EDIT_GRUNUM, m_grp_num);
	DDV_MinMaxInt(pDX, m_grp_num, 1, 64);
}

BOOL CModeCtrlEdit::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;
}

void CModeCtrlEdit::OnClickedNewBoard()
{
	JCASSERT(m_parent);
	m_parent->OnNewBoard();
}

void CModeCtrlEdit::OnClickedSavelayout()
{
	// TODO: Add your control notification handler code here
	JCASSERT(m_parent);
	m_parent->OnSaveLayout();
}




///////////////////////////////////////////////////////////////////////////////
// -- AI
CModeCtrlAiPlayer::CModeCtrlAiPlayer(CWnd * parent)
	: CDialog(IDD_MODE_AIPLAY, parent)
	, m_parent(NULL)
	, m_search_depth(3)
	//, m_enable_peek(FALSE)
	//, m_random_play(FALSE)
	, m_sampling_param(10)
	, m_mark_only(FALSE)
	, m_evaluation_param(10)
	//, m_ai_playcost(1000)
{

}

CModeCtrlAiPlayer::~CModeCtrlAiPlayer(void)
{

}

BEGIN_MESSAGE_MAP(CModeCtrlAiPlayer, CDialog)
	ON_BN_CLICKED(IDC_AI_AUTOTPLAY, &CModeCtrlAiPlayer::OnClickedAutoPlay)
	ON_BN_CLICKED(IDC_AI_NEXT, &CModeCtrlAiPlayer::OnClickedNext)
	ON_BN_CLICKED(IDC_AI_EVALUATE, &CModeCtrlAiPlayer::OnClickedEvaluate)
	ON_BN_CLICKED(IDC_AI_UNDO, &CModeCtrlAiPlayer::OnAiUndo)
END_MESSAGE_MAP()

void CModeCtrlAiPlayer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_AI_SEARCHDEPTH, m_search_depth);
	DDV_MinMaxInt(pDX, m_search_depth, 1, 255);
	//DDX_Check(pDX, IDC_AI_ENABLEPEEK, m_enable_peek);
	//DDX_Check(pDX, IDC_AI_MONTECARLO, m_random_play);
	DDX_Text(pDX, IDC_AI_RANDOM_NUM, m_sampling_param);
	DDX_Check(pDX, IDC_AI_MARKONLY, m_mark_only);
	DDX_Text(pDX, IDC_AI_MONTE_CYCLE, m_evaluation_param);
	//DDX_Text(pDX, IDC_AI_COST, m_ai_playcost);
	DDX_Control(pDX, IDC_SAMPLING_ALGORITHM, m_sampling_algorithm);
	DDX_Control(pDX, IDC_EVALUATION_ALGORITHM, m_evaluation_algorithm);
}

static const wchar_t* ALGORITHM[] = {
	L"eval func", L"motecarlo", L"uct",
	L"normal", L"ucb", L"peeking" };


BOOL CModeCtrlAiPlayer::OnInitDialog()
{
	LOG_STACK_TRACE();
	CDialog::OnInitDialog();

	m_sampling_algorithm.AddString(ALGORITHM[3]);
	m_sampling_algorithm.AddString(ALGORITHM[4]);
	m_sampling_algorithm.AddString(ALGORITHM[5]);
	m_sampling_algorithm.SetCurSel(1);

	m_evaluation_algorithm.AddString(ALGORITHM[0]);
	m_evaluation_algorithm.AddString(ALGORITHM[1]);
	m_evaluation_algorithm.AddString(ALGORITHM[2]);
	m_evaluation_algorithm.SetCurSel(1);

	return TRUE;
}

LRESULT CModeCtrlAiPlayer::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ir = CDialog::WindowProc(message, wParam, lParam);
	return ir;
}

void CModeCtrlAiPlayer::OnClickedAutoPlay()
{
	LOG_STACK_TRACE();
	// TODO: Add your control notification handler code here
	JCASSERT(m_parent);
	m_parent->AutoPlay();

}

void CModeCtrlAiPlayer::OnClickedNext()
{
	LOG_STACK_TRACE();
	// TODO: Add your control notification handler code here
	JCASSERT(m_parent);
	m_parent->OnAiNext();
}

void CModeCtrlAiPlayer::OnClickedEvaluate()
{
	// TODO: Add your control notification handler code here
	JCASSERT(m_parent);
	m_parent->Evaluate();
}



///////////////////////////////////////////////////////////////////////////////
// -- Play
CModeCtrlPlay::CModeCtrlPlay(CWnd * parent)
	: CDialog(IDD_MODE_AIPLAY, parent)
	, m_parent(NULL)
	, m_search_depth(3)
	, m_enable_peek(FALSE)
	//, m_random_play(FALSE)
	//, m_random_num(0)
{

}

CModeCtrlPlay::~CModeCtrlPlay(void)
{

}

BEGIN_MESSAGE_MAP(CModeCtrlPlay, CDialog)
	ON_BN_CLICKED(IDC_PLAY_NEXT, &CModeCtrlPlay::OnPlaySuggest)
	ON_BN_CLICKED(IDC_PLAY_UNDO, &CModeCtrlPlay::OnPlayUndo)
END_MESSAGE_MAP()

void CModeCtrlPlay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CModeCtrlPlay::OnInitDialog()
{
	LOG_STACK_TRACE();
	CDialog::OnInitDialog();
	return TRUE;
}



void CModeCtrlPlay::OnPlaySuggest()
{	// TODO: Add your control notification handler code here
	JCASSERT(m_parent);
	//m_parent->OnPlaySuggest();
}


void CModeCtrlPlay::OnPlayUndo()
{	// TODO: Add your control notification handler code here
	JCASSERT(m_parent);
	m_parent->PlayUndo();
}


void CModeCtrlAiPlayer::OnAiUndo()
{
	// TODO: Add your control notification handler code here
	JCASSERT(m_parent);
	m_parent->PlayUndo();
}

bool CModeCtrlAiPlayer::SelectSampling(CMjPlayer * player)
{
	UpdateData(TRUE);
	CString str;
	m_sampling_algorithm.GetWindowTextW(str);
	for (int ii = 0; ii < 3; ++ii)
	{
		if (str == ALGORITHM[ii + CMjPlayer::EVS_SAMPLING_NORMAL])
		{
			player->SelectSampling(ii + CMjPlayer::EVS_SAMPLING_NORMAL, m_sampling_param);
			return true;
		}
	}
	JCASSERT(0);
	return false;
}

bool CModeCtrlAiPlayer::SelectEvaluation(CMjPlayer * player)
{
	UpdateData(TRUE);
	CString str;
	m_evaluation_algorithm.GetWindowTextW(str);
	for (int ii = 0; ii < 3; ++ii)
	{
		if (str == ALGORITHM[ii])
		{
			player->SelectEvaluater(ii, m_evaluation_param);
			return true;
		}
	}
	JCASSERT(0);
	return false;
}

bool CModeCtrlAiPlayer::IsEnablePeek(void)
{
	CString str;
	m_sampling_algorithm.GetWindowTextW(str);
	return str == ALGORITHM[CMjPlayer::EVS_PEEKING];
}
