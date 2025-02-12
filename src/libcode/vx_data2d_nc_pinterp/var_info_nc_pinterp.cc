// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_nc_pinterp.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <map>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "var_info.h"
#include "var_info_nc_pinterp.h"

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoNcPinterp
//
///////////////////////////////////////////////////////////////////////////////

VarInfoNcPinterp::VarInfoNcPinterp() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcPinterp::~VarInfoNcPinterp() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcPinterp::VarInfoNcPinterp(const VarInfoNcPinterp &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcPinterp & VarInfoNcPinterp::operator=(const VarInfoNcPinterp &f) {

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();

   clear();

}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::assign(const VarInfoNcPinterp &v) {
   int i;

   // First call the parent's assign
   VarInfo::assign(v);

   // Copy
   clear_dimension();
   for(i=0; i<v.n_dimension(); i++) {
      add_dimension(v.dimension(i), v.is_offset(i), v.dim_value(i));
   }

}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
   clear_dimension();

}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoNcPinterp::dump():\n"
       << "  Dimension:\n";
   Dimension.dump(out);
   out << "  Is_offset:\n";
   Is_offset.dump(out);
   out << "  Dim_value:\n";
   Dim_value.dump(out);

}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::add_dimension(int dim, bool as_offset, double dim_value) {
   Dimension.add(dim);
   Is_offset.add(as_offset);
   Dim_value.add(dim_value);
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::clear_dimension() {
   Dimension.clear();
   Is_offset.clear();
   Dim_value.clear();
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::set_dimension(int i_dim, int dim) {
   Dimension[i_dim] = dim;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::set_magic(const ConcatString &nstr, const ConcatString &lstr) {
   ConcatString tmp_str;
   char *ptr = (char *) 0, *ptr2 = (char *) 0, *ptr3 = (char *) 0, *save_ptr = (char *) 0;
   const char *method_name = "VarInfoNcPinterp::set_magic() -> ";

   // Store the magic string
   VarInfo::set_magic(nstr, lstr);

   // Set the requested name and default output name
   set_req_name(nstr.c_str());
   set_name(nstr);

   // If there's no level specification, assume (0,*,*)
   if(strchr(lstr.c_str(), '(') == nullptr) {
      Level.set_req_name("0,*,*");
      Level.set_name("0,*,*");
      clear_dimension();
      add_dimension(0);
      add_dimension(vx_data2d_star);
      add_dimension(vx_data2d_star);
   }
   // Parse the level specification
   else {

      // Initialize the temp string
      tmp_str = lstr;

      // Retreive the NetCDF level specification
      ptr = strtok_r((char*)tmp_str.c_str(), "()", &save_ptr);

      // Set the level name
      Level.set_req_name(ptr);
      Level.set_name(ptr);

      // If dimensions are specified, clear the default value
      if(strchr(ptr, ',') != nullptr) clear_dimension();

      // Parse the dimensions
      bool as_offset = true;
      while((ptr2 = strtok_r(ptr, ",", &save_ptr)) != nullptr) {

         // Check for wildcards
         if(strchr(ptr2, '*') != nullptr) add_dimension(vx_data2d_star);
         else {

            as_offset = (*ptr2 != '@');
            if (!as_offset) ptr2++;

            // Check for a range of levels
            if((ptr3 = strchr(ptr2, '-')) != nullptr) {

               // Check if a range has already been supplied
               if(Dimension.has(range_flag)) {
                  mlog << Error << "\n" << method_name
                       << "only one dimension can have a range for NetCDF variable \""
                       << MagicStr << "\".\n\n";
                  exit(1);
               }
               // Store the dimension of the range and limits
               else {
                  add_dimension(range_flag);
                  Level.set_lower(atoi(ptr2));
                  Level.set_upper(atoi(++ptr3));

                  // Assume pressure level type for a range of levels
                  Level.set_type(LevelType_Pres);
               }
            }
            // Single level
            else {
               int level = 0;
               double level_value = bad_data_double;
               if (is_datestring(ptr2)) {
                  unixtime unix_time = timestring_to_unix(ptr2);
                  level = vx_data2d_dim_by_value;
                  level_value = unix_time;
                  as_offset = false;
               }
	       else if (is_number(ptr2)) {
                  if (as_offset) level = atoi(ptr2);
                  else {
                     level = vx_data2d_dim_by_value;
                     level_value = atof(ptr2);
                  }
               }
               else if (is_datestring(ptr2)) {
                  unixtime unix_time = timestring_to_unix(ptr2);
                  level = vx_data2d_dim_by_value;
                  level_value = unix_time;
                  as_offset = false;
               }
               else {
                  mlog << Error << "\n" << method_name
                       << "trouble parsing NetCDF dimension value \""
                       << ptr2 << "\"!\n\n";
                  exit(1);
               }
               if (as_offset) add_dimension(level, as_offset);
               else add_dimension(level, as_offset, level_value);
            }
         }

         // Set ptr to nullptr for next call to strtok
         ptr = nullptr;

      } // end while

   } // end else

   // Check for "/PROB" to indicate a probability forecast
   if(strstr(MagicStr.c_str(), "/PROB") != nullptr) PFlag = 1;

   // Set the long name
   tmp_str.format("%s(%s)", name().text(), Level.name().text());
   set_long_name(tmp_str.c_str());

   // Set the units
   set_units(na_str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::set_dict(Dictionary & dict) {

   VarInfo::set_dict(dict);

   set_magic(dict.lookup_string("name"),
             dict.lookup_string("level"));
   set_req_name( dict.lookup_string("name").c_str() );

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_precipitation() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsPrecipitation)) {
      return(SetAttrIsPrecipitation != 0);
   }

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // precipitation variables.
   //
   return(has_prefix(pinterp_precipitation_names,
                     n_pinterp_precipitation_names,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_specific_humidity() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsSpecificHumidity)) {
      return(SetAttrIsSpecificHumidity != 0);
   }

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // specific humidity variables.
   //
   return(has_prefix(pinterp_specific_humidity_names,
                     n_pinterp_specific_humidity_names,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_u_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsUWind)) {
      return(SetAttrIsUWind != 0);
   }

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // U-component of wind variables.
   //
   return(has_prefix(pinterp_u_wind_names,
                     n_pinterp_u_wind_names,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_v_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsVWind)) {
      return(SetAttrIsVWind != 0);
   }

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // V-component of wind variables.
   //
   return(has_prefix(pinterp_v_wind_names,
                     n_pinterp_v_wind_names,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_wind_speed() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindSpeed)) {
      return(SetAttrIsWindSpeed != 0);
   }

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // wind speed variables.
   //
   return(has_prefix(pinterp_wind_speed_names,
                     n_pinterp_wind_speed_names,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_wind_direction() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindDirection)) {
      return(SetAttrIsWindDirection != 0);
   }

   return(false);
}


///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_grid_relative() const {

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // variables that should be rotated from grid-relative to earth-relative.
   //
   return(has_prefix(pinterp_grid_relative_names,
                     n_pinterp_grid_relative_names,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////
