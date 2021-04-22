building the builder:

emcmake cmake \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -Draylib_INCLUDE_DIR=/usr/local/include \
  -Draylib_LIBRARY=/usr/local/lib/libraylib.a \
  -DCMAKE_INSTALL_PREFIX=../install \
  ../repo


building:

emmake make install
