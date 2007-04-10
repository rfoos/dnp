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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include "master.hpp"
#include "timer_widget.hpp"

class QGroupBox;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QVBoxLayout;
class QFrame;
class QLineEdit;

class Settings : public QFrame
{
    Q_OBJECT

  public:
    Settings(Master* m_p, TimersGroup* t_p, QWidget* parent, DnpAddr_t addr);

    QCheckBox*     verbose;
    QCheckBox*     secAuth;

  signals:
    void masterStateChange();

  public slots:
    void allowRequests(bool allow=true);

  private slots:
    void mode( bool singleShotMode);
    void singleShotRequest();
    void continuousRequest(bool on);
    void delayTimerTimeout();
    void setDelay();
    void setSecAuth(bool on);
    void trip();
    void close();

  private:
    void createOperationGroup();
    void createControlGroup();
    void createSecureAuthGroup( QLayout* layout);
    void createLogGroup( QLayout* layout);
    void control( ControlOutputRelayBlock::Code code);

    QVBoxLayout*   settingsGroupLayout;
    QGroupBox*     operationGroup;
    QPushButton*   pollButton;   // if single shot, disabled when state != idle
    QRadioButton*  singleShot;   
    QGroupBox*     controlGroup; // if single shot, disabled when state != idle
    QLineEdit*     controlEdit;
    QGroupBox*     secAuthGroup; 
    QLineEdit*     delayEdit;

    QTimer*        delayTimer;

    Master*        master_p;
    TimersGroup*   timers_p;

    // address for the outstation we are doing the settings on
    DnpAddr_t outstationAddr;

    bool requestsAllowed;
};

class ConfigurationFrame : public QFrame
{
    Q_OBJECT

  public:
    ConfigurationFrame();
};

class ConfigItem : public QObject
{
    Q_OBJECT

  public:
    ConfigItem(ConfigurationFrame* cf, QString name, QString name);
    QLineEdit* itemEdit;
    QLabel*    itemLabel;
  private slots:
    void setItem();
  private:
    ConfigurationFrame* parent;
    static bool warningHasBeenDisplayed;
};


#endif
