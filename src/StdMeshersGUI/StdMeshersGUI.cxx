//  SMESH StdMeshersGUI : GUI for plugged-in meshers
//
//  Copyright (C) 2003  CEA
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : StdMeshersGUI.cxx
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_HypothesesUtils.h"

#include "SMESHGUI_aParameterDlg.h"
#include "StdMeshersGUI_Parameters.h"
#include "StdMeshersGUI_CreateStdHypothesisDlg.h"

#include "QAD_Desktop.h"
#include "QAD_ResourceMgr.h"

#include <qobject.h>

#include "utilities.h"

using namespace std;

//=============================================================================
/*! class HypothesisCreator
 *
 */
//=============================================================================
class StdMeshersGUI_HypothesisCreator : public SMESHGUI_GenericHypothesisCreator
{
 public:
  StdMeshersGUI_HypothesisCreator (const QString& aHypType,
                                   const QString& aServerLibName,
                                   SMESHGUI* aSMESHGUI)
    : myHypType(aHypType),
    myServerLibName(aServerLibName),
    mySMESHGUI(aSMESHGUI) {}

  virtual void CreateHypothesis (const bool isAlgo, QWidget* parent = 0);
  virtual void EditHypothesis (SMESH::SMESH_Hypothesis_ptr theHyp);

 private:
  QString   myHypType;
  QString   myServerLibName;
  SMESHGUI* mySMESHGUI;
};

//=============================================================================
/*! HypothesisCreator::CreateHypothesis
 *
 */
//=============================================================================
void StdMeshersGUI_HypothesisCreator::CreateHypothesis
                                      (bool isAlgo, QWidget* parent)
{
  MESSAGE("StdMeshersGUI_HypothesisCreator::CreateHypothesis");

  // Get default name for hypothesis/algorithm creation
  char* sHypType = (char*)myHypType.latin1();
  HypothesisData* aHypData = SMESH::GetHypothesisData(sHypType);
  QString aHypName;
  if (aHypData)
    aHypName = aHypData->Label;
  else
    aHypName = myHypType;

  // Create hypothesis/algorithm
  if (isAlgo)
  {
    SMESH::CreateHypothesis(myHypType, aHypName, isAlgo);
  }
  else
  {
    if ( StdMeshersGUI_Parameters::HasParameters( myHypType ))
    // Show Dialog for hypothesis creation
      StdMeshersGUI_CreateStdHypothesisDlg *aDlg =
        new StdMeshersGUI_CreateStdHypothesisDlg(myHypType, parent, "");
    else
      SMESH::CreateHypothesis(myHypType, aHypName, isAlgo); // without GUI
  }
}

//=============================================================================
/*! HypothesisCreator::EditHypothesis
 *
 */
//=============================================================================
void StdMeshersGUI_HypothesisCreator::EditHypothesis
                                      (SMESH::SMESH_Hypothesis_ptr theHyp)
{
  MESSAGE("StdMeshersGUI_HypothesisCreator::EditHypothesis");

  SALOMEDS::Study::ListOfSObject_var listSOmesh =
    SMESH::GetMeshesUsingAlgoOrHypothesis(theHyp);
  
  list<SMESHGUI_aParameterPtr> paramList;
  StdMeshersGUI_Parameters::GetParameters( theHyp, paramList );

  bool modified = false;
  if ( SMESHGUI_aParameterDlg::Parameters( paramList, QObject::tr("SMESH_VALUE")) )
    modified = StdMeshersGUI_Parameters::SetParameters( theHyp, paramList );

  if ( modified ) {
    //set new Attribute Comment for hypothesis which parameters were modified
    QString aParams = "";
    StdMeshersGUI_Parameters::GetParameters( theHyp, paramList, aParams );
    SALOMEDS::SObject_var SHyp = SMESH::FindSObject(theHyp);
    if (!SHyp->_is_nil()) 
      if (!aParams.isEmpty()) {
	SMESH::SetValue(SHyp, aParams);
	//mySMESHGUI->GetActiveStudy()->updateObjBrowser(true);
      }    
	
    for (int i=0; i<listSOmesh->length(); i++)
      SMESH::ModifiedMesh(listSOmesh[i], false);
  }
}

//=============================================================================
/*! GetHypothesisCreator
 *
 */
//=============================================================================
extern "C"
{
  SMESHGUI_GenericHypothesisCreator* GetHypothesisCreator
    (QString aHypType, QString aServerLibName, SMESHGUI* aSMESHGUI)
    {
      return new StdMeshersGUI_HypothesisCreator
        (aHypType, aServerLibName, aSMESHGUI);
    }
}
