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

using namespace std;

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESHGUI.h"
#include "SMESHGUI_Hypotheses.h"

#include "StdMeshersGUI_LocalLengthDlg.h"
#include "StdMeshersGUI_NbSegmentsDlg.h"
#include "StdMeshersGUI_MaxElementAreaDlg.h"
#include "StdMeshersGUI_MaxElementVolumeDlg.h"

#include "QAD_Desktop.h"
#include "QAD_ResourceMgr.h"

#include <qobject.h>

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
  HypothesisData* aHypData = mySMESHGUI->GetHypothesisData(sHypType);
  QString aHypName;
  if (aHypData)
    aHypName = aHypData->Label;
  else
    aHypName = myHypType;

  // Create hypothesis/algorithm
  if (isAlgo)
  {
    mySMESHGUI->CreateHypothesis(myHypType, aHypName, isAlgo);
  }
  else
  {
    // Show Dialog for hypothesis creation
    if      (myHypType == "LocalLength")
      StdMeshersGUI_LocalLengthDlg *aDlg = new StdMeshersGUI_LocalLengthDlg(myHypType, parent, "");
    else if (myHypType == "NumberOfSegments")
      StdMeshersGUI_NbSegmentsDlg *aDlg = new StdMeshersGUI_NbSegmentsDlg(myHypType, parent, "");
    else if (myHypType == "MaxElementArea")
      StdMeshersGUI_MaxElementAreaDlg *aDlg = new StdMeshersGUI_MaxElementAreaDlg(myHypType, parent, "");
    else if (myHypType == "MaxElementVolume")
      StdMeshersGUI_MaxElementVolumeDlg *aDlg = new StdMeshersGUI_MaxElementVolumeDlg(myHypType, parent, "");
    else if (myHypType == "LengthFromEdges")
      mySMESHGUI->CreateHypothesis(myHypType, aHypName, isAlgo); // without GUI
    else if (myHypType == "NotConformAllowed")
      mySMESHGUI->CreateHypothesis(myHypType, aHypName, isAlgo); // without GUI
    else ;
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

  Standard_Boolean res = Standard_True;
  SALOMEDS::Study::ListOfSObject_var listSOmesh =
    mySMESHGUI->GetMeshesUsingAlgoOrHypothesis(theHyp);
  QString Name = theHyp->GetName();
  if (Name.compare("LocalLength") == 0)
  {
    StdMeshers::StdMeshers_LocalLength_var LL =
      StdMeshers::StdMeshers_LocalLength::_narrow(theHyp);
    double beforeLength = LL->GetLength() ;
    double Length = mySMESHGUI->Parameter(res, 
                                          beforeLength,
                                          QObject::tr("SMESH_LOCAL_LENGTH_HYPOTHESIS"), 
                                          QObject::tr("SMESH_VALUE"), 
                                          1.0E-5, 1E6, 6);
    if (res && Length != beforeLength)
    {
      LL->SetLength(Length);
      for (int i=0; i<listSOmesh->length(); i++)
      {
        mySMESHGUI->GetStudyAPI().ModifiedMesh(listSOmesh[i], false);
      }
    }
  }
  else if (Name.compare("NumberOfSegments") == 0)
  {
    StdMeshers::StdMeshers_NumberOfSegments_var NOS =
      StdMeshers::StdMeshers_NumberOfSegments::_narrow(theHyp);
    int beforeNbSeg = NOS->GetNumberOfSegments() ;
    int NbSeg = mySMESHGUI->Parameter(res,
                                      beforeNbSeg, 
                                      QObject::tr("SMESH_NB_SEGMENTS_HYPOTHESIS"), 
                                      QObject::tr("SMESH_VALUE"), 
                                      1, 1000000);
    if (res && NbSeg != beforeNbSeg)
    {
      NOS->SetNumberOfSegments(NbSeg);
      for (int i=0; i<listSOmesh->length(); i++)
      {
        SALOMEDS::SObject_var SO = listSOmesh[i] ;
        mySMESHGUI->GetStudyAPI().ModifiedMesh(listSOmesh[i], false);
      }
    }
  }
  else if (Name.compare("MaxElementArea") == 0)
  {
    StdMeshers::StdMeshers_MaxElementArea_var MEA =
      StdMeshers::StdMeshers_MaxElementArea::_narrow(theHyp);
    double beforeMaxArea = MEA->GetMaxElementArea();
    double MaxArea = mySMESHGUI->Parameter(res,
                                           beforeMaxArea,
                                           QObject::tr("SMESH_MAX_ELEMENT_AREA_HYPOTHESIS"), 
                                           QObject::tr("SMESH_VALUE"), 
                                           1.0E-5, 1E6, 6);
    if (res && MaxArea != beforeMaxArea)
    {
      MEA->SetMaxElementArea(MaxArea);
      for (int i=0; i<listSOmesh->length(); i++)
      {
        mySMESHGUI->GetStudyAPI().ModifiedMesh(listSOmesh[i], false);
      }
    }
  }
  else if (Name.compare("MaxElementVolume") == 0)
  {
    StdMeshers::StdMeshers_MaxElementVolume_var MEV =
      StdMeshers::StdMeshers_MaxElementVolume::_narrow(theHyp);
    double beforeMaxVolume = MEV->GetMaxElementVolume() ;
    double MaxVolume = mySMESHGUI->Parameter(res, 
                                             beforeMaxVolume,
                                             QObject::tr("SMESH_MAX_ELEMENT_VOLUME_HYPOTHESIS"), 
                                             QObject::tr("SMESH_VALUE"), 
                                             1.0E-5, 1E6, 6);
    if (res && MaxVolume != beforeMaxVolume)
    {
      MEV->SetMaxElementVolume(MaxVolume);
      for (int i=0; i<listSOmesh->length(); i++)
      {
        mySMESHGUI->GetStudyAPI().ModifiedMesh(listSOmesh[i], false);
      }
    }
  }
//  else if (Name.compare("Regular_1D") == 0)
//  {}
//  else if (Name.compare("MEFISTO_2D") == 0)
//  {}
  else
  {}
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
