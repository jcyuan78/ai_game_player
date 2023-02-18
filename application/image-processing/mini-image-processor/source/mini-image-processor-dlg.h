
// mini-image-processorDlg.h: 头文件
//

#pragma once

#include "box_manager.h"
#include "processor_ctrl.h"
#include <img_comm_lib.h>

// CMiniImageProcessorDlg 对话框
class CMiniImageProcessorDlg : public CDialogEx
{
// 构造
public:
	CMiniImageProcessorDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MINIIMAGEPROCESSOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClickedOpenSource();
	afx_msg void OnClickedLoadConfig();
	afx_msg void OnClickedSrcSelectPre();
	afx_msg void OnClickedSrcSelectNext();
	afx_msg void OnClickedDstSelectPre();
	afx_msg void OnClickedDstSelectNext();
	afx_msg void OnClickedRefresh();
	afx_msg void OnChangeDstIndex() { UpdateImageIndex(1); }
	afx_msg void OnChangeSrcIndex() { UpdateImageIndex(0); }
	afx_msg void OnClickedSrcSaveImg() { m_image_ctrls[0].SaveImage(); }
	afx_msg void OnClickedDstSaveImg() { m_image_ctrls[1].SaveImage(); }
	DECLARE_MESSAGE_MAP()


protected:
	void GetImageInfo(int img_id)
	{
		std::wstring prop, index_prop;
		m_image_ctrls[img_id].GetImageProperty(prop, index_prop);
		int box_num = (int)(m_boxes.GetBoxNum());
		m_properties[img_id].Format(L"box:%d/%d, %s\n%s", m_box_id[img_id]+1, box_num, prop.c_str(), index_prop.c_str());

	}
	void UpdateImage(int img_id)
	{
		m_image_ctrls[img_id].Update();
		GetImageInfo(img_id);
		UpdateData(FALSE);
	}
	void SetBoxToCtrl(int img_id)
	{
		jcvos::auto_interface<IProcessorBox> box0;
		m_boxes.GetProcessorBox(box0, m_box_id[img_id]);
		m_image_ctrls[img_id].SetProcessorBox(box0);
	}
	void UpdateImageIndex(int img_id)
	{
		if (m_image_ctrls[img_id].GetSafeHwnd() == NULL) return;
		int cur_index = m_image_index[img_id];
		UpdateData(TRUE);
		bool br = m_image_ctrls[img_id].SetImageIndex(m_image_index[img_id]);
		if (!br) m_image_index[img_id] = cur_index;
		GetImageInfo(img_id);

		UpdateData(FALSE);
	}
	//void UpdateDst();
public:
	CEdit m_config_edit;
	CProcessorCtrl m_image_ctrls[2];
	// 两个显示单元对应的box索引号
	int m_box_id[2];

protected:
	// 管理处理器的
	CBoxManager m_boxes;
public:
	CString m_properties[2];
//	int m_src_index;
//	int m_dst_index;
	int m_image_index[2];
//	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnClickedSaveConfig();
};
