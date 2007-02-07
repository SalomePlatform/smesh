//  SMESH SMESH : GUI for SMESH component
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESHGUI_Swig.hxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef _SMESHGUI_SWIG_HXX_
#define _SMESHGUI_SWIG_HXX_

#include "SMESH_SMESHGUI.hxx"

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SALOMEDS)
#include CORBA_SERVER_HEADER(SALOMEDS_Attributes)

class SMESHGUI_EXPORT SMESH_Swig
{
public:
  SMESH_Swig();
  ~SMESH_Swig();
    
  void Init(int studyID);

  const char* AddNewMesh(const char* IOR);

  const char* AddNewHypothesis(const char* IOR);
  const char* AddNewAlgorithms(const char* IOR);

  void SetShape(const char* ShapeEntry, const char* MeshEntry);

  void SetHypothesis(const char* Mesh_Or_SubMesh_Entry, const char* Hypothesis_Entry);
  void SetAlgorithms(const char* Mesh_Or_SubMesh_Entry, const char* Algorithms_Entry);

  void UnSetHypothesis(const char* Applied_Hypothesis_Entry );

  const char* AddSubMesh (const char* Mesh_Entry, const char* SM_IOR, int ST);
  const char* AddSubMeshOnShape (const char* Mesh_Entry, const char* GeomShape_Entry, const char* SM_IOR, int ST);

  void CreateAndDisplayActor( const char* Mesh_Entry );

  void SetName(const char* Entry, const char* Name);

  /*!
   * \brief Set mesh icon according to compute status
    * \param Mesh_Entry - entry of a mesh
    * \param isComputed - is mesh computed or not
   */
  void SetMeshIcon(const char* Mesh_Entry, const bool isComputed);

private:
  SALOMEDS::Study_var        myStudy;
  SALOMEDS::StudyBuilder_var myStudyBuilder;
  SALOMEDS::SComponent_var   mySComponentMesh;
};


#endif
