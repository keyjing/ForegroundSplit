#include<iostream>
#include "Solution.h"
#include "VideoTest.h"
#include "CDW_2014_Test.h"
#include <fstream>
#include <string>

#define CONF_FILE_PATH "../../../fgspconf.txt"

using namespace std;

int cmpOptStr(const string& src, const string& dist)
{
    int len1 = src.length();
    int len2 = dist.length();
    if (len1 < len2 + 2)
        return -1;
    for (int i = 0; i < len2; ++i)
    {
        if (src[i] != dist[i])
            return -1;
    }
    if (src[len2] != '=')
        return -1;
    return len2 + 1;
}

int main()
{
    // 读取配置文件
    ifstream conf(CONF_FILE_PATH, ios::in);
    if (!conf.is_open())
    {
        cout << "ERROR: Open file \"" << CONF_FILE_PATH << "\" FAILED." << endl;
        return 1;
    }

    bool choice = false;
    bool cdw_qk = false;
    string path;
    bool showed_vide_fg = false;
    bool showed_vibe_up = false;
    bool showed_ffd = false;
    bool showed_input = false;
    bool showed_output = false;
    bool showed_result = false;
    int part = 0;
    bool msg_prt = false;
    bool msg_save = false;
    bool res_save = false;

    string line;
    int k;
    // 获取配置
    while (getline(conf, line))
    {
        if (line.length() == 0 || line[0] == '#')
            continue;
        if ((k = cmpOptStr(line, "CHOICE_CDW")) > 0)
            choice = (line[k] == '1');
        else if ((k = cmpOptStr(line, "PATH")) > 0)
            path = line.substr(k, line.length() - k);
        else if ((k = cmpOptStr(line, "CDW_QK")) > 0)
            cdw_qk = (line[k] == '1');
        else if ((k = cmpOptStr(line, "SHOWED_VIDE")) > 0)
        {
            showed_vide_fg = (line[k] != '0');
            showed_vibe_up = (line[k] == '2');
        }
        else if ((k = cmpOptStr(line, "SHOWED_FFD")) > 0)
            showed_ffd = (line[k] == '1');
        else if ((k = cmpOptStr(line, "SHOWED_INPUT")) > 0)
            showed_input = (line[k] == '1');
        else if ((k = cmpOptStr(line, "SHOWED_OUTPUT")) > 0)
            showed_output = (line[k] == '1');
        else if ((k = cmpOptStr(line, "SHOWED_RESULT")) > 0)
            showed_result = (line[k] == '1');
        else if ((k = cmpOptStr(line, "PARTITION")) > 0)
        {
            part = 0;
            for (; k < line.length(); ++k)
            {
                if (line[k] >= '0' && line[k] <= '9')
                    part = part * 10 + (line[k] - '0');
            }
        }
        else if ((k = cmpOptStr(line, "MSG_PRT")) > 0)
            msg_prt = (line[k] == '1');
        else if ((k = cmpOptStr(line, "MSG_SAVE")) > 0)
            msg_save = (line[k] == '1');
        else if ((k = cmpOptStr(line, "RES_SAVE")) > 0)
            res_save = (line[k] == '1');
    }
    conf.close();

    // 进行配置
    int id = 0;
    FrameSet* fs;
    if (choice)
    {
        CDW_2014_Test* cdw = new CDW_2014_Test(path);
        if (cdw_qk)
            id = cdw->FromStart(-2);
        fs = cdw;
    }
    else
        fs = new VideoTest(path);

    Solution s(*fs);
    s.setShowed_viBe_fg(showed_vide_fg);
    s.setShowed_vibe_up(showed_vibe_up);
    s.setShowed_ffd_fg(showed_ffd);
    s.setShowed_input(showed_input);
    s.setShowed_output(showed_output);
    s.setShowed_res_fg(showed_result);
    s.setPart(part);
    s.setMsg_prt(msg_prt);
    s.setMsg_save(msg_save);
    s.setRes_save(res_save);

    int start, end;
    start = end = k = path.length();
    for (k -= 1; k >= 0; --k)
    {
        if (path[k] == '.')
            end = k;
        else if (path[k] == '/')
        {
            start = k + 1;
            break;
        }
    }
    if (start != end)
    {
        string tmp = path.substr(start, end - start);
        s.setFile_name_msg("./" + tmp + "_msg.txt");
        s.setFile_name_res("./" + tmp + "_msg.txt");
    }
    s.setStartId(id);
    s.Run();

    delete fs;

    return 0;
}
