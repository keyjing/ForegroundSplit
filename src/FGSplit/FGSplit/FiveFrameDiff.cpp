#include "FiveFrameDiff.h"
#include <opencv2/imgproc/types_c.h>

using namespace std;
using namespace cv;

cv::Mat FiveFrameDiff::Run(cv::Mat img)
{
    if (img.empty())
    {
        cout << "ERROR FiveFrameDiff: The image is empty." << endl;
        return Mat();
    }

    // 预处理，中值滤波、图像灰度化
    Mat frame = FirstProcess(img);

    // 未满五帧时不进行差分
    if (capacity != 0)
    {
        // 保存到序列向量
        frame.copyTo(frame_vec[5 - capacity]);
        // 计算该帧8邻域和
//        CalNeighSum(5 - capacity);
        // 刚好满五帧可开始
        if (--capacity == 0)
            goto startpos;
        return Mat();
    }
    else
    {
        // 替换掉最早一帧
        int oldest = (id + 5 - 2) % 5;
        frame.copyTo(frame_vec[oldest]);
        // 计算该帧8邻域和
//        CalNeighSum(oldest);
        id = (id + 1) % 5;
    }
startpos:
    // 保存下标
    int index[5];
    for (int i = 0; i < 5; ++i)
        index[i] = (id + 5 - 2 + i) % 5;

    Mat fgModel = Mat::zeros(frame_vec[0].size(), CV_8UC1);

    // 差分运算 diffs[4][rows][cols]
    vector<vector<vector<bool>>> diffs(4, vector<vector<bool>>(frame.rows, vector<bool>(frame.cols, false)));
    for (int i = 1; i < frame.rows - 1; ++i)
    {
        for (int j = 1; j < frame.cols - 1; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                // 小于阈值T2的为背景点
//                if (neighSum[index[k + 1]][i][j] <= FRAME_DIFF_THRESHOLD_2)
//                    continue;
                uchar res = abs(frame_vec[index[k]].at<uchar>(i, j) - frame_vec[index[k + 1]].at<uchar>(i, j));
                // 大于阈值T1为前景点
                if (res > FRAME_DIFF_THRESHOLD_1)
                    diffs[k][i][j] = true;
            }

            // Diff1 = (d12 || d23) && (d23 || d34)
            bool Diff1 = (diffs[0][i][j] || diffs[1][i][j]) && (diffs[1][i][j] || diffs[2][i][j]);
            // Diff2 = (d23 || d34) && (d34 || d45)
            bool Diff2 = (diffs[1][i][j] || diffs[2][i][j]) && (diffs[2][i][j] || diffs[3][i][j]);

            // Dfg = Diff1 & Diff2
            if (Diff1 && Diff2)
                fgModel.at<uchar>(i, j) = 255;
        }
    }

    // 后期处理，斑点去除，孔洞填充
    return FinalProcess(fgModel);
}

// 预处理，中值滤波、图像灰度化
Mat FiveFrameDiff::FirstProcess(Mat img)
{
    // 中值滤波
    Mat imgtmp;
    medianBlur(img, imgtmp, MEDIAN_FILTER_SIZE);

    // 图像灰度化
    if (img.channels() == 3)
    {
        Mat res;
        cvtColor(imgtmp, res, CV_BGR2GRAY);
        return res;
    }
    else
        return imgtmp;
}

// 后期处理，斑点去除，孔洞填充
Mat FiveFrameDiff::FinalProcess(Mat img)
{
    Mat imgtmp, fg;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    img.copyTo(imgtmp);
    img.copyTo(fg);

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
            if (area <= FILL_FFD_FG_AREA)
                drawContours(fg, contours, i, Scalar(255), -1);
        }

        // 无父轮廓，说明该轮廓是最外围轮廓，即前景斑点区域，抹除过小的前景斑点
        if (father < 0)
        {
            if (contourArea(contours[i]) < DEL_FFD_FG_AREA)
                drawContours(fg, contours, i, Scalar(0), -1);
        }
    }
    return fg;
}



/*
// 计算该帧8邻域和
void FiveFrameDiff::CalNeighSum(int k)
{
    // 初始化，分配内存
    if (neighSum == nullptr)
    {
        if (frame_vec[k].empty())
        {
            cout << "ERROR FiveFrameDiff: Can not get image size." << endl;
            return;
        }
        neighSum = new int** [5];
        for (int i = 0; i < 5; ++i)
        {
            neighSum[i] = new int* [frame_vec[k].rows];
            for (int j = 0; j < frame_vec[k].rows; ++j)
            {
                neighSum[i][j] = new int[frame_vec[k].cols];
                for (int m = 0; m < frame_vec[k].cols; ++m)
                    neighSum[i][j][m] = 0;
            }
        }
    }

    // 计算序列向量中第k帧的邻域和
    for (int x = 1; x < frame_vec[k].rows - 1; ++x)
    {
        for (int y = 1; y < frame_vec[k].cols - 1; ++y)
        {
            neighSum[k][x][y] = 0;
            for (int m = 0; m < 8; ++m)
            {
                int row = x + c_xoff[m];
                int col = y + c_yoff[m];
                neighSum[k][x][y] += frame_vec[k].at<uchar>(row, col);

            }
        }
    }
}
*/
