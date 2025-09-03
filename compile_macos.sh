#!/bin/bash
#
# Argument 1: filename (without extension, e.g. "f_quadratic")
# Argument 2: filesDirectory (~/Library/Application Support/It/)
# Argument 3: main executable path (/Applications/It.app/Contents/MacOS/It)
#
DIR=${2:-${HOME}/Library/Application\ Support/It/}
EXE=${3:-${HOME}/Code/it3/build/Qt_6_9_2_for_macOS-Debug/It.app/Contents/MacOS/It}

ARCH=`uname -m` # arm64 or x86_64
COMPILER=`which c++`
LINKER=${COMPILER}
CPPFLAGS="-std=gnu++17 -arch ${ARCH} -g"
IFLAGS="-I../it"
#LFLAGS="-L /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib -stdlib=libc++ -arch x86_64 -dynamiclib -L. -F. -w"
LFLAGS="-stdlib=libc++ -arch ${ARCH} -dynamiclib -L. -F."

COMPILE="${COMPILER} ${CPPFLAGS} ${IFLAGS}"
LINK="${LINKER} ${LFLAGS}"
NEEDLINK="NO"

if [ "$COMPILER" == "" ]; then
  echo "No compiler found" > "${DIR}build/errors.txt"
  exit 1
fi
if [ ! -d "${DIR}" ]; then
  echo "Directory '${DIR}' not found" >  "${DIR}build/errors.txt"
  exit 1
fi

cd "$DIR"
mkdir -p build
cd build

echo "Compiling $1"

if [ "CLEAN" == "$4" ]; then
  echo "Cleaning..."
  rm -f *.o
  rm -f "$1.dylib"
fi
rm -f ITFUN.cpp
rm -f errors.txt
touch errors.txt

if [ -a "$1.dylib" -a "$1.dylib" -nt $EXE ]; then
  rm -f "$1.dylib"
fi

if [ ! -a prefix.txt ]; then
cat <<PREFIX > prefix.txt
#include "Function.h"
#include "State.h"
#include "MTComplex.h"
#define TWO_PI 6.2831853071796
#define ABS(x) sqrt(norm(x))	/* abs is slow */
#ifdef WIN32
#pragma warning(disable: 4127)
#endif

PREFIX
fi

if [ ! -a "$1.dylib" -o "../$1.cpp" -nt "$1.dylib" ]; then
  cat prefix.txt "../$1.cpp" > ITFUN.cpp
  CLASSNAME=`grep -o 'CLASS([^,]*' ITFUN.cpp | sed s/CLASS\(//`
  POSTFIX="extern \"C\" void *_createFunction(int pspace) { return new ${CLASSNAME}(\"${CLASSNAME}\", \"label\", pspace); }"
  echo $POSTFIX >> ITFUN.cpp
  $COMPILE -c ITFUN.cpp -o ITFUN.o >> errors.txt 2>&1
fi

for f in Args Colormap Function State MTComplex MTRandom debug; do
  if [ ! -a "${f}.o" -o "../it/${f}.cpp" -nt "${f}.o" ]; then
    $COMPILE -c "../it/${f}.cpp" -o ${f}.o >> errors.txt 2>&1
    NEEDLINK="YES"
  fi
done

$LINK ITFUN.o Args.o Colormap.o Function.o State.o MTComplex.o MTRandom.o debug.o -o "$1.dylib" >> errors.txt 2>&1

if [ ! -s errors.txt ]; then
    echo "Compiled successfully"
    exit 0
else
    echo "Could not compile"
    exit 1  # Exit with error if any errors
fi
