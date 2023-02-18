#include "pch.h"
#include "box_manager.h"
#include <boost/property_tree/xml_parser.hpp>

LOCAL_LOGGER_ENABLE(L"dialog", LOGGER_LEVEL_DEBUGINFO);


CBoxManager::CBoxManager(void)
{
}

CBoxManager::~CBoxManager(void)
{
	for (auto it = m_box_list.begin(); it != m_box_list.end(); ++it)
	{
		RELEASE(*it);
	}
}

bool CBoxManager::LoadFromFile(const std::wstring & fn)
{
	std::string str_fn;
	boost::property_tree::wptree root_pt;
	jcvos::UnicodeToUtf8(str_fn, fn);
	boost::property_tree::read_xml(str_fn, root_pt);
	boost::property_tree::wptree & list_pt= root_pt.get_child(L"boxes");
	for (auto it = list_pt.begin(); it != list_pt.end(); ++it)
	{
		if (it->first != L"box") continue;
		boost::property_tree::wptree & pt = it->second;
		const std::wstring & type = pt.get<std::wstring>(L"<xmlattr>.type");
		const std::wstring & name = pt.get<std::wstring>(L"<xmlattr>.name");
		jcvos::auto_interface<IProcessorBox> box;
		CProcessorBoxBase::CreateBox(box, type, name);
		if (box == NULL) THROW_ERROR(ERR_APP, L"failed on creating box, type=%s, name=%s", type.c_str(), name.c_str());
		box->Load(pt);
		box->AddRef();
		LOG_DEBUG(L"insert process box name=%s, type=%s, ptr=%p", name.c_str(), type.c_str(), box.m_ptr);
		m_box_list.push_back(box);
		//如果以box为make_pair的参数，会调用一次复制构建函数和一次析构，导致box被release一次。box不是smart prt
		// 或者显示申明make_pair的参数<string, IprocessorBox*>
		m_name_index.insert(std::make_pair(name, (IProcessorBox*)box));
		//配置连接
		auto & input_pt = pt.get_child_optional(L"inputs");
		if (!input_pt) continue;
		int inport = 0;
		for (auto ii = input_pt->begin(); ii != input_pt->end(); ++ii)
		{
			if (ii->first != L"source") continue;
			const std::wstring & src_name = ii->second.get<std::wstring>(L"<xmlattr>.name");
			int channel = ii->second.get<int>(L"<xmlattr>.channel");
			LOG_NOTICE(L"connect processor=%s, port=%d to source=%s, channel=%d", 
				name.c_str(), inport, src_name.c_str(), channel);
			Connect(box, channel, src_name);
			inport++;
		}
	}

	return true;
}

void CBoxManager::SaveToFile(const std::wstring & fn)
{
	std::string str_fn;
	jcvos::UnicodeToUtf8(str_fn, fn);

	boost::property_tree::wptree list_pt;

	for (auto it = m_box_list.begin(); it != m_box_list.end(); ++it)
	{
		JCASSERT(*it);
		boost::property_tree::wptree box_pt;
		(*it)->Save(box_pt);
		list_pt.add_child(L"box", box_pt);
		// 对连接部分的保存在 ProcessorBox中实现
	}
	boost::property_tree::wptree root_pt;
	root_pt.add_child(L"boxes", list_pt);
	boost::property_tree::write_xml(str_fn, root_pt);

}

void CBoxManager::GetSourceProcessor(IProcessorBox *& box)
{
	JCASSERT(box == NULL);
	box = m_box_list[0];
	if (box) box->AddRef();
}

void CBoxManager::Update(UINT from)
{
	size_t ii;
	try
	{
		size_t box_num = m_box_list.size();
		for (ii = from; ii < box_num; ++ii)
		{
			m_box_list[ii]->OnUpdateBox();
		}
	}
	catch (std::exception & err)
	{
		std::wstring str_err;
		jcvos::Utf8ToUnicode(str_err, err.what());
		wchar_t str[256];
		swprintf_s(str, L"calculatio error at box[%d](%s) %s", ii, m_box_list[ii]->GetName().c_str(), str_err.c_str());
//		LOG_ERROR(L"[err] calculatio error at box[%d](%s) %s", ii, m_box_list[ii]->GetName().c_str, str_err.c_str());
		LOG_ERROR(L"[err] %s", str);
		MessageBox(NULL, str, L"Calculation Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
	}
}