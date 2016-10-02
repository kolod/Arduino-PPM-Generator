﻿//    Arduino PPM Generator
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
			emit deviceConnected();
			update();
		} else {
			emit deviceDisconnected();
		}
	});
}

// Передать новые параметры в устройство, если соблюдены все условия
void ppm::update()
{
	qDebug() << "update";

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

			qDebug() << "good";

			auto request = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, REG_STATE, 5 + mChannel.count());
			auto sync = quint32(mPeriod * mQuant);
			auto minSync = quint32(mMaximum * mQuant + 1);

			qDebug() << mQuant << mChannel << mPause;

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
			request.setValue(1, mChannel.count());
			request.setValue(2, uint16_t(mPause * mQuant));
			request.setValue(3, quint16(sync & 0x0000FFFF));
			request.setValue(4, quint16(sync >> 16 & 0x0000FFFF));

			qDebug() << request.values();

			auto *reply = mClient->sendWriteRequest(request, mAddress);
			if (reply) {
				if (reply->isFinished()) {
					reply->deleteLater();
				} else {
					connect(reply, &QModbusReply::finished, this, [this, reply] {
						if (reply->error() == QModbusDevice::NoError) {
							emit updated();
							if (mRun != mRuning) {
								if (mRun) emit started(); else emit stoped();
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
	qDebug() << "readQuant";

	auto request = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, REG_QUANT, 1);
	auto *reply = mClient->sendReadRequest(request, mAddress);
	if (reply) {
		if (reply->isFinished()) {
			reply->deleteLater();
		} else {
			connect(reply, &QModbusReply::finished, this, [this, reply] {
				qDebug() << "onReadQuant" << reply->error() << reply->errorString();

				if (reply->error() == QModbusDevice::NoError) {
					auto result = reply->result();
					if (result.valueCount() > 0) {
						mQuant = result.value(0) * 1000;
						emit maxPulseLengthChanged(maxPulseLength());
						qDebug() << mQuant;
						update();
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
	if (mQuant > 0.0) return floor((double) 0xFFFF / mQuant);
	return 100000.0;
}
