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

#ifndef QDOUBLESLIDER_H
#define QDOUBLESLIDER_H

#include <QSlider>

class QDoubleSlider : public QSlider
{
	Q_OBJECT
public:
	explicit QDoubleSlider(QWidget *parent = 0);
	explicit QDoubleSlider(Qt::Orientation orientation, QWidget * parent = 0);

	double value();
	double tickInterval();
	double minimum()    {return mDoubleMin;}
	double maximum()    {return mDoubleMax;}
	double singleStep() {return mDoubleStep;}

public slots:
	void setValue(double x);
	void setTickInterval(double x);
	void setMinimum(double x)    {mDoubleMin  = x; updateRange();}
	void setMaximum(double x)    {mDoubleMax  = x; updateRange();}
	void setSingleStep(double x) {mDoubleStep = x; updateRange();}

private slots:
	void notifyValueChanged(int x);

signals:
	void valueChanged(double);

private:
	double mDoubleMin;
	double mDoubleMax;
	double mDoubleStep;
	double mDoubleValue;
	int    mCorrespondingIntValue;

	void updateRange();
	void updateStep();
};

#endif // QDOUBLESLIDER_H
