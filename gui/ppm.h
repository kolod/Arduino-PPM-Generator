//    Arduino PPM Generator
//    Copyright (C) 2015-2016  Alexandr Kolodkin <alexandr.kolodkin@gmail.com>
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

#define REG_QUANT              0           // 1 мксек в тактах системной частоты
#define REG_MAX_COUNT          1           // Максимальное количество каналов
#define REG_STATE              2           // Регистр состояния
#define REG_COUNT              3           // Количество каналов
#define REG_PAUSE              4           // Пауза
#define REG_SYNC_LO            5           // Импульс синхронизации
#define REG_SYNC_HI            6           //

class ppm : public QObject
{
	Q_OBJECT

public:
	explicit ppm(QObject *parent = 0);

public slots:
	void setModbusClient(QModbusClient *client);

	void start()                    {mRun       = true;    update();}
	void stop()                     {mRun       = false;   update();}
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

private:
	void read(int address, quint16 size);
	void write(QString command);
	void write(int command);
	void write(int command, int value);
	void write(int command, int channel, int value);
	void readQuant();
	void update();

signals:
	void deviceConnected();
	void deviceDisconnected();
	void deviceConnectionFailed();
	void started();
	void stoped();
	void updated();
	void sync2small();
	void maxPulseLengthChanged(double value);

private:
	QModbusClient *mClient;

	int mAddress;
	bool mRun;                // Включение генерации
	bool mRuning;             //
	bool mInversion;          // Инверсия PPM сигнала
	int mQuant;               // в мсек
	double mMinimum;          // в мсек
	double mMaximum;          // в мсек
	double mPause;            // в мсек
	double mPeriod;           // в мсек
	QVector<double> mChannel; // в % от (mMaximum - mMinimum)
};

#endif // PPM_H
