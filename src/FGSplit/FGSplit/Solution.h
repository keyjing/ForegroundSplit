#ifndef SOLUTION_H
#define SOLUTION_H

#include "FrameSet.h"

#define DILATION_SIZE           5       // 膨胀大小

#define FILL_MERGE_FG_AREA		20		// 合并后填充的前景空洞大小
#define	DEL_MERGE_FG_AREA		10		// 合并后抹除的前景斑点大小

class Solution
{

public:
    Solution(FrameSet& p) : fs(p), cnt(0), start_id(0) {};

    void Run();

    void setStartId(int k) { start_id = k; }


    void setShowed_vibe_fg(bool value);

    void setSave_vibe_fg(bool value);

    void setShowed_vibe_up(bool value);

    void setSave_vibe_up(bool value);

    void setShowed_ffd_fg(bool value);

    void setSave_ffd_fg(bool value);

    void setShowed_input(bool value);

    void setShowed_output(bool value);

    void setSave_output(bool value);

    void setShowed_result(bool value);

    void setShowed_merge(bool value);

    void setSave_merge(bool value);

    void setPart(int value);

    void setMsg_prt(bool value);

    void setMsg_save(bool value);

    void setRes_save(bool value);

    void setFile_name(const std::string& value);

private:
    cv::Mat MergeFG(cv::Mat vibe_fg, cv::Mat ffd_fg, cv::Mat& merge);		// 合并两种算法结果

    FrameSet& fs;
    int cnt;
    int start_id;

    bool showed_vibe_fg = false;
    bool save_vibe_fg = false;
    bool showed_vibe_up = false;
    bool save_vibe_up = false;
    bool showed_ffd_fg = false;
    bool save_ffd_fg = false;
    bool showed_input = false;
    bool showed_output = false;
    bool save_output = false;
    bool showed_result = false;
    bool showed_merge = false;
    bool save_merge = false;

    int part = 1;

    bool msg_prt = true;
    bool msg_save = true;
    bool res_save = true;

    std::string file_name = "name";
};

#endif // SOLUTION_H
