///
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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

class QAction;
class QMenu;
class QVBoxLayout;
class QLabel;

#include "endpoint.hpp"
#include "event_interface.hpp"
#include "timer_widget.hpp"
#include "station.hpp"
#include "master.hpp"
#include "display.hpp"
#include "settings.hpp"
#include "outstation_window.hpp"

class OutstationThread : public QThread
{
  public:
     void run();
     OutstationWindow* mainWindow;
};

class MainWindow : public QMainWindow, public EventInterface
{
    Q_OBJECT
public:
    MainWindow();

    // implementation of EventInterface
    void changePoint(        DnpAddr_t addr, DnpIndex_t index,
			     PointType_t    pointType,
			     int value, DnpTime_t timestamp=0);

    void registerName(       DnpAddr_t      addr,
			     DnpIndex_t index,
			     EventInterface::PointType_t    pointType,
			     char*          name,
			     int initialValue );

private slots:

    void about();
    void license();
    void rxData(Bytes*, unsigned long timeRxd=0);
    void setDebugLevel(int);
    void responseTimeout();
    void updateStateLabel();

    // opens an outstation main window in a separate thread
    void startOutstation();

private:

    void closeEvent(QCloseEvent *event);
    void writeSettings();
    void readSettings();

    void createDnpMaster();
    void createStatDisplay();
    void createPointDisplay();
    void createCosDisplay();
    void createActions();
    void createMenus();
    void createStatusBar();


    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *outstationAct;

    Settings*           settingsTab;
    DataDisplay*        statDisplay;
    DataDisplay*        pointDisplay;
    CosDisplay*         cosDisplay;

    QLabel*              stateLabel;

    QLabel*              commsLabel;
    static const QString commsStrings[2];

    QHash<QString, QString> pointNameHash;
    QString convertDnpIndexToName(DnpAddr_t id, DnpIndex_t dnpIndex,
				  PointType_t  pointType);
    static const QString shortPtNames[NUM_POINT_TYPES];

    // master stuff
    TimersGroup ti;
    Master* m_p;
    RemoteDevice remoteDevice;
    std::map<unsigned short, RemoteDevice>  deviceMap;
    Endpoint* ep_p; 
    int debugLevel;
    int respTimeout;
    int integrityPollInterval;

    Master::MasterConfig          masterConfig;
    Station::StationConfig        stationConfig;
    Datalink::DatalinkConfig      datalinkConfig;
    Endpoint::EndpointConfig      endpointConfig;


    OutstationThread outstationThread;
};

#endif
