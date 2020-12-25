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

#define REG_QUANT              0           // 1 μs in system clock cycles
#define REG_MAX_COUNT          1           // Maximum number of channels
#define REG_STATE              2           // Status register
#define REG_COUNT              3           // Number of channels
#define REG_PAUSE              4           // Pause
#define REG_SYNC_LO            5           // Synchronization pulse
#define REG_SYNC_HI            6           //

class ppm : public QObject
{
	Q_OBJECT

public:
	explicit ppm(QObject *parent = nullptr);

public slots:
	void setModbusClient(QModbusClient *client);

	void start()                    {mRun       = true;              update();}
	void stop()                     {mRun       = false;             update();}
	void setInversion(bool value)   {mInversion = value;   if (mRun) update();}
	void setPeriod(double period)   {mPeriod    = period;  if (mRun) update();}
	void setPause(double pause)     {mPause     = pause;   if (mRun) update();}
	void setMinimum(double minimum) {mMinimum   = minimum; if (mRun) update();}
	void setMaximum(double maximum) {mMaximum   = maximum; if (mRun) update();}

	void setChannelsCount(int count);
	void setChanelValue(int chanel, double value);

	double maxPulseLength();

	bool isDeviceConnected()    const {return mClient ? mClient->state() == QModbusDevice::ConnectedState   : false;}
	bool isDeviceDisconnected() const {return mClient ? mClient->state() == QModbusDevice::UnconnectedState : true;}
	bool isRunning()            const {return mRun;}
	bool isInverted()           const {return mInversion;}

private:
	void read(int address, quint16 size);
	void write(QString command);
	void write(int command);
	void write(int command, int value);
	void write(int command, int channel, int value);
	void readQuant();
	void readState();
	void update();

signals:
	void deviceConnected();
	void deviceDisconnected();
	void deviceConnectionFailed();
	void started();
	void stopped();
	void updated();
	void sync2small();
	void maxPulseLengthChanged(double value);
	void inversion(bool state);

private:
	QModbusClient *mClient;

	int mAddress;
	bool mRun;                // Enabling generation
	bool mRuning;             //
	bool mInversion;          // PPM signal inversion
	int mQuant;               // in ms
	double mMinimum;          // in ms
	double mMaximum;          // in ms
	double mPause;            // in ms
	double mPeriod;           // in ms
	QVector<double> mChannel; // in % of (mMaximum - mMinimum)
};

#endif // PPM_H
