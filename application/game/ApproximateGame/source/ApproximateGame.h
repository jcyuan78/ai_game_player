///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <jcapp.h>

class CApproximateApp
	: public jcvos::CJCAppSupport<jcvos::AppArguSupport>
{
protected:
	typedef jcvos::CJCAppSupport<jcvos::AppArguSupport> BaseAppClass;

public:
	static const TCHAR LOG_CONFIG_FN[];
	CApproximateApp(void);
	virtual ~CApproximateApp(void);

public:
	virtual int Initialize(void);
	virtual int Run(void);
	virtual void CleanUp(void);
	virtual LPCTSTR AppDescription(void) const {
		return L"File System Tester, by Jingcheng Yuan\n";
	};

public:
	std::wstring m_config_file;
	std::wstring m_mount;
	std::wstring m_volume_name;
	std::wstring m_log_fn;
	bool m_unmount;

	size_t m_test_depth;


};


