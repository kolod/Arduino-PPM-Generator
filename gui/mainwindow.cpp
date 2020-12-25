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

#include "mainwindow.h"
#include <QtMath>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, isStarted(false)
	, isFirmwareUploadingRequested(false)
{
	mClient = new QModbusRtuSerialMaster();
	mClient->setNumberOfRetries(1);

	device.setModbusClient(mClient);

	setupUi();
	retranslateUi();

	connect(&loader, &Loader::stateChanged, this, [this] (QString msg) {
		statusBar()->showMessage(msg);
	});

	connect(&loader, &Loader::uploadFinished, this, [this] (bool result) {
		if (result) {
			mClient->setConnectionParameter(QModbusDevice::SerialPortNameParameter, inputPort->currentText());
			mClient->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
			mClient->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, inputSpeed->currentText().toInt());
			mClient->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
			mClient->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);
			mClient->connectDevice();
		} else {
			QMessageBox message(this);
			message.setIconPixmap(QPixmap(":/icons/error.svg"));
			message.setText(tr("Firmware uploading failed."));
			message.setInformativeText(tr("Please check if the Arduino connected and right serial port selected."));
			message.setStandardButtons(QMessageBox::Ok);
			message.exec();
		}
	});

	connect(inputStartStop, &QPushButton::clicked, this, [this] {
		if (isStarted) device.stop(); else device.start();
	});

	connect(inputInversion, &QCheckBox::toggled, this, [this] (bool invert) {
		device.setProperty(DEVICE_PROPERTY_INVERSION, invert);
		drawPlot();
	});

	connect(&device, &ppm::started, this, [this] {
		isStarted = true;
		inputStartStop->setText(tr("Stop"));
	});

	connect(&device, &ppm::propertyChanged, this, [this](int id, QVariant value) {
		switch (id) {
		case DEVICE_PROPERTY_INVERSION:
			inputInversion->setChecked(value.toBool());
			break;

		case DEVICE_PROPERTY_MAX_PULSE:
			inputMaximum->setMaximum(value.toDouble());
			break;

		};
	});

	connect(&device, &ppm::stopped, this, [this] {
		isStarted = false;
		inputStartStop->setText(tr("Start"));
	});

	connect(mClient, &QModbusClient::stateChanged, this, [this] (QModbusDevice::State state) {
		if (isFirmwareUploadingRequested && (state == QModbusClient::UnconnectedState)) {
			isFirmwareUploadingRequested = false;
			uploadFirmware();
		}
	});

	connect(&device, &ppm::deviceConnectionFailed, this, [this] {
		QMessageBox message(this);
		message.setIconPixmap(QPixmap(":/icons/error.svg"));
		message.setText(tr("The device does not respond."));
		message.setInformativeText(tr("Try to update the firmware?"));
		message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		if (message.exec() == QMessageBox::Yes) {
			if (mClient->state() != QModbusClient::UnconnectedState) {
				isFirmwareUploadingRequested = true;
				mClient->disconnectDevice();
			} else {
				uploadFirmware();
			}
		}
	});

	connect(inputConnect, &QPushButton::clicked, this, [this] {
		if (mClient->state() == QModbusDevice::ConnectedState) {
			mClient->disconnectDevice();
		} else {
			mClient->setConnectionParameter(QModbusDevice::SerialPortNameParameter, inputPort->currentText());
			mClient->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
			mClient->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, inputSpeed->currentText().toInt());
			mClient->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
			mClient->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);
			mClient->connectDevice();
		}
	});

	connect(&device, &ppm::deviceConnected, this, [this] {
		inputConnect->setText(tr("Disconnect"));
		inputConnect->setIcon(QIcon(":/icons/disconnect.svg"));
		inputStartStop->setEnabled(true);
	});

	connect(&device, &ppm::deviceDisconnected, this, [this] {
		inputConnect->setText(tr("Connect"));
		inputConnect->setIcon(QIcon(":/icons/connect.svg"));
		inputStartStop->setDisabled(true);
	});


	connect(chartView->chart(), SIGNAL(widthChanged()), this, SLOT(xAxisUpdate()));

	connect(inputChannelsCount, SIGNAL(valueChanged(int)),    &device, SLOT(setChannelsCount(int)));
	connect(inputPeriod,        SIGNAL(valueChanged(double)), &device, SLOT(setPeriod(double)));
	connect(inputPause,         SIGNAL(valueChanged(double)), &device, SLOT(setPause(double)));
	connect(inputMinimum,       SIGNAL(valueChanged(double)), &device, SLOT(setMinimum(double)));
	connect(inputMaximum,       SIGNAL(valueChanged(double)), &device, SLOT(setMaximum(double)));

	// Updating the chart
	connect(inputPeriod,        SIGNAL(valueChanged(double)), SLOT(drawPlot()));
	connect(inputPause,         SIGNAL(valueChanged(double)), SLOT(drawPlot()));
	connect(inputMinimum,       SIGNAL(valueChanged(double)), SLOT(drawPlot()));
	connect(inputMaximum,       SIGNAL(valueChanged(double)), SLOT(drawPlot()));

	// Checking the correctness of the parameters affecting the pause duration
	connect(inputPeriod,        SIGNAL(valueChanged(double)), SLOT(check()));
	connect(inputMaximum,       SIGNAL(valueChanged(double)), SLOT(check()));
	connect(inputChannelsCount, SIGNAL(valueChanged(int)),    SLOT(check()));

	connect(inputUpdatePorts,   SIGNAL(clicked()),            SLOT(enumeratePorts()));
	connect(inputChannelsCount, SIGNAL(valueChanged(int)),    SLOT(setupChannelsUi(int)));
}

MainWindow::~MainWindow()
{
	mClient->disconnectDevice();
	mClient->deleteLater();
}

void MainWindow::setupUi()
{
	centralWidget      = new QWidget(this);

	gridLayout         = new QGridLayout(centralWidget);
	gridLayout->setContentsMargins(5, 5, 5, 5);

	statusBar()->show();

	// Port
	labelPort          = new QLabel(centralWidget);
	inputPort          = new QComboBox(centralWidget);
	inputPort->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	inputUpdatePorts   = new QPushButton(centralWidget);
	inputUpdatePorts->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	inputUpdatePorts->setIcon(QIcon(":/icons/update.svg"));
	enumeratePorts();

	// Speed
	labelSpeed         = new QLabel(centralWidget);
	inputSpeed         = new QComboBox(centralWidget);
	inputConnect       = new QPushButton(centralWidget);
	inputConnect->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	inputConnect->setIcon(QIcon(":/icons/connect.svg"));
	inputSpeed->setEditable(true);
	inputSpeed->setValidator(new QIntValidator(1, 24000000, this));
	enumerateBaudRates();

	// Start / Stop button
	inputStartStop     = new QPushButton(centralWidget);
	inputStartStop->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	inputStartStop->setDisabled(true);

	// Inversion of PPM signal
	inputInversion     = new QCheckBox(centralWidget);

	// Input channel quantity
	labelChannelsCount = new QLabel(centralWidget);
	inputChannelsCount = new QSpinBox(centralWidget);
	inputChannelsCount->setMinimum(1);
	inputChannelsCount->setMaximum(16);
	inputChannelsCount->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	// Input duration period (milliseconds)
	labelPeriod        = new QLabel(centralWidget);
	inputPeriod        = new QDoubleSpinBox(centralWidget);
	inputPeriod->setMinimum(0.0);
	inputPeriod->setMaximum(100.0);
	inputPeriod->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	// Input pause duration (milliseconds)
	labelPause         = new QLabel(centralWidget);
	inputPause         = new QDoubleSpinBox(centralWidget);
	inputPause->setMinimum(0.0);
	inputPause->setMaximum(10.0);
	inputPause->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	// Input minimum duration (milliseconds)
	labelMinimum       = new QLabel(centralWidget);
	inputMinimum       = new QDoubleSpinBox(centralWidget);
	inputMinimum->setMinimum(0.0);
	inputMinimum->setMaximum(10.0);
	inputMinimum->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	// Input maximum duration (milliseconds)
	labelMaximum       = new QLabel(centralWidget);
	inputMaximum       = new QDoubleSpinBox(centralWidget);
	inputMaximum->setMinimum(0.0);
	inputMaximum->setMaximum(10.0);
	inputMaximum->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	// Graph
	line               = new QtCharts::QLineSeries;
	xAxis              = new QtCharts::QValueAxis;
	yAxis              = new QtCharts::QValueAxis;
	chartView          = new QtCharts::QChartView;

	xAxis->setMinorTickCount(5);

	yAxis->setRange(-0.01, 1.01);
	yAxis->setTickCount(2);
	yAxis->setMinorTickCount(5);
	yAxis->setLabelsVisible(false);

	chartView->setRenderHint(QPainter::Antialiasing);
	chartView->chart()->addSeries(line);
	chartView->chart()->legend()->hide();
	chartView->chart()->addAxis(xAxis, Qt::AlignBottom);
	chartView->chart()->addAxis(yAxis, Qt::AlignLeft);

	line->attachAxis(xAxis);
	line->attachAxis(yAxis);

	// The output of the sync pulse length (microseconds)
	labelSyncPulse     = new QLabel(centralWidget);
	outputSyncPulse    = new QDoubleSpinBox(centralWidget);
	outputSyncPulse->setReadOnly(true);
	outputSyncPulse->setMinimum(0.0);
	outputSyncPulse->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	// Location of widgets
	gridLayout->addWidget(labelPort          , 0, 0, 1, 1);
	gridLayout->addWidget(labelSpeed         , 1, 0, 1, 1);
	gridLayout->addWidget(labelChannelsCount , 2, 0, 1, 1);
	gridLayout->addWidget(labelPeriod        , 3, 0, 1, 1);
	gridLayout->addWidget(labelPause         , 4, 0, 1, 1);
	gridLayout->addWidget(labelMinimum       , 5, 0, 1, 1);
	gridLayout->addWidget(labelMaximum       , 6, 0, 1, 1);
	gridLayout->addWidget(chartView          , 7, 0, 1, 4);

	gridLayout->addWidget(labelSyncPulse     , 8, 0, 1, 1);

	gridLayout->addWidget(inputPort          , 0, 1, 1, 1);
	gridLayout->addWidget(inputSpeed         , 1, 1, 1, 1);
	gridLayout->addWidget(inputChannelsCount , 2, 1, 1, 3);
	gridLayout->addWidget(inputPeriod        , 3, 1, 1, 3);
	gridLayout->addWidget(inputPause         , 4, 1, 1, 3);
	gridLayout->addWidget(inputMinimum       , 5, 1, 1, 3);
	gridLayout->addWidget(inputMaximum       , 6, 1, 1, 3);
	gridLayout->addWidget(outputSyncPulse    , 8, 1, 1, 3);

	gridLayout->addWidget(inputUpdatePorts   , 0, 2, 1, 1);
	gridLayout->addWidget(inputConnect       , 1, 2, 1, 1);

	gridLayout->addWidget(inputStartStop     , 0, 3, 1, 1);
	gridLayout->addWidget(inputInversion     , 1, 3, 1, 1);

//	gridLayout->setColumnMinimumWidth(2, 150);
//	gridLayout->setColumnMinimumWidth(3, 150);

	setCentralWidget(centralWidget);
}

void MainWindow::setupChannelsUi(int count)
{

	for (int index = channels.count(); index < count; index++) {
		TChannelWidgets *widgets = new TChannelWidgets;

		widgets->label   = new QLabel(centralWidget);
		widgets->label->setText(tr("Channel #%1, %:").arg(channels.count()));
		widgets->label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

		widgets->slider  = new QSlider(Qt::Horizontal, centralWidget);
		widgets->slider->setTickPosition(QSlider::TicksBothSides);
		widgets->slider->setRange(0, 1000);
		widgets->slider->setTickInterval(100);
		widgets->slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

		widgets->spinBox = new QDoubleSpinBox(centralWidget);
		widgets->spinBox->setMaximum(100);
		widgets->spinBox->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		widgets->spinBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

#ifdef BINDING
		widgets->bind = new QPushButton(centralWidget);
		widgets->bind->setText(tr("Bind"));
		widgets->bind->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
#endif

		connect(widgets->spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this, index] (double value) {
			channels[index]->slider->setValue(int(value * 10));
			device.setChanelValue(index, value);
			updateSyncPulseValue();
			drawPlot();
		});

		connect(widgets->slider, &QSlider::valueChanged, this, [this, index] (int value) {
			channels[index]->spinBox->setValue(double(value) / 10);
		});

		// Location of widgets

		gridLayout->addWidget(widgets->label   , 9 + channels.count(), 0, 1, 1);
		gridLayout->addWidget(widgets->slider  , 9 + channels.count(), 1, 1, 2);
		gridLayout->addWidget(widgets->spinBox , 9 + channels.count(), 3, 1, 1);

#ifdef BINDING
		gridLayout->addWidget(widgets->label   , 9 + channels.count(), 0, 1, 1);
		gridLayout->addWidget(widgets->slider  , 9 + channels.count(), 1, 1, 1);
		gridLayout->addWidget(widgets->spinBox , 9 + channels.count(), 2, 1, 1);
		gridLayout->addWidget(widgets->bind    , 9 + channels.count(), 3, 1, 1);
#endif

		channels.append(widgets);
	}

	// Removing unnecessary widgets when the number of PPM signal channels decreases
	while (channels.count() > count) {
		TChannelWidgets *widgets = channels.last();

		channels.removeLast();
		gridLayout->removeWidget(widgets->label);
		gridLayout->removeWidget(widgets->slider);
		gridLayout->removeWidget(widgets->spinBox);
		gridLayout->removeWidget(widgets->bind);

		delete widgets->label;
		delete widgets->slider;
		delete widgets->spinBox;
#ifdef BINDING
		delete widgets->bind;
#endif
	}

	drawPlot();
}

void MainWindow::retranslateUi()
{
	setWindowTitle(tr("Arduino PPM Generator"));
	labelChannelsCount->setText(tr("Channels count:"));
	labelPeriod->setText(tr("Period, ms:"));
	labelPause->setText(tr("Pause, ms:"));
	labelMinimum->setText(tr("Minimum, ms:"));
	labelMaximum->setText(tr("Maximum, ms:"));
	labelPort->setText(tr("Serial port:"));
	inputUpdatePorts->setText(tr("Update"));
	labelSpeed->setText(tr("Baud rate, Bd:"));
	inputConnect->setText(tr("Connect"));
	labelSyncPulse->setText(tr("Sync period, ms:"));
	inputStartStop->setText(tr("Start"));
	inputInversion->setText(tr("Inversion"));
	xAxis->setTitleText(tr("Time, ms"));
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
			"Product Identifier: %6"
		  )
		  .arg(info.portName())
		  .arg(info.systemLocation())
		  .arg(info.description())
		  .arg(info.manufacturer())
		  .arg(info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString())
		  .arg(info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16) : QString());

		inputPort->addItem(info.portName());
		inputPort->setItemData(id, QVariant(tooltip), Qt::ToolTipRole);
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

void MainWindow::saveSession()
{
	QSettings settings;

	settings.setValue("geometry"     , saveGeometry());
	settings.setValue("state"        , saveState());
	settings.setValue("count"        , inputChannelsCount->value());
	settings.setValue("inversion"    , inputInversion->isChecked());
	settings.setValue("period"       , inputPeriod->value());
	settings.setValue("pause"        , inputPause->value());
	settings.setValue("min"          , inputMinimum->value());
	settings.setValue("max"          , inputMaximum->value());
	settings.setValue("port"         , inputPort->currentText());
	settings.setValue("speed"        , inputSpeed->currentText());

	QStringList values;
	foreach (auto channel, channels) {
		values.append(QString("%1").arg(channel->spinBox->value()));
	}

	settings.setValue("values", values.join(";"));
}

void MainWindow::restoreSession()
{
	QSettings settings;

	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("state").toByteArray());
	inputChannelsCount->setValue(settings.value("count", 8).toInt());
	inputInversion->setChecked(settings.value("inversion", false).toBool());
	inputPeriod->setValue(settings.value("period", 22.5).toDouble());
	inputPause->setValue(settings.value("pause", 0.2).toDouble());
	inputMinimum->setValue(settings.value("min", 0.3).toDouble());
	inputMaximum->setValue(settings.value("max", 2.3).toDouble());
	inputPort->setCurrentText(settings.value("port").toString());
	inputSpeed->setCurrentText(settings.value("speed", "115200").toString());

	QStringList values = settings.value("values").toString().split(";");
	for (int i = 0; i < values.count(); ++i) {
		channels[i]->spinBox->setValue(values[i].toDouble());
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
	qreal period = static_cast<qreal>(inputPeriod->value());  // Time of the entire sequence of pulses (milliseconds)
	qreal left   = static_cast<qreal>(-qCeil(0.03 * period)); // The left gap for better clarity
	qreal right  = static_cast<qreal>(qCeil(1.03 * period));  // The right gap for better clarity
	qreal pause  = static_cast<qreal>(inputPause->value());   // High-level pause time between pulses (milliseconds)
	qreal min    = static_cast<qreal>(inputMinimum->value()); // Minimum channel pulse and pause time (milliseconds)
	qreal max    = static_cast<qreal>(inputMaximum->value()); // Maximum channel pulse and pause time (milliseconds)
	int count    = channels.count();                          // The number of channels
	qreal x = 0.0;
	qreal y1, y2;

	if (inputInversion->isChecked()) {
		y1 = 0.0, y2 = 1.0;
	} else {
		y1 = 1.0, y2 = 0.0;
	}

	line->clear();
	line->append(left, y1);
	line->append( 0.0, y1);
	line->append( 0.0, y2);

	for (int i = 0; i < count; i++) {
		x += pause;

		line->append(x, y2);
		line->append(x, y1);
		x += channels[i]->spinBox->value() * (max - min) / 100 + min - pause;

		line->append(x, y1);
		line->append(x, y2);
	}

	x += pause;
	line->append(x, y2);
	line->append(x, y1);

	line->append(period, y1);
	line->append(period, y2);

	x = period + pause;
	line->append(x, y2);
	line->append(x, y1);

	line->append(right, y1);

	xAxis->setRange(left, right);

	xAxisUpdate();
	updateSyncPulseValue();
}

void MainWindow::updateSyncPulseValue()
{
	double sync    = static_cast<double>(inputPeriod->value());  // Time of the entire sequence of pulses (milliseconds)
	double min     = static_cast<double>(inputMinimum->value()); // Minimum channel pulse and pause time (milliseconds)
	double max     = static_cast<double>(inputMaximum->value()); // Maximum channel pulse and pause time (milliseconds)

	foreach (auto *channel, channels) {
		sync -= static_cast<double>(channel->spinBox->value()) * (max - min) / 100 + min;
	}

	outputSyncPulse->setMaximum(sync);
	outputSyncPulse->setValue(sync);
	outputSyncPulse->setPalette(gradient(sync, max));
}

void MainWindow::check()
{
	// Maximum channel pulse and pause time (milliseconds)
	double max = (double) inputMaximum->value();

	// Synchronization pulse time at the maximum value of all signals (milliseconds)
	double sync = inputPeriod->value() - max * double(channels.count());

	inputPeriod->setPalette(gradient(sync, max));
	inputMaximum->setPalette(gradient(sync, max));
}

//
QPalette MainWindow::gradient(double value, double max)
{
	QColor color(Qt::white);
	QPalette newPalete(palette());

	if      (value <= max * 1.0) color = QColor(255,   0,   0);
	else if (value <= max * 1.2) color = QColor(255, 119,   0);
	else if (value <= max * 1.5) color = QColor(255, 208,   0);
	else if (value <= max * 2.0) color = QColor(255, 255,   0);

	newPalete.setColor(QPalette::Base, color);
	return newPalete;
}

void MainWindow::xAxisUpdate()
{
	qreal period = static_cast<qreal>(inputPeriod->value());  // Time of the entire sequence of pulses (milliseconds)
	qreal left   = static_cast<qreal>(-qCeil(0.03 * period)); // The left gap for better clarity
	qreal right  = static_cast<qreal>(qCeil(1.03 * period));  // The right gap for better clarity
	int range    = static_cast<int>(right - left);            // Range of x values on the graph

	// TODO: Fractional values on the x-axis look ugly.

	if (chartView->width() > 1000) {
		xAxis->setTickCount(range + 1);
	} else if (chartView->width() > 600) {
		xAxis->setTickCount(range / 2 + 1);
	} else if (chartView->width() > 300) {
		xAxis->setTickCount(range / 4 + 1);
	} else{
		xAxis->setTickCount(range / 8 + 1);
	}
}

void MainWindow::uploadFirmware()
{
	QFile firmware(":/firmware.bin");
	firmware.open(QIODevice::ReadOnly);
	QByteArray data = firmware.readAll();
	firmware.close();

	loader.setPortName(inputPort->currentText());
	loader.uploadFirmware(data);
}
