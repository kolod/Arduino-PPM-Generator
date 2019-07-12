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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//using namespace QtCharts;

#include <QMainWindow>
#include <QSettings>
#include <QString>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QCloseEvent>
#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include <QSlider>
#include <QStringList>
#include <QPen>
#include <QComboBox>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QtSerialBus>
#include <QIntValidator>
#include <QColor>
#include <QPalette>
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QChart>
#include <QMessageBox>
#include <QFile>
#include <QStatusBar>

#include "ppm.h"
#include "loader.h"

typedef struct {
	QLabel         *label;
	QSlider        *slider;
	QDoubleSpinBox *spinBox;
	QPushButton    *bind;
} TChannelWidgets;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void saveSession();
	void restoreSession();

private slots:
	void enumeratePorts();
	void enumerateBaudRates();
	void updateSyncPulseValue();
	void drawPlot();
	void setupChannelsUi(int count);
	void check();
	void xAxisUpdate();
	void uploadFirmware();

private:
	void setupUi();
	void retranslateUi();
	void closeEvent(QCloseEvent *event);
	QPalette gradient(double value, double max);

	bool isStarted;
	bool isFirmwareUploadingRequested;

	QModbusClient  *mClient;
	ppm            devise;
	Loader         loader;

	QWidget        *centralWidget;
	QGridLayout    *gridLayout;

	QLabel         *labelPort;
	QLabel         *labelSpeed;
	QLabel         *labelChannelsCount;
	QLabel         *labelPeriod;
	QLabel         *labelPause;
	QLabel         *labelMinimum;
	QLabel         *labelMaximum;
	QLabel         *labelSyncPulse;

	QPushButton    *inputUpdatePorts;
	QPushButton    *inputConnect;
	QPushButton    *inputStartStop;

	QComboBox      *inputPort;
	QComboBox      *inputSpeed;

	QSpinBox       *inputChannelsCount;

	QDoubleSpinBox *inputPeriod;
	QDoubleSpinBox *inputPause;
	QDoubleSpinBox *inputMinimum;
	QDoubleSpinBox *inputMaximum;
	QDoubleSpinBox *outputSyncPulse;

	QCheckBox      *inputInversion;

	QVector<TChannelWidgets*> channels;

	QtCharts::QChartView *chartView;
	QtCharts::QLineSeries *line;
	QtCharts::QValueAxis *xAxis;
	QtCharts::QValueAxis *yAxis;

};

#endif // MAINWINDOW_H
