@echo off
REM Windows batch equivalent of the macOS build script
REM
REM Usage: compile.bat filename [filesDirectory] [mainExecutable] [CLEAN]
REM Example: compile.bat f_quadratic
REM Example: compile.bat f_quadratic "C:\MyApp\Data" "C:\MyApp\It.exe" CLEAN

setlocal enabledelayedexpansion

REM Get arguments
set FILENAME=%~1
set DIR=%~2
set EXE=%~3
set CLEAN_FLAG=%~4

REM Set defaults if not provided
if "%FILENAME%"=="" (
    echo Usage: %0 filename [filesDirectory] [mainExecutable] [CLEAN]
    exit /b 1
)

if "%DIR%"=="" set DIR=%APPDATA%\It\
if "%EXE%"=="" set EXE=%USERPROFILE%\Code\it3\build\Debug\It.exe

REM Determine architecture
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set ARCH=x64
) else if "%PROCESSOR_ARCHITEW6432%"=="AMD64" (
    set ARCH=x64
) else (
    set ARCH=x86
)

REM Find and initialize Visual Studio
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VSINSTALLPATH=%%i"
    )
    if defined VSINSTALLPATH (
        call "!VSINSTALLPATH!\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
    )
)

REM Check if compiler is available
where cl.exe >nul 2>&1
if errorlevel 1 (
    echo No Visual Studio compiler found. Please ensure Visual Studio is installed with C++ tools.
    if not exist "%DIR%\build" mkdir "%DIR%\build"
    echo No Visual Studio compiler found > "%DIR%\build\errors.txt"
    exit /b 1
)

REM Compiler settings
set CPPFLAGS=/std:c++17 /EHsc /MD /Zi /D WIN32 /D _WINDOWS /D _USRDLL /FC /D_CRT_SECURE_NO_WARNINGS
set IFLAGS=/I..\it
set LFLAGS=/DLL /MACHINE:%ARCH% /VERBOSE

echo Using Visual Studio compiler
echo.

REM Check directory exists
if not exist "%DIR%" (
    echo Directory '%DIR%' not found
    mkdir "%DIR%\build" 2>nul
    echo Directory '%DIR%' not found > "%DIR%\build\errors.txt"
    exit /b 1
)

REM Change to directory and create build folder
cd /d "%DIR%"
if not exist build mkdir build
cd build

echo Compiling %FILENAME%

REM Clean if requested
if "%CLEAN_FLAG%"=="CLEAN" (
    echo Cleaning...
    del /q *.obj 2>nul
    del /q "%FILENAME%.dll" 2>nul
)

REM Remove old files
del /q ITFUN.cpp 2>nul
del /q errors.txt 2>nul
del /q "%FILENAME%.dll" 2>nul

REM Create errors.txt
echo. > errors.txt

REM Create prefix.txt if it doesn't exist
if not exist prefix.txt (
    echo Creating prefix.txt...
    (
        echo #include "Function.h"
        echo #include "State.h"
        echo #include "MTComplex.h"
        echo #define TWO_PI 6.2831853071796
        echo #define ABS^(x^) sqrt^(norm^(x^)^)	/* abs is slow */
        echo #ifdef WIN32
        echo #pragma warning^(disable: 4127^)
        echo #endif
    ) > prefix.txt
)

REM Check if source file exists
if not exist "..\%FILENAME%.cpp" (
    echo Source file '..\%FILENAME%.cpp' not found
    echo Source file '..\%FILENAME%.cpp' not found >> errors.txt
    exit /b 1
)

REM Create ITFUN.cpp by combining prefix and source
echo Creating ITFUN.cpp...
copy /b prefix.txt + "..\%FILENAME%.cpp" ITFUN_temp.cpp >nul

REM Extract class name from the source
findstr /r "CLASS(" ITFUN_temp.cpp > temp_class.txt
for /f "tokens=1 delims=," %%a in ('findstr /r "CLASS(" temp_class.txt') do (
    set LINE=%%a
    set LINE=!LINE:CLASS(=!
    set CLASSNAME=!LINE!
)
del temp_class.txt

if "%CLASSNAME%"=="" (
    echo Could not find CLASS definition in %FILENAME%.cpp
    echo Could not find CLASS definition >> errors.txt
    exit /b 1
)

REM Add the extern C function to ITFUN.cpp
(
    type ITFUN_temp.cpp
    echo.
    rem echo extern "C" __declspec^(dllexport^) void *_createFunction^(int pspace^) { return new %CLASSNAME%^("%CLASSNAME%", "label", pspace^); }
    echo extern "C" __declspec^(dllexport^) void * __cdecl _createFunction^(int pspace^) { static %CLASSNAME% instance^("%CLASSNAME%", "label", pspace^); return ^&instance; }
) > ITFUN.cpp
del ITFUN_temp.cpp

echo Found class: %CLASSNAME%

REM Compile ITFUN.cpp
echo Compiling ITFUN.cpp...
cl.exe %CPPFLAGS% %IFLAGS% /c ITFUN.cpp /Fo:ITFUN.obj >> errors.txt 2>&1
if errorlevel 1 (
    echo Compilation failed for ITFUN.cpp
    goto :show_errors
)

REM Compile other source files
set SOURCEFILES=Args Colormap Function State MTComplex MTRandom debug
set OBJFILES=ITFUN.obj

for %%f in (%SOURCEFILES%) do (
    set SOURCEPATH=..\it\%%f.cpp
    set OBJPATH=%%f.obj

    if exist "!SOURCEPATH!" (
        REM Check if we need to recompile (source newer than object)
        set NEED_COMPILE=0
        if not exist "!OBJPATH!" set NEED_COMPILE=1

        REM Simple timestamp check - if source exists and obj doesn't, compile
        if !NEED_COMPILE!==1 (
            echo Compiling %%f.cpp...
            cl.exe %CPPFLAGS% %IFLAGS% /c "!SOURCEPATH!" /Fo:!OBJPATH! >> errors.txt 2>&1
            if errorlevel 1 (
                echo Compilation failed for %%f.cpp
                goto :show_errors
            )
        )
        set OBJFILES=!OBJFILES! !OBJPATH!
    )
)

REM Link
echo Linking %FILENAME%.dll...
link.exe %LFLAGS% /OUT:%FILENAME%.dll %OBJFILES% >> errors.txt 2>&1
if errorlevel 1 (
    echo Linking failed
    goto :show_errors
)

REM Check for success
if exist "%FILENAME%.dll" (
    echo.
    echo Compiled successfully
    exit /b 0
) else (
    echo Build failed - DLL not created
    goto :show_errors
)

:show_errors
echo.
echo Build failed with errors:
echo ========================
type errors.txt
echo ========================
exit /b 1
