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

#ifndef PPM_H
#define PPM_H

#include <QObject>
#include <QIODevice>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QModbusClient>
#include <QModbusDataUnit>
#include <QModbusDevice>

#include "device.h"

#define REG_QUANT              0           // 1 μs in system clock cycles
#define REG_MAX_COUNT          1           // Maximum number of channels
#define REG_STATE              2           // Status register
#define REG_COUNT              3           // Number of channels
#define REG_PAUSE              4           // Pause
#define REG_SYNC_LO            5           // Synchronization pulse
#define REG_SYNC_HI            6           //

#define DEVICE_PROPERTY_INVERSION    1
#define DEVICE_PROPERTY_PERIOD       2
#define DEVICE_PROPERTY_PAUSE        3
#define DEVICE_PROPERTY_MIN          4
#define DEVICE_PROPERTY_MAX          5
#define DEVICE_PROPERTY_MAX_PULSE    6
#define DEVICE_PROPERTY_TOO_SMALL    7


class ppm : public Device
{
	Q_OBJECT

public:
	explicit ppm(QObject *parent = nullptr);
	QVariant property(int id) override;

public slots:
	bool setProperty(int id, QVariant value) override;

private:
	void read(int address, quint16 size);
	void write(QString command);
	void write(int command);
	void write(int command, int value);
	void write(int command, int channel, int value);
	void readQuant();
	void readState();
	void update() override;
	double maxPulseLength();

	void connectionFailed() override {mQuant = 0;}

private:
	bool mInversion;          // PPM signal inversion
	int mQuant;               // in ms
	double mMinimum;          // in ms
	double mMaximum;          // in ms
	double mPause;            // in ms
	double mPeriod;           // in ms
};

#endif // PPM_H
