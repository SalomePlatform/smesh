// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMESHGUI_Measurements.h
//  Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#ifndef SMESHGUI_MEASUREMENTS_H
#define SMESHGUI_MEASUREMENTS_H

#include "SMESH_SMESHGUI.hxx"

#include <QDialog>

class QButtonGroup;
class QLineEdit;
class QTabWidget;
class QGroupBox;
class SUIT_SelectionFilter;
class SALOME_Actor;
class SMESH_Actor;
class SMESHGUI_IdValidator;
class SMESHGUI_MeshEditPreview;

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include <gp_Vec.hxx>

class SMESHGUI_EXPORT SMESHGUI_MinDistance : public QWidget
{
  Q_OBJECT

  enum { NoTgt, FirstTgt, SecondTgt };
  enum { OriginTgt, NodeTgt, ElementTgt, ObjectTgt };

public:
  SMESHGUI_MinDistance( QWidget* = 0 );
  ~SMESHGUI_MinDistance();

  bool eventFilter( QObject*, QEvent* );
  void updateSelection();
  void deactivate();

private: 
  void setTarget( int );
  void erasePreview();
  void displayPreview();
  void createPreview( double, double, double, double, double, double );

private slots:
  void selectionChanged();
  void firstChanged();
  void secondChanged();
  void firstEdited();
  void secondEdited();
  void compute();
  void clear();

private:
  QButtonGroup*             myFirst;
  QButtonGroup*             mySecond;
  QLineEdit*                myFirstTgt;
  QLineEdit*                mySecondTgt;
  QLineEdit*                myDX;
  QLineEdit*                myDY;
  QLineEdit*                myDZ;
  QLineEdit*                myDistance;
  int                       myCurrentTgt;
  SMESH::SMESH_IDSource_var myFirstSrc;
  SMESH::SMESH_IDSource_var mySecondSrc;
  SMESH_Actor*              myFirstActor;
  SMESH_Actor*              mySecondActor;
  SMESHGUI_IdValidator*     myValidator;
  SUIT_SelectionFilter*     myFilter;
  SALOME_Actor*             myPreview;
};

class SMESHGUI_EXPORT SMESHGUI_BoundingBox : public QWidget
{
  Q_OBJECT

  enum { ObjectsSrc, NodesSrc, ElementsSrc };
  
public:
  SMESHGUI_BoundingBox( QWidget* = 0 );
  ~SMESHGUI_BoundingBox();

  void updateSelection();
  void deactivate();
  void erasePreview();

private:
  void displayPreview();
  void createPreview( double, double, double, double, double, double );

private slots:
  void selectionChanged();
  void sourceChanged();
  void sourceEdited();
  void compute();
  void clear();

private:
  typedef QList<SMESH::SMESH_IDSource_var> SourceList;
  QButtonGroup*             mySourceMode;
  QLineEdit*                mySource;
  QLineEdit*                myXmin;
  QLineEdit*                myXmax;
  QLineEdit*                myDX;
  QLineEdit*                myYmin;
  QLineEdit*                myYmax;
  QLineEdit*                myDY;
  QLineEdit*                myZmin;
  QLineEdit*                myZmax;
  QLineEdit*                myDZ;
  SourceList                mySrc;
  SMESH_Actor*              myActor;
  SMESHGUI_IdValidator*     myValidator;
  QString                   myIDs;
  SUIT_SelectionFilter*     myFilter;
  SALOME_Actor*             myPreview;
};

class SMESHGUI_EXPORT SMESHGUI_BasicProperties : public QWidget
{
  Q_OBJECT
  
public:
  //! Property type
  enum Mode { Length, Area, Volume };

  SMESHGUI_BasicProperties( QWidget* = 0 );
  ~SMESHGUI_BasicProperties();

  void setMode( const Mode );

  void updateSelection();
  void deactivate();

private slots:
  void selectionChanged();
  void modeChanged( int);
  void compute();
  void clear();

private:
  QButtonGroup*             myMode;
  QLineEdit*                mySource;
  QGroupBox*                myResultGrp;
  QLineEdit*                myResult;
  SMESH::SMESH_IDSource_var mySrc;
  SUIT_SelectionFilter*     myFilter;
};

class SMESHGUI_EXPORT SMESHGUI_Angle : public QWidget
{
  Q_OBJECT
  
public:

  SMESHGUI_Angle( QWidget* = 0 );
  ~SMESHGUI_Angle();

  void deactivate();
  void updateSelection();

private slots:
  void selectionChanged();
  void nodesEdited();
  void compute();
  void clear();

private:

  bool addPointByActor( int id );
  bool addPointByIDSource( int id );
  gp_Vec getNormal(const gp_Vec& vec10 );

  QLineEdit* myNodes;
  QLineEdit* myResult;

  SMESH::SMESH_IDSource_var myIDSrc;
  SMESH_Actor*              myActor;

  std::vector< SMESH::PointStruct > myPoints;
  SMESHGUI_MeshEditPreview*         myPreview;
};

class SMESHGUI_EXPORT SMESHGUI_MeasureDlg : public QDialog
{ 
  Q_OBJECT

  enum { NodeMode, ElemMode };

public:
  //! Measurement type
  enum { 
    MinDistance,   //!< minimum distance
    BoundingBox,   //!< bounding box
    Length,        //!< length
    Area,          //!< area
    Volume,        //!< volume
    Angle
  };

  SMESHGUI_MeasureDlg( QWidget* = 0, int = MinDistance );
  ~SMESHGUI_MeasureDlg();

  void reject();

protected:
  void keyPressEvent( QKeyEvent* );
  void enterEvent( QEvent* );

private slots:
  void help();
  void updateSelection();
  void activate();
  void deactivate();

private:
  QTabWidget*           myTabWidget;
  SMESHGUI_MinDistance* myMinDist;   
  SMESHGUI_BoundingBox* myBndBox;
  SMESHGUI_BasicProperties* myBasicProps;
  SMESHGUI_Angle*       myAngle;
};

#endif // SMESHGUI_MEASUREMENTS_H
