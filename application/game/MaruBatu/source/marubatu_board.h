#pragma once

#define BOARD_SIZE	(3)
#define BOARD_SIZE_2	(BOARD_SIZE*BOARD_SIZE)

class CMaruBatuBoard
{
public:
	enum CELL_TYPE {CELL_EMPTY=0, CELL_MARU=1, CELL_BATU=2};

public:
	CMaruBatuBoard(void);
	virtual ~CMaruBatuBoard(void);

public:
	void Initialize(void);
	bool Play(BYTE x, BYTE y, CELL_TYPE c);
	BYTE IsWin(void);
	inline BYTE & Cell(BYTE x, BYTE y) {return m_board[x+y*BOARD_SIZE];};

	CMaruBatuBoard & operator = (const CMaruBatuBoard & node);	// Copy Node
	//bool IsGoal(void);
	inline int Value(void) {return m_level/* + m_heuristic*/;}
	//UINT32 Hash(void);
	//bool ExpandFirst(CSixteenBoard* child);		// 从自身扩展第一个子节点
	//bool ExpandNext(CSixteenBoard* brother);	// 从父接点扩展下一个兄弟节点
	//bool IsSame(CSixteenBoard * node);

	virtual void OutBoard(void);

protected:
	BYTE m_board[BOARD_SIZE_2];	// 棋盘
	BYTE m_play_x, m_play_y;	// 下棋位置
	CMaruBatuBoard * m_parent;

	BYTE m_level;

public:
	float m_win;

};

//class CMonteCarlo
//{
//
//};