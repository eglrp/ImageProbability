#include<iostream>
#include "stdafx.h"
using namespace std;
#include "gdal.h"
#include "gdal_priv.h"
#include "ogr_srs_api.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "ogrsf_frmts.h"

#include "ogr_spatialref.h"
#include "ogr_geometry.h"
#include "gdalwarper.h"


class SmallpatchSieveFilter
{
public:
	SmallpatchSieveFilter();
	~SmallpatchSieveFilter();
	void SieveFilter(const char* Src_path, const char* DstImgPath, int SizeThresthod, int Connectedness);
private:

};

