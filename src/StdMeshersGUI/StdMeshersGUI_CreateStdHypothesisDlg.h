//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : StdMeshersGUI_CreateStdHypothesisDlg.h
//           Moved here from SMESHGUI_CreateStdHypothesisDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_CreateStdHypothesisDlg_H
#define DIALOGBOX_CreateStdHypothesisDlg_H

#include "StdMeshersGUI_CreateHypothesisDlg.h"

//=================================================================================
// class    : StdMeshersGUI_CreateStdHypothesisDlg
// purpose  :
//=================================================================================
class StdMeshersGUI_CreateStdHypothesisDlg : public StdMeshersGUI_CreateHypothesisDlg
{ 
  Q_OBJECT

 public:
    StdMeshersGUI_CreateStdHypothesisDlg (const QString& hypType,
					  QWidget* parent = 0,
					  const char* name = 0,
					  bool modal = FALSE,
					  WFlags fl = 0);
    ~StdMeshersGUI_CreateStdHypothesisDlg ();

 private:
  virtual void GetParameters(const QString &                     hypType,
			     std::list<SMESHGUI_aParameterPtr> & params);
  
  virtual bool SetParameters(SMESH::SMESH_Hypothesis_ptr               theHyp,
			     const std::list<SMESHGUI_aParameterPtr> & params);
};

#endif
