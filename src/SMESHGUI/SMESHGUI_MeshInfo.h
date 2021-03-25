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

#ifndef SMESHGUI_MESHINFO_H
#define SMESHGUI_MESHINFO_H

#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_SelectionProxy.h"

#ifndef DISABLE_PLOT2DVIEWER
  #include <Plot2d_Histogram.h>
#else
  #include <qwt_plot.h>
#endif

#include <QDialog>
#include <QList>
#include <QMap>
#include <QSet>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Filter)

#include <SALOME_InteractiveObject.hxx>
#include <SALOME_GenericObj_wrap.hxx>

class QAbstractButton;
class QButtonGroup;
class QCheckBox;
class QContextMenuEvent;
class QLabel;
class QLineEdit;
class QTabWidget;
class QTextBrowser;
class QTreeWidget;
class QTreeWidgetItem;
class SMDS_MeshElement;
class SMDS_MeshNode;
class SMESHGUI_IdPreview;
class SMESHGUI_SpinBox;

class ExtraWidget;
class GroupCombo;
class InfoWriter;

class SMESHGUI_EXPORT SMESHGUI_Info : public QWidget
{
public:
  SMESHGUI_Info( QWidget* = 0 );
  virtual void saveInfo( QTextStream& ) = 0;
};

class SMESHGUI_EXPORT SMESHGUI_BaseInfo : public SMESHGUI_Info
{
  Q_OBJECT
  
  enum
  {
    iStart,
    iObjectStart = iStart,
    iName = iObjectStart,
    iObject,
    iObjectEnd,
    iNodesStart = iObjectEnd,
    iNodes,
    iNodesEnd,
    iElementsStart = iNodesEnd,
    iElementsTitleStart = iElementsStart,
    iElementsTitle,
    iElementsTitleEnd,
    iElementsTotalStart = iElementsTitleEnd,
    iElementsTotal,
    iElementsTotalEnd,
    i0DStart = iElementsTotalEnd,
    i0D,
    i0DEnd,
    iBallsStart = i0DEnd,
    iBalls,
    iBallsEnd,
    i1DStart = iBallsEnd,
    i1D,
    i1DEnd,
    i2DStart = i1DEnd,
    i2D,
    i2DTriangles,
    i2DQuadrangles,
    i2DPolygons,
    i2DEnd,
    i3DStart = i2DEnd,
    i3D,
    i3DTetrahedrons,
    i3DHexahedrons,
    i3DPyramids,
    i3DPrisms,
    i3DHexaPrisms,
    i3DPolyhedrons,
    i3DEnd,
    iElementsEnd = i3DEnd,
    iEnd,
    iOther = iEnd
  };

  enum
  {
    iLabel,
    iSingle,
    iTotal = iSingle,
    iLinear,
    iQuadratic,
    iBiQuadratic,
    iNbColumns
  };

  typedef QMap<int, QWidget*> wlist;
  typedef QMap<int, wlist> iwlist;

public:
  SMESHGUI_BaseInfo( QWidget* = 0 );
  ~SMESHGUI_BaseInfo();

  void showInfo( const SMESH::SelectionProxy& );
  void clear();
  void saveInfo( QTextStream& );

private:
  QWidget* addWidget( QWidget*, int, int, int = 1 );
  QWidget* widget( int, int ) const;
  QString value( int, int ) const;
  void setFieldsVisible( int, int, bool );

private slots:
  void updateInfo();
  void loadMesh();

private:
  iwlist myWidgets;
  SMESH::SelectionProxy myProxy;
};

class SMESHGUI_EXPORT SMESHGUI_ElemInfo : public SMESHGUI_Info
{
  Q_OBJECT

public:
  SMESHGUI_ElemInfo( QWidget* = 0 );
  ~SMESHGUI_ElemInfo();

  void showInfo( const SMESH::SelectionProxy&, uint, bool );
  void showInfo( const SMESH::SelectionProxy&, QSet<uint>, bool );
  void showInfo( const SMESH::SelectionProxy& );
  void clear();
  void saveInfo( QTextStream& );

protected:
  enum { ShowNone, ShowNodes, ShowElements };

  QWidget* centralWidget() const;

  SMESH::SelectionProxy proxy() const;
  int what() const;

  QString type2str( int, bool = false );
  QString stype2str( int );
  QString etype2str( int );
  QString ctrl2str( int );
  void writeInfo( InfoWriter*, const QList<uint>& );
  virtual void information( const QList<uint>& ) = 0;
  virtual void clearInternal();

signals:
  void itemInfo( int type, const QString& ids );

private slots:
  void showPrevious();
  void showNext();
  void updateControls();

private:
  QWidget* myFrame;
  ExtraWidget* myExtra;
  SMESH::SelectionProxy myProxy;
  int myWhat;
  QList<uint> myIDs;
  int myIndex;
};

class SMESHGUI_EXPORT SMESHGUI_SimpleElemInfo : public SMESHGUI_ElemInfo
{
  Q_OBJECT

public:
  SMESHGUI_SimpleElemInfo( QWidget* = 0 );

protected:
  void information( const QList<uint>& );
  void clearInternal();

private slots:
  void connectivityClicked(const QUrl &);

 private:
  QTextBrowser* myInfo;
};

class SMESHGUI_EXPORT SMESHGUI_TreeElemInfo : public SMESHGUI_ElemInfo
{
  Q_OBJECT

  class ItemDelegate;
  class ItemCreator;

public:
  SMESHGUI_TreeElemInfo( QWidget* = 0 );

protected:
  void contextMenuEvent( QContextMenuEvent* );
  void information( const QList<uint>& );
  void nodeInfo( const SMDS_MeshNode*, int, int, QTreeWidgetItem* );
  void clearInternal();

private slots:
  void itemDoubleClicked( QTreeWidgetItem*, int );
  void saveExpanded( QTreeWidgetItem* );

private:
  QTreeWidgetItem* createItem( QTreeWidgetItem* = 0, int = 0 );
  QString          expandedResource( QTreeWidgetItem* );
  
private:
  QTreeWidget* myInfo;
};

class InfoComputor: public QObject
{
  Q_OBJECT

public:
  enum { GrpSize, GrpNbNodes };
  
  InfoComputor( QObject*, const SMESH::SelectionProxy&, int );

signals:
  void computed();

public slots:
  void compute();

private:
  SMESH::SelectionProxy myProxy;
  int myOperation;
};

class SMESHGUI_EXPORT SMESHGUI_AddInfo : public SMESHGUI_Info
{
  Q_OBJECT

public:
  SMESHGUI_AddInfo( QWidget* = 0 );
  ~SMESHGUI_AddInfo();

  void showInfo( const SMESH::SelectionProxy& );
  void clear();
  void saveInfo( QTextStream& );

private slots:
  void updateInfo();
  void showPreviousGroups();
  void showNextGroups();
  void showPreviousSubMeshes();
  void showNextSubMeshes();

private:
  QTreeWidgetItem* createItem( QTreeWidgetItem* = 0, int = 0 );
  void meshInfo( const SMESH::SelectionProxy&, QTreeWidgetItem* );
  void subMeshInfo( const SMESH::SelectionProxy&, QTreeWidgetItem* );
  void groupInfo( const SMESH::SelectionProxy&, QTreeWidgetItem* );

  void showGroups();
  void showSubMeshes();

private:
  SMESH::SelectionProxy myProxy;
  QTreeWidget* myTree;
  QList<InfoComputor*> myComputors;
  QList<SMESH::SelectionProxy> myGroups;
  QList<SMESH::SelectionProxy> mySubMeshes;
};

class SMESHGUI_EXPORT SMESHGUI_CtrlInfo : public SMESHGUI_Info
{
  Q_OBJECT

public:
  SMESHGUI_CtrlInfo( QWidget* = 0 );
  ~SMESHGUI_CtrlInfo();

  void showInfo( const SMESH::SelectionProxy& );
  void saveInfo( QTextStream& );

private:
  enum ObjectType { Mesh, SubMesh, Group };
  QwtPlot* createPlot( QWidget* );
  void clearInternal();
#ifndef DISABLE_PLOT2DVIEWER
  Plot2d_Histogram* getHistogram( SMESH::NumericalFunctor_ptr );
#endif
  void computeNb( int, int, int );

private slots:
  void computeAspectRatio();
  void computeAspectRatio3D();
  void computeFreeNodesInfo();
  void computeNodesNbConnInfo();
  void computeDoubleNodesInfo();
  void computeDoubleEdgesInfo();
  void computeDoubleFacesInfo();
  void computeOverConstrainedFacesInfo();
  void computeDoubleVolumesInfo();
  void computeOverConstrainedVolumesInfo();
  void setTolerance( double );

private:
  typedef SALOME::GenericObj_wrap< SMESH::Predicate > TPredicate;
  typedef SALOME::GenericObj_wrap< SMESH::NumericalFunctor > TNumFunctor;
  SMESH::SelectionProxy myProxy;
  ObjectType myObjectType;
  SMESHGUI_SpinBox* myToleranceWidget;
  QList<QLabel*> myWidgets;
  QwtPlot* myPlot;
  QwtPlot* myPlot3D;
  QList<QAbstractButton*> myButtons;
  QList<TPredicate> myPredicates;
  TNumFunctor myAspectRatio, myAspectRatio3D, myNodeConnFunctor;
};

class SMESHGUI_EXPORT SMESHGUI_MeshInfoDlg : public QDialog
{ 
  Q_OBJECT

  enum { NodeMode, ElemMode, GroupMode };

public:
  //! Information type
  enum
  { 
    BaseInfo,  //!< base mesh information
    ElemInfo,  //!< mesh element information
    AddInfo,   //!< additional information
    CtrlInfo   //!< controls information
  };

  SMESHGUI_MeshInfoDlg( QWidget* = 0, int = BaseInfo );
  ~SMESHGUI_MeshInfoDlg();

  void showInfo( const Handle(SALOME_InteractiveObject)& );
  void reject();

protected:
  void keyPressEvent( QKeyEvent* );

signals:
  void switchMode( int );

private slots:
  void help();
  void updateSelection();
  void updateInfo();
  void deactivate();
  void modeChanged();
  void idChanged();
  void idPreviewChange( bool );
  void showItemInfo( int type, const QString& ids );
  void dump();

private:
  void showInfo( const SMESH::SelectionProxy& );

  SMESH::SelectionProxy myProxy;
  QTabWidget* myTabWidget;
  SMESHGUI_BaseInfo* myBaseInfo;
  SMESHGUI_ElemInfo* myElemInfo;
  SMESHGUI_AddInfo* myAddInfo;
  SMESHGUI_CtrlInfo* myCtrlInfo;
  QButtonGroup* myMode;
  QLineEdit* myID;
  QCheckBox* myIDPreviewCheck;
  GroupCombo* myGroups;
  SMESHGUI_IdPreview* myIDPreview;
};

class SMESHGUI_EXPORT SMESHGUI_CtrlInfoDlg : public QDialog
{ 
  Q_OBJECT

public:
  SMESHGUI_CtrlInfoDlg( QWidget* = 0 );
  ~SMESHGUI_CtrlInfoDlg();

  void showInfo( const Handle(SALOME_InteractiveObject)& );
  void reject();

private slots:
  void updateInfo();
  void deactivate();
  void updateSelection();
  void help();
  void dump();

private:
  void showInfo( const SMESH::SelectionProxy& );

  SMESH::SelectionProxy myProxy;
  SMESHGUI_CtrlInfo* myCtrlInfo;
};

#endif // SMESHGUI_MESHINFO_H
