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
//  File   : StdMeshersGUI_QuadrangleParamWdg.h
//  Author : Open CASCADE S.A.S. (jfa)

#ifndef STDMESHERSGUI_QUADRANGLEPARAMWDG_H
#define STDMESHERSGUI_QUADRANGLEPARAMWDG_H

// SMESH includes
#include "SMESH_StdMeshersGUI.hxx"
#include "StdMeshersGUI_StdHypothesisCreator.h"

#include <SALOME_InteractiveObject.hxx>

// Qt includes
#include <QWidget>
#include <QItemDelegate>


class QButtonGroup;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTabWidget;
class QTreeWidget;
class StdMeshersGUI_QuadrangleParamWdg;
class StdMeshersGUI_SubShapeSelectorWdg;

//================================================================================
/*!
 * \brief Quadrangle Parameters Creator
 */
class STDMESHERSGUI_EXPORT StdMeshersGUI_QuadrangleParamCreator : public StdMeshersGUI_StdHypothesisCreator
{
  Q_OBJECT

    class ItemDelegate;

 public:
  StdMeshersGUI_QuadrangleParamCreator( const QString& aHypType );

  //virtual bool     checkParams( QString& ) const;
  virtual QString  helpPage() const;

 protected:
  virtual QFrame*  buildFrame();
  virtual void     retrieveParams() const;
  virtual QString  storeParams() const;

 private slots:

  void onTypeChanged(int type);
  void onAddShape();
  void onRemoveShape();
  void onAddPoint();
  void onRemovePoint();
  void onSelectionChanged();
  void onTabChanged(int);

 private:

  QLineEdit*                         myName;
  StdMeshersGUI_QuadrangleParamWdg*  myTypeWdg;
  StdMeshersGUI_SubShapeSelectorWdg* myVertexSelWdg;
  QListWidget*                       myShapesList;
  QPushButton*                       myAddShapeBut;
  QTreeWidget*                       myCoordsTreeWdg;
  QTabWidget*                        myTabs;
  Handle(SALOME_InteractiveObject)   mySelectedShapeIO;
};

//================================================================================
/*!
 * \brief Transition type widget
 */
class STDMESHERSGUI_EXPORT StdMeshersGUI_QuadrangleParamWdg : public QWidget
{
  Q_OBJECT

    public:
  StdMeshersGUI_QuadrangleParamWdg (QWidget* parent = 0);
  ~StdMeshersGUI_QuadrangleParamWdg();

  void SetType (int theType);
  int  GetType ();

  GEOM::ListOfGO*     GetShapes();
  SMESH::nodes_array* GetPoints();
  void                Set( GEOM::ListOfGO_var shapes, SMESH::nodes_array_var points );

 signals:

  void typeChanged(int);

 private:
  QButtonGroup* myType; // Quadranle preference, Triangle preference, Reduced
};

//================================================================================
/*!
 * \brief Item delegate for a tree widget
 */
class StdMeshersGUI_QuadrangleParamCreator::ItemDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  ItemDelegate( QObject* );
  QWidget* createEditor( QWidget*, const QStyleOptionViewItem&, const QModelIndex& ) const;
};

#endif // STDMESHERSGUI_QUADRANGLEPARAMWDG_H
