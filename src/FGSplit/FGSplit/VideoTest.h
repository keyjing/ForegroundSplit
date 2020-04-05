#ifndef _VideoTest_h
#define _VideoTest_h

#include "FrameSet.h"
#include <string>

class VideoTest : public FrameSet
{
public:
	VideoTest(const std::string& path);

	// 通过 FrameSet 继承
	virtual cv::Mat getNextInput(int k = 1) override;

	// 通过 FrameSet 继承
	virtual cv::Mat getNextResult(int k = 1) override;

private:
	cv::VideoCapture capture;
};


#endif _VideoTest_h
