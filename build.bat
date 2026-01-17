@echo off
cls
echo Flushing build caches and output folders
rmdir /Q /S build
mkdir build

echo Creating build folders
cd build

echo Building project...
echo -------------------

cmake ..
cmake --build . --config Release

cd ..