
// mini-image-processorDlg.cpp: 实现文件
//

#include "pch.h"
#include "../framework.h"
#include "mini-image-processor.h"
#include "mini-image-processor-dlg.h"
#include "afxdialogex.h"

LOCAL_LOGGER_ENABLE(L"dialog", LOGGER_LEVEL_NOTICE);


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CMiniImageProcessorDlg 对话框



CMiniImageProcessorDlg::CMiniImageProcessorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MINIIMAGEPROCESSOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	memset(m_image_index, 0, sizeof(int) * 2);
}

void CMiniImageProcessorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONFIG_FILE, m_config_edit);
	DDX_Control(pDX, IDC_SRC_IMG, m_image_ctrls[0]);
	DDX_Control(pDX, IDC_DST_IMG, m_image_ctrls[1]);
	DDX_Text(pDX, IDC_SRC_PROPERTY, m_properties[0]);
	DDX_Text(pDX, IDC_DST_PROPERTY, m_properties[1]);
	DDX_Text(pDX, IDC_SRC_INDEX, m_image_index[0]);
	DDX_Text(pDX, IDC_DST_INDEX, m_image_index[1]);
}

BEGIN_MESSAGE_MAP(CMiniImageProcessorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_LOAD_CONFIG, &CMiniImageProcessorDlg::OnClickedLoadConfig)
	ON_BN_CLICKED(IDC_OPEN_SOURCE, &CMiniImageProcessorDlg::OnClickedOpenSource)
	ON_BN_CLICKED(IDC_SRC_SELECT_PRE, &CMiniImageProcessorDlg::OnClickedSrcSelectPre)
	ON_BN_CLICKED(IDC_SRC_SELECT_NEXT, &CMiniImageProcessorDlg::OnClickedSrcSelectNext)
	ON_BN_CLICKED(IDC_DST_SELECT_PRE, &CMiniImageProcessorDlg::OnClickedDstSelectPre)
	ON_BN_CLICKED(IDC_DST_SELECT_NEXT, &CMiniImageProcessorDlg::OnClickedDstSelectNext)
	ON_BN_CLICKED(IDC_REFRESH, &CMiniImageProcessorDlg::OnClickedRefresh)
	ON_EN_CHANGE(IDC_DST_INDEX, &CMiniImageProcessorDlg::OnChangeDstIndex)
//	ON_EN_UPDATE(IDC_DST_INDEX, &CMiniImageProcessorDlg::OnUpdateDstIndex)
ON_EN_CHANGE(IDC_SRC_INDEX, &CMiniImageProcessorDlg::OnChangeSrcIndex)
ON_BN_CLICKED(IDC_SRC_SAVE_IMG, &CMiniImageProcessorDlg::OnClickedSrcSaveImg)
ON_BN_CLICKED(IDC_DST_SAVE_IMG, &CMiniImageProcessorDlg::OnClickedDstSaveImg)
ON_BN_CLICKED(IDC_SAVE_CONFIG, &CMiniImageProcessorDlg::OnClickedSaveConfig)
END_MESSAGE_MAP()


// CMiniImageProcessorDlg 消息处理程序

BOOL CMiniImageProcessorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	for (int ii = 0; ii < 2; ++ii)
	{
		UINT id = (ii == 0) ? IDC_SRC_PARAMETER_AREA : IDC_DST_PARAMETER_AREA;
		CWnd * wnd = GetDlgItem(id);
//		CRect client_rect;
//		wnd->GetClientRect(client_rect);
		m_image_ctrls[ii].SetParamArea(wnd);
		m_image_ctrls[ii].m_ctrl_start_id = 5000 + ii * 1000;
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMiniImageProcessorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMiniImageProcessorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMiniImageProcessorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMiniImageProcessorDlg::OnClickedLoadConfig()
{
	CFileDialog dlg(TRUE, L"xml", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		L"Config File(*.xml)|*.xml|All files (*.*)|*.*||");
	if (dlg.DoModal() != IDOK) return;
	CString fn = dlg.GetPathName();

	try
	{
		m_boxes.LoadFromFile((const wchar_t*)(fn));
	}
	catch (std::exception & exp)
	{
		std::wstring str_err;
		jcvos::Utf8ToUnicode(str_err, exp.what());
		MessageBox(str_err.c_str(), L"failed on loading config");
	}
	
	int box_num = m_boxes.GetBoxNum();
	m_box_id[0] = 0;
	m_box_id[1] = box_num - 1;
	SetBoxToCtrl(0);
	SetBoxToCtrl(1);
}

void CMiniImageProcessorDlg::OnClickedSaveConfig()
{
	CFileDialog dlg(FALSE, L"xml", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"Config File(*.xml)|*.xml|All files (*.*)|*.*||");
	if (dlg.DoModal() != IDOK) return;
	CString fn = dlg.GetPathName();
//	fn += L".xml";
	m_boxes.SaveToFile((const wchar_t*)fn);
}


void CMiniImageProcessorDlg::OnClickedOpenSource()
{
	jcvos::auto_interface<IProcessorBox> src_box;
	m_boxes.GetSourceProcessor(src_box);
	if (src_box == NULL)
	{
		LOG_ERROR(L"there is no source processor box");
		return;
	}
	CFileDialog dlg(TRUE, L"jpeg", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		L"image file |*.jpg;*.png|All files|*.*||");
	if (dlg.DoModal() != IDOK) return;
	CString fn = dlg.GetPathName();
	src_box->SetParameter(L"file_name", (const wchar_t *)(fn));
	m_boxes.Update(0);
	m_box_id[0] = 0;
	UpdateImage(0);
	UpdateImage(1);
}


void CMiniImageProcessorDlg::OnClickedSrcSelectPre()
{
	if (m_box_id[0] > 0)
	{
		m_box_id[0]--;
		SetBoxToCtrl(0);
		UpdateImage(0);
	}
}


void CMiniImageProcessorDlg::OnClickedSrcSelectNext()
{
	if (m_box_id[0] < m_boxes.GetBoxNum() - 1)
	{
		m_box_id[0]++;
		SetBoxToCtrl(0);
		UpdateImage(0);
	}
}


void CMiniImageProcessorDlg::OnClickedDstSelectPre()
{
	if (m_box_id[1] > 0)
	{
		m_box_id[1]--;
		SetBoxToCtrl(1);
		UpdateImage(1);
	}
}


void CMiniImageProcessorDlg::OnClickedDstSelectNext()
{
	if (m_box_id[1] < m_boxes.GetBoxNum()-1)
	{
		m_box_id[1]++;
		SetBoxToCtrl(1);
		UpdateImage(1);
	}
}

void CMiniImageProcessorDlg::OnClickedRefresh()
{
	m_image_ctrls[0].UpdateParam();
	m_image_ctrls[1].UpdateParam();
	m_boxes.Update(0);
	UpdateImage(0);
	UpdateImage(1);
}



