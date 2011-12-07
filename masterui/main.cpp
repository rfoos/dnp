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

#include <QtGui>
#include <QThread>
#include "main_window.hpp"
#include "_test_lpdu.hpp"
#include "_test_master.hpp"
#include "_test_security.hpp"


int main(int argc, char *argv[])
{
    QApplication   app(argc, argv);

    // execute all unit tests
    TestLpdu*       testLpdu      = new TestLpdu();
    QTest::qExec(testLpdu);
    delete testLpdu;

    TestMaster*     testMaster    = new TestMaster();
    QTest::qExec(testMaster);
    delete testMaster;

    TestSecurity*   testSecurity  = new TestSecurity();
    QTest::qExec(testSecurity);
    delete testSecurity;

    MainWindow mainWindow;
    mainWindow.show();

    app.exec();

    return 1;

}
