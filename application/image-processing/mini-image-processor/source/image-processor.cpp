#include "pch.h"

//#define _CRT_SECURE_NO_WARNINGS
#include "image-processor.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


LOCAL_LOGGER_ENABLE(L"image_processor", LOGGER_LEVEL_NOTICE);

///////////////////////////////////////////////////////////////////////////////
//--
LOG_CLASS_SIZE(CImageProcessorBase);

CImageProcessorBase::CImageProcessorBase(void)
	: m_proc_box(NULL), m_active_channel(0), m_source_num(0)
{
	memset(m_source, 0, sizeof(IImageProcessor*) * MAX_SOURCE);
}

CImageProcessorBase::~CImageProcessorBase(void)
{
	LOG_STACK_TRACE();
}

void CImageProcessorBase::ConnectTo(int , IImageProcessor * src, int port)
{
	LOG_STACK_TRACE();
	int input_id = m_source_num;
	LOG_DEBUG(L"connect src: <%p> to dst: <%p>, port %d", src, this, input_id);
	if (input_id >= MAX_SOURCE) THROW_ERROR(ERR_APP, L"too many sources, cur=%d, total=%d", input_id, MAX_SOURCE);
//	JCASSERT(input_id < MAX_SOURCE);
	if (m_source[input_id] != NULL)
	{
		THROW_ERROR(ERR_PARAMETER, L"source %d has already been set.", input_id);
	}
	m_source[input_id] = src;
	m_source_channel[input_id] = port;
	m_source_num++;
}

void CImageProcessorBase::GetSource(int input_id, IImageProcessor * & src_proc)
{
	JCASSERT(src_proc == NULL);
	JCASSERT(input_id < MAX_SOURCE);
	src_proc = m_source[input_id];		JCASSERT(src_proc);
	src_proc->AddRef();
}

void CImageProcessorBase::OnInitialize(void)
{
	const PARAMETER_DEFINE * param_tab = NULL;
	size_t param_num = GetParamDefineTab(param_tab);
	if ((param_num > 0) && (param_tab) && (m_proc_box))
	{	// m_proc_box允许为空，为空时忽略GUI处理
		for (size_t ii = 0; ii < param_num; ++ii)
		{
			*((int*)((char *)(this) + param_tab[ii].offset)) = param_tab[ii].default_val;
			m_proc_box->RegistTrackBar(param_tab[ii].name,
				param_tab[ii].default_val, param_tab[ii].max_val);
		}
	}
}

bool CImageProcessorBase::OnParameterUpdated(const wchar_t * name, int val)
{
	bool updated = false;
	const PARAMETER_DEFINE * param_tab = NULL;
	size_t param_num = GetParamDefineTab(param_tab);
	if ((param_num > 0) && (param_tab))
	{
		for (size_t ii = 0; ii < param_num; ++ii)
		{
			if (wcscmp(name, param_tab[ii].name) == 0)
			{
				*((int*)((char *)(this) + param_tab[ii].offset)) = val;
				updated = true;
				break;
			}
		}
	}
	return updated;
}


//void CImageProcessorBase::read(cv::FileNode & node)
//{
//	LOG_STACK_TRACE();
//	const PARAMETER_DEFINE * param_tab = NULL;
//	size_t param_num = GetParamDefineTab(param_tab);
//	if ((param_num > 0) && (param_tab))
//	{
//		//for (size_t ii = 0; ii < param_num; ++ii)
//		//{
//		//	int &param_val = *((int*)((char *)(this) + param_tab[ii].offset));
//		//	param_val = node[param_tab[ii].name];
//		//	LOG_DEBUG(_T("read '%S' = %d"), param_tab[ii].name, param_val);
//		//	if (m_proc_box) m_proc_box->UpdateTrackBar(ii, param_val);
//		//}
//	}
//	if (m_proc_box) m_proc_box->OnUpdateBox();
//}

//void CImageProcessorBase::GetSourceImage(int inport, cv::Mat & img)
//{
//	if (inport >= m_source_num) THROW_ERROR(ERR_APP, L"invalid inport=%d, source num=%d", inport, m_source_num);
//	IImageProcessor * src_proc = m_source[inport];
//	int channel = m_source_channel[inport];
//	src_proc->GetOutputImage(img, channel);
//}

void CImageProcessorBase::SetActiveChannel(int ch)
{
	int chs = GetChannelNum();
	m_active_channel = ch % chs;
}

//void CImageProcessorBase::SaveResult(const std::string & fn)
//{
//	std::string _fn = fn + ".jpg";
//	cv::Mat img;
//	GetOutputImage(img);
//	cv::imwrite(_fn, img);
//}

void CImageProcessorBase::OnRactSelected(const cv::Rect & rect)
{
	LOG_DEBUG(_T("ROI selected: (%d, %d) - (%d, %d)"), rect.x, rect.y, rect.width, rect.height);
	m_roi = rect;
}

///////////////////////////////////////////////////////////////////////////////
//--
LOG_CLASS_SIZE(CProcSource);

bool CProcSource::SetProperty(const std::wstring & prop_name, const jcvos::IValue * val)
{
	if (prop_name == L"file_name")	val->GetValueText(m_file_name);
	else return __super::SetProperty(prop_name, val);
	return false;
}

bool CProcSource::OnCalculate(void)
{
	std::string str_fn;
	jcvos::UnicodeToUtf8(str_fn, m_file_name.c_str());
	m_dst_img[0] = cv::imread(str_fn);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
//--
#define SCALE 4
LOG_CLASS_SIZE(CPreProcessor);

const PARAMETER_DEFINE CPreProcessor::m_param_def[] = {
	{ 0, L"blur", 2, 10, offsetof(CPreProcessor, m_blur_size), CTRL_EDIT },
	{ 1, L"threshold", 2, 10, offsetof(CPreProcessor, m_threshold), CTRL_EDIT },
};
const size_t CPreProcessor::m_param_table_size = sizeof(CPreProcessor::m_param_def) / sizeof(PARAMETER_DEFINE);

bool CPreProcessor::OnCalculate(void)
{
	cv::Mat tmp;
	GetSourceData(0, tmp);
	int width = tmp.cols, hight = tmp.rows;
	if (width == 0 || hight == 0) return false;
	LOG_NOTICE(L"source image size: (%d, %d)", width, hight);

	cv::Mat dst;
	cv::cvtColor(tmp, dst, CV_BGR2GRAY);

	// 降噪
	// kernel size 一定是单数
	if (m_blur_size == 0)
	{	// 不做降噪处理
		LOG_NOTICE(L"igrnor blur");
		dst.copyTo(m_dst_img[0]);
	}
	else
	{
		int blur = (m_blur_size - 1) * 2 + 1;
		cv::GaussianBlur(dst, m_dst_img[0], cv::Size(blur, blur), 0, 0);
	}

	// 二值化
//	cv::adaptiveThreshold(m_dst_img, m_binary, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 51, 2);
	cv::threshold(m_dst_img[0], m_dst_img[1], m_threshold, 255, cv::THRESH_BINARY_INV);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
//-- 预处理：灰度+减噪
const PARAMETER_DEFINE CCornerDetect::m_param_def[] = {
	{ 0, L"block_size", 3, 10, offsetof(CCornerDetect, block_size), CTRL_EDIT },
	{ 0, L"max_corner", 100, 1000, offsetof(CCornerDetect, max_corner), CTRL_EDIT },
	// k = val / 100
	{ 0, L"k", 40, 1000, offsetof(CCornerDetect, m_k), CTRL_EDIT },
};
const size_t CCornerDetect::m_param_table_size = sizeof(CCornerDetect::m_param_def) / sizeof(PARAMETER_DEFINE);

bool CCornerDetect::OnCalculate(void)
{
	cv::Mat tmp;
	GetSourceData(0, tmp);
	int width = tmp.cols, hight = tmp.rows;
	if (width == 0 || hight == 0) return false;
	LOG_NOTICE(L"loaded source image, size:(%d, %d)", width, hight);
//	cv::Mat dst;
//	cv::cornerHarris(tmp, dst, block_size, ksize, m_k);
//	cv::Mat dst_norm;
//	cv::normalize(dst, dst_norm, 1.0, 0.0, cv::NORM_INF);
//	LOG_DEBUG(L"depth of normalize=%d", dst_norm.depth());
//	for (int yy = 0; yy < dst_norm.rows; ++yy)
//	{
//		float * p = dst_norm.ptr<float>(yy);
//		for (int xx = 0; xx < dst_norm.cols; ++xx)
//		{
//			if (p[xx] <-0.1)
//			{
//				m_corners.push_back(cv::Point2i(xx, yy));
//				LOG_DEBUG(L"find corner (%d,%d)", xx, yy);
//			}
//		}
//	}


	cv::goodFeaturesToTrack(tmp, m_corners, max_corner, 0.05, 50.0, cv::Mat(), block_size, false, m_k);

	cv::Mat & review = m_review_img[0];
	cv::cvtColor(tmp, review, CV_GRAY2BGR);
	for (auto it = m_corners.begin(); it != m_corners.end(); ++it)
	{
		cv::circle(review, *it, 5, (0, 0, 255), 2);
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
//--
LOG_CLASS_SIZE(CProcHoughLine);

const PARAMETER_DEFINE CProcHoughLine::m_param_def[] = {
	{0, L"hough_thresh", 80, 500, offsetof(CProcHoughLine, m_threshold), CTRL_EDIT },
	{1, L"hough_length", 150, 500, offsetof(CProcHoughLine, m_length), CTRL_EDIT },
	{2, L"hough_gap", 10, 50, offsetof(CProcHoughLine, m_gap), CTRL_EDIT },
	{3, L"canny_kernel", 3, 10, offsetof(CProcHoughLine, m_kernel_size), CTRL_EDIT },
	//{4, L"scale", 90, 300, offsetof(CProcHoughLine, m_threshold_low), CTRL_EDIT },
	//{5, L"delta", 270, 500, offsetof(CProcHoughLine, m_threshold_hi), CTRL_EDIT },
	{4, L"canny_th_low", 90, 300, offsetof(CProcHoughLine, m_threshold_low), CTRL_EDIT },
	{5, L"canny_th_hi", 270, 500, offsetof(CProcHoughLine, m_threshold_hi), CTRL_EDIT },
};

const size_t CProcHoughLine::m_param_table_size = sizeof(CProcHoughLine::m_param_def) / sizeof(PARAMETER_DEFINE);

CProcHoughLine::CProcHoughLine(void)
	: /*CImageProcessorBase(3, 3),*/ m_threshold(80), m_length(150), m_gap(10), m_kernel_size(3), m_threshold_low(50), m_threshold_hi(200), m_theta(0)
	, scale(1), delta(0)
{
}

bool CProcHoughLine::OnCalculate(void)
{
	m_lines.clear();
//	jcvos::auto_interface<IImageProcessor> src_canny;
	//jcvos::auto_interface<IImageProcessor> src_img;

	cv::Mat source;
	//GetSource(0, src_img);		JCASSERT(src_img.valid());
	//src_img->GetOutputImage(source);

//	GetOutputData(source, 0);
	GetSourceData(0, source);

	cv::Canny(source, m_canny, m_threshold_low, m_threshold_hi, m_kernel_size);
	LOG_DEBUG(L"depth of canny = %d", m_canny.depth());
	//cv::Mat grad_x, grad_y;
	//int ddepth = CV_16S;
	//cv::Sobel(source, grad_x, ddepth, 1, 0, m_kernel_size, scale, delta, cv::BORDER_DEFAULT);
	//cv::Sobel(source, grad_y, ddepth, 0, 1, m_kernel_size, scale, delta, cv::BORDER_DEFAULT);

	//cv::Mat abs_grad_x, abs_grad_y;
	//cv::convertScaleAbs(grad_x, abs_grad_x);
	//cv::convertScaleAbs(grad_y, abs_grad_y);
	//cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, m_canny);

//	std::vector<cv::Vec4i> lines;
	HoughLinesP(m_canny, m_lines, 1, CV_PI / 180, m_threshold, m_length, m_gap);
	cv::HoughLines(m_canny, m_lines_rt, 1, CV_PI / 180, m_threshold);
	return true;
}

bool CProcHoughLine::GetReviewImage(cv::Mat & img, int index)
{
	cv::Mat source;
	GetSourceData(0, source);

	if (index == 0)
	{	// index == 0，获取原始图片+线条
		//jcvos::auto_interface<IImageProcessor> src_img;
		//GetSource(0, src_img);		JCASSERT(src_img.valid());
		//src_img->GetOutputImage(source);
		cv::cvtColor(source, img, CV_GRAY2BGR);
		for (size_t ii = 0; ii < m_lines.size(); ++ii)
		{
			cv::Vec4i ll = m_lines[ii];
			LOG_DEBUG(L"find line: (%d,%d), (%d,%d)", ll[0], ll[1], ll[2], ll[3]);
			cv::line(img, cv::Point(ll[0], ll[1]), cv::Point(ll[2], ll[3]), cv::Scalar(255, 0, 0), 3/*, CV_AA*/);
		}
	}
	else if (index == 1)
	{	// 以极坐标参数划线
		//jcvos::auto_interface<IImageProcessor> src_img;
		//cv::Mat source;
		//GetSource(0, src_img);		JCASSERT(src_img.valid());
		//src_img->GetOutputImage(source);
		cv::cvtColor(source, img, CV_GRAY2BGR);
		for (size_t ii = 0; ii < m_lines_rt.size(); ++ii)
		{
			cv::Vec2f & ll = m_lines_rt[ii];
			float rho = ll[0], theta = ll[1];
			cv::Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a * rho, y0 = b * rho;
			pt1.x = cvRound(x0 + 2000 * (-b));
			pt1.y = cvRound(y0 + 2000 * (a));
			pt2.x = cvRound(x0 - 2000 * (-b));
			pt2.y = cvRound(y0 - 2000 * (a));
			line(img, pt1, pt2, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
		}
	}
	else if (index == 2)
	{	// index == 1, 获取canny的输出
//		m_canny.copyTo(img);
		cv::Mat temp;
		cv::cvtColor(m_canny, img, CV_GRAY2BGR);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//-- 图形分割
const PARAMETER_DEFINE CProcBoundingCircle::m_param_def[] = {
	{0, L"canny_kernel", 3, 10, offsetof(CProcBoundingCircle, m_kernel_size), CTRL_EDIT },
	{1, L"canny_th_low", 90, 300, offsetof(CProcBoundingCircle, m_threshold_low), CTRL_EDIT },
	{3, L"canny_rate", 270, 500, offsetof(CProcBoundingCircle, m_rate), CTRL_EDIT },
};

const size_t CProcBoundingCircle::m_param_table_size = sizeof(CProcBoundingCircle::m_param_def) / sizeof(PARAMETER_DEFINE);

bool CProcBoundingCircle::OnCalculate(void)
{
	cv::Mat src_img;
	GetSourceData(0, src_img);	// 灰度

	cv::Mat canny;
	GetSourceData(1, canny);

	cv::Mat &review1 = m_review_img[1];
	cv::cvtColor(src_img, review1, CV_GRAY2BGR);

//	cv::Mat canny;
//	cv::Canny(src_img, canny, m_threshold_low, m_threshold_low*m_rate, m_kernel_size);
	cv::cvtColor(canny, m_review_img[0], CV_GRAY2BGR);

	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(canny, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	m_enclosing.clear();

	for (auto it = contours.begin(); it != contours.end(); ++it)
	{
		cv::Point2f center;
		float radius;
		cv::minEnclosingCircle(*it, center, radius);
		m_enclosing.push_back(cv::Vec3f(center.x, center.y, radius));
		cv::circle(review1, center, radius, cv::Scalar(0, 255, 0), 3);
		char str[20];
		sprintf_s(str, "r=%.2f", radius);
//		cv::putText(review1, str, cv::Point(center.x - 10, center.y - 10), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 3);
		LOG_DEBUG(L"find enclosing, (%.2f, %.2f), r=%.2f", center.x, center.y, radius);
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////
//-- 

const PARAMETER_DEFINE CTestFindTriangle::m_param_def[] = {
	{0, L"expand", 10, 500, offsetof(CTestFindTriangle, m_expand), CTRL_EDIT },
};

const size_t CTestFindTriangle::m_param_table_size = sizeof(CTestFindTriangle::m_param_def) / sizeof(PARAMETER_DEFINE);


bool CTestFindTriangle::OnCalculate(void)
{
	cv::Mat & review0 = m_review_img[0];
	GetSourceData(0, review0);	// 灰度图像
	if (review0.channels() == 1) cv::cvtColor(review0, review0, cv::COLOR_GRAY2BGR);

	cv::Mat & review1 = m_review_img[1];
	GetSourceData(0, review1);	// 灰度图像
	if (review1.channels() == 1) cv::cvtColor(review1, review1, cv::COLOR_GRAY2BGR);

	cv::Mat & review2 = m_review_img[2];
	GetSourceData(3, review2);	// 二值图像
	if (review2.channels() == 1) cv::cvtColor(review2, review2, cv::COLOR_GRAY2BGR);

	std::vector<cv::Point2f> corner;
	GetSourceData(1, corner);	
	std::vector<cv::Vec3f> enclosing;
	GetSourceData(2, enclosing);

	for (auto it = corner.begin(); it != corner.end(); ++it)
	{
		cv::circle(review0, *it, 5, (0, 0, 255), 2);
		cv::circle(review1, *it, 5, (0, 0, 255), 2);
		cv::circle(review2, *it, 5, (0, 0, 255), 2);
	}

	for (int ii = 0; ii < enclosing.size(); ++ii)
	{
		float x0 = (enclosing[ii])[0];
		float y0 = (enclosing[ii])[1];
		float r0 = (enclosing[ii])[2]+m_expand;
		float rr = r0 * r0;
		int corner_num = 0;
		cv::circle(review0, cv::Point((int)x0, (int)y0), (int)r0, cv::Scalar(0, 255, 0), 3);
//		cv::circle(review1, cv::Point((int)x0, (int)y0), (int)r0, cv::Scalar(0, 255, 0), 3);
		std::vector<cv::Point2i> triangle;
		for (auto it = corner.begin(); it != corner.end(); ++it)
		{
			float x1 = (*it).x, y1 = (*it).y;
			float dd = (x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0);
			if (dd < rr)
			{
				corner_num++;
				triangle.push_back(*it);
			}
		}
		LOG_DEBUG(L"area[%d], (%.2f,%.2f), r=%.2f, corner=%d", ii, x0, y0, rr, corner_num);
		char str[10];
		sprintf_s(str, "%d", corner_num);
		cv::putText(review0, str, cv::Point(x0 - 10, y0 - 10), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 3);
		cv::putText(review2, str, cv::Point(x0 - 10, y0 - 10), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 3);
//		sprintf_s(str, "#%d", ii);
//		cv::putText(review1, str, cv::Point(x0 - 10, y0 - 10), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 3);
		if (corner_num == 3)
		{
			cv::polylines(review1, triangle, true, cv::Scalar(255, 0, 0), 8);
		}
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
//--

bool CTestHoughCircle::OnCalculate(void)
{
	LOG_STACK_TRACE();
	cv::Mat canny;
	GetSourceData(0, canny);
	canny.copyTo(m_review_img[1]);

	const int max_r = 200;
	double dth = CV_PI / 180;

	int dims[3]={ 0 };

	int rows = canny.rows, cols = canny.cols;
	dims[0] = rows, dims[1] = cols, dims[2] = max_r;
//	cv::Mat hough(3, dims, CV_32F);

	UINT * hough = new UINT[rows * cols * max_r];
	memset(hough, 0, sizeof(UINT) * rows * cols * max_r);
	cv::Mat review(rows, cols, CV_32F, cv::Scalar::all(0));
	LOG_DEBUG(L"channle of review = %d", review.channels());


	float max_val = 0;
#if 0
	for (int rr = 1; rr < max_r; ++rr)
	{
		double d = CV_PI / (3 * rr);
//		double d = CV_PI / 180;
		for (double thth = 0; thth < 2 * CV_PI; thth += d)
		{
			double a0 = rr * cos(thth), b0 = rr * sin(thth);
			for (int yy = 0; yy < rows; ++yy)
			{
				BYTE * p = canny.ptr<BYTE>(yy);
				for (int xx = 0; xx < cols; ++xx)
				{
					if (p[xx] < 127) continue;
					int aa = xx - a0, bb = yy - b0;

					//int a = xx - rr * cc;
					//int b = yy - rr * ss;
					if ((aa >= 0) && (aa < cols) && (bb >= 0) && (bb < rows))
					{
						review.at<float>(cv::Point(aa, bb)) += 1;
						int index = (bb*cols + aa) * max_r + rr;
						hough[index] += 1;
						if (hough[index] > max_val)
						{
							max_val = hough[index];
							max_x0 = aa, max_y0 = bb;
							max_rr = rr;
						}
					}
				}
			}
		}
	}
#else
	delete[] m_circles;
	m_circles = NULL;
	m_circles = new CircleDetect[m_max_points];
	memset(m_circles, 0, sizeof(CircleDetect) * m_max_points);
//	m_circle_num = 0;

//	m_points = new cv::Vec3i[max_points];

	for (int yy = 0; yy < rows; ++yy)
	{
		BYTE * p = canny.ptr<BYTE>(yy);
		for (int xx = 0; xx < cols; ++xx)
		{
			if (p[xx] < 127) continue;
			for (int rr = 1; rr < max_r; ++rr)
			{
				double d = CV_PI / (3 * rr);
				for (double thth = 0; thth < 2 * CV_PI; thth += d)
				{
					double a0 = rr * cos(thth), b0 = rr * sin(thth);
					int aa = xx - a0, bb = yy - b0;

					//int a = xx - rr * cc;
					//int b = yy - rr * ss;
					if ((aa >= 0) && (aa < cols) && (bb >= 0) && (bb < rows))
					{
//						review.at<float>(cv::Point(aa, bb)) += 1;
						int index = (bb*cols + aa) * max_r + rr;
						hough[index] += 1;
					}
				}
			}
		}
	}
	m_max_circle.val = 0;

	for (int yy = 0; yy < rows; ++yy)
	{
		for (int xx = 0; xx < cols; ++xx)
		{
			int index = (yy*cols + xx) * max_r;
			int max_rr = 0;
			for (int rr = 1; rr < max_r; ++rr)
			{
				int hh = hough[index + rr];
				if (hh > max_rr) max_rr = hh;
				if (hh > m_max_circle.val)
				{
					m_max_circle.x0 = xx;
					m_max_circle.y0 = yy;
					m_max_circle.r = rr;
					m_max_circle.val = hh;
				}
				//CircleDetect cc;
				//cc.x0 = xx, cc.y0 = yy, cc.r = rr;
				//cc.val = hough[index+rr];
				//AddCircle(cc);
			}
			review.at<float>(cv::Point(xx, yy)) = max_rr;
		}
	}

#endif

	LOG_DEBUG(L"max value in (a,b,r) = %.2f", max_val);
/*
	//max_val = 0;
	for (int rr = 0; rr < max_r; ++rr)
	{
		cv::Mat map(rows, cols, CV_32F, cv::Scalar::all(0));

		for (int bb = 0; bb < rows; ++bb)
		{
			float * p = map.ptr<float>(bb);
			for (int aa = 0; aa < cols; ++aa)
			{
				int index = (bb*cols + aa) * max_r +rr;
				p[aa] = hough[index];
				//for (int rr = 0; rr < max_r; ++rr)
				//{
				//	p[aa] += hough[index + rr];
				//}
				//if (p[aa] > max_val) max_val = p[aa];
			}
		}
//		cv::normalize(map, map, 1.0, 0, cv::NORM_INF);
		review += map;
	}
	//LOG_DEBUG(L"max value in (a,) = %.2f", max_val);
*/
	cv::normalize(review, m_review_img[0], 255, 0, cv::NORM_INF, CV_8U);

	cv::cvtColor(m_review_img[1], m_review_img[1], CV_GRAY2BGR);
	char str[20];
	//for (int ii = 0; ii < m_max_points; ++ii)
	//{
	//	CircleDetect & circle = m_circles[ii];
	//	LOG_DEBUG(L"circle = (%d,%d), r=%d, val=%d", circle.x0, circle.y0, circle.r, circle.val);
	//	if (circle.val == 0) continue;
	//	
	//	sprintf_s(str, "r=%d", circle.r);
	//	cv::circle(m_review_img[1], cv::Point(circle.x0, circle.y0), 3, cv::Scalar(0, 0, 255), 1);
	//	cv::putText(m_review_img[1], str, cv::Point(circle.x0 + 10, circle.y0 + 10), 
	//		cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
	//	
	//}
	sprintf_s(str, "r=%d", m_max_circle.r);
	int x0 = m_max_circle.x0, y0 = m_max_circle.y0;
//	cv::circle(m_review_img[1], cv::Point(m_max_circle.x0, m_max_circle.y0), 3, cv::Scalar(0, 0, 255), 1);
	cv::line(m_review_img[1], cv::Point(x0 - 5, y0), cv::Point(x0 + 5, y0), cv::Scalar(255, 0, 0), 2);
	cv::line(m_review_img[1], cv::Point(x0, y0-5), cv::Point(x0, y0+5), cv::Scalar(255, 0, 0), 2);

	cv::circle(m_review_img[1], cv::Point(m_max_circle.x0, m_max_circle.y0), m_max_circle.r, cv::Scalar(255, 0, 0), 2);
	cv::putText(m_review_img[1], str, cv::Point(m_max_circle.x0 + 10, m_max_circle.y0 + 10),
			cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);
	delete[] hough;
	return true;
}

void CTestHoughCircle::AddCircle(CircleDetect & c)
{	// 按照堆排序，选出最少的
	if (c.val <= m_circles[1].val) return;
	m_circles[1] = c;
	UINT ptr = 1;
	while (ptr < m_max_points)
	{
		UINT left = ptr * 2, right = ptr * 2 + 1;
		if (left >= m_max_points) break;
		UINT pp;
		if (m_circles[left].val < m_circles[right].val) pp = left;
		else pp = right;
		if (m_circles[ptr].val > m_circles[pp].val)
		{
			CircleDetect a = m_circles[pp];
			m_circles[pp] = m_circles[ptr];
			m_circles[ptr] = a;
		}
		else break;
		ptr = pp;

	}
}

bool CTestSobel::OnCalculate(void)
{
	LOG_STACK_TRACE();
	cv::Mat src_img;
	GetSourceData(0, src_img);

	cv::Mat sobel1, sobel2;
	cv::Sobel(src_img, sobel1, -1, 1, 0, 3, 1);
	cv::Sobel(src_img, sobel2, -1, 1, 0, 3, -1);
	cv::addWeighted(sobel1, 1, sobel2, 1, 0, m_dst_img[2]);
	sobel1.copyTo(m_dst_img[0]);
	sobel2.copyTo(m_dst_img[1]);

	cv::Sobel(src_img, sobel1, -1, 0, 1, 3, 1);
	cv::Sobel(src_img, sobel2, -1, 0, 1, 3, -1);
	cv::addWeighted(sobel1, 1, sobel2, 1, 0, m_dst_img[3]);
	cv::addWeighted(m_dst_img[2], 1, m_dst_img[3], -1, 0, m_dst_img[4]);

	

	m_review_img[0] = m_dst_img[0];
	m_review_img[1] = m_dst_img[1];
	m_review_img[2] = m_dst_img[2];
	m_review_img[3] = m_dst_img[3];
	m_review_img[4] = m_dst_img[4];
	

	return true;
}


const PARAMETER_DEFINE CBinary::m_param_def[] = {
	{ 1, L"threshold", 2, 10, offsetof(CBinary, m_threshold), CTRL_EDIT },
};
const size_t CBinary::m_param_table_size = sizeof(CBinary::m_param_def) / sizeof(PARAMETER_DEFINE);



bool CBinary::OnCalculate(void)
{
	cv::Mat tmp;
	GetSourceData(0, tmp);
	int width = tmp.cols, hight = tmp.rows;
	if (tmp.channels() ==3)
	cv::cvtColor(tmp, tmp, CV_BGR2GRAY);

	// 二值化
	cv::threshold(tmp, m_dst_img[0], m_threshold, 255, cv::THRESH_BINARY);
	m_review_img[0] = m_dst_img[0];
	return true;
}
