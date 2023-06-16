// Copyright (C) 2007-2023  CEA/DEN, EDF R&D, OPEN CASCADE
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

#include "SMESH_Gen_i.hxx"


#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

//=============================================================================
/*!
 *  Constructor
 */
//=============================================================================

SMESH_ParallelMesh_i::SMESH_ParallelMesh_i( PortableServer::POA_ptr thePOA,
                            SMESH_Gen_i*            gen_i )
: SMESH_Mesh_i(thePOA, gen_i)
{
}

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

//================================================================================
/*!
 * \brief Set mesh implementation
 */
//================================================================================

void SMESH_ParallelMesh_i::SetImpl(::SMESH_ParallelMesh * impl)
{
  if(MYDEBUG) MESSAGE("SMESH_ParallelMesh_i::SetImpl");
  _impl = impl;
  if ( _impl )
    _impl->SetCallUp( new TCallUp_i(this));
}

//=============================================================================
/*!
 * Return a mesh implementation
 */
//=============================================================================

::SMESH_ParallelMesh & SMESH_ParallelMesh_i::GetImpl()
{
  if(MYDEBUG) MESSAGE("SMESH_ParallelMesh_i::GetImpl()");
  return *_impl;
}