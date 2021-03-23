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

// File   : StdMeshersGUI_BlockRenumberCreator.h
// Author : Open CASCADE S.A.S.
//
#ifndef STDMESHERSGUI_BlockRenumberCreator_H
#define STDMESHERSGUI_BlockRenumberCreator_H

#include "SMESH_NumberFilter.hxx"
#include "SMESH_StdMeshersGUI.hxx"
#include "StdMeshersGUI_StdHypothesisCreator.h"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(GEOM_Gen)
//#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

class QButtonGroup;
class QLineEdit;
class QString;
class QTreeWidget;
class QTreeWidgetItem;

class STDMESHERSGUI_EXPORT StdMeshersGUI_BlockRenumberCreator : public StdMeshersGUI_StdHypothesisCreator
{
  Q_OBJECT

public:
  StdMeshersGUI_BlockRenumberCreator( const QString& aHypType );
  ~StdMeshersGUI_BlockRenumberCreator();

  virtual bool     checkParams( QString& ) const override;
  virtual QString  helpPage() const override;
  virtual QString  hypTypeName( const QString& ) const override;

protected:
  virtual QFrame*  buildFrame() override;
  virtual void     retrieveParams() const override;
  virtual QString  storeParams() const override;

private slots:
  void             onSelectionChange();
  void             onTreeSelectionChange();
  void             updateButtons();
  void             onSelectBtnClick();
  void             onAddBtnClick();
  void             onModifBtnClick();
  void             onRemoveBtnClick();

private:
  
  void             setBlockToTree( QTreeWidgetItem* item = 0 );

  QLineEdit*       myName;
  QTreeWidget*     myBlockTree;

  QPushButton*     myShapeSelectBut[3];
  QButtonGroup*    myButGroup;
  QLineEdit*       myLineEdit[3];

  QPushButton*     myAddBut;
  QPushButton*     myModifBut;
  QPushButton*     myRemoveBut;

  GEOM::GEOM_Object_var myGO[3];

  SMESH_NumberFilter mySolidFilter, myVertexFilter;
};

#endif // STDMESHERSGUI_BlockRenumberCreator_H
