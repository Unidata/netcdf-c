#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
Script to create Zarr datasets using zarr-python"
"""
import zarr, xarray as xr

def run ():
    print(f"Testing zarr-python compatibility using: zarr={zarr.__version__}")

if __name__ == "__main__":
    run()