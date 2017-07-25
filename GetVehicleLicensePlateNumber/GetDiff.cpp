#include "main.h"
int GetDiff(Mat &InputMat, Mat & OutputMat)
{
	if ( InputMat.channels() != 1)
	{
		ofstream LogStream(LogFilePath, ios::app);
		time_t CurrentTime;
		tm CurrentLocalTime;
		char CurrentTimeString[20];
		localtime_s(&CurrentLocalTime, &CurrentTime);
		strftime(CurrentTimeString, sizeof(CurrentTimeString), "[%Y/%m/%d %X]", &CurrentLocalTime);
		LogStream << CurrentTimeString << "Error:" << endl << "Ilegal input parameter." << endl;
		return 1;
	}

	OutputMat = Mat::zeros(
		int(InputMat.rows),
		int(InputMat.cols),
		CV_32FC1
	);

	for (int iRow = 0; iRow < InputMat.rows; iRow++)
	{
		//叠加同一行每一列的梯度值
		for (int iCol = 1; iCol < InputMat.cols; iCol++)
		{
			OutputMat.ptr<float>(iRow)[iCol] = InputMat.ptr<float>(iRow)[iCol] - InputMat.ptr<float>(iRow)[iCol-1];
		}

	}
	return 0;

}