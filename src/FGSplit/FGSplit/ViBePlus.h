#ifndef _ViBePlus_h
#define _ViBePlus_h

#include <opencv2/opencv.hpp>
#include "ViBePlusMacro.h"

class ViBePlus
{
public:
	ViBePlus(int num_samples = DEFAULT_NUM_SAMPLES,
		int min_match = DEFAULT_MIN_MATCH,
		int radius = DEFAULT_RADIUS,
		int rand_sam = DEFAULT_RAND_SAM);

	~ViBePlus(void);

	cv::Mat Run(cv::Mat img);	// 执行Vibe+算法，输出蒙版

	cv::Mat getSegModel();		// 返回分割蒙版 
	cv::Mat getUpdateModel();	// 返回更新蒙版

private:
	void FrameCapture(cv::Mat img);	// 分RGB和灰度格式存储图像

	void InitMemory();	// 分配内存

	void InitModel();	// 初始化背景模型	

	void ExtractBG();		// 提取分割蒙版

	void CalUpdateModel();	// 计算更新蒙版

	void UpdateModel();		// 更新背景模型s

	void UpdatePixSampleAveAndSumSqr(int i, int j);		// 更新样本点均值、方差

	int num_samples;	// 样本数
	int min_match;		// #min参数
	int radius;			// 半径
	int rand_sam;		// 采样概率

	cv::Mat frame_rgb;		// 当前帧图像 RGB版本
	cv::Mat frame_gray;		// 灰度图像版本
	int Channels;		// 通道数，RGB为3，灰度为1

	unsigned char**** samples_rgb = nullptr;	// RGB下样本库
	unsigned char*** samples_gray = nullptr;	// 灰度下样本库

	double** samples_ave;		// 样本集均值
	double** samples_sumSqr;	// 样本集方差

	int** samples_fgCnt;		// 样本连续前景计数

	bool** samples_bgInner;		// 样本是否为背景边缘
	int** samples_innerState;	// 样本八邻域状态

	int** samples_bLinkLevel;	// 样本闪烁等级

	int** samples_maxInnerGrad;	// 样本邻域梯度最大值

	cv::Mat segModel;		// 分割蒙版
	cv::Mat updateModel;	// 更新蒙版

	int c_xoff[9] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };		// x的邻居点
	int c_yoff[9] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };		// y的邻居点
};



#endif _ViBePlus_h
