//=============================================================================
// File      : SMESH_subMesh_i.hxx
// Created   : jeu mai 30 10:01:17 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_SUBMESH_I_HXX_
#define _SMESH_SUBMESH_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)
#include CORBA_CLIENT_HEADER(GEOM_Gen)
#include CORBA_CLIENT_HEADER(GEOM_Shape)
#include CORBA_CLIENT_HEADER(MED)

class SMESH_Gen_i;
class SMESH_Mesh_i;

class SMESH_subMesh_i:
  public POA_SMESH::SMESH_subMesh
{
public:
  SMESH_subMesh_i();
  SMESH_subMesh_i(SMESH_Gen_i* gen_i,
		  SMESH_Mesh_i* mesh_i,
		  int localId);
  ~SMESH_subMesh_i();

  CORBA::Long GetNumberOfElements()
    throw (SALOME::SALOME_Exception);
  
  CORBA::Long  GetNumberOfNodes()
    throw (SALOME::SALOME_Exception);
  
  SMESH::long_array* GetElementsId()
    throw (SALOME::SALOME_Exception);
  
  SMESH::long_array* GetNodesId()
    throw (SALOME::SALOME_Exception);
  
  SMESH::SMESH_Mesh_ptr GetFather()
    throw (SALOME::SALOME_Exception);
  
  CORBA::Long GetId();   

  SALOME_MED::FAMILY_ptr GetFamily()
    throw (SALOME::SALOME_Exception);

  SMESH_Mesh_i* _mesh_i; //NRI

protected:
  SMESH_Gen_i* _gen_i;
  int _localId;
};

#endif
