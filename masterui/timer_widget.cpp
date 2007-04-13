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


#include <QtGui>
#include <iostream>
#include <assert.h>
#include "timer_widget.hpp"

TimerWidget::TimerWidget( QString name )
{
    onPixmap = QPixmap(":images/redledon.png");
    offPixmap = QPixmap(":images/redledoff.png");

    timer = new QTimer();
    timer->setSingleShot(true);

    QLabel* timerName = new QLabel(name);
    activeLed = new QLabel();
    manual = new QCheckBox(this);
    expire = new QPushButton(this);

    expire->setText("Expire");
    expire->setEnabled(false);

    setLedOn(false);

    QHBoxLayout* timerLayout = new QHBoxLayout(this);
    timerLayout->addWidget(activeLed);
    timerLayout->addWidget(timerName);
    timerLayout->addWidget(manual);
    timerLayout->addWidget(expire);

    setLayout(timerLayout);
    connect( timer, SIGNAL(timeout()), this, SLOT( qTimerTimeout()));
    connect( expire, SIGNAL(pressed()), this, SLOT( qTimerTimeout()));
    connect( manual, SIGNAL(stateChanged(int)), this, SLOT( manualMode(int)));
}

void TimerWidget::setLedOn(bool on)
{
    if (on)
    {
	active = true;
	activeLed->setPixmap(onPixmap);
    }
    else
    {
	active = false;
	activeLed->setPixmap(offPixmap);
    }

}

void TimerWidget::manualMode(int state)
{
    if (active)
    {
	if (state == Qt::Checked)
	{
	    timer->stop();
	    if (active)
		expire->setEnabled(true);
	}
	else if (state == Qt::Unchecked)
	{
	    expire->setEnabled(false);
	    timer->start();
	}
	else
	    assert(0);
    }
}

void TimerWidget::qTimerTimeout()
{
    setLedOn(false);
    expire->setEnabled(false);
    emit timeout();
}

void TimerWidget::start()
{
    setLedOn(true);
    if (manual->isChecked())
    {
	expire->setEnabled(true);
    }
    else
    {
	timer->start();
    }
}

void TimerWidget::stop()
{
    setLedOn(false);
    timer->stop();
}


  TimersGroup::TimersGroup(bool m) :
    QGroupBox("Timers   (check box for manual)"),
    master(m)
{
    timers.resize(NUM_TIMERS);

    QVBoxLayout* timersLayout = new QVBoxLayout(this);

    QSettings qsettings("TurnerTech", "DnpMasterStation");
    responseTimer = new TimerWidget("Response");
    responseTimer->timer->setInterval(
		        qsettings.value("timers/response_ms").toInt());


    timers[TimerInterface::RESPONSE] = responseTimer;
    timersLayout->addWidget(responseTimer);

    challengeTimer = new TimerWidget("Challenge");
    challengeTimer->timer->setInterval(
			      qsettings.value("timers/challenge_ms").toInt());

    timers[TimerInterface::CHALLENGE] = keyChangeTimer;
    timersLayout->addWidget(challengeTimer);
    challengeTimer->setEnabled(false);

    if (master)
    {
	keyChangeTimer = new TimerWidget("Key Change");
	keyChangeTimer->timer->setInterval(
		         qsettings.value("timers/keyChange_ms").toInt());
	
	timers[TimerInterface::KEY_CHANGE] = keyChangeTimer;
	timersLayout->addWidget(keyChangeTimer);
	keyChangeTimer->setEnabled(false);

    }
    else
    {
	sessionKeyTimer = new TimerWidget("Session Key");
	sessionKeyTimer->timer->setInterval(
			  qsettings.value("timers/sessionKey_ms").toInt());

	timers[TimerInterface::SESSION_KEY] = sessionKeyTimer;
	timersLayout->addWidget(sessionKeyTimer);
	sessionKeyTimer->setEnabled(false);
    }


}

void TimersGroup::activate( TimerId timerId)
{
    timers[timerId]->start();
}

void TimersGroup::cancel( TimerId timerId)
{
    timers[timerId]->stop();
}

