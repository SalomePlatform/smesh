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
// File   : SMESHGUI_HypothesesUtils.h
// Author : Julia DOROVSKIKH, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_HYPOTHESESUTILS_H
#define SMESHGUI_HYPOTHESESUTILS_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// Qt includes
#include <QString>
#include <QStringList>

// SALOME GUI includes
#include <SALOME_InteractiveObject.hxx>

// SALOME KERNEL includes
#include <SALOMEDSClient_definitions.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

// STL includes
#include <vector>

// boost includes
#include <boost/shared_ptr.hpp>

class HypothesisData;
class HypothesesSet;
class SMESHGUI_GenericHypothesisCreator;
class SALOMEDSClient_SObject;
class algo_error_array;


namespace SMESH
{
  SMESHGUI_EXPORT
  void InitAvailableHypotheses();

  SMESHGUI_EXPORT
  QStringList GetAvailableHypotheses( const bool, 
                                      const int = -1, 
                                      const bool = false,
                                      const bool = true,
                                      const bool = false);
  SMESHGUI_EXPORT
  QStringList GetHypothesesSets( int );

  SMESHGUI_EXPORT
  HypothesesSet* GetHypothesesSet( const QString& );

  SMESHGUI_EXPORT
  HypothesisData* GetHypothesisData( const QString& );

  SMESHGUI_EXPORT
  HypothesisData* GetGroupTitle( const HypothesisData* hyp, const bool isAlgo );

  SMESHGUI_EXPORT
  bool IsAvailableHypothesis( const HypothesisData*,
                              const QString&,
                              bool& );

  SMESHGUI_EXPORT
  bool IsCompatibleAlgorithm( const HypothesisData*,
                              const HypothesisData* );

  SMESHGUI_EXPORT
  SMESHGUI_GenericHypothesisCreator* GetHypothesisCreator( const QString& );

  SMESHGUI_EXPORT
  SMESH::SMESH_Hypothesis_ptr CreateHypothesis( const QString&,
                                                const QString&,
                                                const bool = false );
  SMESHGUI_EXPORT
  bool IsApplicable( const QString&,
                     GEOM::GEOM_Object_ptr,
                     const bool = false );

  SMESHGUI_EXPORT
  bool AddHypothesisOnMesh( SMESH::SMESH_Mesh_ptr, SMESH::SMESH_Hypothesis_ptr );

  SMESHGUI_EXPORT
  bool AddHypothesisOnSubMesh( SMESH::SMESH_subMesh_ptr, SMESH::SMESH_Hypothesis_ptr );

  SMESHGUI_EXPORT
  bool RemoveHypothesisOrAlgorithmOnMesh( const Handle(SALOME_InteractiveObject)& );

  SMESHGUI_EXPORT
  bool RemoveHypothesisOrAlgorithmOnMesh( _PTR(SObject),
                                          SMESH::SMESH_Hypothesis_ptr );

  typedef std::vector<_PTR(SObject)> SObjectList;
  SObjectList GetMeshesUsingAlgoOrHypothesis( SMESH::SMESH_Hypothesis_ptr );

  SMESHGUI_EXPORT
  QString GetMessageOnAlgoStateErrors( const algo_error_array& );

  SMESHGUI_EXPORT
  // name of proprty saving plug-in of a hypothesis
  inline const char* Plugin_Name() { return "PLUGIN_NAME"; }
}

#endif // SMESHGUI_HYPOTHESESUTILS_H
