#include "CDW_2014_Test.h"
#include <opencv2/highgui/highgui_c.h>
#include <fstream>

using namespace std;
using namespace cv;

CDW_2014_Test::CDW_2014_Test(const std::string& path) : FrameSet(path), start_id(0), end_id(0), input_id(0), res_id(0)
{
	ifstream ifs;
	ifs.open(path + DEFAULT_FILE_MESSAGE, ios::in);
	if (!ifs.is_open())
	{
		cout << "ERROR CDW-2014: Open Message File Failed." << endl;
		return;
	}
	ifs >> start_id >> end_id;
	if (start_id < 0 || start_id > end_id)
	{
		cout << "ERROR CDW-2014: Message File Invalid." << endl;
		return;
	}
	cout << "CDW-2014 \nPATH: " << path << endl;
	cout << "All Frame:\t" << end_id << "\n" << "Start From:\t" << start_id << endl;
}

cv::Mat CDW_2014_Test::getNextInput(int k)
{
	input_id += k;
	if (input_id > end_id)
	{
		//cout << "CDW_2014: Image Over." << endl;
		return Mat();
	}

	stringstream ss;
	ss << PREFIX_OF_INPUT_FILE << setw(6) << setfill('0') << input_id;

	string name = FrameSet::getPath() + DEFAULT_FOLDER_INPUT + ss.str() + INPUT_IMAGE_TYPE;
	//cout << "Load: " << name << endl;

	Mat frame = imread(name);

	if (frame.empty())
	{
		cout << "ERROR CDW_2014: Can not load the image. " << input_id << endl;
		return Mat();
	}

	return frame;
}

void CDW_2014_Test::setId(int id)
{
	input_id = id;
	if (input_id < 0) input_id = 0;
	if (input_id > end_id) input_id = end_id;
	res_id = input_id;
}

void CDW_2014_Test::FromStart(int k)
{
	setId(start_id + k);
}

cv::Mat CDW_2014_Test::getNextResult(int k)
{
	res_id += k;
	if (res_id > end_id)
	{
		//cout << "CDW_2014: Image Over." << endl;
		return Mat();
	}

	stringstream ss;
	ss << PREFIX_OF_RESULT_FILE << setw(6) << setfill('0') << res_id;

	string name = FrameSet::getPath() + DEFAULT_FOLDER_RESULT + ss.str() + RESULT_IMAGE_TYPE;

	Mat frame = imread(name);

	if (frame.empty())
	{
		cout << "ERROR CDW_2014: Can not load the image. " << res_id << endl;
		return Mat();
	}

	return frame;

	return cv::Mat();
}

