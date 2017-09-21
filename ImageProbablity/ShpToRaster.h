#pragma once
#include <iostream>
#include <cstring>
#include "stdafx.h"

#include "gdal.h"
#include "gdal_priv.h"
#include "gdal_alg.h"
#include "ogrsf_frmts.h"
#include "ogr_core.h"

using namespace std;

class ShpToRaster
{
public:
	ShpToRaster();

	// 1. ��ʼ��·������
	int Initial(char*,char*,char*);  
	
	// 2. ʸ��ͼ��դ��
	bool RasterizeShp();
	int RasterizeShp(OGRDataSource* shpfile, GDALDataset* poDataTif, char* pszOutFileName); 

	// ����6�����������к�
	void Projection2ImageRowCol(double* adfGeoTransform, double dProjX, double dProjY, int &iCol, int &iRow);

	~ShpToRaster();
private:	
	char *shpName;
	char *tifName;
	char* outName;
};

