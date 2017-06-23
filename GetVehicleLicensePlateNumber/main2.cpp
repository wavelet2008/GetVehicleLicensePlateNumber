//#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <cmath>

//使用C++标准库命名空间
using namespace std;

//使用OpenCV库命名空间
using namespace cv;

//从模板图片中获得的位置信息
//模板图片的宽、高
const double TemplateWidth = 550;
const double TemplateHeight = 371;
//模板图片标题所在区域的宽、高及区域中心的XY坐标
const double TemplateTitleWidth = 340;
const double TemplateTitleHeight = 36;
const double TemplateTitleCenterX = 270;
const double TemplateTitleCenterY = 39;
//模板图片车牌区域的宽、高及区域中心的XY坐标
const double TemplatePlateWidth = 84;
const double TemplatePlateHeight = 20;
const double TemplatePlateCenterX = 138;
const double TemplatePlateCenterY = 81;


//主函数，输入命令行参数为待处理图片的路径
int main(int ArgumentCount, char** ArgumentVector)
{

	//检查命令行所带参数数目是否正确，如果不正确则显示用法说明并退出程序
	if (ArgumentCount != 3)
	{
		//显示程序用法说明
		cout << " Usage:  " << ArgumentVector[0] << "ImagePathGlobExpression" << "OutputPath" << endl;
		//返回错误码并退出程序
		return -1;
	}

	vector<String> ImagePathList;
	String SearchGlobExpression = ArgumentVector[1];
	String OutputPath = ArgumentVector[2];

	glob(SearchGlobExpression,//文件查找Glob表达式
		ImagePathList, //输出图像文件列表
		false//不进行递归，即不对子文件夹进行搜索
	);


	for (size_t iImage = 0; iImage<ImagePathList.size(); iImage++)
	{
		//新建矩阵RawImageMat用于存储原始图片数据
		Mat RawImageMat;

		//根据第一个参数的文件路径进行图片读取
		RawImageMat = imread(
			ImagePathList[iImage],//输入图片路径
			CV_LOAD_IMAGE_UNCHANGED//以不修改通道类型的方式读取图片
		);

		//检查读入的Mat是否包含图像数据
		if (!RawImageMat.data)
		{
			//显示图片读取失败提示信息
			cout << " Error:  Can't read image data from" << ArgumentVector[1] << endl;
			//返回错误码并退出程序
			return -1;
		}
		//计算输入图像的宽、高、面积和宽高比
		const double InputImageWidth = RawImageMat.cols;
		const double InputImageHeight = RawImageMat.rows;


		//计算模板图片标题的宽高比和面积
		const float TemplateTitleRatio = TemplateTitleWidth / TemplateTitleHeight;
		const float TemplateTitleArea = TemplateTitleWidth*TemplateTitleHeight;

		//计算模板的车牌区域的宽高比和面积
		const float TemplatePlateRatio = TemplatePlateWidth / TemplatePlateHeight;
		const float TemplatePlateArea = TemplatePlateWidth*TemplatePlateHeight;

		//将图片转换成灰阶图像
		Mat GrayImageMat;
		//获取图片的通道数
		int NumRawImageChannel = RawImageMat.channels();

		//如果图像为3通道彩色图像
		if (NumRawImageChannel == 3)
		{
			//将图片由BGR转换成灰阶图像
			cvtColor(
				RawImageMat,//输入图片矩阵
				GrayImageMat,//输出图片矩阵 
				COLOR_BGR2GRAY//将图片由BGR（OpenCV默认通道格式）转换成灰阶图像
			);
		}
		//如果图像已经为4通道（包含alpha通道）图像，则将其转换成灰阶图像
		else if (NumRawImageChannel == 4)
		{
			//将图片由BGRA转换成灰阶图像
			cvtColor(
				RawImageMat,
				GrayImageMat,
				COLOR_BGRA2GRAY//将图片由BGRA转换成灰阶图像
			);
		}
		//如果图像已经为单通道灰阶图像，直接不作转换将ResizedImageMat赋给GrayImageMat
		else if (NumRawImageChannel == 1)
		{
			GrayImageMat = RawImageMat;
		}

		//如果通道数不为1,3或4，输出错误码并退出程序
		else
		{
			cout << "Unkown image channel type: " << RawImageMat.channels();
		}


		//指定窗口名称
		const char* WindowName = "get plate number from vehicle license";
		//创建相应的窗口
		namedWindow(
			WindowName,//窗口名称
			CV_WINDOW_NORMAL//建立一个根据图片自动设置大小的窗口，但不能手动修改尺寸
		);
		double WindowWidth = 600.0;
		resizeWindow(WindowName, int(GrayImageMat.cols / GrayImageMat.rows*WindowWidth), WindowWidth);
		//在上述窗口中显示灰阶图像
		imshow(
			WindowName,//希望绘制图片的窗口名称
			GrayImageMat//希望绘制的图片矩阵
		);

		//等待键盘响应，参数0表示等待时间为无限长
		waitKey(0);


		////对图像进行高斯模糊
		////设置高斯模糊核尺寸-----------------------------------------
		//double GaussianBlurKernelSize = 11;
		////-----------------------------------------------------------
		//Mat BlurGrayImageMat;
		//GaussianBlur(
		//	GrayImageMat, //输入图片矩阵
		//	BlurGrayImageMat, //输出图片矩阵
		//					  //Size(int(GaussianBlurKernelSize), int(GaussianBlurKernelSize)), //高斯核尺寸
		//	Size(5, 5),//高斯核尺寸必须是正值且是奇数
		//	0, //??高斯核X方向标准差 
		//	0, //??高斯核Y方向标准差
		//	BORDER_DEFAULT //??像素插值方式 
		//);

		////在上述窗口中显示高斯模糊后的图像
		//imshow(
		//	WindowName,//希望绘制图片的窗口名称
		//	BlurGrayImageMat//希望绘制的图片矩阵
		//);

		//等待键盘响应（按下任意键），参数0表示等待时间为无限长
		//waitKey(0);



		Mat GrayImageGradX(GrayImageMat.rows, GrayImageMat.cols, CV_8UC1, Scalar(0));
		for (size_t iRow = 1; iRow<GrayImageMat.rows-1; iRow++)
		{
			for (size_t iCol = 1; iCol<(GrayImageMat.cols - 1); iCol++)
			{
				GrayImageGradX.at<uchar>(iRow, iCol) = (2 * abs(GrayImageMat.at<uchar>(iRow, iCol - 1) - GrayImageMat.at<uchar>(iRow, iCol + 1))
					+ abs(GrayImageMat.at<uchar>(iRow - 1, iCol - 1) - GrayImageMat.at<uchar>(iRow - 1, iCol + 1))
					+ abs(GrayImageMat.at<uchar>(iRow + 1, iCol - 1) - GrayImageMat.at<uchar>(iRow + 1, iCol + 1)))/4;
			}
		}


		//梯度矩阵二值化
		threshold(
			GrayImageGradX, //输入矩阵
			GrayImageGradX, //输出矩阵
			128, //迭代初始阈值
			255, //最大值（超过阈值将设为此值）
			CV_THRESH_OTSU //阈值化选择的方法:Otsu法
		);

		//显示边缘二值图像
		imshow(
			WindowName,
			GrayImageGradX
		);

		//等待键盘响应，参数0表示等待时间为无限长
		waitKey(0);

		//创建X梯度投影向量
		Mat  GradXProject(
			GrayImageGradX.rows,//矩阵行数
			1,//矩阵列数
			CV_8UC1,//矩阵值的类型（8位无符号整数单通道）
			Scalar(0)//矩阵填入的初始值
		);

		double SumTemp;
		for (size_t iRow = 0; iRow<GradXProject.rows; iRow++)
		{
			SumTemp = 0;
			for (size_t iCol = 0; iCol< GrayImageGradX.cols; iCol++)
			{
				SumTemp += double(GrayImageGradX.at<uchar>(iRow , iCol));
			}
			GradXProject.at<uchar>(iRow, 0) = unsigned char(SumTemp / GrayImageGradX.cols);
		}

		threshold(
			GradXProject, //输入矩阵
			GradXProject, //输出矩阵
			128, //迭代初始阈值
			255, //最大值（超过阈值将设为此值）
			CV_THRESH_OTSU //阈值化选择的方法:Otsu法
		);


		//显示边缘二值图像
		imshow(
			WindowName,
			GradXProject
		);

		//等待键盘响应，参数0表示等待时间为无限长
		waitKey(0);

		size_t iRow = 0;
		vector<size_t> RisingRow , FallingRow;
		for (size_t iRow = 0; iRow < GradXProject.rows;iRow++)
		{
			for (; iRow < GradXProject.rows - 1 && GradXProject.at<uchar>(iRow, 0) == 0 && GradXProject.at<uchar>(iRow + 1, 0) == 255; iRow++);
			if (iRow < GradXProject.rows - 1)
			{
				RisingRow.push_back(iRow);
				iRow++;
			}
			for (; iRow < GradXProject.rows - 1 && GradXProject.at<uchar>(iRow + 1, 0) == 0; iRow++);
			if (iRow < GradXProject.rows - 1)
			{
				FallingRow.push_back(iRow);
			}
		}

		size_t TextHeightMin = 10;

		vector<size_t>::iterator itTextRowStart = RisingRow.begin(), itTextRowEnd = FallingRow.begin();

		for (;itTextRowStart != RisingRow.end() && itTextRowEnd != FallingRow.end(); )
		{
			if (*(itTextRowStart)-*(itTextRowEnd) <= TextHeightMin)
			{
				itTextRowStart = RisingRow.erase(itTextRowStart);
				itTextRowEnd = RisingRow.erase(itTextRowEnd);
			}
			else
			{
				itTextRowStart++;
				itTextRowEnd++;
			}
			
		}

		Mat GrayImageGradY(GrayImageMat.rows, GrayImageMat.cols, CV_8UC1, Scalar(0));
		for (size_t iRow = 1; iRow<GrayImageMat.rows - 1; iRow++)
		{
			for (size_t iCol = 1; iCol<(GrayImageMat.cols - 1); iCol++)
			{
				GrayImageGradY.at<uchar>(iRow, iCol) = (2 * abs(GrayImageMat.at<uchar>(iRow-1, iCol) - GrayImageMat.at<uchar>(iRow+1, iCol))
					+ abs(GrayImageMat.at<uchar>(iRow - 1 , iCol - 1) - GrayImageMat.at<uchar>(iRow + 1, iCol - 1))
					+ abs(GrayImageMat.at<uchar>(iRow - 1, iCol + 1) - GrayImageMat.at<uchar>(iRow + 1, iCol + 1))) / 4;
			}
		}


		//梯度矩阵二值化
		threshold(
			GrayImageGradY, //输入矩阵
			GrayImageGradY, //输出矩阵
			128, //迭代初始阈值
			255, //最大值（超过阈值将设为此值）
			CV_THRESH_OTSU //阈值化选择的方法:Otsu法
		);

		//显示边缘二值图像
		imshow(
			WindowName,
			GrayImageGradY
		);

		//等待键盘响应，参数0表示等待时间为无限长
		waitKey(0);

		//创建X梯度投影向量
		Mat  GradYProject(
			1,//矩阵行数
			GrayImageGradY.rows,//矩阵列数
			CV_8UC1,//矩阵值的类型（8位无符号整数单通道）
			Scalar(0)//矩阵填入的初始值
		);

		double SumTemp;
		for (size_t iCol = 0; iCol<GradYProject.cols; iCol++)
		{
			SumTemp = 0;
			for (size_t iRow = 0; iRow< GrayImageGradY.rows; iRow++)
			{
				SumTemp += double(GrayImageGradY.at<uchar>(iRow, iCol));
			}
			GradYProject.at<uchar>(0,iCol) = unsigned char(SumTemp / GrayImageGradY.rows);
		}

		threshold(
			GradYProject, //输入矩阵
			GradYProject, //输出矩阵
			128, //迭代初始阈值
			255, //最大值（超过阈值将设为此值）
			CV_THRESH_OTSU //阈值化选择的方法:Otsu法
		);


		//显示边缘二值图像
		imshow(
			WindowName,
			GradYProject
		);

		//等待键盘响应，参数0表示等待时间为无限长
		waitKey(0);


		for (size_t iRow = 2; iRow< EdgeXProject.rows - 2; iRow++)
		{

			if (BinaryEdgeXProject.at<uchar>(iRow - 2, 0) == 0 && BinaryEdgeXProject.at<uchar>(iRow - 1, 0) == 0 &&
				BinaryEdgeXProject.at<uchar>(iRow, 0) == 0 && BinaryEdgeXProject.at<uchar>(iRow + 1, 0) == 0 &&
				BinaryEdgeXProject.at<uchar>(iRow + 2, 0) == 0)

			{
				for (size_t iCol = CropRect.x; iCol < CropRect.x + CropRect.width; iCol++)
				{
					GrayImageEdge.at<uchar>(CropRect.y + iRow, iCol) = 0;
				}
			}
		}

		//显示边缘二值图像
		imshow(
			WindowName,
			GrayImageEdge
		);

		//等待键盘响应，参数0表示等待时间为无限长
		//waitKey(0);




		//对灰阶图像的边缘进行水平方向的膨胀
		//设置膨胀操作结构元的尺寸-----------------------------------
		double DilateElementWidth = 9;
		//-----------------------------------------------------------
		//构造膨胀操作的结构元，构建一个1行DilateElementWidth列的结构元以完成水平膨胀
		Mat DilateElement = Mat(
			1,//第一维（Y轴尺寸）
			int(DilateElementWidth),//第二维（X轴尺寸）
			CV_8U,//矩阵类型：8位无符号整数
			cvScalar(1)//填充的数值，全0
		);
		//对边缘进行膨胀操作
		Mat DilatedGrayImageEdge;
		dilate(
			GrayImageEdge,//输入矩阵
			DilatedGrayImageEdge,//输出矩阵
			DilateElement,//膨胀结构元
			Point(-1, -1), //膨胀元锚点，在这里取中心作为锚点
			1 //进行膨胀的次数
		);
		//显示膨胀后的二值图像
		imshow(
			WindowName,
			DilatedGrayImageEdge
		);

		//等待键盘响应，参数0表示等待时间为无限长
		//waitKey(0);



		//对水平膨胀后的图像进行轮廓提取
		vector<vector<Point> > AllContour;
		vector<Vec4i> ContourHierarchy;
		findContours(DilatedGrayImageEdge,//输入图像矩阵
			AllContour, //输出轮廓向量
			ContourHierarchy,//输出轮廓等级 
			RETR_EXTERNAL,//只提取外部轮廓（忽略其中包含的内部轮廓） 
			CHAIN_APPROX_SIMPLE//轮廓的近似办法，在此选择CHAIN_APPROX_SIMPLE，它压缩水平方向，垂直方向，对角线
							   //方向的元素，只保留该方向的终点坐标，例如一个矩形轮廓只需4个点来保存轮廓信息
		);

		//sort(AllContour.begin(),AllContour.begin()+2,CompareContourCenter);
		////连通域检测之后在原图上绘制图片裁切的框
		rectangle(ResizedImageMat,//绘制矩形的对象
			CropRect, //要绘制的矩形
			Scalar(255, 255, 255), //线条的颜色
			3,//线条宽度
			CV_AA,//线型（抗混叠） 
			0 //??坐标数值（二进制）的小数位数
		);

		//边长误差限制-----------------------------------------------
		double LengthErrorLimit = 0.3;
		//角度误差限制
		float AngleErrorLimit = 5;
		//-----------------------------------------------------------

		//借助模板估计标题行中心点在输入图像中的位置
		double EstimatedTitleCenterX = (ResizedImageWidth - CropRect.width) / 2.0 +
			TemplateTitleCenterX / TemplateWidth *  CropRect.width;
		double EstimatedTitleCenterY = (ResizedImageHeight - CropRect.height) / 2.0 +
			TemplateTitleCenterY / TemplateHeight *  CropRect.height;

		//使用iContour遍历提取到的轮廓
		size_t iContour = 0;

		//记录是否找到标题行和车牌区域的标志变量
		bool FlagFoundTitle = false;
		bool FlagFoundPlate = false;

		//保存根据标题区域估计出来的车牌区域中心点坐标、宽和高
		double EstimatedPlateCenterX;
		double EstimatedPlateCenterY;


		//保存轮廓外接矩阵的 高度、宽度、面积、中心点位置和旋转角
		double ContourRectHeight;
		double ContourRectWidth;
		double ContourRectRotateAngle;
		double ContourArea;
		double ContourRectCenterX;
		double ContourRectCenterY;
		double ContourRectAngle;

		//保存轮廓的最小外接矩形（可旋转）
		RotatedRect  ContourRect;

		//保存判断轮廓外接矩形是否符合宽高比例、宽度、高度、中心点XY位置,以及面积大小的标志变量
		bool IsWidthMatch, IsHeightMatch, IsCenterXMatch, IsCenterYMatch, IsAreaMatch;

		//保存第一次遍历（寻找标题区域）时顺便进行的车牌尺寸宽高比和尺寸检查中符合要求的轮廓序号（第二次检测只需遍历这部分轮廓）
		vector<int> PlateToBe;
		//遍历PlateToBe的迭代器
		vector<int>::iterator itContour;

		//当未找到标题区域时且还有未检测轮廓时继续循环
		while (FlagFoundTitle == false && iContour < AllContour.size())
		{
			////绘制遍历过的轮廓
			//drawContours (ResizedImageMat,//绘制对象
			//	AllContour,//轮廓点数组
			//	iContour,//绘制轮廓的索引/序号
			//	Scalar(255,0,0),//绘制的线的颜色
			//	1,//绘制的线宽
			//	CV_AA,//线型：抗混叠
			//	ContourHierarchy,//轮廓的等级
			//	0,//只绘制当前级别的轮廓
			//	Point(0,0)//轮廓在水平和垂直方向的偏置
			//	);

			//imshow(WindowName,ResizedImageMat);
			//waitKey(0);

			//获取轮廓的最小外接矩形（可旋转）
			ContourRect = minAreaRect(AllContour[iContour]);

			//获取外接矩阵的高度、宽度、旋转角、面积和中心点位置
			ContourRectHeight = ContourRect.size.height;
			ContourRectWidth = ContourRect.size.width;
			ContourRectRotateAngle = ContourRect.angle;

			Moments ContourMoment = moments(AllContour[iContour], false);
			ContourRectCenterX = ContourMoment.m10 / ContourMoment.m00;
			ContourRectCenterY = ContourMoment.m01 / ContourMoment.m00;

			//计算轮廓包围的面积
			ContourArea = contourArea(AllContour[iContour],//输入轮廓
				false//是否根据轮廓的旋转角（顺时针还是逆时针）计算带符号的面积值，这里选否
			);

			//对外接矩形的旋转角进行重新映射（-90～0 -> -45~45），使其更直观
			//当旋转角小于-45证明矩形被顺时针旋转了
			if (ContourRect.angle < -45.0)
			{
				//将其加90度转换为正值
				ContourRectAngle = ContourRect.angle + 90.0;
				//并将宽高值交换以符合直觉（详见RotatedRect的angle属性的定义）
				swap(ContourRectWidth, ContourRectHeight);
			}

			ContourRectWidth = ContourArea / ContourRectHeight;

			//注意：因为前面做了水平方向的膨胀操作，所以对这里将宽度减去一个膨胀元的宽度
			ContourRectWidth = ContourRectWidth - DilateElementWidth;

			//先判断是否符合车牌号码区域的宽高比和面积比例并储存起来以避免后面车牌区域搜索重复运算
			//如果轮廓外接矩形的宽高比和模板相差比例不超过LengthErrorLimit，水平倾斜不超过AngleErrorLimit度（顺时针为正，旋转角度为-90+AngleErrorLimit～-90度，此时竖边被识别为宽）
			//IsRatioMatch = (fabs(ContourRectWidth/ ContourRectHeight - TemplatePlateRatio ) / TemplatePlateRatio <= LengthErrorLimit && ContourRectRotateAngle <= -90+AngleErrorLimit) ||
			//或轮廓外接矩形的宽高比和模板相差比例不超过LengthErrorLimit，水平倾斜不超过AngleErrorLimit度（逆时针为负，旋转角度为-0～-1*AngleErrorLimit度，此时横边被识别为宽）
			//(fabs(ContourRectWidth / ContourRectHeight - TemplatePlateRatio) / TemplatePlateRatio <= LengthErrorLimit && ContourRectRotateAngle >= -1*AngleErrorLimit);

			//宽和高占图片的比例与模板相差不超过LengthErrorLimit
			IsWidthMatch = fabs(ContourRectWidth / CropRect.width - TemplatePlateWidth / TemplateWidth) / (TemplatePlateWidth / TemplateWidth) <= LengthErrorLimit;
			IsHeightMatch = fabsf(ContourRectHeight / CropRect.height - TemplatePlateHeight / TemplateHeight) / (TemplatePlateHeight / TemplateHeight) <= LengthErrorLimit;
			//轮廓围成的面积与模板相差不超过LengthErrorLimit*(2+LengthErrorLimit)
			if (ContourArea >= TemplateTitleArea)
			{
				IsAreaMatch = fabsf(ContourArea - TemplatePlateArea) / (TemplatePlateArea) <= fabsf(LengthErrorLimit*(2.0 + LengthErrorLimit));
			}
			else
			{
				IsAreaMatch = fabsf(ContourArea - TemplatePlateArea) / (TemplatePlateArea) <= fabs(LengthErrorLimit*(LengthErrorLimit - 2.0));
			}

			//如果区域满足车牌区域的高度及面积要求，即将其压入准车牌区域矢量PlateToBe中（第二次遍历将只检测此区域）
			//IsRatioMatch && IsWidthMatch &&
			if (IsWidthMatch && IsHeightMatch && IsAreaMatch)
			{
				PlateToBe.push_back(iContour);
			}

			//如果轮廓外接矩形的宽高比和模板相差比例不超过LengthErrorLimit，水平倾斜不超过AngleErrorLimit度（顺时针为正，旋转角度为-90+AngleErrorLimit～-90度，此时竖边被识别为宽）
			//IsRatioMatch = fabs(ContourRectWidth / ContourRectHeight - TemplateTitleRatio) / TemplateTitleRatio <= LengthErrorLimit ;

			//宽和高占图片的比例与模板相差不超过LengthErrorLimit
			IsWidthMatch = fabs(ContourRectWidth / CropRect.width - TemplateTitleWidth / TemplateWidth) / (TemplateTitleWidth / TemplateWidth) <= LengthErrorLimit;
			IsHeightMatch = fabsf(ContourRectHeight / CropRect.height - TemplateTitleHeight / TemplateHeight) / (TemplateTitleHeight / TemplateHeight) <= LengthErrorLimit;

			//且轮廓中心与根据模板估计出来的点在X轴和Y轴方向上的误差比例均不超过LengthErrorLimit
			IsCenterXMatch = (fabsf(ContourRectCenterX - EstimatedTitleCenterX) / EstimatedTitleCenterX <= LengthErrorLimit);
			IsCenterYMatch = (fabsf(ContourRectCenterY - EstimatedTitleCenterY) / EstimatedTitleCenterY <= LengthErrorLimit);
			//轮廓围成的面积与模板相差不超过LengthErrorLimit*(2+LengthErrorLimit)
			if (ContourArea >= TemplateTitleArea)
			{
				IsAreaMatch = fabsf(ContourArea - TemplateTitleArea) / (TemplateTitleArea) <= fabsf(LengthErrorLimit*(2.0 + LengthErrorLimit));
			}
			else
			{
				IsAreaMatch = fabsf(ContourArea - TemplateTitleArea) / (TemplateTitleArea) <= fabs(LengthErrorLimit*(LengthErrorLimit - 2.0));
			}


			//如果上述条件均满足，则认为当前轮廓为标题所对应的轮廓
			if (IsWidthMatch && IsHeightMatch  && IsCenterXMatch &&IsCenterYMatch && IsAreaMatch)
				////对提取到的轮廓进行筛选
				//if (
				//	//如果轮廓外接矩形的宽高比和模板相差比例不超过LengthErrorLimit，水平倾斜不超过AngleErrorLimit度（顺时针为正，旋转角度为-90+AngleErrorLimit～-90度，此时竖边被识别为宽）
				//	(fabs(ContourRectHeight / ContourRectWidth - TemplateTitleRatio ) / TemplateTitleRatio <= LengthErrorLimit && ContourRectRotateAngle <= -90+AngleErrorLimit) ||
				//	//或轮廓外接矩形的宽高比和模板相差比例不超过LengthErrorLimit，水平倾斜不超过AngleErrorLimit度（逆时针为负，旋转角度为-0～-1*AngleErrorLimit度，此时横边被识别为宽）
				//	(fabs(ContourRectWidth / ContourRectHeight - TemplateTitleRatio) / TemplateTitleRatio <= LengthErrorLimit && ContourRectRotateAngle >= -1*AngleErrorLimit) &&
				//	//且轮廓面积占裁切后输入图片面积的比例与模板相差不超过(2+LengthErrorLimit)*LengthErrorLimit
				//	(fabs(ContourRectArea / CroppedInputImageArea - TemplateTitleArea/TemplateArea) / (TemplateTitleArea/TemplateArea) <= (2+LengthErrorLimit)*LengthErrorLimit) &&
				//	//且轮廓中心与根据模板估计出来的点在X轴和Y轴方向上的误差比例均不超过LengthErrorLimit
				//	(fabs(ContourRectCenterX - EstimatedTitleCenterX) / EstimatedTitleCenterX <= LengthErrorLimit) &&
				//	(fabs(ContourRectCenterY - EstimatedTitleCenterY) / EstimatedTitleCenterY <= LengthErrorLimit)
				//	)//满足上述条件我们便将此轮廓当作标题行所在的位置
			{
				//将FlagFoundTitle置true表示我们已经找到了标题行对应的轮廓
				FlagFoundTitle = true;
				//在原始图像上绘制出这一轮廓
				drawContours(ResizedImageMat,//绘制对象
					AllContour,//轮廓点数组21
					iContour,//绘制轮廓的索引/序号
					Scalar(255, 0, 0),//绘制的线的颜色：蓝色
					3,//绘制的线宽
					CV_AA,//线型：抗混叠
					ContourHierarchy,//轮廓的等级   00000.
					0,//只绘制当前级别的轮廓
					Point(0, 0)//轮廓在水平和垂直方向的偏置
				);
				CropRect.height = int(ContourRectHeight / TemplateTitleHeight * TemplateHeight);
				CropRect.width = TemplateRatio	* CropRect.height;

				CropRect.x = int(ContourRectCenterX - (TemplateTitleCenterX / TemplateWidth)* CropRect.width);
				CropRect.y = int(ContourRectCenterY - (TemplateTitleCenterY / TemplateWidth)* CropRect.height);



				rectangle(ResizedImageMat,//绘制矩形的对象
					CropRect, //要绘制的矩形
					Scalar(255, 0, 0), //线条的颜色
					3,//线条宽度
					CV_AA,//线型（抗混叠） 
					0 //??坐标数值（二进制）的小数位数
				);

				imshow(WindowName, ResizedImageMat);
				//waitKey(0);


				//根据找到的标题区域估计车牌号区域中心点坐标
				EstimatedPlateCenterX = CropRect.x + (TemplatePlateCenterX) / TemplateWidth  * CropRect.width;
				EstimatedPlateCenterY = CropRect.y + (TemplatePlateCenterY) / TemplateHeight * CropRect.height;
				//EstimatedPlateWidth = TemplatePlateHeight/TemplateWidth * CropRect.width;
				//EstimatedPlateHeight = TemplatePlateHeight/TemplateHeight * CropRect.height;

			}

			////顺时针倾斜情况
			//if (ContourRectRotateAngle <= -90+AngleErrorLimit)
			//{
			//	//从标题行中心水平和垂直方向移动至估计的车牌号对应的中心点
			//	EstimatedPlateCenterX = ContourRectCenterX -
			//		TemplateOffsetX/TemplateTitleWidth  * ContourRectWidth * cos(-1.0*ContourRectRotateAngle/180.0*pi);
			//    EstimatedPlateCenterY = ContourRectCenterY + 
			//		TemplateOffsetX/TemplateTitleHeight * ContourRectHeight * sin(-1.0*ContourRectRotateAngle/180.0*pi);
			//}
			//else if  (ContourRectRotateAngle >= -1*AngleErrorLimit)
			//{
			//	EstimatedPlateCenterX = ContourRectCenterX +
			//		TemplateOffsetX/TemplateTitleWidth  * ContourRectWidth * sin(-1.0*ContourRectRotateAngle/180.0*pi);
			//	EstimatedPlateCenterY = ContourRectCenterY + 
			//		TemplateOffsetY/TemplateTitleHeight * ContourRectHeight * cos(-1.0*ContourRectRotateAngle/180.0*pi);
			//}	


			//如果找到了标题区域,则开始进行车牌区域检测
			if (FlagFoundTitle == true)
			{
				//将迭代器赋为候选准车牌区域轮廓序号数组的开头
				itContour = PlateToBe.begin();

				//当未找到车牌区域或者还有区域未被检测时继续循环
				while (FlagFoundPlate == false && itContour != PlateToBe.end())
				{
					//取得当前循环的轮廓序号
					iContour = *(itContour);
					//drawContours (ResizedImageMat,//绘制对象
					//	AllContour,//轮廓点数组
					//	iContour,//绘制轮廓的索引/序号
					//	Scalar(0,0,255),//绘制的线的颜色
					//	1,//绘制的线宽
					//	CV_AA,//线型：抗混叠
					//	ContourHierarchy,//轮廓的等级
					//	0,//只绘制当前级别的轮廓
					//	Point(0,0)//轮廓在水平和垂直方向的偏置
					//	);

					//imshow(WindowName,ResizedImageMat);
					//waitKey(0);

					ContourRect = minAreaRect(AllContour[iContour]);

					//获取外接矩阵的高度、宽度、旋转角、面积和中心点位置
					ContourRectHeight = ContourRect.size.height;
					ContourRectWidth = ContourRect.size.width;
					ContourRectRotateAngle = ContourRect.angle;
					//计算轮廓包围的面积
					ContourArea = contourArea(AllContour[iContour],//输入轮廓
						false//是否根据轮廓的旋转角（顺时针还是逆时针）计算带符号的面积值，这里选否
					);
					ContourRectCenterX = ContourRect.center.x;
					ContourRectCenterY = ContourRect.center.y;

					//角度变换和宽高交换
					if (ContourRect.angle < -45.0)
					{
						ContourRectAngle = ContourRect.angle + 90.0;
						swap(ContourRectWidth, ContourRectHeight);
					}

					//注意：因为前面做了水平方向的膨胀操作，所以对这里宽度减去了一个膨胀元的宽度
					ContourRectWidth = ContourRectWidth - DilateElementWidth;

					//如果轮廓外接矩形的宽高比和模板相差比例不超过LengthErrorLimit，水平倾斜不超过AngleErrorLimit度（顺时针为正，旋转角度为-90+AngleErrorLimit～-90度，此时竖边被识别为宽）
					//IsRatioMatch = fabs(ContourRectWidth / ContourRectHeight  - TemplatePlateRatio ) / TemplatePlateRatio <= LengthErrorLimit ;
					////或轮廓外接矩形的宽高比和模板相差比例不超过LengthErrorLimit，水平倾斜不超过AngleErrorLimit度（逆时针为负，旋转角度为-0～-1*AngleErrorLimit度，此时横边被识别为宽）
					//(fabs(ContourRectWidth / ContourRectHeight - TemplatePlateRatio) / TemplatePlateRatio <= LengthErrorLimit && ContourRectRotateAngle >= -1*AngleErrorLimit);

					////宽和高占图片的比例与模板相差不超过LengthErrorLimit
					//IsWidthMatch = fabs(ContourRectWidth  / CroppedInputImageWidth  - TemplatePlateWidth/TemplateWidth) / (TemplatePlateWidth/TemplateWidth) <= LengthErrorLimit ;
					//IsHeightMatch = fabs(ContourRectHeight / CroppedInputImageHeight  - TemplatePlateHeight/TemplateHeight) / (TemplatePlateHeight/TemplateHeight) <= LengthErrorLimit ;

					//检测轮廓中心与根据模板估计出来的点在X轴和Y轴方向上的误差比例是否均不超过LengthErrorLimit
					IsCenterXMatch = (fabsf(ContourRectCenterX - EstimatedPlateCenterX) / EstimatedTitleCenterX <= LengthErrorLimit);
					IsCenterYMatch = (fabsf(ContourRectCenterY - EstimatedPlateCenterY) / EstimatedTitleCenterY <= LengthErrorLimit);

					//如果均不超过，则认为当前轮廓对应车牌号码所在的区域
					if (IsCenterXMatch &&IsCenterYMatch)
						//if (
						//	//如果轮廓外接矩形的宽高比和模板相差比例不超过LengthErrorLimit，水平倾斜不超过AngleErrorLimit度（顺时针为正，旋转角度为-90+AngleErrorLimit～-90度，此时竖边被识别为宽）
						//	(fabs(ContourRectHeight / ContourRectWidth - TemplatePlateRatio ) / TemplatePlateRatio <= LengthErrorLimit && ContourRectRotateAngle <= -90+AngleErrorLimit) ||
						//	//或轮廓外接矩形的宽高比和模板相差比例不超过LengthErrorLimit，水平倾斜不超过AngleErrorLimit度（逆时针为负，旋转角度为-0～-1*AngleErrorLimit度，此时横边被识别为宽）
						//	(fabs(ContourRectWidth / ContourRectHeight - TemplatePlateRatio) / TemplatePlateRatio <= LengthErrorLimit && ContourRectRotateAngle >= -1*AngleErrorLimit) &&
						//	//且轮廓面积占裁切后输入图片面积的比例与模板相差不超过(2+LengthErrorLimit)*LengthErrorLimit
						//	(fabs(ContourRectArea / CroppedInputImageArea - TemplatePlateArea/TemplateArea) / (TemplatePlateArea/TemplateArea) <= (2+LengthErrorLimit)*LengthErrorLimit) &&
						//	//且轮廓中心与根据模板估计出来的点在X轴和Y轴方向上的误差比例均不超过LengthErrorLimit
						//	(fabs(ContourRectCenterX - EstimatedPlateCenterX) / EstimatedPlateCenterX <= LengthErrorLimit) &&
						//	(fabs(ContourRectCenterY - EstimatedPlateCenterY) / EstimatedPlateCenterY <= LengthErrorLimit)
						//	)//满足上述条件我们便将此轮廓当作车牌号码所在的位置
					{
						//将找到车牌区域的标志变量置true，表明我们找到了车牌区域
						FlagFoundPlate = true;
						drawContours(ResizedImageMat,//绘制对象
							AllContour,//轮廓点数组
							iContour,//绘制轮廓的索引/序号
							Scalar(0, 0, 255),//绘制的线的颜色
							3,//绘制的线宽
							CV_AA,//线型：抗混叠
							ContourHierarchy,//轮廓的等级
							0,//S只绘制当前级别的轮廓
							Point(0, 0)//轮廓在水平和垂直方向的偏置
						);

						CropRect.height = int(ContourRectHeight / TemplatePlateHeight * TemplateHeight);
						CropRect.width = TemplateRatio	* CropRect.height;
						CropRect.x = int(ContourRectCenterX - (TemplatePlateCenterX / TemplateWidth)* CropRect.width);
						CropRect.y = int(ContourRectCenterY - (TemplatePlateCenterY / TemplateWidth)* CropRect.height);

						rectangle(ResizedImageMat,//绘制矩形的对象
							CropRect, //要绘制的矩形
							Scalar(0, 0, 255), //线条的颜色
							3,//线条宽度
							CV_AA,//线型（抗混叠） 
							0 //??坐标数值（二进制）的小数位数
						);


					}
					//将迭代器加1转而检测下一个轮廓
					itContour++;
				}
			}
			//将轮廓索引加1检测下一个轮廓
			iContour++;
		}
		imshow(WindowName, ResizedImageMat);

		//waitKey(0);

		int SepPos = ImagePathList[iImage].rfind('\\');//rfind 反向查找
		string FolderPath = ImagePathList[iImage].substr(0, SepPos);
		string ImageFileName = ImagePathList[iImage].substr(SepPos + 1, -1);
		string OutputImagePath = OutputPath + "\\结果" + ImageFileName;
		imwrite(OutputImagePath, ResizedImageMat);
	}
	//返回0并正常退出程序
	return 0;
}