#ifndef DEVICE_H
#define DEVICE_H

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


#include <QObject>
#include <QIODevice>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QModbusClient>
#include <QModbusDataUnit>
#include <QModbusDevice>
#include <QVariant>

class Device : public QObject
{
	Q_OBJECT
public:
	explicit Device(QObject *parent = nullptr);

public slots:
	void setModbusClient(QModbusClient *client);
	void start() {mRun = true, update();}
	void stop() {mRun = false, update();}
	void setChannelsCount(int count);
	void setChanelValue(int chanel, double value);
	virtual bool setProperty(int id, QVariant value) {
		Q_UNUSED(id);
		Q_UNUSED(value);
		return false;
	}

public:
	bool isDeviceConnected()    const {return mClient ? mClient->state() == QModbusDevice::ConnectedState   : false;}
	bool isDeviceDisconnected() const {return mClient ? mClient->state() == QModbusDevice::UnconnectedState : true;}
	bool isRunning()            const {return mRun;}
	virtual QVariant property(int) {return QVariant();}

signals:
	void deviceConnected();
	void deviceDisconnected();
	void deviceConnectionFailed();
	void started();
	void stopped();
	void updated();
	void propertyChanged(int id, QVariant value);

protected:
	QModbusClient *mClient;

	bool mRun;                // Enabling generation
	bool mRuning;             // Is generation enabled
	QVector<double> mChannel; // in % of (mMaximum - mMinimum)
	const int mAddress = 1;

	virtual void update() {;}
	virtual void connectionFailed() {;}
};

#endif // DEVICE_H
