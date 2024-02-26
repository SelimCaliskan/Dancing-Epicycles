rmdir build /s /q
mkdir build
cd build
cmake ..
cmake --build . --config Release
cd Release
copy dancing_circles.exe ..\..\Release
cd ..\..\Release
dancing_circles.exe