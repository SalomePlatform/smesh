// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

//  File   : StdMeshers_BlockRenumber.cxx
//  Author : Edward AGAPOV, OCC
//  Module : SMESH

#include "StdMeshers_BlockRenumber.hxx"

#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "SMESH_Algo.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_TryCatch.hxx"

#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS.hxx>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

//=============================================================================
/*!
 * Constructor 
 */
//=============================================================================

StdMeshers_BlockRenumber::StdMeshers_BlockRenumber(int hypId, SMESH_Gen * gen)
  :SMESH_Hypothesis(hypId, gen)
{
  _name = "BlockRenumber";
  _param_algo_dim = 3; // is used by StdMeshers_Hexa_3D and StdMeshers_CompositeHexa_3D
}

//================================================================================
/*!
 * \brief Set local CS of blocks
 */
//================================================================================

void StdMeshers_BlockRenumber::SetBlocksOrientation( std::vector< StdMeshers_BlockCS > & blockCS )
{
  if ( _blockCS != blockCS )
  {
    NotifySubMeshesHypothesisModification();
    _blockCS.swap( blockCS );
    _solids2vertices.Clear();
  }
}

//================================================================================
/*
 * Return true and vertices if block orientation is defined for a given solid
 */
//================================================================================

bool StdMeshers_BlockRenumber::IsSolidIncluded( SMESH_Mesh&         mesh,
                                                const TopoDS_Shape& solid,
                                                TopoDS_Vertex&      vertex000,
                                                TopoDS_Vertex&      vertex001 ) const
{
  bool result = false;
  vertex000.Nullify();
  vertex001.Nullify();

  if ( _solids2vertices.IsEmpty() )
  {
    StdMeshers_BlockRenumber* me = const_cast<StdMeshers_BlockRenumber*>(this);
    for ( StdMeshers_BlockCS& bcs : me->_blockCS )
    {
      TopoDS_Shape so   = mesh.GetShapeByEntry( bcs._solid );
      TopoDS_Shape s000 = mesh.GetShapeByEntry( bcs._vertex000 );
      TopoDS_Shape s001 = mesh.GetShapeByEntry( bcs._vertex001 );
      TopoDS_Vertex v000 = StdMeshers_RenumberHelper::GetVertexAtPoint( so, s000 );
      TopoDS_Vertex v001 = StdMeshers_RenumberHelper::GetVertexAtPoint( so, s001 );
      if ( !v000.IsNull() && !v001.IsNull() )
      {
        me->_solids2vertices.Bind( so, std::make_pair( v000, v001 ));
        if ( so.IsSame( solid ))
        {
          result = true;
          vertex000 = v000;
          vertex001 = v001;
        }
      }
    }
  }
  else if ( !solid.IsNull() )
  {
    if (( result = _solids2vertices.IsBound( solid )))
    {
      auto vvPairPtr = _solids2vertices.Seek( solid );
      vertex000 = vvPairPtr->first;
      vertex001 = vvPairPtr->second;
    }
  }
  return result;
}

//=======================================================================
//function : CheckHypothesis
//purpose  : 
//=======================================================================

SMESH_ComputeErrorPtr StdMeshers_BlockRenumber::CheckHypothesis(SMESH_Mesh&         aMesh,
                                                                const TopoDS_Shape& /*shape*/) const
{
  SMESH_Comment errorTxt;
  for ( size_t i = 0; i < _blockCS.size() &&  errorTxt.empty(); ++i )
  {
    TopoDS_Shape solid = aMesh.GetShapeByEntry( _blockCS[i]._solid );
    TopoDS_Shape  v000 = aMesh.GetShapeByEntry( _blockCS[i]._vertex000 );
    TopoDS_Shape  v001 = aMesh.GetShapeByEntry( _blockCS[i]._vertex001 );
    v000 = StdMeshers_RenumberHelper::GetVertexAtPoint( solid, v000 );
    v001 = StdMeshers_RenumberHelper::GetVertexAtPoint( solid, v001 );

    if ( solid.IsNull() || solid.ShapeType() != TopAbs_SOLID )
      errorTxt << "Can't find a SOLID by entry '" << _blockCS[i]._solid << "'";
    else if ( v000.IsNull() || v000.ShapeType() != TopAbs_VERTEX )
      errorTxt << "Can't find a VERTEX by entry '" << _blockCS[i]._vertex000 << "'";
    else if ( v001.IsNull() || v001.ShapeType() != TopAbs_VERTEX )
      errorTxt << "Can't find a VERTEX by entry '" << _blockCS[i]._vertex001 << "'";
    else if ( !SMESH_MesherHelper::IsSubShape( v000, solid ))
      errorTxt << "VERTEX '" << _blockCS[i]._vertex001 << "' does not belong to SOLID '"
               << _blockCS[i]._solid << "'";
    else if ( !SMESH_MesherHelper::IsSubShape( v001, solid ))
      errorTxt << "VERTEX '" << _blockCS[i]._vertex001 << "' does not belong to SOLID '"
               << _blockCS[i]._solid << "'";
    else if ( SMESH_MesherHelper::Count( solid, TopAbs_VERTEX, true ) == 8 &&
              SMESH_MesherHelper::GetCommonAncestor( v000, v001, aMesh, TopAbs_EDGE ).IsNull() )
      errorTxt << "Vertices '" << _blockCS[i]._vertex000 << "' and '" << _blockCS[i]._vertex001
               << "' are not connected by an edge";
  }

  SMESH_ComputeErrorPtr error;
  if ( !errorTxt.empty() )
  {
    error = SMESH_ComputeError::New( COMPERR_BAD_PARMETERS,
                                     SMESH_Comment("Renumber hypothesis: ") << errorTxt );
  }
  return error;
}

//=======================================================================
//function : StdMeshers_RenumberHelper
//purpose  : constructor
//=======================================================================

StdMeshers_RenumberHelper::StdMeshers_RenumberHelper( SMESH_Mesh&                     mesh,
                                                      const StdMeshers_BlockRenumber* hyp)
  : _mesh( &mesh ), _hyp( hyp ), _newOldNodes( 2, nullptr )
{
}

//=======================================================================
//function : GetVertex000
//purpose  : Find default vertex at (0,0,0) local position
//=======================================================================

TopoDS_Vertex StdMeshers_RenumberHelper::GetVertex000( const TopTools_MapOfShape& cornerVertices )
{
  TopoDS_Vertex v000;
  if ( cornerVertices.Extent() < 8 )
    return TopoDS_Vertex();

  double minVal = DBL_MAX, minX = DBL_MAX, val;
  for ( auto it = cornerVertices.cbegin(); it != cornerVertices.cend(); ++it )
  {
    gp_Pnt P = BRep_Tool::Pnt( TopoDS::Vertex( *it ));
    val = P.X() + P.Y() + P.Z();
    if ( val < minVal || ( val == minVal && P.X() < minX ))
    {
      v000 = TopoDS::Vertex( *it );
      minVal = val;
      minX = P.X();
    }
  }
  return v000;
}

//=======================================================================
//function : GetVertex000
//purpose  : Find default vertex at (0,0,0) local position
//=======================================================================

TopoDS_Vertex StdMeshers_RenumberHelper::GetVertexAtPoint( const TopoDS_Shape&  solid,
                                                           const TopoDS_Shape& point )
{
  if ( !solid.IsNull() && !point.IsNull() && point.ShapeType() == TopAbs_VERTEX )
  {
    gp_Pnt   p = BRep_Tool::Pnt( TopoDS::Vertex( point ));
    double tol = Precision::Confusion();
    for ( TopExp_Explorer exp( solid, TopAbs_VERTEX ); exp.More(); exp.Next() )
    {
      const TopoDS_Vertex& v = TopoDS::Vertex( exp.Current() );
      if ( v.IsSame( point ) || p.IsEqual( BRep_Tool::Pnt( v ), tol ))
        return v;
    }
  }
  return TopoDS_Vertex();
}

//================================================================================
/*
 * Create a copy of an old node and remember this couple of nodes for replacement
 */
//================================================================================

void StdMeshers_RenumberHelper::AddReplacingNode( const SMDS_MeshNode* & oldNode )
{
  SMESHDS_Mesh*     mesh = _mesh->GetMeshDS();
  SMESH_NodeXYZ   oldXYZ = oldNode;
  SMDS_MeshNode* newNode = mesh->AddNode( oldXYZ.X(), oldXYZ.Y(), oldXYZ.Z() );
  _newOldNodes.front() = newNode;
  _newOldNodes.back()  = oldNode;
  _nodesToMerge.push_back( _newOldNodes );
  oldNode = newNode;

  int               shapeID = oldXYZ->GetShapeID();
  const TopoDS_Shape& shape = mesh->IndexToShape( shapeID );
  if ( !shape.IsNull() )
    switch ( shape.ShapeType() )
    {
    case TopAbs_FACE:
      if ( SMDS_FacePositionPtr pos = oldXYZ->GetPosition() )
        mesh->SetNodeOnFace( newNode, shapeID, pos->GetUParameter(), pos->GetVParameter() );
      break;
    case TopAbs_EDGE:
      if ( SMDS_EdgePositionPtr pos = oldXYZ->GetPosition() )
        mesh->SetNodeOnEdge( newNode, shapeID, pos->GetUParameter() );
      break;
    case TopAbs_VERTEX:
      mesh->SetNodeOnVertex( newNode, shapeID );
      break;
    default:
      mesh->SetNodeInVolume( newNode, shapeID );
    }
}

//================================================================================
/*
 * Replace old nodes by new ones
 */
//================================================================================

void StdMeshers_RenumberHelper::DoReplaceNodes()
{
  SMESH_MeshEditor( _mesh ).MergeNodes( _nodesToMerge );
}

//=============================================================================
/*!
 * Persistence
 */
//=============================================================================

ostream & StdMeshers_BlockRenumber::SaveTo(ostream & save)
{
  boost::archive::text_oarchive archive( save );
  archive << *this;

  return save;
}

//=============================================================================
/*!
 * Persistence
 */
//=============================================================================

istream & StdMeshers_BlockRenumber::LoadFrom(istream & load)
{
  SMESH_TRY;

  boost::archive::text_iarchive archive( load );
  archive >> *this;

  SMESH_CATCH( SMESH::doNothing );

  return load;
}

namespace boost {
  namespace serialization {

    //=======================================================================
    //function : serialize
    //purpose  : serialize StdMeshers_BlockCS
    //=======================================================================

    template<class Archive>
    void serialize(Archive & ar, StdMeshers_BlockCS & blockCS, const unsigned int /*version*/)
    {
      ar & blockCS._solid;
      ar & blockCS._vertex000;
      ar & blockCS._vertex001;
    }

  } // namespace serialization
} // namespace boost
