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

#include <assert.h>
#include <QtGui>
#include "outstation_window.hpp"
#include "timer_interface.hpp"

class QTextEdit;

const QString OutstationWindow::shortPtNames[EventInterface::NUM_POINT_TYPES] =
  {"Ai",
   "Bi",
   "Ci",
   "Ao",
   "Bo",
   "St",
   "AP_St",
   "AP_St",
   "DL_St",
   "DL_St",
   "SA_St",
   "SA_St",
   "EP_St",
   "EP_St" };

OutstationWindow::OutstationWindow( Outstation::OutstationConfig&  oConfig,
				    Endpoint::EndpointConfig&      epConfig,
				    Datalink::DatalinkConfig&      dlConfig ) :
  ti(false), // not master
  debugLevel(1),
  outstationConfig( oConfig),
  endpointConfig( epConfig),
  datalinkConfig( dlConfig)
{
    // used by QSettings default contructor
    QCoreApplication::setOrganizationName("TurnerTech");
    QCoreApplication::setApplicationName("DnpOutstation");

    setWindowTitle(tr("DNP Outstation"));
    createActions();
    createMenus();
    createStatusBar();

    readSettings();

    statDisplay = new DataDisplay();
    createDnpOutstation();

    settings = new OutstationSettings(o_p, &ti, this);

    QDockWidget *dock = new QDockWidget(tr("Settings"), this);
    dock->setWidget(settings);
    dock->setFixedSize(dock->minimumSizeHint());
    addDockWidget(Qt::LeftDockWidgetArea, dock, Qt::Horizontal);
    viewMenu->addAction(dock->toggleViewAction());

    createStatDisplay();

    connect( settings, SIGNAL(outstationStateChange()),
	     this, SLOT(updateStateLabel()));

    connect( settings->verbose, SIGNAL(stateChanged(int)),
	     this, SLOT(setDebugLevel(int)));

    connect( ti.responseTimer, SIGNAL(timeout( TimerInterface::TimerId)),
	     this,SLOT( timeoutSlot( TimerInterface::TimerId)));
    connect( ti.challengeTimer, SIGNAL(timeout( TimerInterface::TimerId)),
	     this,SLOT( timeoutSlot( TimerInterface::TimerId)));
    connect( ti.sessionKeyTimer, SIGNAL(timeout( TimerInterface::TimerId)),
	     this,SLOT( timeoutSlot( TimerInterface::TimerId)));

    updateStateLabel();
}

void OutstationWindow::createStatDisplay()
{
    QDockWidget *dock = new QDockWidget(tr("Protocol Statistics"), this);
    dock->setWidget(statDisplay);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
}

void OutstationWindow::createActions()
{
     exitAct = new QAction(tr("E&xit"), this);
     exitAct->setShortcut(tr("Ctrl+Q"));
     exitAct->setStatusTip(tr("Exit the application"));
     connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

     aboutAct = new QAction(tr("&About"), this);
     aboutAct->setStatusTip(tr("Show the application's About box"));
     connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void OutstationWindow::createMenus()
{
     fileMenu = menuBar()->addMenu(tr("&File"));
     fileMenu->addAction(exitAct);

     viewMenu = menuBar()->addMenu(tr("&View"));

     menuBar()->addSeparator();

     helpMenu = menuBar()->addMenu(tr("&Help"));
     helpMenu->addAction(aboutAct);
}


void OutstationWindow::about()
{
    QMessageBox mb;

    mb.setIconPixmap(QPixmap(":images/TurnerTechIcon32x32.png"));
    mb.setWindowTitle("About DNP Outstation");

    QString aboutText(tr(
            "<h3>DNP Outstation</h3>"
            "%1"
	    "<p>"
	    "<b>Copyright (C) 2007 Turner Technologies Inc.</b> "
	    "<a href=\"http://www.turner.ca\">www.turner.ca</a>"
	    "</p>"
	    "<p>"
	    "This program implements the outstation portion of the DNP protocol. "
	    "For more information on DNP see "
	    "<a href=\"http://www.dnp.org\">www.dnp.org</a>"
	    "</p>"

	    "<p>"
"Turner Technologies developed this software using standard C++. Portions of the GUI code use the Qt4 open source cross platform toolkit, "
 "<a href=\"http://www.trolltech.com/qt\">www.trolltech.com/qt</a>"
	    "</p>"
	    ).arg(tr(version())));


    mb.setText(aboutText);
    QAbstractButton* licenseButton =
    mb.addButton(tr("View License"), QMessageBox::ActionRole);
    disconnect(licenseButton, 0, 0, 0);
    connect(licenseButton, SIGNAL(clicked()), this, SLOT(license()));
    mb.setStandardButtons(QMessageBox::Ok);
    mb.exec();
}
void OutstationWindow::license()
{
    QMessageBox mb;

    mb.setIconPixmap(QPixmap(":images/TurnerTechIcon32x32.png"));
    mb.setWindowTitle("DNP Outstation License");

    mb.setText(tr(x11_license()));

    mb.setStandardButtons(QMessageBox::Ok);
    mb.exec();
}

void OutstationWindow::createStatusBar()
{
    stateLabel = new QLabel();
    statusBar()->addWidget(stateLabel);
}

void OutstationWindow::createDnpOutstation()
{
    endpointConfig.debugLevel_p = &debugLevel;
    datalinkConfig.debugLevel_p = &debugLevel;
    outstationConfig.debugLevel_p = &debugLevel;

    ep_p = new Endpoint(endpointConfig, this);

    connect( ep_p, SIGNAL(data( Bytes*, unsigned long)),
   	     this, SLOT(rxData( Bytes*, unsigned long)));

    // datalink required pointer to the transmit interface
    datalinkConfig.tx_p  = ep_p;

    o_p = new Outstation ( outstationConfig, datalinkConfig,
			   this,   // event interface
			   &ti);   // timer interface
}

QString OutstationWindow::convertDnpIndexToName(DnpAddr_t   addr,
						DnpIndex_t  index,
						PointType_t pointType )
{
return QString("Stn%1_%2%3").arg(addr).arg(shortPtNames[pointType]).arg(index);
}

// implementation of EventInterface
void OutstationWindow::changePoint(  DnpAddr_t addr, DnpIndex_t index,
				     PointType_t    pointType,
				     int value, DnpTime_t timestamp)
{
    QString key = convertDnpIndexToName(addr, index, pointType);
    QString name = pointNameHash.value( key, key);

    if (pointType >= ST)
	statDisplay->updateData( name, pointType, value);
}

void  OutstationWindow::registerName(       DnpAddr_t      addr,
				      DnpIndex_t     index,
				      PointType_t    pointType,
				      char*          name,
				      int            initialValue )
{
    assert( pointType < NUM_POINT_TYPES);
    QString key = convertDnpIndexToName(addr, index, pointType);
    pointNameHash[key] = QString(name);
    changePoint( addr, index, pointType, initialValue);
}


void OutstationWindow::readSettings()
{
}
 
void OutstationWindow::writeSettings()
{
    QSettings qsettings;
    qsettings.setValue("pos", pos());
    qsettings.setValue("size", size());
}
 
void OutstationWindow::closeEvent(QCloseEvent *event)
{
    //event-ignore();
    // Save settings before closing:
    writeSettings();
    // Close:
    event->accept();
}

void OutstationWindow::rxData( Bytes* buf, unsigned long timeRxd)
{
    o_p->rxData(buf, timeRxd);
    updateStateLabel();
}

void OutstationWindow::timeoutSlot( TimerInterface::TimerId id)
{
    o_p->timeout( id);
    updateStateLabel();
}

void OutstationWindow::updateStateLabel()
{
    QString s("State: <b>");
    s.append(Outstation::stateStrings[ o_p->getState()]);
    s.append("</b>");

    if (settings->secAuth->isChecked())
    {
	s.append(" | Secure Authentication State: <b>");
	s.append(SecureAuthentication::stateStrings[ o_p->getSecAuthState()]);
	s.append("</b>");
    }

    stateLabel->setText(s);
}

void OutstationWindow::setDebugLevel(int state)
{
    if (state == Qt::Checked)
	debugLevel = 1;
    else
	debugLevel = 0;
}
