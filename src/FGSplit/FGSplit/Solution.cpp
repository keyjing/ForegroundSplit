#include "Solution.h"

#include <iostream>
#include "ViBePlus.h"
#include "FiveFrameDiff.h"
#include <queue>
#include <opencv2\imgproc\types_c.h>

using namespace std;
using namespace cv;

void Solution::Run()
{
	Mat frame_in, frame_res;

	ViBePlus vibeplus;
	FiveFrameDiff ffd;

	// 五帧差分法前两帧不输出
	for (int i = 0; i < 2; ++i)
	{
		frame_in = fs.getNextInput();
		ffd.Run(frame_in);
	}
	// 第二帧 Vibe+建模
	vibeplus.Run(frame_in);

	// 结果帧跳过前两帧
	fs.getNextResult(2);

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
	double time, start;

	float Pr, Re, F1;
	double Pr_last, Re_last, F1_last;
	Pr_last = Re_last = F1_last = 0;

	cout << "IMAGE ID    IIME(ms)      Pr(%)      Re(%)      F1(%)" << endl;

	while (!inque.empty())
	{
		// vibe结果、ffd结果、最终结果、当前处理输入帧、当前处理结果帧
		Mat vibe_fg, ffd_fg, fg, input, result;

		start = static_cast<double>(getTickCount());

		// 从队列获取当前处理输入帧、结果帧
		inque.front().copyTo(input);
		inque.pop();
		if (input.empty())
			continue;
		resque.front().copyTo(result);
		resque.pop();

		// Vibe+从当前输入处理
		vibe_fg = vibeplus.Run(input);

		// ffd从后两帧处理
		frame_in = fs.getNextInput();
		if (!frame_in.empty())
		{
			ffd_fg = ffd.Run(frame_in);
			inque.push(frame_in);

			frame_res = fs.getNextResult();
			resque.push(frame_res);
		}

		fg = MergeFG(vibe_fg, ffd_fg);

		time = ((double)getTickCount() - start) / getTickFrequency() * 1000;

		std::cout << setw(6) << ++cnt;
		std::cout << setw(14) << time;
		if (!result.empty() && ForegroundCompare(fg, result, Pr, Re, F1))
		{
			std::cout << setw(11) << Pr;
			std::cout << setw(11) << Re;
			std::cout << setw(11) << F1;

			// 计算平局值
			Pr_last = (Pr_last * (cnt - 1) + Pr) / cnt;
			Re_last = (Re_last * (cnt - 1) + Re) / cnt;
			F1_last = (F1_last * (cnt - 1) + F1) / cnt;
		}
		//if(Pr < 1)
		//{
		//	imshow("对比", result);
		//	imshow("最终前景蒙版", fg);
		//}
		std::cout << endl;

		if (!ffd_fg.empty() && !vibe_fg.empty() && !fg.empty())
		{
			imshow("输入", input);
			if(!result.empty())
				imshow("对比", result);
			//imshow("vibe前景蒙版", vibe_fg);
			//imshow("帧差前景蒙版", ffd_fg);
			imshow("最终前景蒙版", fg);
			//imshow("更新蒙版", vibeplus.getupdatemodel());
		}
		cv::waitKey(25);
	}
	cout << "Last Pr: " << Pr_last << endl;
	cout << "Last Re: " << Re_last << endl;
	cout << "Last F1: " << F1_last << endl;
}

cv::Mat Solution::MergeFG(cv::Mat vibe_fg, cv::Mat ffd_fg)
{
	if (vibe_fg.empty())
		return ffd_fg;
	if (ffd_fg.empty())
		return vibe_fg;

	Mat fg;
	vibe_fg.copyTo(fg);

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


	return fg;
}


