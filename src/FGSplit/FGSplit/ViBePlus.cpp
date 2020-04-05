#include "ViBePlus.h"
#include <iostream>
#include <opencv2\imgproc\types_c.h>

using namespace std;
using namespace cv;

// 使用默认参数
ViBePlus::ViBePlus(int num_samples, int min_match, int radius, int rand_sam)
{
	this->num_samples = num_samples;
	this->min_match = min_match;
	this->radius = radius;
	this->rand_sam = rand_sam;
}

// 释放内存
ViBePlus::~ViBePlus(void)
{
	delete samples_rgb;
	delete samples_gray;
	delete samples_ave;
	delete samples_sumSqr;
	delete samples_fgCnt;
	delete samples_bgInner;
	delete samples_innerState;
	delete samples_bLinkLevel;
	delete samples_maxInnerGrad;
}

// 输入视频帧，执行Vibe+算法输出前景蒙版，并更新背景模型
Mat ViBePlus::Run(Mat img)
{
	if (img.empty())
	{
		cout << "ERROR Run: The image is empty." << endl;
		return Mat();
	}
	// 分格式存储图像
	FrameCapture(img);
	// 若样本库都为空，表示未初始化，进行初始化
	if (!samples_rgb && !samples_gray)
	{
		// 分配内存
		InitMemory();
		// 初始化模型
		InitModel();
		return Mat();
	}
	// 提取分割蒙版
	ExtractBG();

	// 由分割蒙版计算更新蒙版
	CalUpdateModel();

	// 根据更新蒙版更新背景模型
	UpdateModel();

	// 返回分割蒙版
	return segModel;
}

cv::Mat ViBePlus::getSegModel()
{
	return segModel;
}

cv::Mat ViBePlus::getUpdateModel()
{
	return updateModel;
}

// 分RGB和灰度格式存储图像
void ViBePlus::FrameCapture(cv::Mat img)
{
	img.copyTo(frame_rgb);
	if (img.channels() == 3)
	{
		cvtColor(frame_rgb, frame_gray, CV_BGR2GRAY);
		Channels = 3;
	}
	else
	{
		img.copyTo(frame_gray);
		Channels = 1;
	}
}

// 分配内存
void ViBePlus::InitMemory()
{
	segModel = Mat::zeros(frame_gray.size(), CV_8UC1);
	updateModel = Mat::zeros(frame_gray.size(), CV_8UC1);

	// 为样本库分配内存
	samples_rgb = new unsigned char*** [frame_gray.rows];	// RGB样本库
	samples_gray = new unsigned char** [frame_gray.rows];	// 灰度样本库

	// 为样本集均值、方差分配内存
	samples_ave = new double* [frame_gray.rows];
	samples_sumSqr = new double* [frame_gray.rows];

	// 为更新模型相关信息分配内存
	samples_fgCnt = new int* [frame_gray.rows];
	samples_bgInner = new bool* [frame_gray.rows];
	samples_innerState = new int* [frame_gray.rows];
	samples_bLinkLevel = new int* [frame_gray.rows];
	samples_maxInnerGrad = new int* [frame_gray.rows];

	for (int i = 0; i < frame_gray.rows; ++i)
	{
		samples_rgb[i] = new unsigned char** [frame_gray.cols];
		samples_gray[i] = new unsigned char* [frame_gray.cols];
		samples_ave[i] = new double[frame_gray.cols];
		samples_sumSqr[i] = new double[frame_gray.cols];
		samples_fgCnt[i] = new int[frame_gray.cols];
		samples_bgInner[i] = new bool[frame_gray.cols];
		samples_innerState[i] = new int[frame_gray.cols];
		samples_bLinkLevel[i] = new int[frame_gray.cols];
		samples_maxInnerGrad[i] = new int[frame_gray.cols];
		for (int j = 0; j < frame_gray.cols; ++j)
		{
			samples_rgb[i][j] = new unsigned char* [num_samples];
			samples_gray[i][j] = new unsigned char[num_samples];
			for (int k = 0; k < num_samples; ++k)
			{
				samples_rgb[i][j][k] = new unsigned char[3];
				for (int m = 0; m < 3; ++m)
					samples_rgb[i][j][k][m] = 0;
				samples_gray[i][j][k] = 0;
			}
			samples_ave[i][j] = 0;
			samples_sumSqr[i][j] = 0;
			samples_fgCnt[i][j] = 0;
			samples_bgInner[i][j] = false;
			samples_innerState[i][j] = 0;
			samples_bLinkLevel[i][j] = 0;
			samples_maxInnerGrad[i][j] = 0;
		}
	}
}

// 初始化背景模型
void ViBePlus::InitModel()
{
	RNG rng;	// RNG：OpenCV中的C++版本随机数
	int row, col;
	// 为每个像素点建立样本库
	for (int i = 0; i < frame_gray.rows; ++i)
	{
		for (int j = 0; j < frame_gray.cols; ++j)
		{
			// 每个像素点有num_samples个样本点
			for (int k = 0; k < num_samples; ++k)
			{
				// 8邻域中随机选择
				int random;
				random = rng.uniform(0, 9);		// 均匀分布
				row = i + c_xoff[random];
				random = rng.uniform(0, 9);
				col = j + c_yoff[random];

				// 防止像素点越界
				if (row < 0) row = 0;
				if (row >= frame_gray.rows) row = frame_gray.rows - 1;
				if (col < 0) col = 0;
				if (col >= frame_gray.cols) col = frame_gray.cols - 1;

				// 保存到灰度、RGB样本库
				samples_gray[i][j][k] = frame_gray.at<uchar>(row, col);
				for (int m = 0; m < 3; ++m)
					samples_rgb[i][j][k][m] = frame_rgb.at<Vec3b>(row, col)[m];

				// 累加到样本均值
				samples_ave[i][j] += samples_gray[i][j][k];
			}
			// 计算均值
			samples_ave[i][j] /= num_samples;

			// 计算方差
			for (int k = 0; k < num_samples; ++k)
				samples_sumSqr[i][j] += pow(samples_gray[i][j][k] - samples_ave[i][j], 2);
			samples_sumSqr[i][j] /= num_samples;
		}
	}
}

// 提取分割蒙版
void ViBePlus::ExtractBG()
{
	RNG rng;
	for (int i = 0; i < frame_gray.rows; ++i)
	{
		for (int j = 0; j < frame_gray.cols; ++j)
		{
			//====  计算自适应阈值  ====//
			// 距离的自适应阈值
			double adaThreshold = DEFAULT_THRESHOLD_MIN;
			// 根据方差计算标准差
			double sigma = sqrt(samples_sumSqr[i][j]);
			// 得到距离的自适应阈值
			adaThreshold = sigma * AMP_MULTIFACTOR;

			// 调整自适应阈值范围
			if (adaThreshold < DEFAULT_THRESHOLD_MIN)
				adaThreshold = DEFAULT_THRESHOLD_MIN;
			if (adaThreshold > DEFAULT_THRESHOLD_MAX)
				adaThreshold = DEFAULT_THRESHOLD_MAX;

			//====  计算颜色畸变和匹配情况  ====//
			// 当前帧在(i, j)点的RGB通道值
			int B = frame_rgb.at<Vec3b>(i, j)[0];
			int G = frame_rgb.at<Vec3b>(i, j)[1];
			int R = frame_rgb.at<Vec3b>(i, j)[2];

			int k = 0, matches = 0;
			for (; matches < min_match && k < num_samples; ++k)
			{
				// 当前帧在(i, j)点的第k个样本点的RGB通道值
				int B_sam = samples_rgb[i][j][k][0];
				int G_sam = samples_rgb[i][j][k][1];
				int R_sam = samples_rgb[i][j][k][2];

				// 颜色畸变
				//double colorDist, RGB_Norm2, RGBSam_Norm2, RGB_Vev, p2;
				double RGB_Norm2 = pow(B, 2) + pow(G, 2) + pow(R, 2);
				double RGBSam_Norm2 = pow(B_sam, 2) + pow(G_sam, 2) + pow(R_sam, 2);
				double RGB_Vec = pow(R_sam * R + G_sam * G + B_sam * B, 2);
				double p2 = RGB_Vec / RGBSam_Norm2;
				double colordist = (RGB_Norm2 > p2) ? sqrt(RGB_Norm2 - p2) : 0;

				// 若当前值与样本值之差小于自适应阈值，且颜色畸变小于阈值，满足匹配条件
				int dist = abs(samples_gray[i][j][k] - frame_gray.at<uchar>(i, j));
				if (dist < adaThreshold && colordist < DEFAULT_COLOR_THRESHOLD)
					++matches;
			}
			// 匹配次数超过#min指数为背景点，否则为前景点
			if (matches >= min_match)
			{
				// 当前点为背景点，重置前景统计数
				samples_fgCnt[i][j] = 0;

				// 在分割蒙版中标记为背景点
				segModel.at<uchar>(i, j) = 0;
			}
			else
			{
				// 当前点为前景点，前景统计数加一
				samples_fgCnt[i][j]++;

				// 在分割蒙版中标记为前景点
				segModel.at<uchar>(i, j) = 255;

				// 连续前景次数超过上限值时，该点应该被认为是背景点
				if (samples_fgCnt[i][j] > DEFAULT_FG_COUNT)
				{
					// 随机选择该点样本库的一个位置保存当前点
					int random = rng.uniform(0, num_samples);
					samples_gray[i][j][random] = frame_gray.at<uchar>(i, j);
					for (int m = 0; m < 3; ++m)
						samples_rgb[i][j][random][m] = frame_rgb.at<Vec3b>(i, j)[m];
				}
			}
		}
	}
}

// 由分割蒙版计算更新蒙版
void ViBePlus::CalUpdateModel()
{
	Mat img, imgtmp;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	segModel.copyTo(updateModel);

	segModel.copyTo(img);
	updateModel.copyTo(imgtmp);

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

			// 对更新蒙版中孔洞面积小于默认值的进行填充
			if (area <= FILL_UP_AREA_SIZE)
				drawContours(updateModel, contours, i, Scalar(255), -1);

			// 对分割蒙版中孔洞面积小于默认值的进行填充
			if (area <= FILL_SEG_AREA_SIZE)
				drawContours(segModel, contours, i, Scalar(255), -1);
		}

		// 无父轮廓，说明该轮廓是最外围轮廓，即前景斑点区域，抹除过小的前景斑点
		if (father < 0)
		{
			if (contourArea(contours[i]) < DEL_SEG_AREA_SIZE)
				drawContours(segModel, contours, i, Scalar(0), -1);
		}
	}

	// 遍历拥有完整8邻域的内部像素点
	for (int i = 1; i < frame_gray.rows - 1; ++i)
	{
		for (int j = 1; j < frame_gray.cols - 1; ++j)
		{
			// 邻域总状态位
			int state = 0;
			int maxGrad = 0;

			// 查找背景内边缘
			if (img.at<uchar>(i, j) == 0)
			{
				// 定义8邻域范围
				int i_min = i - 1;
				int i_max = i + 1;
				int j_min = j - 1;
				int j_max = j + 1;

				// 计算8邻域状态、邻域灰度最大梯度
				for (int x = i_min; x < i_max; ++x)
				{
					for (int y = j_max; y < j_max; ++y)
					{
						// 排除当前点
						if (x == i && y == j)
							continue;

						// 邻域点是否为前景点
						int bitstate = (img.at<uchar>(x, y) == 255) ? 1 : 0;
						state = (state << 1) + bitstate;

						// 计算最大梯度
						maxGrad = max(abs(frame_gray.at<uchar>(i, j) - frame_gray.at<uchar>(x, y)), maxGrad);
					}
				}
				// 当前状态位限定在8bit内
				state = state & 255;

				// 状态位大于0，则说明8邻域内有前景点，即当前点为背景内边缘
				samples_bgInner[i][j] = (state > 0);
			}
			else
			{
				samples_bgInner[i][j] = false;
				samples_innerState[i][j] = 0;
			}

			// 计算闪烁等级
			if (samples_bgInner[i][j])
			{
				// 当前点8邻域状态与上一帧相同，说明当前点不闪烁，闪烁等级减少
				if (state == samples_innerState[i][j])
					samples_bLinkLevel[i][j] = max(samples_bLinkLevel[i][j] - MINUS_BLINK_LEVEL, 0);
				else
				{
					// 当前点闪烁，闪烁等级增加
					samples_bLinkLevel[i][j] += ADD_BLINK_LEVEL;
					samples_bLinkLevel[i][j] = min(samples_bLinkLevel[i][j], MAX_BLINK_LEVEL);
				}
			}
			else
			{
				// 非背景内边缘闪烁等级减少
				samples_bLinkLevel[i][j] = max(samples_bLinkLevel[i][j] - MINUS_BLINK_LEVEL, 0);
			}

			// 更新状态位、最大梯度
			samples_bLinkLevel[i][j] = state;
			samples_maxInnerGrad[i][j] = maxGrad;

			// 闪烁等级超过阈值时，从更新蒙版中移除
			if (samples_bLinkLevel[i][j] > TC_BLINK_LEVEL)
				updateModel.at<uchar>(i, j) = 255;
		}
	}
}

// 根据更新蒙版更新背景模型
void ViBePlus::UpdateModel()
{
	RNG rng;
	for (int i = 0; i < frame_gray.rows; ++i)
	{
		for (int j = 0; j < frame_gray.cols; ++j)
		{
			// 只对背景点进行更新，忽略前景点
			if (updateModel.at<uchar>(i, j) > 0)
				continue;

			// 更新该点样本库，概率为 1/rand_sam
			int random = rng.uniform(0, rand_sam);
			if (random == 0)
			{
				// 随机选取样本库中的样本点
				random = rng.uniform(0, num_samples);

				// 更新样本库
				uchar newVal = frame_gray.at<uchar>(i, j);
				samples_gray[i][j][random] = newVal;
				for (int m = 0; m < 3; ++m)
					samples_rgb[i][j][random][m] = frame_rgb.at<Vec3b>(i, j)[m];

				// 更新样本库均值、方差
				UpdatePixSampleAveAndSumSqr(i, j);
			}

			// 同时更新邻居点的样本库，概率为 1/rand_sam
			random = rng.uniform(0, rand_sam);
			if (random == 0)
			{
				// 根据当前点最大梯度
				if (samples_maxInnerGrad[i][j] > MAX_INNER_GRAD)
					continue;

				// 随机选取邻居点
				random = rng.uniform(0, 9);
				int row = i + c_xoff[random];
				random = rng.uniform(0, 9);
				int col = j + c_yoff[random];

				// 防止像素点越界
				if (row < 0) row = 0;
				if (row >= frame_gray.rows) row = frame_gray.rows - 1;
				if (col < 0) col = 0;
				if (col >= frame_gray.cols) col = frame_gray.cols - 1;

				// 随机选取样本库中的样本点
				random = rng.uniform(0, num_samples);

				// 更新样本库
				uchar newVal = frame_gray.at<uchar>(i, j);
				samples_gray[row][col][random] = newVal;
				for (int m = 0; m < 3; ++m)
					samples_rgb[row][col][random][m] = frame_rgb.at<Vec3b>(i, j)[m];

				// 更新样本库均值、方差
				UpdatePixSampleAveAndSumSqr(row, col);
			}
		}
	}
}

// 更新样本库均值、方差
void ViBePlus::UpdatePixSampleAveAndSumSqr(int i, int j)
{
	double ave = 0, sumSqr = 0;

	// 计算均值
	for (int m = 0; m < num_samples; ++m)
		ave += samples_gray[i][j][m];
	ave /= num_samples;

	// 写入均值
	samples_ave[i][j] = ave;

	// 计算方差
	for (int m = 0; m < num_samples; ++m)
		sumSqr += pow(samples_gray[i][j][m] - ave, 2);
	sumSqr /= num_samples;

	// 写入方差
	samples_sumSqr[i][j] = sumSqr;
}
