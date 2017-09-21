#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include "SHPFileAddFeature.h"

/*************************************GDAL头文件**********************************/
#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_srs_api.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "ogrsf_frmts.h"

#include "ogr_spatialref.h"
#include "ogr_geometry.h"
#include "gdalwarper.h"


CSHPFileAddFeature::CSHPFileAddFeature(void)
{
}


CSHPFileAddFeature::~CSHPFileAddFeature(void)
{
}

void CSHPFileAddFeature::shpaddferture00000(const char* shpname, string shpkeyname, string label,double * dd)
{

	//vector<SamplePolygon> outData;
	GDALAllRegister();
	OGRRegisterAll();
	// （2）得到shp文件的处理器
	OGRSFDriver* poDriver =OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
	//（3）打开shp文件
	OGRDataSource* poDS = poDriver->Open(shpname,1 );

	//（4）获取shp图层
	OGRLayer* poLayer = poDS->GetLayer(0);
	
	//（5）读取几何和属性值
	OGRFeature * pFeature = NULL;

	OGRFeatureDefn* featureDefn =  poLayer->GetLayerDefn();
	pFeature = poLayer->GetFeature(0);
	OGRFeatureDefn * fieldDefs = pFeature->GetDefnRef();
	int j =0;
	int w = fieldDefs->GetFieldCount();

	int ZD_num = 0;

	for(int i=0;i<fieldDefs->GetFieldCount();i++)
	{
		OGRFieldDefn* fieldDef = fieldDefs->GetFieldDefn(i);
		const char* name = fieldDef->GetNameRef();
		if (name==label.c_str())
		{
			ZD_num = i;
		}
		//if(name == label.c_str())
		//{
		//	//OGRFieldDefn oFieldLabel(label.c_str(),OFTReal);
		//	poLayer->ResetReading();
		//	//poLayer->CreateField( &oFieldLabel ) ;
		//	while ((pFeature=poLayer->GetNextFeature())!=NULL)
		//	{

		//		pFeature->SetField(label.c_str(),dd[j]);
		//		poLayer->SetFeature(pFeature);

		//		j++;
		//		OGRFeature::DestroyFeature(pFeature);

		//	}

		//}
		//else
		//{
		//	OGRFieldDefn oFieldLabel(label.c_str(),OFTInteger);
		//	poLayer->CreateField( &oFieldLabel ) ;

		//	while ((pFeature=poLayer->GetNextFeature())!=NULL)
		//	{

		//		pFeature->SetField(label.c_str(),dd[j]);
		//		poLayer->SetFeature(pFeature);

		//		j++;

		//		//OGRFeature::DestroyFeature(pFeature);
		//		//poLayer->CreateFeature( pFeature );	
		//	}
		//}
	}

	if (ZD_num!=0)
	{
		poLayer->ResetReading();
		//poLayer->CreateField( &oFieldLabel ) ;
		while ((pFeature=poLayer->GetNextFeature())!=NULL)
		{

			pFeature->SetField(label.c_str(),dd[j]);
			poLayer->SetFeature(pFeature);

			j++;
			OGRFeature::DestroyFeature(pFeature);

		}
	}
	else
	{
		OGRFieldDefn oFieldLabel(label.c_str(),OFTInteger);
		poLayer->CreateField( &oFieldLabel ) ;

		while ((pFeature=poLayer->GetNextFeature())!=NULL)
		{

			pFeature->SetField(label.c_str(),dd[j]);
			poLayer->SetFeature(pFeature);

			j++;

			OGRFeature::DestroyFeature(pFeature);
			//poLayer->CreateFeature( pFeature );	
		}
	}

	poLayer->SyncToDisk();
	string executsqlStr = "repack "+ shpkeyname;
	char *executeSQLStr = const_cast<char*>(executsqlStr.c_str());
	poDS->ExecuteSQL(executeSQLStr,NULL,NULL);
	//OGRFeature::DestroyFeature(pFeature);
	OGRDataSource::DestroyDataSource( poDS );

//	MessageBox(_T("OK"),_T("Wang"));
}

void CSHPFileAddFeature::WriteFeature(OGRLayer *poLayer,string FieldName,int* FeatureM)
{
	OGRFeature* poFeature;
	poFeature = poLayer->GetFeature(0);	
	OGRFeatureDefn * fieldDefs = poFeature->GetDefnRef();
	bool FindField = false;
	for(int i=0;i<fieldDefs->GetFieldCount();i++)
	{
		OGRFieldDefn* fieldDef = fieldDefs->GetFieldDefn(i);
		const char* name = fieldDef->GetNameRef();		
		if(_stricmp(name,FieldName.c_str())==0)
		{			
			FindField = true;
			break;
		}		
	}
	if(!FindField)
	{
		OGRFieldDefn oFieldLabel(FieldName.c_str(),OFTInteger);
		poLayer->CreateField( &oFieldLabel ) ;
	}
	poLayer->ResetReading();
	int num = 0;
	while ((poFeature=poLayer->GetNextFeature())!=NULL)
	{
		poFeature->SetField(FieldName.c_str(),FeatureM[num]);
		poLayer->SetFeature(poFeature);
		OGRFeature::DestroyFeature(poFeature);
		num++;
	}
}
void CSHPFileAddFeature::WriteFeature(OGRLayer *poLayer,string FieldName,vector<FeatureDense> result)
{
	OGRFeature* poFeature;
	poFeature = poLayer->GetFeature(0);	
	OGRFeatureDefn * fieldDefs = poFeature->GetDefnRef();
	bool FindField = false;
	for(int i=0;i<fieldDefs->GetFieldCount();i++)
	{
		OGRFieldDefn* fieldDef = fieldDefs->GetFieldDefn(i);
		const char* name = fieldDef->GetNameRef();
		if(_stricmp(name,FieldName.c_str())==0)
		{			
			FindField = true;
			break;
		}		
	}
	if(!FindField)
	{
		OGRFieldDefn oFieldLabel(FieldName.c_str(),OFTReal);  ////gai  oftstring
		poLayer->CreateField(&oFieldLabel);
	}
	poLayer->ResetReading();
	int num = 0;
	while ((poFeature=poLayer->GetNextFeature())!=NULL)
	{
		int index = poFeature->GetFID();
		poFeature->SetField(FieldName.c_str(),result.at(index).ID);
		poLayer->SetFeature(poFeature);
		OGRFeature::DestroyFeature(poFeature);
		num++;
	}
}

void CSHPFileAddFeature::WriteFeature11(OGRLayer *poLayer,string FieldName,string* FeatureM)
{
	OGRFeature* poFeature;
	poFeature = poLayer->GetFeature(0);	
	OGRFeatureDefn * fieldDefs = poFeature->GetDefnRef();
	bool FindField = false;
	for(int i=0;i<fieldDefs->GetFieldCount();i++)
	{
		OGRFieldDefn* fieldDef = fieldDefs->GetFieldDefn(i);
		const char* name = fieldDef->GetNameRef();
		if(_stricmp(name,FieldName.c_str())==0)
		{			
			FindField = true;
			break;
		}		
	}
	if(!FindField)
	{
		OGRFieldDefn oFieldLabel(FieldName.c_str(),OFTString);////gai  oftstring
		poLayer->CreateField( &oFieldLabel ) ;
	}
	poLayer->ResetReading();
	int num = 0;
	while ((poFeature=poLayer->GetNextFeature())!=NULL)
	{
		poFeature->SetField(FieldName.c_str(),FeatureM[num].c_str());
		poLayer->SetFeature(poFeature);
		OGRFeature::DestroyFeature(poFeature);
		num++;
	}


}
void CSHPFileAddFeature::ChangeDetecViaSTD(OGRLayer *poLayer,string FieldName,int recordNum,int scale,double * FeatureM)
{
	cv::Mat srcMat = Mat(recordNum,1,CV_32F,Scalar::all(0));
	for (int i=0;i<recordNum;i++)
	{
		srcMat.at<float>(i,0) = FeatureM[i];
	}
	/*const CvArr* temp = (CvArr*)&srcMat;*/
	CvMat temp = srcMat;
	CvScalar Mean,Std;
	cvAvgSdv(&temp,&Mean,&Std);
	int* changeM = new int[recordNum];
	memset(changeM,0,sizeof(int)*recordNum);
	//取scale倍的标准差
	for (int i=0;i<recordNum;i++)
	{
		if (abs(FeatureM[i]-Mean.val[0])>=scale*Std.val[0])
			changeM[i] = 1;
	}
	OGRFeature* poFeature;
	poFeature = poLayer->GetFeature(0);	
	OGRFeatureDefn * fieldDefs = poFeature->GetDefnRef();
	bool FindField = false;
	for(int i=0;i<fieldDefs->GetFieldCount();i++)
	{
		OGRFieldDefn* fieldDef = fieldDefs->GetFieldDefn(i);
		const char* name = fieldDef->GetNameRef();
		if(_stricmp(name,FieldName.c_str())==0)
		{			
			FindField = true;
			break;
		}		
	}
	if(!FindField)
	{
		OGRFieldDefn oFieldLabel(FieldName.c_str(),OFTInteger);
		poLayer->CreateField( &oFieldLabel ) ;
	}
	poLayer->ResetReading();
	int num = 0;
	while ((poFeature=poLayer->GetNextFeature())!=NULL)
	{
		poFeature->SetField(FieldName.c_str(),changeM[num]);
		poLayer->SetFeature(poFeature);
		OGRFeature::DestroyFeature(poFeature);
		num++;
	}
	delete[] changeM;
}