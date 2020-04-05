#include<iostream>
#include "Solution.h"
#include "VideoTest.h"
#include "CDW_2014_Test.h"

using namespace std;


int main()
{
	//VideoTest vt("../../../data/video/Camera Road 01.avi");

	CDW_2014_Test cdw("../../../data/CDW-2014/dataset/baseline/office");
	cdw.FromStart(-2);

	Solution s(cdw);

	s.Run();

	return 0;
}