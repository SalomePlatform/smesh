//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_PatternWidget.h
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#ifndef WIDGET_PATTERN_H
#define WIDGET_PATTERN_H

#include <qframe.h>
#include <qvaluevector.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

typedef QValueVector<SMESH::PointStruct> PointVector;
typedef QValueVector< QValueVector<int> >  ConnectivityVector;

class QPoint;

//=================================================================================
// class    : SMESHGUI_PatternWidget
// purpose  :
//=================================================================================
class SMESHGUI_PatternWidget : public QFrame
{ 
    Q_OBJECT

public:
    SMESHGUI_PatternWidget( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );

    ~SMESHGUI_PatternWidget();

    void SetPoints( PointVector thePoints,  QValueVector<int> theKeys, ConnectivityVector theConnections );

private :
    PointVector myPoints;
    QValueVector<int> myKeys;
    ConnectivityVector myConnections;

    double myMinU, myMaxU, myMinV, myMaxV;
      
private slots:

protected:
    void paintEvent( QPaintEvent * );
    QPoint MapCoords( const double u, const double v );
};

#endif // WIDGET_PATTERN_H

