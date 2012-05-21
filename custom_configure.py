
######
# Custom build script.
#
# Defines a number of options for 'configure' for netcdf4. 
# This script will, based on command-line switches,
# configure netcdf source for different builds. If no options
# are specified, it will prompt for configuration options
# interactively.
######
from optparse import OptionParser
import os


#global C_FLAGS, LD_FLAGS, STD_ARGS
C_FLAGS="CFLAGS=\""
LD_FLAGS="LDFLAGS=\""
STD_ARGS="--disable-doxygen --enable-diskless "

### Parse the options which were passed to the program.
def parse_options(ops):
    cur_args=STD_ARGS
    cur_c_flags=C_FLAGS
    cur_ld_flags=LD_FLAGS    
    if ops.BUILD_64 == True:
        cur_args += "--host=x86_64-w64-mingw32 "
        
    if ops.BUILD_MIN == True:
        cur_args += "--disable-dap --disable-netcdf-4 --disable-diskless"
        
    if ops.BUILD_DEBUG == True:
        cur_c_flags = cur_c_flags + "-ggdb -O0 "
        cur_ld_flags = cur_ld_flags + "-ggdb -O0 "       
        
    if ops.BUILD_STATIC == True:
        cur_args += "--enable-static "        
        if ops.BUILD_32 == True:
            cur_c_flags += "-I/c/Users/wfisher/Desktop/hdf5-1.8.8/src -I/c/Users/wfisher/Desktop/hdf-1.8.8/hl/src "
            cur_ld_flags += "-static -L/c/GnuWin32/lib/libz.a -L/c/Users/wfisher/Desktop/hdf5-1.8.8/build_win/bin "
        else:
            cur_c_flags += "-I/c/HDF5189/include "
            cur_ld_flags += "-static -L/c/HDF5189/bin "
    
    if ops.BUILD_SHARED == True:
        cur_args += "--enable-shared "
        if ops.BUILD_32 == True:
            cur_c_flags += "-I/c/Users/wfisher/Desktop/hdf5-1.8.8/src -I/c/Users/wfisher/Desktop/hdf-1.8.8/hl/src "
            cur_ld_flags += "-L/c/GnuWin32/lib/libz.a -L/c/Users/wfisher/Desktop/hdf5-1.8.8/build_win/bin "
        else:
            cur_c_flags += "-I/c/HDF5189/include "
            cur_ld_flags += "-L/c/HDF5189/bin "
    
    return "bash ./configure " + cur_args + " " + cur_c_flags + "\" " + cur_ld_flags + "\""

###
# Main Function.
###
def main():
    parser = OptionParser()
    parser.add_option("--m32","--build-32",dest="BUILD_32",action="store_true",
                      help="Enable 32-bit build.",default=False)
                      
    parser.add_option("--m64","--build-64",dest="BUILD_64",action="store_true",
                      help="Enable 64-bit build.",default=False)
    
    parser.add_option("--static",dest="BUILD_STATIC",action="store_true",
                      help="Enable static build.",default=False)
                      
    parser.add_option("--shared","--dynamic",dest="BUILD_SHARED",action="store_true",
                      help="Enable shard build.",default=False)
                      
    parser.add_option("--min","--min_build",dest="BUILD_MIN",action="store_true",
                      help="Build Minimal netcdf. No netcdf4, dap, diskless, etc.",default=True)
                      
    parser.add_option("-d","--debug",dest="BUILD_DEBUG",action="store_true",
                      help="Build with debug options (-ggdb, -O0)",default=False)
                      
    parser.add_option("-e","--echo",dest="ECHO_ONLY",action="store_true",
                      help="Don't execute configure, just echo output.",default=False)
    (options,args) = parser.parse_args()
    #print("options:",str(options))
    #print("arguments:",args)
    all_args = parse_options(options)
    print all_args
    if options.ECHO_ONLY == False:
        os.system(all_args)
    
main()     
                      
                      
    
    
