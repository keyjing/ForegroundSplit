#ifndef FRAMESET_H
#define FRAMESET_H

#include <opencv2/opencv.hpp>
#include <string>

//===================================
//			帧序列集合
//
//	path:			路径
//	getPath():		获取路径
//	getNextFrame(): 获取下一帧
//	compreResult():	与结果集比较
//
//===================================
class FrameSet
{
public:
    FrameSet(const std::string& str) : path(str) {}

    virtual ~FrameSet() {}

    // 获取下一帧
    virtual cv::Mat getNextInput(int k = 1) = 0;

    virtual cv::Mat getNextResult(int k = 1) = 0;

    std::string getPath() const { return path; }

private:
    std::string path;
};

//===============================================================
//
// 进行比较，计算得精准度Pr、召回率Re、F1值
//
// TP: 真正    TN: 真负    FP: 假正    FN: 假负
//
// Pr = TP / (TP + FP)	预测值为正的例子中真实值为正的比例
// Re = TP / (TP + FN)	真实值为正的例子中被预测为正的比例
// F1 = (2 x Pr x Re) / (Pr + Re)
//
//===============================================================
bool ForegroundCompare(const cv::Mat& src, const cv::Mat& dist, double& Pr, double& Re, double& F1);

#endif // FRAMESET_H
