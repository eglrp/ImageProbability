#include "stdafx.h"
#include <iostream>
#include "Histmatch.h"
#include "GFImage.h"
#include <float.h>  
#include <algorithm>  
#include <deque>  
#include <fstream>  
#include <limits>  
#include <map>  
#include <stack>  
#include <vector>
#include <Windows.h>

using namespace std;

HistmatchedImge::HistmatchedImge()
{

}

void HistmatchedImge::Onhistmatch(const char* pathImage1, const char* pathImage2, const char* pathcvHM,const char* pathHistmatched)
{
	cout<<"第一步,直方图匹配...";
	GFImage image1(pathImage1);
	GFImage image2(pathImage2);
	image1.HistogramEqualization();
	image1.HistogramMatching(image2);
	image1.Save(pathcvHM);

/////////////////////////////////////////////////////////////////////////////////
	
	// 注册
	GDALAllRegister();

	GDALDataset* pOrginal=(GDALDataset*)GDALOpen(pathImage1,GA_ReadOnly);

	GDALDataset* cvhist=(GDALDataset*)GDALOpen(pathcvHM,GA_ReadOnly);

	//获取影像的信息
	int iSrcWidth1=cvhist->GetRasterXSize();
	int iSrcHeight1=cvhist->GetRasterYSize();
	int iSrcCount1=cvhist->GetRasterCount();

	GDALRasterBand *cvhistmatchR = cvhist->GetRasterBand(1);
    GDALDataType Imagetype=cvhistmatchR->GetRasterDataType();

	//  创建文件并设置空间参考和坐标信息	
	GDALDriver *poDriver=(GDALDriver*)GDALGetDriverByName("GTIFF");
	GDALDataset *pHistmatched=poDriver->Create(pathHistmatched,iSrcWidth1,iSrcHeight1,3,Imagetype,NULL);
	GDALRasterBand *HistMatchedR = cvhist->GetRasterBand(1);

	int panBandMap[3]={1,2,3};
	BYTE *pBuff=new BYTE[iSrcWidth1*iSrcHeight1*iSrcCount1];
	cvhist->RasterIO(GF_Read,0,0,iSrcWidth1,iSrcHeight1,pBuff,iSrcWidth1,iSrcHeight1,Imagetype,3,panBandMap,3,iSrcWidth1*3,1);
	pHistmatched->RasterIO(GF_Write,0,0,iSrcWidth1,iSrcHeight1,pBuff,iSrcWidth1,iSrcHeight1,Imagetype,3,panBandMap,3,iSrcWidth1*3,1);

	double adfGeotranceform[6];
	const char* proj;
	pOrginal->GetGeoTransform(adfGeotranceform);
	proj = pOrginal->GetProjectionRef();	
	pHistmatched->SetGeoTransform(adfGeotranceform);
	pHistmatched->SetProjection(proj);

	GDALClose((GDALDatasetH)pOrginal);
	GDALClose((GDALDatasetH)pHistmatched);
	GDALClose((GDALDatasetH)cvhist);

	cout<<"-->直方图匹配完成！"<<endl;
}