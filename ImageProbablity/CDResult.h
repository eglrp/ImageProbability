#include "stdafx.h"
#include <iostream>
#include <algorithm>
#include <string>
#include "SHPFileAddFeature.h"
using namespace std;
#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"     
#include "opencv2/calib3d/calib3d.hpp"   
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/legacy/legacy.hpp"
#include "opencv2/legacy/compat.hpp"
using namespace cv;
#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_srs_api.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "ogrsf_frmts.h"

#include "ogr_geos.h"
#include "ogr_spatialref.h"
#include "ogr_geometry.h"
#include "gdalwarper.h"

class MarkPatch
{
public:
	MarkPatch();
	bool InitDataFile(const char* path_Shp);
	void Process(const char* path_Shp,char* path_txt);
	static bool cmp(FeatureDense A, FeatureDense B);
	void createMarkPatch(const char* path_Shp, const char* path_Image,int CCcode);
	void AutothresholdCC(const char* src,const char* path_shp,const char *dst);
	bool ProjToImageRowCol(double *adfGeoTransform, double dProjX, double dProjY, int &iCol, int &iRow);
	bool ImageRowColToProj(double *adfGeoTransform, int iCol, int iRow, double &dProjX, double &dProjY);
	void CreateMaskMat(OGRGeometry* geom,CvRect ROIRect,double* ImageTransform,const char* Proj);
	bool BurnGeometryToMask(GDALDatasetH labelDataset, OGRGeometry* poGeometry);
	bool IsPointInRegion(int x, int y);
	void WriteShp(string Shppath,vector<FeatureDense> result);
public:
	CvMat* CachedLabelMat;
	vector<vector<FeatureDense>> VFeatDen;
	int FeatureCount;
};