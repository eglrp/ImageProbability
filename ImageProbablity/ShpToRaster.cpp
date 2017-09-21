#include "stdafx.h"
#include "ShpToRaster.h"


ShpToRaster::ShpToRaster()
{
}


ShpToRaster::~ShpToRaster()
{

}


int ShpToRaster::Initial(char* shp_path, char* tif_path, char* out_path)
{
	shpName = shp_path;
	tifName = tif_path;
	outName = out_path;

	cout << "参数导入成功！" << endl;
	return true;
}

bool ShpToRaster::RasterizeShp()
{
	GDALAllRegister();
	OGRRegisterAll();
	OGRDataSource  *poDS;
	GDALDataset *poDataTif;
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
	poDataTif = (GDALDataset*)GDALOpen(tifName, GA_ReadOnly);
	poDS = OGRSFDriverRegistrar::Open(shpName, true);

	if (poDS == NULL || poDataTif == NULL)
	{
		cout << "Open " << shpName << " failed." << endl;
		return 0;
	}
	
	if (RasterizeShp(poDS, poDataTif, outName) == true)
	{
		cout << "矢量层栅格化成功！" << endl;
	}
	return true;
}

int ShpToRaster::RasterizeShp(OGRDataSource* shpfile, GDALDataset* poDataTif, char* pszOutFileName)
{
	OGRLayer* poLayer;
	poLayer = shpfile->GetLayer(0);
	poLayer->ResetReading();
	OGRFeature* oFeature;
	OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();

	int iField;
	bool existing = 0;
	int fieldcount = poFDefn->GetFieldCount();
	for (int i = 0; i < fieldcount; i++)
	{
		OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(i);
		string s = "FID";
		string s1 = "VALUE";
		if (poFieldDefn->GetNameRef() == s)
		{
			iField = i;
		}
		else if (poFieldDefn->GetNameRef()==s1)
		{
			existing = true;
		}
	}
	
	if (!existing)
	{
		OGRFieldDefn tmpField("VALUE", OFTString);
		tmpField.SetWidth(8);
		if (poLayer->CreateField(&tmpField) != OGRERR_NONE)
		{
			cout << "Creating Name field failed.";
			return FALSE;
		}

		while ((oFeature = poLayer->GetNextFeature()) != NULL)
		{
			// 添加字段并设置属性值
			oFeature->SetField(tmpField.GetNameRef(),255);
			poLayer->SetFeature(oFeature);
		}
	}	
				
	char * pszProjection;
	OGRSpatialReference *poSRS = poLayer->GetSpatialRef();  //  获取空间信息
	OGREnvelope  shpExtent;
	poLayer->GetExtent(&shpExtent);
	
	double a[6];
	poDataTif->GetGeoTransform(a);
	a[0] = shpExtent.MinX;
	a[3] = shpExtent.MaxY;
	int  m_nImageWidth, m_nImageHeight;
	Projection2ImageRowCol(a, shpExtent.MaxX, shpExtent.MinY, m_nImageWidth, m_nImageHeight);

	if (!poSRS)
	{
		cout << "Can't open the " << shpName << "geospatial reference" << endl;
		return false;
	}
	else
	{
		poSRS->exportToWkt(&pszProjection);
	}

	const char *pszFormat = "GTiff";
	GDALDriver *poDriver;
	poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);	
	GDALDataset *poNewDS = poDriver->Create(pszOutFileName, m_nImageWidth, m_nImageHeight, 1, GDT_Byte, NULL);	
	
	if (!poSRS)
	{
		double adfGeoTransform[6];
		adfGeoTransform[0] = shpExtent.MinX;
		adfGeoTransform[1] = a[1];
		adfGeoTransform[2] = a[2];
		adfGeoTransform[3] = shpExtent.MaxY;
		adfGeoTransform[4] = a[4];
		adfGeoTransform[5] = a[5];
		GDALSetGeoTransform(poNewDS, adfGeoTransform);
		OGRSpatialReference * poOGRSR = NULL;
		poOGRSR->SetUTM(50);
		poSRS->exportToWkt(&pszProjection);
		poNewDS->SetProjection(pszProjection);
	}
	else
	{
		double adfGeoTransform[6];
		adfGeoTransform[0] = shpExtent.MinX;
		adfGeoTransform[1] = a[1];
		adfGeoTransform[2] = a[2];
		adfGeoTransform[3] = shpExtent.MaxY;
		adfGeoTransform[4] = a[4];
		adfGeoTransform[5] = a[5];
		GDALSetGeoTransform(poNewDS, adfGeoTransform);
		poNewDS->SetProjection(pszProjection);
	}

	int * pnbandlist;
	pnbandlist = new int[1];
	pnbandlist[0] = 1;
	double *dburnValues;
	dburnValues = new double[1];
	dburnValues[0] = 255;
	OGRGeometryH * pgeometryArr;
	pgeometryArr = new OGRGeometryH[1];
	pgeometryArr[0] = (OGRGeometryH)poLayer;
	char **papszOptions = NULL;
	papszOptions = CSLSetNameValue(papszOptions, "ALL_TOUCHED", "TRUE");
	papszOptions = CSLSetNameValue(papszOptions, "ATTRIBUTE", "VALUE");

	CPLErr err = GDALRasterizeLayers((GDALDatasetH)poNewDS, 1, pnbandlist, 1,
		pgeometryArr, NULL, NULL, NULL, papszOptions, NULL, "矢量栅格化");
	if (!true)
	{
		poLayer->ResetReading();
		OGRFeatureDefn *poFDefn_new = poLayer->GetLayerDefn();
		int newFieldcount = poFDefn_new->GetFieldCount();
		int newiField;
		for (int i = 0; i < newFieldcount; i++)
		{
			OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(i);
			string s = "VALUE";
			if (poFieldDefn->GetNameRef() == s)
			{
				newiField = i;
				break;
			}
		}
		poLayer->DeleteField(newiField);
	}
	
	GDALClose(poNewDS);
	OGR_DS_Destroy(shpfile);

	delete[]pgeometryArr;
	delete[]pnbandlist;
	delete[]dburnValues;

	return true;
}

void ShpToRaster::Projection2ImageRowCol(double *adfGeoTransform, double dProjX, double dProjY, int &iCol, int &iRow)
{
	double dTemp = adfGeoTransform[1] * adfGeoTransform[5] - adfGeoTransform[2] * adfGeoTransform[4];
	double dCol = 0.0, dRow = 0.0;

	dCol = (adfGeoTransform[5] * (dProjX - adfGeoTransform[0]) -
		adfGeoTransform[2] * (dProjY - adfGeoTransform[3])) / dTemp + 0.5; // 不要四舍五入
	dRow = (adfGeoTransform[1] * (dProjY - adfGeoTransform[3]) -
		adfGeoTransform[4] * (dProjX - adfGeoTransform[0])) / dTemp + 0.5;
	iCol = static_cast<int>(dCol);
	iRow = static_cast<int>(dRow);
}