#include "pch.h"
#include "processor_ctrl.h"

LOCAL_LOGGER_ENABLE(L"image_ctrl", LOGGER_LEVEL_DEBUGINFO);


CProcessorCtrl::CProcessorCtrl(void)
	:m_box(NULL), m_image_index(0), edit(NULL), m_processer(NULL)
{
}

CProcessorCtrl::~CProcessorCtrl(void)
{
	RELEASE(m_processer);
	RELEASE(m_box);
}

void CProcessorCtrl::SetProcessorBox(IProcessorBox * box)
{
	Reset();
	m_box = box;
	if (!m_box) return;
	m_box->AddRef();
	m_box->GetProcessor(m_processer);
	if (!m_processer) return;
	m_processer->AddRef();

	// 添加参数调节控件
	boost::property_tree::wptree pt;
	box->GetParameters(pt);
	size_t param_num = box->GetParamNumber();
	for (size_t ii = 0; ii < param_num; ++ii)
	{
		const PARAMETER_DEFINE * param = box->GetParamInfo(ii);
		CParamCtrl ctrl;
		ctrl.m_ctrl = NULL;
		ctrl.m_label = NULL;

		UINT ctrl_id = m_ctrl_list.size();
		ctrl.m_ctrl_id = m_ctrl_start_id + ctrl_id;
		int row = ctrl_id / 2;
		int col = ctrl_id & 1;

		ctrl.m_name = param->name;
		CRect rect;
		rect.left = col * 300 + 10;
		rect.top = row * 40 + 10;
		// 100: 标签的宽度
		rect.right = rect.left + 150;
		rect.bottom = rect.top + 20;

		ctrl.m_label = new CStatic;
		ctrl.m_label->Create(ctrl.m_name.c_str(), WS_CHILD | WS_VISIBLE, rect, m_param_area);
		ctrl.m_label->SetWindowText(ctrl.m_name.c_str());

		// 10: 余量
		rect.left = rect.right+10;
		// 150: 输入框的宽度
		rect.right = rect.left + 100;

		CEdit * edit = new CEdit;
		edit->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rect, m_param_area, ctrl.m_ctrl_id);
		const std::wstring & val = pt.get<std::wstring>(ctrl.m_name);
		edit->SetWindowText(val.c_str());
		ctrl.m_ctrl = static_cast<CWnd*>(edit);
		m_ctrl_list.push_back(ctrl);
	}
}

bool CProcessorCtrl::OnParamNotify(NMHDR * hdr)
{
	LOG_DEBUG(L"got message from %d, msg=%d", hdr->idFrom, hdr->code);
	return false;
}

BOOL CProcessorCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT * pResult)
{
	bool br;
	LPNMHDR hdr = (LPNMHDR)(lParam);
//	LOG_DEBUG(L"id=%d, wnd=%08X, code=%d", hdr->idFrom, hdr->hwndFrom, hdr->code)
	return 0;
}
