# Install base development packages.
apt-get update
apt-get -y install ubuntu-dev-tools m4 git libjpeg-dev libcurl4-openssl-dev wget htop libtool bison flex autoconf

## Install several packages from source.
# * cmake
# * hdf4
# * hdf5

# Install cmake from source
wget http://www.cmake.org/files/v2.8/cmake-2.8.12.2.tar.gz
tar -zxf cmake-2.8.12.2.tar.gz
pushd cmake-2.8.12.2
./configure --prefix=/usr/local
make install
popd
rm -rf cmake-2.8.12.2

# Install hdf4 from source.
wget http://www.hdfgroup.org/ftp/HDF/HDF_Current/src/hdf-4.2.10.tar.bz2
tar -jxf hdf-4.2.10.tar.bz2
pushd hdf-4.2.10
./configure --disable-static --enable-shared --disable-netcdf --disable-fortran --prefix=/usr/local
sudo make install
popd
rm -rf hdf-4.2.10

# Install hdf5 from source
wget http://www.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.8.12.tar.bz2
tar -jxf hdf5-1.8.12.tar.bz2
pushd hdf5-1.8.12
./configure --disable-static --enable-shared --disable-fortran --enable-hl --disable-fortran --prefix=/usr/local
make install
popd
rm -rf hdf5-1.8.12

# Clone netcdf-c from the local repository into the VM.
sudo -i -u vagrant git clone /vagrant /home/vagrant/netcdf-c

# Create a text file indicating provisioning is finished.
sudo -i -u vagrant touch /home/vagrant/PROVISION_FINISHED_SUCCESSFULLY
