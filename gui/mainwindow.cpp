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

#include "mainwindow.h"
#include <math.h>
#include <limits>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	setupUi();
	retranslateUi();

	devise.setPort(&port);

	connect(inputChannelsCount, SIGNAL(valueChanged(int)),    &devise, SLOT(setChannelsCount(int)));
	connect(inputPeriod,        SIGNAL(valueChanged(double)), &devise, SLOT(setPeriod(double)));
	connect(inputPause,         SIGNAL(valueChanged(double)), &devise, SLOT(setPause(double)));
	connect(inputMinimum,       SIGNAL(valueChanged(double)), &devise, SLOT(setMinimum(double)));
	connect(inputMaximum,       SIGNAL(valueChanged(double)), &devise, SLOT(setMaximum(double)));

	connect(inputPeriod,        SIGNAL(valueChanged(double)), SLOT(drawPlot()));
	connect(inputPause,         SIGNAL(valueChanged(double)), SLOT(drawPlot()));
	connect(inputMinimum,       SIGNAL(valueChanged(double)), SLOT(drawPlot()));
	connect(inputMaximum,       SIGNAL(valueChanged(double)), SLOT(drawPlot()));

	connect(&mapper,            SIGNAL(mapped(int)),          SLOT(onChanelValueChanged(int)));
	connect(inputConnect,       SIGNAL(clicked()),            SLOT(connectDisconnect()));
	connect(inputUpdatePorts,   SIGNAL(clicked()),            SLOT(enumeratePorts()));
	connect(inputChannelsCount, SIGNAL(valueChanged(int)),    SLOT(setupChannelsUi(int)));
}

void MainWindow::retranslateUi()
{
	labelChannelsCount->setText(tr("Channels count:"));
	labelPeriod->setText(tr("Period, ms:"));
	labelPause->setText(tr("Pause, ms:"));
	labelMinimum->setText(tr("Minimum, ms:"));
	labelMaximum->setText(tr("Maximum, ms:"));
	labelPort->setText(tr("Serial port:"));
	inputUpdatePorts->setText(tr("Update"));
	labelSpeed->setText(tr("Baud rate, Bd:"));
	inputConnect->setText(port.isOpen() ? tr("Disconnect") : tr("Connect"));
	labelSyncPulse->setText(tr("Sync period, ms"));
}

void MainWindow::setupUi()
{
	centralWidget     = new QWidget(this);

	gridLayout        = new QGridLayout(centralWidget);
	gridLayout->setMargin(5);

	// Порт
	labelPort         = new QLabel(centralWidget);
	inputPort         = new QComboBox(centralWidget);
	inputUpdatePorts  = new QPushButton(centralWidget);
	enumeratePorts();

	// Скорость
	labelSpeed        = new QLabel(centralWidget);
	inputSpeed        = new QComboBox(centralWidget);
	inputConnect      = new QPushButton(centralWidget);
	inputSpeed->setEditable(true);
	inputSpeed->setValidator(new QIntValidator(1, 24000000, this));
	enumerateBaudRates();

	// Ввод количества каналов
	labelChannelsCount = new QLabel(centralWidget);
	inputChannelsCount = new QSpinBox(centralWidget);
	inputChannelsCount->setMinimum(1);
	inputChannelsCount->setMaximum(16);
	inputChannelsCount->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	// Ввод длительности периода (мсек)
	labelPeriod       = new QLabel(centralWidget);
	inputPeriod       = new QDoubleSpinBox(centralWidget);
	inputPeriod->setMinimum(0.0);
	inputPeriod->setMaximum(100.0);
	inputPeriod->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	// Ввод длительности паузы (мсек)
	labelPause        = new QLabel(centralWidget);
	inputPause        = new QDoubleSpinBox(centralWidget);
	inputPause->setMinimum(0.0);
	inputPause->setMaximum(10.0);
	inputPause->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	// Ввод минимальной длительности (мсек)
	labelMinimum      = new QLabel(centralWidget);
	inputMinimum      = new QDoubleSpinBox(centralWidget);
	inputMinimum->setMinimum(0.0);
	inputMinimum->setMaximum(10.0);
	inputMinimum->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	// Ввод максимальной длительности (мсек)
	labelMaximum      = new QLabel(centralWidget);
	inputMaximum      = new QDoubleSpinBox(centralWidget);
	inputMaximum->setMinimum(0.0);
	inputMaximum->setMaximum(10.0);
	inputMaximum->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	// График
	plot              = new QCustomPlot(centralWidget);

	// Вывод длительности синхроимпульса (мксек)
	labelSyncPulse    = new QLabel(centralWidget);
	outputSyncPulse   = new QDoubleSpinBox(centralWidget);
	outputSyncPulse->setReadOnly(true);
	outputSyncPulse->setMinimum(0.0);
	outputSyncPulse->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	QPen pen;
	pen.setColor(QColor(Qt::blue));
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(1);

	curve = new QCPCurve(plot->xAxis, plot->yAxis);
	curve->setPen(pen);
	plot->addPlottable(curve);

	plot->xAxis->setLabel(tr("Time, ms"));
	plot->xAxis->setAutoTickStep(false);
	plot->xAxis->setTickStep(1.0);

//	plot->yAxis->setLabel(tr("PPM"));
	plot->yAxis->setRange(-0.05, 1.05);
	plot->yAxis->setAutoTicks(false);
	plot->yAxis->setAutoTickLabels(false);
	plot->yAxis->setTickVector(QVector<double>() << 0.0 << 1.0);
	plot->yAxis->setTickVectorLabels(QVector<QString>() << "low" << "high");

	// Расположение виджетов
	gridLayout->addWidget(labelPort          , 0, 0, 1, 1);
	gridLayout->addWidget(inputPort          , 0, 1, 1, 1);
	gridLayout->addWidget(inputUpdatePorts   , 0, 2, 1, 1);
	gridLayout->addWidget(labelSpeed         , 1, 0, 1, 1);
	gridLayout->addWidget(inputSpeed         , 1, 1, 1, 1);
	gridLayout->addWidget(inputConnect       , 1, 2, 1, 1);

	gridLayout->addWidget(labelChannelsCount , 2, 0, 1, 1);
	gridLayout->addWidget(inputChannelsCount , 2, 1, 1, 2);
	gridLayout->addWidget(labelPeriod        , 3, 0, 1, 1);
	gridLayout->addWidget(inputPeriod        , 3, 1, 1, 2);
	gridLayout->addWidget(labelPause         , 4, 0, 1, 1);
	gridLayout->addWidget(inputPause         , 4, 1, 1, 2);
	gridLayout->addWidget(labelMinimum       , 5, 0, 1, 1);
	gridLayout->addWidget(inputMinimum       , 5, 1, 1, 2);
	gridLayout->addWidget(labelMaximum       , 6, 0, 1, 1);
	gridLayout->addWidget(inputMaximum       , 6, 1, 1, 2);
	gridLayout->addWidget(plot               , 7, 0, 1, 3);
	gridLayout->addWidget(labelSyncPulse     , 8, 0, 1, 1);
	gridLayout->addWidget(outputSyncPulse    , 8, 1, 1, 2);

	setCentralWidget(centralWidget);
}

void MainWindow::enumeratePorts()
{
	int id = 0;
	inputPort->clear();
	foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
		QString tooltip =
		  QObject::tr(
			"Port: %1\n"
			"Location: %2\n"
			"Description: %3\n"
			"Manufacturer: %4\n"
			"Vendor Identifier: %5\n"
			"Product Identifier: %6\n"
			"Busy: %7"
		  )
		  .arg(info.portName())
		  .arg(info.systemLocation())
		  .arg(info.description())
		  .arg(info.manufacturer())
		  .arg(info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString())
		  .arg(info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16) : QString())
		  .arg(info.isBusy() ? QObject::tr("Yes") : QObject::tr("No"));

		inputPort->addItem(info.portName());
		inputPort->setItemData(id, QVariant(tooltip), Qt::ToolTipRole);
		if (info.isBusy()) {
			inputPort->setItemData(id, QVariant(QBrush(Qt::red)), Qt::ForegroundRole);
			//ui->inputPort->setItemData(id, QVariant(false), Qt::ItemIsEnabled);
		}
		id++;
	}
}

void MainWindow::enumerateBaudRates()
{
	inputSpeed->clear();
	foreach (qint32 BaudRate, QSerialPortInfo::standardBaudRates()) {
		inputSpeed->addItem(QString("%1").arg(BaudRate), QVariant(BaudRate));
	}
}

void MainWindow::setupChannelsUi(int count)
{

	for (int index = channels.count(); index < count; index++) {
		TChannelWidgets *widgets = new TChannelWidgets;

		widgets->label   = new QLabel(centralWidget);
		widgets->slider  = new QDoubleSlider(Qt::Horizontal, centralWidget);
		widgets->spinBox = new QDoubleSpinBox(centralWidget);

		widgets->slider->setTickPosition(QSlider::TicksBothSides);
		widgets->slider->setTickInterval(10.0);
		widgets->slider->setMaximum(100.0);

		widgets->spinBox->setMaximum(100);
		widgets->spinBox->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		connect(widgets->spinBox, SIGNAL(valueChanged(double)), &mapper, SLOT(map()));
		mapper.setMapping(widgets->spinBox, index);

		widgets->label->setText(tr("Channel #%1, %:").arg(channels.count()));

		connect(widgets->slider , SIGNAL(valueChanged(double)), widgets->spinBox, SLOT(setValue(double)));
		connect(widgets->spinBox, SIGNAL(valueChanged(double)), widgets->slider, SLOT(setValue(double)));
		connect(widgets->slider , SIGNAL(valueChanged(double)), SLOT(drawPlot()));

		// Расположение виджетов
		gridLayout->addWidget(widgets->label   , 9 + channels.count(), 0, 1, 1);
		gridLayout->addWidget(widgets->slider  , 9 + channels.count(), 1, 1, 1);
		gridLayout->addWidget(widgets->spinBox , 9 + channels.count(), 2, 1, 1);

		channels.append(widgets);
	}

	// Удаляем лишние виджеты при уменьшении количества каналов PPM сигнала
	while (channels.count() > count) {
		TChannelWidgets *widgets = channels.last();

		channels.removeLast();
		gridLayout->removeWidget(widgets->label);
		gridLayout->removeWidget(widgets->slider);
		gridLayout->removeWidget(widgets->spinBox);

		delete widgets->label;
		delete widgets->slider;
		delete widgets->spinBox;
	}

	drawPlot();
}

void MainWindow::saveSession()
{
	QSettings settings;

	settings.setValue("geometry"     , saveGeometry());
	settings.setValue("state"        , saveState());
	settings.setValue("count"        , inputChannelsCount->value());
	settings.setValue("period"       , inputPeriod->value());
	settings.setValue("pause"        , inputPause->value());
	settings.setValue("min"          , inputMinimum->value());
	settings.setValue("max"          , inputMaximum->value());
	settings.setValue("port"         , inputPort->currentText());
	settings.setValue("speed"        , inputSpeed->currentText());

	QStringList values;
	foreach (auto channel, channels) {
		values.append(QString("%1").arg(channel->slider->value()));
	}

	settings.setValue("values", values.join(";"));
}

void MainWindow::restoreSession()
{
	QSettings settings;

	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());
	inputChannelsCount->setValue(settings.value("count", 8).toInt());
	inputPeriod->setValue(settings.value("period", 22.5).toDouble());
	inputPause->setValue(settings.value("pause", 0.2).toDouble());
	inputMinimum->setValue(settings.value("min", 0.3).toDouble());
	inputMaximum->setValue(settings.value("max", 2.3).toDouble());
	inputPort->setCurrentText(settings.value("port").toString());
	inputSpeed->setCurrentText(settings.value("speed", "115200").toString());

	QStringList values = settings.value("values").toString().split(";");
	for (int i = 0; i < values.count(); ++i) {
		channels[i]->slider->setValue(values[i].toDouble());
	}

	drawPlot();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	saveSession();
	event->accept();
}

void MainWindow::drawPlot()
{
	double period = inputPeriod->value();       // Время всей последовательности импульсов, мсек
	double pause  = inputPause->value();        // Время паузы высокого уровня между импульсами, мсек
	double min    = inputMinimum->value();      // Минимальное время импульса и пазу канала, мсек
	double max    = inputMaximum->value();      // Максимальное время импульса и пазу канала, мсек
	double time   = 0.0;                        // Время, мсек
	int index     = 0;                          // Номер точки графика
	int count     = channels.count();           // Количество каналов
	int dotsCount = count * 4 + 10;             // Количество точек графика
	QVector<double> x(dotsCount), y(dotsCount); // Координаты точек графика

	// Координата Y
	for (int i = 0; i < dotsCount; ++i) {
		y[i] = (i / 2 % 2) ? 0.0 : 1.0;
	}

	// Первые 3 точки - завершение предидущего периода PPM сигнала
	x[index++] = -1.0;
	x[index++] = time;
	x[index++] = time;

	// По четыре точки на каждый канал
	for (int i = 0; i < count; i++) {
		time += pause;
		x[index++] = time;
		x[index++] = time;
		time += (double) channels[i]->slider->value() * (max - min) / 100 + min - pause;
		x[index++] = time;
		x[index++] = time;
	}

	time += pause;
	x[index++] = time;
	x[index++] = time;
	time = period;
	x[index++] = time;
	x[index++] = time;
	time += pause;
	x[index++] = time;
	x[index++] = time;
	time = qCeil(time + .5);
	x[index] = time;

	curve->setData(x, y);
	plot->xAxis->setRange(x[0], x[index]);
	plot->replot();

	updateSyncPulseValue();
}

void MainWindow::connectDisconnect()
{
	if (port.isOpen()) {
		port.close();
	} else {
		port.setPortName(inputPort->currentText());
		port.setBaudRate(inputSpeed->currentText().toInt());
		port.open(QIODevice::ReadWrite);
		devise.updateFrequency();
	}
	retranslateUi();
}

void MainWindow::onChanelValueChanged(int chanel)
{
	devise.setChanelValue(chanel, channels[chanel]->spinBox->value());
	updateSyncPulseValue();
}

void MainWindow::updateSyncPulseValue()
{
	double period = (double) inputPeriod->value();  // Время всей последовательности импульсов, мсек
	double min    = (double) inputMinimum->value(); // Минимальное время импульса и пазу канала, мсек
	double max    = (double) inputMaximum->value(); // Максимальное время импульса и пазу канала, мсек

	foreach (TChannelWidgets *channel, channels) {
		period -= (double) channel->slider->value() * (max - min) / 100 + min;
	}

	outputSyncPulse->setMaximum(period);
	outputSyncPulse->setValue(period);
}
