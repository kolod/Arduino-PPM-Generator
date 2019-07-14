//    Arduino PPM Generator
//    Copyright (C) 2015-2019  Alexandr Kolodkin <alexandr.kolodkin@gmail.com>
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
#include <math.h>
#include <QDebug>
#include "ppm.h"

ppm::ppm(QObject *parent)
	: QObject(parent)
	, mClient(nullptr)
	, mAddress(1)
	, mRun(false)
	, mRuning(false)
	, mInversion(false)
	, mQuant(0)
	, mMinimum(0.0)
	, mMaximum(0.0)
	, mPause(0.0)
	, mPeriod(0.0)
{}

void ppm::setModbusClient(QModbusClient *client)
{
	mClient = client;
	connect(mClient, &QModbusClient::stateChanged, this, [this] {
		if (mClient->state() == QModbusDevice::ConnectedState) {
			update();
			emit deviceConnected();
		} else {
			mQuant = 0;
			emit deviceDisconnected();
		}
	});
}

// Передать новые параметры в устройство, если соблюдены все условия
void ppm::update()
{
	if (
		(mClient != nullptr) &&
		(mClient->state() == QModbusDevice::ConnectedState) &&
		(mChannel.count() > 0) &&
		(mPause > 0) &&
		(mPause < mMinimum) &&
		(mMinimum > 0) &&
		(mMinimum < mMaximum)
	) {
		if (mQuant > 0) {

			auto request = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, REG_STATE, 5 + static_cast<quint16>(mChannel.count()));
			auto sync    = static_cast<quint32>(mPeriod * mQuant);
			auto minSync = static_cast<quint32>(mMaximum * mQuant + 1);

			int index = 5;
			for (int i = 0; i < mChannel.count(); i++) {
				auto channel = uint16_t((mMinimum + mChannel[i] * (mMaximum - mMinimum) / 100) * mQuant);
				sync -= uint32_t(channel);
				request.setValue(index++, channel);
				if (sync < minSync) {
					qDebug() << "sync2small" << sync << minSync;
					emit sync2small();
					return;
				}
			}

			request.setValue(0, mRun ? mInversion ? 2 : 1 : 0);
			request.setValue(1, static_cast<quint16>(mChannel.count()));
			request.setValue(2, uint16_t(mPause * mQuant));
			request.setValue(3, quint16(sync       & 0x0000FFFF));
			request.setValue(4, quint16(sync >> 16 & 0x0000FFFF));

			auto *reply = mClient->sendWriteRequest(request, mAddress);
			if (reply) {
				if (reply->isFinished()) {
					reply->deleteLater();
				} else {
					connect(reply, &QModbusReply::finished, this, [this, reply] {
						if (reply->error() == QModbusDevice::NoError) {
							emit updated();
							if (mRun != mRuning) {
								if (mRun) emit started(); else emit stopped();
								mRuning = mRun;
							}
						}
						reply->deleteLater();
					});
				}
			} else {
				delete reply;
			}
		} else {
			readQuant();
			readState();
		}
	}
}

// Установить количество каналов
void ppm::setChannelsCount(int count)
{
	int oldCount = mChannel.count();
	mChannel.resize(count);
	for (int i = oldCount; i < count; mChannel[i++] = 0.0);
	update();
}

// Установит значение канала в %
void ppm::setChanelValue(int chanel, double value)
{
	if ((chanel >= 0) && (chanel < mChannel.count())) {
		mChannel[chanel] = qBound(0.0, value, 100.0);
		update();
	}
}

void ppm::readQuant()
{
	auto request = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, REG_QUANT, 1);
	auto *reply = mClient->sendReadRequest(request, mAddress);
	if (reply) {
		if (reply->isFinished()) {
			reply->deleteLater();
		} else {
			connect(reply, &QModbusReply::finished, this, [this, reply] {
				if (reply->error() == QModbusDevice::NoError) {
					auto result = reply->result();
					if (result.valueCount() > 0) {
						auto quant = result.value(0);
						if (quant == 0) emit deviceConnectionFailed();
						mQuant = quant * 1000;
						emit maxPulseLengthChanged(maxPulseLength());
						update();
					} else {
						emit deviceConnectionFailed();
					}
				} else {
					emit deviceConnectionFailed();
				}
			});
		}
	} else {
		delete reply;
	}
}

void ppm::readState()
{
	auto request = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, REG_STATE, 1);
	auto *reply = mClient->sendReadRequest(request, mAddress);
	if (reply) {
		if (reply->isFinished()) {
			reply->deleteLater();
		} else {
			connect(reply, &QModbusReply::finished, this, [this, reply] {
				if (reply->error() == QModbusDevice::NoError) {
					auto result = reply->result();
					if (result.valueCount() > 0) {
						auto state = result.value(0);
						bool lastState = mRun;
						switch (state) {
						case 0:
							mRun = false;
							if (lastState != mRun) emit stopped();
							break;

						case 1:
							mRun = true;
							mInversion = false;
							if (lastState != mRun) emit started();
							emit inversion(false);
							break;

						case 2:
							mRun = true;
							mInversion = true;
							if (lastState != mRun) emit started();
							emit inversion(true);
						}
					}
				}
			});
		}
	} else {
		delete reply;
	}
}

double ppm::maxPulseLength()
{
	if (mQuant > 0.0) return floor(static_cast<double>(0xFFFF) / mQuant);
	return 100000.0;
}
