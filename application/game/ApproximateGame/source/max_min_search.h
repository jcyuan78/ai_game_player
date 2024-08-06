///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <stdext.h>
#include <list>

class CSearchingNode
{
public:
	CSearchingNode(void);
	~CSearchingNode(void);
	// 挑选一个数。如果可选，则修改并返回true；否则返回false
	bool Play(BYTE v, BYTE player);

	void CloneFrom(const CSearchingNode& src);
	BYTE GetPlayer(void) const { return m_player; }
	void SetPlayer(BYTE player) { m_player = player; }
	WORD GetState(void) const { return (m_board & 0x7FF); }
	void AddChild(CSearchingNode* child);
	BYTE GetRemain(void) const { return m_remain; }
	void SetScore(char s) { m_score = s; }
	char GetStore(void) const { return m_score; }
	void PrintRemain(void) const;
	void GetState(std::wstring& str) const;

public:
	// for debug
	UINT64 m_id;
	BYTE m_depth;


protected:
	CSearchingNode* m_parent;
	CSearchingNode* m_children[10];
	// 用BIT表示被写入的数值。最后以为表示游戏者。
	WORD m_board;
	// 还可写的数值个数
	BYTE m_remain;
	// 写出的数，如何从上一步到达这一步的。
	BYTE m_step;	
	// 这一步时谁下的。
	BYTE m_player;
	BYTE m_child_nr;	//子节点的个数
	char m_score;		//节点分数。由于打算搜索到最后，1为胜，-1为负，0未设置
};

class CMaxMinSearch
{
public:
	CMaxMinSearch(void);
	~CMaxMinSearch(void);
public:
	void InitState(CSearchingNode& init_node);
	// 对于当前状态，找到最佳走法。
	char FindBestApproach(CSearchingNode& state, BYTE & step);

protected:
	std::list<CSearchingNode*> m_open_list;
	CSearchingNode* m_bitmaps[2048];	// 前10位表示数字状态，最后以为表示棋手。
	UINT64 m_state_id;

};
