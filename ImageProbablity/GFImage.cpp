#include "stdafx.h"
#include "GFImage.h"
GFImage::GFImage(const string& strPath)
{
	m_img = cv::imread(strPath);
}

GFImage::GFImage(const GFImage& image)
{
	m_img = image.m_img;
}

GFImage GFImage::Clone()
{
	GFImage image;
	image.m_img = m_img.clone();
	return image;
}

GFImage& GFImage::operator = (const GFImage& image)
{
	if (this == &image)
	{
		return *this;
	}
	m_img = image.m_img;
	return *this;
}

bool GFImage::BinaryTransform(uchar thresHold)
{
	assert(thresHold >=0 && thresHold <= 255);
	if (thresHold == 255)
	{
		return ContrastStretching(0,0,255,0);
	}
	return ContrastStretching(thresHold,0,thresHold + 1,255);
}

GFImage::GFImage(int nWidth,int nHeight,int nChannel)
{
	m_img.create(nHeight,nWidth,CV_8UC(nChannel));
}
bool GFImage::GetBitImages(vector<GFImage>& vBitImages)
{
	uchar mask[8] = {
		0x80,0x40,0x20,0x10,
		0x08,0x04,0x02,0x01
	};


	vBitImages.clear();
	for (int i = 0;i < 8;i++)
	{
		GFImage image(GetWidth(),GetHeight(),GetChannel());
		for (int r = 0; r < GetHeight();r++)
		{
			uchar * pLine = GetData() + r * GetWidthStep();
			uchar * pDstLine = image.GetData() + r * image.GetWidthStep();
			for (int c = 0; c < GetWidth(); c++)
			{
				for (int ch = 0; ch < GetChannel(); ch ++)
				{
					uchar val = pLine[c * GetChannel() + ch];
					if (val & mask[i])
					{
						pDstLine[c * GetChannel() + ch] = 255;
					}else
					{
						pDstLine[c * GetChannel() + ch] = 0;
					}
				}
			}
		}
		vBitImages.push_back(image);
	}
	return true;
}

bool GFImage::ContrastStretching(uchar r1,uchar s1,
								 uchar r2,uchar s2)
{

	int nWidth = GetWidth();
	int nHeight = GetHeight();
	int nStep = GetWidthStep();
	int nChannel = GetChannel();

	uchar * pData = GetData();
	double k1 = 100000;
	double k12 = 100000;
	double k2 = 100000;
	if (r1 != 0 )
	{
		k1 = s1 / double(r1);
	}
	
	if (r2!=r1)
	{
		k12 = (s2 - s1) / double(r2 - r1);
	}
	if (255 != r2)
	{
		k2 = (255 - s2) / (255 - r2);
	}

	uchar mapVals[256];
	for (int i = 0;i < 256;i++)
	{
		if (i < r1)
		{
			mapVals[i] = (uchar)(k1 * i);
		}else if (i < r2)
		{
			mapVals[i] = (uchar)(s1 + (i - r1) * k12);
		}else{
			mapVals[i] = (uchar)(s2 + (i - r2) * k2);
		}
		//cout << i << ":" << (int)mapVals[i]<<endl;
		if (mapVals[i]!=i)
		{
			//cout << "changed" <<endl;
		}
	}
	for (int r = 0;r < nHeight; r++)
	{
		uchar * pLine = pData + r * nStep;
		for (int c = 0; c < nWidth; c++)
		{
			for (int channel = 0; channel < nChannel; channel++)
			{
				uchar val = pLine[nChannel * c + channel];
				pLine[nChannel * c + channel] = mapVals[val];
			}

		}
	}
	return true;	


}
bool GFImage::LogTransform(double param_c)
{
	int nWidth = GetWidth();
	int nHeight = GetHeight();
	int nStep = GetWidthStep();
	int nChannel = GetChannel();

	uchar * pData = GetData();

	for (int r = 0;r < nHeight; r++)
	{
		uchar * pLine = pData + r * nStep;
		for (int c = 0; c < nWidth; c++)
		{
			for (int channel = 0; channel < nChannel; channel++)
			{
				double val = pLine[nChannel * c + channel];
				pLine[nChannel * c + channel] = uchar(param_c * log(val + param_c));
			}

		}
	}
	return true;	

}

bool GFImage::GammaTransform(double gamma,double param_c)
{
	int nWidth = GetWidth();
	int nHeight = GetHeight();
	int nStep = GetWidthStep();
	int nChannel = GetChannel();

	uchar * pData = GetData();

	for (int r = 0;r < nHeight; r++)
	{
		uchar * pLine = pData + r * nStep;
		for (int c = 0; c < nWidth; c++)
		{
			for (int channel = 0; channel < nChannel; channel++)
			{
				double val = pLine[nChannel * c + channel];
				double newVal = param_c * pow(val,gamma);
				if (newVal >= 255)
				{
					pLine[nChannel * c + channel] = 255;
				}else{
					pLine[nChannel * c + channel] = uchar(newVal);
				}
			}

		}
	}
	return true;	

}
bool GFImage::Gray()
{

	int nWidth = GetWidth();
	int nHeight = GetHeight();
	int nStep = GetWidthStep();
	int nChannel = GetChannel();
	GFImage grayImage(nWidth,nHeight,1);

	uchar * pSrcData = GetData();
	uchar * pDstData = grayImage.GetData();
	for (int r = 0;r < nHeight; r++)
	{
		uchar * pLine = pSrcData + r * nStep;
		uchar * pDstLine = pDstData + r * grayImage.GetWidthStep();
		for (int c = 0; c < nWidth; c++)
		{
			int nTotValue = 0;
			for (int channel = 0; channel < nChannel; channel++)
			{
				nTotValue += pLine[nChannel * c + channel];
			}
			pDstLine[c] = nTotValue / nChannel;
		}
	}

	*this = grayImage;
	return true;	
}

bool GFImage::Revert()
{
	int nWidth = GetWidth();
	int nHeight = GetHeight();
	int nStep = GetWidthStep();
	int nChannel = GetChannel();
	uchar * pData = GetData();

	for (int r = 0;r < nHeight; r++)
	{
		uchar * pLine = pData + r * nStep;
		for (int c = 0; c < nWidth; c++)
		{
			for (int channel = 0; channel < nChannel; channel++)
			{
				pLine[nChannel * c + channel] = 255 - pLine[nChannel * c + channel];
			}
		}
	}
	return true;
}

void GFImage::ShowImage(const string& strWinName)
{
	cv::namedWindow(strWinName, CV_WINDOW_AUTOSIZE); //创建窗口  
	cv::imshow(strWinName, m_img); //显示图像  
}

bool GFImage::HistogramEqualization()
{
	vector<vector<uchar> > pixMaps;
	CalculateMapFunByHisEq(pixMaps);
	for (int ch = 0; ch < GetChannel(); ch++)
	{
		uchar * pData = GetData();
		for (int r = 0;r < GetHeight(); r++)
		{
			uchar * pLine = pData + r * GetWidthStep();
			for (int c = 0; c < GetWidth(); c++)
			{
				uchar val = pLine[GetChannel() * c + ch];
				pLine[GetChannel() * c + ch] = pixMaps[ch][val];
			}
		}
	}
	return true;
}
bool GFImage::CalculateMapFunByHisEq(vector<vector<uchar> >& vMappings) const
{
	vMappings.resize(GetChannel());
	for (int i = 0; i < vMappings.size(); i++)
	{
		vMappings[i].resize(256);
	}

	vector<GFHistogram> vHistograms;
	vHistograms.resize(GetChannel());
	for (int i = 0;i < GetChannel(); i++)
	{
		vHistograms[i].Calculate(*this, 256, i);
	}
	double tmp;
	for (int ch = 0; ch < GetChannel(); ch++)
	{
		tmp = vHistograms[ch].GetFrequencyAt(0) * 255;
		vMappings[ch][0] = (uchar)tmp;
		for (int j = 1;j < 256; j++)
		{
			tmp = tmp + vHistograms[ch].GetFrequencyAt(j) * 255;
			vMappings[ch][j] = (uchar)tmp;
		}
	}
	return true;
}

bool GFImage::HistogramMatching(const GFImage& anoImage)
{
	assert(GetChannel() == anoImage.GetChannel());
	//r->s
	vector<vector<uchar> > vRSMap;
	CalculateMapFunByHisEq(vRSMap);
	//z->s
	vector<vector<uchar> > vZSMap;
	anoImage.CalculateMapFunByHisEq(vZSMap);

	vector<vector<uchar> > vRZMap;
	vRZMap.resize(GetChannel());
	for(int ch = 0; ch < GetChannel(); ch++)
	{
		vRZMap[ch].resize(256);
	}

	for (int ch = 0; ch < GetChannel(); ch++)
	{
		vector<int> vSZMap;
		vSZMap.resize(256);
		for (int i = 0;i < 256;i++)
		{
			vSZMap[i] = -1;
		}
		for (int z = 255; z>=0; z--)
		{
			vSZMap[vZSMap[ch][z]] = z;
		}
		vector<int> vSIndex;
		//加前哨
		vSIndex.push_back(-1);
		for (int s = 0; s< 256 ;s++)
		{
			if (vSZMap[s] != -1)
			{
				vSIndex.push_back(s);
			}
		}
		//加后哨
		vSIndex.push_back(256);
		for (int i = 0; i < vSIndex.size() - 1; i++)
		{
			int startIdx = vSIndex[i] + 1;
			int endIdx = vSIndex[i + 1] - 1;

			int lowerIdx = vSIndex[i];
			int upperIdx = vSIndex[i + 1];

			if (i == 0)
			{
				lowerIdx = upperIdx;
			}
			if ( i == vSIndex.size() - 2)
			{
				upperIdx = lowerIdx;
			}

			int nLen = endIdx - startIdx + 1;
			int nMidIdx = startIdx + nLen / 2;
			for (int j = startIdx; j < nMidIdx; j++)
			{
				vSZMap[j] = vSZMap[lowerIdx];
			}
			for (int j = nMidIdx; j <= endIdx ; j++)
			{
				vSZMap[j] = vSZMap[upperIdx];
			}			
		}
		
		for (int r = 0; r < 256; r++)
		{
			vRZMap[ch][r] = vSZMap[vRSMap[ch][r]];	
		}
	}

	for (int ch = 0; ch < GetChannel(); ch++)
	{
		uchar * pData = GetData();
		for (int r = 0;r < GetHeight(); r++)
		{
			uchar * pLine = pData + r * GetWidthStep();
			for (int c = 0; c < GetWidth(); c++)
			{
				uchar val = pLine[GetChannel() * c + ch];
				pLine[GetChannel() * c + ch] = vRZMap[ch][val];
			}
		}
	}
	return true;

}