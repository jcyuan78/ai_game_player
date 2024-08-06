///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <boost/property_tree/json_parser.hpp>

#define MAX_STONE 64

// 棋盘的配置，包括棋盘大小。下棋规则，最多/最少取子。
// 减少board复制时的内存消耗
struct BOARD_CONFIG
{
	int board_size;		// 棋盘大小
	int min_remove;		// 最少取走石子个数
	int max_remove;		// 最多取走石子个数
};

// 石子的取法
struct MOVEMENT
{
	int start;			// 从第几个石子开始取
	int num;			// 取走的石子数量
};

class CBoard
{
public:
	CBoard(BOARD_CONFIG * config);
	~CBoard() {};

public:
	void Init(void);
	bool Play(MOVEMENT& mv);					// 当mv不合法时，返回false；
	// 调用者保证mvs的大小足够大
	size_t EnumateMovement(MOVEMENT* mvs);
	static void LoadConfig(BOARD_CONFIG& config, boost::property_tree::wptree& pt);
	void DrawBoard(void);
	bool IsWin(void);

protected:
//	char m_board[MAX_STONE];
	UINT64 m_board;
	int m_stone_nr;				// 剩余石子数量，剩余0时胜利。
	BOARD_CONFIG* m_config;
};
