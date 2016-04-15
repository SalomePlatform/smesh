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

// File   : StdMeshersGUI_LayerDistributionParamWdg.h
// Author : Open CASCADE S.A.S.
//
#ifndef STDMESHERSGUI_LAYERDISTRIBUTIONPARAMWGD_H
#define STDMESHERSGUI_LAYERDISTRIBUTIONPARAMWGD_H

// SMESH includes
#include "SMESH_StdMeshersGUI.hxx"

// Qt includes
#include <QWidget>
#include <QStringList>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

class SMESHGUI;
class QPushButton;
class QMenu;
class QAction;
class QDialog;

/*!
 *  \brief Widget controlling hypothesis parameter that is another hypothesis
 */
class STDMESHERSGUI_EXPORT StdMeshersGUI_LayerDistributionParamWdg : public QWidget
{
  Q_OBJECT

public:
  StdMeshersGUI_LayerDistributionParamWdg(SMESH::SMESH_Hypothesis_ptr holderHyp,
                                          SMESH::SMESH_Hypothesis_ptr distribHyp,
                                          const QString&              name,
                                          QDialog*                    dlg);
  ~StdMeshersGUI_LayerDistributionParamWdg();

  SMESH::SMESH_Hypothesis_var GetHypothesis() { return myHyp; }

  QString GetValue() const { return myParamValue; }

  bool IsOk() const { return !myHyp->_is_nil(); }

private slots:
  void onCreate(); 
  void onEdit(); 
  void onHypTypePopup( QAction* );
  void onEdited(int);

private:
  void init();
  void set(SMESH::SMESH_Hypothesis_ptr);
  
private:
 SMESH::SMESH_Hypothesis_var myHyp;
 SMESH::SMESH_Hypothesis_var myHolderHyp;
 SMESHGUI*                   mySMESHGUI;

 QPushButton*           myCreateButton;
 QPushButton*           myEditButton;
 QMenu*                 myHypTypePopup;
 QDialog*               myDlg;
 QString                myName;
 QString                myParamValue;

 QStringList            myHypTypes;
};

#endif // STDMESHERSGUI_LAYERDISTRIBUTIONPARAMWGD_H
