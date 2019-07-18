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


#include <QtMath>
#include <QDebug>
#include "loader.h"

const quint8 sync[]          = {STK_GET_SYNC, CRC_EOP};
const quint8 hardware[]      = {STK_GET_PARAMETER, STK_PARAM_HARDWARE_VERSION, CRC_EOP};
const quint8 softwareMajor[] = {STK_GET_PARAMETER, STK_PARAM_SOFTWARE_VERSION_MAJOR, CRC_EOP};
const quint8 softwareMinor[] = {STK_GET_PARAMETER, STK_PARAM_SOFTWARE_VERSION_MINOR, CRC_EOP};
const quint8 enterProg[]     = {STK_ENTER_PROGMODE, CRC_EOP};
const quint8 getSignature[]  = {STK_READ_SIGN, CRC_EOP};
const quint8 leaveProg[]     = {STK_LEAVE_PROGMODE, CRC_EOP};

Loader::Loader(QObject *parent) : QObject(parent)
{
	connect(&mPort, SIGNAL(readyRead()), this, SLOT(worker()));
}

bool Loader::openPort()
{
	bool result = mPort.open(QIODevice::ReadWrite);

	if (result) {
		mPort.setBaudRate(QSerialPort::Baud57600);
		mPort.setFlowControl(QSerialPort::NoFlowControl);
		mPort.setDataBits(QSerialPort::Data8);
	}

	return result;
}

void Loader::uploadFirmware(QByteArray data)
{
	if (openPort()) {
		mFirmware         = data;
		mPageSize         = 0;
		mExpectedLength   = 0;
		mState            = LoaderState::Reset;
		mAction           = LoaderAction::Upload;
		worker();
	}
}

void Loader::worker()
{
	// If incoming message expected
	if (mExpectedLength) {
		mInput.append(mPort.readAll());
		if (mInput.count() < mExpectedLength) return;
		if (!mInput.startsWith(STK_INSYNC) || !mInput.endsWith(STK_OK)) clear();
	}

//	qDebug() << (int) mState << mInput.count();

	switch (mState) {
	case LoaderState::None:
		break;

	case LoaderState::Reset:
		emit stateChanged(tr("Resetting the Arduino"));
		mState = LoaderState::Sync;
		mPort.setDataTerminalReady(true);
		mPort.setRequestToSend(true);
		QTimer::singleShot(100, this, SLOT(worker()));
		break;

	case LoaderState::Sync:
		emit stateChanged(tr("Retrieving the bootloader parameters"));
		mInput.clear();
		mState = LoaderState::GetHardwareVersion;
		mExpectedLength = 2;
		mPort.setDataTerminalReady(false);
		mPort.setRequestToSend(false);
		mPort.write(reinterpret_cast<const char*>(sync), sizeof(sync));
		QTimer::singleShot(1000, this, SLOT(clear()));
		break;

	case LoaderState::GetHardwareVersion:
		if (!mInput.startsWith(STK_INSYNC) || !mInput.endsWith(STK_OK)) clear();

		mInput.clear();
		mExpectedLength = 3;
		mState = LoaderState::GetSoftwareVersionMajor;
		mPort.write(reinterpret_cast<const char*>(hardware), sizeof(hardware));
		QTimer::singleShot(100, this, SLOT(clear()));
		break;

	case LoaderState::GetSoftwareVersionMajor:
		if (!mInput.startsWith(STK_INSYNC) || !mInput.endsWith(STK_OK)) clear();

		mInput.clear();
		mState = LoaderState::GetSoftwareVersionMinor;
		mPort.write(reinterpret_cast<const char*>(softwareMajor), sizeof(softwareMajor));
		QTimer::singleShot(100, this, SLOT(clear()));
		break;

	case LoaderState::GetSoftwareVersionMinor:
		if (!mInput.startsWith(STK_INSYNC) || !mInput.endsWith(STK_OK)) clear();

		mInput.clear();
		mState = LoaderState::EnterProgramming;
		mPort.write(reinterpret_cast<const char*>(softwareMinor), sizeof(softwareMinor));
		QTimer::singleShot(100, this, SLOT(clear()));
		break;

	case LoaderState::EnterProgramming:
		if (!mInput.startsWith(STK_INSYNC) || !mInput.endsWith(STK_OK)) clear();

		mInput.clear();
		mExpectedLength = 2;
		mState = LoaderState::GetDeviceSignature;
		mPort.write(reinterpret_cast<const char*>(enterProg), sizeof(enterProg));
		QTimer::singleShot(100, this, SLOT(clear()));
		break;

	case LoaderState::GetDeviceSignature:
		if (!mInput.startsWith(STK_INSYNC) || !mInput.endsWith(STK_OK)) clear();

		mInput.clear();
		mExpectedLength = 5;
		mState = LoaderState::BeginLoop;
		mPort.write(reinterpret_cast<const char*>(getSignature), sizeof(getSignature));
		QTimer::singleShot(100, this, SLOT(clear()));
		break;

	case LoaderState::BeginLoop:
		if (mAction == LoaderAction::Upload) {
			emit stateChanged(tr("Uploading firmware"));
			if (!mInput.startsWith(STK_INSYNC) || !mInput.endsWith(STK_OK)) clear();
			mPageSize = getPageSize(static_cast<quint8>(mInput[2]), static_cast<quint8>(mInput[3]));
			if (mPageSize == 0) clear();
		} else {
			emit stateChanged(tr("Validating firmware"));
		}

		mAddress = 0;
		mExpectedLength = 0;
		mState = LoaderState::SetAddress;
		worker();
		break;

	case LoaderState::SetAddress:

		if (mAction == LoaderAction::Upload) {
			mState = LoaderState::WritePage;
		} else {
			mState = LoaderState::ReadPage;
		}

		mInput.clear();
		mOutput.clear();
		mExpectedLength = 2;
		mOutput.append(STK_LOAD_ADDRESS);
		mOutput.append(static_cast<char>((mAddress >> 1) & 0xFF)); // low
		mOutput.append(static_cast<char>((mAddress >> 9) & 0xFF)); // high
		mOutput.append(CRC_EOP);
		mPort.write(mOutput);
		QTimer::singleShot(100, this, SLOT(clear()));
		break;

	case LoaderState::WritePage:
		if (!mInput.startsWith(STK_INSYNC) || !mInput.endsWith(STK_OK)) clear();

		mState = LoaderState::EndLoop;
		mExpectedLength = 2;

		mInput.clear();
		mOutput.clear();
		mOutput.append(STK_PROG_PAGE);
		mOutput.append(static_cast<char>((mPageSize >> 8) & 0xFF)); // high
		mOutput.append(static_cast<char>(mPageSize & 0xFF));        // low
		mOutput.append('F');

		for (int i = mAddress; i < mAddress + mPageSize; i++) {
			mOutput.append(i >= mFirmware.count() ? '\xFF' : mFirmware[i]);
		}

		mOutput.append(CRC_EOP);
		mPort.write(mOutput);
		QTimer::singleShot(500, this, SLOT(clear()));
		break;

	case LoaderState::ReadPage:
		if (!mInput.startsWith(STK_INSYNC) || !mInput.endsWith(STK_OK)) clear();

		mState = LoaderState::EndLoop;
		mExpectedLength = mPageSize + 2;

		qDebug() << mExpectedLength;

		mInput.clear();
		mOutput.clear();
		mOutput.append(STK_READ_PAGE);
		mOutput.append(static_cast<char>((mPageSize >> 8) & 0xFF)); // high
		mOutput.append(static_cast<char>(mPageSize & 0xFF));        // low
		mOutput.append('F');
		mOutput.append(CRC_EOP);
		mPort.write(mOutput);
		QTimer::singleShot(500, this, SLOT(clear()));
		break;

	case LoaderState::EndLoop:
		if (!mInput.startsWith(STK_INSYNC) || !mInput.endsWith(STK_OK)) clear();

		if (mAction == LoaderAction::Validate) {
			for (int j = 1, i = mAddress; i < mAddress + mPageSize; i++, j++) {
				if (i < mFirmware.count()) {
					if (mInput[j] != mFirmware[i]) clear();
				} else {
					if (mInput[j] != '\xFF') clear();
				}
			}
		}

		mExpectedLength = 0;
		mInput.clear();

		mAddress += mPageSize;
		if (mAddress < mFirmware.count()) {
			mState = LoaderState::SetAddress;
		} else {
			if (mAction == LoaderAction::Upload) {
				mAction = LoaderAction::Validate;
				mState  = LoaderState::BeginLoop;
			} else {
				mAction = LoaderAction::None;
				mState  = LoaderState::LeaveProgramming;
			}
		}

		worker();
		break;

	case LoaderState::LeaveProgramming:
		mExpectedLength = 2;
		mState = LoaderState::Wait;
		mPort.write(reinterpret_cast<const char*>(leaveProg), sizeof(leaveProg));
		QTimer::singleShot(100, this, SLOT(clear()));
		break;

	case LoaderState::Wait:
		if (!mInput.startsWith(STK_INSYNC) || !mInput.endsWith(STK_OK)) clear();

		emit stateChanged(tr("Wait device ready"));
		mState = LoaderState::EmitUploadFinished;
		mPort.close();
		QTimer::singleShot(2000, this, SLOT(worker()));
		break;

	case LoaderState::EmitUploadFinished:
		clear(true);
	}
}

void Loader::clear(bool result)
{
	emit uploadFinished(result);
	if ((result == false) && (mExpectedLength > 0)) emit stateChanged(tr("Error: Timeout"));

	mInput.clear();
	mOutput.clear();
	mFirmware.clear();
	mExpectedLength = 0;
	mState = LoaderState::None;
	mAction = LoaderAction::None;
}

quint16 Loader::getPageSize(unsigned char sig2, unsigned char sig3) const
{
	typedef struct {
		quint8 sig2;
		quint8 sig3;
		quint16 pageLength;
	} ChipSignatures;

	const QList<ChipSignatures> signatures = {
		{0x97, 0x02, 0x80*2},	// ATmega128  - 128 words
		{0x97, 0x03, 0x80*2},	// ATmega1280 - 128 words
		{0x97, 0x04, 0x80*2},	// ATmega1281 - 128 words
		{0x96, 0x02, 0x80*2},	// ATmega64   - 128 words
		{0x95, 0x02, 0x40*2},	// ATmega32   -  64 words
		{0x95, 0x0F, 0x40*2},	// ATmega328P -  64 words
		{0x95, 0x14, 0x40*2},	// ATmega328  -  64 words
		{0x94, 0x02, 0x40*2},	// ATmega163  -  64 words
		{0x94, 0x03, 0x40*2},	// ATmega16   -  64 words
		{0x94, 0x04, 0x40*2},	// ATmega162  -  64 words
		{0x94, 0x05, 0x40*2},	// ATmega169  -  64 words
		{0x94, 0x06, 0x40*2},	// ATmega168  -  64 words
		{0x93, 0x06, 0x20*2},	// ATmega8515 -  32 words
		{0x93, 0x07, 0x20*2},	// ATmega8    -  32 words
		{0x93, 0x08, 0x20*2},	// ATmega8535 -  32 words
		{0x93, 0x0a, 0x20*2} 	// ATmega88   -  32 words
	};

	foreach (ChipSignatures chip, signatures) {
		if ((chip.sig2 == sig2) && (chip.sig3 == sig3)) {
			return chip.pageLength;
		}
	}

	return 0;
}
