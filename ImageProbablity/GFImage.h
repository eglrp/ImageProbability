#pragma once
#include "opencv/highgui.h"
#include <string>
#include "GFHistogram.h"
using namespace std;
class GFImage
{
public:
	GFImage()
	{

	}
	GFImage(int nWidth,int nHeight,int nChannel);

	explicit GFImage(const string& strPath);
	GFImage(const GFImage& image);
	GFImage& operator = (const GFImage& image);
	void ShowImage(const string& strWinName);
	bool IsEmpty()
	{
		return m_img.empty();
	}

	GFImage Clone();
	int GetWidth() const
	{
		return m_img.cols;
	}
	int GetHeight()	const
	{
		return m_img.rows;
	}
	int GetChannel() const
	{
		return m_img.channels();
	}
	//每行有多少个字节
	int GetWidthStep() const
	{
		return m_img.step;
	}

	uchar * GetData() const
	{
		return m_img.data;
	}
	~GFImage()
	{

	}

	void Save(const string& strFilePath)
	{
		cv::imwrite(strFilePath,m_img);
	}
	//图像处理的操作
	//反色
	bool Revert();
	bool Gray();
	bool GammaTransform(double gamma,double c = 1);
	bool LogTransform(double c = 1);
	bool BinaryTransform(uchar thresHold);

	bool GetBitImages(vector<GFImage>& vBitImages);

	// r1->s1
	// r2->s2
	bool ContrastStretching(uchar r1,uchar s1,
							uchar r2,uchar s2);


	//计算每个通道的RGB直方图
	bool CalculateHistograms(vector<GFHistogram>& vHistograms);
	bool HistogramEqualization();

	bool HistogramMatching(const GFImage& anoImage);
protected:
private:
	bool CalculateMapFunByHisEq(vector<vector<uchar> >& vMapping) const;
	cv::Mat m_img; 
};