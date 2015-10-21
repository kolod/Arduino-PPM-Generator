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

#ifndef PPM_H
#define PPM_H

#include <QObject>
#include <QIODevice>
#include <QVector>
#include <QString>
#include <QStringList>

class ppm : public QObject
{
	Q_OBJECT

public:
	explicit ppm(QObject *parent = 0);

public slots:
	void updateFrequency();
	void updateAll();
	void updateSync() ;
	void setPort(QIODevice *port);
	void setChannelsCount(int count);
	void setPeriod(double period);
	void setPause(double pause);
	void setMinimum(double minimum);
	void setMaximum(double maximum);
	void setChanelValue(int chanel, double value);

	void start();
	void stop();

private:
	bool isReady();
	void read(int command);
	void write(QString command);
	void write(int command);
	void write(int command, int value);
	void write(int command, int channel, int value);

private slots:
	void onReadyRead();

private:
	QIODevice *p;             //
	double mQuant;            // в мсек (
	double mMinimum;          // в мсек
	double mMaximum;          // в мсек
	double mPause;            // в мсек
	double mPeriod;           // в мсек
	QVector<double> mChannel; // в % от (mMaximum - mMinimum)
};

#endif // PPM_H
