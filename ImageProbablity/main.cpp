// main.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

#include "ShpToRaster.h"
#include <cv.h>
#include <highgui.h>
#include "CDResult.h"
#include "SieveFilter.h"
#include "gdalwarper.h"


int getShpfeatureNumber(string shp_path)
{
	char* path_Shp = const_cast<char*>(shp_path.c_str());
	OGRRegisterAll();
	OGRDataSource *poDS;
	poDS = OGRSFDriverRegistrar::Open(path_Shp, true);
	if (poDS == NULL)
	{
		printf("Open failed.\n%s");
		OGRDataSource::DestroyDataSource(poDS);
		exit(1);
	}
	OGRLayer  *poLayer;
	poLayer = poDS->GetLayer(0);
	int fetureCount = poLayer->GetFeatureCount();
	if (fetureCount == 0)
	{
		printf("Feature is NULL !.\n%s");
		exit(1);
	}

	return fetureCount;
}
void Run(char *shpName, char* tifName, char *outName1)
{
	GDALAllRegister();
	OGRRegisterAll();

	GDALDataset *poDataTif;
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	poDataTif = (GDALDataset*)GDALOpen(tifName, GA_ReadOnly);
	int Width = poDataTif->GetRasterXSize();
	int Height = poDataTif->GetRasterYSize();

	double AdfGeoTransform[6];
	poDataTif->GetGeoTransform(AdfGeoTransform);
	const char* projRef;
	projRef = poDataTif->GetProjectionRef();

	OGRDataSource  *poDS;
	poDS = OGRSFDriverRegistrar::Open(shpName, FALSE);
	if (poDS == NULL)
	{
		return;
	}
	OGRLayer* poLayer;
	poLayer = poDS->GetLayer(0);
	char * pszProjection=NULL;
	OGRSpatialReference * poSRS = poLayer->GetSpatialRef();
	OGREnvelope  psExent;
	poLayer->GetExtent(&psExent);

	// 栅格化后的宽、高
	int m_nImageWidth = (psExent.MaxX - psExent.MinX) / AdfGeoTransform[1];
	int m_nImageHeight = (psExent.MinY - psExent.MaxY) / AdfGeoTransform[5];

	if (!poSRS)
	{
		m_nImageWidth = psExent.MaxX;
		m_nImageHeight = psExent.MaxY;
	}
	else
	{
		poSRS->exportToWkt(&pszProjection);
	}

	const char *pszFormat = "GTiff";
	GDALDriver *poDriver;
	poDriver = GetGDALDriverManager()->GetDriverByName("MEM");

	GDALDataset *poNewDS = poDriver->Create("", m_nImageWidth, m_nImageHeight, 1, GDT_Byte, NULL);
	if (!poSRS)
	{
		double adfGeoTransform[6];
		adfGeoTransform[0] = psExent.MinX;
		adfGeoTransform[1] = (psExent.MaxX - psExent.MinX) / m_nImageWidth;
		adfGeoTransform[2] = 0;
		adfGeoTransform[3] = psExent.MaxY;
		adfGeoTransform[4] = 0;
		adfGeoTransform[5] = (psExent.MinY - psExent.MaxY) / m_nImageHeight;
		GDALSetGeoTransform(poNewDS, adfGeoTransform);
	}
	else
	{
		double adfGeoTransform[6];
		adfGeoTransform[0] = psExent.MinX;
		adfGeoTransform[1] = (psExent.MaxX - psExent.MinX) / m_nImageWidth;
		adfGeoTransform[2] = 0;
		adfGeoTransform[3] = psExent.MaxY;
		adfGeoTransform[4] = 0;
		adfGeoTransform[5] = (psExent.MinY - psExent.MaxY) / m_nImageHeight;
		GDALSetGeoTransform(poNewDS, adfGeoTransform);
		poNewDS->SetProjection(pszProjection);
	}

	int * pnbandlist;
	pnbandlist = new int[1];
	pnbandlist[0] = 1;

	double *dburnValues = NULL;
	dburnValues = new double[1];
	dburnValues[0] = 255;

	OGRLayerH * player;
	player = new OGRLayerH[1];
	player[0] = (OGRLayerH)poLayer;
	char **papszOptions = NULL;

	//papszOptions = CSLSetNameValue( papszOptions, "CHUNKSIZE", "1" );  
	papszOptions = CSLSetNameValue(papszOptions, "ATTRIBUTE", "CCode");

	void * pTransformArg = NULL;
	void * m_hGenTransformArg = NULL;
	m_hGenTransformArg = GDALCreateGenImgProjTransformer(NULL,
		pszProjection,
		(GDALDatasetH)poNewDS,
		poNewDS->GetProjectionRef(),
		FALSE, 1000.0, 3);
	pTransformArg = GDALCreateApproxTransformer(GDALGenImgProjTransform, m_hGenTransformArg, 0.125);
	CPLErr err = GDALRasterizeLayers((GDALDatasetH)poNewDS, 1, pnbandlist, 1, player, GDALGenImgProjTransform, m_hGenTransformArg, dburnValues, papszOptions, GDALTermProgress, "shiliang");
	GDALDestroyGenImgProjTransformer(m_hGenTransformArg);
	GDALDestroyApproxTransformer(pTransformArg);
	OGR_DS_Destroy(poDS);

	GDALDriver *poDriver1;
	poDriver1 = GetGDALDriverManager()->GetDriverByName("PNG");
	poDriver1->CreateCopy(outName1, poNewDS, FALSE, NULL, NULL, NULL);
	GDALClose(poNewDS);
	delete[]player;
	delete[]pnbandlist;
	delete[]dburnValues;
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		cout << "请输入： 矢量路径、原始影像路径、概率图路径txt文件、输出png路径(.png)" << endl;
		return 0;
	}
	char* path_Shp = argv[1]; //"C:\\Users\\Libo\\Desktop\\8bit\\460-4\\460-4.shp";
	char* path_Img = argv[2]; //"C:\\Users\\Libo\\Desktop\\8bit\\460.jpg";
	char* path_Txt = argv[3]; //"C:\\Users\\Libo\\Desktop\\8bit\\path.txt";
	char* out_png = argv[4];  //"C:\\Users\\Libo\\Desktop\\8bit\\460_result.png";

	MarkPatch* mp = new MarkPatch();
	mp->InitDataFile(path_Shp);
	mp->Process(path_Shp, path_Txt);
	Run(path_Shp, path_Img, out_png);
	system("pause");
}


