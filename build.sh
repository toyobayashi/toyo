type="Release"
dll="false"

until [ $# -eq 0 ]
do
if [ "$1" == "Release" ]; then type="$1"; fi
if [ "$1" == "Debug" ]; then type="$1"; fi
if [ "$1" == "dll" ]; then dll="true"; fi
shift
done

unamestr=`uname`
os=`echo $unamestr | tr "A-Z" "a-z"`

mkdir -p "./build/$os/$type"
cd "./build/$os/$type"
echo "cmake -DBUILD_DLL=$dll -DCMAKE_BUILD_TYPE=$type ../../.."
cmake -DBUILD_DLL="$dll" -DCMAKE_BUILD_TYPE=$type ../../..
cmake --build .
cd ../../..

if [ "$type" == "Release" ]; then
  headerout="dist/include/toyo"
  mkdir -p "$headerout"
  mkdir -p "dist/$os/lib"
  mkdir -p "dist/$os/bin"
  cp ./include/* "$headerout"
  cp ./build/"$os"/Release/*.a "dist/$os/lib"
  cp ./build/"$os"/Release/{*.so,*.dylib} "dist/$os/bin"

  src_dir="./build/$os/Release"
  dest_dir="dist/$os/bin"
  for f in `find build/$os/Release -maxdepth 1 -type f -regex ".*/[^.]+$" | grep -v Makefile`;
  do
    cp $f $dest_dir;
  done
fi
