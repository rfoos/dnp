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
#include "station.hpp"
#include "datalink.hpp"
#include "event_interface.hpp"
#include "transmit_interface.hpp"
#include "dummy.hpp"
#include "_test_master.hpp"

void TestMaster::testIntegrityPoll()
{
    Master* master_p;
    DummyDb db;
    DummyTimer timer;
    int debugLevel = -1;
    DummyTx tx(&debugLevel, 'M', 'S');
    int integrityPollInterval = 10;

    Master::MasterConfig          masterConfig;
    Datalink::DatalinkConfig      datalinkConfig;
    Station::StationConfig        stationConfig;

    masterConfig.addr = 1;
    masterConfig.consecutiveTimeoutsForCommsFail = 3;
    masterConfig.integrityPollInterval_p = &integrityPollInterval;
    masterConfig.debugLevel_p = &debugLevel;

    stationConfig.addr = 2;
    stationConfig.debugLevel_p = &debugLevel;

    datalinkConfig.addr                  = masterConfig.addr;
    datalinkConfig.isMaster              = 1;
    datalinkConfig.keepAliveInterval_ms  = 10000;
    datalinkConfig.tx_p                  = &tx;
    datalinkConfig.debugLevel_p          = &debugLevel;

    master_p = new Master (masterConfig, datalinkConfig, &stationConfig, 1,
                           &db, &timer);

    // full resp to an integrity poll
    unsigned char r[] ={ 0x05, 0x64, 0x10, 0x44, 0x01, 0x00, 0x02, 0x00,
                         0x50, 0x8e, 0xc0, 0xc5, 0x81, 0x80, 0x00, 0x01,
                         0x02, 0x00, 0x00, 0x00, 0xff, 0xde, 0xa7 };


    // first test the repsonse without sending the poll
    Bytes bytes1(r, r+sizeof(r));
    master_p->rxData( &bytes1, 0);
    QVERIFY (master_p->getStat(2, TransportStats::RX_SEGMENT)           == 1);
    QVERIFY (master_p->getStat(2, TransportStats::RX_FRAGMENT)          == 1);
    QVERIFY (master_p->getStat(2, Station       ::RX_UNEXP_RESPONSE)    == 1);

    // send the poll
    master_p->poll( Master::INTEGRITY);
    QVERIFY (master_p->getStat(2, TransportStats::TX_SEGMENT)           == 1);
    QVERIFY (master_p->getStat(2, TransportStats::TX_FRAGMENT)          == 1);
    QVERIFY (master_p->getStat(2, Station       ::TX_INTEGRITY_POLL)    == 1);
    QVERIFY (master_p->getStat(2, Station       ::TX_READ_REQUEST)      == 1);

    // send a response to the poll
    Bytes bytes2(r, r+sizeof(r));
    master_p->rxData( &bytes2, 0);
    QVERIFY (master_p->getStat(2, TransportStats::RX_SEGMENT)           == 2);
    QVERIFY (master_p->getStat(2, TransportStats::RX_FRAGMENT)          == 2);
    QVERIFY (master_p->getStat(2, Station       ::RX_RESPONSE)          == 1);

}

