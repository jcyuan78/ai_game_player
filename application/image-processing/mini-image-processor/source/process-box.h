#pragma once
#include "../include/iimage_proc.h"
#include <opencv2/imgproc.hpp>

class CProcessorBoxBase : public IProcessorBox
{
public:
	CProcessorBoxBase(void);
	virtual ~CProcessorBoxBase(void);

public:
	static void CreateBox(IProcessorBox * & box, const std::wstring & type, const std::wstring & name);
	static void CreateImageProcessor(IImageProcessor * & processor, const std::wstring & type);

public:
	virtual UINT RegistTrackBar(const wchar_t * name, int def_val, int max_val) { return 0; }
	virtual const std::wstring&  GetName(void) const { return m_name; }
	virtual void OnTrackBarUpdated(UINT id, const std::string & name, int val) {}
	virtual void SetContainer(IBoxContainer * cont) {}
	virtual void OnKeyPressed(int key_code) {}
	virtual void UpdateTrackBar(UINT id, int val) {}
	virtual void OnUpdateBox(void)
	{
		m_processor->OnCalculate();
//		FillReviewBuf(m_local_buf);
	}

	virtual bool Load(const boost::property_tree::wptree & pt);
	virtual bool Save(boost::property_tree::wptree & pt);

	virtual bool SetParameter(const std::wstring & prop_name, const std::wstring & prop_val)
	{
		void * data = reinterpret_cast<void*>(const_cast<std::wstring*>(&prop_val));
		jcvos::IValue * val = jcvos::CreateTypedValue(jcvos::VT_STRING, data);
		m_processor->SetProperty(prop_name, val);
		RELEASE(val);
		return true;
	}
	virtual cv::Mat & GetReviewImage(int index)
	{
		cv::Mat img;
		m_processor->GetReviewImage(img, index);
		if (img.channels() < 3)	cv::cvtColor(img, m_local_buf, CV_GRAY2BGR);
		else					img.copyTo(m_local_buf);

		return m_local_buf;
	}

	virtual void ConnectTo(int port, IProcessorBox * src_box)
	{
		jcvos::auto_interface<IImageProcessor> proc;
		src_box->GetProcessor(proc);
		if (proc == NULL) THROW_ERROR(ERR_APP, L"src box is not contian a processor");
		m_processor->ConnectTo(0, proc, port);
	}

	virtual void GetProcessor(IImageProcessor * & proc)
	{
		JCASSERT(proc == NULL && m_processor);
		proc = m_processor;
		proc->AddRef();
	}

	virtual void GetBoxProperty(std::wstring & str)
	{
		wchar_t ch[64];
		swprintf_s(ch, L"name:%s, type:%s", m_name.c_str(), m_type.c_str());
		str = ch;
	}
	virtual void GetImageProperty(std::wstring & str, int index) 
	{
		JCASSERT(m_processor);
		m_processor->GetImageProperty(str, index);
	}
	virtual size_t GetParamNumber(void)
	{
		const PARAMETER_DEFINE * ptab = NULL;
		size_t num = m_processor->GetParamDefineTab(ptab);
		return num;
	}
	virtual const PARAMETER_DEFINE * GetParamInfo(size_t index)
	{
		const PARAMETER_DEFINE * ptab = NULL;
		size_t num = m_processor->GetParamDefineTab(ptab);
		if (index >= num) return NULL;
		return ptab + index;
	}

	//virtual double GetParamVal(std::wstring & name)
	//{
	//	m_processor->getP
	//}
	virtual void SetParameter(const boost::property_tree::wptree & pt)
	{
		m_processor->SetProperties(pt);
	}

	virtual void GetParameters(boost::property_tree::wptree & pt)
	{
		m_processor->GetProperties(pt);
	}


protected:
	void FillReviewBuf(cv::Mat & buf)
	{
		cv::Mat img;
		m_processor->GetReviewImage(img,0);
//		cv::resize(img, img, cv::Size(img.cols / m_review_scale, img.rows / m_review_scale));
		if (img.channels() < 3)	cv::cvtColor(img, buf, CV_GRAY2BGR);
		else					img.copyTo(buf);
	}

protected:
	IImageProcessor * m_processor;
	std::wstring m_name;
	std::wstring m_type;
	cv::Mat m_local_buf;
};

