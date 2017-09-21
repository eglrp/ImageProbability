#pragma once
#include <vector>
#include "stdafx.h"
using namespace std;
class GFImage;
class GFHistogram
{
public:
	vector<double> m_vBinNums;
public:
	GFHistogram(){}
	GFHistogram(const GFImage& image,int nBins,int nChannelIdx);
	void Calculate(const GFImage& image,int nBins,int nChannelIdx);
	int GetBins() const
	{
		return m_nBins;
	}
	int GetTotNum() const
	{
		return m_nTot;
	}
	double GetFrequencyAt(int nBinIdx)
	{
		return m_vBinNums[nBinIdx];
	}
private:
	int m_nBins;
	int m_nTot;

	
};