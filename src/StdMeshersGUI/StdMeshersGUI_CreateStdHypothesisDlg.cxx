//  SMESH StdMeshersGUI : GUI for StdMeshers plugin
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
//  File   : StdMeshersGUI_CreateStdHypothesisDlg.cxx
//           Moved here from SMESHGUI_CreateStdHypothesisDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "StdMeshersGUI_CreateStdHypothesisDlg.h"
#include "StdMeshersGUI_Parameters.h"
#include "SMESHGUI.h"
#include "SMESHGUI_Hypotheses.h"

#include "utilities.h"

#include "SALOMEGUI_QtCatchCorbaException.hxx"
#include "QAD_MessageBox.h"
#include "QAD_WaitCursor.h"
#include "QAD_Desktop.h"

//=================================================================================
// class    : StdMeshersGUI_CreateStdHypothesisDlg()
// purpose  : 
//=================================================================================
StdMeshersGUI_CreateStdHypothesisDlg::StdMeshersGUI_CreateStdHypothesisDlg (const QString& hypType,
                                                                            QWidget* parent,
                                                                            const char* name,
                                                                            bool modal,
                                                                            WFlags fl)
     : StdMeshersGUI_CreateHypothesisDlg (hypType, parent, name, modal, fl)
{
  QString hypTypeStr;
  if (hypType.compare("LocalLength") == 0)
    hypTypeStr = "LOCAL_LENGTH";
  else if (hypType.compare("NumberOfSegments") == 0)
    hypTypeStr = "NB_SEGMENTS";
  else if (hypType.compare("MaxElementArea") == 0)
    hypTypeStr = "MAX_ELEMENT_AREA";
  else if (hypType.compare("MaxElementVolume") == 0)
    hypTypeStr = "MAX_ELEMENT_VOLUME";
  else if (hypType.compare("StartEndLength") == 0)
    hypTypeStr = "START_END_LENGTH";
  else if (hypType.compare("Deflection1D") == 0)
    hypTypeStr = "DEFLECTION1D";
  else if (hypType.compare("Arithmetic1D") == 0)
    hypTypeStr = "ARITHMETIC_1D";
  else
    return;

  QString caption( tr ( QString( "SMESH_%1_TITLE" ).arg( hypTypeStr )));
  QString hypTypeName( tr ( QString( "SMESH_%1_HYPOTHESIS" ).arg( hypTypeStr )));
  QString hypIconName( tr ( QString( "ICON_DLG_%1" ).arg( hypTypeStr )));
  
  CreateDlgLayout(caption, 
                  QAD_Desktop::getResourceManager()->loadPixmap( "SMESH", hypIconName ),
                  hypTypeName);
}

//=================================================================================
// function : ~StdMeshersGUI_CreateStdHypothesisDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
StdMeshersGUI_CreateStdHypothesisDlg::~StdMeshersGUI_CreateStdHypothesisDlg()
{
    // no need to delete child widgets, Qt does it all for us
}
//=================================================================================
// function : FillParameters()
// purpose  :
//=================================================================================
void StdMeshersGUI_CreateStdHypothesisDlg::GetParameters
  (const QString &                     hypType,
   std::list<SMESHGUI_aParameterPtr> & params)
{
  StdMeshersGUI_Parameters::GetParameters( hypType, params );
}

//=================================================================================
// function : SetParameters()
// purpose  :
//=================================================================================
bool StdMeshersGUI_CreateStdHypothesisDlg::SetParameters
  (SMESH::SMESH_Hypothesis_ptr theHyp, const list<SMESHGUI_aParameterPtr> & params)
{
  StdMeshersGUI_Parameters::SetParameters( theHyp, params );
  return true;
}
