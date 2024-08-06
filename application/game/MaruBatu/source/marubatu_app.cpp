#include "stdafx.h"
#include <vld.h>
#include <time.h>

#include "marubatu_app.h"
#include "marubatu_board.h"
//#include "../../../puzzle/AStarSearch/a_star_search.h"

LOCAL_LOGGER_ENABLE(_T("blackwhite.app"), LOGGER_LEVEL_DEBUGINFO);

const TCHAR CMaruBatuApp::LOG_CONFIG_FN[] = _T("blackwhite.cfg");
typedef jcvos::CJCApp<CMaruBatuApp>	CApplication;
CApplication _app;

#define _class_name_	CApplication
BEGIN_ARGU_DEF_TABLE()
	ARGU_DEF(_T("play"),	'p', m_playout,	_T("playout"), size_t(100) )
	ARGU_DEF(_T("seed"),	's', m_seed,	_T("seed"), UINT(0) )
END_ARGU_DEF_TABLE()

CMaruBatuApp::CMaruBatuApp(void)
{
}

int CMaruBatuApp::Initialize(void)
{
	return 0;
}

void CMaruBatuApp::CleanUp(void)
{
#ifdef _DEBUG
	getc(stdin);
#endif
}

static const BYTE PLAY[3][2] = {
	{0, 0}, {1, 0}, {1, 1},
};


int CMaruBatuApp::Run(void)
{
	CMaruBatuBoard boards[20];
	for (size_t kk=0; kk<3; ++kk)
	{
		boards[0].Initialize();
		CMaruBatuBoard::CELL_TYPE player=CMaruBatuBoard::CELL_MARU;

		if (m_seed==0)	m_seed=(UINT)time(0);
		srand(m_seed);

		boards[0].Play(PLAY[kk][0], PLAY[kk][1], player);

		for (size_t nn=0; nn<m_playout; ++nn)
		{
			player = CMaruBatuBoard::CELL_MARU;
			BYTE win=0;
			size_t ii;
			for (ii=1; ii<9; ++ii)
			{
				boards[ii] = boards[ii-1];

				if (player == CMaruBatuBoard::CELL_MARU)	player = CMaruBatuBoard::CELL_BATU;
				else										player = CMaruBatuBoard::CELL_MARU;
				while (1)
				{
					WORD r=rand();
					BYTE x = LOBYTE(r) % BOARD_SIZE, y = HIBYTE(r) % BOARD_SIZE;
					if ( boards[ii].Play(x, y, player) ) break;
				}

				win=boards[ii].IsWin();
				if (win!=0) break;
			}
			boards[ii].OutBoard();
			LOG_DEBUG(L"win=%d", win);

			if (win == CMaruBatuBoard::CELL_MARU) boards[0].m_win +=1;
			else if (win == CMaruBatuBoard::CELL_EMPTY)	boards[0].m_win +=0.5;
		}
		wprintf_s(L"win rate of (%d,%d) = %.2f %%\n", PLAY[kk][0], PLAY[kk][1], boards[0].m_win*100 / m_playout);
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	return jcvos::local_main(argc, argv);
}

