@ECHO off

SET BINARYCREATOR="C:\Qt\Tools\QtInstallerFramework\3.1\bin\binarycreator.exe"

ECHO Build ppm-installer-32bit.exe
%BINARYCREATOR% --offline-only -c MSVC2017_32bit\config\config.xml -p MSVC2017_32bit\packages ppm-installer-32bit.exe

ECHO Build ppm-installer-64bit.exe
%BINARYCREATOR% --offline-only -c MSVC2017_64bit\config\config.xml -p MSVC2017_64bit\packages ppm-installer-64bit.exe

PAUSE
