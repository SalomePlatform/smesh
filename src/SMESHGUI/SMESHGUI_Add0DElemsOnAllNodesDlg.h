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

#ifndef SMESHGUI_ADD0DELEMSONALLNODESDLG_H
#define SMESHGUI_ADD0DELEMSONALLNODESDLG_H

#include "SMESH_SMESHGUI.hxx"

#include "SMESHGUI_SelectionOp.h"
#include "SMESHGUI_Dialog.h"
#include "SMESHGUI_IdValidator.h"

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class SMESHGUI_Add0DElemsOnAllNodesOp;
class SMESHGUI_FilterDlg;

//---------------------------------------------------------------------------------
/*!
 * \brief Dialog creating 0D elements on all nodes of given elements
 */
class SMESHGUI_EXPORT SMESHGUI_Add0DElemsOnAllNodesDlg : public SMESHGUI_Dialog
{ 
  Q_OBJECT

 public:
  SMESHGUI_Add0DElemsOnAllNodesDlg();

  int                            getSelectionType() const;
  bool                           isValid();

signals:

  void                           selTypeChanged( int selType );


 private slots:

  void                           onGroupChecked ( bool on );
  void                           onSelTypeChange( int selType );

 private:

  friend class SMESHGUI_Add0DElemsOnAllNodesOp;

  QButtonGroup* mySelTypeBtnGrp;
  QPushButton*  myFilterBtn;
  QGroupBox*    myGroupBox;
  QLabel*       myGroupLabel;
  QComboBox*    myGroupListCmBox;
  QCheckBox*    myDuplicateElemsChkBox;

  SMESHGUI_IdValidator myIDValidator;
};

//---------------------------------------------------------------------------------
/*!
 * \brief Operation creating 0D elements on all nodes of given elements
 */
class SMESHGUI_EXPORT SMESHGUI_Add0DElemsOnAllNodesOp : public SMESHGUI_SelectionOp
{
  Q_OBJECT

 public:
  SMESHGUI_Add0DElemsOnAllNodesOp();
  ~SMESHGUI_Add0DElemsOnAllNodesOp();

  virtual LightApp_Dialog*       dlg() const { return myDlg; }
  
 protected:
  virtual void                   startOperation();
  virtual void                   selectionDone();
  virtual SUIT_SelectionFilter*  createFilter( const int ) const;

 protected slots:
  virtual bool                   onApply();
  void                           onSelTypeChange(int);
  void                           onSetFilter();
  virtual void                   onTextChanged( int, const QStringList& );
  void                           updateButtons();

 private:
  SMESHGUI_Add0DElemsOnAllNodesDlg* myDlg;
  SMESHGUI_FilterDlg*               myFilterDlg;
  Handle(SALOME_InteractiveObject)  myIO;
};

#endif
