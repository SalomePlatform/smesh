using namespace std;
// File:	SMDSControl_MeshBoundary.cxx
// Created:	Tue Mar 12 23:42:53 2002
// Author:	Jean-Michel BOULCOURT
//		<jmb@localhost.localdomain>


#include "SMDSControl_MeshBoundary.ixx"

//=======================================================================
//function : SMDSControl_MeshBoundary
//purpose  : 
//=======================================================================

SMDSControl_MeshBoundary::SMDSControl_MeshBoundary()
{
}

//=======================================================================
//function : SMDSControl_MeshBoundary
//purpose  : 
//=======================================================================

SMDSControl_MeshBoundary::SMDSControl_MeshBoundary(const Handle(SMDS_Mesh)& M)
  :myMesh(M)
{
}

//=======================================================================
//function : ResultMesh
//purpose  : 
//=======================================================================

Handle(SMDS_Mesh) SMDSControl_MeshBoundary::ResultMesh()
{
  if (myBoundaryMesh.IsNull())
    Compute();
  return myBoundaryMesh;
}

