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
#include <iostream>
#include <assert.h>
#include "display.hpp"

const QString DataDisplay::ptNames[EventInterface::NUM_POINT_TYPES] = 
  { "Analog Inputs",
    "Binary Inputs",
    "Counter Inputs",
    "Analog Outputs",
    "Binary Outputs",
    "Generic Stat",
    "Application Abnormal",
    "Application Normal",
    "Datalink Abormal",
    "Datalink Normal",
    "Secure Auth Abnormal",
    "Secure Auth Normal",
    "End Point Abnormal",
    "End Point Normal" };

DataDisplay::DataDisplay()
{
    dataGroupBox = new QGroupBox(); 

    dataView = new QTreeView;
    dataView->setRootIsDecorated(true);
    dataView->setAlternatingRowColors(true);

    QHBoxLayout *dataLayout = new QHBoxLayout;
    dataLayout->addWidget(dataView);
    dataGroupBox->setLayout(dataLayout);

    QVBoxLayout *dataDisplayLayout = new QVBoxLayout;
    dataDisplayLayout->setDirection(QBoxLayout::LeftToRight);
    dataDisplayLayout->addWidget(dataView);
    setLayout(dataDisplayLayout);

    QFont font;
    font.setPointSize(8);
    // font.setFamily("Courier");
    // font.setBold(true);
    setFont(font);

    createDataModel();
}

void DataDisplay::addRow( QString name, EventInterface::PointType_t pointType,
			  int value, DnpTime_t timestamp)
{
    QModelIndexList r;
    QModelIndex p; // for parent

    r = dataModel->match( dataModel->index(0, 0), Qt::DisplayRole,
			  ptNames[pointType]);
    p = r.first();

    dataModel->insertRow(0, p);
    //printf("r=%d c=%d\n", dataModel->rowCount(p), dataModel->columnCount(p));
    dataModel->setData(dataModel->index(0, 0, p), name, Qt::DisplayRole );
    dataModel->setData(dataModel->index(0, 1, p), value, Qt::DisplayRole );

    dataView->resizeColumnToContents(0);
}

void DataDisplay::addParent(  EventInterface::PointType_t pointType )
{
    QString parentName = ptNames[pointType];
    QModelIndex p; // for parent
    dataModel->insertRow(0);
    p = dataModel->index(0, 0);
    dataModel->setData(p, parentName, Qt::DisplayRole );
    dataModel->insertColumns(0, 2, p);
    // dataView->expand( p);
    //printf("r=%d c=%d\n", dataModel->rowCount(p), dataModel->columnCount(p));

}

bool DataDisplay::updateData( QString name,
			      EventInterface::PointType_t pointType,
			      int value, DnpTime_t timestamp)
{
    bool cos = false;
    QModelIndexList r;
    QModelIndex p; // for parent

    // find the parent using the point type name
    r = dataModel->match( dataModel->index(0, 0), Qt::DisplayRole,
			  ptNames[pointType]);
    if (r.isEmpty())
    {
	addParent( pointType);
	r = dataModel->match( dataModel->index(0, 0), Qt::DisplayRole,
			      ptNames[pointType]);
	assert(!r.isEmpty());
    }
    p = r.first();
    // find the row using the id
    r = dataModel->match(dataModel->index(0, 0, p), Qt::DisplayRole, name);
    if (r.isEmpty())
    {
	addRow( name, pointType, value, timestamp);
	cos = true;
    }
    else
    {
	QModelIndex di = dataModel->index(r.first().row(), 1, p);
	if (dataModel->data(di) != value)
	{
	    dataModel->setData(di, value, Qt::DisplayRole);
	    cos = true;
	}
    }
    return cos;
}

void DataDisplay::createDataModel()
{
    dataModel = new QStandardItemModel(0, 2, this);
    dataModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
    dataModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Value"));
    dataView->setModel( dataModel);
}

CosDisplay::CosDisplay()
{
    createDataModel();
}

bool CosDisplay::updateData( QString name,
			     EventInterface::PointType_t pointType,
			     int value,
			     DnpTime_t timestamp)
{
    addRow( name, pointType, value, timestamp);
    return true;
}

void CosDisplay::addRow( QString name, EventInterface::PointType_t pointType,
			  int value, DnpTime_t timestamp)
{

    QModelIndexList r;
    QModelIndex p; // for parent

    dataModel->insertRow(0);
    dataModel->setData(dataModel->index(0, 0), name, Qt::DisplayRole );
    dataModel->setData(dataModel->index(0, 1), value, Qt::DisplayRole );
    if (timestamp == 0)
	dateTime = QDateTime::currentDateTime();
    else
	dateTime.setTime_t( (unsigned int) ( timestamp/1000));
    dataModel->setData(dataModel->index(0, 2),
		       dateTime.toString("ddd MMM dd yyyy  hh:mm:ss:zzz"),
		       Qt::DisplayRole );

    dataView->resizeColumnToContents(0);
}

void CosDisplay::createDataModel()
{
    dataModel = new QStandardItemModel(0, 3, this);
    // the dataModel created from the base class is still there but not used
    // it should probably be deleted
    dataModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
    dataModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Value"));
    dataModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Time Stamp"));
    dataView->setModel( dataModel);
}
