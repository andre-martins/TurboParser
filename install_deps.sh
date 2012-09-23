BASE_DIR=$(cd $(dirname "$0"); pwd)
LOCAL_DEPS_DIR=${BASE_DIR}/deps/local

cd deps
mkdir -p ${LOCAL_DEPS_DIR}/lib
mkdir -p ${LOCAL_DEPS_DIR}/include

# Add the local lib folder to the search path. This is important because glog
# searches for libgflags.
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${LOCAL_DEPS_DIR}/lib:"

# Install gflags.
echo ""
echo "Installing gflags..."
tar -zxf gflags-2.0-no-svn-files.tar.gz
cd gflags-2.0
./configure --prefix=${LOCAL_DEPS_DIR} && make && make install
cd ..
echo "Done."

# Install glog.
echo ""
echo "Installing glog..."
tar -zxf glog-0.3.2.tar.gz
cd glog-0.3.2
./configure --prefix=${LOCAL_DEPS_DIR} --with-gflags=${LOCAL_DEPS_DIR} \
  && make && make install
cd ..
echo "Done."

# Install ad3.
echo ""
echo "Installing ad3..."
tar -zxf ad3_v2.tar.gz
cd ad3_v2
make
cp ad3/libad3.a ${LOCAL_DEPS_DIR}/lib
mkdir ${LOCAL_DEPS_DIR}/include/ad3
cp -r ad3/*.h ${LOCAL_DEPS_DIR}/include/ad3
cp -r Eigen ${LOCAL_DEPS_DIR}/include
cd ..
echo "Done."
