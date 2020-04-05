#include "VideoTest.h"
#include <opencv2/highgui/highgui_c.h>
#include <iostream>
#include <cstdio>

using namespace std;
using namespace cv;


VideoTest::VideoTest(const std::string& path) : FrameSet(path)
{
    capture = VideoCapture(path);

    if (!capture.isOpened())
    {
        cout << "ERROR: Did't find this video!" << endl;
        return;
    }

    capture.set(CAP_PROP_FRAME_WIDTH, 160);
    capture.set(CAP_PROP_FRAME_HEIGHT, 120);

    if (!capture.isOpened())
    {
        cout << "No camera or video input!" << endl;
        return;
    }
}

cv::Mat VideoTest::getNextResult(int k)
{
    return cv::Mat();
}

cv::Mat VideoTest::getNextInput(int k)
{
    Mat frame;

    for (int i = 0; i < k; ++i)
        capture >> frame;

    return frame;
}
