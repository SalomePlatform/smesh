// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : StdMeshersGUI_FixedPointsParamWdg.h
// Author : Open CASCADE S.A.S. (dmv)
//
#ifndef STDMESHERSGUI_FIXEDPOINTSPARAMWGD_H
#define STDMESHERSGUI_FIXEDPOINTSPARAMWGD_H

// SMESH includes
#include <SMESHGUI.h>
#include "SMESH_StdMeshersGUI.hxx"

// Qt includes
#include <QWidget>
#include <QStringList>

class SMESHGUI;
class SMESHGUI_SpinBox;
class QPushButton;
class QLineEdit;
class QCheckBox;
class QListWidget;
class QListWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;

class STDMESHERSGUI_EXPORT StdMeshersGUI_FixedPointsParamWdg : public QWidget
{
  Q_OBJECT

    class LineDelegate;

public:
  StdMeshersGUI_FixedPointsParamWdg( QWidget* parent = 0 );
  ~StdMeshersGUI_FixedPointsParamWdg();

  bool                           eventFilter( QObject*, QEvent* );

  SMESH::double_array_var        GetListOfPoints();
  void                           SetListOfPoints( SMESH::double_array_var );

  SMESH::long_array_var          GetListOfSegments();
  void                           SetListOfSegments( SMESH::long_array_var );

  QString                        GetValue() const { return myParamValue; }

private slots:
  void                           onAdd(); 
  void                           onRemove(); 
  void                           onCheckBoxChanged();
  void                           updateState();

private:
  void                           clear();
  void                           addPoint( double ); 
  void                           removePoints(); 
  double                         point( int ) const;
  void                           setNbSegments( int, int );
  int                            nbSegments( int ) const;

  static QTreeWidgetItem*        newTreeItem( double v1, double v2 );
  static QListWidgetItem*        newListItem( double v1 );
  static QString                 treeItemText( double v1, double v2 );

private:
  QListWidget*                   myListWidget;
  QTreeWidget*                   myTreeWidget;
  SMESHGUI_SpinBox*              mySpinBox;
  QPushButton*                   myAddButton;
  QPushButton*                   myRemoveButton;
  QCheckBox*                     mySameValues;
  QString                        myParamValue;
};

#endif // STDMESHERSGUI_FIXEDPOINTSPARAMWGD_H
