#include "stdafx.h"

#include "marubatu_board.h"

LOCAL_LOGGER_ENABLE(_T("marubats"), LOGGER_LEVEL_DEBUGINFO);


CMaruBatuBoard::CMaruBatuBoard(void)
{
}

CMaruBatuBoard::~CMaruBatuBoard(void)
{
}
	
void CMaruBatuBoard::Initialize(void)
{
	memset(m_board, 0, BOARD_SIZE_2);
	m_play_x = 0, m_play_y=0;
	m_parent=NULL;
	m_level = 0;
	m_win = 0;
}

CMaruBatuBoard & CMaruBatuBoard::operator = (const CMaruBatuBoard & node)
{
	memcpy_s(m_board, BOARD_SIZE_2, node.m_board, BOARD_SIZE_2);
	m_play_x = node.m_play_x;
	m_play_y = node.m_play_y;
	m_parent = const_cast<CMaruBatuBoard*>(&node);
	m_level = node.m_level +1;
	m_win = 0;
	return (*this);
}

bool CMaruBatuBoard::Play(BYTE x, BYTE y, CELL_TYPE c)
{
	BYTE & cell=Cell(x, y);
	if (cell != CELL_EMPTY) return false;
	cell = c;
	m_play_x = x, m_play_y=y;
	return true;
}

BYTE CMaruBatuBoard::IsWin(void)
{
	for (size_t yy=0; yy<BOARD_SIZE; ++yy)
	{
		BYTE cell0=Cell(0, yy);
		if (cell0!=0 && Cell(1, yy) == cell0 && Cell(2,yy) == cell0) return cell0;
	}

	for (size_t xx=0; xx<BOARD_SIZE; ++xx)
	{
		BYTE cell0=Cell(xx, 0);
		if (cell0!=0 && Cell(xx, 1) == cell0 && Cell(xx,2) == cell0) return cell0;
	}

	BYTE cell0=Cell(1, 1);
	if (cell0!=0 && cell0 == Cell(0,0) && cell0==Cell(2,2)) return cell0;
	if (cell0!=0 && cell0 == Cell(0,2) && cell0==Cell(2,0)) return cell0;

	return CELL_EMPTY;
}

void CMaruBatuBoard::OutBoard(void)
{
	wchar_t str[BOARD_SIZE +1];
	for (size_t yy=0; yy<BOARD_SIZE; ++yy)
	{
		memset(str, 0, BOARD_SIZE +1);
		for (size_t xx=0; xx<BOARD_SIZE; ++xx)
		switch (Cell(xx, yy))
		{
		case CELL_EMPTY: str[xx]=L' ';	break;
		case CELL_MARU: str[xx]=L'O';	break;
		case CELL_BATU: str[xx]=L'X';	break;
		}
		str[BOARD_SIZE]=0;
		LOG_DEBUG(L"%s", str);
	}
}
