if [ "$1" == "" ]; then
  type="Release"
else
  type="$1"
fi

unamestr=`uname`
os=`echo $unamestr | tr "A-Z" "a-z"`

mkdir -p "./build/$os/$type"
cd "./build/$os/$type"
cmake -DCMAKE_BUILD_TYPE=$type ../../..
cmake --build .
cd ../../..

if [ "$type" == "Release" ]; then
  mkdir -p "dist/include/toyo"
  mkdir -p "dist/$os/lib"
  mkdir -p "dist/$os/bin"
  cp ./include/* dist/include/toyo
  cp ./build/"$os"/Release/*.a "dist/$os/lib"
  cp ./build/"$os"/Release/{*.so,*.dylib} "dist/$os/bin"

  src_dir="./build/$os/Release"
  dest_dir="dist/$os/bin"
  for f in `find build/$os/Release -maxdepth 1 -type f -regex ".*/[^.]+$" | grep -v Makefile`;
  do
    cp $f $dest_dir;
  done
fi
