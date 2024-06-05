//    Arduino PPM Generator
//    Copyright (C) 2015-...  Oleksandr Kolodkin <oleksandr.kolodkin@ukr.net>
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
#include <QtGlobal>

#ifndef NDEBUG
#include <QLocale>
#include <QDirIterator>
#endif

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setApplicationName("Arduino PPM Generator");
	a.setOrganizationName("Alexandr Kolodkin");
	a.setApplicationVersion("1.1.0");
	a.setStyle(QStyleFactory::create("Fusion"));

    #ifndef NDEBUG
    qDebug() << QLocale::system() << QLocale::system().bcp47Name();

    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) qDebug() << it.next();
    #endif

//	QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));

	QTranslator qtTranslator;
	if (qtTranslator.load(
		QLocale(),
		QLatin1String("qt"),
		QLatin1String("_"),
        QLibraryInfo::path(QLibraryInfo::TranslationsPath)
	)) {
		a.installTranslator(&qtTranslator);
		qDebug() << "Qt translator installed.";
	} else {
		qDebug() << "Qt translator not found.";
    }

    QStringList ApplicationTranslationPaths = {
        qApp->applicationDirPath() + "/translations/",
        qApp->applicationDirPath() + "/../share/ppm-generator/translations/"
    };

    QTranslator myTranslator;
    for (auto ApplicationTranslationPath : ApplicationTranslationPaths) {
        if (myTranslator.load(
            QLocale(),
            QLatin1String("ppm"),
            QLatin1String("_"),
            ApplicationTranslationPath
        )) {
            qDebug() << "Application translator installed from " << ApplicationTranslationPath << ".";
            a.installTranslator(&myTranslator);
            break;
        } {
            qDebug() << "Application translator not found in " << ApplicationTranslationPath << ".";
        }
    }

	MainWindow w;
	w.show();
	w.restoreSession();
	return a.exec();
}
