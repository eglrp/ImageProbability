#include "stdafx.h"
#include "SieveFilter.h"



SmallpatchSieveFilter::SmallpatchSieveFilter()
{
}

SmallpatchSieveFilter::~SmallpatchSieveFilter()
{
}

void SmallpatchSieveFilter::SieveFilter(const char* Src_path, const char* Dst_Path, int SizeThresthod, int Connectedness)
{
	GDALAllRegister();
	cout <<"第四步,执行小斑去除....";
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTIFF");
	if (poDriver == NULL)
	{
		cout << "不能创建指定类型的文件：" << endl;
	}
	GDALDataset* poSrc = (GDALDataset*)GDALOpen(Src_path,GA_ReadOnly);
	int NewBandXsize = poSrc->GetRasterXSize();
	int NewBandYsize = poSrc->GetRasterYSize();

	GDALDataType Type = poSrc->GetRasterBand(1)->GetRasterDataType();
	GDALDataset* poDstDS = poDriver->Create(Dst_Path, NewBandXsize, NewBandYsize, 1, Type, NULL);
	double GeoTrans[6] = { 0 };
	poSrc->GetGeoTransform(GeoTrans);
	poDstDS->SetGeoTransform(GeoTrans);
	poDstDS->SetProjection(poSrc->GetProjectionRef());
	GDALRasterBandH HImgBand = (GDALRasterBandH)poSrc->GetRasterBand(1);
	GDALRasterBandH HPDstDSBand = (GDALRasterBandH)poDstDS->GetRasterBand(1);
	GDALSetRasterColorTable(HPDstDSBand, GDALGetRasterColorTable(HImgBand));
	
	GDALSieveFilter(HImgBand, NULL, HPDstDSBand, SizeThresthod, Connectedness, NULL, NULL, NULL);
	cout<<"-->小班去除完成！"<<endl;
	GDALClose((GDALDatasetH)poDstDS);
};