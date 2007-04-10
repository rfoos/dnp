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

#ifndef DNP_ENDPOINT_H
#define DNP_ENDPOINT_H

#include <map>
#include <QtNetwork>
#include <QHostAddress>
#include "common.hpp"
#include "stats.hpp"
#include "lpdu.hpp"
#include "transmit_interface.hpp"

typedef struct {

    unsigned short portOfLastRequest;   // only used for UDP
    uint16_t       port;                // for tx
    QHostAddress   ip;
    Uptime_t       timeOfLastRx;

} RemoteDevice;


class Endpoint : public QObject, public TransmitInterface
{
    Q_OBJECT

public:

    // the key for deviceInfoMap is the remote IP - this is to
    // to comply with 4.3.5.1 Connection Establishment Method 1
    // of the DNP3 IP Networking Specification

    typedef struct
    {
	DnpAddr_t                             ownerDnpAddr;
	bool                                  tcp;
	bool                                  initiating;
	unsigned short                        listenPort;
	std::map<DnpAddr_t, RemoteDevice>     deviceMap;
	int*                                  debugLevel_p;

    } EndpointConfig;

    Endpoint(const EndpointConfig& config, EventInterface* eventInterface_p);

    // implement the transmit interface
    Uptime_t transmit( const Lpdu& lpdu);

    ~Endpoint();

signals:
    void data( Bytes* buf, unsigned long timeRxd);

private slots:
    void readDatagrams();

private:

    enum statIndex {       RX_UDP_MULTICAST,
			   RX_UDP_PACKET,
			   TX_UDP_PACKET,
			   RX_UNKNOWN_IP,
			   NUM_STATS       };

    QUdpSocket         udpSocket;
    bool               tcp;
    unsigned short     listenPort;

    // dnp address is the key - used by tx
    std::map<DnpAddr_t, RemoteDevice> deviceMap;

    // ip address is the key - used by rx
    std::map<uint32_t, RemoteDevice> deviceIpMap;

    Stats              stats;
    Stats::Element     statElements[NUM_STATS];
    char               strbuf[Stats::MAX_LOG_LEN];

};

#endif //DNP_ENDPOINT_H
