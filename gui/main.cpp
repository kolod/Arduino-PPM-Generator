//    Arduino PPM Generator
//    Copyright (C) 2015-2020  Alexandr Kolodkin <alexandr.kolodkin@gmail.com>
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


QString qtTranslationsPath() {
#if QT_VERSION_MAJOR >= 6
	return QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
	return QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setApplicationName("Arduino PPM Generator");
	a.setOrganizationName("Alexandr Kolodkin");
	a.setApplicationVersion("1.0.1");
	a.setStyle(QStyleFactory::create("Fusion"));

//	QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));

	QTranslator qtTranslator;
	if (qtTranslator.load(
		QLocale(),
		QLatin1String("qt"),
		QLatin1String("_"),
		qtTranslationsPath()
	)) {
		a.installTranslator(&qtTranslator);
		qDebug() << "Qt translator installed.";
	} else {
		qDebug() << "Qt translator not found.";
	}

	QTranslator myTranslator;
	if (myTranslator.load(
		QLocale(),
		QLatin1String("ppm"),
		QLatin1String("_"),
		QLatin1String(":/i18n/")
	)) {
		a.installTranslator(&myTranslator);
		qDebug() << "Application translator installed.";
	} else {
		qDebug() << "Application translator not found.";
	}

	MainWindow w;
	w.show();
	w.restoreSession();
	return a.exec();
}
