#!/bin/sh

echo
echo "*** Running PLOT_DATA_PLANE on a sample GRIB1 gridded file  ***"
plot_data_plane \
  ../data/sample_fcst/2007033000/nam.t00z.awip1236.tm00.20070330.grb \
  ${TEST_OUT_DIR}/plot_data_plane/nam.t00z.awip1236.tm00.20070330_TMPZ2.ps \
  'name="TMP"; level="Z2";' \
   -title "GRIB1 NAM Z2 TMP" \
   -v 1

echo
echo "*** Running PLOT_DATA_PLANE on a sample netCDF file  ***"
plot_data_plane \
  ${TEST_OUT_DIR}/pcp_combine/sample_fcst_12L_2005080712V_12A.nc \
  ${TEST_OUT_DIR}/plot_data_plane/sample_fcst_12L_2005080712V_12A_APCP12_NC_MET.ps \
  'name = "APCP_12"; level = "(*,*)";' \
  -title "NC MET 12-hour APCP" \
  -v 1
