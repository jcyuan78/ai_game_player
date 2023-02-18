#pragma once

#include <img_comm_lib.h>
#include "../include/iimage_proc.h"

class CParamCtrl
{
public:
	CWnd * m_ctrl;
	CStatic *m_label;
	std::wstring m_name;
	UINT  m_ctrl_id;
};

class CProcessorCtrl :
	public COpenCvImageCtrl
{
public:
	CProcessorCtrl(void);
	virtual ~CProcessorCtrl(void);
public:
	void SetProcessorBox(IProcessorBox * box);
	void Update()
	{
		if (!m_box) return;
		cv::Mat & img = m_box->GetReviewImage(m_image_index);
		SetImage(img, 0);
		Invalidate();
	}
	void GetImageProperty(std::wstring & prop, std::wstring & index_prop)
	{
		if (!m_box) return;
		m_box->GetBoxProperty(prop);
		m_box->GetImageProperty(index_prop, m_image_index);
	}

	void SetParamArea(CWnd * wnd) 	{		m_param_area = wnd;  	}

	bool OnParamNotify(NMHDR * hdr);

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	void UpdateParam(void)
	{
		boost::property_tree::wptree pt;
		for (auto it = m_ctrl_list.begin(); it != m_ctrl_list.end(); ++it)
		{
			CString str;
			if (it->m_ctrl) it->m_ctrl->GetWindowText(str);
			pt.add(it->m_name, (const wchar_t *)(str));
		}
		m_box->SetParameter(pt);
	}

	bool SetImageIndex(int index)
	{
		m_image_index = index;
		if (this->GetSafeHwnd() != NULL)
		{
			Update();
			return true;
		}
		else return false;
	}

	bool SaveImage(void)
	{
		CFileDialog dlg(FALSE, L"jpeg", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"image file|*.jpg;*.png||");
		if (dlg.DoModal() != IDOK) return false;
		CString fn = dlg.GetPathName();
		std::string str_fn;
		jcvos::UnicodeToUtf8(str_fn, (const wchar_t*)fn);
//		str_fn += ".jpg";
		cv::Mat &img = m_box->GetReviewImage(m_image_index);
		cv::imwrite(str_fn, img);
		return true;
	}

protected:
	void Reset(void)
	{
		RELEASE(m_box);
		RELEASE(m_processer);
		for (auto it = m_ctrl_list.begin(); it!= m_ctrl_list.end(); ++it)
		{
			if (it->m_ctrl)
			{
				it->m_ctrl->CloseWindow();
				it->m_ctrl->DestroyWindow();
				delete it->m_ctrl;
			}
			if (it->m_label)
			{
				it->m_label->CloseWindow();
				it->m_label->DestroyWindow();
				delete it->m_label;
			}
		}
		m_ctrl_list.clear();
		m_param_area->Invalidate();
		GetParent()->Invalidate();
		if (edit)
		{
			edit->DestroyWindow();
			delete edit;
		}
	}

public:
	// 当前显示的预览号，
	int m_image_index;
	UINT m_ctrl_start_id;

	CEdit * edit;

protected:
	IProcessorBox * m_box;
	IImageProcessor * m_processer;
//	CRect m_param_area;
	CWnd * m_param_area;
	std::vector<CParamCtrl> m_ctrl_list;

};

