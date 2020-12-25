@ECHO off

SET BINARYCREATOR="C:\Qt\Tools\QtInstallerFramework\4.0\bin\binarycreator.exe"

ECHO Build ppm-installer-32bit.exe
copy /y /b ..\build-ppm-generator-Desktop_Qt_5_15_2_MSVC2019_32bit-Release\release\ppm-generator.exe .\MSVC2019_32bit\packages\org.kolodkin.ppm\data
C:\Qt\5.15.2\msvc2019\bin\windeployqt.exe .\MSVC2019_32bit\packages\org.kolodkin.ppm\data
%BINARYCREATOR% --offline-only -c MSVC2019_32bit\config\config.xml -p MSVC2019_32bit\packages ppm-installer-32bit.exe

ECHO Build ppm-installer-64bit.exe
copy /y /b ..\build-ppm-generator-Desktop_Qt_5_15_2_MSVC2019_64bit-Release\release\ppm-generator.exe .\MSVC2019_64bit\packages\org.kolodkin.ppm\data
C:\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe .\MSVC2019_64bit\packages\org.kolodkin.ppm\data
%BINARYCREATOR% --offline-only -c MSVC2019_64bit\config\config.xml -p MSVC2019_64bit\packages ppm-installer-64bit.exe

PAUSE
