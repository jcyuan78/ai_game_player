#pragma once

#include <jcapp.h>
//#include "a_star_search.h"

class CMaruBatuApp : public jcvos::CJCAppSupport<jcvos::AppArguSupport>
{
protected:
	typedef jcvos::CJCAppSupport<jcvos::AppArguSupport> BaseAppClass;

public:
	static const TCHAR LOG_CONFIG_FN[];
	CMaruBatuApp(void);

public:
	virtual int Initialize(void);
	virtual int Run(void);
	virtual void CleanUp(void);
	virtual LPCTSTR AppDescription(void) const {
		return _T("Maru-Batsu Game, by Jingcheng Yuan\n");
	};

protected:

	//void PrintSteps(CBWNode * res);
	//void Search(size_t pair);


public:
	size_t m_seed, m_playout;
};

