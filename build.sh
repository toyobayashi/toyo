if [ "$1" == "" ]; then
  type="Release"
else
  type="$1"
fi

mkdir -p "./build/linux/$type"
cd "./build/linux/$type"
cmake -DCMAKE_BUILD_TYPE=$type ../../..
cmake --build .
cd ../../..

if [ "$type" == "Release" ]; then
  uname=`uname`
  mkdir -p "dist/include/toyo"
  mkdir -p "dist/${uname,,}/lib"
  mkdir -p "dist/${uname,,}/bin"
  cp ./include/* dist/include/toyo
  cp ./build/linux/Release/{*.a,*.so,*.dylib} "dist/${uname,,}/lib"

  src_dir="./build/linux/Release"
  dest_dir="dist/${uname,,}/bin"
  for f in `find build/linux/Release -maxdepth 1 -type f -regex ".*/[^.]+$" | grep -v Makefile`;
  do
    cp $f $dest_dir;
  done
fi
