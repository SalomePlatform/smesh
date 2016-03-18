// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_PatternWidget.cxx
// Author : Michael ZORIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_PatternWidget.h"

// Qt includes
#include <QPainter>

const int Shift  = 4;  // shift of the point number from point
const int Border = 20; // border size
const int Radius = 3;  // radius of a point

//=================================================================================
// class    : SMESHGUI_PatternWidget()
// purpose  :
//=================================================================================
SMESHGUI_PatternWidget::SMESHGUI_PatternWidget( QWidget* parent )
  : QFrame( parent )
{
  myMinU = myMinV = myMaxU = myMaxV = 0;
  //setMinimumHeight( 150 );
}

//=================================================================================
// function : ~SMESHGUI_PatternWidget()
// purpose  :
//=================================================================================
SMESHGUI_PatternWidget::~SMESHGUI_PatternWidget()
{
}

//=================================================================================
// function : SetPoints()
// purpose  :
//=================================================================================
void SMESHGUI_PatternWidget::SetPoints( const PointVector&        thePoints,
                                        const QVector<int>&       theKeys,
                                        const ConnectivityVector& theConnections )
{
  myPoints      = thePoints;
  myKeys        = theKeys;
  myConnections = theConnections;

  if ( myPoints.isEmpty() )
    return;

  myMinU = myMaxU = myPoints[0].x;
  myMinV = myMaxV = myPoints[0].y;

  for ( int i = 1; i < myPoints.size(); i++ ) {
    myMinU = qMin( myPoints[i].x, myMinU );
    myMaxU = qMax( myPoints[i].x, myMaxU );
    myMinV = qMin( myPoints[i].y, myMinV );
    myMaxV = qMax( myPoints[i].y, myMaxV );
  }

  repaint();
}

//=================================================================================
// function : paintEvent()
// purpose  :
//=================================================================================
void SMESHGUI_PatternWidget::paintEvent( QPaintEvent* )
{
  QPainter painter( this );
  painter.setBrush( Qt::SolidPattern );

  // Draw points
  for ( int i = 0; i < myKeys.size() && i < myPoints.size(); i++ ) {
    SMESH::PointStruct aPoint = myPoints[ myKeys[i] ];
    QPoint aQPnt = mapCoords( aPoint.x, aPoint.y );

    painter.drawPie( aQPnt.x() - Radius, aQPnt.y() - Radius, 
                     Radius * 2, Radius * 2, 0, 360 * 16 );
    painter.drawText( aQPnt.x() + Shift, aQPnt.y() - Shift, 
                      QString::number( i+1 ) );
  }

  // Draw lines
  for ( int i = 0; i < myConnections.size(); i++ ) {
    QVector<int> aCVector = myConnections[i];

    if ( aCVector.isEmpty() )
      continue;

    SMESH::PointStruct aPoint = myPoints[ aCVector[0] ];
    const QPoint aBeginPnt = mapCoords( aPoint.x, aPoint.y );
    QPoint aFirstPnt = aBeginPnt, aSecondPnt;

    for ( int j = 1; j < aCVector.size(); j++ ) {
      aPoint = myPoints[ aCVector[j] ];
      aSecondPnt = mapCoords( aPoint.x, aPoint.y );
      painter.drawLine( aFirstPnt, aSecondPnt );
      aFirstPnt = aSecondPnt;
    }

    painter.drawLine( aBeginPnt, aSecondPnt );
  }
}

//=================================================================================
// function : mapCoords()
// purpose  :
//=================================================================================
QPoint SMESHGUI_PatternWidget::mapCoords( const double u, const double v )
{
  int aWidth  = width()  - 2 * Border;
  int aHeight = height() - 2 * Border;

  double aUBound = myMaxU - myMinU;
  double aVBound = myMaxV - myMinV;

  double aUScale = aWidth  / aUBound;
  double aVScale = aHeight / aVBound;

  double aScale;
  aUScale <= aVScale ? aScale = aUScale : aScale = aVScale;

  double aUMiddle = ( myMaxU + myMinU ) / 2;
  double aVMiddle = ( myMaxV + myMinV ) / 2;

  int x = int( aWidth  / 2 + ( u - aUMiddle ) * aScale + Border - Shift );

  int y = int( aHeight / 2 + ( aVMiddle - v ) * aScale + Border + Shift );

  return QPoint( x, y );
}
