#ifndef VIDEOTEST_H
#define VIDEOTEST_H

#include "FrameSet.h"
#include <string>

class VideoTest : public FrameSet
{
public:
    VideoTest(const std::string& path);

    virtual cv::Mat getNextInput(int k = 1) override;

    virtual cv::Mat getNextResult(int k = 1) override;

private:
    cv::VideoCapture capture;
};

#endif // VIDEOTEST_H
