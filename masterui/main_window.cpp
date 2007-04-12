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
#include <QThread>  // for the outstation thread
#include <QtGui>
#include <QAbstractButton>
#include "common.hpp"
#include "main_window.hpp"
#include "timer_interface.hpp"
#include "outstation.hpp"

class QTextEdit;


void OutstationThread::run()
{
    mainWindow->show();
    exec();
}

const QString MainWindow::commsStrings[2] =
  {
   "Communications: <b><font color='red'>None</font></b>",
   "Communications: <b><font color='green'>Ok</font></b>",
  };

const QString MainWindow::shortPtNames[EventInterface::NUM_POINT_TYPES] =
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

MainWindow::MainWindow() :
    debugLevel(1),
    respTimeout(1000),
    integrityPollInterval(10)
{
    // used by QSettings default contructor
    QCoreApplication::setOrganizationName("TurnerTech");
    QCoreApplication::setApplicationName("DnpMasterStation");

    setWindowTitle(tr("DNP Master Station"));
    setDockNestingEnabled ( true );
    createActions();
    createMenus();
    createStatusBar();


    ConfigurationFrame* configurationTab = new ConfigurationFrame();

    readSettings();

    statDisplay = new DataDisplay();
    pointDisplay = new DataDisplay();
    cosDisplay = new CosDisplay();


    createDnpMaster();

    QTabWidget* settingsTabs = new QTabWidget();
    settingsTab = new Settings(m_p, &ti, this, stationConfig.addr);
    settingsTabs->addTab(settingsTab,"Actions");
    settingsTabs->addTab(configurationTab,"Configuration");


    QDockWidget *dock = new QDockWidget(tr("Settings"), this);
    dock->setWidget(settingsTabs);
    dock->setFixedSize(dock->minimumSizeHint());
    addDockWidget(Qt::RightDockWidgetArea, dock, Qt::Horizontal);
    viewMenu->addAction(dock->toggleViewAction());

    createCosDisplay();
    createStatDisplay();
    createPointDisplay();


    connect( settingsTab, SIGNAL(masterStateChange()),
	     this, SLOT(updateStateLabel()));

    connect( settingsTab->verbose, SIGNAL(stateChanged(int)),
	     this, SLOT(setDebugLevel(int)));

    connect( ti.responseTimer, SIGNAL(timeout()),
	     this,SLOT(responseTimeout()));

    updateStateLabel();
}

void MainWindow::createStatDisplay()
{

    QDockWidget *dock = new QDockWidget(tr("Protocol Statistics"), this);
    dock->setWidget(statDisplay);
    addDockWidget(Qt::RightDockWidgetArea, dock, Qt::Horizontal);
    viewMenu->addAction(dock->toggleViewAction());
}

void MainWindow::createPointDisplay()
{

    QDockWidget *dock = new QDockWidget(tr("Current Point Values"), this);
    dock->setWidget(pointDisplay);
    addDockWidget(Qt::RightDockWidgetArea, dock, Qt::Horizontal);
    viewMenu->addAction(dock->toggleViewAction());
}

void MainWindow::createCosDisplay()
{

    QDockWidget *dock = new QDockWidget(tr("Change Of State Log"), this);
    dock->setWidget(cosDisplay);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
}

void MainWindow::createActions()
{
     exitAct = new QAction(tr("E&xit"), this);
     exitAct->setShortcut(tr("Ctrl+Q"));
     exitAct->setStatusTip(tr("Exit the application"));
     connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

     aboutAct = new QAction(tr("&About"), this);
     aboutAct->setStatusTip(tr("Show the application's About box"));
     connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

     outstationAct = new QAction(tr("&Simulate Outstation"), this);
     outstationAct->setStatusTip(tr("Start an outstation simulator"));
     connect(outstationAct, SIGNAL(triggered()),this,SLOT(startOutstation()));
}

void MainWindow::createMenus()
{
     fileMenu = menuBar()->addMenu(tr("&File"));
     fileMenu->addAction(outstationAct);
     fileMenu->addAction(exitAct);

     viewMenu = menuBar()->addMenu(tr("&View"));

     menuBar()->addSeparator();

     helpMenu = menuBar()->addMenu(tr("&Help"));
     helpMenu->addAction(aboutAct);
}


void MainWindow::startOutstation()
{
    if (outstationThread.isRunning())
	return;

    QSettings qsettings;

    Outstation::OutstationConfig             outstationConfig;
    Endpoint::EndpointConfig                 epConfig;
    Datalink::DatalinkConfig                 dlConfig;
    RemoteDevice                             rd;
    std::map<DnpAddr_t, RemoteDevice>        rdMap;

    // debugs will be set by the outstation window

    // outstation config
    outstationConfig.addr                = stationConfig.addr;
    outstationConfig.masterAddr          = masterConfig.addr;
    outstationConfig.userNum             = 3; // hard coded for prototype

    // end point config
    epConfig.ownerDnpAddr          = masterConfig.addr;
    epConfig.tcp                   = false;
    epConfig.initiating            = false;
    epConfig.listenPort            = qsettings.value("station/port").toInt();

    rd.ip                          = QHostAddress("127.0.0.1");
    rd.port                        = 20000;
    rdMap[masterConfig.addr]       = rd;

    epConfig.deviceMap                   = rdMap;

    // datalink config
    dlConfig.addr                  = outstationConfig.addr;
    dlConfig.isMaster              = false;

    outstationThread.mainWindow = new OutstationWindow( outstationConfig,
							epConfig,
							dlConfig);
    outstationThread.run();
}

void MainWindow::about()
{
    QMessageBox mb;

    mb.setIconPixmap(QPixmap(":images/TurnerTechIcon32x32.png"));
    mb.setWindowTitle("About DNP Master Station");

    QString aboutText(tr(
            "<h3>DNP Master Station</h3>"
            "%1"
	    "<p>"
	    "<b>Copyright (C) 2007 Turner Technologies Inc.</b> "
	    "<a href=\"http://www.turner.ca\">www.turner.ca</a>"
	    "</p>"
	    "<p>"
	    "This program implements the master portion of the DNP protocol. "
	    "For more information on DNP see "
	    "<a href=\"http://www.dnp.org\">www.dnp.org</a>"
	    "</p>"

	    "<p>"
"Turner Technologies developed this software using standard C++. Portions of the GUI code use the Qt4 open source cross platform toolkit, "
 "<a href=\"http://www.trolltech.com/qt\">www.trolltech.com/qt</a>"
	    "</p>"
	    ).arg(version()));


    mb.setText(aboutText);
    QAbstractButton* licenseButton =
    mb.addButton(tr("View License"), QMessageBox::ActionRole);
    disconnect(licenseButton, 0, 0, 0);
    connect(licenseButton, SIGNAL(clicked()), this, SLOT(license()));
    mb.setStandardButtons(QMessageBox::Ok);
    mb.exec();
}
void MainWindow::license()
{
    QMessageBox mb;

    mb.setIconPixmap(QPixmap(":images/TurnerTechIcon32x32.png"));
    mb.setWindowTitle("DNP Master Station License");

    mb.setText(tr(x11_license()));

    mb.setStandardButtons(QMessageBox::Ok);
    mb.exec();
}

void MainWindow::createStatusBar()
{
    stateLabel = new QLabel();
    commsLabel = new QLabel(commsStrings[0]);
    statusBar()->addWidget(stateLabel);
    // permanent makes it go onto the right
    statusBar()->addPermanentWidget(commsLabel);
}


void MainWindow::createDnpMaster()
{
    // create master

    ep_p = new Endpoint(endpointConfig, this);


    connect( ep_p, SIGNAL(data( Bytes*, unsigned long)),
  	     this, SLOT(rxData( Bytes*, unsigned long)));


    // datalink required pointer to the transmit interface
    datalinkConfig.tx_p                  = ep_p;

    m_p = new Master (masterConfig, datalinkConfig, &stationConfig,
		      1,      // one station
		      this,   // event interface
		      &ti);   // timer interface
}

QString MainWindow::convertDnpIndexToName( DnpAddr_t addr, DnpIndex_t index,
					   PointType_t    pointType )
{
return QString("Stn%1_%2%3").arg(addr).arg(shortPtNames[pointType]).arg(index);
}

// implementation of EventInterface
void MainWindow::changePoint(        DnpAddr_t addr, DnpIndex_t index,
				     PointType_t    pointType,
				     int value, DnpTime_t timestamp)
{
    QString key = convertDnpIndexToName(addr, index, pointType);
    QString name = pointNameHash.value( key, key);

    if (pointType >= ST)
	statDisplay->updateData( name, pointType, value);
    else
    {
	bool cos = pointDisplay->updateData( name, pointType, value);
	if (cos)
	    cosDisplay->updateData( name, pointType, value, timestamp);
    }
}

void  MainWindow::registerName(       DnpAddr_t      addr,
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


// the settings frame must be contructed before this method is called
// to ensure all of the default settings are saved 
void MainWindow::readSettings()
{
    QSettings qsettings;
//     QPoint pos = qsettings.value("pos", QPoint(0, 0)).toPoint();
//     QSize size = qsettings.value("size", QSize(770, 550)).toSize();
//     move(pos);
//     resize(size);

    // master and station config
    masterConfig.addr = qsettings.value("master/address").toInt();
    masterConfig.consecutiveTimeoutsForCommsFail = 1;
    integrityPollInterval =qsettings.value("master/integrityInterval").toInt();
    masterConfig.integrityPollInterval_p = &integrityPollInterval;
    masterConfig.debugLevel_p = &debugLevel;
    stationConfig.addr = qsettings.value("station/address").toInt();;
    stationConfig.debugLevel_p = &debugLevel;

    // end point config
    endpointConfig.ownerDnpAddr     = masterConfig.addr;
    endpointConfig.tcp              = false;
    endpointConfig.initiating       = false;
    endpointConfig.listenPort       = 20000;

    remoteDevice.ip = QHostAddress(qsettings.value("station/ip").toString());
    remoteDevice.port = qsettings.value("station/port").toInt();
    deviceMap[stationConfig.addr]   = remoteDevice;

    endpointConfig.deviceMap        = deviceMap;
    endpointConfig.debugLevel_p     = &debugLevel;


    // datalink config
    datalinkConfig.addr                  = masterConfig.addr;
    datalinkConfig.isMaster              = true;
    datalinkConfig.debugLevel_p          = &debugLevel;

}
 
void MainWindow::writeSettings()
{
//     QSettings qsettings;
//     qsettings.setValue("pos", pos());
//     qsettings.setValue("size", size());
}
 
void MainWindow::closeEvent(QCloseEvent *event)
{
    //event-ignore();
    // Save settings before closing:
    writeSettings();
    // Close:
    event->accept();
}

void MainWindow::rxData( Bytes* buf, unsigned long timeRxd)
{
    DnpStat_t state = m_p->rxData(buf, timeRxd);
    if ( state == Station::IDLE)
    {
	settingsTab->allowRequests();
    }
    updateStateLabel();
}

void MainWindow::responseTimeout()
{
    DnpStat_t state = m_p->timeout(TimerInterface::RESPONSE);
    if (state == Station::IDLE)
    {
	settingsTab->allowRequests();
    }
    updateStateLabel();
}

void MainWindow::updateStateLabel()
{
    QString s("State: <b>");
    s.append(Station::stateStrings[ m_p->getState()]);
    s.append("</b>");

    if (settingsTab->secAuth->isChecked())
    {
	s.append(" | Secure Authentication State: <b>");
	s.append(SecureAuthentication::stateStrings[ m_p->getSecAuthState()]);
	s.append("</b>");
    }

    stateLabel->setText(s);

    commsLabel->setText(commsStrings[m_p->getStat(stationConfig.addr,
						  Station::COMMUNICATION)]);
}

void MainWindow::setDebugLevel(int state)
{
    if (state == Qt::Checked)
	debugLevel = 1;
    else
	debugLevel = 0;
}
