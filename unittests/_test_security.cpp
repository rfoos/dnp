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

#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <sstream>

#include "common.hpp"
#include "master.hpp"
#include "outstation.hpp"
#include "station.hpp"
#include "datalink.hpp"
#include "event_interface.hpp"
#include "transmit_interface.hpp"
#include "dummy.hpp"
#include "_test_security.hpp"

void TestSecurity::testMaster()
{
    Master* m_p;
    Outstation* o_p;
    DummyDb db;
    int debugLevel = -1;
    DummyTx masterTx(&debugLevel, 'M', 'S');
    DummyTx outstationTx(&debugLevel, 'O', 'S');
    DummyTimer masterTimer;
    DummyTimer outstationTimer;
    int integrityPollInterval = 10;

    Master::MasterConfig          masterConfig;
    Datalink::DatalinkConfig      datalinkConfig;
    Station::StationConfig        stationConfig;

    masterConfig.addr = 1;
    masterConfig.consecutiveTimeoutsForCommsFail = 3;
    masterConfig.userNum        = 5;
    masterConfig.integrityPollInterval_p = &integrityPollInterval;
    masterConfig.debugLevel_p = &debugLevel;

    stationConfig.addr = 2;
    stationConfig.debugLevel_p = &debugLevel;

    datalinkConfig.addr                  = masterConfig.addr;
    datalinkConfig.isMaster              = 1;
    datalinkConfig.keepAliveInterval_ms  = 10000;
    datalinkConfig.tx_p                  = &masterTx;
    datalinkConfig.debugLevel_p          = &debugLevel;

    m_p = new Master (masterConfig, datalinkConfig, &stationConfig, 1,
                      &db, &masterTimer);

    Outstation::OutstationConfig outstationConfig;
    outstationConfig.addr           = stationConfig.addr;
    outstationConfig.masterAddr     = masterConfig.addr;
    outstationConfig.userNum        = 5;
    outstationConfig.debugLevel_p   = &debugLevel;

    datalinkConfig.addr             = stationConfig.addr;
    datalinkConfig.isMaster         = 0;
    datalinkConfig.tx_p             = &outstationTx;

    o_p = new Outstation( outstationConfig, datalinkConfig, &db,
                          &outstationTimer);

    // check initial states
    QCOMPARE(m_p->getSecAuthStat(2, SecureAuthentication::STATE), 
             (DnpStat_t)  SecureAuthentication::INIT);
    QCOMPARE(m_p->getSecAuthStat(2,MasterSecurity::TX_KEY_STATUS_REQUEST),
             (DnpStat_t)  0);

    QCOMPARE(o_p->getSecAuthStat( SecureAuthentication::STATE), 
             (DnpStat_t)  OutstationSecurity::WAIT_FOR_KEY_CHANGE);    
    o_p->enableSecureAuthentication();

    // init master
    m_p->enableSecureAuthentication(2);
    QCOMPARE(m_p->getSecAuthStat(2,SecureAuthentication::STATE),
             (DnpStat_t)  MasterSecurity::WAIT_FOR_KEY_STATUS);
    QCOMPARE(m_p->getSecAuthStat(2,MasterSecurity::TX_KEY_STATUS_REQUEST),
             (DnpStat_t) 1);
    QVERIFY(masterTimer.isActive(TimerInterface::RESPONSE));

    o_p->rxData(&masterTx.lastTxBytes);
    QCOMPARE(o_p->getSecAuthStat( OutstationSecurity::TX_KEY_STATUS),
             (DnpStat_t) 1);    
    QCOMPARE(o_p->getSecAuthStat( SecureAuthentication::STATE), 
             (DnpStat_t)  OutstationSecurity::WAIT_FOR_KEY_CHANGE);    

    m_p->rxData(&outstationTx.lastTxBytes);
    QCOMPARE(m_p->getSecAuthStat(2, MasterSecurity::RX_KEY_STATUS_NOT_OK),
             (DnpStat_t) 1);    
    QCOMPARE(m_p->getSecAuthStat(2, MasterSecurity::TX_KEY_CHANGE_MSG),
             (DnpStat_t) 1);    
    QCOMPARE(m_p->getSecAuthStat(2,SecureAuthentication::STATE),
             (DnpStat_t)  MasterSecurity::WAIT_FOR_KEY_CONFIRMATION);

    o_p->rxData(&masterTx.lastTxBytes);
    QCOMPARE(o_p->getSecAuthStat( OutstationSecurity::RX_VALID_KEY_CHANGE),
             (DnpStat_t) 1);
    QCOMPARE(o_p->getSecAuthStat( SecureAuthentication::STATE), 
             (DnpStat_t)  SecureAuthentication::OUTSTATION_IDLE);

    m_p->rxData(&outstationTx.lastTxBytes);
    QCOMPARE(m_p->getSecAuthStat( 2, MasterSecurity::RX_KEY_STATUS_OK),
             (DnpStat_t) 1);
    QCOMPARE(m_p->getSecAuthStat( 2, SecureAuthentication::STATE), 
             (DnpStat_t)  SecureAuthentication::MASTER_IDLE);
    QVERIFY(masterTimer.isActive(TimerInterface::KEY_CHANGE));

    // do a normal non-critical poll ( which will be interpetted as critical
    // because it is the first poll after a key change)
    DnpStat_t numRepsonses = m_p->getStat(2, Station::RX_RESPONSE);
    m_p->poll( Master::INTEGRITY);
    o_p->rxData(&masterTx.lastTxBytes);
    m_p->rxData(&outstationTx.lastTxBytes);
    QCOMPARE (m_p->getStat(2, Station::RX_RESPONSE),
              (DnpStat_t) numRepsonses);
    QCOMPARE(m_p->getSecAuthStat(2,SecureAuthentication::RX_CHALLENGE_MSG),
             (DnpStat_t)1);
    QCOMPARE(m_p->getSecAuthStat(2,SecureAuthentication::TX_AUTH_RESPONSE),
             (DnpStat_t)1);
    o_p->rxData(&masterTx.lastTxBytes);
    m_p->rxData(&outstationTx.lastTxBytes);

    // send a critical asdu (write) which should generate a challenge
//     m_p->clearRestartBit();
//     o_p->rxData(&masterTx.lastTxBytes);
//     m_p->rxData(&outstationTx.lastTxBytes);
//     QCOMPARE(m_p->getSecAuthStat(2,SecureAuthentication::RX_CHALLENGE_MSG),
//           (DnpStat_t)1);


}

