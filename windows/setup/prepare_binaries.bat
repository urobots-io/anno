rd /s /q temp
mkdir temp
copy ..\x64\Release\anno.exe temp\anno.exe
set /p QtDir=<..\x64\Release\_qt_path.txt
echo %QtDir%
%QtDir%\bin\windeployqt --no-compiler-runtime --release --no-translations --no-system-d3d-compiler temp/anno.exe