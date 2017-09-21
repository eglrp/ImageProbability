#include <algorithm>
#include "stdafx.h"
#include "CDResult.h"
#include <iostream>
#include <fstream>

#include "omp.h"
#include <math.h> 
#include <Windows.h>

MarkPatch::MarkPatch()
{

}

bool MarkPatch::InitDataFile(const char* path_Shp)
{
	// 注册
	GDALAllRegister(); 
	OGRRegisterAll();
	OGRDataSource *poDS;
	poDS = OGRSFDriverRegistrar::Open(path_Shp, true );
	if( poDS == NULL )
	{
		cout<<"Open failed.\n%s";
		OGRDataSource::DestroyDataSource(poDS);
		return false;
	}
	OGRLayer  *poLayer;
	poLayer = poDS->GetLayer(0);
	FeatureCount = poLayer->GetFeatureCount();
	
	return TRUE;
}

bool MarkPatch::cmp(FeatureDense A, FeatureDense B)
{
	if (A.SUM > B.SUM)  // 按概率由大到小排序
	{
		return true;
	}
	else if (A.SUM == B.SUM)
	{
		if (A.SUM > B.SUM) 
		{
			return true;
		}
		return false;
	}
	else
	{
		return false;
	}
}

void MarkPatch::Process(const char* path_Shp,char* path_txt)
{
	ifstream infile(path_txt,ios::in);
	if (!infile)
	{
		cout<<"读取路径文件错误!"<<endl;
		return ;	
	}
	vector<string> path;
	for (int i=0;i<10;i++)
	{
		string str;
		getline(infile,str);
		path.push_back(str);
	}

	for (int i=0;i<10;i++)
	{
		cout<<"正在处理第"<<i+1<<"个.."<<endl;
		stringstream ss;
		ss <<path.at(i);
		string image = ss.str();
		char* path_Image = const_cast<char*>(image.c_str());
		createMarkPatch(path_Shp,path_Image,i);
	}
	int* FetCC = new int[FeatureCount];
	vector<FeatureDense> result;
	
	for (int i =0;i<FeatureCount;i++)
	{
		vector<FeatureDense> tmp; // 获取每图斑在概率图上的sum
		for (int m=0;m<10;m++)
		{
			tmp.push_back(VFeatDen.at(m).at(i));
		}
		
		std::sort(tmp.begin(),tmp.end(),cmp);
		result.push_back(tmp.at(0));

		cout<<"第"<<i<<"个图斑的最大类别码为"<<result.at(i).ID<<endl;
	}
	WriteShp(path_Shp,result);

}

void MarkPatch::createMarkPatch(const char* path_Shp, const char* path_Image,int CCcode)
{	
	cout<<"开始图斑概率统计..."<<endl;
	GDALAllRegister(); 
	OGRRegisterAll();
	GDALDataset *poDataset = (GDALDataset*)GDALOpen(path_Image,GA_ReadOnly);  
	if( poDataset == NULL )
	{
		cout<<"Open Error!"<<endl;
		return;
	}   
	double adfGeotranceform[6]; 
	const char* proj;
	poDataset->GetGeoTransform(adfGeotranceform);
	proj = poDataset->GetProjectionRef();	

	// shp
	OGRDataSource *poDS;
	poDS = OGRSFDriverRegistrar::Open(path_Shp, true );
	if( poDS == NULL )
	{
		cout<<"Open failed.\n%s";
		OGRDataSource::DestroyDataSource(poDS);
		return;
	}
	OGRLayer  *poLayer;
	poLayer = poDS->GetLayer(0);		
    int fetureCount = poLayer->GetFeatureCount();
	if (fetureCount==0)
	{
		cout<<"Feature is NULL !.\n%s";
		return;
	}	
	
	OGRFeature *poFeature;
	poLayer->ResetReading();
	OGRGeometry* pGeometry;
	OGREnvelope* eRect = new OGREnvelope();
	CvRect RoiRect;	
	int DPIndex=0;
	vector<FeatureDense> VFD;
	while((poFeature = poLayer->GetNextFeature()) != NULL ) 
	{
		double adfGeotransform[6];
		memcpy(adfGeotransform,adfGeotranceform,6*sizeof(double));
		pGeometry = poFeature->GetGeometryRef();
		if (pGeometry == NULL)
		{
			cout<<"Geometry get failed."<<endl;
			return;
		}
	
		pGeometry->getEnvelope(eRect);   // 获取外接矩形范围
		int colStart,rowStart;
		ProjToImageRowCol(adfGeotransform, eRect->MinX, eRect->MaxY, colStart, rowStart);
		ImageRowColToProj(adfGeotransform, colStart, rowStart, adfGeotransform[0], adfGeotransform[3]);
		int width, height;
		ProjToImageRowCol(adfGeotransform, eRect->MaxX, eRect->MinY, width, height);
		if (width<1 || height<1)
		{
			continue;
		}
		RoiRect.x = colStart;
		RoiRect.y = rowStart;
		RoiRect.width = width;
		RoiRect.height = height;
		CreateMaskMat(pGeometry,RoiRect,adfGeotranceform,proj);
		// 读取影像RoiRect区域像素值，存于imgBuf
		BYTE* imgBuf = new BYTE[width*height];   
		poDataset->RasterIO(GF_Read,colStart,rowStart,width,height,imgBuf,width,height,GDT_Byte,1,0,0,0,0);
		
		int ScaleNumber=0;		
		int number =0;	
		int sum = 0;
		// 统计图斑内部所有像素值的∑（sum）
		for (int i=0;i<height;i++)
		{
			for (int j=0;j<width;j++)
			{
				int tmp = i*width+j;      // 在imgbuf中的位置
				int data = imgBuf[tmp];
				if( IsPointInRegion(j,i))  
				{
					sum = sum+data;
				}
			}
		}

		DPIndex = poFeature->GetFID();
		FeatureDense tmp;
		tmp.ID =CCcode+1;
		tmp.SUM = sum;
		cout<<"第"<<tmp.ID<<"概率图，"<<"FID="<<poFeature->GetFID()<<",概率∑=："<<sum<<endl;
		VFD.push_back(tmp);
		delete[] imgBuf;
		OGRFeature::DestroyFeature(poFeature);
	}
	VFeatDen.push_back(VFD);
	GDALClose(poDataset);
	OGRDataSource::DestroyDataSource(poDS);
}

void MarkPatch::AutothresholdCC(const char* src,const char* path_shp,const char *dst)
{
	GDALAllRegister();
	GDALDataset* poDataset=(GDALDataset*)GDALOpen(src,GA_ReadOnly);
	if( poDataset == NULL )
	{  
		cout<<"Open Error!"<<endl;
		return;  
	}   
	// 六参数
	double adfGeotranceform[6]; 
	const char* proj;
	poDataset->GetGeoTransform(adfGeotranceform);
	proj = poDataset->GetProjectionRef();	

	int width=poDataset->GetRasterXSize();
	int height=poDataset->GetRasterYSize();
	int Count=poDataset->GetRasterCount();
	GDALRasterBand *pBand=poDataset->GetRasterBand(1);  
	int *imgdata = new int[width*height];	
	pBand->RasterIO(GF_Write,0,0,width,height,imgdata,width,height,GDT_Byte,1,0);	
	
	
	// shp
	OGRDataSource *poDS;
	poDS = OGRSFDriverRegistrar::Open(path_shp, true );
	if( poDS == NULL )
	{
		cout<<"Open failed.\n%s";
		OGRDataSource::DestroyDataSource(poDS);
		return;
	}
	OGRLayer  *poLayer;
	poLayer = poDS->GetLayer(0);		
    int fetureCount = poLayer->GetFeatureCount();
	if (fetureCount==0)
	{
		cout<<"Feature is NULL !.\n%s";
		return;
	}
	
	OGRFeature *poFeature;
	poLayer->ResetReading();
	OGRGeometry* pGeometry;
	OGREnvelope* eRect = new OGREnvelope();
	//int iBeginCol = 0, iBeginRow = 0;
	CvRect RoiRect;
	double* DPvalue = new double[fetureCount];
	memset(DPvalue,0,fetureCount*sizeof(double));
	int DPIndex=0;
	
	while((poFeature = poLayer->GetNextFeature()) != NULL && poFeature->GetFieldAsInteger("CC")<= 051)  
	{
		double adfGeotransform[6];
		memcpy(adfGeotransform,adfGeotranceform,6*sizeof(double));
		pGeometry = poFeature->GetGeometryRef();
		if (pGeometry == NULL)
		{
			cout<<"Geometry get failed."<<endl;
			return;
		}
	
		pGeometry->getEnvelope(eRect);   //  获取外接矩形范围	  
		int colStart,rowStart;
		ProjToImageRowCol(adfGeotransform, eRect->MinX, eRect->MaxY, colStart, rowStart);   // 左上角行列号、矩形宽度与高度
		ImageRowColToProj(adfGeotransform, colStart, rowStart, adfGeotransform[0], adfGeotransform[3]);
		int width, height;
		ProjToImageRowCol(adfGeotransform, eRect->MaxX, eRect->MinY, width, height);
		if (width<1 || height<1)
		{
			continue;
		}				
		RoiRect.x = colStart;
		RoiRect.y = rowStart;
		RoiRect.width = width;
		RoiRect.height = height;
		CreateMaskMat(pGeometry,RoiRect,adfGeotranceform,proj);

		// 读取影像RoiRect区域像素值，存于imgBuf
		BYTE* imgBuf = new BYTE[width*height];   
		poDataset->RasterIO(GF_Read,colStart,rowStart,width,height,imgBuf,width,height,GDT_Byte,1,0,0,0,0);
				
		int ScaleNumber=0;		
		int number =0;		
		for (int i=0;i<height;i++)
		{
			for (int j=0;j<width;j++)
			{
				int tmp = i*width+j;      // 在imgbuf中的位置
				int data = imgBuf[tmp];
				if( IsPointInRegion(j,i) == false)  
				{
					imgBuf[tmp] = 0;
				}
			}
		}
		pBand->RasterIO(GF_Write,colStart,rowStart,width,height,imgBuf,width,height,GDT_Byte,1,0);	
		
		delete[] imgBuf;
		OGRFeature::DestroyFeature(poFeature);
	}

	GDALClose(poDataset);
	OGRDataSource::DestroyDataSource(poDS);

	double dminmax[2];
	pBand->ComputeRasterMinMax(FALSE,dminmax);
	double dmin=dminmax[0];
	double dmax=dminmax[1];

	// histogram
	int histogram1[256]={0};
	double histogram[256]={0};
	pBand->GetHistogram(dmin,dmax,256,histogram1,FALSE,FALSE,NULL,NULL);
	// normalize histogram
	int size=height*width;
	for(int i=0;i<256;i++) 
	{
		histogram[i]=(double)histogram1[i]/(double)size;
	}

	// average pixel value
	double avgValue=0;
	for(int i=0;i<256;i++) {
		avgValue+=i*histogram[i];
	}

	double threshold;	
	double maxVariance=0;
	double w=0,u=0;
	for(int i=0;i<256;i++) {
		w+=histogram[i];
		u+=i*histogram[i];

		double t=avgValue*w-u;
		double variance=t*t/(w*(1-w));
		if(variance>maxVariance) {
			maxVariance=variance;
			threshold=i;
		}
	}
	
	//  创建文件并设置空间参考和坐标信息
	GDALDriver *poDriver=(GDALDriver*)GDALGetDriverByName("GTIFF");
	GDALDataset *pDstDS=poDriver->Create(dst,width,height,1,GDT_Byte ,NULL);

	int *pBandMap=new int[Count];
	for (int i=0;i<Count;i++)
	{
		pBandMap[i]=i+1;
	}

	BYTE *pData1BufferR=new BYTE[width];
	BYTE *pDataBuffer=new BYTE[width];
	for (int i=0;i<height;i++)
	{
		// i=426;
		pBand->RasterIO(GF_Read,0,i,width,1,pData1BufferR,width,1,GDT_Byte,0,0);
		for (int j=0;j<width;j++)
		{
			if (pData1BufferR[j]>threshold)
			{
				pDataBuffer[j]=255;
			}else
			{
				pDataBuffer[j]=0;
			}
		}
		pDstDS->RasterIO(GF_Write,0,i,width,1,pDataBuffer,width,1,GDT_Byte,1,0,0,0,0);
	}	
	
	cout<<"-->阈值分割完成！"<<endl;	
	GDALClose((GDALDatasetH) pDstDS);
	delete pData1BufferR;

	////////////////////////////////////////////////////////////////////////////////////////
	
}

bool MarkPatch::ProjToImageRowCol(double *adfGeoTransform, double dProjX, double dProjY, int &iCol, int &iRow)
{
	try
	{
		double dTemp = adfGeoTransform[1] * adfGeoTransform[5] - adfGeoTransform[2] * adfGeoTransform[4];
		double dCol = 0.0, dRow = 0.0;
		dCol = (adfGeoTransform[5] * (dProjX - adfGeoTransform[0]) -
			adfGeoTransform[2] * (dProjY - adfGeoTransform[3])) / dTemp + 0.5;
		dRow = (adfGeoTransform[1] * (dProjY - adfGeoTransform[3]) -
			adfGeoTransform[4] * (dProjX - adfGeoTransform[0])) / dTemp + 0.5;

		iCol = static_cast<int>(dCol);
		iRow = static_cast<int>(dRow);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool MarkPatch::ImageRowColToProj(double *adfGeoTransform, int iCol, int iRow, double &dProjX, double &dProjY)
{
	//adfGeoTransform[6]  数组adfGeoTransform保存的是仿射变换中的一些参数，分别含义见下  
	//adfGeoTransform[0]  左上角x坐标   
	//adfGeoTransform[1]  东西方向分辨率  
	//adfGeoTransform[2]  旋转角度, 0表示图像 "北方朝上"  
	//adfGeoTransform[3]  左上角y坐标   
	//adfGeoTransform[4]  旋转角度, 0表示图像 "北方朝上"  
	//adfGeoTransform[5]  南北方向分辨率  

	try
	{
		dProjX = adfGeoTransform[0] + adfGeoTransform[1] * iCol + adfGeoTransform[2] * iRow;
		dProjY = adfGeoTransform[3] + adfGeoTransform[4] * iCol + adfGeoTransform[5] * iRow;
		return true;
	}
	catch (...)
	{
		return false;
	}
}

void MarkPatch::CreateMaskMat(OGRGeometry* geom, CvRect ROIRect, double* ImageTransform, const char* Proj)
{
	CachedLabelMat = cvCreateMat(ROIRect.height, ROIRect.width, CV_8UC1);
	memset(CachedLabelMat->data.ptr, 0, ROIRect.height * ROIRect.width);
	char aMemString[256] = { 0 };
	sprintf(aMemString, "MEM:::DATAPOINTER=%d,PIXELS=%d,LINES=%d",
		CachedLabelMat->data.ptr,
		ROIRect.width, ROIRect.height);

	GDALDataset* labelDataset = (GDALDataset*)GDALOpen(aMemString, GA_Update);

	// GDALDriver* VDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	// GDALDataset* poNewDS = VDriver->Create("ABC.tif", newWidth, newHeight, 1, GDT_Byte, NULL);

	double labelTransform[6] = { 0.0,0.0,0.0,0.0,0.0,0.0 };
	memcpy(labelTransform, ImageTransform, sizeof(double) * 6);
	ImageRowColToProj(ImageTransform, 
		ROIRect.x, ROIRect.y, 
		labelTransform[0], labelTransform[3]);    

	GDALSetGeoTransform(labelDataset, labelTransform);  // 设置六参数
	labelDataset->SetProjection(Proj);  

	BurnGeometryToMask(labelDataset, geom);
	GDALClose(labelDataset);
}

bool MarkPatch::BurnGeometryToMask(GDALDatasetH labelDataset, OGRGeometry* poGeometry)
{
	int pnbandlist[] = { 1 };

	double dburnValues[] = { 255 };
	OGRGeometryH geometry[] = { poGeometry };
	char **papszOptions = NULL;
	papszOptions = CSLSetNameValue(papszOptions, "ALL_TOUCHED", "TRUE");
	int status = GDALRasterizeGeometries(
		labelDataset,  // 栅格化影像
		1, pnbandlist, // 波段列表
		1, geometry, // 要素列表
		NULL, NULL, // 转换函数与参数
		dburnValues, // 栅格化像素值
		papszOptions, // 选项
		NULL, NULL); // 进度
	GDALFlushCache(labelDataset);
	return status == 0;
}

bool MarkPatch::IsPointInRegion(int x, int y)
{
	uchar val = CV_MAT_ELEM(*CachedLabelMat, BYTE, y, x);
	return val != 0;
}

void MarkPatch::WriteShp(string Shppath,vector<FeatureDense> result)
{
	cout<<"第六步，写入可疑程度...";
	OGRRegisterAll();
	OGRDataSource *poDS; 
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
	poDS = OGRSFDriverRegistrar::Open(Shppath.c_str(), TRUE);//第二个参数表示只读还是可读写
	if (poDS == NULL)  
	{  
		printf("Open failed \n");  
	} 
	
	OGRLayer *poLayer;  
	poLayer = poDS->GetLayer(0); 
	if (poLayer == NULL)
	{
		printf("The layer is null !");
		return;
	}
	int layercount = poDS->GetLayerCount(); 
	CSHPFileAddFeature addfeature;
	addfeature.WriteFeature(poLayer ,"CCode",result);
	poLayer->SyncToDisk();
	/*string executsqlStr = "repack "+layname;
	char *executeSQLStr = const_cast<char*>(executsqlStr.c_str());
	poDS->ExecuteSQL(executeSQLStr,NULL,NULL);*/
	cout<<"-->写入完毕,算法处理结束！"<<endl<<endl;
	OGRDataSource::DestroyDataSource(poDS);
}

