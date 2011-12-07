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

#include <algorithm>
#include <assert.h>
#include "endpoint.hpp"

Endpoint::Endpoint(const EndpointConfig& config,
                   EventInterface* eventInterface_p)
  : QObject(), udpSocket(this), tcp(config.tcp),
    listenPort( config.listenPort), deviceMap( config.deviceMap)
{
    qRegisterMetaType<Bytes>("Bytes");

    udpSocket.bind(listenPort);

    // create a device map with IP as the key
    std::map<DnpAddr_t, RemoteDevice>::iterator iter;
    for (iter = deviceMap.begin(); iter != deviceMap.end(); iter++)
        deviceIpMap[iter->second.ip.toIPv4Address()] = iter->second;

    char name[Stats::MAX_USER_NAME_LEN];

    // init stats object
    Stats::Element   temp[] =
    {
        { RX_UDP_MULTICAST,      "Rx UDP Multicast"     , Stats::NORMAL  ,0,0},
        { RX_UDP_PACKET,         "Rx UDP Packet"        , Stats::NORMAL  ,0,0},
        { TX_UDP_PACKET,         "Tx UDP Packet"        , Stats::NORMAL  ,0,0},
        { RX_UNKNOWN_IP,         "Rx Unknown IP"        , Stats::ABNORMAL,0,0},
    };
    assert(sizeof(temp)/sizeof(Stats::Element) == NUM_STATS);
    memcpy(statElements, temp, sizeof(temp));
    snprintf(name, Stats::MAX_USER_NAME_LEN, "EP %6d ", config.ownerDnpAddr);
    stats = Stats( name, config.ownerDnpAddr,
                   config.debugLevel_p, statElements, NUM_STATS,
                   eventInterface_p,
                   EventInterface::EP_AB_ST );

    connect(&udpSocket, SIGNAL(readyRead()), this, SLOT(readDatagrams()));
}

Endpoint::~Endpoint()
{
}


Uptime_t Endpoint::transmit( const Lpdu& lpdu)
{
    Uptime_t       timeSent =0;
    RemoteDevice   device;

    char data[Lpdu::MAX_LEN];

    copy(lpdu.ab.begin(), lpdu.ab.end(), data);


    device = deviceMap[lpdu.getDest()];

    udpSocket.writeDatagram( data, lpdu.ab.size(), device.ip, device.port);

    timeSent = 0;
    stats.increment(TX_UDP_PACKET);
    stats.logNormal( "Tx %s",hex_repr( lpdu.ab, strbuf,sizeof(strbuf)));
    return timeSent;
}

// this method works as long as the receiver(s) of the data are on the same
// thread and the signal/slot connection is a direct connection. Meaning we
// will not return from the emit call until the datagram has been processed
// If the signal/slot connection is queued the datagram will likely be
// out of scope by the time the signal is processed.
void Endpoint::readDatagrams()
{
    uint16_t       port;
    QHostAddress   ip;
    RemoteDevice   device;

    while(udpSocket.hasPendingDatagrams())
    {
        QByteArray         datagram;
        Uptime_t           timeRxd = 0;
        int len = udpSocket.pendingDatagramSize();
        datagram.resize( len);

        udpSocket.readDatagram( datagram.data(), len, &ip, &port);

        if (deviceIpMap.count(ip.toIPv4Address()) > 0)
        {
            device = deviceIpMap[ip.toIPv4Address()];
            device.portOfLastRequest = port;

            const char* data_p = datagram.data();

            // put the char data into a Bytes container
            Bytes bytes((unsigned char*) data_p, (unsigned char*)data_p+len);
            
            stats.logNormal( "Rx %s",hex_repr(bytes, strbuf,sizeof(strbuf)));
            
            stats.increment(RX_UDP_PACKET);

            emit data( &bytes, timeRxd);
        }
        else
        {
            stats.logAbnormal( 0, "Rx UDP packet from unconfigured ip: %s",
                               ip.toString().toLocal8Bit().data());
            stats.increment(RX_UNKNOWN_IP);
        }

    }
}
