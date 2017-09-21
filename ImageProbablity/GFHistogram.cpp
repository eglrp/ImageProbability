#include "stdafx.h"
#include "GFHistogram.h"
#include "GFImage.h"

GFHistogram::GFHistogram(const GFImage& image,int nBins,int nChannelIdx)
{
	Calculate(image,nBins,nChannelIdx);	
}

void GFHistogram::Calculate(const GFImage& image,int nBins,int nChannelIdx)
{
	m_nTot = 0;
	m_nBins = nBins;
	assert(nChannelIdx >=0 && nChannelIdx < image.GetChannel());

	m_vBinNums.resize(m_nBins);
	for (int i = 0; i < m_nBins;i++)
	{
		m_vBinNums[i] = 0;
	}
	for (int r = 0; r < image.GetHeight();r++)
	{
		uchar * pData = image.GetData() + image.GetWidthStep() * r;
		for (int c = 0; c < image.GetWidth();c ++)
		{
			uchar val = pData[c * image.GetChannel() + nChannelIdx];
			int idx = val / (256 / m_nBins);
			m_vBinNums[idx] ++;
			m_nTot ++;
		}
	}

	
	for (int i = 0;i < m_nBins; i++)
	{
		m_vBinNums[i] /= m_nTot;
	}

	double sum = 0;
	for (int i = 0;i < m_nBins; i++)
	{
		sum += m_vBinNums[i];
	}
}


