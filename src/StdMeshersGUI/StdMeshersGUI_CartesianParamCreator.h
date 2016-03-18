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

// File   : StdMeshersGUI_CartesianParamCreator.h
// Author : Open CASCADE S.A.S.
//
#ifndef STDMESHERSGUI_CartesianParamCreator_H
#define STDMESHERSGUI_CartesianParamCreator_H

// SMESH includes
#include "SMESH_StdMeshersGUI.hxx"

#include "StdMeshersGUI_StdHypothesisCreator.h"

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

#include <QFrame>
#include <QItemDelegate>

class QAbstractItemModel;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QModelIndex;
class QString;
class QStyleOptionViewItem;
class QTreeWidget;
class QTreeWidgetItem;
class QWidget;
class SMESHGUI_MeshEditPreview;
class SMESHGUI_SpinBox;

namespace StdMeshersGUI
{
  void getFromItem(QTreeWidgetItem * item, double& t0, double& t1, QString& fun );
  QTreeWidgetItem* setToItem (double  t0, double  t1, const QString& fun, QTreeWidgetItem* item=0);

  double coordFromItem( QListWidgetItem * );
  QListWidgetItem* coordToItem( double coord, QListWidgetItem * item=0);

  /*!
   * \brief Widget defining the grid in one direction
   */
  class GridAxisTab : public QFrame
  {
    Q_OBJECT
  public:
    GridAxisTab( QWidget* parent, const int axisIndex );
    ~GridAxisTab() {}

    void setCoordinates( SMESH::double_array_var coords );
    void setSpacing( SMESH::string_array_var funs, SMESH::double_array_var points );

    bool checkParams(QString& msg, SMESH::SMESH_Hypothesis_var& hyp) const;
    bool isGridBySpacing() const;
    SMESH::double_array* getCoordinates();
    void getSpacing(SMESH::string_array_out funs, SMESH::double_array_out points) const;

  signals:
    void gridModeChanged(int);

  private slots:
    void onInsert();
    void onDelete();
    void onMode(int);
    void onStepChange();
    void updateButtons();

  private:

    int               myAxisIndex;
    QButtonGroup*     myModeGroup;
    QTreeWidget*      mySpacingTreeWdg;
    QListWidget*      myCoordList;
    QPushButton*      myInsertBtn;
    QPushButton*      myDeleteBtn;
    SMESHGUI_SpinBox* myStepSpin;
    QLabel*           myStepLabel;
    double            myStep;
  };
  /*
   * \brief : Custom item delegate
   */
  class LineDelegate : public QItemDelegate
  {
    Q_OBJECT
  public:
    LineDelegate( QWidget* parent );
    ~LineDelegate() {}

    QWidget* createEditor( QWidget*, const QStyleOptionViewItem&, const QModelIndex& ) const;
    void setEditorData ( QWidget * editor, const QModelIndex & index ) const;
    void setModelData( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const;

  private:
    QTreeWidget* mySpacingTreeWdg;
    QListWidget* myCoordList;
  };
}

class STDMESHERSGUI_EXPORT StdMeshersGUI_CartesianParamCreator : public StdMeshersGUI_StdHypothesisCreator
{
  Q_OBJECT

public:
  StdMeshersGUI_CartesianParamCreator( const QString& aHypType );
  virtual ~StdMeshersGUI_CartesianParamCreator();

  virtual bool     checkParams( QString& ) const;
  virtual QString  helpPage() const;

protected:
  virtual QFrame*  buildFrame();
  virtual void     retrieveParams() const;
  virtual QString  storeParams() const;

private slots:
  bool             updateAxesPreview();
  void             onOrthogonalAxes(bool);
  void             onAxisDirChange(const QString&);
  void             onSelectionChange();
  void             onOptimalAxes(bool);
  void             onResetAxes(bool);
  void             onGridModeChanged(int);

private:
  QLineEdit*                  myName;
  SMESHGUI_SpinBox*           myThreshold;
  QCheckBox*                  myAddEdges;

  StdMeshersGUI::GridAxisTab* myAxisTabs[3];
  QGroupBox*                  myFixedPointGrp;
  SMESHGUI_SpinBox*           myPointSpin[3];
  QCheckBox*                  myOrthogonalChk;
  QButtonGroup*               myAxisBtnGrp;
  SMESHGUI_SpinBox*           myXDirSpin[3];
  SMESHGUI_SpinBox*           myYDirSpin[3];
  SMESHGUI_SpinBox*           myZDirSpin[3];
  SMESHGUI_MeshEditPreview*   myAxesPreview;
  double                      myOrigin[3];
  double                      myAxesLen;
  int                         myDirTic[3];
};

#endif // STDMESHERSGUI_CartesianParamCreator_H
