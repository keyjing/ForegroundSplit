#ifndef _Solution_h
#define _Solution_h

#include "FrameSet.h"

#define FILL_MERGE_FG_AREA		20		// 合并后填充的前景空洞大小
#define	DEL_MERGE_FG_AREA		10		// 合并后抹除的前景斑点大小

class Solution
{

public:
	Solution(FrameSet& p) : fs(p), cnt(0) {};

	void Run();

private:
	cv::Mat MergeFG(cv::Mat vibe_fg, cv::Mat ffd_fg);		// 合并两种算法结果

	FrameSet& fs;
	int cnt;
};


#endif _Solution_h
