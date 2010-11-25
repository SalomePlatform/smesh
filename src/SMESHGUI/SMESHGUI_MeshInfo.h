//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  File   : SMESHGUI_MeshInfo.h
//  Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#ifndef SMESHGUI_MESHINFO_H
#define SMESHGUI_MESHINFO_H

#include "SMESH_SMESHGUI.hxx"
#include <SALOME_InteractiveObject.hxx>

#include <QFrame>
#include <QDialog>
#include <QList>
#include <QMap>
#include <QVector>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

class QButtonGroup;
class QLabel;
class QLineEdit;
class QTabWidget;
class QTextBrowser;
class QTreeWidget;
class QTreeWidgetItem;
class SMESH_Actor;
class SMDS_MeshNode;
class SMDS_MeshElement;

class SMESHGUI_EXPORT SMESHGUI_MeshInfo : public QFrame
{
  Q_OBJECT;
  
  enum {
    iName,
    iObject,
    iNodesStart,
    iNodes,
    iNodesEnd,
    iElementsStart = iNodesEnd, 
    iElements,
    i0DStart,
    i0D,
    i0DEnd,
    i1DStart       = i0DEnd,
    i1D,
    i1DEnd,
    i2DStart       = i1DEnd,
    i2D,
    i2DTriangles,
    i2DQuadrangles,
    i2DPolygons,
    i2DEnd,
    i3DStart       = i2DEnd,
    i3D,
    i3DTetrahedrons,
    i3DHexahedrons,
    i3DPyramids,
    i3DPrisms,
    i3DPolyhedrons,
    i3DEnd,
    iElementsEnd   = i3DEnd
  };

  enum {
    iSingle = 1,
    iTotal  = iSingle,
    iLinear,
    iQuadratic
  };

  typedef QList<QWidget*> wlist;
  typedef QVector<wlist>  iwlist;

public:
  SMESHGUI_MeshInfo( QWidget* = 0 );
  ~SMESHGUI_MeshInfo();

  void     showInfo( SMESH::SMESH_IDSource_ptr );
  void     clear();

private:
  enum { Bold = 0x01, Italic = 0x02 };

  QLabel*  createField();
  QWidget* createLine();
  void     setFontAttributes( QWidget*, int, bool = true );
  void     setFieldsVisible( int, int, bool );

private:
  iwlist   myWidgets;
};

class SMESHGUI_EXPORT SMESHGUI_ElemInfo : public QWidget
{
  Q_OBJECT;

public:
  SMESHGUI_ElemInfo( QWidget* = 0 );
  ~SMESHGUI_ElemInfo();

  void         setSource( SMESH_Actor* );
  virtual void showInfo( long, bool );
  virtual void clear() = 0;

protected:
  struct XYZ
  {
    double myX, myY, myZ;
    XYZ() { myX = myY = myZ = 0.0; }
    void add( double x, double y, double z ) { myX += x; myY += y; myZ += z; }
    void divide( double a ) { if ( a != 0.) { myX /= a; myY /= a; myZ /= a; } }
    double x() const  { return myX; }
    double y() const  { return myY; }
    double z() const  { return myZ; }
  };
  typedef QMap< int, QList<int> > Connectivity;

  Connectivity nodeConnectivity( const SMDS_MeshNode* );
  QString      formatConnectivity( Connectivity, int );
  XYZ          gravityCenter( const SMDS_MeshElement* );

protected:
  SMESH_Actor* myActor;
  long         myID;
  int          myIsElement;
};

class SMESHGUI_EXPORT SMESHGUI_SimpleElemInfo : public SMESHGUI_ElemInfo
{
public:
  SMESHGUI_SimpleElemInfo( QWidget* = 0 );

  void          showInfo( long, bool );
  void          clear();

private:
  QTextBrowser* myInfo;
};

class SMESHGUI_EXPORT SMESHGUI_TreeElemInfo : public SMESHGUI_ElemInfo
{
  class ItemDelegate;

public:
  SMESHGUI_TreeElemInfo( QWidget* = 0 );

  void             showInfo( long, bool );
  void             clear();

private:
  QTreeWidgetItem* createItem( QTreeWidgetItem* = 0, int = 100 );
  
private:
  QTreeWidget*     myInfo;
};

class SMESHGUI_EXPORT SMESHGUI_MeshInfoDlg : public QDialog
{ 
  Q_OBJECT;

  enum { NodeMode, ElemMode };

public:
  //! Information type
  enum { 
    BaseInfo,  //!< base mesh information
    ElemInfo   //!< mesh element information
  };

  SMESHGUI_MeshInfoDlg( QWidget* = 0, int = BaseInfo );
  ~SMESHGUI_MeshInfoDlg();

  void showInfo( const Handle(SALOME_InteractiveObject)& );
  void reject();

protected:
  void keyPressEvent( QKeyEvent* );
  void enterEvent( QEvent* );

private slots:
  void help();
  void updateSelection();
  void updateInfo();
  void activate();
  void deactivate();
  void modeChanged();
  void idChanged();

private:
  QTabWidget*        myTabWidget;
  SMESHGUI_MeshInfo* myBaseInfo;
  QButtonGroup*      myMode;
  QLineEdit*         myID;
  SMESHGUI_ElemInfo* myElemInfo;   
  SMESH_Actor*       myActor;
};

#endif // SMESHGUI_MESHINFO_H
