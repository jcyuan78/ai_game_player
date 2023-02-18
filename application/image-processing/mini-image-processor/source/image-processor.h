#pragma once

#include "../include/iimage_proc.h"

#define MAX_SOURCE	8


#define PARAMETER_TABLE_SUPPORT		\
	static const PARAMETER_DEFINE m_param_def[];	\
	static const size_t m_param_table_size;		\
	virtual size_t GetParamDefineTab(const PARAMETER_DEFINE * & tab) const				\
	{tab = m_param_def; return m_param_table_size;}			\

///////////////////////////////////////////////////////////////////////////////
//--

template <class T>
void SetParameterValue(const std::wstring & name, T & val, const boost::property_tree::wptree & prop)
{
	auto vv = prop.get_optional<T>(name);
	if (vv) val = *vv;
}

class CImageProcessorBase : public IImageProcessor
{
public:
	CImageProcessorBase(void);
	// 输出的通道数，review的通道数
	CImageProcessorBase(size_t out_num, size_t review_num = 0)
	{
		if (out_num > 0) m_dst_img.resize(out_num);
		if (review_num > 0) m_review_img.resize(review_num);
	}
	virtual ~CImageProcessorBase(void);

public:
	virtual void SetBox(IProcessorBox * box)
	{
		m_proc_box = box;	JCASSERT(m_proc_box);
	}
	virtual void ConnectTo(int input_id, IImageProcessor * src, int port);
	virtual int GetSourceNum() const { return m_source_num; }
	virtual int GetSourceChannel(int input_id) { return m_source_channel[input_id]; }
	virtual const IProcessorBox * GetParent(void) const { return m_proc_box; }
	virtual const std::wstring & GetProcessorName(void) const { return m_proc_box->GetName(); }


	virtual int GetChannelNum(void) const { return 1; };
	virtual void SetActiveChannel(int ch);
	virtual int GetActiveChannel(void) const { return m_active_channel; }

	// 缺省实现单通道输出
	virtual bool GetOutputImage(cv::Mat & img, int channel = 0) 
	{
		if (channel >= m_dst_img.size()) return false;
		img = m_dst_img[channel];
		return true; 
	}
//	virtual bool GetOutputImage(int img_id, cv::Mat & img) { img = m_dst_img; return true; }

	virtual bool GetProperty(const std::wstring & param_name, jcvos::IValue * & val) { return false; }
	virtual bool SetProperty(const std::wstring & prop_name, const jcvos::IValue * val) { return false; }

	virtual void OnInitialize(void);
	virtual bool OnParameterUpdated(const wchar_t * name, int val);
	virtual void OnRactSelected(const cv::Rect & rect);

	//virtual void write(cv::FileStorage & fs) const;
	//virtual void read(cv::FileNode & node);
	// 缺省实现用于保存current channel的result image
//	virtual void SaveResult(const std::string & fn);
	virtual void GetImageProperty(std::wstring & str, int index) {};
	virtual size_t GetParamDefineTab(const PARAMETER_DEFINE * & tab) const { return 0; }

	virtual void GetProperties(boost::property_tree::wptree & prop) {}
	virtual void SetProperties(const boost::property_tree::wptree & prop) {}

public:
	virtual void GetSource(int input_id, IImageProcessor * & src_proc);
//	void GetSourceImage(int inport, cv::Mat & img);
	virtual bool GetReviewImage(cv::Mat & img, int index)
	{
		if (m_review_img.size() == 0) return GetOutputImage(img, index); 
		if (index >= m_review_img.size()) return false;
	//	if (inport >= m_source_num) THROW_ERROR(ERR_APP, L"invalid inport=%d, source num=%d", inport, m_source_num);
		img = m_review_img[index];
	}
	virtual bool GetOutputData(cv::OutputArray out, int channel)
	{
		if (channel >= m_dst_img.size()) THROW_ERROR(ERR_APP, L"invalid channel=%d, output size=%d", channel, m_dst_img.size());
		m_dst_img[channel].copyTo(out);
		return true;
	}
	void GetSourceData(int inport, cv::OutputArray out)
	{
		if (inport >= m_source_num) THROW_ERROR(ERR_APP, L"invalid inport=%d, source num=%d", inport, m_source_num);
		IImageProcessor * src_proc = m_source[inport];
		int channel = m_source_channel[inport];
		src_proc->GetOutputData(out, channel);
	}

protected:
	IProcessorBox * m_proc_box;	//不需要引用计数管理

	IImageProcessor * m_source[MAX_SOURCE];			// 输入端的processor
	int m_source_channel[MAX_SOURCE];					// source processor的输出端口
	int m_source_num;

//	cv::Mat		m_dst_img;
	std::vector<cv::Mat> m_dst_img;
	std::vector<cv::Mat> m_review_img;
	int	m_active_channel;
	cv::Rect	m_roi;
};


///////////////////////////////////////////////////////////////////////////////
//--
class CProcSource : public CImageProcessorBase
{
public:
	CProcSource(void) { m_dst_img.resize(1); };
	virtual ~CProcSource(void) {};

public:
	virtual bool SetProperty(const std::wstring & prop_name, const jcvos::IValue * val);
	virtual bool OnCalculate(void);
	virtual const char * GetProcTypeA(void) const { return "source"; };

protected:
	std::wstring	m_file_name;
};


///////////////////////////////////////////////////////////////////////////////
//-- 预处理：灰度+减噪，二值
//	输入：[通道0]=原始图像 
//	输出: [通道0]=灰度+减噪 [通道1]=二值图像
class CPreProcessor : public CImageProcessorBase
{
public:
	CPreProcessor(void) { 
		m_threshold = 127; m_blur_size = 5; 
		m_dst_img.resize(2);
	};
	virtual ~CPreProcessor(void) {};

public:
	virtual bool OnCalculate(void);
	virtual const char * GetProcTypeA(void) const { return "pre-proces"; };

	virtual void GetProperties(boost::property_tree::wptree & prop)
	{
		prop.add(L"blur", m_blur_size);
		prop.add(L"threshold", m_threshold);
	}
	virtual void SetProperties(const boost::property_tree::wptree & prop)
	{
//		m_blur_size = prop.get<int>(L"blur");
		SetParameterValue(L"blur", m_blur_size, prop);
		SetParameterValue(L"threshold", m_threshold, prop);
	}

protected:
	PARAMETER_TABLE_SUPPORT;

protected:
	int m_blur_size;
	int m_threshold;
	cv::Mat m_binary;
};

///////////////////////////////////////////////////////////////////////////////
//-- 二值
//	输入：[通道0]=原始图像 
//	输出: [通道0]=灰度+减噪 [通道1]=二值图像
class CBinary : public CImageProcessorBase
{
public:
	CBinary(void) : CImageProcessorBase(1, 1) {
		m_threshold = 127;
	};
	virtual ~CBinary(void) {};

public:
	virtual bool OnCalculate(void);
	virtual const char * GetProcTypeA(void) const { return "binary"; };

	virtual void GetProperties(boost::property_tree::wptree & prop)
	{
		prop.add(L"threshold", m_threshold);
	}
	virtual void SetProperties(const boost::property_tree::wptree & prop)
	{
		SetParameterValue(L"threshold", m_threshold, prop);
	}

protected:
	PARAMETER_TABLE_SUPPORT;

protected:
	int m_threshold;
};

///////////////////////////////////////////////////////////////////////////////
//-- 角检测

class CCornerDetect : public CImageProcessorBase
{
	// 预览：[通道0]：原图叠加顶点
	// 输出：[通道0]：顶点vector
public:
	CCornerDetect(void) : CImageProcessorBase(0, 1) {
		block_size = 2; ksize = 3; m_k = 0.04;
		max_corner = 100;
	};
	virtual ~CCornerDetect(void) {};

public:
	virtual bool OnCalculate(void);
	virtual const char * GetProcTypeA(void) const { return "corner"; };
	//virtual bool GetReviewImage(cv::Mat & img, int index);

	virtual void GetImageProperty(std::wstring & str, int index)
	{
		wchar_t ch[128];
		swprintf_s(ch, L"corner num=%d", m_corners.size());
		str = ch;
	}

	virtual void GetProperties(boost::property_tree::wptree & prop)
	{
		prop.add(L"block_size", block_size);
		prop.add(L"max_corner", max_corner);
		prop.add(L"k", m_k * 1000);
	}
	virtual void SetProperties(const boost::property_tree::wptree & prop)
	{
		block_size = prop.get<int>(L"block_size");
		max_corner = prop.get<int>(L"max_corner");
		m_k = prop.get<double>(L"k") / 1000.0;
	}

	virtual bool GetOutputData(cv::OutputArray out, int channel)
	{
		if (channel == 0) cv::Mat(m_corners).copyTo(out);
		else return false;
		return true;
	}

protected:
	PARAMETER_TABLE_SUPPORT;

protected:
	int block_size, ksize;
	int max_corner;
	double m_k;

	std::vector<cv::Point2f> m_corners;
};


///////////////////////////////////////////////////////////////////////////////
//-- Canny 边界检测 + Hough 直线检测
class CProcHoughLine : public CImageProcessorBase
{
	// 输入：[通道0]：原图
	// 预览：[通道0]：原图+线段，[通道1]：原图+直线检出，[通道2]：轮廓
	// 输出：[通道0]：笛卡尔坐标线段检出，[通道1]：极坐标直线检出，[通道2]：轮廓
public:
	CProcHoughLine(void);
	virtual ~CProcHoughLine(void) {};

public:
	virtual bool OnCalculate(void);
	//virtual bool GetProperty(const CJCStringT & param_name, jcvos::IValue * & val);
	virtual const char * GetProcTypeA(void) const { return "hough-line"; };
	virtual bool GetReviewImage(cv::Mat & img, int index);

	virtual void GetProperties(boost::property_tree::wptree & prop)
	{
		prop.add(L"hough_thresh",	m_threshold		);
		prop.add(L"hough_length",	m_length		);
		prop.add(L"hough_gap",		m_gap			);
		prop.add(L"canny_kernel",	m_kernel_size	);
		prop.add(L"canny_th_low",	m_threshold_low	);
		prop.add(L"canny_th_hi",	m_threshold_hi	);
		//prop.add(L"scale",	scale	);
		//prop.add(L"delta",	delta	);
		prop.add(L"theta", m_theta);
	}

	virtual void SetProperties(const boost::property_tree::wptree & prop)
	{
		SetParameterValue(L"hough_thresh",	m_threshold,	prop);
		SetParameterValue(L"hough_length",	m_length,		prop);
		SetParameterValue(L"hough_gap",		m_gap,			prop);
		SetParameterValue(L"canny_kernel",	m_kernel_size,	prop);
		SetParameterValue(L"canny_th_low",	m_threshold_low,prop);
		SetParameterValue(L"canny_th_hi",	m_threshold_hi,	prop);
		//SetParameterValue(L"scale",	m_threshold_low,prop);
		//SetParameterValue(L"delta",	m_threshold_hi,	prop);
	}
	//virtual bool GetOutputImage(cv::Mat & img, int channel = 0)
	//{
	//	if (channel == 2) img = m_canny;
	//	return true;
	//}
	virtual bool GetOutputData(cv::OutputArray out, int channel)
	{
		if (channel == 0)	{	cv::Mat tmp(m_lines); tmp.copyTo(out);		}
		else if (channel == 1) { cv::Mat tmp(m_lines_rt); tmp.copyTo(out); }
		else if (channel == 2) { m_canny.copyTo(out); }
		else return false;
		return true;
	}

	virtual void GetImageProperty(std::wstring & str, int index)
	{
		wchar_t ch[100];
		if (index == 0)		swprintf_s(ch, L"line num = %d", m_lines.size());
		else if (index == 1) swprintf_s(ch, L"line num = %d", m_lines_rt.size());
		else return;
		str = ch;
	}


protected:
	PARAMETER_TABLE_SUPPORT;

protected:
	int		m_threshold, m_length, m_gap;
	double m_theta;

	int		m_kernel_size;
	int		m_threshold_low;
	int		m_threshold_hi;
	int		scale, delta;

	// canny 边界检测的中间结果
	cv::Mat m_canny;
	std::vector<cv::Vec4i> m_lines;
	std::vector<cv::Vec2f> m_lines_rt;
};


class CProcBoundingCircle : public CImageProcessorBase
{
	// input 0: result of canny out
	// input 1: source for show picture
	// 输入：[通道0]预览用灰度图像，[通道1]计算用图像
	// 预览：[通道0]灰度图像上叠加圆范围
	// 输出：[通道0]bounding列表
public:
	CProcBoundingCircle(void) : CImageProcessorBase(0, 2) {};
	virtual ~CProcBoundingCircle(void) {};

public:
	virtual bool OnCalculate(void);
	virtual const char * GetProcTypeA(void) const { return "contour"; };

	//virtual bool GetReviewImage(cv::Mat & img, int index);
	virtual void GetImageProperty(std::wstring & str, int index)
	{
		wchar_t ch[100];
		if (index == 0)		swprintf_s(ch, L"found contour=%d", m_enclosing.size());
		else if (index == 1)swprintf_s(ch, L"found enclosing=%d", m_enclosing.size());
		str = ch;
	}
	virtual bool GetOutputData(cv::OutputArray out, int channel)
	{
		if (channel == 0) cv::Mat(m_enclosing).copyTo(out);
		else return false;
		return true;
	}

	virtual void GetProperties(boost::property_tree::wptree & prop)
	{
		prop.add(L"canny_kernel", m_kernel_size);
		prop.add(L"canny_th_low", m_threshold_low);
		prop.add(L"canny_rate", m_rate);
	}

	virtual void SetProperties(const boost::property_tree::wptree & prop)
	{
		SetParameterValue(L"canny_kernel", m_kernel_size, prop);
		SetParameterValue(L"canny_th_low", m_threshold_low, prop);
		SetParameterValue(L"canny_rate", m_rate, prop);
	}

protected:
	PARAMETER_TABLE_SUPPORT;
	int m_threshold_low = 50;
	int m_rate = 4;
	int m_kernel_size = 3;

protected:
	std::vector<cv::Vec3f> m_enclosing;
};


class CTestFindTriangle : public CImageProcessorBase
{
	// 输入：[通道0]预览用灰度图像，[通道1]顶点，[通道2]区域
	// 预览：[通道0]灰度图像上叠加圆范围
	// 输出：
public:
	CTestFindTriangle(void) : CImageProcessorBase(1, 3) { m_expand = 10; };
	virtual ~CTestFindTriangle(void) {};

public:
	virtual bool OnCalculate(void);
	virtual const char * GetProcTypeA(void) const { return "contour"; };

	//virtual bool GetReviewImage(cv::Mat & img, int index) {
	//	img = m_review1;
	//	return true;
	//};
	virtual void GetImageProperty(std::wstring & str, int index)
	{
		wchar_t ch[100];
		swprintf_s(ch, L"found contour=%d", m_enclosing.size());
		str = ch;
	}

	virtual void GetProperties(boost::property_tree::wptree & prop)
	{
		prop.add(L"expand", m_expand);
	}

	virtual void SetProperties(const boost::property_tree::wptree & prop)
	{
		SetParameterValue(L"expand", m_expand, prop);
	}

protected:
	PARAMETER_TABLE_SUPPORT;

protected:
	//	void DrawContour(cv::Mat & img, int index, const cv::Scalar & c_contour, const cv::Scalar & c_rect);

protected:
	std::vector<cv::Vec3f> m_enclosing;
	// 扩大圆的半径
	int m_expand;

//	cv::Mat m_review1;
};


class CircleDetect
{
public:
	int x0, y0, r;
	UINT val;
};

class CTestHoughCircle : public CImageProcessorBase
{
	// 输入：[通道0]预览用灰度图像，[通道1]顶点，[通道2]区域
	// 预览：[通道0]灰度图像上叠加圆范围
	// 输出：
public:
	CTestHoughCircle(void) : CImageProcessorBase(0, 2), m_circles(NULL) { m_expand = 10; };
	virtual ~CTestHoughCircle(void) {
		delete[] m_circles;
	};

public:
	virtual bool OnCalculate(void);
	virtual const char * GetProcTypeA(void) const { return "hough_circle"; };

	virtual void GetImageProperty(std::wstring & str, int index)
	{
	}

	virtual void GetProperties(boost::property_tree::wptree & prop)
	{
		prop.add(L"expand", m_expand);
	}

	virtual void SetProperties(const boost::property_tree::wptree & prop)
	{
		SetParameterValue(L"expand", m_expand, prop);
	}

protected:
	void AddCircle(CircleDetect & c);

protected:
	const int m_max_points = 16;
	// 扩大圆的半径
	int m_expand;

	CircleDetect * m_circles;
	CircleDetect m_max_circle;
};

class CTestSobel : public CImageProcessorBase
{
	// 输入：[通道0]灰度图像
	// 输出/预览：[通道0]Sobel filter
public:
	CTestSobel(void) : CImageProcessorBase(5, 5) {};
	virtual ~CTestSobel(void) {};

public:
	virtual bool OnCalculate(void);
	virtual const char * GetProcTypeA(void) const { return "sobel"; };



};