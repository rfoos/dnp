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

#ifndef DISPLAY_H
#define DISPLAY_H

#include <QString>
#include <QWidget>
#include <QDateTime>
#include "common.hpp"
#include "event_interface.hpp"

class QGroupBox;
class QTreeView;
class QAbstractItemModel;

class DataDisplay : public QWidget
{
    Q_OBJECT
public:
    DataDisplay();

    // returns true if it is a Change Of State (COS)
    virtual bool updateData( QString                      name,
			     EventInterface::PointType_t  pointType,
			     int                          value,
			     DnpTime_t                    timestamp=0);

    virtual void createDataModel();
protected:

    virtual void addRow(     QString                      name,
			     EventInterface::PointType_t  pointType,
			     int                          value,
			     DnpTime_t                    timestamp=0);

    void addParent( EventInterface::PointType_t pointType);


    QAbstractItemModel    *dataModel;
    QGroupBox             *dataGroupBox;
    QTreeView             *dataView;

    static const QString ptNames[EventInterface::NUM_POINT_TYPES];
};

class CosDisplay : public DataDisplay
{
    Q_OBJECT

public:
    CosDisplay();

public:
    virtual void createDataModel();

    // return value shuold not be used
    // it is the callers responsibility not to call this method if it
    // it not an event
    bool updateData( QString name,
		     EventInterface::PointType_t pointType,
		     int value,
		     DnpTime_t timestamp=0);

private:

    void addRow( QString name,
		 EventInterface::PointType_t,
		 int value,
		 DnpTime_t timestamp=0);


    QDateTime  dateTime;

};

#endif
