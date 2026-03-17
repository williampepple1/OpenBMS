@echo off
setlocal

set BUILD_DIR=build\Desktop_Qt_6_10_1_MinGW_64_bit-Debug
set QT_DIR=C:\Qt\6.10.1\mingw_64
set DEPLOY_DIR=deploy
set ISCC="C:\Program Files (x86)\Inno Setup 6\ISCC.exe"

echo === Building OpenBMS ===
cmake --build %BUILD_DIR%
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo === Preparing deployment folder ===
if exist %DEPLOY_DIR% rmdir /s /q %DEPLOY_DIR%
mkdir %DEPLOY_DIR%

copy %BUILD_DIR%\appOpenBMS.exe %DEPLOY_DIR%\
copy icon.ico %DEPLOY_DIR%\

echo === Running windeployqt ===
%QT_DIR%\bin\windeployqt6.exe --qmldir . %DEPLOY_DIR%\appOpenBMS.exe
if %ERRORLEVEL% neq 0 (
    echo windeployqt failed!
    exit /b 1
)

echo === Building installer ===
%ISCC% installer.iss
if %ERRORLEVEL% neq 0 (
    echo Installer build failed!
    exit /b 1
)

echo === Done! ===
echo Installer: OpenBMS_Setup.exe
