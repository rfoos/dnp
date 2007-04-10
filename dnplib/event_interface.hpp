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

// abstract class that defines the interface for an event interface

#ifndef EVENT_INTERFACE_H
#define EVENT_INTERFACE_H

#include "common.hpp"

// All data points known to DNP can be uniquely identified using the
// dnp address of the source device, the dnp index, and the type.

class EventInterface
{

public:

    enum PointType_t { AI = 0,   // Analog Input
		       BI,       // Binary Input
		       CI,       // Couter Inout
		       AO,       // Analog Output
		       BO,       // Binary Ouput
		       ST,
		       AP_AB_ST,
		       AP_NM_ST,
		       DL_AB_ST,
		       DL_NM_ST,
		       SA_AB_ST,
		       SA_NM_ST,
		       EP_AB_ST,
		       EP_NM_ST,
		       NUM_POINT_TYPES 
  };     // Statistic
 
    virtual void changePoint(    DnpAddr_t      addr,
			         DnpIndex_t     index,
			         PointType_t    pointType,
			         int            value,
			         DnpTime_t      timestamp=0 )=0;

    // use this method if you want the name of a unique dnp point
    // to be something other than the default
    // name must be NULL terminated
    // 
    // value is optional, if the point is new it will be initialize
    // to the value provided
    virtual void registerName(   DnpAddr_t      addr,
				 DnpIndex_t     index,
				 PointType_t    pointType,
				 char*          name,
                                 int            initValue=0  )=0;

    virtual ~EventInterface();
};

#endif
