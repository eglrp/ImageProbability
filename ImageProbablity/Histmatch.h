#pragma once
#include <iostream>
#include <sstream>
#include <afxtempl.h>
#include "string.h"

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

class HistmatchedImge
{
public:
	HistmatchedImge();
	void Onhistmatch(const char* pathImage1, const char* pathImage2,const char* pathcvHM,const char* pathHistmatched);
};