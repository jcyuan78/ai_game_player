#pragma once

#include "../include/iimage_proc.h"
#include "process-box.h"
#include <map>

class CBoxManager
{
public:
	CBoxManager(void);
	~CBoxManager(void);

public:
	bool LoadFromFile(const std::wstring & fn);
	void SaveToFile(const std::wstring & fn);

	void GetSourceProcessor(IProcessorBox * & box);

	void GetProcessorBox(IProcessorBox * & box, int index)
	{
		JCASSERT(box == NULL);
		if (index >= m_box_list.size()) return;
		box = m_box_list[index];
		if (box) box->AddRef();
	}
	void Update(UINT from);
	size_t GetBoxNum(void) { return m_box_list.size(); }


protected:
	void Connect(IProcessorBox * dst_box, int input_id, const std::wstring & src_name)
	{
		JCASSERT(dst_box);
		// search for source proc box
		IProcessorBox * src_box = NULL;
		auto it = m_name_index.find(src_name);
		if (it==m_name_index.end()) THROW_ERROR(ERR_PARAMETER, L"not found processor %s", src_name.c_str());
		src_box = it->second;
		dst_box->ConnectTo(input_id, src_box);
	}

protected:
	std::vector<IProcessorBox *> m_box_list;
	std::map<std::wstring, IProcessorBox*> m_name_index;


};

