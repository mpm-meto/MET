// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_DIAG_CONF_INFO_H__
#define  __TC_DIAG_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <map>
#include <netcdf>

#include "vx_config.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

// Struct for the -data command line options
struct DataOptInfo {

   StringArray tech_ids;   // ATCF Tech ID(s) corresponding to this data source
   StringArray data_files; // Gridded data file(s)

   void clear();

   DataOptInfo & operator+=(const DataOptInfo &);
};

////////////////////////////////////////////////////////////////////////

class DomainInfo {

   private:

      void init_from_scratch();

   public:

      DomainInfo();
      ~DomainInfo();

      // ATCF Tech ID's
      StringArray tech_ids;

      // Domain data files
      StringArray data_files;

      // Domain name
      string domain;

      // TcrmwData struct for creating a TcrmwGrid object
      TcrmwData data;
      double delta_range_km;

      // Vector of VarInfo pointers (not allocated)
      std::vector<VarInfo *> var_info_ptr;

      // Diagnostic scripts to be run
      StringArray diag_script;

      //////////////////////////////////////////////////////////////////

      void clear();

      void parse_domain_info(Dictionary &);
      void set_data_files(const StringArray &);

      int get_n_data() const;
};

////////////////////////////////////////////////////////////////////////

inline int DomainInfo::get_n_data() const { return var_info_ptr.size(); }

////////////////////////////////////////////////////////////////////////

class TCDiagConfInfo {

   private:

      void init_from_scratch();

   public:

      TCDiagConfInfo();
      ~TCDiagConfInfo();

      //////////////////////////////////////////////////////////////////

      // TCDiag configuration object
      MetConfig conf;

      // Track line filtering criteria
      StringArray  model;
      ConcatString storm_id;
      ConcatString basin;
      ConcatString cyclone;
      unixtime     init_inc;
      unixtime     valid_beg, valid_end;
      TimeArray    valid_inc, valid_exc;
      NumArray     valid_hour;
      NumArray     lead_time;

      // Vector of VarInfo objects from data.field (allocated)
      std::vector<VarInfo *> var_info;

      // Pressure level values from the config file
      std::set<double> pressure_levels;

      // Vector of DomainInfo
      std::vector<DomainInfo> domain_info;

      // Vortext removal settings
      bool vortex_removal_flag;

      // Directory for temporary files
      ConcatString tmp_dir;

      // String to customize output file name
      ConcatString output_prefix;

      // Output file options
      bool nc_rng_azi_flag;
      bool nc_diag_flag;
      bool cira_diag_flag;

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config(const char *, const char *);
      void process_config(GrdFileType,
                          std::map<std::string,DataOptInfo>);

      void parse_domain_info(std::map<std::string,DataOptInfo>);

      int get_n_domain() const;
};

////////////////////////////////////////////////////////////////////////

inline int TCDiagConfInfo::get_n_domain() const { return domain_info.size(); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_DIAG_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
