//  SMESH SMESHGUI : GUI for SMESH component
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_HypothesesUtils.h
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_HypothesesUtils_HeaderFile
#define SMESHGUI_HypothesesUtils_HeaderFile

#include "SMESH_SMESHGUI.hxx"

#include <qstring.h>
#include <qstringlist.h>

#include "SALOME_InteractiveObject.hxx"
#include "SALOMEDSClient_definitions.hxx"

#include "SALOMEconfig.h"
#include CORBA_CLIENT_HEADER(SALOMEDS)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

#include <vector>

class HypothesisData;
class HypothesesSet;
class SMESHGUI_GenericHypothesisCreator;
class SALOMEDSClient_SObject;
class algo_error_array;

namespace SMESH{

  SMESHGUI_EXPORT
  void InitAvailableHypotheses();

  SMESHGUI_EXPORT
  QStringList GetAvailableHypotheses( const bool isAlgo, 
                                      const int theDim = -1, 
                                      const bool isAux = false);
  SMESHGUI_EXPORT
  QStringList GetHypothesesSets();

  SMESHGUI_EXPORT
  HypothesesSet* GetHypothesesSet(const QString theSetName);

  SMESHGUI_EXPORT
  HypothesisData* GetHypothesisData(const char* aHypType);

  SMESHGUI_EXPORT
  bool IsAvailableHypothesis(const HypothesisData* algoData,
                             const QString&        hypType,
                             bool&                 isOptional);

  SMESHGUI_EXPORT
  bool IsCompatibleAlgorithm(const HypothesisData* algo1Data,
                             const HypothesisData* algo2Data);

  SMESHGUI_EXPORT
  SMESHGUI_GenericHypothesisCreator* GetHypothesisCreator(const char* aHypType);

  SMESHGUI_EXPORT
  SMESH::SMESH_Hypothesis_ptr CreateHypothesis(const char* aHypType,
					       const char* aHypName,
					       const bool isAlgo = false);

  SMESHGUI_EXPORT
  bool AddHypothesisOnMesh(SMESH::SMESH_Mesh_ptr aMesh, SMESH::SMESH_Hypothesis_ptr aHyp);

  SMESHGUI_EXPORT
  bool AddHypothesisOnSubMesh(SMESH::SMESH_subMesh_ptr aSubMesh, SMESH::SMESH_Hypothesis_ptr aHyp);

  SMESHGUI_EXPORT
  bool RemoveHypothesisOrAlgorithmOnMesh(const Handle(SALOME_InteractiveObject)& IObject);

  SMESHGUI_EXPORT
  bool RemoveHypothesisOrAlgorithmOnMesh(_PTR(SObject) MorSM,
					 SMESH::SMESH_Hypothesis_ptr anHyp);

  typedef std::vector<_PTR(SObject)> SObjectList;
  SObjectList GetMeshesUsingAlgoOrHypothesis(SMESH::SMESH_Hypothesis_ptr AlgoOrHyp ) ;

  SMESHGUI_EXPORT
  QString GetMessageOnAlgoStateErrors(const algo_error_array& errors);
}

#endif
