///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "max_min_search.h"

#include <strstream>
#include <sstream>

LOCAL_LOGGER_ENABLE(L"max_min_search", LOGGER_LEVEL_DEBUGINFO);


// 约数表，第一个为约数的个数，包括1和其自身，其后为所有约数
static const int appro[11][5] = {
	{0,},	//0
	{1, 1},	//1
	{2, 1, 2}, //2
	{2, 1, 3}, //3
	{3, 1, 2, 4}, //4
	{2, 1, 5}, //5
	{4, 1, 2, 3, 6}, //6
	{2, 1, 7}, //7
	{4, 1, 2, 4, 8}, //8
	{3, 1, 3, 9}, //9
	{4, 1, 2, 5, 10},
};

CSearchingNode::CSearchingNode(void)
{
	memset(m_children, 0, sizeof(CSearchingNode*) * 10);
	m_child_nr = 0;
	m_parent = nullptr;
	m_board = 0xFFFF;
	m_remain = 10;
	m_step = 0;
	m_player = 0;
	m_score = 0;
	m_depth = 0;
}

CSearchingNode::~CSearchingNode(void)
{
	LOG_DEBUG(L"delete: depth=%d, id=%lld", m_depth, m_id);
	for (BYTE ii = 0; ii < m_child_nr; ++ii) delete m_children[ii];
}

bool CSearchingNode::Play(BYTE v, BYTE player)
{
	if (m_board & (1 << v))
	{	// 可以选，移除v以及v的约数
		int n = appro[v][0];
		for (int ii = 1; ii <= n; ii++)
		{
			int a = appro[v][ii];
			// 移除约数
			WORD bit = 1 << a;
			if (m_board & bit)
			{
				m_board &= ~bit;
				m_remain--;
			}
		}
		m_step = v;
		m_player = player;
		if (m_player == 0)	{ m_board &= 0xFFFE; }
		else				{ m_board |= 1; }
		return true;
	}
	else return false;
}

void CSearchingNode::CloneFrom(const CSearchingNode& src)
{
	m_board = src.m_board;
	m_remain = src.m_remain;
}

void CSearchingNode::AddChild(CSearchingNode* child)
{
	JCASSERT(m_child_nr <10);
	m_children[m_child_nr] = child;
	m_child_nr++;
	child->m_parent = this;
	child->m_depth = m_depth + 1;
}

void CSearchingNode::PrintRemain(void) const
{
	std::wstring str;
	GetState(str);
	wprintf_s(str.c_str());
}

void CSearchingNode::GetState(std::wstring & str) const
{
	std::wstringstream ss;
	ss << L"{P"<<m_player<<L"(";
	for (BYTE ii = 1; ii <= 10; ++ii)
	{
		WORD mask = (1 << ii);
		if (m_board & mask) ss << ii/* << L", "*/;
		else ss << L" ";
	}
	ss <<L")="<<m_remain << L"}";
	str = ss.str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ==

CMaxMinSearch::CMaxMinSearch(void)
{
	memset(m_bitmaps, 0, sizeof(CSearchingNode*) * 2048);
	m_state_id = 0;

}

CMaxMinSearch::~CMaxMinSearch(void)
{
	for (auto it = m_open_list.begin(); it != m_open_list.end(); ++it)
	{
		delete (*it);
	}
}

char CMaxMinSearch::FindBestApproach(CSearchingNode& cur, BYTE & step)
{
	// 扩展子节点
	step = 0;
	BYTE best_step =0;
	BYTE player = cur.GetPlayer();
	player = 1 - player;
	UINT64 cur_state = m_state_id++;
	cur.m_id = cur_state;
//	m_state_id++;

	std::wstring str_state;
	cur.GetState(str_state);
	LOG_DEBUG(L"S[%lld]: %s, player=%d", cur_state, str_state.c_str(), player);

	// 扩展closed节点，插入closed之后。
	CSearchingNode* child = nullptr;// = new CSearchingNode;
	char best_score;
	if (player == 0) best_score = -1;
	else best_score = 1;
//	for (int ii = 10; ii >= 1; --ii)
	for (int ii = 1; ii <= 10; ++ii)
	{
		if (child == nullptr) child = new CSearchingNode;
		child->CloneFrom(cur);
		bool br = child->Play(ii, player);
		if (!br)
		{
			delete child;
			child = nullptr;
			continue;
		}
		child->GetState(str_state);
		LOG_DEBUG(L" [%lld] => play=%d, state=%s", cur_state, ii, str_state.c_str());
		// 检查状态
		WORD state_val = child->GetState(); JCASSERT(state_val < 2048);
		char score = 0;
		if (!m_bitmaps[state_val])
		{	//没有被搜索过，保存// 添加到close列表
			cur.AddChild(child);
			//检查是否定胜负
			if (child->GetRemain() == 0)
			{
				if (player == 0)
				{	// 胜
					child->SetScore(1);
					best_score = 1;
				}
				else
				{	// 负
					child->SetScore(-1);
					best_score = -1;
				}
				best_step = ii;
//				child = nullptr;
				break;	//不需要处理兄弟节点
			}
			//没有定胜负
			score = FindBestApproach(*child, best_step);
//			child = nullptr;
		}
		else
		{	// 已经被扩展过
			CSearchingNode* node = m_bitmaps[state_val];
			score = node->GetStore();
			node->GetState(str_state);
			LOG_DEBUG(L" [%lld] => existing state: %s, score=%d", cur_state, str_state.c_str(), score);
			delete child;
			child = node;
		}
//		LOG_DEBUG(L"Expand: play=%d, state=%s, score=%d", ii, str_state.c_str(), score);

		if (player == 0)
		{
			if (score == 1)
			{
				best_score = 1;
				best_step = ii;
//				wprintf_s(L"Solved: depth=%d, %s\n", child->m_depth, str_state.c_str());
				break;
			}
			if (score > best_score) best_score = score, best_step = ii;
		}
		else
		{
			if (score == -1)
			{
				best_score = -1;
				best_step = ii;
//				wprintf_s(L"Solved: depth=%d, %s\n", child->m_depth, str_state.c_str());
				break;
			}
			if (score < best_score) best_score = score, best_step = ii;
		}
		child = nullptr;

	}

//	delete child;
	child = nullptr;
	cur.SetScore(best_score);
	WORD state_val = cur.GetState(); JCASSERT(state_val < 2048);
	m_bitmaps[state_val] = &cur;

	LOG_DEBUG(L" [%lld]: score=%d, play=%d", cur_state, best_score, best_step);
	// 获取子节点的得分
	// 选择最佳子节点
	step = best_step;
	return best_score;
}


/*

int CMaxMinSearch::FindBestApproach(CSearchingNode& state)
{
	BYTE player = 0;		// player: 0: AI(自己），1: player(对手）
//	m_open_list.push_back(&state);
	CSearchingNode* node = new CSearchingNode;
	node->CloneFrom(state);
	m_open_list.push_back(node);
	auto closed = m_open_list.begin();
	// 深度搜索
	while (1)
	{
		CSearchingNode* cur = *closed;
		auto next = closed;
		next++;	//兄弟节点，当当前节点无法扩展时，跳到兄弟节点
		player = cur->GetPlayer();
		player = 1 - player;
		// 扩展closed节点，插入closed之后。
		CSearchingNode* child;// = new CSearchingNode;
		for (int ii = 1; ii <= 10; ++ii)
		{
			if (child == nullptr) child = new CSearchingNode;
			child->CloneFrom(*cur);
			bool br = child->Play(ii, player);
			if (br)
			{	// 检查状态
				WORD state = child->GetState(); JCASSERT(state < 2048);
				if (!m_bitmaps[state])
				{	//没有被搜索过，保存// 添加到close列表
					m_bitmaps[state] = child;
					cur->AddChild(child);
					m_open_list.insert(closed, child);
					//检查是否定胜负
					if (child->GetRemain() == 0)
					{
						if (player == 0)
						{	// 胜
							child->SetScore(1);
							cur->SetScore(1);
						}
						else
						{	// 负
							child->SetScore(-1);
							cur->SetScore(-1);
						}	
						closed = next;
						break;	//不需要处理兄弟节点
					}
					child = nullptr;
				}
				else
				{	// 已经被扩展过
					

				}

			}
			
		}
		delete child;
		// 如果已经走完，则评分并且回溯。
		closed++;

	}
	return 0;
}
*/