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
#include <assert.h>
#include "outstation_settings.hpp"

OutstationSettings::OutstationSettings( Outstation* o_p,
                                        TimersGroup* t_p, QWidget* parent) :
  QFrame(parent),
  outstation_p(o_p),
  timers_p(t_p)
{
    settingsGroupLayout = new QVBoxLayout(this);

    settingsGroupLayout->addWidget(timers_p);

    QFrame* secAndLogFrame = new QFrame;
    QHBoxLayout *layout = new QHBoxLayout(secAndLogFrame);
    createSecureAuthGroup(layout);
    createLogGroup(layout);
    settingsGroupLayout->addWidget(secAndLogFrame);

    setLayout(settingsGroupLayout);
}

void OutstationSettings::createSecureAuthGroup( QLayout* layout)
{
    secAuthGroup = new QGroupBox(this);
    secAuthGroup->setAttribute(Qt::WA_ContentsPropagated);
    secAuthGroup->setTitle(tr("Secure Authentication"));
    secAuth = new QCheckBox(secAuthGroup);
    secAuth->setText("Enabled");

    QVBoxLayout* secGroupLayout = new QVBoxLayout(secAuthGroup);
    secGroupLayout->addWidget(secAuth);

    secAuth->setChecked(true);
    setSecAuth(true);

    secAuth->setEnabled(false);

    // secGroupLayout->setSizeConstraint(QLayout::SetFixedSize);
    layout->addWidget(secAuthGroup);

    connect( secAuth, SIGNAL(toggled(bool)), this, SLOT(setSecAuth(bool)));

}

void OutstationSettings::createLogGroup(QLayout* layout )
{

    QGroupBox* logGroup = new QGroupBox(this);
    logGroup->setAttribute(Qt::WA_ContentsPropagated);
    logGroup->setTitle(tr("Log"));
    verbose = new QCheckBox(logGroup);
    verbose->setText("Verbose");

    QVBoxLayout* logGroupLayout = new QVBoxLayout(logGroup);
    logGroupLayout->addWidget(verbose);

    verbose->setChecked(true);

    layout->addWidget(logGroup);
}

void OutstationSettings::setSecAuth(bool on)
{
    if (on)
    {
        timers_p->sessionKeyTimer->setEnabled(true);
        timers_p->challengeTimer->setEnabled(true);
    }
    else
    {
        timers_p->sessionKeyTimer->setEnabled(false);
        timers_p->challengeTimer->setEnabled(false);
    }


    outstation_p->enableSecureAuthentication( on);
    emit outstationStateChange();
}

