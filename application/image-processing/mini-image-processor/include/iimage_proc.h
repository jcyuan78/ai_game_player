#pragma once
// 定义image processor接口

#include <stdext.h>
#include <opencv2/core/core.hpp>
#include <jcparam.h>
#include <boost/property_tree/ptree.hpp>

class IBoxContainer;
class IImageProcessor;

enum CTRL_TYPE {
	CTRL_EDIT, CTRL_SLIDE,
};


#define MAX_PARAM_NAME	32


class PARAMETER_DEFINE
{
public:
	UINT id;
	wchar_t name[MAX_PARAM_NAME];
	int default_val;
	int max_val;
	size_t offset;
	CTRL_TYPE ctrl_type;
};

class IProcessorBox : public IJCInterface
{	// 不需要引用计数
public:
	// processor请求box创建track bar，返回track bar的id
	virtual UINT RegistTrackBar(const wchar_t * name, int def_val, int max_val) = 0;
	virtual const std::wstring & GetName(void) const = 0;
	virtual void OnTrackBarUpdated(UINT id, const std::string & name, int val) = 0;
	virtual void SetContainer(IBoxContainer * cont) = 0;
	virtual void OnKeyPressed(int key_code) = 0;
	virtual void UpdateTrackBar(UINT id, int val) = 0;
	virtual void OnUpdateBox(void) = 0;

	virtual bool Load(const boost::property_tree::wptree & pt) = 0;
	virtual bool Save(boost::property_tree::wptree & pt) = 0;

	virtual bool SetParameter(const std::wstring & prop_name, const std::wstring & prop_val) = 0;
	virtual void SetParameter(const boost::property_tree::wptree & pt) = 0;
	virtual void GetParameters(boost::property_tree::wptree & pt) = 0;
	virtual cv::Mat & GetReviewImage(int index) = 0;

	virtual void ConnectTo(int port, IProcessorBox * src_box) = 0;
	virtual void GetProcessor(IImageProcessor * & proc) = 0;
	virtual void GetBoxProperty(std::wstring & str) = 0;
	virtual void GetImageProperty(std::wstring & str,int index) = 0;
	virtual size_t GetParamNumber(void) = 0;
	virtual const PARAMETER_DEFINE * GetParamInfo(size_t index) = 0;
//	virtual double GetParamVal(std::wstring & name) = 0;
//	virtual void GetProperties(boost::property_tree::wptree & prop) = 0;
//	virtual void SetProperties(const boost::property_tree::wptree & prop) = 0;
};

class IBoxContainer
{
public:
	virtual void SetActiveBox(IProcessorBox * box) = 0;
};

class IImageProcessor : public IJCInterface
{
public:
	virtual void SetBox(IProcessorBox * box) = 0;
	// 指定processor的输入端(input_id)连接到输出processor，输出端只提供单一输出
	virtual void ConnectTo(int input_id, IImageProcessor * src, int channel) = 0; 
	virtual int GetSourceNum() const = 0;
	virtual int GetSourceChannel(int input_id) = 0;
	virtual const std::wstring & GetProcessorName(void) const = 0;
	virtual void GetSource(int input_id, IImageProcessor * & src_proc) = 0;

	// 启动processor计算，返回是否需要更新结果
	virtual bool OnCalculate(void) = 0;
	// 更新参数，返回是否需要更新结果
	virtual bool OnParameterUpdated(const wchar_t * name, int val) = 0;

	virtual void OnInitialize(void) = 0;
	virtual int GetChannelNum(void) const = 0;
	// ch >=0, set absoluted channel id. ch < set related channel from current (move channel)
	virtual void SetActiveChannel(int ch) = 0;
	virtual int GetActiveChannel(void) const = 0;

	// 获取一些通过计算得到的参数，比如图像倾角等，如果不存在指定参数，侧返回false
	//virtual bool GetProperty(const std::wstring & prop_name, jcvos::IValue * & val) = 0;
	virtual bool SetProperty(const std::wstring & prop_name, const jcvos::IValue * val) = 0;

	virtual void GetProperties(boost::property_tree::wptree & prop) = 0;
	virtual void SetProperties(const boost::property_tree::wptree & prop) = 0;

	virtual const char * GetProcTypeA(void) const = 0;

	virtual bool GetReviewImage(cv::Mat & img, int index) = 0;
	virtual void OnRactSelected(const cv::Rect & rect) = 0;
	virtual void GetImageProperty(std::wstring & str, int index) = 0;
	virtual size_t GetParamDefineTab(const PARAMETER_DEFINE * & tab) const = 0;
	virtual bool GetOutputData(cv::OutputArray out, int channel) = 0;

};
