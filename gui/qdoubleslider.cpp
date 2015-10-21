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

#include "qdoubleslider.h"
#include <QtMath>

QDoubleSlider::QDoubleSlider(QWidget *parent)
	: QSlider(parent)
	, mDoubleMin(0.0)
	, mDoubleMax(1.0)
	, mDoubleStep(0.01)
	, mDoubleValue(0.0)
{
	updateRange();
	connect(this, SIGNAL(valueChanged(int)), SLOT(notifyValueChanged(int)));
}

QDoubleSlider::QDoubleSlider(Qt::Orientation orientation, QWidget *parent)
	: QSlider(orientation, parent)
	, mDoubleMin(0.0)
	, mDoubleMax(1.0)
	, mDoubleStep(0.01)
	, mDoubleValue(0.0)
{
	updateRange();
	connect(this, SIGNAL(valueChanged(int)), SLOT(notifyValueChanged(int)));
}

void QDoubleSlider::updateRange()
{
	QSlider::setMinimum(0);
	QSlider::setMaximum(qCeil((mDoubleMax - mDoubleMin) / mDoubleStep));
	QSlider::setSingleStep(1);
	this->setValue(mDoubleValue);
}

void QDoubleSlider::setValue(double x)
{
	mDoubleValue = x;
	mCorrespondingIntValue = qRound((double) QSlider::maximum() * (x - mDoubleMin) / (mDoubleMax - mDoubleMin));
	QSlider::setValue(mCorrespondingIntValue);
}

void QDoubleSlider::setTickInterval(double x)
{
	QSlider::setTickInterval(qRound((double) QSlider::maximum() * (x - mDoubleMin) / (mDoubleMax - mDoubleMin)));
}

double QDoubleSlider::value()
{
	int intValue = QSlider::value();

	if (mCorrespondingIntValue != intValue) {
		mCorrespondingIntValue  = intValue;
		mDoubleValue = mDoubleMin + (double) intValue * (mDoubleMax - mDoubleMin) / (double) QSlider::maximum();
	}

	return mDoubleValue;
}

double QDoubleSlider::tickInterval()
{
	int intValue = QSlider::tickInterval();
	return mDoubleMin + (double) intValue * (mDoubleMax - mDoubleMin) / (double) QSlider::maximum();
}

void QDoubleSlider::updateStep()
{
	QSlider::setSingleStep(qCeil(100 * mDoubleStep / (mDoubleMax - mDoubleMin)));
}

void QDoubleSlider::notifyValueChanged(int x) {
	Q_UNUSED (x);
	emit valueChanged((double) value());
}
