//    Arduino PPM Generator
//    Copyright (C) 2015  Alexandr Kolodkin <alexandr.kolodkin@gmail.com>
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

#include <stdarg.h>
#include <QDebug>

#include "ppm.h"


enum {
	kStart,             // Запустить генерацию
	kStop,              // Остановить генерацию
	kSetChanelsCount,   // Установить количество каналов
	kSetPause,          // Установить длительности паузы           (в импульсах несущей частоты ШИМ или ×0.0625 мксек)
	kSetChanel,         // Установить длительности канала          (в импульсах несущей частоты ШИМ или ×0.0625 мксек)
	kSetSync,           // Установить длительность синхроимпульса  (в импульсах несущей частоты ШИМ или ×0.0625 мксек)
	kGetFreaquency,     // Запрос частоты ШИМ
	kError,
	kFreaquency
};

ppm::ppm(QObject *parent)
	: QObject(parent)
	, p(0)
	, mQuant(0.0)
	, mMinimum(0.0)
	, mMaximum(0.0)
	, mPause(0.0)
	, mPeriod(0.0)
{}

bool ppm::isReady()
{
	if (p == 0) return false;
	if (mChannel.count() == 0) return false;
	if (mQuant   == 0.0) return false;
	if (mMinimum == 0.0) return false;
	if (mMaximum == 0.0) return false;
	if (mPause   == 0.0) return false;
	if (mPeriod  == 0.0) return false;

	return true;
}

void ppm::updateSync()
{
	double sync = mPeriod;
	for (int i = 0; i < mChannel.count(); i++) {
		sync -= mMinimum + mChannel[i] * (mMaximum - mMinimum) / 100;
	}

	write(kSetSync, qRound(sync / mQuant));
}

void ppm::updateAll()
{
	if (!isReady()) return;

	for (int i = 0; i < mChannel.count(); i++) {
		write(kSetChanel, i, qRound((mMinimum + mChannel[i] * (mMaximum - mMinimum) / 100) / mQuant));
	}

	write(kSetPause, qRound(mPause / mQuant));
	write(kSetChanelsCount, mChannel.count());
	updateSync();
}

void ppm::setPort(QIODevice *port)
{
	p = port;
}

void ppm::setChannelsCount(int count)
{
	int oldCount = mChannel.count();
	mChannel.resize(count);

	if (oldCount < count) for (int i = oldCount; i < count; i++) {
		mChannel[i] = 0.0;
	}

	updateAll();
}

void ppm::setPeriod(double period)
{
	mPeriod = period;
	if (isReady()) updateSync();
}

void ppm::setPause(double pause)
{
	mPause = pause;
	write(kSetPause, qRound(pause / mQuant));
}

void ppm::setMinimum(double minimum)
{
	mMinimum = minimum;
	updateAll();
}

void ppm::setMaximum(double maximum)
{
	mMaximum = maximum;
	updateAll();
}

void ppm::setChanelValue(int chanel, double value)
{
	if (chanel < 0) return;
	if (chanel >= mChannel.count()) return;
	mChannel[chanel] = qBound(0.0, value, 100.0);

	if (isReady()) {
		write(kSetChanel, chanel, qRound((mMinimum + value * (mMaximum - mMinimum) / 100) / mQuant));
		updateSync();
	}
}

void ppm::start()
{
	write(kStart);
}

void ppm::stop()
{
	write(kStop);
}

//
void ppm::updateFrequency()
{
	if (p->isWritable()) {
		connect(p, SIGNAL(readyRead()), SLOT(onReadyRead()));
		write(kGetFreaquency);
	}
}

// Передача данных контроллеру
void ppm::write(QString command)
{
	qDebug() << command;

	if (p && p->isOpen() && p->isWritable()) {
		p->write(command.toLatin1());
	}
}

void ppm::write(int command)
{
	write(QString("%1;").arg(command));
}

void ppm::write(int command, int value)
{
	write(QString("%1,%2;").arg(command).arg(value));
}

void ppm::write(int command, int channel, int value)
{
	write(QString("%1,%2,%3;").arg(command).arg(channel).arg(value));
}

// Получение данных из контроллера
void ppm::read(int command)
{
	if (p && p->isOpen() && p->isWritable()) {

		QString str = QString("%1;").arg(command);

		qDebug() << str;
		p->write(str.toLatin1());
	}
}

void ppm::onReadyRead()
{
	QString str = QString::fromLatin1(p->readAll());
	QStringList list = str.remove(";").split(",");

	int command = list[0].toInt();

	switch (command) {
	case kError:
		qDebug() << list;
		break;

	case kFreaquency:
		mQuant = 1000.0 / list[1].toDouble();
		updateAll();
	}
}
