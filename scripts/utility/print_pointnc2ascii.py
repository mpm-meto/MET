#!/usr/bin/env python3

'''
Created on Oct 14, 2022

@author: hsoh

The script reads the point observation NetCDF which is generated by MET tools and prints out the text.
The header which begins with "#" can be added with "--add-header" option.

'''

from os import path
from optparse import OptionParser

import numpy as np
from netCDF4 import Dataset

def usage():
    print(f'Usage: python3 print_nc2ascii.py MET_point_obs_nc <--hide-header> <--use-comma> <--out=out_filename>')
    print(f'            MET_point_obs_nc: NetCDF filename to read (required)')
    print(f'                --add-header: to add the header (optional, default: not included)')
    print(f'                 --use-comma: use the "comma" as separator for comma separated output (optional, default: use spaces)"')
    print(f'          --out=out_filename: save the text into the file (optional, default: display to the screen)"')
    print(f'       Note: <> indicates optional arguments')

def create_parser_options(parser):
    parser.add_option("--add-header", "--add_header", dest="add_header",
            action="store_true", default=False, help=" Add header (default: not include header)")
    parser.add_option("--use-comma", "--use_comma", dest="use_comma",
            action="store_true", default=False, help=" Use comma as separator (default: False)")
    parser.add_option("-o", "--out", dest="out_file",
            default=None, help=" Save the text into the file (default: False)")
    return parser.parse_args()


class nc_tools():

    @staticmethod
    def get_num_array(nc_group, var_name):
        nc_var = nc_group.variables.get(var_name, None)
        return [] if nc_var is None else nc_var[:]

    @staticmethod
    def get_ncbyte_array_to_str(nc_var):
        nc_str_data = nc_var[:]
        if nc_var.datatype.name == 'bytes8':
            nc_str_data = [ str(s.compressed(),"utf-8") for s in nc_var[:] ]
        return nc_str_data

    @staticmethod
    def get_string_array(nc_group, var_name):
        nc_var = nc_group.variables.get(var_name, None)
        return [] if nc_var is None else nc_tools.get_ncbyte_array_to_str(nc_var)


class met_nc_point_obs():

    #EPSILON = 0.00001
    EPSILON_L = 0.000000001
    EPSILON_U = 0.000000009

    EPSILON2_L = 0.0025
    EPSILON2_U = 0.9996
 
    def dump(self, out_handler, show_header=True, use_comma=False):
        hdr_typ = self.hdr_zero if 0 == self.nhdr_typ else self.hdr_typ
        hdr_sid = self.hdr_zero if 0 == self.nhdr_sid else self.hdr_sid
        hdr_lat, hdr_lat_len = self.formated_num_array(self.hdr_lat, use_comma)
        hdr_lon, hdr_lon_len = self.formated_num_array(self.hdr_lon, use_comma)
        hdr_elv, hdr_elv_len = self.formated_num_array(self.hdr_elv, use_comma)
        headers = [list(i) for i in zip(hdr_typ, hdr_sid, self.hdr_vld, hdr_lat, hdr_lon, hdr_elv)]

        obs_lvl, obs_lvl_len = self.formated_num_array(self.obs_lvl, use_comma)
        obs_hgt, obs_hgt_len = self.formated_num_array(self.obs_hgt, use_comma)
        if use_comma:
            obs_qty = [ '"NA"' if np.ma.is_masked(i) else f'"{self.obs_qty_table[i]}"' for i in self.obs_qty ]
            obs_vid = [ f'"{self.obs_var_table[i]}"' if self.use_var_id else f'"{i}"' for i in self.obs_vid ]
        else:
            obs_qty = [ 'NA' if np.ma.is_masked(i) else f'{self.obs_qty_table[i]}' for i in self.obs_qty ]
            obs_vid = [ f'{self.obs_var_table[i]}' if self.use_var_id else f'{i:3}' for i in self.obs_vid ]

        if self.is_all_int(self.obs_val):
            obs_val = [ f'{i:.0f}' for i in self.obs_val ]
        else:
            obs_precision = self.get_precision(self.obs_val)
            obs_val = [ 'NA' if np.ma.is_masked(i) or np.isnan(i) else
                        f'{i:.1f}' if abs((i*10)-int(i*10)) < 0.000001 else
                        f'{i:.1f}' if abs((i*10)-int(i*10)) > 0.999998 else
                        f'{i:.2f}' if abs((i*100)-int(i*100)) < 0.000001 else
                        f'{i:.2f}' if abs((i*100)-int(i*100)) > 0.999998 else
                        f'{i:.3f}' if abs((i*1000)-int(i*1000)) < 0.000001 else
                        f'{i:.3f}' if abs((i*1000)-int(i*1000)) > 0.999998 else
                        f'{i:.{obs_precision}f}' for i in self.obs_val ]

            for i in range(0, len(obs_val)):
                v = obs_val[i]
                v1 = v.split('.')
                if len(v1) > 1:
                    pad_len = obs_precision - len(v1[-1])
                    obs_val[i] = self.pad(v, pad_len)

        hdr_typ_len = self.get_max_string_length(self.hdr_typ_table)
        hdr_sid_len = self.get_max_string_length(self.hdr_sid_table)
        obs_val_len = self.get_max_string_length(obs_val)
        obs_qty_len = self.get_max_string_length(self.obs_qty_table)
        var_name_len = self.get_max_string_length(self.obs_var_table)

        if show_header:
            if use_comma:
                self.out_data(out_handler, f'#msg_type,station_id,valid_time,lat,lon,elv,var_name/gc,level,height,qty,value')
            else:
                self.out_data(out_handler, f'#msg_type s_id valid_time  lat  lon  elv  var_name/gc  level  height  qty  value')

        for obs_data in [list(i) for i in zip(obs_vid, obs_lvl, obs_hgt, obs_qty, obs_val, self.obs_hid)]:
            header_arr = headers[obs_data[-1]]
            if use_comma:
                self.out_data(out_handler,
                      f'"{self.hdr_typ_table[header_arr[0]]}",'
                      f'"{self.hdr_sid_table[header_arr[1]]}",'
                      f'"{self.hdr_vld_table[header_arr[2]]}",'
                      f'{header_arr[3]},{header_arr[4]},{header_arr[5]},'
                      f'{obs_data[0]},{obs_data[1]},{obs_data[2]},{obs_data[3]},{obs_data[4]}')
            else:
                self.out_data(out_handler,
                      f'{self.hdr_typ_table[header_arr[0]]:<{hdr_typ_len}s} '
                      f'{self.hdr_sid_table[header_arr[1]]:<{hdr_sid_len}s} '
                      f'{self.hdr_vld_table[header_arr[2]]} '
                      f'{header_arr[3]:>{hdr_lat_len}s} {header_arr[4]:>{hdr_lon_len}s} {header_arr[5]:>{hdr_elv_len}s} '
                      f'{obs_data[0]:<{var_name_len}s} {obs_data[1]:>{obs_lvl_len}s} '
                      f'{obs_data[2]:>{obs_hgt_len}s} {obs_data[3]:>{obs_qty_len}s} {obs_data[4]:>{obs_val_len}s}')

    def formated_num_array(self, value_array, use_comma):
        if self.is_all_int(value_array):
            str_array = [ f'{i:.0f}' for i in value_array ]
        else:
            na_str = '"NA"' if use_comma else 'NA'
            format_digits = 2 if self.is_2_digit_precision(value_array) else 4
            str_array = [ na_str if np.ma.is_masked(i) else f'{i:.{format_digits}f}' for i in value_array ]
        max_len = self.get_max_string_length(str_array)
        return str_array, max_len

    def get_dim_size(self, nc_group, dim_name):
        nc_dim = nc_group.dimensions.get(dim_name, None)
        return 0 if nc_dim is None else nc_dim.size

    def get_max_string_length(self, str_list):
        str_len = 0;
        for str_value in str_list:
            if len(str_value) > str_len:
                str_len = len(str_value)
        return str_len

    def get_nc_var_string_data(self, nc_group, var_name):
        nc_var = nc_group.variables.get(var_name, None)
        return ['NA'] if nc_var is None else nc_tools.get_string_array(nc_group, var_name)

    def get_precision(self, data_list):
        precision = 0
        for v in data_list:
            if abs((v*10)-int(v*10)) < 0.000001 or abs((v*10)-int(v*10)) > 0.999998:
                if precision < 1:
                    precision = 1
            elif abs((v*100)-int(v*100)) < 0.000001 or abs((v*100)-int(v*100)) > 0.999998:
                if precision < 2:
                    precision = 2
            elif abs((v*1000)-int(v*1000)) < 0.000001 or abs((v*1000)-int(v*1000)) > 0.999998:
                if precision < 3:
                    precision = 3
            elif abs(v*100000000-int(v*100000000)) > 0.:
                precision = 8
                break
            elif abs(v*10000000-int(v*10000000)) > 0.:
                if precision < 7:
                    precision = 7
            elif abs(v*1000000-int(v*1000000)) > 0.:
                if precision < 6:
                    precision = 6
            elif abs(v*100000-int(v*100000)) > 0.:
                if precision < 5:
                    precision = 5
            elif abs(v*10000-int(v*10000)) > 0.:
                if precision < 4:
                    precision = 4
            elif abs(v*1000-int(v*1000)) > 0.:
                if precision < 3:
                    precision = 3
            elif abs(v*100-int(v*100)) > 0.:
                if precision < 2:
                    precision = 2
        return precision

    def is_2_digit_precision(self, value_array):
        result = True
        for a_value in value_array:
            if np.ma.is_masked(a_value):
                continue
            if self.EPSILON2_L < abs(a_value*100-int(a_value*100)) < self.EPSILON2_U:
                if abs(a_value*10-int(a_value*10)) < self.EPSILON2_U:   # to filter x.x9996
                    result = False
                    break
        return result

    def is_all_int(self, value_array):
        all_int = True
        for a_value in value_array:
            if np.ma.is_masked(a_value) or not self.is_integer(a_value):
                all_int = False
                break
        return all_int

    def is_integer(self, value):
        return (abs(value - int(value)) < self.EPSILON_L) or (abs(value - int(value)) < self.EPSILON_U)

    def out_data(self, out_filehandler, line_buf):
        if out_filehandler is not None:
            out_filehandler.write(line_buf)
            out_filehandler.write('\n')
        else:
            print(line_buf)

    def pad(self, v, pad_len):
        if pad_len == 1:
            v = f'{v}0'
        elif pad_len == 2:
            v = f'{v}00'
        elif pad_len == 3:
            v = f'{v}000'
        elif pad_len == 4:
            v = f'{v}0000'
        elif pad_len == 5:
            v = f'{v}00000'
        elif pad_len == 6:
            v = f'{v}000000'
        elif pad_len == 7:
            v = f'{v}0000000'
        return v

    def read_data(self, met_nc_name):
        nc_group = Dataset(met_nc_name, "r")
        self.use_var_id = False
        if 'use_var_id' in nc_group.ncattrs():
            self.use_var_id = nc_group.use_var_id.lower() == "true"

        self.nhdr = self.get_dim_size(nc_group, 'nhdr')
        self.hdr_zero = [ 0 for _ in range(0, self.nhdr)]
        self.nhdr_typ = self.get_dim_size(nc_group, 'nhdr_typ')   # type table
        self.nhdr_sid = self.get_dim_size(nc_group, 'nhdr_sid')   # station_id table
        self.nhdr_vld = self.get_dim_size(nc_group, 'nhdr_vld')   # valid time strings

        self.hdr_typ = nc_tools.get_num_array(nc_group, 'hdr_typ')   # (nhdr) integer
        self.hdr_sid = nc_tools.get_num_array(nc_group, 'hdr_sid')   # (nhdr) integer
        self.hdr_vld = nc_tools.get_num_array(nc_group, 'hdr_vld')   # (nhdr) integer
        self.hdr_lat = nc_tools.get_num_array(nc_group, 'hdr_lat')   # (nhdr) float
        self.hdr_lon = nc_tools.get_num_array(nc_group, 'hdr_lon')   # (nhdr) float
        self.hdr_elv = nc_tools.get_num_array(nc_group, 'hdr_elv')   # (nhdr) float
        self.hdr_typ_table = self.get_nc_var_string_data(nc_group, 'hdr_typ_table') # (nhdr_typ, mxstr2) string
        self.hdr_sid_table = self.get_nc_var_string_data(nc_group, 'hdr_sid_table') # (nhdr_sid, mxstr2) string
        self.hdr_vld_table = self.get_nc_var_string_data(nc_group, 'hdr_vld_table') # (nhdr_vld, mxstr) string

        #Observation data
        self.nobs = self.get_dim_size(nc_group, 'nobs')
        self.nobs_qty = self.get_dim_size(nc_group, 'nobs_qty')
        self.nobs_var = self.get_dim_size(nc_group, 'nobs_var')

        self.obs_qty = nc_tools.get_num_array(nc_group, 'obs_qty')  # (nobs_qty) integer, index of self.obs_qty_table
        self.obs_hid = nc_tools.get_num_array(nc_group, 'obs_hid')  # (nobs) integer
        self.obs_vid = nc_tools.get_num_array(nc_group, 'obs_vid')  # (nobs) integer, veriable index from self.obs_var_table or GRIB code
        if 0 == len(self.obs_vid):
            self.obs_vid = nc_tools.get_num_array(nc_group, 'obs_gc' )  # (nobs) integer, veriable index from self.obs_var_table or GRIB code
        self.obs_lvl = nc_tools.get_num_array(nc_group, 'obs_lvl')  # (nobs) float
        self.obs_hgt = nc_tools.get_num_array(nc_group, 'obs_hgt')  # (nobs) float
        self.obs_val = nc_tools.get_num_array(nc_group, 'obs_val')  # (nobs) float
        self.obs_qty_table = self.get_nc_var_string_data(nc_group, 'obs_qty_table') # (nobs_qty, mxstr) string
        self.obs_var_table = self.get_nc_var_string_data(nc_group, 'obs_var') # (nobs_var, mxstr2) string, required if self.use_var_id is True


if __name__ == '__main__':
    usage_str = "%prog [options]"
    parser = OptionParser(usage = usage_str)
    options, args = create_parser_options(parser)

    if 0 == len(args):
        print(' == INFO == Missing input NetCDF filename')
        usage()
    else:
        if options.out_file is not None:
            out_handler = open(options.out_file, 'w')
        else:
            out_handler = None
        met_nc_data = met_nc_point_obs()
        for nc_name in args:
            if not path.exists(nc_name):
                if nc_name.startswith('#'):
                    print(f' == INFO == ignore "{nc_name}" option')
                else:
                    print(f' == INFO == The input file {nc_name} does not exist')
                continue

            met_nc_data.read_data(nc_name)
            met_nc_data.dump(out_handler, options.add_header, options.use_comma)
            print()

        if out_handler is not None:
            out_handler.close()
            print(f'Saved to {options.out_file}')