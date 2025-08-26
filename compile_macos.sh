#!/bin/bash

PDIR="${HOME}/Code/it3"
DIR=~/Library/Application\ Support/It
FILE="${DIR}/$1" # $1: f_quadratic (without extension)
EXE="${PDIR}/build/Qt_6_6_0_for_macOS_qt_qt6-Debug/it3.app/Contents/MacOS/it3"

ARCH=`uname -m` # arm64 or x86_64
COMPILER=`which c++`
LINKER=${COMPILER}
CPPFLAGS="-std=gnu++17 -arch ${ARCH}"
IFLAGS="-I${PDIR}/it"
#LFLAGS="-L /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib -stdlib=libc++ -arch x86_64 -dynamiclib -L. -F. -w"
LFLAGS="-stdlib=libc++ -arch ${ARCH} -dynamiclib -L. -F."

COMPILE="${COMPILER} ${CPPFLAGS} ${IFLAGS}"
LINK="${LINKER} ${LFLAGS}"
NEEDLINK="NO"

cd "$DIR"
mkdir -p build
cd build

echo "compile $1"

if [ "CLEAN" == "$2" ]; then
  echo "Cleaning first"
  rm -f *.o
  rm -f "$1.dylib"
fi
rm -f ITFUN.cpp
rm -f errors.txt
rm -f linkerrors.txt

if [ "$1.dylib" -nt $EXE ]; then
  rm -f "$1.dylib"
fi

if [ ! -a "$1.dylib" -o "${FILE}.cpp" -nt "$1.dylib" ]; then
  cat "../Compile/prefix.txt" "${FILE}.cpp" > ITFUN.cpp
  CLASSNAME=`grep -o 'CLASS([^,]*' ITFUN.cpp | sed s/CLASS\(//`
  POSTFIX="extern \"C\" void *_createFunction(int pspace) { return new ${CLASSNAME}(\"${CLASSNAME}\", \"label\", pspace); }"
  echo $POSTFIX >> ITFUN.cpp
  $COMPILE -c ITFUN.cpp -o ITFUN.o > errors.txt 2>&1
else
  touch errors.txt
fi

for f in Args Colormap Function State MTComplex MTRandom; do
  if [ ! -a "${f}.o" -o "${PDIR}/it/${f}.cpp" -nt "${f}.o" ]; then
    $COMPILE -c "${PDIR}/it/${f}.cpp" -o ${f}.o >> errors.txt 2>&1
    NEEDLINK="YES"
  fi
done

$LINK ITFUN.o Args.o Colormap.o Function.o State.o MTComplex.o MTRandom.o -o "$1.dylib" >> errors.txt

#if [ ! -s errors.txt ] && [ ! -s linkerrors.txt ]; then
if [ ! -s errors.txt ]; then
    exit 0
else
    exit 1  # Exit with error if either file has content
fi
