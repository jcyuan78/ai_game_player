///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "ApproximateGame.h"

#include "max_min_search.h"

LOCAL_LOGGER_ENABLE(L"fstester.app", LOGGER_LEVEL_DEBUGINFO);

const TCHAR CApproximateApp::LOG_CONFIG_FN[] = L"fstester.cfg";
typedef jcvos::CJCApp<CApproximateApp>	CApplication;
CApplication _app;

#define _class_name_	CApplication
BEGIN_ARGU_DEF_TABLE()

ARGU_DEF(L"format", 'f', m_volume_name, L"format device using volume name.")
ARGU_DEF(L"mount", 'm', m_mount, L"mount point.")
ARGU_DEF(L"unmount", 'u', m_unmount, L"unmount device.", false)
ARGU_DEF(L"config", 'c', m_config_file, L"configuration file name")
ARGU_DEF(L"log_file", 'l', m_log_fn, L"specify test log file name")
END_ARGU_DEF_TABLE()

int _tmain(int argc, _TCHAR* argv[])
{
	return jcvos::local_main(argc, argv);
}

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType);

CApproximateApp::CApproximateApp(void)
{
}

CApproximateApp::~CApproximateApp(void)
{
}

int CApproximateApp::Initialize(void)
{
	//	EnableSrcFileParam('i');
	EnableDstFileParam('o');
	return 0;
}

void CApproximateApp::CleanUp(void)
{
}


int CApproximateApp::Run(void)
{
	CSearchingNode board;
	int player;
	do
	{
		wprintf_s(L"who first? 1:AI, 0:Player  ");
		wscanf_s(L"%d", &player);
		if (player == 0 || player == 1) break;
		wprintf_s(L"wrong value, input 1 or 0\n");
	} while (1);

	board.SetPlayer(player);
	player = 1 - player;

	int ii = 0;
	std::wstring str_state;
	while (1)
	{
		if (player == 0)
		{
			// AI play: search
			CMaxMinSearch search;
			CSearchingNode next_board;
			next_board.CloneFrom(board);
			next_board.SetPlayer(1);
			char score;
			BYTE step = 0;
			score = search.FindBestApproach(next_board, step);
			// play
			board.Play(step, 0);
			board.GetState(str_state);
//			wprintf_s(L"[+%d], play=%d, %s, remain=%d, score=%d,", ii, step, str_state.c_str(), board.GetRemain(), score);
			wprintf_s(L"[+%d], play=%d, %s, score=%d\n", ii, step, str_state.c_str(), score);
//			board.PrintRemain();
//			wprintf_s(L"\n");
			if (board.GetRemain() == 0)
			{
				wprintf_s(L"I win.\n");
				break;
			}
		}
		else
		{
			wprintf_s(L"Your turn input: ");
			// player
			BYTE pp;
			int _pp;
			do
			{
				wscanf_s(L"%d", &_pp);
				pp = (BYTE)_pp;
//				wprintf_s(L"\n");
				bool br = board.Play(pp, 1);
				if (br) break;
				wprintf_s(L"%d is unavailable. input again \n", pp);
			} while (1);
			board.GetState(str_state);
//			wprintf_s(L"[-%d], play=%d, %s, remain=%d", ii, pp, str_state.c_str()/*, board.GetRemain()*/);
			wprintf_s(L"[-%d], play=%d, %s\n", ii, pp, str_state.c_str()/*, board.GetRemain()*/);
//			board.PrintRemain();
//			wprintf_s(L"\n");
			if (board.GetRemain() == 0)
			{
				wprintf_s(L"You win.\n");
				break;
			}
		}
		player = 1 - player;
		ii++;
	}
	getchar();

	return 0;
}

