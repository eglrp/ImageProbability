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
/*************************************GDALÍ·ÎÄ¼þ**********************************/
#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_srs_api.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "ogrsf_frmts.h"

#include "ogr_spatialref.h"
#include "ogr_geometry.h"
#include "gdalwarper.h"

#include <iostream>
#include <vector>
#include <string>
#include <afxtempl.h>

using namespace cv;
using namespace std;

struct FeatureDense
{
	int ID;
	int SUM;
};

class CSHPFileAddFeature
{
public:
	CSHPFileAddFeature(void);
	~CSHPFileAddFeature(void);

	void shpaddferture00000(const char* shpname, string shpkeyname, string label,double * dd);
	void WriteFeature(OGRLayer *poLayer,string FieldName,int* FeatureM);
	void WriteFeature(OGRLayer *poLayer,string FieldName,vector<FeatureDense> result);
	void ChangeDetecViaSTD(OGRLayer *polayer,string FieldName,int recordNum,int scale,double * FeatureM);
	void WriteFeature11(OGRLayer *poLayer,string FieldName,string* FeatureM);
};

