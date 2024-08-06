///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "pch.h"

#include "../include/board.h"

CBoard::CBoard(BOARD_CONFIG* config)
{
	m_config = config;
}

void CBoard::Init(void)
{
	m_stone_nr = m_config->board_size;
//	m_board = (UINT64)(-1);
	m_board = ((UINT64)1 << m_config->board_size);
	m_board--;
}

bool CBoard::Play(MOVEMENT& mv)
{
	// 符合游戏设定条件
	if (mv.num < m_config->min_remove || mv.num > m_config->max_remove) return false;
	// 可取走的石子不能超过现有石子数量
	if (mv.num > m_stone_nr) return false;
	int start = mv.start - 1;
	int end = start + mv.num;
	if (end > m_config->board_size) return false;

	UINT64 mask = ((UINT64)1 << (mv.num));
	mask--;
	mask <<= start;
	if ((m_board & mask) != mask) return false;
	m_board &= (~mask);
	m_stone_nr -= mv.num;
	return true;
}

size_t CBoard::EnumateMovement(MOVEMENT* mvs)
{
	size_t index = 0;
	for (int jj = m_config->min_remove; jj <= m_config->max_remove; ++jj)
	{
		UINT64 mask = (1 << (jj));
		mask--;
		for (int ii = 0; ii <= (m_config->board_size - jj); ++ii)
		{
			if ((m_board & mask) == mask)
			{
				mvs[index].start = ii + 1;
				mvs[index].num = jj;
				index++;
			}
			mask <<= 1;
		}
	}
	return index;
}

void CBoard::LoadConfig(BOARD_CONFIG& config, boost::property_tree::wptree& pt)
{
	config.board_size = pt.get<int>(L"stone_num", 12);
	config.min_remove = pt.get<int>(L"remove_min", 2);
	config.max_remove = pt.get<int>(L"remove_max", 4);
}

void CBoard::DrawBoard(void)
{
	wprintf_s(L"==");
	for (int ii = 0; ii < m_config->board_size; ii+=2)
	{
		wprintf_s(L"%d ", (ii+1) % 10);
	}
	wprintf_s(L"\n  ");
	UINT64 mask = 1;
	for (int ii = 0; ii < m_config->board_size; ++ii)
	{
		if (m_board & mask) wprintf_s(L"O");
		else wprintf_s(L"_");
		mask <<= 1;
	}
	wprintf_s(L"\n");
}

bool CBoard::IsWin(void)
{
	return (m_stone_nr == 0);
}
