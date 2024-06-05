cmake_minimum_required(VERSION 3.5)

project(ppm-generator VERSION 1.1.0 LANGUAGES CXX)

set(PROJECT_NAME "Arduino-PPM-Generator")
set(PROJECT_DESCRIPTION "The GUI for the Arduino Nano sketch that allows you to generate a PPM signal.")
set(PROJECT_AUTHOR "Oleksandr Kolodkin")
set(PROJECT_URL "https://github.com/kolod/Arduino-PPM-Generator")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts SerialBus SerialPort LinguistTools)

set(TS_FILES
    translations/ppm_uk.ts
    translations/ppm_ru.ts
    translations/ppm_de.ts
)

set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")

set(QRC_FILES resources.qrc)
set(RCC_FILES ${CMAKE_CURRENT_BINARY_DIR}/resources.rcc)

set(OTHER_FILES
    README.md
    BUILD.md
    LICENSE
)

set(PROJECT_SOURCES
    loader.cpp
    loader.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    ppm.cpp
    ppm.h
    ${TS_FILES}
    ${QRC_FILES}
#    ${APP_WINDOWS_RESOURCE}
)

if(WIN32)
    list(APPEND PROJECT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
endif()

qt_add_executable(ppm-generator MANUAL_FINALIZATION ${PROJECT_SOURCES})
qt_add_lupdate(ppm-generator TS_FILES ${TS_FILES} OPTIONS -locations relative -no-obsolete)
qt_add_lrelease(ppm-generator TS_FILES ${TS_FILES} QM_FILES_OUTPUT_VARIABLE QM_FILES OPTIONS -compress -removeidentical)

target_link_libraries(ppm-generator PRIVATE
    Qt6::Widgets
    Qt6::Charts
    Qt6::SerialBus
    Qt6::SerialPort
)

set_target_properties(ppm-generator PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER oleksandr.kolodkin.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ppm-generator)
endif()

# Installation
if (WIN32)
    install(TARGETS ppm-generator RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${VX_EXTRA_RESOURCE_FILES_RCC} DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${QM_FILES} DESTINATION "${CMAKE_INSTALL_BINDIR}/translations" OPTIONAL)

else()
    include(GNUInstallDirs)
    install(CODE "include(BundleUtilities) ")
    install(TARGETS ppm-generator LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/ppm-generator/translations)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/com.github.kolod.ppm-generator.desktop DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/com.github.kolod.ppm-generator.svg DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps)

    # Debian package build
    set(QT_MINIMUM_VERSION 6.4.2)
    set(DEBIAN_PACKAGE_DEPENDS
        "libqt6core6 (>=${QT_MINIMUM_VERSION}+dfsg)"
        "libqt6gui6 (>=${QT_MINIMUM_VERSION}+dfsg)"
        "libqt6widgets6 (>=${QT_MINIMUM_VERSION}+dfsg)"
        "libqt6serialbus6 (>=${QT_MINIMUM_VERSION})"
        "libqt6serialport6 (>=${QT_MINIMUM_VERSION})"
        "libqt6charts6 (>=${QT_MINIMUM_VERSION})"
    )

    list(JOIN DEBIAN_PACKAGE_DEPENDS ", " CPACK_DEBIAN_PACKAGE_DEPENDS)

    set(CPACK_DEB_COMPONENT_INSTALL ON)
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/kolod/Arduino-PPM-Generator")

    set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/../LICENSE")
    set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/../README.md")

    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Oleksandr Kolodkin <oleksandr.kolodkin@ukr.net>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
    include(CPack)

endif()
