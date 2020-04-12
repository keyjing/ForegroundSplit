#include "Solution.h"

#include <iostream>
#include "ViBePlus.h"
#include "FiveFrameDiff.h"
#include <queue>
#include <opencv2/imgproc/types_c.h>
#include <iomanip>
#include <fstream>

using namespace std;
using namespace cv;

void CalNewAve(double& ave, double val, int n)
{
    ave = (ave * (n - 1) + val) / n;
}

void Solution::Run()
{
    ofstream msgfile, resfile;
    if (msg_save)
    {
        msgfile = ofstream(file_name + "/msglog.txt", ios::out);
        if (!msgfile)
        {
            cout << "ERROR: Can not create folder \"" + file_name + "\"" << endl;
            return;
        }
    }

    Mat frame_in, frame_res;

    ViBePlus vibeplus;
    FiveFrameDiff ffd;

    // 五帧差分法前两帧不输出
    for (int i = 0; i < 2; ++i)
    {
        frame_in = fs.getNextInput();
        if (frame_in.empty())
            return;
        ffd.Run(frame_in);
    }
    // 第二帧 Vibe+建模
    vibeplus.Run(frame_in);

    // 结果帧跳过前两帧
    fs.getNextResult(2);
    start_id += 2;

    // 保存第三、四帧，五帧差分法需要提前知道下两帧
    queue<Mat> inque, resque;
    for (int i = 0; i < 2; ++i)
    {
        frame_in = fs.getNextInput();
        inque.push(frame_in);
        ffd.Run(frame_in);

        frame_res = fs.getNextResult();
        resque.push(frame_res);
    }

    // 程序运行时间统计变量
    double start, time_vibe, time_ffd, time_merge;
    double time_ave_vibe, time_ave_ffd, time_ave_merge;
    time_ave_vibe = time_ave_ffd = time_ave_merge = 0;

    // ViBe+
    double Pr_vibe, Re_vibe, F1_vibe;
    double Pr_ave_vibe, Re_ave_vibe, F1_ave_vibe;
    Pr_ave_vibe = Re_ave_vibe = F1_ave_vibe = 0;

    // ffd
    double Pr_ffd, Re_ffd, F1_ffd;
    double Pr_ave_ffd, Re_ave_ffd, F1_ave_ffd;
    Pr_ave_ffd = Re_ave_ffd = F1_ave_ffd = 0;

    // merge
    double Pr_merge, Re_merge, F1_merge;
    double Pr_ave_merge, Re_ave_merge, F1_ave_merge;
    Pr_ave_merge = Re_ave_merge = F1_ave_merge = 0;

    int part_cnt = part;
    while (!inque.empty())
    {
        ++cnt;
        // vibe+结果、ffd结果、最终结果、当前处理输入帧、当前处理结果帧
        Mat vibe_fg, ffd_fg, fg, merge, input, result;

        //======  ViBe+ START  =======//
        start = static_cast<double>(getTickCount());

        // 从队列获取当前处理输入帧、结果帧
        inque.front().copyTo(input);
        inque.pop();
        if (input.empty())
            break;
        resque.front().copyTo(result);
        resque.pop();

        // Vibe+从当前输入处理
        vibe_fg = vibeplus.Run(input);

        time_vibe = ((double)getTickCount() - start) / getTickFrequency() * 1000;
        //======  ViBe+ MESSAGE  =======//
        // 计算平局值
        CalNewAve(time_ave_vibe, time_vibe, cnt);
        if (!result.empty() && ForegroundCompare(vibe_fg, result, Pr_vibe, Re_vibe, F1_vibe))
        {
            // 计算平局值
            CalNewAve(Pr_ave_vibe, Pr_vibe, cnt);
            CalNewAve(Re_ave_vibe, Re_vibe, cnt);
            CalNewAve(F1_ave_vibe, F1_vibe, cnt);

        }
        //======  Vibe+ END =======//

        //======  ffd START  =======//
        start = static_cast<double>(getTickCount());

        // ffd从后两帧处理
        frame_in = fs.getNextInput();
        if (!frame_in.empty())
        {
            ffd_fg = ffd.Run(frame_in);
            inque.push(frame_in);

            frame_res = fs.getNextResult();
            resque.push(frame_res);
        }

        time_ffd = ((double)getTickCount() - start) / getTickFrequency() * 1000;
        //======  ffd MESSAGE  =======//
        // 计算平局值
        CalNewAve(time_ave_ffd, time_ffd, cnt);
        if (!result.empty() && ForegroundCompare(ffd_fg, result, Pr_ffd, Re_ffd, F1_ffd))
        {
            // 计算平局值
            CalNewAve(Pr_ave_ffd, Pr_ffd, cnt);
            CalNewAve(Re_ave_ffd, Re_ffd, cnt);
            CalNewAve(F1_ave_ffd, F1_ffd, cnt);
        }
        //======  Vibe+ END =======//

        //======  merge START  =======//
        start = static_cast<double>(getTickCount());

        fg = MergeFG(vibe_fg, ffd_fg, merge);

        time_merge = ((double)getTickCount() - start) / getTickFrequency() * 1000;
        time_merge += time_vibe + time_ffd;
        //======  merge MESSAGE  =======//
        // 计算平局值
        CalNewAve(time_ave_merge, time_merge, cnt);
        if (!result.empty() && ForegroundCompare(fg, result, Pr_merge, Re_merge, F1_merge))
        {
            // 计算平局值
            CalNewAve(Pr_ave_merge, Pr_merge, cnt);
            CalNewAve(Re_ave_merge, Re_merge, cnt);
            CalNewAve(F1_ave_merge, F1_merge, cnt);
        }
        //======  merge END =======//

        if (showed_input && !input.empty())
            imshow("输入", input);

        if (showed_result && !result.empty())
            imshow("对比", result);

        if (showed_vibe_fg && !vibe_fg.empty())
            imshow("vibe前景蒙版", vibe_fg);

        if (showed_vibe_up)
            imshow("更新蒙版", vibeplus.getUpdateModel());

        if (showed_ffd_fg && !ffd_fg.empty())
            imshow("帧差前景蒙版", ffd_fg);

        if (showed_merge && !merge.empty())
            imshow("交运算蒙版", merge);

        if (showed_output && !fg.empty())
            imshow("最终前景蒙版", fg);

        cv::waitKey(25);

        // 计数刷新
        if (--part_cnt > 0)
            continue;
        part_cnt = part;

        // 消息输出保存
        stringstream ntos;
        ntos << cnt + start_id;

        if (save_vibe_fg && !vibe_fg.empty())
            imwrite(file_name + "/" + ntos.str() + "_vibe_fg.jpg", vibe_fg);

        if (save_vibe_up)
            imwrite(file_name + "/" + ntos.str() + "_vibe_up.jpg", vibeplus.getUpdateModel());

        if (save_ffd_fg && !ffd_fg.empty())
            imwrite(file_name + "/" + ntos.str() + "_ffd.jpg", ffd_fg);

        if (save_merge && !merge.empty())
            imwrite(file_name + "/" + ntos.str() + "_merge.jpg", merge);

        if (save_output && !fg.empty())
            imwrite(file_name + "/" + ntos.str() + "_output.jpg", fg);


        stringstream ss;
        ss << "======================================================\n";
        // PRINT IMAGE NUMBER
        ss << "NO: ";
        ss << setw(4) << (cnt + start_id) << "  TIME[ms]      Pr[%]      Re[%]      F1[%]\n";

        ss << "ViBe+   ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << time_vibe << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Pr_vibe << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Re_vibe << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << F1_vibe << " ";
        ss << "\n";

        ss << "FFD     ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << time_ffd << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Pr_ffd << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Re_ffd << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << F1_ffd << " ";
        ss << "\n";

        ss << "Merge   ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << time_merge << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Pr_merge << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Re_merge << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << F1_merge << " ";
        ss << "\n";

        if (msg_prt)
            cout << ss.str();

        if (msg_save)
            msgfile << ss.str();
    }
    msgfile.close();

    stringstream ss;

    ss << "======================================================\n";
    ss << "AVERAGE " << "  TIME[ms]      Pr[%]      Re[%]      F1[%]\n";

    ss << "ViBe+   ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << time_ave_vibe << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Pr_ave_vibe << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Re_ave_vibe << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << F1_ave_vibe << " ";
    ss << "\n";

    ss << "FFD     ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << time_ave_ffd << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Pr_ave_ffd << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Re_ave_ffd << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << F1_ave_ffd << " ";
    ss << "\n";

    ss << "Merge   ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << time_ave_merge << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Pr_ave_merge << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Re_ave_merge << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << F1_ave_merge << " ";
    ss << "\n";

    cout << ss.str();
    if (res_save)
    {
        if (res_save)
            resfile = ofstream(file_name + "/result.txt", ios::app);
        resfile << ss.str();
        resfile.close();
    }
}

cv::Mat Solution::MergeFG(cv::Mat vibe_fg, cv::Mat ffd_fg, cv::Mat& merge)
{
    if (vibe_fg.empty())
        return ffd_fg;
    if (ffd_fg.empty())
        return vibe_fg;

    Mat fg;
    vibe_fg.copyTo(fg);

    merge = Mat::zeros(vibe_fg.size(), CV_8UC1);

    // 膨胀
    Mat imgtmp;

    Mat ele = getStructuringElement(MORPH_RECT, Size(DILATION_SIZE, DILATION_SIZE));
    dilate(vibe_fg, imgtmp, ele);

    // 与运算、合并
    for (int i = 0; i < vibe_fg.rows; ++i)
    {
        for (int j = 0; j < vibe_fg.cols; ++j)
        {
            if (imgtmp.at<uchar>(i, j) == 255 && ffd_fg.at<uchar>(i, j) == 255)
                //fg.at<uchar>(i, j) = 255;
                merge.at<uchar>(i, j) = 255;
        }
    }

    for (int i = 0; i < vibe_fg.rows; ++i)
    {
        for (int j = 0; j < vibe_fg.cols; ++j)
        {
            if (merge.at<uchar>(i, j) == 255)
                fg.at<uchar>(i, j) = 255;
        }
    }

/*
    // 合并两种前景蒙版
    for (int i = 0; i < vibe_fg.rows; ++i)
    {
        for (int j = 0; j < vibe_fg.rows; ++j)
        {
            if (ffd_fg.at<uchar>(i, j) == 255)
                fg.at<uchar>(i, j) = 255;
        }
    }

    Mat imgtmp;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    fg.copyTo(imgtmp);

    // 提取轮廓
    findContours(imgtmp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

    // 遍历轮廓
    for (int i = 0; i < contours.size(); ++i)
    {
        // 一级父轮廓
        int father = hierarchy[i][3];
        // 二级父轮廓
        int grandpa = -1;
        if (father >= 0)
            grandpa = hierarchy[father][3];


        // 有一级父轮廓而没有二级父轮廓，说明其为前景孔洞
        if (father >= 0 && grandpa < 0)
        {
            // 轮廓区域大小
            double area = contourArea(contours[i]);

            // 孔洞面积小于默认值的进行填充
            if (area <= FILL_MERGE_FG_AREA)
                drawContours(fg, contours, i, Scalar(255), -1);
        }

        // 无父轮廓，说明该轮廓是最外围轮廓，即前景斑点区域，抹除过小的前景斑点
        if (father < 0)
        {
            if (contourArea(contours[i]) < DEL_MERGE_FG_AREA)
                drawContours(fg, contours, i, Scalar(0), -1);
        }
    }
*/
    return fg;
}

void Solution::setFile_name(const std::string& value)
{
    file_name = value;
}

void Solution::setRes_save(bool value)
{
    res_save = value;
}

void Solution::setMsg_save(bool value)
{
    msg_save = value;
}

void Solution::setMsg_prt(bool value)
{
    msg_prt = value;
}

void Solution::setSave_merge(bool value)
{
    save_merge = value;
}

void Solution::setPart(int value)
{
    part = value;
    if (part <= 0)
        part = 1;
}

void Solution::setShowed_result(bool value)
{
    showed_result = value;
}

void Solution::setShowed_merge(bool value)
{
    showed_merge = value;
}

void Solution::setSave_output(bool value)
{
    save_output = value;
}

void Solution::setShowed_output(bool value)
{
    showed_output = value;
}

void Solution::setShowed_input(bool value)
{
    showed_input = value;
}

void Solution::setSave_ffd_fg(bool value)
{
    save_ffd_fg = value;
}

void Solution::setShowed_ffd_fg(bool value)
{
    showed_ffd_fg = value;
}

void Solution::setSave_vibe_up(bool value)
{
    save_vibe_up = value;
}

void Solution::setShowed_vibe_up(bool value)
{
    showed_vibe_up = value;
}

void Solution::setSave_vibe_fg(bool value)
{
    save_vibe_fg = value;
}

void Solution::setShowed_vibe_fg(bool value)
{
    showed_vibe_fg = value;
}
