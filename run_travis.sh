#!/bin/bash

###
# Travis script, so that we can more easily
# create a test matrix using travis-ci and docker.
###

docker run --rm -it -e USEDASH=OFF -e RUNF=OFF -e RUNCXX=OFF -e RUNP=OFF -e RUNNCO=OFF -e USEAC=OFF -v $(pwd):/netcdf-c $DOCKIMG
