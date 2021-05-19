#!/bin/bash  

export ROOT_DIR=$PWD
export PATH=$PATH:$ROOT_DIR/build/3part/7-Zip:$ROOT_DIR/build/3part/mingw32/bin
echo $PATH

cd $ROOT_DIR/build/3part
if [ ! -d "mingw32" ]; then
	7z x mingw32.rar -aoa
else
    echo mingw32 is exist... 	
fi

cd $ROOT_DIR

target="builds"

if [ ! -d $target ]; then
	mkdir -p $target
else
	cd $target
	rm -rf $target
	cd ..
fi

cp CMakeLists_mingw.txt CMakeLists.txt

cd $target
echo "****************************************"
echo "start to build judger ..."

cmake -G "MinGW Makefiles" ..
cmake -G "MinGW Makefiles" .

echo "cmake mingw makefiles ok ..."

cmake --build . 

echo "build success ..."

echo "****************************************"




