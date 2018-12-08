#ifndef _DEPTH_MAP_BIN_FILE_IO_H
#define _DEPTH_MAP_BIN_FILE_IO_H

#include "DepthMap.h"

int ReadDepthMapBinFileHeader(FILE * fp, int &retNumFrames, int &retNCols, int &retNRows);

//the caller needs to allocate space for <retDepthMap>
int ReadDepthMapBinFileNextFrame(FILE * fp, int numCols, int numRows, CDepthMap & retDepthMap);

//<fp> must be opened with "wb"
int WriteDepthMapBinFileHeader(FILE * fp, int nFrames, int nCols, int nRows);

//<fp> must be opened with "wb"
int WriteDepthMapBinFileNextFrame(FILE * fp, const CDepthMap & depthMap);

#endif