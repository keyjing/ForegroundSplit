#ifndef SOLUTION_H
#define SOLUTION_H

#include "FrameSet.h"

#define FILL_MERGE_FG_AREA		20		// 合并后填充的前景空洞大小
#define	DEL_MERGE_FG_AREA		10		// 合并后抹除的前景斑点大小

class Solution
{

public:
    Solution(FrameSet& p) : fs(p), cnt(0), start_id(0) {};

    void Run();

    void setStartId(int k) { start_id = k; }

    void setShowed_viBe_fg(bool value);

    void setShowed_ffd_fg(bool value);

    void setShowed_input(bool value);

    void setShowed_output(bool value);

    void setShowed_res_fg(bool value);

    void setPart(int value);

    void setMsg_prt(bool value);

    void setMsg_save(bool value);

    void setRes_save(bool value);

    void setFile_name_msg(const std::string& value);

    void setFile_name_res(const std::string& value);

    void setShowed_vibe_up(bool value);

private:
    cv::Mat MergeFG(cv::Mat vibe_fg, cv::Mat ffd_fg);		// 合并两种算法结果

    FrameSet& fs;
    int cnt;
    int start_id;

    bool showed_viBe_fg = false;
    bool showed_vibe_up = false;
    bool showed_ffd_fg = false;
    bool showed_input = true;
    bool showed_output = true;
    bool showed_res_fg = false;

    int part = 1;

    bool msg_prt = true;
    bool msg_save = true;
    bool res_save = true;

    std::string file_name_msg = "msglog.txt";
    std::string file_name_res = "result.txt";
};

#endif // SOLUTION_H
