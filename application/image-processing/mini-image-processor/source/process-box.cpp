#include "pch.h"
#include "process-box.h"
#include "image-processor.h"

LOCAL_LOGGER_ENABLE(L"processor_box", LOGGER_LEVEL_NOTICE);


CProcessorBoxBase::CProcessorBoxBase(void)
	:m_processor(NULL)
{
}

CProcessorBoxBase::~CProcessorBoxBase(void)
{
	RELEASE(m_processor);
}

void CProcessorBoxBase::CreateBox(IProcessorBox *& box, const std::wstring & type, const std::wstring & name)
{
	CProcessorBoxBase * bb = jcvos::CDynamicInstance<CProcessorBoxBase>::Create();
	bb->m_name = name;
//	bb->m_type = type;
	box = static_cast<IProcessorBox*>(bb);
}

void CProcessorBoxBase::CreateImageProcessor(IImageProcessor *& proc, const std::wstring & type)
{
	JCASSERT(proc == NULL);
	if (0) {}
	else if (type == L"source")		proc = static_cast<IImageProcessor*>(jcvos::CDynamicInstance<CProcSource>::Create());
	else if (type == L"pre-proces")	proc = static_cast<IImageProcessor*>(jcvos::CDynamicInstance<CPreProcessor>::Create());
	else if (type == L"corner")		proc = static_cast<IImageProcessor*>(jcvos::CDynamicInstance<CCornerDetect>::Create());
	//else if (type == _T("canny"))		proc = static_cast<IImageProcessor*>(new CProcCanny);
	//else if (type == _T("mophology"))	proc = static_cast<IImageProcessor*>(new CProcMophology);
	else if (type == L"houghline")	proc = static_cast<IImageProcessor*>(jcvos::CDynamicInstance<CProcHoughLine>::Create());
	//else if (type == _T("rotation"))	proc = static_cast<IImageProcessor*>(new CProcRotation);
	//else if (type == _T("contour"))	proc = static_cast<IImageProcessor*>(new CProcContour);
	//else if (type == _T("thresh"))		proc = static_cast<IImageProcessor*>(new CProcThresh);
	//else if (type == _T("hist"))		proc = static_cast<IImageProcessor*>(new CProcHist);
	//else if (type == _T("rotate_clip"))proc = static_cast<IImageProcessor*>(new CProcRotateClip);
	//else if (type == _T("barcode"))	proc = static_cast<IImageProcessor*>(new CProcBarCode);
	//else if (type == _T("split"))		proc = static_cast<IImageProcessor*>(new CProcSplit);
	//else if (type == _T("bright"))		proc = static_cast<IImageProcessor*>(new CProcBrightness);
	//else if (type == _T("roi"))		proc = static_cast<IImageProcessor*>(new CProcRoi);
	//else if (type == _T("contour_match"))		proc = static_cast<IImageProcessor*>(new CContourMatch);
	//else if (type == _T("match"))		proc = static_cast<IImageProcessor*>(new CProcMatch);

	else if (type == L"bounding")	proc = static_cast<IImageProcessor*>(jcvos::CDynamicInstance<CProcBoundingCircle>::Create());
	//else if (type == _T("correction"))	proc = static_cast<IImageProcessor*>(new CProcCorrection);
	else if (type == L"find_triangle")	proc = static_cast<IImageProcessor*>(jcvos::CDynamicInstance<CTestFindTriangle>::Create());
	//else if (type == _T("ferri"))	proc = static_cast<IImageProcessor*>(new CProcFerriRecognizer);
	else if (type == L"hough_circle")	proc = static_cast<IImageProcessor*>(jcvos::CDynamicInstance<CTestHoughCircle>::Create());
	else if (type == L"sobel")			proc = static_cast<IImageProcessor*>(jcvos::CDynamicInstance<CTestSobel>::Create());
	else if (type == L"binary")			proc = static_cast<IImageProcessor*>(jcvos::CDynamicInstance<CBinary>::Create());

	else
	{
		LOG_ERROR(L"processor: %s does not exist", type.c_str());
	}
}

bool CProcessorBoxBase::Load(const boost::property_tree::wptree & pt)
{
	const boost::property_tree::wptree processor_pt = pt.get_child(L"processor");
	m_type = processor_pt.get<std::wstring>(L"<xmlattr>.type");

	CreateImageProcessor(m_processor, m_type);
	if (m_processor == NULL) THROW_ERROR(ERR_APP, L"failed on creating processor type=%s", m_type.c_str());
	m_processor->SetBox(static_cast<IProcessorBox*>(this));

	auto param = processor_pt.get_child_optional(L"parameter");
	if (param) m_processor->SetProperties(*param);

	return true;
}

bool CProcessorBoxBase::Save(boost::property_tree::wptree & pt)
{
	pt.add(L"<xmlattr>.type", L"normal");
	pt.add(L"<xmlattr>.name", m_name);
	boost::property_tree::wptree param_pt;
	m_processor->GetProperties(param_pt);

	boost::property_tree::wptree proc_pt;
	proc_pt.add(L"<xmlattr>.type", m_type);
	proc_pt.add_child(L"parameter", param_pt);
	pt.add_child(L"processor", proc_pt);
	// inputs
	boost::property_tree::wptree input_pt;
	int src_num = m_processor->GetSourceNum();
	for (int ii = 0; ii < src_num; ++ii)
	{
		jcvos::auto_interface<IImageProcessor> src;
		m_processor->GetSource(ii, src);
		JCASSERT(src);
		int channel = m_processor->GetSourceChannel(ii);

		boost::property_tree::wptree src_pt;
		src_pt.add(L"<xmlattr>.name", src->GetProcessorName() );
		src_pt.add(L"<xmlattr>.channel", channel);
		input_pt.add_child(L"source", src_pt);
	}
	pt.add_child(L"inputs", input_pt);

	return true;
}
