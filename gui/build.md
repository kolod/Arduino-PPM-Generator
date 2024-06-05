# Build instructions

## Debian

1. Install basic build tools
```bash
apt update
apt install build-essentials
apt install cmake
apt install qt6-serialbus-dev
apt install qt6-serialport-dev
apt install qt6-tools-dev
```

2. Install IDE (Optional)
```bash
apt update
apt install qtcreator
apt install linguist-qt6
```

3. Install help files
Packages isn't not present in Debian 12 Bookworm therefore install them from Debian 13 Trixie
```bash
wget http://ftp.debian.org/debian/pool/main/q/qt6-base/qt6-base-doc_6.4.2+dfsg-21.1_all.deb
apt install ./qt6-base-doc_6.4.2+dfsg-21.1_all.deb
wget http://ftp.debian.org/debian/pool/main/q/qt6-serialbus/qt6-serialbus-doc_6.4.2-4_all.deb
apt install ./qt6-serialbus-doc_6.4.2-4_all.deb
wget http://ftp.debian.org/debian/pool/main/q/qt6-serialport/qt6-serialport-doc_6.4.2-4_all.deb
apt install ./qt6-serialport-doc_6.4.2-4_all.deb
wget http://ftp.debian.org/debian/pool/main/q/qt6-charts/qt6-charts-doc_6.4.2-5_all.deb
apt install ./qt6-charts-doc_6.4.2-5_all.deb
```

4. Configure
```bash
cmake -S ./gui -B ./build-gui-release -DCMAKE_GENERATOR:STRING=Ninja -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++

-- The CXX compiler identification is GNU 12.2.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE
-- Performing Test HAVE_STDATOMIC
-- Performing Test HAVE_STDATOMIC - Success
-- Found WrapAtomic: TRUE
-- Found OpenGL: /usr/lib/x86_64-linux-gnu/libOpenGL.so
-- Found WrapOpenGL: TRUE
-- Found XKB: /usr/lib/x86_64-linux-gnu/libxkbcommon.so (Required is at least version "0.5.0")
-- Found WrapVulkanHeaders: /usr/include
-- Configuring done
-- Generating done
-- Build files have been written to: /home/alex/Projects/Arduino-PPM-Generator/build-gui-release

```

5. Build
```bash
cmake --build ./build-gui-release --target all

[1/12] Generating translations/ppm_de.qm
Updating '/home/alex/Projects/Arduino-PPM-Generator/build-gui-release/translations/ppm_de.qm'...
Removing translations equal to source text in '/home/alex/Projects/Arduino-PPM-Generator/build-gui-release/translations/ppm_de.qm'...
    Generated 24 translation(s) (24 finished and 0 unfinished)
[2/12] Generating translations/ppm_ru.qm
Updating '/home/alex/Projects/Arduino-PPM-Generator/build-gui-release/translations/ppm_ru.qm'...
Removing translations equal to source text in '/home/alex/Projects/Arduino-PPM-Generator/build-gui-release/translations/ppm_ru.qm'...
    Generated 30 translation(s) (30 finished and 0 unfinished)
[3/12] Generating translations/ppm_uk.qm
Updating '/home/alex/Projects/Arduino-PPM-Generator/build-gui-release/translations/ppm_uk.qm'...
Removing translations equal to source text in '/home/alex/Projects/Arduino-PPM-Generator/build-gui-release/translations/ppm_uk.qm'...
    Generated 30 translation(s) (30 finished and 0 unfinished)
[12/12] Linking CXX executable ppm-generator
```

6. Make Debian package
```bash
cd build-gui-release
cpack

CPack: Create package using DEB
CPack: Install projects
CPack: - Install project: ppm-generator []
CPack: Create package
CPack: - package: /home/alex/Projects/Arduino-PPM-Generator/build-gui-release/ppm-generator_1.1.0_amd64.deb generated.
```
