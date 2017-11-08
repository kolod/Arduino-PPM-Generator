//    Arduino PPM Generator
//    Copyright (C) 2015-2017  Alexandr Kolodkin <alexandr.kolodkin@gmail.com>
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLoggingCategory>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setApplicationName("Arduino PPM Generator");
	a.setOrganizationName("Alexandr Kolodkin");
	a.setStyle(QStyleFactory::create("Fusion"));

//	QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));

	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	a.installTranslator(&qtTranslator);

	QTranslator myTranslator;
	myTranslator.load(":/translations/ppm_ru.qm");
	a.installTranslator(&myTranslator);

	MainWindow w;
	w.show();
	w.restoreSession();

	return a.exec();
}
