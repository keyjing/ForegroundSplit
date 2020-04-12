#include<iostream>
#include "Solution.h"
#include "VideoTest.h"
#include "CDW_2014_Test.h"
#include <fstream>
#include <string>

// 配置文件所在路径
#define CONF_FILE_PATH "../../../fgspconf.txt"

using namespace std;

// 比较src首部是否包含dist，返回src中dist之后的字符下标
int cmpOptStr(const string& src, const string& dist)
{
    int len1 = src.length();
    int len2 = dist.length();
    if (len1 < len2 + 1)
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
    ifstream conf(CONF_FILE_PATH, ios::in);
    if (!conf.is_open())
    {
        cout << "ERROR: Open file \"" << CONF_FILE_PATH << "\" FAILED." << endl;
        return 1;
    }

    // 设置参数变量
    bool choice = false;
    bool cdw_qk = false;
    string path;
    bool showed_vibe_fg = false;
    bool save_vibe_fg = false;
    bool showed_vibe_up = false;
    bool save_vibe_up = false;
    bool showed_ffd = false;
    bool save_ffd = false;
    bool showed_input = false;
    bool showed_output = false;
    bool save_output = false;
    bool showed_result = false;
    int part = 0;
    bool msg_prt = false;
    string save_folder = "./";
    bool msg_save = false;
    bool res_save = false;

    string line;
    int k;

    // 读取参数
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
        else if ((k = cmpOptStr(line, "SHOWED_VIBE")) > 0)
        {
            showed_vibe_fg = (line[k] != '0');
            showed_vibe_up = (line[k] == '2');
        }
        else if ((k = cmpOptStr(line, "SAVE_VIBE")) > 0)
        {
            save_vibe_fg = (line[k] != '0');
            save_vibe_up = (line[k] == '2');
        }
        else if ((k = cmpOptStr(line, "SHOWED_FFD")) > 0)
            showed_ffd = (line[k] == '1');
        else if ((k = cmpOptStr(line, "SAVE_FFD")) > 0)
            save_ffd = (line[k] == '1');
        else if ((k = cmpOptStr(line, "SHOWED_INPUT")) > 0)
            showed_input = (line[k] == '1');
        else if ((k = cmpOptStr(line, "SHOWED_OUTPUT")) > 0)
            showed_output = (line[k] == '1');
        else if ((k = cmpOptStr(line, "SAVE_OUTPUT")) > 0)
            save_output = (line[k] == '1');
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
        else if ((k = cmpOptStr(line, "SAVE_FOLDER")) > 0)
            save_folder = line.substr(k, line.length() - k);
        else if ((k = cmpOptStr(line, "MSG_SAVE")) > 0)
            msg_save = (line[k] == '1');
        else if ((k = cmpOptStr(line, "RES_SAVE")) > 0)
            res_save = (line[k] == '1');
    }
    conf.close();

    // 对获得的参数进行设置
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
    s.setStartId(id);
    s.setShowed_vibe_fg(showed_vibe_fg);
    s.setSave_vibe_fg(save_vibe_fg);
    s.setShowed_vibe_up(showed_vibe_up);
    s.setSave_vibe_up(save_vibe_up);
    s.setShowed_ffd_fg(showed_ffd);
    s.setSave_ffd_fg(save_ffd);
    s.setShowed_input(showed_input);
    s.setShowed_output(showed_output);
    s.setSave_output(save_output);
    s.setShowed_result(showed_result);
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
        string command = "mkdir ";
        if (save_folder.length() != 0)
        {
            // linux下
            if (save_folder[0] == '.' || save_folder[0] == '/')
            {
                if (save_folder[save_folder.length() - 1] != '/')
                    save_folder += '/';
                command += "-p ";
                
            }
            // windows下
            else
            {
                for (int i = 0; i < save_folder.length(); ++i)
                    if (save_folder[i] == '/')
                        save_folder[i] = '\\';
                if (save_folder[save_folder.length() - 1] != '\\')
                    save_folder += '\\';
            }
        }
        command += save_folder + tmp;
        system(command.c_str());
        s.setFile_name(save_folder + tmp);
    }

    s.setStartId(id);

    // 开始执行
    s.Run();

    delete fs;

    return 0;
}
