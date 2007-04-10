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
#include <string.h>  // for memcpy and memcmp

#include <iostream>
#include <sstream>

#include "common.hpp"

#include "lpdu.hpp"
#include "stats.hpp"

#include "dummy.hpp"
#include "_test_lpdu.hpp"


// the max string for a fragment hex representation
// is 2048 bytes * 3 + 1 for NULL.
#define MAX_STRING_LEN 6145

char dnpStrBuf1[ MAX_STRING_LEN];	   
char dnpStrBuf2[ MAX_STRING_LEN];	   

#define VERIFY(expr, ...)                                        \
{                                                                 \
    char buf[ MAX_STRING_LEN];                                    \
    sprintf(buf, ## __VA_ARGS__);                                 \
    QVERIFY2(expr, buf);					  \
}

#define lpduToArrayCmp(lpdu, ar)                                  \
{                                                                 \
    Bytes ba(ar, ar + sizeof(ar) / sizeof(uint8_t));              \
    VERIFY(lpdu.ab == ba, "\n%s != \n%s",                        \
            hex_repr(lpdu.ab, dnpStrBuf1, MAX_STRING_LEN),	  \
	    hex_repr(ba, dnpStrBuf2, MAX_STRING_LEN));	          \
}

#define TO_BYTES(a) (Bytes (a, a + sizeof(a) / sizeof(uint8_t)))

void TestLpdu::testBuildLpdu()
{

  /* application layer data for r1 */
  uint8_t a1[] = { 0xcd, 0xcc, 0x01, 0x3c, 0x02, 0x06, 0x3c, 0x03,
                         0x06, 0x3c, 0x04, 0x06 };

  /* application layer data for r2 */
  uint8_t a2[] = { 0xc1, 0xe3, 0x81, 0x96, 0x00, 0x02, 0x01, 0x28,
                         0x01, 0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x28,
                         0x01, 0x00, 0x01, 0x00, 0x01, 0x02, 0x01, 0x28,
                         0x01, 0x00, 0x02, 0x00, 0x01, 0x02, 0x01, 0x28,
                         0x01, 0x00, 0x03, 0x00, 0x01, 0x20, 0x02, 0x28,
                         0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x20,
                         0x02, 0x28, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
                         0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x03, 0x00,
                         0x00, 0x1e, 0x02, 0x01, 0x00, 0x00, 0x01, 0x00,
                         0x01, 0x00, 0x00, 0x01, 0x00, 0x00 };

  /* these are known good DNP LPDUs */
  uint8_t r0[] = { 0x05, 0x64, 0x05, 0xc0, 0x02, 0x00, 0x01, 0x00,
                         0x9e, 0x59 }; /* header only */

  uint8_t r1[] = { 0x05, 0x64, 0x11, 0xc4, 0x15, 0x00, 0x17, 0x00,
                         0x63, 0x62, 0xcd, 0xcc, 0x01, 0x3c, 0x02, 0x06,
                         0x3c, 0x03, 0x06, 0x3c, 0x04, 0x06, 0x08, 0x9e };
    
  uint8_t r2[] = { 0x05, 0x64, 0x53, 0x73, 0x00, 0x04, 0x01, 0x00,
                         0x03, 0xfc, 0xc1, 0xe3, 0x81, 0x96, 0x00, 0x02,
                         0x01, 0x28, 0x01, 0x00, 0x00, 0x00, 0x01, 0x02,
                         0x01, 0x28, 0x05, 0x24, 0x01, 0x00, 0x01, 0x00,
                         0x01, 0x02, 0x01, 0x28, 0x01, 0x00, 0x02, 0x00,
                         0x01, 0x02, 0x01, 0x28, 0xb4, 0x77, 0x01, 0x00,
                         0x03, 0x00, 0x01, 0x20, 0x02, 0x28, 0x01, 0x00,
                         0x00, 0x00, 0x01, 0x00, 0x00, 0x20, 0xa5, 0x25,
                         0x02, 0x28, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
                         0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x03, 0x00,
                         0x2f, 0xac, 0x00, 0x1e, 0x02, 0x01, 0x00, 0x00,
                         0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
                         0x16, 0xed };


  uint8_t twoLpdus[] = { 0x05, 0x64, 0x08, 0xc4, 0x02, 0x00, 0x2d, 0x00,
                         0xa4, 0x1c, 0xc0, 0xcd, 0x00, 0x23, 0x65, 0x05,
                         0x64, 0x0b, 0xc4, 0x02, 0x00, 0x2d, 0x00, 0xf4,
                         0x8f, 0xc0, 0xce, 0x01, 0x3c, 0x02, 0x06, 0x09,
                         0x7f };

  /* header only - no application data */
  uint8_t r3[] = { 0x05, 0x64, 0x05, 0xc0, 0x01, 0x00, 0x00, 0x04,
                         0xe9, 0x21 };

  /* header only - no application data */
  uint8_t r4[] = { 0x05, 0x64, 0x05, 0x00, 0x00, 0x04, 0x01, 0x00,
                         0x19, 0xa6 };

  uint8_t r5[] = { 0x05, 0x64, 0x0a, 0x44, 0x01, 0x00, 0x02, 0x00,
                   0xfa, 0x4a, 0xc1, 0xe1, 0x81, 0x10, 0x00, 0x6d,
                   0xd5 };

  /* known bad lpdu data */
  uint8_t r10[] ={ 108, 68, 3, 0, 17, 8, 127, 38, 193, 201 };
  uint8_t r11[] ={ 129, 144, 0, 20, 6, 0, 0, 8, 16, 0 };
  uint8_t r12[] ={ 0, 0, 16, 0, 48, 78, 16, 0, 7, 0 };
  uint8_t r13[] ={ 0, 0, 7, 0, 7, 0, 17, 0, 30, 4 };
  uint8_t r14[] ={ 0, 0, 156, 186, 34, 120, 0, 120, 0, 120 };
  uint8_t r15[] ={ 0, 120, 0, 48, 81, 66, 81, 69, 81, 61 };
  uint8_t r16[] ={ 76, 53, 81, 4, 0, 5, 0, 4, 0, 4 };
  uint8_t r20[] ={ 81, 48, 81, 48, 223, 36, 81, 4, 0, 5 };

  enum StatIndex { RX_START_OCTETS = 0,
		   RX_LPDUS,
		   LOST_BYTES,
		   CRC_ERRORS,
		   NUM_STATS };

  Stats::Element statElements[] =
  {
      { RX_START_OCTETS,       "Rx Start Octets",         Stats::NORMAL,0,0},
      { RX_LPDUS,              "Rx Lpdus",                Stats::NORMAL,0,0},
      { LOST_BYTES,            "Lost Bytes",            Stats::ABNORMAL,0,0},
      { CRC_ERRORS,            "CRC Errors",            Stats::ABNORMAL,0,0},
  };

  Stats stats;
  char name[7];
  int debugLevel = -1;
  DummyDb db;

  assert(sizeof(statElements)/sizeof(Stats::Element) == NUM_STATS);
  snprintf(name, Stats::MAX_USER_NAME_LEN, "DL TEST");
  stats = Stats( name, 1, &debugLevel, statElements, NUM_STATS, &db);

  Lpdu lpdu = Lpdu( &stats);

  unsigned int i;
  bool lpduFound;

  lpdu.build( 1, 1, 0, 0, 0, 2, 1);
  lpduToArrayCmp( lpdu, r0);

  lpdu.build( 1, 1, 0, 0, 4, 21, 23, TO_BYTES(a1));
  lpduToArrayCmp( lpdu, r1);

  lpdu.build( 0, 1, 1, 1, 3, 1024, 1, TO_BYTES(a2));
  lpduToArrayCmp( lpdu, r2);

  lpdu.reset();
  /* test building from incoming bytes */
  Bytes data1( r5, r5 + 10);
  lpduFound = lpdu.buildFromBytes( data1);
  QVERIFY( lpduFound == false);
  QVERIFY (lpdu.ab.size() == 10);

  Bytes data2( &r5[10], &r5[10] + sizeof(r5) - 10);
  lpduFound = lpdu.buildFromBytes( data2);
  QVERIFY( lpduFound == true);
  QVERIFY (lpdu.getStat(Lpdu::RX_START_OCTETS)    == 1);
  QVERIFY (lpdu.getStat(Lpdu::LOST_BYTES)         == 0);
  QVERIFY (lpdu.getStat(Lpdu::CRC_ERRORS)         == 0);

  Bytes data3(TO_BYTES(r3));
  lpduFound = lpdu.buildFromBytes( data3);
  QVERIFY( lpduFound == true);

  Bytes data4(TO_BYTES(r4));
  lpduFound = lpdu.buildFromBytes( data4);
  QVERIFY( lpduFound == true);

  /* create 4x a good lpdu */
  Bytes data5(TO_BYTES(r1));
  Bytes data6(data5);
  data6.insert(data6.end(), data5.begin(), data5.end());
  data6.insert(data6.end(), data5.begin(), data5.end());
  data6.insert(data6.end(), data5.begin(), data5.end());

  /* ensure we receive 4 good ones */
  for (i=0; i<4; i++)
  {
    lpdu.reset();
    lpduFound = lpdu.buildFromBytes( data6);
    QVERIFY( lpduFound == true);
    QVERIFY (data6.size() == (sizeof(r1)*(3-i)));
    QVERIFY (lpdu.getStat(Lpdu::RX_START_OCTETS) == (2+i));
    QVERIFY (lpdu.getStat(Lpdu::LOST_BYTES)      == 0);
    QVERIFY (lpdu.getStat(Lpdu::CRC_ERRORS)      == 0);
  }
 
  /* header only lpdu */
  lpdu.reset();
  Bytes data7(TO_BYTES(r0));
  lpduFound = lpdu.buildFromBytes( data7);
  QVERIFY( lpduFound == true);
  QVERIFY (lpdu.getStat(Lpdu::RX_START_OCTETS)   == 6);
  QVERIFY (lpdu.getStat(Lpdu::LOST_BYTES)        == 0);
  QVERIFY (lpdu.getStat(Lpdu::CRC_ERRORS)        == 0);

  /* building of two lpdus coming in across two simulated reads */
  /* with the break on the last byte of the last CRC            */
  lpdu.reset();
  Bytes data8(TO_BYTES(twoLpdus));
  Bytes data9;
  data9.push_back(data8.back());
  data8.pop_back();

  lpduFound = lpdu.buildFromBytes( data8);
  QVERIFY( lpduFound == true);
  QVERIFY (lpdu.ab.size() == 15);  /* manually counted */
  QVERIFY (lpdu.getStat(Lpdu::RX_START_OCTETS)   == 7);
  QVERIFY (lpdu.getStat(Lpdu::LOST_BYTES)        == 0);
  QVERIFY (lpdu.getStat(Lpdu::CRC_ERRORS)        == 0);
  QVERIFY( data8.size() == 17);

  lpdu.reset(); 

  lpduFound = lpdu.buildFromBytes( data8);
  QVERIFY( lpduFound == false);
  QVERIFY (lpdu.ab.size() == 17);
  QVERIFY (lpdu.getStat(Lpdu::RX_START_OCTETS)   == 8);
  QVERIFY (lpdu.getStat(Lpdu::LOST_BYTES)        == 0);
  QVERIFY (lpdu.getStat(Lpdu::CRC_ERRORS)        == 0);

  lpduFound = lpdu.buildFromBytes( data9);
  QVERIFY( lpduFound == true);
  QVERIFY (lpdu.getStat(Lpdu::RX_START_OCTETS)   == 8);
  QVERIFY (lpdu.getStat(Lpdu::LOST_BYTES)        == 0);
  QVERIFY (lpdu.getStat(Lpdu::CRC_ERRORS)        == 0);

  lpdu.reset();

  /* test known bad cases */
  Bytes data10(TO_BYTES(r10));
  lpduFound = lpdu.buildFromBytes( data10);
  QVERIFY( lpduFound == false);
  QVERIFY (lpdu.ab.size() == 0);

  lpdu.reset();
  Bytes data11(TO_BYTES(r11));
  lpduFound = lpdu.buildFromBytes( data11);
  QVERIFY( lpduFound == false);
  QVERIFY (lpdu.ab.size() == 0);

  lpdu.reset();
  Bytes data12(TO_BYTES(r12));
  lpduFound = lpdu.buildFromBytes( data12);
  QVERIFY( lpduFound == false);
  QVERIFY (lpdu.ab.size() == 0);

  lpdu.reset();
  Bytes data13(TO_BYTES(r13));
  lpduFound = lpdu.buildFromBytes( data13);
  QVERIFY( lpduFound == false);
  QVERIFY (lpdu.ab.size() == 0);

  lpdu.reset();
  Bytes data14(TO_BYTES(r14));
  lpduFound = lpdu.buildFromBytes( data14);
  QVERIFY( lpduFound == false);
  QVERIFY (lpdu.ab.size() == 0);

  lpdu.reset();
  Bytes data15(TO_BYTES(r15));
  lpduFound = lpdu.buildFromBytes( data15);
  QVERIFY( lpduFound == false);
  QVERIFY (lpdu.ab.size() == 0);

  lpdu.reset();
  Bytes data16(TO_BYTES(r16));
  lpduFound = lpdu.buildFromBytes( data16);
  QVERIFY( lpduFound == false);
  QVERIFY (lpdu.ab.size() == 0);

  lpdu.reset();
  Bytes data20(TO_BYTES(r20));
  lpduFound = lpdu.buildFromBytes( data20);
  QVERIFY( lpduFound == false);
  QVERIFY (lpdu.ab.size() == 0);
}

// QTEST_APPLESS_MAIN(TestLpdu) for standalone tests
