// Copyright (C) 2007-2025  CEA, EDF, OPEN CASCADE
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
//  File   : SMESH_ParallelMesh_i.cxx
//  Author : Yoann AUDOUIN, EDF
//  Module : SMESH

#include "SMESH_ParallelMesh_i.hxx"
#include "SMESH_Mesh_i.hxx"

#include "SMESH_Gen_i.hxx"


//=============================================================================
namespace
{
  //!< implementation of struct used to call methods of SMESH_Mesh_i from SMESH_Mesh
  // issue 0020918: groups removal is caused by hyp modification
  // issue 0021208: to forget not loaded mesh data at hyp modification
  struct TCallUp_i : public SMESH_ParallelMesh::TCallUp
  {
    SMESH_ParallelMesh_i* _mesh;
    TCallUp_i(SMESH_ParallelMesh_i* mesh):_mesh(mesh) {}
    void RemoveGroup (const int theGroupID) override { _mesh->removeGroup( theGroupID ); }
    void HypothesisModified( int hypID,
                             bool updIcons) override { _mesh->onHypothesisModified( hypID,
                                                                                    updIcons ); }
    void Load ()                            override { _mesh->Load(); }
    bool IsLoaded()                         override { return _mesh->IsLoaded(); }
    TopoDS_Shape GetShapeByEntry(const std::string& entry) override
    {
      GEOM::GEOM_Object_var go = SMESH_Gen_i::GetGeomObjectByEntry( entry );
      return SMESH_Gen_i::GeomObjectToShape( go );
    }
  };
}

::SMESH_ParallelMesh* SMESH_ParallelMesh_i::DownCast()
{
  ::SMESH_ParallelMesh* myImpl = dynamic_cast<::SMESH_ParallelMesh*>(_impl);
  if (myImpl == NULL)
    THROW_SALOME_CORBA_EXCEPTION("Could not cast as ParallelMesh", SALOME::INTERNAL_ERROR);

  return myImpl;
}

//=============================================================================
/*!
 * \brief Get the parallellism method
 */
//=============================================================================

CORBA::Long SMESH_ParallelMesh_i::GetParallelismMethod(){
  return DownCast()->GetParallelismMethod();
}

//=============================================================================
/*!
 * \brief Set the parallellism method
 */
//=============================================================================
void SMESH_ParallelMesh_i::SetParallelismMethod(CORBA::Long aMethod){
  DownCast()->SetParallelismMethod(aMethod);
}

//=============================================================================
/*!
 * \brief Get the parallell dimension
 */
//=============================================================================

CORBA::Long SMESH_ParallelMesh_i::GetParallelismDimension(){
  return DownCast()->GetParallelismDimension();
}

//=============================================================================
/*!
 * \brief Set the parallell dimension
 */
//=============================================================================
void SMESH_ParallelMesh_i::SetParallelismDimension(CORBA::Long aDim){
  DownCast()->SetParallelismDimension(aDim);
}

//=============================================================================
/*!
 * \brief Get the number of threads for a parallel computation
 */
//=============================================================================
CORBA::Long SMESH_ParallelMesh_i::GetNbThreads(){
  return DownCast()->GetNbThreads();
}

//=============================================================================
/*!
 * \brief Set the number of threads for a parallel computation
 */
//=============================================================================
void SMESH_ParallelMesh_i::SetNbThreads(CORBA::Long nbThreads){
  DownCast()->SetNbThreads(nbThreads);
}

//=============================================================================
/*!
 * \brief Get the ressource to connect to
 */
//=============================================================================
char* SMESH_ParallelMesh_i::GetResource(){
  return CORBA::string_dup(DownCast()->GetResource().c_str());
}

//=============================================================================
/*!
 * \brief Set the ressource to connect to
 */
//=============================================================================
void SMESH_ParallelMesh_i::SetResource(const char* aResource){
  DownCast()->SetResource(std::string(aResource));
}

//=============================================================================
/*!
 * \brief Get the number of processor to use on ressource
 */
//=============================================================================
CORBA::Long SMESH_ParallelMesh_i::GetNbProc(){
  return DownCast()->GetNbProc();
}

//=============================================================================
/*!
 * \brief Set the number of processor to use on ressource
 */
//=============================================================================
void SMESH_ParallelMesh_i::SetNbProc(CORBA::Long nbProcs){
  DownCast()->SetNbProc(nbProcs);
}

//=============================================================================
/*!
 * \brief Get the number of processor per node to use on ressource
 */
//=============================================================================
CORBA::Long SMESH_ParallelMesh_i::GetNbProcPerNode(){
  return DownCast()->GetNbProcPerNode();
}

//=============================================================================
/*!
 * \brief Set the number of processor per node to use on ressource
 */
//=============================================================================
void SMESH_ParallelMesh_i::SetNbProcPerNode(CORBA::Long nbProcPerNodes){
  DownCast()->SetNbProcPerNode(nbProcPerNodes);
}

//=============================================================================
/*!
 * \brief Get the number of node to use on ressource
 */
//=============================================================================
CORBA::Long SMESH_ParallelMesh_i::GetNbNode(){
  return DownCast()->GetNbNode();
}

//=============================================================================
/*!
 * \brief Set the number of node to use on ressource
 */
//=============================================================================
void SMESH_ParallelMesh_i::SetNbNode(CORBA::Long nbNodes){
  DownCast()->SetNbNode(nbNodes);
}

//=============================================================================
/*!
 * \brief Get the wckey to use on ressource
 */
//=============================================================================
char* SMESH_ParallelMesh_i::GetWcKey(){
  return CORBA::string_dup(DownCast()->GetWcKey().c_str());
}

//=============================================================================
/*!
 * \brief Set the wckey to use on ressource
 */
//=============================================================================
void SMESH_ParallelMesh_i::SetWcKey(const char* wcKey){
  DownCast()->SetWcKey(std::string(wcKey));
}

//=============================================================================
/*!
 * \brief Get the walltime to use on ressource
 */
//=============================================================================
char* SMESH_ParallelMesh_i::GetWalltime(){
  return CORBA::string_dup(DownCast()->GetWalltime().c_str());
}

//=============================================================================
/*!
 * \brief Set the walltime to use on ressource
 */
//=============================================================================
void SMESH_ParallelMesh_i::SetWalltime(const char* walltime){
  DownCast()->SetWalltime(std::string(walltime));
}
