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
// File   : SMESHGUI_PatternWidget.h
// Author : Michael ZORIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_PATTERNWIDGET_H
#define SMESHGUI_PATTERNWIDGET_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QFrame>
#include <QVector>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

typedef QVector<SMESH::PointStruct> PointVector;
typedef QVector< QVector<int> >     ConnectivityVector;

//=================================================================================
// class    : SMESHGUI_PatternWidget
// purpose  :
//=================================================================================
class SMESHGUI_EXPORT SMESHGUI_PatternWidget : public QFrame
{ 
  Q_OBJECT

public:
  SMESHGUI_PatternWidget( QWidget* = 0 );
  ~SMESHGUI_PatternWidget();
  
  void               SetPoints( const PointVector&,
                                const QVector<int>&,
                                const ConnectivityVector& );

private:
  PointVector        myPoints;
  QVector<int>       myKeys;
  ConnectivityVector myConnections;
  
  double             myMinU, myMaxU, myMinV, myMaxV;
  
protected:
  void               paintEvent( QPaintEvent* );

private:
  QPoint             mapCoords( const double, const double );
};

#endif // SMESHGUI_PATTERNWIDGET_H
