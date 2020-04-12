#ifndef CDW_2014_TEST_H
#define CDW_2014_TEST_H

#include "FrameSet.h"
#include <string>

/*		CDW-2014数据集子集目录信息参数		*/
#define DEFAULT_FOLDER_INPUT	"/input/"			// 视频帧目录
#define DEFAULT_FOLDER_RESULT	"/groundtruth/"		// 结果蒙版目录
#define DEFAULT_FILE_MESSAGE	"/temporalROI.txt"	// 参数信息：起始帧数  总帧数
#define PREFIX_OF_INPUT_FILE	"in"				// 输入图片名称前缀
#define PREFIX_OF_RESULT_FILE	"gt"				// 结果图片名称前缀
#define INPUT_IMAGE_TYPE		".jpg"				// 输入图片格式
#define RESULT_IMAGE_TYPE		".png"				// 对比图片格式


class CDW_2014_Test : public FrameSet
{
public:
    CDW_2014_Test(const std::string& path);

    // 通过 FrameSet 继承
    virtual cv::Mat getNextInput(int k = 1) override;

    // 通过 FrameSet 继承
    virtual cv::Mat getNextResult(int k = 1) override;

    void setId(int id);
    int FromStart(int k = 0);		// 将指标 input_id 和 res_id 指向 start + k
private:
    int start_id, end_id, input_id, res_id;
};

#endif // CDW_2014_TEST_H
