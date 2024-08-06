#include "pch.h"
#include <jcapp.h>
#include <boost/property_tree/json_parser.hpp>

#include "../include/board.h"

LOCAL_LOGGER_ENABLE(_T("blackwhite.app"), LOGGER_LEVEL_DEBUGINFO);

class CStoneGameApp : public jcvos::CJCAppSupport<jcvos::AppArguSupport>
{
protected:
	typedef jcvos::CJCAppSupport<jcvos::AppArguSupport> BaseAppClass;

public:
	static const TCHAR LOG_CONFIG_FN[];
	CStoneGameApp(void);

public:
	virtual int Initialize(void);
	virtual int Run(void);
	virtual void CleanUp(void);
	virtual LPCTSTR AppDescription(void) const {
		return _T("Perceptron, by Jingcheng Yuan\n");
	};

protected:

public:
	size_t m_pair;
	std::wstring m_config_fn;
};

const TCHAR CStoneGameApp::LOG_CONFIG_FN[] = _T("blackwhite.cfg");
typedef jcvos::CJCApp<CStoneGameApp>	CApplication;
CApplication _app;

#define _class_name_	CApplication
BEGIN_ARGU_DEF_TABLE()
ARGU_DEF(L"pair", 'p', m_pair, L"pair of stones, default=5", size_t(5))
ARGU_DEF(L"config", 'c', m_config_fn, L"pair of stones, default=5")
END_ARGU_DEF_TABLE()

CStoneGameApp::CStoneGameApp(void)
{
}

int CStoneGameApp::Initialize(void)
{
	return 0;
}

void CStoneGameApp::CleanUp(void)
{
#ifdef _DEBUG
	getc(stdin);
#endif
}

int CStoneGameApp::Run(void)
{
	// load config
	std::string str_fn;
	jcvos::UnicodeToUtf8(str_fn, m_config_fn);
	boost::property_tree::wptree pt;
	boost::property_tree::read_json(str_fn, pt);
	BOARD_CONFIG config;
	CBoard::LoadConfig(config, pt);
	// play game
	CBoard* board = new CBoard(&config);
	board->Init();
	size_t max_movement_nr = config.board_size * config.max_remove;
	MOVEMENT *mv = new MOVEMENT[max_movement_nr];
	bool win = false;
	while (1)
	{
		int player = 0;
		for (player = 0; player < 2; player++)
		{
			board->DrawBoard();
			size_t mv_nr = board->EnumateMovement(mv);
			wprintf_s(L"Player: %d, select the movement\n", player);
			for (size_t ii = 0; ii < mv_nr; ++ii)
			{
				wprintf_s(L"[%02zd](%d,%d); ", ii, mv[ii].start, mv[ii].num);
			}
			wprintf_s(L"\n");
			int mv_index = 0;
			wscanf_s(L"%d", &mv_index);
			board->Play(mv[mv_index]);

			if (board->IsWin())
			{
				wprintf_s(L"Player %d win the game\n", player);
				break;
			}
		}
		if (win) break;
	}
	delete[] mv;
	return 0;
}

int _tmain(int argc, _TCHAR * argv[])
{
	return jcvos::local_main(argc, argv);
}
