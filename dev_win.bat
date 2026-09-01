cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=windows.cm -DCMAKE_BUILD_TYPE=Release . -B build-windows
cd build-windows
make -j8
pause