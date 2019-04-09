SE_DIR=$(cd $(dirname "$0"); pwd)
LOCAL_DEPS_DIR=${BASE_DIR}/deps/local
export CFLAGS='-stdlib=libstdc++'
export CXXFLAGS='-stdlib=libstdc++'
export LFLAGS='-stdlib=libstdc++'

rm -rf ${LOCAL_DEPS_DIR}

cd deps
mkdir -p ${LOCAL_DEPS_DIR}/lib
mkdir -p ${LOCAL_DEPS_DIR}/include

# Add the local lib folder to the search path. This is important because glog
# searches for libgflags.
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${LOCAL_DEPS_DIR}/lib:"

# Set to true to run autoconf and automake (sometimes necessary in Mac OS-X).
RUN_AUTOTOOLS=true

# Install gflags.
echo ""
echo "Installing gflags..."
tar -zxf gflags-2.0-no-svn-files.tar.gz
cd gflags-2.0
if ${RUN_AUTOTOOLS}
then
  rm missing
  aclocal
  autoconf
  automake --add-missing
fi
./configure --prefix=${LOCAL_DEPS_DIR} && make && make install
cd ..
echo "Done."

# Install glog.
echo ""
echo "Installing glog..."
tar -zxf glog-0.3.2.tar.gz
cd glog-0.3.2
if ${RUN_AUTOTOOLS}
then
  rm missing
  aclocal
  autoconf
  automake --add-missing
fi
./configure --prefix=${LOCAL_DEPS_DIR} --with-gflags=${LOCAL_DEPS_DIR} \
  && make && make install
cd ..
echo "Done."

# Install ad3.
echo ""
echo "Installing ad3..."
rm -rf AD3-2.0.2
tar -zxf AD3-2.0.2.tar.gz
cd AD3-2.0.2
sed -i '' -E 's/^((CF|LF).*)/\1 -stdlib=libstdc++/g' Makefile ad3/Makefile examples/*/Makefile
make
cp ad3/libad3.a ${LOCAL_DEPS_DIR}/lib
mkdir -p ${LOCAL_DEPS_DIR}/include/ad3
cp -r ad3/*.h ${LOCAL_DEPS_DIR}/include/ad3
cd ..
rm -rf Eigen
tar --strip-components=1 -zxf eigen-eigen-c58038c56923.tar.bz2 eigen-eigen-c58038c56923/Eigen
cp -r Eigen ${LOCAL_DEPS_DIR}/include
cd ..
sed -i '' 's/#include <unordered_map>/#include <tr1\/unordered_map>/' src/classifier/Features.h
echo "Done."
