// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __SERIES_DATA_H__
#define  __SERIES_DATA_H__

////////////////////////////////////////////////////////////////////////

#include "vx_util.h"
#include "vx_log.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "vx_grid.h"

////////////////////////////////////////////////////////////////////////

StringArray parse_file_list(const StringArray&, const GrdFileType);

void get_series_data(int, VarInfo*, DataPlane&);

void get_series_entry(int, VarInfo*, const StringArray&,
    const GrdFileType, StringArray&, DataPlane&, Grid&);

bool read_single_entry(VarInfo*, const ConcatString&,
    const GrdFileType, DataPlane&, Grid&);

////////////////////////////////////////////////////////////////////////

#endif  // __SERIES_DATA_H__

////////////////////////////////////////////////////////////////////////