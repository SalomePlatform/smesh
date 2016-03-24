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
#ifndef STDMESHERSGUI_PropagationHelperWdg_H
#define STDMESHERSGUI_PropagationHelperWdg_H

#include "SMESH_StdMeshersGUI.hxx"

#include <QWidget>
#include <vector>

class QPushButton;
class QListWidget;
class StdMeshersGUI_SubShapeSelectorWdg;
class vtkRenderer;
class GEOM_Actor;
class QCheckBox;
class QGroupBox;

/*!
 * \brief A widget showing a list of propagation chains of EDGEs.
 * Selecting a chain shows its EDGEs in a viewer with all EDGEs equally oriented,
 * 'Reverse' button reverses the EDGEs of a selected chain. 'Add' button adds
 *  EDGEs to a list of reversed EDGEs of StdMeshersGUI_SubShapeSelectorWdg
 */
class STDMESHERSGUI_EXPORT StdMeshersGUI_PropagationHelperWdg : public QWidget
{
  Q_OBJECT

 public:
  StdMeshersGUI_PropagationHelperWdg( StdMeshersGUI_SubShapeSelectorWdg* subSelectWdg,
                                      QWidget* parent = 0,
                                      bool show = true);
  ~StdMeshersGUI_PropagationHelperWdg();

  void                           Clear();

 private slots:

  void                           onShowGeometry(bool toShow);
  void                           onListSelectionChanged();
  void                           onAdd(); 
  void                           onReverse(); 
  void                           updateList(bool enable);

 private:

  bool                           buildChains();
  std::vector< int > *           getSelectedChain();

  StdMeshersGUI_SubShapeSelectorWdg* mySubSelectWdg;
  vtkRenderer*                       myRenderer;
  GEOM_Actor*                        myActor;
  GEOM_Actor*                        myModelActor;

  QListWidget*                       myListWidget;
  QPushButton*                       myAddButton;
  QPushButton*                       myReverseButton;
  QCheckBox*                         myShowGeomChkBox;
  QGroupBox*                         myChainBox;

  std::vector< std::vector<int> >    myChains;
};

#endif
