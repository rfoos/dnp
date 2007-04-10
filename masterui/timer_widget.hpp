//
// $Id$
//
// Copyright (C) 2007 Turner Technolgoies Inc. http://www.turner.ca
//
// Permission is hereby granted, free of charge, to any person 
// obtaining a copy of this software and associated documentation 
// files (the "Software"), to deal in the Software without 
// restriction, including without limitation the rights to use, 
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following 
// conditions:
//      
// The above copyright notice and this permission notice shall be 
// included in all copies or substantial portions of the Software. 
//      
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
// OTHER DEALINGS IN THE SOFTWARE.

#ifndef TIMER_WIDGET_H
#define TIMER_WIDGET_H

#include <QGroupBox>
#include "timer_interface.hpp"

class QCheckBox;
class QLabel;
class QPushButton;
class QTimer;

class TimerWidget : public QWidget
{
    Q_OBJECT
public:
    TimerWidget( QString name);

    void start();
    void stop();

    QTimer* timer;
signals:
    void timeout(); 

private slots:
    void qTimerTimeout();
    void manualMode(int state);

private:
    void setLedOn(bool on);

    bool active;

    QCheckBox*    manual;
    QLabel*       activeLed;
    QPushButton*  expire;

    QPixmap     onPixmap;
    QPixmap     offPixmap;
};

class TimersGroup : public QGroupBox, public TimerInterface
{
    Q_OBJECT
public:
    TimersGroup( bool master=true );

    bool master;

    // implementation of TimerInterface
    void activate( TimerId timerId);
    void cancel( TimerId timerId);

    // simply to update gui
    void timedout( TimerId timerId);

    TimerWidget* responseTimer;
    TimerWidget* keyChangeTimer;
    TimerWidget* challengeTimer;
    TimerWidget* sessionKeyTimer;

private:
    std::vector<TimerWidget*>  timers;

};

#endif
