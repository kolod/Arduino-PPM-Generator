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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QString>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCloseEvent>
#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include <QSlider>
#include <QStringList>
#include <QPen>
#include <QComboBox>
#include <QPushButton>
#include <QSignalMapper>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QIntValidator>

#include "qdoubleslider.h"
#include "qcustomplot.h"
#include "ppm.h"

typedef struct {
	QLabel         *label;
	QDoubleSlider  *slider;
	QDoubleSpinBox *spinBox;
} TChannelWidgets;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	void saveSession();
	void restoreSession();

private slots:
	void setupUi();
	void setupChannelsUi(int count);
	void retranslateUi();
	void enumeratePorts();
	void enumerateBaudRates();
	void updateSyncPulseValue();
	void connectDisconnect();
	void onChanelValueChanged(int chanel);
	void drawPlot();

private:
	QLabel         *labelPort;
	QComboBox      *inputPort;
	QPushButton    *inputUpdatePorts;
	QLabel         *labelSpeed;
	QComboBox      *inputSpeed;
	QPushButton    *inputConnect;
	QGridLayout    *gridLayout;
	QWidget        *centralWidget;
	QLabel         *labelChannelsCount;
	QSpinBox       *inputChannelsCount;
	QLabel         *labelPeriod;
	QDoubleSpinBox *inputPeriod;
	QLabel         *labelPause;
	QDoubleSpinBox *inputPause;
	QLabel         *labelMinimum;
	QDoubleSpinBox *inputMinimum;
	QLabel         *labelMaximum;
	QDoubleSpinBox *inputMaximum;
	QCustomPlot    *plot;
	QCPCurve       *curve;
	QLabel         *labelSyncPulse;
	QDoubleSpinBox *outputSyncPulse;

	QSerialPort   port;
	ppm           devise;
	QSignalMapper mapper;
	QVector<TChannelWidgets*> channels;

	void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
