#ifndef _FiveFrameDiff_h
#define _FiveFrameDiff_h

#include <opencv2/opencv.hpp>
#include "FiveFrameDiffMacro.h"

class FiveFrameDiff
{
public:
	FiveFrameDiff() : id(2), capacity(5), neighSum(nullptr) {};

	~FiveFrameDiff() { delete neighSum; }

	cv::Mat Run(cv::Mat img);		// 执行五帧差分法，返回前景蒙版

private:
	void CalNeighSum(int k);		// 计算邻域和;

	cv::Mat frame_vec[5];	// 存放五帧序列向量
	int id;							// 当前帧所在向量中下标，初始为2
	int capacity;

	int*** neighSum;				// 8邻域之和

	int c_xoff[9] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };
	int c_yoff[9] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };
};

#endif _FiveFrameDiff_h