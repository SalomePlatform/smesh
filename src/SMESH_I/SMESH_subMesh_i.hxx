//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : SMESH_subMesh_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//
#ifndef _SMESH_SUBMESH_I_HXX_
#define _SMESH_SUBMESH_I_HXX_

#include "SMESH.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)
#include CORBA_CLIENT_HEADER(GEOM_Gen)
#include CORBA_CLIENT_HEADER(MED)

#include "SALOME_GenericObj_i.hh"
#include "SMESH_Mesh_i.hxx"

class SMESH_Gen_i;

class SMESH_I_EXPORT SMESH_subMesh_i:
  public virtual POA_SMESH::SMESH_subMesh,
  public virtual SALOME::GenericObj_i
{
public:
  SMESH_subMesh_i();
  SMESH_subMesh_i( PortableServer::POA_ptr thePOA,
                   SMESH_Gen_i*            gen_i,
                   SMESH_Mesh_i*           mesh_i,
                   int                     localId );
  ~SMESH_subMesh_i();

  CORBA::Long GetNumberOfElements()
    throw (SALOME::SALOME_Exception);
  
  CORBA::Long  GetNumberOfNodes( CORBA::Boolean all )
    throw (SALOME::SALOME_Exception);
  
  SMESH::long_array* GetElementsId()
    throw (SALOME::SALOME_Exception);

  SMESH::long_array* GetElementsByType( SMESH::ElementType theElemType )
    throw (SALOME::SALOME_Exception);
  
  //for omniORB conflict compilation
  /*SMESH::ElementType GetElementType( const CORBA::Long id, const bool iselem )
    throw (SALOME::SALOME_Exception);*/
  SMESH::ElementType GetElementType( CORBA::Long id, bool iselem )
    throw (SALOME::SALOME_Exception);
  
  SMESH::long_array* GetNodesId()
    throw (SALOME::SALOME_Exception);
  
  SMESH::SMESH_Mesh_ptr GetFather()
    throw (SALOME::SALOME_Exception);
  
  GEOM::GEOM_Object_ptr GetSubShape()
    throw (SALOME::SALOME_Exception);

  CORBA::Long GetId();   

  SALOME_MED::FAMILY_ptr GetFamily()
    throw (SALOME::SALOME_Exception);


  // =========================
  // interface SMESH_IDSource
  // =========================
  /*!
   * Returns a sequence of all element IDs
   */
  virtual SMESH::long_array* GetIDs();
  /*!
   * Returns statistic of mesh elements
   * Result array of number enityties
   * Inherited from SMESH_IDSource
   */
  virtual SMESH::long_array* GetMeshInfo();
  /*!
   * Returns types of elements it contains
   */
  virtual SMESH::array_of_ElementType* GetTypes();
  /*!
   * Returns the mesh
   */
  SMESH::SMESH_Mesh_ptr GetMesh();


  SMESH_Mesh_i* _mesh_i; //NRI

protected:
  void changeLocalId(int localId) { _localId = localId; }
  SMESH_Gen_i* _gen_i;
  int _localId;

  friend void SMESH_Mesh_i::CheckGeomGroupModif();
};

#endif
