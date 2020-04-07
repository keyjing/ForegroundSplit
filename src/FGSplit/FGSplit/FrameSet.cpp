#include "FrameSet.h"
#include <opencv2/imgproc/types_c.h>

using namespace cv;

void GetGray(const Mat& src, Mat& dist)
{
    if (src.channels() == 3)
        cvtColor(src, dist, CV_BGR2GRAY);
    else
        src.copyTo(dist);
}

bool ForegroundCompare(const cv::Mat& src, const cv::Mat& dist, double& Pr, double& Re, double& F1)
{
    Pr = Re = F1 = 0;

    if (src.empty() || dist.empty())
        return false;

    if (src.rows != dist.rows || src.cols != dist.cols)
    {
        std::cout << "ERROR Foregound Comapre: Invalid.\n";
        return false;
    }

    Mat src_gray, dist_gray;

    GetGray(src, src_gray);
    GetGray(dist, dist_gray);

    float TP, TN, FP, FN;
    TP = TN = FP = FN = 0;

    for (int i = 0; i < src_gray.rows; ++i)
    {
        for (int j = 0; j < src_gray.cols; ++j)
        {
            if (dist_gray.at<uchar>(i, j) <= 0)
            {
                if (src_gray.at<uchar>(i, j) <= 0) ++TN;
                else ++FP;
            }
            else
            {
                if (src_gray.at<uchar>(i, j) <= 0) ++FN;
                else ++TP;
            }
        }
    }

    //std::cout << " " << TP << " " << TN << " " << FP << std::endl;

    if (TP + FP != 0)
        Pr = TP / (TP + FP);

    if (TP + FN != 0)
        Re = TP / (TP + FN);

    if (Pr + Re != 0)
        F1 = (2 * Pr * Re) / (Pr + Re);

    // 转化为百分比
    Pr *= 100;
    Re *= 100;
    F1 *= 100;

    return true;
}
