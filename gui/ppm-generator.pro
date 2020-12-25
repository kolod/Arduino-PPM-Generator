#    Arduino PPM Generator
#    Copyright (C) 2015-2020  Alexandr Kolodkin <alexandr.kolodkin@gmail.com>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

QT       += charts core gui widgets printsupport serialport serialbus
TARGET    = ppm-generator
TEMPLATE  = app

DEFINES  += QT_DEPRECATED_WARNINGS
DEFINES  += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    ppm.cpp \
    loader.cpp

HEADERS  += \
    mainwindow.h \
    ppm.h \
    loader.h

TRANSLATIONS = \
    ppm_ru.ts \
    ppm_de.ts

# Версия программы
VERSION  = 1.0

# Добавление информации о файле для win32 приложения
QMAKE_TARGET_COMPANY     = Alexandr Kolodkin
QMAKE_TARGET_PRODUCT     = The PPM Generator
QMAKE_TARGET_DESCRIPTION = The Arduino based Pulse Position Modulation Generator
QMAKE_TARGET_COPYRIGHT   = Copyright © 2015-2020 Alexandr Kolodkin

# Иконка
win32: RC_ICONS = app.ico

DISTFILES += \
    app.ico

RESOURCES += \
    resources.qrc
