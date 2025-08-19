#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
Script to create Zarr datasets using zarr-python"
"""
from sys import version as python_version
import zarr, xarray as xr, numpy as np
import numcodecs
import argparse
from pathlib import Path

def xarray_create_zarr_vs_netcdf(name):
    ds = xr.Dataset({'var':np.int8(2**7-1)})
    ds.to_netcdf(f'{name}.nc')
    ds.to_zarr(f'{name}.zarr', mode='w', consolidated=True)

def zarr_create_dataset(name):
    ds = zarr.open(f'{name}.zarr','w')
    # Attributes
    ds.attrs['Description'] = 'Zarr dataset created using zarr-python'
    ds.attrs['python'] = python_version
    ds.attrs['zarr-python'] = zarr.__version__
    ds.attrs[1] = 1
    
    scalars = ds.create_group('scalars')
    scalars.attrs['Description'] = "'Scalar' data types or unlimited arrays with size 1"

    # # scalar like arrays
    scalars.ones('pi',shape=())
    scalars.pi.fill_value = 3.1415
    scalars.create(name='int-b',shape=(),dtype='uint8')
    scalars.array(name='int-c',data=(3))
    # scalars with fill-in
    scalars.create(name='int-fillin',shape=(),fill_value=3.1415)

    
    arrays= ds.create_group('arrays')
    arrays.attrs['Description'] = 'Arrays groups by data type, int or float'
    
    # # Arrays
    ints = arrays.create_group('ints')
    arrays.attrs['Description'] = 'Integer arrays with different int sizes'
    for t in ('int8', 'int16', 'int32', 'int64', 'uint8', 'uint16', 'uint32', 'uint64'):
        ints.create(name=f'{t}-2x3',shape=(2,3),chunks=(2,2),dtype=t)
    
    floats = arrays.create_group('floats')
    arrays.attrs['Description'] = 'Float arrays with different float sizes'
    for t in ('float32', 'float64'):
        floats.create(name=f'{t}-2x3',shape=(2,3),chunks=(2,2),dtype=t)
    
    # Misc
    misc = arrays.create_group('misc')
    misc.attrs['Description'] = 'Miscellaneous arrays, with other types'
    misc['string'] = 'string'
    misc['byte-string'] = b'string'

    zarr.consolidate_metadata(ds.store)

def argument_parser():
    parser = argparse.ArgumentParser(description='Create Zarr datasets using python tooling')
    parser.add_argument('operation', choices=('create',), help='Use xarray or zarr to create the Zarr dataset')
    parser.add_argument('type', default='zarr', choices=['zarr','xarray'], help='Use xarray or zarr to create the Zarr dataset')
    parser.add_argument('filename', nargs='?', type=Path, default=f'python-{zarr.__version__}', help=f'Name of the Zarr dataset (defaults to `zarr-python-{zarr.__version__}.zarr`')
    parser.add_argument('--consolidate', action='store_true', help='Consolidate metadata')
    return parser

def sanitize_filename(args):
    filename = Path(args.filename)
    if filename.is_dir():
        filename = filename / Path(f'python-{zarr.__version__}')
    filename = filename.parent / Path(args.type +'-' + filename.name)
    while filename.suffix in ('.zarr', '.nc'):
        filename = filename.with_suffix('')
    return filename

def run ():
    
    args = argument_parser().parse_args()

    print(f"""Testing zarr-python compatibility using:
               python = {python_version} 
                 zarr = {zarr.__version__} 
               xarray = {xr.__version__} 
                numpy = {np.__version__} 
            numcodecs = {numcodecs.__version__}""")
    filename = sanitize_filename(args)    
    
    if args.type == 'xarray':
        xarray_create_zarr_vs_netcdf(filename)
        print(f"Datasets {filename}.zarr and {filename}.nc created") 
    elif args.type == 'zarr':
        zarr_create_dataset(filename)
        print(f"Zarr dataset {filename}.zarr created") 


if __name__ == "__main__":
    run()