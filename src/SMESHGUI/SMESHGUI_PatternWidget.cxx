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
//  File   : SMESHGUI_PatternWidget.cxx
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#include "SMESHGUI_PatternWidget.h"

//Qt includes
#include <qpainter.h>
#include <qpoint.h>


//=================================================================================
// class    : SMESHGUI_PatternWidget()
// purpose  : 
//=================================================================================
SMESHGUI_PatternWidget::SMESHGUI_PatternWidget( QWidget* parent, const char* name, WFlags fl )
                                  : QFrame( parent, name, WStyle_Customize)
{
  myMinU =  myMinV =  myMaxU =  myMaxV = 0;
  setMinimumHeight(150);
  repaint();
}



//=================================================================================
// function : ~SMESHGUI_PatternWidget()
// purpose  : 
//=================================================================================
SMESHGUI_PatternWidget::~SMESHGUI_PatternWidget()
{
}


//=================================================================================
// function : SMESHGUI_PatternWidget::SetPoints
// purpose  : 
//=================================================================================
void SMESHGUI_PatternWidget::SetPoints( PointVector thePoints,  QValueVector<int> theKeys, ConnectivityVector theConnections )
{
  myPoints = thePoints;
  myKeys = theKeys;
  myConnections  = theConnections;
  
  if (!thePoints.size())
    return;
  
  myMinU = myMaxU = (thePoints[0]).x;
  myMinV = myMaxV = (thePoints[0]).y;
  double x, y;
  
  for (int i = 1; i < thePoints.size(); i++)
    {
      x = (thePoints[i]).x;
      y = (thePoints[i]).y;
      
      if ( myMinU > x )
	myMinU = x;
      if ( myMaxU < x)
	myMaxU = x;
      if ( myMinV > y )
	myMinV = y;
      if ( myMaxV < y)
	myMaxV = y;
    }
  
  repaint();
}

static const int Shift  = 4; // shift of the point number from point
static const int Border = 20;

//=================================================================================
// function : SMESHGUI_PatternWidget::paintEvent
// purpose  : 
//=================================================================================
void SMESHGUI_PatternWidget::paintEvent( QPaintEvent * )
{
  QPainter paint( this );
  paint.setBrush (Qt::SolidPattern ); 

  //Draw points
  const int aRadius = 3; // radius of a point
  
  for (int i = 0; i < myKeys.size() && i < myPoints.size(); i++)
    {
      SMESH::PointStruct aPoint = myPoints[ myKeys[i] ];
      QPoint aQPnt = MapCoords( aPoint.x, aPoint.y);
      
      paint.drawPie( aQPnt.x() - aRadius, aQPnt.y() - aRadius, aRadius*2, aRadius*2, 5760, 5760 );
      paint.drawText( aQPnt.x() +  Shift, aQPnt.y() -   Shift, QString::number( i+1 ) );
    }
  
  //Draw lines
  for (int i = 0; i < myConnections.size(); i++)
    {
      QValueVector<int> aCVector = myConnections[i];

      if ( aCVector.size() == 0 )
        continue;
      
      SMESH::PointStruct aPoint = myPoints[ aCVector[0] ];
      const QPoint aBeginPnt = MapCoords( aPoint.x, aPoint.y);
      QPoint aFirstPnt = aBeginPnt, aSecondPnt;  
      
      for (int j = 1; j < aCVector.size(); j++)
	{
	  aPoint = myPoints[ aCVector[j] ];
	  aSecondPnt = MapCoords( aPoint.x, aPoint.y);
	  paint.drawLine(aFirstPnt, aSecondPnt);
	  aFirstPnt = aSecondPnt;
	}
      
      paint.drawLine(aBeginPnt, aSecondPnt);
    }
}


//=================================================================================
// function : SMESHGUI_PatternWidget::MapCoords
// purpose  : 
//=================================================================================
QPoint SMESHGUI_PatternWidget::MapCoords( const double u, const double v )
{
  int aWidth  = width()  - 2*Border;
  int aHeight = height() - 2*Border;

  double aUBound = myMaxU - myMinU;
  double aVBound = myMaxV - myMinV;
  
  double aUScale = aWidth/aUBound;
  double aVScale = aHeight/aVBound;
  
  double aScale;
  aUScale <= aVScale ? aScale = aUScale : aScale = aVScale;

  double aUMiddle = ( myMaxU + myMinU )/2;
  double aVMiddle = ( myMaxV + myMinV )/2;
  
  int x = int(  aWidth/2  + (u - aUMiddle)*aScale + Border - Shift);
  
  int y = int(  aHeight/2 + (aVMiddle - v)*aScale + Border + Shift);
  
  QPoint aPoint = QPoint(x, y);
  
  return aPoint;
}
