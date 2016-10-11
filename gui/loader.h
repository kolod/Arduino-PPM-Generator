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


#ifndef LOADER_H
#define LOADER_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QByteArray>

/* STK500 constants list, from AVRDUDE */
#define STK_OK              0x10
#define STK_FAILED          0x11
#define STK_UNKNOWN         0x12
#define STK_NODEVICE        0x13
#define STK_INSYNC          0x14
#define STK_NOSYNC          0x15
#define ADC_CHANNEL_ERROR   0x16
#define ADC_MEASURE_OK      0x17
#define PWM_CHANNEL_ERROR   0x18
#define PWM_ADJUST_OK       0x19
#define CRC_EOP             ' '
#define STK_GET_SYNC        '0'
#define STK_GET_SIGN_ON     '1'
#define STK_SET_PARAMETER   '@'
#define STK_GET_PARAMETER   'A'
#define STK_SET_DEVICE      'B'
#define STK_SET_DEVICE_EXT  'E'
#define STK_ENTER_PROGMODE  'P'
#define STK_LEAVE_PROGMODE  'Q'
#define STK_CHIP_ERASE      'R'
#define STK_CHECK_AUTOINC   'S'
#define STK_LOAD_ADDRESS    'U'
#define STK_UNIVERSAL       'V'
#define STK_PROG_FLASH      '`'
#define STK_PROG_DATA       'a'
#define STK_PROG_FUSE       'b'
#define STK_PROG_LOCK       'c'
#define STK_PROG_PAGE       'd'
#define STK_PROG_FUSE_EXT   'e'
#define STK_READ_FLASH      'p'
#define STK_READ_DATA       'q'
#define STK_READ_FUSE       'r'
#define STK_READ_LOCK       's'
#define STK_READ_PAGE       't'
#define STK_READ_SIGN       'u'
#define STK_READ_OSCCAL     'v'
#define STK_READ_FUSE_EXT   'w'
#define STK_READ_OSCCAL_EXT 'x'

#define STK_PARAM_HARDWARE_VERSION        0x80
#define STK_PARAM_SOFTWARE_VERSION_MAJOR  0x81
#define STK_PARAM_SOFTWARE_VERSION_MINOR  0x82

enum class LoaderState {
	None,                      //  0
	Reset,                     //  1
	Sync,                      //  2
	GetHardwareVersion,        //  3
	GetSoftwareVersionMajor,   //  4
	GetSoftwareVersionMinor,   //  5
	EnterProgramming,          //  6
	GetDeviceSignature,        //  7
	BeginLoop,                 //  8
	SetAddress,                //  9
	WritePage,                 // 10
	ReadPage,                  // 11
	EndLoop,                   // 12
	LeaveProgramming,          // 13
	Wait,                      // 14
	EmitUploadFinished         // 15
};

enum class LoaderAction {
	None,
	Upload,
	Validate
};

class Loader : public QObject
{
	Q_OBJECT

public:
	explicit Loader(QObject *parent = 0);

	void setPortName(QString name) {mPort.setPortName(name);}
	void uploadFirmware(QByteArray data);
	void validateFirmware(QByteArray data);

signals:
	void resetFinished();
	void uploadFinished();

private slots:
	void worker();

private:
	QByteArray mFirmware;
	QByteArray mInput;
	QByteArray mOutput;

	QSerialPort mPort;
	QTimer mTimer;

	LoaderState mState;
	LoaderAction mAction;

	int mAddress;
	int mExpectedLength;
	quint16 mPageSize;
	quint16 getPageSize(unsigned char sig2, unsigned char sig3) const;

	bool openPort();
};

#endif // LOADER_H
