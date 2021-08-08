cmake_minimum_required(VERSION 3.5)

project(ppm-generator VERSION 1.0.1 LANGUAGES CXX)

set(PROJECT_NAME "Arduino-PPM-Generator")
set(PROJECT_DESCRIPTION "The GUI for the Arduino Nano sketch that allows you to generate a PPM signal.")
set(PROJECT_AUTHOR "Oleksandr Kolodkin")
set(PROJECT_URL "https://github.com/kolod/Arduino-PPM-Generator")
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 1)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

#if(ANDROID)
#	set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#	if (ANDROID_ABI STREQUAL "armeabi-v7a")
#		set(ANDROID_EXTRA_LIBS
#			${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#			${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so
#		)
#	endif()
#endif()

find_package(QT
	NAMES Qt5 Qt6
	REQUIRED COMPONENTS Widgets Charts SerialBus SerialPort LinguistTools
)

find_package(Qt${QT_VERSION_MAJOR}
	REQUIRED COMPONENTS Widgets Charts SerialBus SerialPort LinguistTools
)

set(TS_FILES
	ppm_ua.ts
	ppm_ru.ts
	ppm_de.ts
)

set_source_files_properties(${TS_FILES}
	PROPERTIES OUTPUT_LOCATION
	"${CMAKE_CURRENT_SOURCE_DIR}/i18n"
)

set(QRC_FILES
	resources.qrc
	translations.qrc
)

set(OTHER_FILES
	README.md
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
	${APP_WINDOWS_RESOURCE}
)

if(WIN32)
	list(APPEND PROJECT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
	qt_add_binary_resources(resources ${QRC_FILES})
	qt_add_resources(PROJECT_SOURCES ${QRC_FILES})
	qt_add_executable(ppm-generator MANUAL_FINALIZATION ${PROJECT_SOURCES})
	qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
	qt5_add_binary_resources(resources ${QRC_FILES})
	qt5_add_resources(PROJECT_SOURCES ${QRC_FILES})
	if(ANDROID)
		add_library(ppm-generator SHARED ${PROJECT_SOURCES})
	else()
		add_executable(ppm-generator ${PROJECT_SOURCES})
	endif()
	qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

add_dependencies(ppm-generator resources)

target_link_libraries(ppm-generator PRIVATE
	Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::Charts
	Qt${QT_VERSION_MAJOR}::SerialBus
	Qt${QT_VERSION_MAJOR}::SerialPort
)

set_target_properties(ppm-generator PROPERTIES
	MACOSX_BUNDLE_GUI_IDENTIFIER oleksandr.kolodkin.com
	MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
	MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
	qt_finalize_executable(ppm-generator)
endif()
