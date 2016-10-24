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
//  File   : SMESHGUI_MeshInfo.h
//  Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#ifndef SMESHGUI_MESHINFO_H
#define SMESHGUI_MESHINFO_H

#include "SMESH_SMESHGUI.hxx"
#include "SMESH_ControlsDef.hxx"

#ifndef DISABLE_PLOT2DVIEWER
  #include <Plot2d_Histogram.h>
#else
  #include <qwt_plot.h>
#endif

#include <QFrame>
#include <QDialog>
#include <QList>
#include <QMap>
#include <QSet>
#include <QTreeWidget>
#include <QVector>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Filter)

#include <SALOME_InteractiveObject.hxx>
#include <SALOME_GenericObj_wrap.hxx>

class QAbstractButton;
class QButtonGroup;
class QContextMenuEvent;
class QLabel;
class QLineEdit;
class QPushButton;
class QTabWidget;
class QTextBrowser;
class QGridLayout;
class SMESH_Actor;
class SMDS_MeshNode;
class SMDS_MeshElement;
class SMESHGUI_SpinBox;

class ExtraWidget;

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
    iNbStart,
    iNb,
    iNbEnd,
    i0DStart = iNbEnd,
    i0D,
    i0DEnd,
    iBallsStart = i0DEnd,
    iBalls,
    iBallsEnd,
    i1DStart       = iBallsEnd,
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
    i3DHexaPrisms,
    i3DPolyhedrons,
    i3DEnd,
    iElementsEnd   = i3DEnd
  };

  enum {
    iSingle = 1,
    iTotal  = iSingle,
    iLinear,
    iQuadratic,
    iBiQuadratic
  };

  typedef QList<QWidget*> wlist;
  typedef QVector<wlist>  iwlist;

public:
  SMESHGUI_MeshInfo( QWidget* = 0 );
  ~SMESHGUI_MeshInfo();

  void     showInfo( SMESH::SMESH_IDSource_ptr );
  void     clear();
  void     saveInfo( QTextStream &out );

private:
  enum { Bold = 0x01, Italic = 0x02 };

  QLabel*  createField();
  QWidget* createLine();
  void     setFontAttributes( QWidget*, int, bool = true );
  void     setFieldsVisible( int, int, bool );

private slots:
  void loadMesh();

private:
  iwlist       myWidgets;
  QPushButton* myLoadBtn;
};

class SMESHGUI_EXPORT SMESHGUI_ElemInfo : public QWidget
{
  Q_OBJECT;

public:
  SMESHGUI_ElemInfo( QWidget* = 0 );
  ~SMESHGUI_ElemInfo();

  void         setSource( SMESH_Actor* );
  void         showInfo( long, bool );
  void         showInfo( QSet<long>, bool );
  void         clear();
  virtual void saveInfo( QTextStream &out ) = 0;

protected:
  struct XYZ
  {
    double myX, myY, myZ;
    XYZ() { myX = myY = myZ = 0.0; }
    XYZ(double x, double y, double z) { myX = x; myY = y; myZ = z; }
    void add( double x, double y, double z ) { myX += x; myY += y; myZ += z; }
    void divide( double a ) { if ( a != 0.) { myX /= a; myY /= a; myZ /= a; } }
    double x() const  { return myX; }
    double y() const  { return myY; }
    double z() const  { return myZ; }
  };
  typedef QMap< int, QList<int> > Connectivity;

  QWidget*     frame() const;
  SMESH_Actor* actor() const;
  bool         isElements() const;

  virtual void information( const QList<long>& ) = 0;
  virtual void clearInternal();

  Connectivity nodeConnectivity( const SMDS_MeshNode* );
  QString      formatConnectivity( Connectivity, int );
  XYZ          gravityCenter( const SMDS_MeshElement* );
  XYZ          normal( const SMDS_MeshElement* );

signals:
  void         itemInfo( int );
  void         itemInfo( const QString& );

private slots:
  void         showPrevious();
  void         showNext();
  void         updateControls();

private:
  SMESH_Actor*     myActor;
  QList<long>      myIDs;
  int              myIsElement;
  QWidget*         myFrame;
  ExtraWidget*     myExtra;
  int              myIndex;
};

class SMESHGUI_EXPORT SMESHGUI_SimpleElemInfo : public SMESHGUI_ElemInfo
{
  Q_OBJECT

public:
  SMESHGUI_SimpleElemInfo( QWidget* = 0 );
  void          saveInfo( QTextStream &out );

protected:
  void          information( const QList<long>& );
  void          clearInternal();

private:
  QTextBrowser* myInfo;
};

class SMESHGUI_EXPORT SMESHGUI_TreeElemInfo : public SMESHGUI_ElemInfo
{
  Q_OBJECT;

  class ItemDelegate;

  enum { Bold = 0x01, All = 0x80 };

public:
  SMESHGUI_TreeElemInfo( QWidget* = 0 );
  void             saveInfo( QTextStream &out );

protected:
  void             contextMenuEvent( QContextMenuEvent* e );
  void             information( const QList<long>& );
  void             nodeInfo( const SMDS_MeshNode*, int, int, QTreeWidgetItem* );
  void             clearInternal();

private slots:
  void             itemDoubleClicked( QTreeWidgetItem*, int );
  
private:
  QTreeWidgetItem* createItem( QTreeWidgetItem* = 0, int = 0 );
  
private:
  QTreeWidget*     myInfo;
};

class GrpComputor: public QObject
{
  Q_OBJECT;

public:
  GrpComputor( SMESH::SMESH_GroupBase_ptr, QTreeWidgetItem*, QObject*, bool = false);
  QTreeWidgetItem* getItem() { return myItem; }

public slots:
  void compute();

private:
  SMESH::SMESH_GroupBase_var myGroup;
  QTreeWidgetItem*           myItem;
  bool                       myToComputeSize;
};

class SMESHGUI_EXPORT SMESHGUI_AddInfo : public QTreeWidget
{
  Q_OBJECT;

  enum { Bold = 0x01, All = 0x80 };

public:
  SMESHGUI_AddInfo( QWidget* = 0 );
  ~SMESHGUI_AddInfo();

  void             showInfo( SMESH::SMESH_IDSource_ptr );
  //  void             clear();
  void             saveInfo( QTextStream &out );

private slots:
  void             changeLoadToCompute();
  void             showPreviousGroups();
  void             showNextGroups();
  void             showPreviousSubMeshes();
  void             showNextSubMeshes();

private:
  QTreeWidgetItem* createItem( QTreeWidgetItem* = 0, int = 0 );
  void             meshInfo( SMESH::SMESH_Mesh_ptr, QTreeWidgetItem* );
  void             subMeshInfo( SMESH::SMESH_subMesh_ptr, QTreeWidgetItem* );
  void             groupInfo( SMESH::SMESH_GroupBase_ptr, QTreeWidgetItem* );

  void             showGroups();
  void             showSubMeshes();

private:
  QList<GrpComputor*>      myComputors;
  SMESH::ListOfGroups_var  myGroups;
  SMESH::submesh_array_var mySubMeshes;
};

class SMESHGUI_EXPORT SMESHGUI_CtrlInfo : public QFrame
{
  Q_OBJECT;

public:
  SMESHGUI_CtrlInfo( QWidget* = 0 );
  ~SMESHGUI_CtrlInfo();

  void                  showInfo( SMESH::SMESH_IDSource_ptr );
  void                  saveInfo( QTextStream &out );

private:
  enum ObjectType { Mesh, SubMesh, Group };
  QLabel*               createField();
  QwtPlot*              createPlot( QWidget* );
  void                  setFontAttributes( QWidget* );
  void                  clearInternal();
#ifndef DISABLE_PLOT2DVIEWER
  Plot2d_Histogram*     getHistogram( SMESH::NumericalFunctor_ptr functor );
#endif
  void                  computeNb( int ft, int iBut, int iWdg );

private slots:
  void                  computeAspectRatio();
  void                  computeAspectRatio3D();
  void                  computeFreeNodesInfo();
  void                  computeNodesNbConnInfo();
  void                  computeDoubleNodesInfo();
  void                  computeDoubleEdgesInfo();
  void                  computeDoubleFacesInfo();
  void                  computeOverConstrainedFacesInfo();
  void                  computeDoubleVolumesInfo();
  void                  computeOverConstrainedVolumesInfo();
  void                  setTolerance( const double theTolerance );
  

private:
  typedef SALOME::GenericObj_wrap< SMESH::Predicate >        TPredicate;
  typedef SALOME::GenericObj_wrap< SMESH::NumericalFunctor > TNumFunctor;
  SMESH::SMESH_IDSource_var myObject;
  ObjectType                myObjectType;
  SMESHGUI_SpinBox*         myToleranceWidget;
  QList<QLabel*>            myWidgets;
  QGridLayout*              myMainLayout;
  QwtPlot*                  myPlot;
  QwtPlot*                  myPlot3D;
  QList<QAbstractButton*>   myButtons;
  QList<TPredicate>         myPredicates;
  TNumFunctor               myAspectRatio, myAspectRatio3D, myNodeConnFunctor;
};

class SMESHGUI_EXPORT SMESHGUI_MeshInfoDlg : public QDialog
{ 
  Q_OBJECT;

  enum { NodeMode, ElemMode };

public:
  //! Information type
  enum { 
    BaseInfo,  //!< base mesh information
    ElemInfo,  //!< mesh element information
    AddInfo,   //!< additional information
    CtrlInfo //!< controls information
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
  void showItemInfo( int );
  void showItemInfo( const QString& );
  void dump();

private:
  QTabWidget*        myTabWidget;
  SMESHGUI_MeshInfo* myBaseInfo;
  QButtonGroup*      myMode;
  QLineEdit*         myID;
  SMESHGUI_ElemInfo* myElemInfo;   
  SMESHGUI_AddInfo*  myAddInfo;
  SMESHGUI_CtrlInfo* myCtrlInfo;
  SMESH_Actor*       myActor;
  Handle(SALOME_InteractiveObject) myIO;
};

class SMESHGUI_EXPORT SMESHGUI_CtrlInfoDlg : public QDialog
{ 
  Q_OBJECT;

public:
  SMESHGUI_CtrlInfoDlg( QWidget* = 0 );
  ~SMESHGUI_CtrlInfoDlg();

  void showInfo( const Handle(SALOME_InteractiveObject)& );
  void reject();

private slots:
  void updateInfo();
  void activate();
  void deactivate();
  void updateSelection();
  void help();
  void dump();

private:
  SMESHGUI_CtrlInfo*  myCtrlInfo;
};

#endif // SMESHGUI_MESHINFO_H
