//添加OpenCV相关头文件
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
//添加标准库输入输出流头文件
#include <iostream>
//添加算法库头文件
#include<algorithm>//为了使用swap
#include<numeric>//为了使用iota
//添加数学运算库头文件
//#include <cmath>

#include <fstream> //为了使用ifstream判断文件是否存在
//添加TinyXml2头文件
#include "tinyxml2.h" //为了使用TinyXml2


//使用C++标准库命名空间
using namespace std;
////使用TinyXml2命名空间
//using namespace tinyxml2;


//使用OpenCV库命名空间
using namespace cv;

//从模板图片中获得的位置信息
//模板图片的宽、高
const double TemplateImageLineGapHeight = 83;

double WindowHeight = 700.0;

double MinMatchScale = 0.6;
double MaxMatchScale = 1.2;

template <typename SortValueType>
vector<int>  SortIndex(vector<SortValueType> &InputValueVector) {

	// initialize original index locations
	vector<int> IndexVector(InputValueVector.size());
	iota(IndexVector.begin(), IndexVector.end(), 0);

	// sort indexes based on comparing values in v
	sort(IndexVector.begin(),
		IndexVector.end(),
		[&InputValueVector](int x, int y) {return InputValueVector[x] > InputValueVector[y]; });
	return IndexVector;
}


//主函数，输入命令行参数：
//第1个为待处理图片的路径：可以使用Glob表达式选中多个文件，不同文件将逐个循环处理
//第2个参数为结果输出路径,默认在该文件夹下以“Result_”+输入图像文件名的方式输出结果
int main(int ArgumentCount, char** ArgumentVector)
{

	//检查命令行所带参数数目是否正确，如果不正确则显示用法说明并退出程序
	if (ArgumentCount != 4)
	{
		//显示程序用法说明
		cout << " Usage:  " << ArgumentVector[0] << "ImagePathGlobExpression" << "OutputPath" << endl;
		//返回错误码并退出程序
		return -1;
	}
	//分别读取两个命令参数
	String SearchGlobExpression = ArgumentVector[1];
	String SearchTemplatePath = ArgumentVector[2];
	String OutputPath = ArgumentVector[3];

	////////////////////////////////////////////////////////////////
	//新建矩阵RawImageMat用于存储原始图片数据
	Mat TemplateImage;

	//根据第一个参数的文件路径进行图片读取
	TemplateImage = imread(
		SearchTemplatePath,//输入图片路径
		CV_LOAD_IMAGE_UNCHANGED//以不修改通道类型的方式读取图片
	);

	//检查读入的Mat是否包含图像数据
	if (!TemplateImage.data)
	{
		//显示图片读取失败提示信息
		cout << " Error:  Can't read image data from" << SearchTemplatePath << endl;
		//返回错误码并退出程序
		return -1;
	}

	int TemplateImageHeight = TemplateImage.rows;
	int TemplateImageWidth = TemplateImage.cols;
	double TemplateImageRatio = double(TemplateImageWidth) / TemplateImageHeight;

	//将图片转换成灰阶图像
	Mat Gray_TemplateImage;

	//获取图片的通道数
	int NumTemplateImageChannel = TemplateImage.channels();

	//如果图像为3通道彩色图像
	if (NumTemplateImageChannel == 3)
	{
		//将图片由BGR转换成灰阶图像
		cvtColor(
			TemplateImage,//输入图片矩阵
			Gray_TemplateImage,//输出图片矩阵 
			COLOR_BGR2GRAY//将图片由BGR（OpenCV默认通道格式）转换成灰阶图像
		);
	}

	//如果图像为4通道（包含alpha通道）图像，则将其转换成灰阶图像
	else if (NumTemplateImageChannel == 4)
	{
		//将图片由BGRA转换成灰阶图像
		cvtColor(
			TemplateImage,//输入图片矩阵
			Gray_TemplateImage,//输出图片矩阵 
			COLOR_BGRA2GRAY//将图片由BGRA转换成灰阶图像
		);
	}
	//如果图像已经为单通道灰阶图像，直接将ResizedImageMat赋给GrayImageMat
	else if (NumTemplateImageChannel == 1)
	{
		Gray_TemplateImage = TemplateImage;
	}

	//如果通道数不为1,3或4，输出错误码并退出程序
	else
	{
		cout << "Unkown image channel type: " << NumTemplateImageChannel;
	}


	//创建矩阵用于存放图像X方向的梯度值
	Mat TemplateImage_Grad(
		TemplateImageHeight,//矩阵的第一维（高度）尺寸
		TemplateImageWidth, //矩阵的第二维（宽度）尺寸
		CV_8UC1,//矩阵的值类型，在这里是单通道8位无符号整数 
		Scalar(0)//矩阵填充的初始值
	);

	//创建矩阵用于存放图像X方向的梯度值
	Mat TemplateImage_GradX(
		TemplateImageHeight,//矩阵的第一维（高度）尺寸
		TemplateImageWidth, //矩阵的第二维（宽度）尺寸
		CV_8UC1,//矩阵的值类型，在这里是单通道8位无符号整数 
		Scalar(0)//矩阵填充的初始值
	);

	//创建矩阵用于存放图像X方向的梯度值
	Mat TemplateImage_GradY(
		TemplateImageHeight,//矩阵的第一维（高度）尺寸
		TemplateImageWidth, //矩阵的第二维（宽度）尺寸
		CV_8UC1,//矩阵的值类型，在这里是单通道8位无符号整数 
		Scalar(0)//矩阵填充的初始值
	);

	//逐个像素计算垂直梯度，上下左右边缘不作计算，其值为填充的初始值0
	for (int iRow = 1; iRow < TemplateImageHeight - 1; iRow++)
	{
		for (int iCol = 1; iCol < (TemplateImageWidth - 1); iCol++)
		{
			TemplateImage_GradY.at<uchar>(iRow, iCol) = uchar(abs(10.0 * (Gray_TemplateImage.at<uchar>(iRow + 1, iCol) - Gray_TemplateImage.at<uchar>(iRow - 1, iCol)) +
				3.0 * (Gray_TemplateImage.at<uchar>(iRow + 1, iCol - 1) - Gray_TemplateImage.at<uchar>(iRow - 1, iCol - 1)) +
				3.0 * (Gray_TemplateImage.at<uchar>(iRow + 1, iCol + 1) - Gray_TemplateImage.at<uchar>(iRow - 1, iCol + 1))) / 16);

			TemplateImage_GradX.at<uchar>(iRow, iCol) = uchar(abs(10.0 * (Gray_TemplateImage.at<uchar>(iRow, iCol + 1) - Gray_TemplateImage.at<uchar>(iRow, iCol + 1)) +
				3.0 * (Gray_TemplateImage.at<uchar>(iRow + 1, iCol + 1) - Gray_TemplateImage.at<uchar>(iRow + 1, iCol - 1)) +
				3.0 * (Gray_TemplateImage.at<uchar>(iRow - 1, iCol + 1) - Gray_TemplateImage.at<uchar>(iRow - 1, iCol - 1))) / 16);
			TemplateImage_Grad.at<uchar>(iRow, iCol) = (TemplateImage_GradY.at<uchar>(iRow, iCol) + TemplateImage_GradX.at<uchar>(iRow, iCol)) / 2;
		}
	}

	Mat Binary_GradY;
	threshold(
		TemplateImage_Grad, //输入矩阵
		Binary_GradY, //输出矩阵
		128, //迭代初始阈值
		255, //最大值（超过阈值将设为此值）
		CV_THRESH_OTSU //阈值化选择的方法:Otsu法
	);


	//创建X方向梯度投影向量
	Mat  Binary_ProjectX_GradY(
		int(TemplateImageHeight),//矩阵行数
		1,//矩阵列数
		CV_32FC1,//矩阵值的类型（8位无符号整数单通道）
		Scalar(0)//矩阵填入的初始值
	);

	//临时加和变量
	float SumTemp;
	for (int iRow = 0; iRow < TemplateImageHeight; iRow++)
	{
		//每次叠加前将加和变量清零
		SumTemp = 0;

		//叠加同一行每一列的梯度值
		for (int iCol = 0; iCol < TemplateImageWidth; iCol++)
		{
			SumTemp += float(Binary_GradY.at<uchar>(iRow, iCol));
		}
		//求叠加值的均值作为水平投影后的梯度值
		Binary_ProjectX_GradY.at<float>(iRow, 0) = float(SumTemp / TemplateImageWidth);
	}


	Mat Binary_Grad;
	threshold(
		TemplateImage_Grad, //输入矩阵
		Binary_Grad, //输出矩阵
		128, //迭代初始阈值
		255, //最大值（超过阈值将设为此值）
		CV_THRESH_OTSU //阈值化选择的方法:Otsu法
	);


	//创建X方向梯度投影向量
	Mat  Binary_ProjectX_Grad(
		int(TemplateImageHeight),//矩阵行数
		1,//矩阵列数
		CV_32FC1,//矩阵值的类型（8位无符号整数单通道）
		Scalar(0)//矩阵填入的初始值
	);

	for (int iRow = 0; iRow < TemplateImageHeight; iRow++)
	{
		//每次叠加前将加和变量清零
		SumTemp = 0;

		//叠加同一行每一列的梯度值
		for (int iCol = 0; iCol < TemplateImageWidth; iCol++)
		{
			SumTemp += float(Binary_Grad.at<uchar>(iRow, iCol));
		}
		//求叠加值的均值作为水平投影后的梯度值
		Binary_ProjectX_Grad.at<float>(iRow, 0) = float(SumTemp / TemplateImageWidth);
	}

	Mat  TemplateImageProjectXGrad = Binary_ProjectX_Grad;
	Mat  TemplateImageProjectXGradY = Binary_ProjectX_GradY;
	vector<int> TemplateLineRow = { 40,107,190,269,356,439,522,605,683 };
	int TemplateMatchHeight = *TemplateLineRow.rbegin() - *TemplateLineRow.begin();

	////////////////////////////////////////////////////////////////

	//创建用于存储图片路径的String数组
	vector<String> XmlPathList;

	//根据输入Glob表达式查找符合的图片
	glob(
		SearchGlobExpression,//文件查找Glob表达式
		XmlPathList, //输出图像文件列表
		false//不进行递归，即不对子文件夹进行搜索
	);


	int InputImageHeight;
	int InputImageWidth;

	//对符合的图片进行遍历
	for (vector<String>::iterator itInputXmlPath = XmlPathList.begin();
		itInputXmlPath < XmlPathList.end();
		itInputXmlPath++)
	{

		ifstream XmlFileStream(*itInputXmlPath);
		if (!XmlFileStream)
		{
			cout << "Can't find XML file: '" << *itInputXmlPath <<
				"'. This item would be skipped." << endl;
			continue;
		}

		tinyxml2::XMLDocument XmlDoc;
		XmlDoc.LoadFile((*itInputXmlPath).c_str());
		tinyxml2::XMLElement *LabelElement = XmlDoc.FirstChildElement("annotation")->FirstChildElement("object");
		Rect DetectAreaRect;
		bool IsDetectAreaRectFound = false;
		if (LabelElement == nullptr)
		{
			cout << "Can't find \"object\" element of XML file: '" << *itInputXmlPath <<
				"'. This item would be skipped." << endl;
		}
		else {

			while (LabelElement)
			{

				tinyxml2::XMLElement *LabelNameElement = LabelElement->FirstChildElement("name");
				if (LabelNameElement != nullptr)
				{
					String LabelName = LabelNameElement->GetText();

					if (LabelName == "14.DetectArea")
					{
						tinyxml2::XMLElement *LabelRectElement = LabelElement->FirstChildElement("bndbox");

						DetectAreaRect.x = atoi(LabelRectElement->FirstChildElement("xmin")->GetText());
						DetectAreaRect.y = atoi(LabelRectElement->FirstChildElement("ymin")->GetText());
						DetectAreaRect.width = atoi(LabelRectElement->FirstChildElement("xmax")->GetText()) - DetectAreaRect.x;
						DetectAreaRect.height = atoi(LabelRectElement->FirstChildElement("ymax")->GetText()) - DetectAreaRect.y;
						IsDetectAreaRectFound = true;
						break;
					}
				}


				LabelElement = LabelElement->NextSiblingElement("object");
			}

		}
		if (!IsDetectAreaRectFound)
		{
			cout << "Can't not find '14.DetectedArea' element of XML file: '" << *itInputXmlPath <<
				"'. This item would be skipped." << endl;
			continue;
		}
		size_t LastDotPos = (*itInputXmlPath).rfind('.');//rfind 反向查找'.'
														 //获取输出图片保存路径（文件名为输入图像名称前面加上“Result_”）
		String InputImagePath = (*itInputXmlPath).substr(0, LastDotPos) + ".png";
		//新建矩阵RawImageMat用于存储原始图片数据
		Mat InputImage;

		Mat InputImageSegmentResult;

		vector<int> SegmentLineRow(TemplateLineRow.size());

		//根据第一个参数的文件路径进行图片读取
		Mat InputImageAll = imread(
			InputImagePath,//输入图片路径
			CV_LOAD_IMAGE_UNCHANGED//以不修改通道类型的方式读取图片
		);


		//检查读入的Mat是否包含图像数据
		if (!InputImageAll.data)
		{
			//显示图片读取失败提示信息
			cout << " Error:  Can't read image data from '" << InputImagePath <<
				"'. This item would be skipped." << endl;
			//返回错误码并跳过此图片
			continue;
		}

		InputImage = InputImageAll(DetectAreaRect);

		InputImageHeight = InputImage.rows;
		InputImageWidth = InputImage.cols;

		//将图片转换成灰阶图像
		Mat Gray_InputImage;

		//获取图片的通道数
		int NumInputImageChannel = InputImage.channels();

		//如果图像为3通道彩色图像
		if (NumInputImageChannel == 3)
		{
			//将图片由BGR转换成灰阶图像
			cvtColor(
				InputImage,//输入图片矩阵
				Gray_InputImage,//输出图片矩阵 
				COLOR_BGR2GRAY//将图片由BGR（OpenCV默认通道格式）转换成灰阶图像
			);
		}

		//如果图像为4通道（包含alpha通道）图像，则将其转换成灰阶图像
		else if (NumInputImageChannel == 4)
		{
			//将图片由BGRA转换成灰阶图像
			cvtColor(
				InputImage,//输入图片矩阵
				Gray_InputImage,//输出图片矩阵 
				COLOR_BGRA2GRAY//将图片由BGRA转换成灰阶图像
			);
		}
		//如果图像已经为单通道灰阶图像，直接将ResizedImageMat赋给GrayImageMat
		else if (NumInputImageChannel == 1)
		{
			Gray_InputImage = InputImage;
		}

		//如果通道数不为1,3或4，输出错误码并退出程序
		else
		{
			cout << "Unkown image channel type: " << NumInputImageChannel;
		}

		//创建矩阵用于存放图像X方向的梯度值
		Mat InputImage_GradX(
			int(InputImageHeight),//矩阵的第一维（高度）尺寸
			int(InputImageWidth), //矩阵的第二维（宽度）尺寸
			CV_8UC1,//矩阵的值类型，在这里是单通道8位无符号整数 
			Scalar(0)//矩阵填充的初始值
		);

		//创建矩阵用于存放图像X方向的梯度值
		Mat InputImage_GradY(
			int(InputImageHeight),//矩阵的第一维（高度）尺寸
			int(InputImageWidth), //矩阵的第二维（宽度）尺寸
			CV_8UC1,//矩阵的值类型，在这里是单通道8位无符号整数 
			Scalar(0)//矩阵填充的初始值
		);

		//创建矩阵用于存放图像X方向的梯度值
		Mat InputImage_Grad(
			int(InputImageHeight),//矩阵的第一维（高度）尺寸
			int(InputImageWidth), //矩阵的第二维（宽度）尺寸
			CV_8UC1,//矩阵的值类型，在这里是单通道8位无符号整数 
			Scalar(0)//矩阵填充的初始值
		);

		//逐个像素计算垂直梯度，上下左右边缘不作计算，其值为填充的初始值0
		for (int iRow = 1; iRow < InputImageHeight - 1; iRow++)
		{
			for (int iCol = 1; iCol < (InputImageWidth - 1); iCol++)
			{
				InputImage_GradY.at<uchar>(iRow, iCol) = uchar(abs(10.0 * (Gray_InputImage.at<uchar>(iRow + 1, iCol) - Gray_InputImage.at<uchar>(iRow - 1, iCol)) +
					3.0 * (Gray_InputImage.at<uchar>(iRow + 1, iCol - 1) - Gray_InputImage.at<uchar>(iRow - 1, iCol - 1)) +
					3.0 * (Gray_InputImage.at<uchar>(iRow + 1, iCol + 1) - Gray_InputImage.at<uchar>(iRow - 1, iCol + 1))) / 16);

				InputImage_GradX.at<uchar>(iRow, iCol) = uchar(abs(10.0 * (Gray_InputImage.at<uchar>(iRow, iCol + 1) - Gray_InputImage.at<uchar>(iRow, iCol + 1)) +
					3.0 * (Gray_InputImage.at<uchar>(iRow + 1, iCol + 1) - Gray_InputImage.at<uchar>(iRow + 1, iCol - 1)) +
					3.0 * (Gray_InputImage.at<uchar>(iRow - 1, iCol + 1) - Gray_InputImage.at<uchar>(iRow - 1, iCol - 1))) / 16);
				InputImage_Grad.at<uchar>(iRow, iCol) = (InputImage_GradX.at<uchar>(iRow, iCol) + InputImage_GradY.at<uchar>(iRow, iCol)) / 2;
			}
		}

		Mat Binary_GradY;
		threshold(
			InputImage_GradY, //输入矩阵
			Binary_GradY, //输出矩阵
			128, //迭代初始阈值
			255, //最大值（超过阈值将设为此值）
			CV_THRESH_OTSU //阈值化选择的方法:Otsu法
		);

		////创建X方向梯度投影向量
		Mat  Binary_ProjectX_GradY(
			int(InputImageHeight),//矩阵行数
			1,//矩阵列数
			CV_32FC1,//矩阵值的类型（8位无符号整数单通道）
			Scalar(0)//矩阵填入的初始值
		);

		//临时加和变量
		double SumTemp;
		for (int iRow = 0; iRow < InputImageHeight; iRow++)
		{
			//每次叠加前将加和变量清零
			SumTemp = 0;

			//叠加同一行每一列的梯度值
			for (int iCol = 0; iCol < InputImageWidth; iCol++)
			{
				SumTemp += float(Binary_GradY.at<uchar>(iRow, iCol));
			}
			//求叠加值的均值作为水平投影后的梯度值
			Binary_ProjectX_GradY.at<float>(iRow, 0) = float(SumTemp / InputImageWidth);
		}



		Mat Binary_Grad;
		threshold(
			InputImage_Grad, //输入矩阵
			Binary_Grad, //输出矩阵
			128, //迭代初始阈值
			255, //最大值（超过阈值将设为此值）
			CV_THRESH_OTSU //阈值化选择的方法:Otsu法
		);



		////创建X方向梯度投影向量
		Mat  Binary_ProjectX_Grad(
			int(InputImageHeight),//矩阵行数
			1,//矩阵列数
			CV_32FC1,//矩阵值的类型（8位无符号整数单通道）
			Scalar(0)//矩阵填入的初始值
		);

		//临时加和变量
		for (int iRow = 0; iRow < InputImageHeight; iRow++)
		{
			//每次叠加前将加和变量清零
			SumTemp = 0;

			//叠加同一行每一列的梯度值
			for (int iCol = 0; iCol < InputImageWidth; iCol++)
			{
				SumTemp += float(Binary_Grad.at<uchar>(iRow, iCol));
			}
			//求叠加值的均值作为水平投影后的梯度值
			Binary_ProjectX_Grad.at<float>(iRow, 0) = float(SumTemp / InputImageWidth);
		}



		//创建X方向梯度投影差分向量
		Mat  GradY_DiffGradY(
			int(InputImageHeight),//矩阵行数
			1,//矩阵列数
			CV_32FC1,//矩阵值的类型（8位无符号整数单通道）
			Scalar(0)//矩阵填入的初始值
		);

		for (int iRow = 1; iRow < InputImageHeight; iRow++)
		{
			GradY_DiffGradY.at<float>(iRow, 0) = fabsf(Binary_ProjectX_GradY.at<float>(iRow, 0) -
				Binary_ProjectX_GradY.at<float>(iRow - 1, 0));
		}

		Mat Histogram_DiffGradY(
			InputImageHeight,
			InputImageWidth,
			CV_8UC1,
			Scalar(0)
		);
		for (int iRow = 1; iRow < InputImageHeight - 1; iRow++)
		{
			//根据投影后的梯度值绘制Stem图，每一行根据Stem值大小绘制不同宽度的Stem
			Histogram_DiffGradY(Range(iRow, iRow + 1), Range(0, int(GradY_DiffGradY.at<float>(iRow, 0) / 255.0*InputImageWidth))) = 100;

		}

		vector<int> DiffGradY_PeakRow;
		vector<float> DiffGradY_PeakAmp;

		for (int iRow = 1; iRow < InputImageHeight - 1; iRow++)
		{
			if (GradY_DiffGradY.at<float>(iRow, 0) >= GradY_DiffGradY.at<float>(iRow - 1, 0) &&
				GradY_DiffGradY.at<float>(iRow, 0) >= GradY_DiffGradY.at<float>(iRow + 1, 0))
			{

				DiffGradY_PeakRow.push_back(iRow);
				DiffGradY_PeakAmp.push_back(GradY_DiffGradY.at<float>(iRow, 0));

			}
		}


		vector<int> SortPeakIndex = SortIndex(DiffGradY_PeakAmp);
		vector <int> Sort_PeakRow;
		for (vector<int>::iterator itPeakIndex = SortPeakIndex.begin();
			itPeakIndex < SortPeakIndex.end();
			itPeakIndex++)
		{
			Sort_PeakRow.push_back(DiffGradY_PeakRow[*itPeakIndex]);
		}

		bool FlagIgnorePeak = false;
		vector <int>  PeakRow_LineRowTemp;

		double MinPeakDistance;

		Mat InputImageTemp;
		Mat Histogram_DiffGradYTemp;
		float TemplateScaleStep = float(0.01);
		int NumTemplateScale = int((MaxMatchScale - MinMatchScale) / TemplateScaleStep);
		float CurrentMatchScale;

		bool IsLineRowChanged = true;
		bool IsResizedTemplateMatchHeightChanged = true;


		int ResizedTemplateMatchHeight;
		int ResizedTemplateMatchHeightTemp = -1;

		double MaxCorrCoef = 0.0;
		double DilatestMatchScale = 0.0;


		Mat TemplateGrad = TemplateImageProjectXGrad(Range(*TemplateLineRow.begin(), *TemplateLineRow.rbegin()), Range::all());
		for (int iScale = 0; iScale < NumTemplateScale; iScale++)
		{
			CurrentMatchScale = float(MinMatchScale + iScale*TemplateScaleStep);
			InputImageTemp = InputImage.clone();
			Histogram_DiffGradYTemp = Histogram_DiffGradY.clone();
			MinPeakDistance = InputImageHeight * CurrentMatchScale * (TemplateImageLineGapHeight / TemplateImageHeight) * 0.8;
			ResizedTemplateMatchHeight = int(InputImageHeight * CurrentMatchScale * TemplateMatchHeight / TemplateImageHeight);
			if (ResizedTemplateMatchHeightTemp == -1 || ResizedTemplateMatchHeight != ResizedTemplateMatchHeightTemp)
			{
				IsResizedTemplateMatchHeightChanged = true;
				ResizedTemplateMatchHeightTemp = ResizedTemplateMatchHeight;
			}
			else
			{
				IsResizedTemplateMatchHeightChanged = false;
			}
			vector<int> PeakRow_LineRow;

			for (size_t iPeak = 0; iPeak < Sort_PeakRow.size(); iPeak++)
			{
				for (size_t iLine = 0; iLine < PeakRow_LineRow.size(); iLine++)
				{
					if (abs(int(Sort_PeakRow[iPeak]) - int(PeakRow_LineRow[iLine])) < MinPeakDistance)
					{
						FlagIgnorePeak = true;
						break;
					}
				}
				if (FlagIgnorePeak == true)
				{
					FlagIgnorePeak = false;
				}
				else
				{
					PeakRow_LineRow.push_back(Sort_PeakRow[iPeak]);
					//InputImageTemp.row(Sort_PeakRow[iPeak]) = Scalar(0, 0, 255);

					//根据投影后的梯度值绘制Stem图，每一行根据Stem值大小绘制不同宽度的Stem
					Histogram_DiffGradYTemp(Range(Sort_PeakRow[iPeak], Sort_PeakRow[iPeak] + 1),
						Range(0, int(GradY_DiffGradY.at<float>(Sort_PeakRow[iPeak], 0) / 255.0*InputImageWidth))) = 255;
				}
			}
			sort(PeakRow_LineRow.begin(), PeakRow_LineRow.end());
			if (PeakRow_LineRowTemp.empty() || PeakRow_LineRow != PeakRow_LineRowTemp)
			{
				IsLineRowChanged = true;
				PeakRow_LineRowTemp = PeakRow_LineRow;
			}
			else
			{
				IsLineRowChanged = false;
			}



			if (IsLineRowChanged || IsResizedTemplateMatchHeightChanged)
			{
				int InputMatchHeight = *(PeakRow_LineRow.rbegin()) - *PeakRow_LineRow.begin();

				Mat Resize_TemplateGrad;
				resize(
					TemplateGrad,
					Resize_TemplateGrad,
					Size(1, ResizedTemplateMatchHeight),
					0,
					0,
					INTER_LINEAR
				);

				vector <int> ResizeShift_TemplateLineRow;
				ResizeShift_TemplateLineRow.push_back(-1 * ResizedTemplateMatchHeight + 1);

				for (vector<int>::iterator itLine = TemplateLineRow.begin() + 1; itLine != TemplateLineRow.end() - 1; itLine++)
				{

					ResizeShift_TemplateLineRow.push_back(int((*itLine - TemplateLineRow.back())*ResizedTemplateMatchHeight / TemplateMatchHeight));
				}
				ResizeShift_TemplateLineRow.push_back(0);
				//vector <int> Resize_Shift_TemplateLineRow;
				//for (vector<int>::iterator itLine = TemplateLineRow.begin(); itLine != TemplateLineRow.end(); itLine++)
				//{
				//	Resize_Shift_TemplateLineRow.push_back(int((*itLine - *TemplateLineRow.begin())*CurrentMatchScale));
				//}

				bool FlagNextStep = false;
				vector<int> TemplateMatchStep;
				for (int iStep = *PeakRow_LineRow.begin(); iStep < InputImageHeight + ResizedTemplateMatchHeight; iStep++)
				{
					for (vector<int> ::iterator itTemplateLine = ResizeShift_TemplateLineRow.begin();
						itTemplateLine != ResizeShift_TemplateLineRow.end();
						itTemplateLine++)
					{
						if (*itTemplateLine + iStep >= 0 && *itTemplateLine + iStep < InputImageHeight)
						{
							for (vector<int> ::iterator itInputLine = PeakRow_LineRow.begin();
								itInputLine != PeakRow_LineRow.end();
								itInputLine++)
							{
								if (*itTemplateLine + iStep == *itInputLine)
								{
									FlagNextStep = true;
									break;
								}
							}
						}
						if (FlagNextStep == true) break;
					}
					if (FlagNextStep == true)
					{
						FlagNextStep = false;
						TemplateMatchStep.push_back(iStep);
						continue;
					}
				}
				int InputMatchBegin, InputMatchEnd, TemplateMatchBegin, TemplateMatchEnd;

				for (vector<int>::iterator itStep = TemplateMatchStep.begin();
					itStep != TemplateMatchStep.end();
					itStep++)
				{
					InputMatchBegin = *ResizeShift_TemplateLineRow.begin() + *itStep < 0 ?
						0 : *ResizeShift_TemplateLineRow.begin() + *itStep;
					InputMatchEnd = *ResizeShift_TemplateLineRow.rbegin() + *itStep < InputImageHeight ?
						*ResizeShift_TemplateLineRow.rbegin() + *itStep : InputImageHeight;

					TemplateMatchBegin = InputMatchBegin - (*ResizeShift_TemplateLineRow.begin() + *itStep);
					TemplateMatchEnd = InputMatchEnd - (*ResizeShift_TemplateLineRow.begin() + *itStep);


					if (double(TemplateMatchEnd - TemplateMatchBegin + 1) / ResizedTemplateMatchHeight < 1.0 / MaxMatchScale)
					{
						continue;
					}

					Mat InputData = Binary_ProjectX_Grad(Range(InputMatchBegin, InputMatchEnd), Range::all());
					Mat TemplateData = Resize_TemplateGrad(Range(TemplateMatchBegin, TemplateMatchEnd), Range::all());

					Mat  DataMeanMat, DataStdDevMat;
					cv::meanStdDev(InputData, DataMeanMat, DataStdDevMat);
					float InputDataMean = float(DataMeanMat.at<double>(0, 0));
					float InputDataStdDev = float(DataStdDevMat.at<double>(0, 0));
					cv::meanStdDev(TemplateData, DataMeanMat, DataStdDevMat);
					float TemplateDataMean = float(DataMeanMat.at<double>(0, 0));
					float TemplateDataStdDev = float(DataStdDevMat.at<double>(0, 0));

					double CorrCoef = 0.0;
					for (int iRow = 0; iRow < InputData.rows; iRow++)
					{
						CorrCoef = CorrCoef + InputData.at<float>(iRow, 0) * TemplateData.at<float>(iRow, 0);
					}

					CorrCoef = (CorrCoef - InputData.rows*InputDataMean*TemplateDataMean) /
						(InputData.rows * InputDataStdDev * TemplateDataStdDev);

					if (CorrCoef > MaxCorrCoef)
					{
						MaxCorrCoef = CorrCoef;
						DilatestMatchScale = CurrentMatchScale;

						for (int  iLineRow = 0;
							iLineRow < ResizeShift_TemplateLineRow.size();
							iLineRow++)
						{
							SegmentLineRow[iLineRow] = (ResizeShift_TemplateLineRow[iLineRow] + *itStep);
						}
					}

				}
			}

		}

		InputImageSegmentResult = InputImageTemp.clone();
		for (int iLineRow = 0; iLineRow < SegmentLineRow.size(); iLineRow++)
		{
			if (SegmentLineRow[iLineRow] >= 0 && SegmentLineRow[iLineRow] < InputImageHeight)
			{
				if (iLineRow == 1)
				{
					InputImageSegmentResult.row(SegmentLineRow[iLineRow]) = Scalar(0, 255, 0);
				}
				else
				{
					InputImageSegmentResult.row(SegmentLineRow[iLineRow]) = Scalar(255, 0, 0);
				}
			}

		}
		//改到这里 下面需要判断行是否完整 如果完整的话求梯度的沿Y轴方向的投影 比较大小
		vector <double> LineDutyRatio;
		double MinDutyRatio = 1.0;


		int TemplateXWidth = int(DilatestMatchScale*InputImageHeight*TemplateImageRatio);
		Mat Binary_LineGrad;
		Mat  Binary_ProjectY_LineGrad(
			int(SegmentLineRow.size()),
			InputImageWidth,
			CV_8UC1,
			Scalar(0.0)
		);
		Mat  ProjectY_Binary_LineGrad(
			int(SegmentLineRow.size()),
			InputImageWidth,
			CV_8UC1,
			Scalar(0.0)
		);

		Mat Binary_TitleLineGrad;
		bool IsTitleLineFound = false;
		bool IsPlateNumberLineFound = false;
		int TitleLineIndex;

		int PlateNumberLineIndex;

		for (int iLine = 0; iLine < SegmentLineRow.size() - 1; iLine++)
		{
			 if (SegmentLineRow[iLine] < 0)
			{
				 SegmentLineRow[iLine] = 0;
			}
			else if (SegmentLineRow[iLine] >= InputImageHeight)
			{
				SegmentLineRow[iLine] = InputImageHeight - 1;
			}

			if (SegmentLineRow[iLine + 1] < 0)
			{
				SegmentLineRow[iLine + 1] = 0;
			}
			else if (SegmentLineRow[iLine + 1] >= InputImageHeight)
			{
				SegmentLineRow[iLine + 1] = InputImageHeight - 1;
			}

		
			if (SegmentLineRow[iLine]<SegmentLineRow[iLine + 1])
			{
				threshold(
					InputImage_Grad(Range(SegmentLineRow[iLine], SegmentLineRow[iLine + 1]), Range::all()),
					Binary_LineGrad,
					128, //迭代初始阈值
					255, //最大值（超过阈值将设为此值）
					CV_THRESH_OTSU //阈值化选择的方法:Otsu法
				);

				if (iLine == 0)
				{
					IsTitleLineFound = true;
					Binary_TitleLineGrad = Binary_LineGrad.clone();
					TitleLineIndex = iLine;

				}
				if (iLine == 1)
				{
					IsPlateNumberLineFound = true;
					//Binary_TitleLineGrad = Binary_LineGrad.clone();
					PlateNumberLineIndex = iLine;

				}


				for (int iCol = 0; iCol < InputImageWidth; iCol++)
				{
					SumTemp = 0;
					for (int iRow = 0; iRow < SegmentLineRow[iLine + 1] - SegmentLineRow[iLine]; iRow++)
					{
						SumTemp = SumTemp + Binary_LineGrad.at<uchar>(iRow, iCol);
					}
					Binary_ProjectY_LineGrad.at<uchar>(iLine, iCol) = uchar(SumTemp / (SegmentLineRow[iLine + 1] - SegmentLineRow[iLine]));
				}

				threshold(
					Binary_ProjectY_LineGrad.row(iLine),
					ProjectY_Binary_LineGrad.row(iLine),
					128, //迭代初始阈值
					1, //最大值（超过阈值将设为此值）
					CV_THRESH_OTSU //阈值化选择的方法:Otsu法
				);



				SumTemp = 0;
				for (int iCol = 0;
					iCol < InputImageWidth;
					iCol++)
				{

					SumTemp = SumTemp + ProjectY_Binary_LineGrad.row(iLine).at<uchar>(0, iCol)/255.0;
				}
				LineDutyRatio.push_back(SumTemp / TemplateXWidth);
				//if (LineDutyRatio.back()< MinDutyRatio)
				//{
				//	MinDutyRatio = LineDutyRatio.back();
				//	MinDutyRatioLineIndex = iLine;
				//}

			}
			else
			{
				LineDutyRatio.push_back(NAN);
			}

		}


		double TitleLineDutyRatio = 0.458213270;
		double PlateNumberLineDutyRatio = 0.412103742;
		double OwnerLineDutyRatio = 0.139289141;
		vector<double>::iterator itMinLineDutyRatio = min_element(LineDutyRatio.begin(),LineDutyRatio.end());
		if (distance(LineDutyRatio.begin(), itMinLineDutyRatio) >2)
		{
			if (*(itMinLineDutyRatio - 2) / *(itMinLineDutyRatio - 1) >= 0.6*TitleLineDutyRatio / PlateNumberLineDutyRatio  &&
				*(itMinLineDutyRatio - 2) / *(itMinLineDutyRatio - 1) < 1.4*TitleLineDutyRatio / PlateNumberLineDutyRatio  &&
				*(itMinLineDutyRatio) / *(itMinLineDutyRatio - 1) >= 0.6*PlateNumberLineDutyRatio / OwnerLineDutyRatio  &&
				*(itMinLineDutyRatio) / *(itMinLineDutyRatio - 1) < 1.4*PlateNumberLineDutyRatio / OwnerLineDutyRatio)
			{
				PlateNumberLineIndex = int(distance(LineDutyRatio.begin(), itMinLineDutyRatio) - 1);
				TitleLineIndex = PlateNumberLineIndex - 1;
				IsTitleLineFound = true;
				IsPlateNumberLineFound = true;

			}

		}
		else if (distance(LineDutyRatio.begin(), itMinLineDutyRatio) ==0)
		{
			LineDutyRatio.erase(itMinLineDutyRatio);
			itMinLineDutyRatio = min_element(LineDutyRatio.begin(), LineDutyRatio.end());
			if (distance(LineDutyRatio.begin(), itMinLineDutyRatio) >=2)
			{
				if (*(itMinLineDutyRatio - 2) / *(itMinLineDutyRatio - 1) >= 0.6*TitleLineDutyRatio / PlateNumberLineDutyRatio  &&
					*(itMinLineDutyRatio - 2) / *(itMinLineDutyRatio - 1) < 1.4*TitleLineDutyRatio / PlateNumberLineDutyRatio  &&
					*(itMinLineDutyRatio) / *(itMinLineDutyRatio - 1) >= 0.6*PlateNumberLineDutyRatio / OwnerLineDutyRatio  &&
					*(itMinLineDutyRatio) / *(itMinLineDutyRatio - 1) < 1.4*PlateNumberLineDutyRatio / OwnerLineDutyRatio)
				{
					PlateNumberLineIndex = int(distance(LineDutyRatio.begin(), itMinLineDutyRatio) -1);
					IsPlateNumberLineFound = true;

					TitleLineIndex = PlateNumberLineIndex -1;
					IsTitleLineFound = true;
					
				}

			}
		}

		Mat ProjectY_PlateNumberLineGrad = Binary_ProjectY_LineGrad.row(PlateNumberLineIndex);


		//int TemplateImageHeadBlankStartCol = 0;
		//int TemplateImagePlateNumberAnnotationStartCol = 37;
		//int TemplateImagePlateNumberBlankStartCol = 159;
		//int TemplateImageVehicleTypeAnnotationStartCol = 452;
		//int TemplateImageVehicleTypeBlankStartCol = 575;
		//int TemplateImageTailBlanStartCol = 1003;
		//int TemplateImageTailBlanEndCol = 1042;

		int TemplateImageTitleWidth  = 832 - 193 + 1;
		double TemplateImageTitleLineHeight = TemplateImageLineGapHeight;
		int  TitleLineHeight = SegmentLineRow[TitleLineIndex + 1] - SegmentLineRow[TitleLineIndex] + 1;
			//int TemplateXLeftEdgeCol = 0;
			//int TemplateXTitleStartCol = int(double(TemplateImageTitleStartCol) /
			//	(TemplateImageWidth - 1)*(TemplateXWidth - 1));
			//int TemplateXTitleEndCol = int(double(TemplateImageTitleEndCol) /
			//	(TemplateImageWidth - 1)*(TemplateXWidth - 1));
			//int TemplateXRightEdgeCol = TemplateXWidth;
		int  EstimatedTitleWidth = int(double(TitleLineHeight) / double(TemplateImageTitleLineHeight)*double(TemplateImageTitleWidth));

		Mat ProjectY_TitleLineGrad = Binary_ProjectY_LineGrad.row(TitleLineIndex);

		Mat Dilate_ProjectY_TitleLineGrad(
			1,
			InputImageWidth,
			CV_8UC1,
			Scalar(0)
		);
		Mat ProjectY_Binary_TitleLineGrad(
			1,
			InputImageWidth,
			CV_8UC1,
			Scalar(0)
			);
		//TODO：从这里开始编程找到最合适的标题左右范围 方法为逐个像素长度步进先膨胀后腐蚀 找到最接近的行分割估计出的长度
		int TitleStartCol = 0, TitleEndCol= 0;
		int LastTitleStartCol = 0, LastTitleEndCol = 0;
		Mat Binary_Dilate_TitleLineGrad = Binary_TitleLineGrad.clone();
		if (IsTitleLineFound == true)
		{
			//IsTitleLineExist = false;
			Mat DilateStructingElement;
			//进行形态学闭操作
			int TitleWidthTemp = 0;
			int TitleStartColTemp, TitleEndColTemp;
			bool IsDilateLoopExitByBreak;
			for (int iElementSize = 3; iElementSize < EstimatedTitleWidth / 10.0; iElementSize = iElementSize + 2)
			{

				if (((TitleEndCol - TitleStartCol + 1) - EstimatedTitleWidth)*
					((LastTitleEndCol - LastTitleStartCol + 1) - EstimatedTitleWidth) <= 0)
				{
					IsDilateLoopExitByBreak = true;
					break;
				}
				LastTitleStartCol = TitleStartCol;
				LastTitleEndCol = TitleEndCol;
				DilateStructingElement = getStructuringElement(MORPH_RECT, Size(iElementSize, 1));
				morphologyEx(Binary_TitleLineGrad, Binary_Dilate_TitleLineGrad, MORPH_DILATE, DilateStructingElement);
			
				for (int iCol = 0; iCol < InputImageWidth; iCol++)
				{
					SumTemp = 0;
					for (int iRow = 0; iRow < Binary_Dilate_TitleLineGrad.rows; iRow++)
					{
						SumTemp = SumTemp + Binary_Dilate_TitleLineGrad.at<uchar>(iRow, iCol);
					}
					Dilate_ProjectY_TitleLineGrad.at<uchar>(0, iCol) = uchar(SumTemp / Binary_Dilate_TitleLineGrad.rows);
				}

				threshold(
					Dilate_ProjectY_TitleLineGrad,
					ProjectY_Binary_TitleLineGrad,
					128, //迭代初始阈值
					255, //最大值（超过阈值将设为此值）
					CV_THRESH_OTSU //阈值化选择的方法:Otsu法
				);

				if (ProjectY_Binary_TitleLineGrad.at<uchar>(0, 0) = 255)
				{
					TitleStartColTemp = 0;
				}
				for (int iCol = 1; iCol < ProjectY_Binary_TitleLineGrad.cols; iCol++)
				{
					if (ProjectY_Binary_TitleLineGrad.at<uchar>(0, iCol) == 255 && ProjectY_Binary_TitleLineGrad.at<uchar>(0, iCol - 1) == 0)
					{
						TitleStartColTemp = iCol;

					}
					else if (ProjectY_Binary_TitleLineGrad.at<uchar>(0, iCol) == 0 && ProjectY_Binary_TitleLineGrad.at<uchar>(0, iCol - 1) == 255)
					{
						TitleEndColTemp = iCol;
						if (TitleEndColTemp - TitleStartColTemp + 1 > (TitleEndCol - TitleStartCol +  iElementSize))
						{
							TitleStartCol = TitleStartColTemp + iElementSize/2;
							TitleEndCol = TitleEndColTemp - TitleStartColTemp + 1 + TitleStartCol - iElementSize;
						}
					}
				}

			}

			rectangle(
				InputImageSegmentResult,
				Point(LastTitleStartCol, SegmentLineRow[TitleLineIndex]),
				Point(LastTitleEndCol, SegmentLineRow[TitleLineIndex+1]),
				Scalar(0, 0, 0),
				1,
				LINE_AA,
				0
			);

			rectangle(
				InputImageSegmentResult,
				Point(TitleStartCol, SegmentLineRow[TitleLineIndex]),
				Point(TitleEndCol, SegmentLineRow[TitleLineIndex + 1]),
				Scalar(0, 0, 0),
				1,
				LINE_AA,
				0
			);


			if (IsDilateLoopExitByBreak == true)
			{
				if (abs(TitleEndCol - TitleStartCol + 1 - EstimatedTitleWidth) >
					abs(LastTitleEndCol - LastTitleStartCol + 1 - EstimatedTitleWidth))
				{
					TitleStartCol = LastTitleStartCol;
					TitleEndCol = LastTitleEndCol;
				}
			}

			rectangle(
				InputImageSegmentResult,
				Point(TitleStartCol, SegmentLineRow[TitleLineIndex]),
				Point(TitleEndCol, SegmentLineRow[TitleLineIndex + 1]),
				Scalar(0, 0, 255),
				1,
				LINE_AA,
				0
			);

			int TemplateImageTitleStartCol = 193;
			int TemplateImageTitleEndCol = 832;
			int TemplateImageTitleCenterCol = int((TemplateImageTitleStartCol + TemplateImageTitleEndCol)/2.0);
			Rect TemplateImagePlateNumberAreaRect = Rect(159, 106, 426 - 159, 228 - 106);
			int TemplateImagePlateNumberAreaCenterCol = TemplateImagePlateNumberAreaRect.x + TemplateImagePlateNumberAreaRect.width / 2;
			
			Rect InputImagePlateNumberAreaRect;
			int LicenseWidth = int(double(TitleEndCol - TitleStartCol + 1) /
				double(TemplateImageTitleEndCol - TemplateImageTitleStartCol + 1)*TemplateImageWidth);
			double InputImageHorizonScale = double(LicenseWidth) / double(InputImageWidth);
			if (InputImageHorizonScale >= MinMatchScale &&
				InputImageHorizonScale < MaxMatchScale)
			{
				InputImagePlateNumberAreaRect.y = SegmentLineRow[PlateNumberLineIndex];
				InputImagePlateNumberAreaRect.x = int((TitleStartCol + TitleEndCol)/2.0 - (TemplateImageTitleCenterCol - TemplateImagePlateNumberAreaCenterCol)/double(TemplateImageWidth)
					*InputImageWidth*InputImageHorizonScale  - double(TemplateImagePlateNumberAreaRect.width) / double(TemplateImageWidth)*InputImageWidth*InputImageHorizonScale/2.0);
				InputImagePlateNumberAreaRect.height = int(double(TemplateImagePlateNumberAreaRect.height) / double(TemplateImageHeight)*InputImageHeight*DilatestMatchScale);
				InputImagePlateNumberAreaRect.width = int(double(TemplateImagePlateNumberAreaRect.width) / double(TemplateImageWidth)*InputImageWidth*InputImageHorizonScale);
			}

			rectangle(
				InputImageSegmentResult,
				Point(InputImagePlateNumberAreaRect.x, InputImagePlateNumberAreaRect.y),
				Point(InputImagePlateNumberAreaRect.x + InputImagePlateNumberAreaRect.width,
					InputImagePlateNumberAreaRect.y + InputImagePlateNumberAreaRect.height),
				Scalar(0, 0, 255),
				1,
				LINE_AA,
				0
			);

			Mat InputImage_PlateNumberAreaGrad = InputImage_Grad(InputImagePlateNumberAreaRect);
			Mat Binary_PlateNumberAreaGrad;
			threshold(
				InputImage_PlateNumberAreaGrad,
				Binary_PlateNumberAreaGrad,
				128, //迭代初始阈值
				255, //最大值（超过阈值将设为此值）
				CV_THRESH_OTSU //阈值化选择的方法:Otsu法
			);

			Mat  Binary_Dilate_PlateNumberAreaGrad;
			Mat Dilate_ProjectX_PlateNumberAreaGrad
			(
				Binary_PlateNumberAreaGrad.cols,
				1,
				CV_8UC1,
				Scalar(0)
			);
			Mat ProjectX_Binary_PlateNumberAreaGrad;
			int PlateNumberStartRowTemp = 0;
			int PlateNumberEndRowTemp = 0;
			int PlateNumberStartRow = 0, PlateNumberEndRow = 0;
			int LastPlateNumberStartRow = 0, LastPlateNumberEndRow = 0;
			int TemplateImagePlateNumberLineHeight = 83;
			int TemplateImagePlateNumberHeight = 39;
			int EstimatePlateNumberHeight = int ( double(SegmentLineRow[PlateNumberLineIndex+1] - SegmentLineRow[PlateNumberLineIndex] + 1) / double(TemplateImagePlateNumberLineHeight)*double( TemplateImagePlateNumberHeight));

			for (int iElementSize = 3; iElementSize < EstimatePlateNumberHeight / 6.0; iElementSize = iElementSize + 2)
			{
				if (((PlateNumberEndRow - PlateNumberStartRow + 1) - EstimatePlateNumberHeight)*
					((LastPlateNumberEndRow - LastPlateNumberStartRow + 1) - EstimatePlateNumberHeight) <= 0)
				{
					break;
				}
				LastPlateNumberStartRow = PlateNumberStartRow;
				LastPlateNumberEndRow = PlateNumberEndRow;
				DilateStructingElement = getStructuringElement(MORPH_RECT, Size( 1,iElementSize));
				morphologyEx(Binary_PlateNumberAreaGrad, Binary_Dilate_PlateNumberAreaGrad, MORPH_DILATE, DilateStructingElement);
				for (int iRow = 0; iRow < Binary_Dilate_PlateNumberAreaGrad.rows; iRow++)
				{

					SumTemp = 0;
					for (int iCol = 0; iCol < Binary_Dilate_PlateNumberAreaGrad.cols; iCol++)
					{
						SumTemp = SumTemp + Binary_Dilate_PlateNumberAreaGrad.at<uchar>(iRow, iCol);
					}
					Dilate_ProjectX_PlateNumberAreaGrad.at<uchar>(iRow,0) = uchar(SumTemp / Binary_Dilate_TitleLineGrad.cols);
				}

				threshold(
					Dilate_ProjectX_PlateNumberAreaGrad,
					ProjectX_Binary_PlateNumberAreaGrad,
					128, //迭代初始阈值
					255, //最大值（超过阈值将设为此值）
					CV_THRESH_OTSU //阈值化选择的方法:Otsu法
				);

				if (Dilate_ProjectX_PlateNumberAreaGrad.at<uchar>(0, 0) = 255)
				{
					PlateNumberStartRowTemp = 0;
				}
				for (int iRow = 1; iRow < ProjectX_Binary_PlateNumberAreaGrad.rows; iRow++)
				{
					if (ProjectX_Binary_PlateNumberAreaGrad.at<uchar>(iRow,0) == 255 && ProjectX_Binary_PlateNumberAreaGrad.at<uchar>(iRow-1,0) == 0)
					{
						PlateNumberStartRowTemp = iRow;

					}
					else if (ProjectX_Binary_PlateNumberAreaGrad.at<uchar>(iRow,0) == 0 && ProjectX_Binary_PlateNumberAreaGrad.at<uchar>(iRow-1,0) == 255)
					{
						PlateNumberEndRowTemp = iRow;
						if (PlateNumberEndRowTemp - PlateNumberStartRowTemp + 1 >= (PlateNumberEndRow - PlateNumberStartRow + 1))
						{
							PlateNumberStartRow = PlateNumberStartRowTemp;
							PlateNumberEndRow = PlateNumberEndRowTemp;
						}
					}
				}
			}

			//int PlateNumberStartRowTemp = 0;
			//int PlateNumberEndRowTemp = 0;
			//int PlateNumberStartRow, PlateNumberEndRow;
			//int PlateNumberHeight = 0;

			//for (int iRow = 1; iRow < ProjectX_PlateNumberAreaGrad.rows; iRow++)
			//{
			//	if (ProjectX_PlateNumberAreaGrad.at<uchar>(iRow, 0) == 255 && ProjectX_PlateNumberAreaGrad.at<uchar>(iRow - 1, 0) == 0)
			//	{
			//		PlateNumberStartRowTemp = iRow;
			//	}
			//	else if (ProjectX_PlateNumberAreaGrad.at<uchar>(iRow, 0) == 0 && ProjectX_PlateNumberAreaGrad.at<uchar>(iRow - 1, 0) == 255)
			//	{
			//		PlateNumberEndRowTemp = iRow;
			//	}
			//	if ((PlateNumberEndRowTemp - PlateNumberStartRowTemp + 1) > PlateNumberHeight)
			//	{
			//		PlateNumberStartRow = PlateNumberEndRowTemp;
			//		PlateNumberEndRow = PlateNumberEndRowTemp;
			//	}
			//}

			InputImagePlateNumberAreaRect.y = PlateNumberStartRow;
			InputImagePlateNumberAreaRect.height = PlateNumberEndRow - PlateNumberStartRow + 1;
			rectangle(
				InputImageSegmentResult,
				Point(InputImagePlateNumberAreaRect.x, InputImagePlateNumberAreaRect.y),
				Point(InputImagePlateNumberAreaRect.x + InputImagePlateNumberAreaRect.width,
					InputImagePlateNumberAreaRect.y + InputImagePlateNumberAreaRect.height),
				Scalar(0, 0, 255),
				1,
				LINE_AA,
				0
			);

			InputImage_PlateNumberAreaGrad = InputImage_Grad(InputImagePlateNumberAreaRect);

			threshold(
				InputImage_PlateNumberAreaGrad,
				Binary_PlateNumberAreaGrad,
				128, //迭代初始阈值
				255, //最大值（超过阈值将设为此值）
				CV_THRESH_OTSU //阈值化选择的方法:Otsu法
			);

			Mat Dilate_ProjectY_PlateNumberAreaGrad(
				1,
				InputImageWidth,
				CV_8UC1,
				Scalar(0)
			);
			Mat ProjectY_Binary_PlateNumberAreaGrad
			(
				1,
				InputImageWidth,
				CV_8UC1,
				Scalar(0)
			);

			int PlateNumberStartColTemp = 0;
			int PlateNumberEndColTemp = 0;
			int PlateNumberStartCol =0 , PlateNumberEndCol=0;
			int LastPlateNumberStartCol = 0, LastPlateNumberEndCol = 0;
			int PlateNumberWidth = 0;
			int TemplateImagePlateNumberWidth = 175;
			int EstimatePlateNumberWidth = int(double(PlateNumberEndRow - PlateNumberStartRow + 1) / double(TemplateImagePlateNumberHeight)*double(TemplateImagePlateNumberWidth));

			for (int iElementSize = 3; iElementSize < EstimatePlateNumberWidth / 6.0; iElementSize = iElementSize + 2)
			{
				if (((PlateNumberEndCol - PlateNumberStartCol + 1) - EstimatePlateNumberWidth)*
					((LastTitleEndCol - LastTitleStartCol + 1) - EstimatePlateNumberWidth) <= 0)
				{
					break;
				}
				LastTitleStartCol = TitleStartCol;
				LastTitleEndCol = TitleEndCol;
				DilateStructingElement = getStructuringElement(MORPH_RECT, Size(iElementSize, 1));
				morphologyEx(Binary_PlateNumberAreaGrad, Binary_Dilate_PlateNumberAreaGrad, MORPH_DILATE, DilateStructingElement);

				for (int iCol = 0; iCol < InputImageWidth; iCol++)
				{
					SumTemp = 0;
					for (int iRow = 0; iRow < Binary_Dilate_PlateNumberAreaGrad.rows; iRow++)
					{
						SumTemp = SumTemp + Binary_Dilate_PlateNumberAreaGrad.at<uchar>(iRow, iCol);
					}
					Dilate_ProjectY_PlateNumberAreaGrad.at<uchar>(0, iCol) = uchar(SumTemp / Binary_Dilate_TitleLineGrad.rows);
				}

				threshold(
					Dilate_ProjectY_PlateNumberAreaGrad,
					ProjectY_Binary_PlateNumberAreaGrad,
					128, //迭代初始阈值
					255, //最大值（超过阈值将设为此值）
					CV_THRESH_OTSU //阈值化选择的方法:Otsu法
				);

				if (Dilate_ProjectY_PlateNumberAreaGrad.at<uchar>(0, 0) = 255)
				{
					PlateNumberStartColTemp = 0;
				}
				for (int iCol = 1; iCol < ProjectY_Binary_PlateNumberAreaGrad.cols; iCol++)
				{
					if (ProjectY_Binary_PlateNumberAreaGrad.at<uchar>(0, iCol) == 255 && ProjectY_Binary_PlateNumberAreaGrad.at<uchar>(0, iCol - 1) == 0)
					{
						PlateNumberStartColTemp = iCol;

					}
					else if (ProjectY_Binary_PlateNumberAreaGrad.at<uchar>(0, iCol) == 0 && ProjectY_Binary_PlateNumberAreaGrad.at<uchar>(0, iCol - 1) == 255)
					{
						PlateNumberEndColTemp = iCol;
						if (PlateNumberEndColTemp - PlateNumberStartColTemp + 1 >= (PlateNumberEndCol - PlateNumberStartCol + 1))
						{
							PlateNumberStartCol = PlateNumberStartColTemp;
							PlateNumberEndCol = PlateNumberEndColTemp;
						}
					}
				}
			}


			InputImagePlateNumberAreaRect.x = PlateNumberStartCol;
			InputImagePlateNumberAreaRect.height = PlateNumberEndCol - PlateNumberStartCol + 1;
			rectangle(
				InputImageSegmentResult,
				Point(InputImagePlateNumberAreaRect.x, InputImagePlateNumberAreaRect.y),
				Point(InputImagePlateNumberAreaRect.x + InputImagePlateNumberAreaRect.width,
					InputImagePlateNumberAreaRect.y + InputImagePlateNumberAreaRect.height),
				Scalar(0, 0, 255),
				1,
				LINE_AA,
				0
			);



		}

		
		//寻找文件路径最后一个“\”
		size_t SepPos = InputImagePath.rfind('\\');//rfind 反向查找
												   //获取文件夹路径
		string FolderPath = InputImagePath.substr(0, SepPos);
		//获取图片文件名
		string ImageFileName = InputImagePath.substr(SepPos + 1, -1);
		//获取输出图片保存路径（文件名为输入图像名称前面加上“Result_”）
		string OutputImagePath = OutputPath + "Result_" + ImageFileName;
		//进行图像保存
		imwrite(OutputImagePath, InputImageSegmentResult);
	}
	//返回0并正常退出程序
	return 0;
}