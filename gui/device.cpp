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


#include "device.h"


Device::Device(QObject *parent)
    : QObject(parent)
    , mClient(nullptr)
    , mRun(false)
    , mRuning(false)
{}

void Device::setModbusClient(QModbusClient *client)
{
	mClient = client;
	connect(mClient, &QModbusClient::stateChanged, this, [this] {
		if (mClient->state() == QModbusDevice::ConnectedState) {
			update();
			emit deviceConnected();
		} else {
			connectionFailed();
			emit deviceDisconnected();
		}
	});
}

// Set the number of channels
void Device::setChannelsCount(int count)
{
	int oldCount = mChannel.count();
	mChannel.resize(count);
	for (int i = oldCount; i < count; mChannel[i++] = 0.0);
	update();
}

// Set channel value in %
void Device::setChanelValue(int chanel, double value)
{
	if ((chanel >= 0) && (chanel < mChannel.count())) {
		mChannel[chanel] = qBound(0.0, value, 100.0);
		update();
	}
}
