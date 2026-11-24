@echo off
rem Builds the third-party libraries of anno for Windows (Visual Studio, x64):
rem   Imath + OpenEXR (EXR images) and OpenCV (image and video loading, processing).
rem   OpenCV downloads its prebuilt FFmpeg DLL (opencv_videoio_ffmpeg) for video decoding.
rem
rem Usage:   build_dependencies.bat [target folder]
rem Default target folder: %UR_LIB_DIR%, or ..\..\anno-deps next to the repository.
rem
rem Requirements: Visual Studio 2022 or newer with C++ (its bundled CMake is used
rem if cmake is not in PATH), Git, internet access.
rem Takes about 20-40 minutes. Already cloned sources are reused.
rem
rem After the build, set the environment variables printed at the end
rem (OPENCV_DIR, OPENEXR_DIR) or keep the default folder %UR_LIB_DIR%.

setlocal EnableExtensions

set IMATH_VERSION=3.2.3
set OPENEXR_VERSION=3.5.5
set OPENCV_VERSION=5.1.0

rem --- target folder ---
set ROOT=%~1
if "%ROOT%"=="" set ROOT=%UR_LIB_DIR%
if "%ROOT%"=="" set ROOT=%~dp0..\..\anno-deps
if not exist "%ROOT%" mkdir "%ROOT%" || goto :error
pushd "%ROOT%" || goto :error
set ROOT=%CD%
popd

set EXR_ROOT=%ROOT%\openexr-%OPENEXR_VERSION%
set EXR_INSTALL=%EXR_ROOT%\install
set CV_ROOT=%ROOT%\opencv-%OPENCV_VERSION%
set CV_INSTALL=%CV_ROOT%\install

rem --- tools ---
where git >nul 2>&1 || (echo Git not found in PATH & goto :error)
set CMAKE=cmake
where cmake >nul 2>&1 && goto :cmake_found
set VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -find Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`) do set CMAKE=%%i
if not exist "%CMAKE%" (echo CMake not found & goto :error)
:cmake_found
echo Using CMake: %CMAKE%
echo Target folder: %ROOT%

rem --- Imath (installed together with OpenEXR) ---
call :clone https://github.com/AcademySoftwareFoundation/Imath.git v%IMATH_VERSION% "%EXR_ROOT%\imath-src" || goto :error
"%CMAKE%" -S "%EXR_ROOT%\imath-src" -B "%EXR_ROOT%\imath-build" -A x64 ^
    -DCMAKE_INSTALL_PREFIX="%EXR_INSTALL%" ^
    -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DPYTHON=OFF || goto :error
call :build_install "%EXR_ROOT%\imath-build" || goto :error

rem --- OpenEXR ---
call :clone https://github.com/AcademySoftwareFoundation/openexr.git v%OPENEXR_VERSION% "%EXR_ROOT%\src" || goto :error
"%CMAKE%" -S "%EXR_ROOT%\src" -B "%EXR_ROOT%\build" -A x64 ^
    -DCMAKE_INSTALL_PREFIX="%EXR_INSTALL%" ^
    -DCMAKE_PREFIX_PATH="%EXR_INSTALL%" ^
    -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF ^
    -DOPENEXR_BUILD_TOOLS=OFF -DOPENEXR_INSTALL_TOOLS=OFF -DOPENEXR_BUILD_EXAMPLES=OFF ^
    -DOPENEXR_FORCE_INTERNAL_DEFLATE=ON -DOPENEXR_FORCE_INTERNAL_OPENJPH=ON || goto :error
call :build_install "%EXR_ROOT%\build" || goto :error

rem --- OpenCV (only the modules used by anno) ---
call :clone https://github.com/opencv/opencv.git %OPENCV_VERSION% "%CV_ROOT%\src" || goto :error
"%CMAKE%" -S "%CV_ROOT%\src" -B "%CV_ROOT%\build" -A x64 ^
    -DCMAKE_INSTALL_PREFIX="%CV_INSTALL%" ^
    -DCMAKE_PREFIX_PATH="%EXR_INSTALL%" ^
    -DBUILD_LIST=core,imgproc,imgcodecs,videoio -DBUILD_opencv_videoio=ON ^
    -DBUILD_SHARED_LIBS=ON -DWITH_OPENEXR=ON ^
    -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_opencv_apps=OFF ^
    -DBUILD_DOCS=OFF -DBUILD_JAVA=OFF -DBUILD_opencv_python3=OFF ^
    -DWITH_FFMPEG=ON -DWITH_CUDA=OFF -DWITH_PROTOBUF=OFF -DBUILD_PROTOBUF=OFF || goto :error
call :build_install "%CV_ROOT%\build" || goto :error

echo.
echo ================================================================
echo Done. Set these environment variables (or keep UR_LIB_DIR=%ROOT%):
echo   OPENCV_DIR=%CV_INSTALL%
echo   OPENEXR_DIR=%EXR_INSTALL%
echo ================================================================
exit /b 0

rem --- clone <url> <tag> <folder>, skipped if the folder exists ---
:clone
if exist "%~3\.git" (
    echo Using existing sources in %~3
    exit /b 0
)
git clone --depth 1 --branch %2 %1 "%~3"
exit /b %errorlevel%

rem --- build and install Release and Debug ---
:build_install
"%CMAKE%" --build "%~1" --config Release --parallel || exit /b 1
"%CMAKE%" --install "%~1" --config Release || exit /b 1
"%CMAKE%" --build "%~1" --config Debug --parallel || exit /b 1
"%CMAKE%" --install "%~1" --config Debug || exit /b 1
exit /b 0

:error
echo.
echo *** Build of the dependencies failed ***
exit /b 1
