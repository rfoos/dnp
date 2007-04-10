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
#include "settings.hpp"

Settings::Settings( Master* m_p, TimersGroup* t_p, QWidget* parent,
		    DnpAddr_t addr) :
  QFrame(parent),
  master_p(m_p),
  timers_p(t_p),
  outstationAddr(addr),
  requestsAllowed(true)
{

    settingsGroupLayout = new QVBoxLayout(this);

    createOperationGroup();
    createControlGroup();
    settingsGroupLayout->addWidget(timers_p);

    QFrame* secAndLogFrame = new QFrame;
    QHBoxLayout *layout = new QHBoxLayout(secAndLogFrame);
    createSecureAuthGroup(layout);
    createLogGroup(layout);
    settingsGroupLayout->addWidget(secAndLogFrame);

    setLayout(settingsGroupLayout);
}

void Settings::createOperationGroup()
{
    operationGroup = new QGroupBox(this);
    operationGroup->setAttribute(Qt::WA_ContentsPropagated);
    operationGroup->setTitle(tr("Polling"));

    pollButton = new QPushButton(operationGroup);
    pollButton->setText(tr("Poll"));

    QRadioButton *continuous = new QRadioButton(operationGroup);
    singleShot = new QRadioButton(operationGroup);
    singleShot->setText("Single Shot");
    continuous->setText("Continuous");

    delayEdit = new QLineEdit(operationGroup);
    QSettings qsettings;
    delayEdit->setText(qsettings.value("master/pollDelay_ms", 500).toString());
    delayEdit->setValidator(new QIntValidator(delayEdit));

    QLabel* delayEditName = new QLabel(tr("Delay(ms)"));
    delayEditName->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QGridLayout *operationGroupLayout= new QGridLayout(operationGroup);
    operationGroupLayout->addWidget(pollButton, 0, 0, 1, 2);
    operationGroupLayout->addWidget(singleShot, 1, 0);
    operationGroupLayout->addWidget(continuous, 1, 1);
    operationGroupLayout->addWidget(delayEditName, 2, 0);
    operationGroupLayout->addWidget(delayEdit, 2, 1);

    settingsGroupLayout->addWidget(operationGroup);

    delayTimer = new QTimer(this);
    delayTimer->setSingleShot(true);
    delayTimer->setInterval(delayEdit->text().toInt());
    connect(delayTimer, SIGNAL(timeout()), this,SLOT(delayTimerTimeout()));

    connect( singleShot, SIGNAL(toggled(bool)), this, SLOT(mode(bool)));
    singleShot->setChecked(true);

    connect( delayEdit, SIGNAL(editingFinished()), this, SLOT(setDelay()));


}

void Settings::createControlGroup()
{
			      

    controlGroup = new QGroupBox(this);
    controlGroup->setAttribute(Qt::WA_ContentsPropagated);
    controlGroup->setTitle("Binary Output");


    QPushButton* trip = new QPushButton(controlGroup);
    trip->setText(tr("Trip"));

    QPushButton* close = new QPushButton(controlGroup);
    close->setText(tr("Close"));

    QLabel* controlEditName = new QLabel(tr("Index"));
    controlEditName->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    controlEdit = new QLineEdit(0, controlGroup);
    controlEdit->setValidator(new QIntValidator(controlEdit));
    controlEdit->setText("0");

    QHBoxLayout *controlGroupLayout = new QHBoxLayout(controlGroup);
    controlGroupLayout->addWidget(trip);
    controlGroupLayout->addWidget(close);
    controlGroupLayout->addWidget(controlEditName);
    controlGroupLayout->addWidget(controlEdit);

    settingsGroupLayout->addWidget(controlGroup);

    connect( trip,  SIGNAL( pressed()), this, SLOT( trip()));
    connect( close, SIGNAL( pressed()), this, SLOT( close()));

 }

void Settings::trip()
{
    allowRequests(false);
    control(ControlOutputRelayBlock::TRIP);
}

void Settings::close()
{
    allowRequests(false);
    control(ControlOutputRelayBlock::CLOSE);
}

void Settings::control( ControlOutputRelayBlock::Code code)
{
    ControlOutputRelayBlock cb( code, controlEdit->text().toInt() );
    master_p->control(cb);
}

void Settings::createSecureAuthGroup( QLayout* layout)
{
    secAuthGroup = new QGroupBox(this);
    secAuthGroup->setAttribute(Qt::WA_ContentsPropagated);
    secAuthGroup->setTitle(tr("Secure Authentication"));
    secAuth = new QCheckBox(secAuthGroup);
    secAuth->setText("Enabled");

    QVBoxLayout* secGroupLayout = new QVBoxLayout(secAuthGroup);
    secGroupLayout->addWidget(secAuth);

    secAuth->setChecked(false);

    // secGroupLayout->setSizeConstraint(QLayout::SetFixedSize);
    layout->addWidget(secAuthGroup);

    connect( secAuth, SIGNAL(toggled(bool)), this, SLOT(setSecAuth(bool)));

}


void Settings::createLogGroup(QLayout* layout )
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

void Settings::allowRequests(bool allow)
{

    if (allow)
    {
	// printf("****** Allowing Requests **********\n");
	if (singleShot->isChecked() == false)
	{
	    // this is continuous mode
	    if (!delayTimer->isActive() && pollButton->isChecked()) 
		delayTimer->start();
	}
	else
	{
	    // ensure aftert switch from continuous it is right button type
	    pollButton->setChecked(false);
	    pollButton->setCheckable(false);
	}
	    
	if (requestsAllowed == allow)
	    return; // no need to do more - buttons should be in proper state

	pollButton->setEnabled(true);
	operationGroup->setEnabled(true);
	controlGroup->setEnabled(true);
    }
    else
    {
	if (singleShot->isChecked())
	    // allow the user to disable continuous by checking single shot
	    operationGroup->setEnabled(false);
	else
	    if (!(pollButton->isChecked()))
		pollButton->setEnabled(false);

	if (requestsAllowed == allow)
	    return; // no need to do more - buttons should be in proper state

	controlGroup->setEnabled(false);
    }

    requestsAllowed = allow;
}

void Settings::mode(bool singleShotMode)
{
    disconnect(pollButton, 0, 0, 0);
    if (singleShotMode == true)
    {
	delayTimer->stop();
	// if we are in the middle of a poll ensure the button is disabled
	if (requestsAllowed == false)
	    pollButton->setEnabled(false);
	connect( pollButton,SIGNAL( pressed()),this,SLOT(singleShotRequest()));
	delayEdit->setEnabled(false);
    }
    else
    {
	pollButton->setCheckable(true);
 	connect( pollButton, SIGNAL( toggled(bool)),
		 this, SLOT(continuousRequest(bool)));
	delayEdit->setEnabled(true);
    }
}

void Settings::singleShotRequest()
{
    allowRequests(false);
    master_p->startNewTransaction();
    emit masterStateChange();
}

void Settings::continuousRequest(bool on)
{
    if (on)
    {
	// get the first one started
	delayTimerTimeout();
    }
    else
    {
	// ensure the current continuous poll is completed before reenabling
	// continuous
       allowRequests(false);
    }
}

void Settings::setDelay()
{
    QSettings qsettings;
    qsettings.setValue("master/pollDelay_ms", delayEdit->text() );
    delayTimer->setInterval(delayEdit->text().toInt());
}

void Settings::setSecAuth(bool on)
{
    if (on)
    {
	timers_p->keyChangeTimer->setEnabled(true);
	timers_p->challengeTimer->setEnabled(true);
    }
    else
    {
	timers_p->keyChangeTimer->setEnabled(false);
	timers_p->challengeTimer->setEnabled(false);
    }
    master_p->enableSecureAuthentication( outstationAddr, on);
    emit masterStateChange();
}

void Settings::delayTimerTimeout()
{
    // printf("Delay Timer Timeout\n");
    if (singleShot->isChecked() == false)
    {
	allowRequests(false);
	master_p->startNewTransaction();
	emit masterStateChange();
    }
}

bool ConfigItem::warningHasBeenDisplayed = false;

ConfigItem::ConfigItem(ConfigurationFrame* cf,
			 QString name, QString defaultValue) :
    parent(cf)
{
    QSettings qsettings;

    // write the default value to the file if it does not already exist
    if (!qsettings.contains(name))
	qsettings.setValue(name, defaultValue );

    itemEdit = new QLineEdit(qsettings.value(name).toString());
    itemLabel = new QLabel(name);

    connect( itemEdit, SIGNAL(editingFinished()), this, SLOT(setItem()));
}

void ConfigItem::setItem()
{
    QSettings qsettings;

//     QTextStream out(stdout);
//     out << itemEdit->text() << "\n";
//     out << qsettings.value(itemLabel->text()).toString() << "\n";

    if ( itemEdit->text() != qsettings.value(itemLabel->text()).toString())
    {
	qsettings.setValue(itemLabel->text(), itemEdit->text() );
	// to indicate that it has changed since last restart
	itemLabel->setText(QString("<font color='red'>%1</font>").arg(
			   itemLabel->text()));
	if ( warningHasBeenDisplayed == false)
	{
	    QMessageBox::warning(parent, tr("Configuration Change Warning"),
				 tr("DNP Master Station must be restarted\n"
			      "before configuration changes take effect."));
	    warningHasBeenDisplayed = true;
	}
    }
}


ConfigurationFrame::ConfigurationFrame()
{
    char pair[][2][40] = 
    { {"master/address",           "1"},
      {"master/integrityInterval", "10"},
      {"station/address",          "2"},
      {"station/ip",               "192.168.2.164"},
      {"station/port",             "20001"},
      {"timers/response_ms",       "5000"},
      {"timers/keyChange_ms",      "120000"},
      {"timers/challenge_ms",      "240000"},
      {"timers/sessionKey_ms",     "480000"} };

    int numItems = sizeof(pair) / 80;

    QGridLayout* layout = new QGridLayout(this);

    for (int i=0; i<numItems; i++)
    {
	// printf("Adding %d %s=%s\n", i, pair[i][0], pair[i][1]);
	ConfigItem* item = new ConfigItem(this, pair[i][0], pair[i][1]);
	layout->addWidget(item->itemLabel,     i, 0);
	layout->addWidget(item->itemEdit,      i, 1);
    }
}
