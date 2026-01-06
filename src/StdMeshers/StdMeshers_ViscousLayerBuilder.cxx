// Copyright (C) 2007-2026  CEA, EDF, OPEN CASCADE
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

// File      : StdMeshers_ViscousLayerBuilder.cxx
// Module    : SMESH
// Author    : Cesar Conopoima (cce)
//

#include "SMESHDS_Mesh.hxx"
#include "SMESH_ControlsDef.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MeshAlgos.hxx"
#include "SMESH_MeshEditor.hxx"
#include "SMESH_MesherHelper.hxx"
#include "StdMeshers_ViscousLayerBuilder.hxx"

#include <TopoDS.hxx>

#include <vector>

//=======================================================================
//function : StdMeshers_ViscousLayerBuilder
//purpose  : Implements 
//=======================================================================

StdMeshers_ViscousLayerBuilder::StdMeshers_ViscousLayerBuilder(int        hypId,
                                                                SMESH_Gen* gen)
  : SMESH_2D_Algo(hypId, gen)
{
  _name = "StdMeshers_ViscousLayerBuilder";
  _hyp    = new StdMeshers_ViscousLayers2D( hypId, gen );  
}

bool StdMeshers_ViscousLayerBuilder::CheckHypothesis(SMESH_Mesh&                          aMesh,
                                                      const TopoDS_Shape&                  aShape,
                                                      SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
  (void) aMesh;
  (void) aShape;
  (void) aStatus;
  return true;
}

bool StdMeshers_ViscousLayerBuilder::Compute(SMESH_Mesh& /*aMesh*/, const TopoDS_Shape& /*aShape*/)
{
  return true;
}

bool StdMeshers_ViscousLayerBuilder::Evaluate(SMESH_Mesh & /*aMesh*/, const TopoDS_Shape & /*aShape*/,
                                                MapShapeNbElems& /*aResMap*/ )
{
  return true;
}

bool StdMeshers_ViscousLayerBuilder::IsApplicable( const TopoDS_Shape &S, bool /*toCheckAll*/, int algoDim )
{
  if ( S.ShapeType() > TopAbs_FACE )
    return false;

  return ( std::abs( algoDim ) == 3 || std::abs( algoDim ) == 2 ) ? true : false;
}

void StdMeshers_ViscousLayerBuilder::SetBndShapes(const std::vector<int>& faceIds, bool toIgnore)
{
  _hyp->SetBndShapes( faceIds, toIgnore );  
} // --------------------------------------------------------------------------------
void StdMeshers_ViscousLayerBuilder::SetTotalThickness(double thickness)
{
  _hyp->SetTotalThickness( thickness );
} // --------------------------------------------------------------------------------
void StdMeshers_ViscousLayerBuilder::SetNumberLayers(int nb)
{
  _hyp->SetNumberLayers( nb );
} // --------------------------------------------------------------------------------
void StdMeshers_ViscousLayerBuilder::SetStretchFactor(double factor)
{
  _hyp->SetStretchFactor( factor );
} // --------------------------------------------------------------------------------
void StdMeshers_ViscousLayerBuilder::SetMethod( StdMeshers_ViscousLayers::ExtrusionMethod method )
{
  _hyp->SetMethod( method );
} // --------------------------------------------------------------------------------
void StdMeshers_ViscousLayerBuilder::SetGroupName(const std::string& name)
{
  _hyp->SetGroupName( name );
} 

//=======================================================================
//function : ~StdMeshers_ViscousLayerBuilder
//purpose  :
//=======================================================================

StdMeshers_ViscousLayerBuilder::~StdMeshers_ViscousLayerBuilder()
{
}

TopoDS_Shape StdMeshers_ViscousLayerBuilder::GetShrinkGeometry( SMESH_Mesh & theMesh, const TopoDS_Shape& theShape )
{
  _vlBuilder  = new StdMeshers_Cartesian_VL::ViscousBuilder( _hyp, theMesh, theShape );

  std::string error = "";
  _offsetShape = _vlBuilder->MakeOffsetShape( theShape, theMesh, error );
  if ( error != "" )
    throw SALOME_Exception( error );
  
  return _offsetShape;
}

bool StdMeshers_ViscousLayerBuilder::AddLayers( SMESH_Mesh & shrinkMesh, SMESH_Mesh & theMesh, const TopoDS_Shape& theShape )
{
  bool success = _vlBuilder->MakeViscousLayers( shrinkMesh, theMesh, theShape );
  
  if ( !success )
    throw SALOME_Exception( "Error building viscous layer from shrink geometry." );

  return success;
}
