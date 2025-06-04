// Copyright (C) 2016-2025  CEA, EDF
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  File   : StdMeshers_Cartesian_3D_Grid.cxx
//  Module : SMESH
//  Purpose: Make BodyFitting mesh algorithm more modular and testable
//

#include "StdMeshers_Cartesian_3D_Grid.hxx"

#include <BRepIntCurveSurface_Inter.hxx>

#ifdef WITH_TBB

#ifdef WIN32
// See https://docs.microsoft.com/en-gb/cpp/porting/modifying-winver-and-win32-winnt?view=vs-2019
// Windows 10 = 0x0A00  
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#endif

#include <tbb/parallel_for.h>
#endif

using namespace std;
using namespace SMESH;
using namespace StdMeshers::Cartesian3D;

std::mutex _bMutex;

//=============================================================================
/*
  * Remove coincident intersection points
  */
void GridLine::RemoveExcessIntPoints( const double tol )
{
  if ( _intPoints.size() < 2 ) return;

  set< Transition > tranSet;
  multiset< F_IntersectPoint >::iterator ip1, ip2 = _intPoints.begin();
  while ( ip2 != _intPoints.end() )
  {
    tranSet.clear();
    ip1 = ip2++;
    while ( ip2 != _intPoints.end() && ip2->_paramOnLine - ip1->_paramOnLine <= tol )
    {
      tranSet.insert( ip1->_transition );
      tranSet.insert( ip2->_transition );
      ip2->Add( ip1->_faceIDs );
      _intPoints.erase( ip1 );
      ip1 = ip2++;
    }
    if ( tranSet.size() > 1 ) // points with different transition coincide
    {
      bool isIN  = tranSet.count( Trans_IN );
      bool isOUT = tranSet.count( Trans_OUT );
      if ( isIN && isOUT )
        (*ip1)._transition = Trans_TANGENT;
      else
        (*ip1)._transition = isIN ? Trans_IN : Trans_OUT;
    }
  }
}
//================================================================================
/*
  * Return ID of SOLID for nodes before the given intersection point
  */
TGeomID GridLine::GetSolidIDBefore( multiset< F_IntersectPoint >::iterator ip,
                                    const TGeomID                          prevID,
                                    const Geometry&                        geom )
{
  if ( ip == _intPoints.begin() )
    return 0;

  if ( geom.IsOneSolid() )
  {
    bool isOut = true;
    switch ( ip->_transition ) {
    case Trans_IN:      isOut = true;            break;
    case Trans_OUT:     isOut = false;           break;
    case Trans_TANGENT: isOut = ( prevID != 0 ); break;
    case Trans_APEX:
    {
      // singularity point (apex of a cone)
      multiset< F_IntersectPoint >::iterator ipBef = ip, ipAft = ++ip;
      if ( ipAft == _intPoints.end() )
        isOut = false;
      else
      {
        --ipBef;
        if ( ipBef->_transition != ipAft->_transition )
          isOut = ( ipBef->_transition == Trans_OUT );
        else
          isOut = ( ipBef->_transition != Trans_OUT );
      }
      break;
    }
    case Trans_INTERNAL: isOut = false;
    default:;
    }
    return isOut ? 0 : geom._soleSolid.ID();
  }

  GeomIDVecHelder solids = geom.GetSolidIDsByShapeID( ip->_faceIDs );

  --ip;
  if ( ip->_transition == Trans_INTERNAL )
    return prevID;

  GeomIDVecHelder solidsBef = geom.GetSolidIDsByShapeID( ip->_faceIDs );

  if ( ip->_transition == Trans_IN ||
        ip->_transition == Trans_OUT )
  {
    if ( solidsBef.size() == 1 )
    {
      if ( solidsBef[0] == prevID )
        return ip->_transition == Trans_OUT ? 0 : solidsBef[0];
      else
        return solidsBef[0];
    }

    if ( solids.size() == 2 )
    {
      if ( solids == solidsBef )
        return solids.contain( prevID ) ? solids.otherThan( prevID ) : theUndefID; // bos #29212
    }
    return solids.oneCommon( solidsBef );
  }

  if ( solidsBef.size() == 1 )
    return solidsBef[0];

  return solids.oneCommon( solidsBef );
}

//================================================================================
/*
  * Adds face IDs
  */
bool B_IntersectPoint::Add( const vector< TGeomID >& fIDs,
                            const SMDS_MeshNode*       n) const
{
  const std::lock_guard<std::mutex> lock(_bMutex);
  size_t prevNbF = _faceIDs.size();

  if ( _faceIDs.empty() )
    _faceIDs = fIDs;
  else
    for ( size_t i = 0; i < fIDs.size(); ++i )
    {
      vector< TGeomID >::iterator it =
        std::find( _faceIDs.begin(), _faceIDs.end(), fIDs[i] );
      if ( it == _faceIDs.end() )
        _faceIDs.push_back( fIDs[i] );
    }
  if ( !_node && n != NULL )
    _node = n;

  return prevNbF < _faceIDs.size();
}
//================================================================================
/*
  * Return ID of a common face if any, else zero
  */
TGeomID B_IntersectPoint::HasCommonFace( const B_IntersectPoint * other, TGeomID avoidFace ) const
{
  if ( other )
    for ( size_t i = 0; i < other->_faceIDs.size(); ++i )
      if ( avoidFace != other->_faceIDs[i] &&
            IsOnFace   ( other->_faceIDs[i] ))
        return other->_faceIDs[i];
  return 0;
}
//================================================================================
/*
  * Return faces common with other point
  */
size_t B_IntersectPoint::GetCommonFaces( const B_IntersectPoint * other, TGeomID* common ) const
{
  size_t nbComm = 0;
  if ( !other )
    return nbComm;
  if ( _faceIDs.size() > other->_faceIDs.size() )
    return other->GetCommonFaces( this, common );
  for ( const TGeomID& face : _faceIDs )
    if ( other->IsOnFace( face ))
      common[ nbComm++ ] = face;
  return nbComm;
}
//================================================================================
/*
  * Return \c true if \a faceID in in this->_faceIDs
  */
bool B_IntersectPoint::IsOnFace( TGeomID faceID ) const // returns true if faceID is found
{
  vector< TGeomID >::const_iterator it =
    std::find( _faceIDs.begin(), _faceIDs.end(), faceID );
  return ( it != _faceIDs.end() );
}

//================================================================================
/*
  * OneOfSolids initialization
  */
void OneOfSolids::Init( const TopoDS_Shape& solid,
                        TopAbs_ShapeEnum    subType,
                        const SMESHDS_Mesh* mesh )
{
  SetID( mesh->ShapeToIndex( solid ));

  if ( subType == TopAbs_FACE )
    SetHasInternalFaces( false );

  for ( TopExp_Explorer sub( solid, subType ); sub.More(); sub.Next() )
  {
    _subIDs.Add( mesh->ShapeToIndex( sub.Current() ));
    if ( subType == TopAbs_FACE )
    {
      _faces.Add( sub.Current() );
      if ( sub.Current().Orientation() == TopAbs_INTERNAL )
        SetHasInternalFaces( true );

      TGeomID faceID = mesh->ShapeToIndex( sub.Current() );
      if ( sub.Current().Orientation() == TopAbs_INTERNAL ||
            sub.Current().Orientation() == mesh->IndexToShape( faceID ).Orientation() )
        _outFaceIDs.Add( faceID );
    }
  }
}

//=============================================================================
/*
  * Return a vector of SOLIDS sharing given shapes
  */
GeomIDVecHelder Geometry::GetSolidIDsByShapeID( const vector< TGeomID >& theShapeIDs ) const
{
  if ( theShapeIDs.size() == 1 )
    return GeomIDVecHelder( _solidIDsByShapeID[ theShapeIDs[ 0 ]], /*owner=*/false );

  // look for an empty slot in _solidIDsByShapeID
  vector< TGeomID > * resultIDs = 0;
  for ( const vector< TGeomID >& vec : _solidIDsByShapeID )
    if ( vec.empty() )
    {
      resultIDs = const_cast< vector< TGeomID > * >( & vec );
      break;
    }
  // fill in resultIDs
  for ( const TGeomID& id : theShapeIDs )
    for ( const TGeomID& solid : _solidIDsByShapeID[ id ])
    {
      if ( std::find( resultIDs->begin(), resultIDs->end(), solid ) == resultIDs->end() )
        resultIDs->push_back( solid );
    }
  return GeomIDVecHelder( *resultIDs, /*owner=*/true );
}

//================================================================================
/*
  * Return an iterator on GridLine's in a given direction
  */
LineIndexer Grid::GetLineIndexer(size_t iDir) const
{
  const size_t indices[] = { 1,2,0, 0,2,1, 0,1,2 };
  const string s      [] = { "X", "Y", "Z" };
  LineIndexer li( _coords[0].size(),  _coords[1].size(),    _coords[2].size(),
                  indices[iDir*3],    indices[iDir*3+1],    indices[iDir*3+2],
                  s[indices[iDir*3]], s[indices[iDir*3+1]], s[indices[iDir*3+2]]);
  return li;
}
//================================================================================
/*
  * Return direction [0,1,2] of a GridLine
  */
size_t Grid::GetLineDir( const GridLine* line, size_t & index ) const
{
  for ( size_t iDir = 0; iDir < 3; ++iDir )
    if ( &_lines[ iDir ][0] <= line && line <= &_lines[ iDir ].back() )
    {
      index = line - &_lines[ iDir ][0];
      return iDir;
    }
  return -1;
}
//=============================================================================
/*
  * Creates GridLine's of the grid
  */
void Grid::SetCoordinates(const vector<double>& xCoords,
                          const vector<double>& yCoords,
                          const vector<double>& zCoords,
                          const double*         axesDirs,
                          const Bnd_Box&        shapeBox)
{
  _coords[0] = xCoords;
  _coords[1] = yCoords;
  _coords[2] = zCoords;

  _axes[0].SetCoord( axesDirs[0],
                      axesDirs[1],
                      axesDirs[2]);
  _axes[1].SetCoord( axesDirs[3],
                      axesDirs[4],
                      axesDirs[5]);
  _axes[2].SetCoord( axesDirs[6],
                      axesDirs[7],
                      axesDirs[8]);
  _axes[0].Normalize();
  _axes[1].Normalize();
  _axes[2].Normalize();

  _invB.SetCols( _axes[0], _axes[1], _axes[2] );
  _invB.Invert();

  // compute tolerance
  _minCellSize = Precision::Infinite();
  for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
  {
    for ( size_t i = 1; i < _coords[ iDir ].size(); ++i )
    {
      double cellLen = _coords[ iDir ][ i ] - _coords[ iDir ][ i-1 ];
      if ( cellLen < _minCellSize )
        _minCellSize = cellLen;
    }
  }
  if ( _minCellSize < Precision::Confusion() )
    throw SMESH_ComputeError (COMPERR_ALGO_FAILED,
                              SMESH_Comment("Too small cell size: ") << _minCellSize );
  _tol = _minCellSize / 1000.;

  // attune grid extremities to shape bounding box

  double sP[6]; // aXmin, aYmin, aZmin, aXmax, aYmax, aZmax
  shapeBox.Get(sP[0],sP[1],sP[2],sP[3],sP[4],sP[5]);
  double* cP[6] = { &_coords[0].front(), &_coords[1].front(), &_coords[2].front(),
                    &_coords[0].back(),  &_coords[1].back(),  &_coords[2].back() };
  for ( int i = 0; i < 6; ++i )
    if ( fabs( sP[i] - *cP[i] ) < _tol )
      *cP[i] = sP[i];// + _tol/1000. * ( i < 3 ? +1 : -1 );

  for ( int iDir = 0; iDir < 3; ++iDir )
  {
    if ( _coords[iDir][0] - sP[iDir] > _tol )
    {
      _minCellSize = Min( _minCellSize, _coords[iDir][0] - sP[iDir] );
      _coords[iDir].insert( _coords[iDir].begin(), sP[iDir] + _tol/1000.);
    }
    if ( sP[iDir+3] - _coords[iDir].back() > _tol  )
    {
      _minCellSize = Min( _minCellSize, sP[iDir+3] - _coords[iDir].back() );
      _coords[iDir].push_back( sP[iDir+3] - _tol/1000.);
    }
  }
  _tol = _minCellSize / 1000.;

  _origin = ( _coords[0][0] * _axes[0] +
              _coords[1][0] * _axes[1] +
              _coords[2][0] * _axes[2] );

  // create lines
  for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
  {
    LineIndexer li = GetLineIndexer( iDir );
    _lines[iDir].resize( li.NbLines() );
    double len = _coords[ iDir ].back() - _coords[iDir].front();
    for ( ; li.More(); ++li )
    {
      GridLine& gl = _lines[iDir][ li.LineIndex() ];
      gl._line.SetLocation( _coords[0][li.I()] * _axes[0] +
                            _coords[1][li.J()] * _axes[1] +
                            _coords[2][li.K()] * _axes[2] );
      gl._line.SetDirection( _axes[ iDir ]);
      gl._length = len;
    }
  }
}
//================================================================================
/*
  * Return local ID of shape
  */
TGeomID Grid::ShapeID( const TopoDS_Shape& s ) const
{
  return _helper->GetMeshDS()->ShapeToIndex( s );
}
//================================================================================
/*
  * Return a shape by its local ID
  */
const TopoDS_Shape& Grid::Shape( TGeomID id ) const
{
  return _helper->GetMeshDS()->IndexToShape( id );
}
//================================================================================
/*
  * Initialize _geometry
  */
void Grid::InitGeometry( const TopoDS_Shape& theShapeToMesh )
{
  SMESH_Mesh* mesh = _helper->GetMesh();

  _geometry._mainShape = theShapeToMesh;
  _geometry._extIntFaceID = mesh->GetMeshDS()->MaxShapeIndex() * 100;
  _geometry._soleSolid.SetID( 0 );
  _geometry._soleSolid.SetHasInternalFaces( false );

  InitClassifier( theShapeToMesh, TopAbs_VERTEX, _geometry._vertexClassifier );
  InitClassifier( theShapeToMesh, TopAbs_EDGE  , _geometry._edgeClassifier );

  TopExp_Explorer solidExp( theShapeToMesh, TopAbs_SOLID );

  bool isSeveralSolids = false;
  if ( _toConsiderInternalFaces ) // check nb SOLIDs
  {
    solidExp.Next();
    isSeveralSolids = solidExp.More();
    _toConsiderInternalFaces = isSeveralSolids;
    solidExp.ReInit();

    if ( !isSeveralSolids ) // look for an internal FACE
    {
      TopExp_Explorer fExp( theShapeToMesh, TopAbs_FACE );
      for ( ; fExp.More() &&  !_toConsiderInternalFaces; fExp.Next() )
        _toConsiderInternalFaces = ( fExp.Current().Orientation() == TopAbs_INTERNAL );

      _geometry._soleSolid.SetHasInternalFaces( _toConsiderInternalFaces );
      _geometry._soleSolid.SetID( ShapeID( solidExp.Current() ));
    }
    else // fill Geometry::_solidByID
    {
      for ( ; solidExp.More(); solidExp.Next() )
      {
        OneOfSolids & solid = _geometry._solidByID[ ShapeID( solidExp.Current() )];
        solid.Init( solidExp.Current(), TopAbs_FACE,   mesh->GetMeshDS() );
        solid.Init( solidExp.Current(), TopAbs_EDGE,   mesh->GetMeshDS() );
        solid.Init( solidExp.Current(), TopAbs_VERTEX, mesh->GetMeshDS() );
      }
    }
  }
  else
  {
    _geometry._soleSolid.SetID( ShapeID( solidExp.Current() ));
  }

  if ( !_toCreateFaces )
  {
    int nbSolidsGlobal = _helper->Count( mesh->GetShapeToMesh(), TopAbs_SOLID, false );
    int nbSolidsLocal  = _helper->Count( theShapeToMesh,         TopAbs_SOLID, false );
    _toCreateFaces = ( nbSolidsLocal < nbSolidsGlobal );
  }

  TopTools_IndexedMapOfShape faces;
  TopExp::MapShapes( theShapeToMesh, TopAbs_FACE, faces );

  // find boundary FACEs on boundary of mesh->ShapeToMesh()
  if ( _toCreateFaces )
    for ( int i = 1; i <= faces.Size(); ++i )
      if ( faces(i).Orientation() != TopAbs_INTERNAL &&
            _helper->NbAncestors( faces(i), *mesh, TopAbs_SOLID ) == 1 )
      {
        _geometry._boundaryFaces.Add( ShapeID( faces(i) ));
      }

  if ( isSeveralSolids )
    for ( int i = 1; i <= faces.Size(); ++i )
    {
      SetSolidFather( faces(i), theShapeToMesh );
      for ( TopExp_Explorer eExp( faces(i), TopAbs_EDGE ); eExp.More(); eExp.Next() )
      {
        const TopoDS_Edge& edge = TopoDS::Edge( eExp.Current() );
        SetSolidFather( edge, theShapeToMesh );
        SetSolidFather( _helper->IthVertex( 0, edge ), theShapeToMesh );
        SetSolidFather( _helper->IthVertex( 1, edge ), theShapeToMesh );
      }
    }

  // fill in _geometry._shape2NbNodes == find already meshed sub-shapes
  _geometry._shape2NbNodes.Clear();
  if ( mesh->NbNodes() > 0 )
  {
    for ( TopAbs_ShapeEnum type : { TopAbs_FACE, TopAbs_EDGE, TopAbs_VERTEX })
      for ( TopExp_Explorer exp( theShapeToMesh, type ); exp.More(); exp.Next() )
      {
        if ( _geometry._shape2NbNodes.IsBound( exp.Current() ))
          continue;
        if ( SMESHDS_SubMesh* sm = mesh->GetMeshDS()->MeshElements( exp.Current() ))
          if ( sm->NbNodes() > 0 )
            _geometry._shape2NbNodes.Bind( exp.Current(), sm->NbNodes() );
      }
  }

  // fill in Solid::_concaveVertex
  vector< TGeomID > soleSolidID( 1, _geometry._soleSolid.ID() );
  for ( int i = 1; i <= faces.Size(); ++i )
  {
    const TopoDS_Face& F = TopoDS::Face( faces( i ));
    TError error;
    TSideVector wires = StdMeshers_FaceSide::GetFaceWires( F, *mesh, 0, error,
                                                            nullptr, nullptr, false );
    for ( StdMeshers_FaceSidePtr& wire : wires )
    {
      const int nbEdges = wire->NbEdges();
      if ( nbEdges < 2 && SMESH_Algo::isDegenerated( wire->Edge(0)))
        continue;
      for ( int iE1 = 0; iE1 < nbEdges; ++iE1 )
      {
        if ( SMESH_Algo::isDegenerated( wire->Edge( iE1 ))) continue;
        int iE2 = ( iE1 + 1 ) % nbEdges;
        while ( SMESH_Algo::isDegenerated( wire->Edge( iE2 )))
          iE2 = ( iE2 + 1 ) % nbEdges;
        TopoDS_Vertex V = wire->FirstVertex( iE2 );
        double angle = _helper->GetAngle( wire->Edge( iE1 ),
                                          wire->Edge( iE2 ), F, V );
        if ( angle < -5. * M_PI / 180. )
        {
          TGeomID faceID = ShapeID( F );
          const vector< TGeomID > & solids =
            _geometry.IsOneSolid() ? soleSolidID : GetSolidIDs( faceID );
          for ( const TGeomID & solidID : solids )
          {
            Solid* solid = GetSolid( solidID );
            TGeomID   V1 = ShapeID( wire->FirstVertex( iE1 ));
            TGeomID   V2 = ShapeID( wire->LastVertex ( iE2 ));
            solid->SetConcave( ShapeID( V ), faceID,
                                wire->EdgeID( iE1 ), wire->EdgeID( iE2 ), V1, V2 );
          }
        }
      }
    }
  }

  return;
}
//================================================================================
/*
  * Store ID of SOLID as father of its child shape ID
  */
void Grid::SetSolidFather( const TopoDS_Shape& s, const TopoDS_Shape& theShapeToMesh )
{
  if ( _geometry._solidIDsByShapeID.empty() )
    _geometry._solidIDsByShapeID.resize( _helper->GetMeshDS()->MaxShapeIndex() + 1 );

  vector< TGeomID > & solidIDs = _geometry._solidIDsByShapeID[ ShapeID( s )];
  if ( !solidIDs.empty() )
    return;
  solidIDs.reserve(2);
  PShapeIteratorPtr solidIt = _helper->GetAncestors( s,
                                                      *_helper->GetMesh(),
                                                      TopAbs_SOLID,
                                                      & theShapeToMesh );
  while ( const TopoDS_Shape* solid = solidIt->next() )
    solidIDs.push_back( ShapeID( *solid ));
}
//================================================================================
/*
  * Return IDs of solids given sub-shape belongs to
  */
const vector< TGeomID > & Grid::GetSolidIDs( TGeomID subShapeID ) const
{
  return _geometry._solidIDsByShapeID[ subShapeID ];
}
//================================================================================
/*
  * Check if a sub-shape belongs to several SOLIDs
  */
bool Grid::IsShared( TGeomID shapeID ) const
{
  return !_geometry.IsOneSolid() && ( _geometry._solidIDsByShapeID[ shapeID ].size() > 1 );
}
//================================================================================
/*
  * Check if any of FACEs belongs to several SOLIDs
  */
bool Grid::IsAnyShared( const std::vector< TGeomID >& faceIDs ) const
{
  for ( size_t i = 0; i < faceIDs.size(); ++i )
    if ( IsShared( faceIDs[ i ]))
      return true;
  return false;
}
//================================================================================
/*
  * Return Solid by ID
  */
Solid* Grid::GetSolid( TGeomID solidID )
{
  if ( !solidID || _geometry.IsOneSolid() || _geometry._solidByID.empty() )
    return & _geometry._soleSolid;

  return & _geometry._solidByID[ solidID ];
}
//================================================================================
/*
  * Return OneOfSolids by ID
  */
Solid* Grid::GetOneOfSolids( TGeomID solidID )
{
  map< TGeomID, OneOfSolids >::iterator is2s = _geometry._solidByID.find( solidID );
  if ( is2s != _geometry._solidByID.end() )
    return & is2s->second;

  return & _geometry._soleSolid;
}
//================================================================================
/*
  * Check if transition on given FACE is correct for a given SOLID
  */
bool Grid::IsCorrectTransition( TGeomID faceID, const Solid* solid )
{
  if ( _geometry.IsOneSolid() )
    return true;

  const vector< TGeomID >& solidIDs = _geometry._solidIDsByShapeID[ faceID ];
  return solidIDs[0] == solid->ID();
}

//================================================================================
/*
  * Assign to geometry a node at FACE intersection
  * Return a found supporting VERTEX
  */
void Grid::SetOnShape( const SMDS_MeshNode* n, const F_IntersectPoint& ip, TopoDS_Vertex* vertex, bool unset )
{
  TopoDS_Shape s;
  SMESHDS_Mesh* mesh = _helper->GetMeshDS();
  if ( ip._faceIDs.size() == 1 )
  {
    mesh->SetNodeOnFace( n, ip._faceIDs[0], ip._u, ip._v );
  }
  else if ( _geometry._vertexClassifier.IsSatisfy( n, &s ))
  {
    if ( unset ) mesh->UnSetNodeOnShape( n );
    mesh->SetNodeOnVertex( n, TopoDS::Vertex( s ));
    if ( vertex )
      *vertex = TopoDS::Vertex( s );
  }
  else if ( _geometry._edgeClassifier.IsSatisfy( n, &s ))
  {
    if ( unset ) mesh->UnSetNodeOnShape( n );
    mesh->SetNodeOnEdge( n, TopoDS::Edge( s ));
  }
  else if ( ip._faceIDs.size() > 0 )
  {
    mesh->SetNodeOnFace( n, ip._faceIDs[0], ip._u, ip._v );
  }
  else if ( !unset && _geometry.IsOneSolid() )
  {
    mesh->SetNodeInVolume( n, _geometry._soleSolid.ID() );
  }
}
//================================================================================
/*
  * Fill in B_IntersectPoint::_faceIDs with all FACEs sharing a VERTEX
  */
void Grid::UpdateFacesOfVertex( const B_IntersectPoint& ip, const TopoDS_Vertex& vertex )
{
  if ( vertex.IsNull() )
    return;
  std::vector< int > faceID(1);
  PShapeIteratorPtr fIt = _helper->GetAncestors( vertex, *_helper->GetMesh(),
                                                  TopAbs_FACE, & _geometry._mainShape );
  while ( const TopoDS_Shape* face = fIt->next() )
  {
    faceID[ 0 ] = ShapeID( *face );
    ip.Add( faceID );
  }
}
//================================================================================
/*
  * Initialize a classifier
  */
void Grid::InitClassifier( const TopoDS_Shape&        mainShape,
                            TopAbs_ShapeEnum           shapeType,
                            Controls::ElementsOnShape& classifier )
{
  TopTools_IndexedMapOfShape shapes;
  TopExp::MapShapes( mainShape, shapeType, shapes );

  TopoDS_Compound compound; BRep_Builder builder;
  builder.MakeCompound( compound );
  for ( int i = 1; i <= shapes.Size(); ++i )
    builder.Add( compound, shapes(i) );

  classifier.SetMesh( _helper->GetMeshDS() );
  //classifier.SetTolerance( _tol ); // _tol is not initialised
  classifier.SetShape( compound, SMDSAbs_Node );
}

//================================================================================
/*
  * Return EDGEs with FACEs to implement into the mesh
  */
void Grid::GetEdgesToImplement( map< TGeomID, vector< TGeomID > > & edge2faceIDsMap,
                                const TopoDS_Shape&                 shape,
                                const vector< TopoDS_Shape >&       faces )
{
  // check if there are strange EDGEs
  TopTools_IndexedMapOfShape faceMap;
  TopExp::MapShapes( _helper->GetMesh()->GetShapeToMesh(), TopAbs_FACE, faceMap );
  int nbFacesGlobal = faceMap.Size();
  faceMap.Clear( false );
  TopExp::MapShapes( shape, TopAbs_FACE, faceMap );
  int nbFacesLocal  = faceMap.Size();
  bool hasStrangeEdges = ( nbFacesGlobal > nbFacesLocal );
  if ( !_toAddEdges && !hasStrangeEdges )
    return; // no FACEs in contact with those meshed by other algo

  for ( size_t i = 0; i < faces.size(); ++i )
  {
    _helper->SetSubShape( faces[i] );
    for ( TopExp_Explorer eExp( faces[i], TopAbs_EDGE ); eExp.More(); eExp.Next() )
    {
      const TopoDS_Edge& edge = TopoDS::Edge( eExp.Current() );
      if ( hasStrangeEdges )
      {
        bool hasStrangeFace = false;
        PShapeIteratorPtr faceIt = _helper->GetAncestors( edge, *_helper->GetMesh(), TopAbs_FACE);
        while ( const TopoDS_Shape* face = faceIt->next() )
          if (( hasStrangeFace = !faceMap.Contains( *face )))
            break;
        if ( !hasStrangeFace && !_toAddEdges )
          continue;
        _geometry._strangeEdges.Add( ShapeID( edge ));
        _geometry._strangeEdges.Add( ShapeID( _helper->IthVertex( 0, edge )));
        _geometry._strangeEdges.Add( ShapeID( _helper->IthVertex( 1, edge )));
      }
      if ( !SMESH_Algo::isDegenerated( edge ) &&
            !_helper->IsRealSeam( edge ))
      {
        edge2faceIDsMap[ ShapeID( edge )].push_back( ShapeID( faces[i] ));
      }
    }
  }
  return;
}

//================================================================================
/*
  * Computes coordinates of a point in the grid CS
  */
void Grid::ComputeUVW(const gp_XYZ& P, double UVW[3])
{
  gp_XYZ p = P * _invB;
  p.Coord( UVW[0], UVW[1], UVW[2] );
}
//================================================================================
/*
  * Creates all nodes
  */
void Grid::ComputeNodes(SMESH_MesherHelper& helper)
{
  // state of each node of the grid relative to the geometry
  const size_t nbGridNodes = _coords[0].size() * _coords[1].size() * _coords[2].size();
  vector< TGeomID > shapeIDVec( nbGridNodes, theUndefID );
  _nodes.resize( nbGridNodes, 0 );
  _allBorderNodes.resize( nbGridNodes, 0 );
  _gridIntP.resize( nbGridNodes, NULL );

  SMESHDS_Mesh* mesh = helper.GetMeshDS();

  for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
  {
    LineIndexer li = GetLineIndexer( iDir );

    // find out a shift of node index while walking along a GridLine in this direction
    li.SetIndexOnLine( 0 );
    size_t nIndex0 = NodeIndex( li.I(), li.J(), li.K() );
    li.SetIndexOnLine( 1 );
    const size_t nShift = NodeIndex( li.I(), li.J(), li.K() ) - nIndex0;
    
    const vector<double> & coords = _coords[ iDir ];
    for ( ; li.More(); ++li ) // loop on lines in iDir
    {
      li.SetIndexOnLine( 0 );
      nIndex0 = NodeIndex( li.I(), li.J(), li.K() );

      GridLine& line = _lines[ iDir ][ li.LineIndex() ];
      const gp_XYZ lineLoc = line._line.Location().XYZ();
      const gp_XYZ lineDir = line._line.Direction().XYZ();

      line.RemoveExcessIntPoints( _tol );
      multiset< F_IntersectPoint >&     intPnts = line._intPoints;
      multiset< F_IntersectPoint >::iterator ip = intPnts.begin();

      // Create mesh nodes at intersections with geometry
      // and set OUT state of nodes between intersections

      TGeomID solidID = 0;
      const double* nodeCoord = & coords[0];
      const double* coord0    = nodeCoord;
      const double* coordEnd  = coord0 + coords.size();
      double nodeParam = 0;
      for ( ; ip != intPnts.end(); ++ip )
      {
        solidID = line.GetSolidIDBefore( ip, solidID, _geometry );

        // set OUT state or just skip IN nodes before ip
        if ( nodeParam < ip->_paramOnLine - _tol )
        {
          while ( nodeParam < ip->_paramOnLine - _tol )
          {
            TGeomID & nodeShapeID = shapeIDVec[ nIndex0 + nShift * ( nodeCoord-coord0 ) ];
            nodeShapeID = Min( solidID, nodeShapeID );
            if ( ++nodeCoord <  coordEnd )
              nodeParam = *nodeCoord - *coord0;
            else
              break;                
          }
          if ( nodeCoord == coordEnd ) break;
        }
        
        // create a mesh node on a GridLine at ip if it does not coincide with a grid node
        if ( nodeParam > ip->_paramOnLine + _tol )
        {
          gp_XYZ xyz = lineLoc + ip->_paramOnLine * lineDir;
          ip->_node = mesh->AddNode( xyz.X(), xyz.Y(), xyz.Z() );
          ip->_indexOnLine = nodeCoord-coord0-1;
          TopoDS_Vertex v;
          SetOnShape( ip->_node, *ip, & v );
          UpdateFacesOfVertex( *ip, v );
        }
        // create a mesh node at ip coincident with a grid node
        else
        {
          int nodeIndex = nIndex0 + nShift * ( nodeCoord-coord0 );
          if ( !_nodes[ nodeIndex ] )
          {
            gp_XYZ xyz = lineLoc + nodeParam * lineDir;
            _nodes   [ nodeIndex ] = mesh->AddNode( xyz.X(), xyz.Y(), xyz.Z() );
            //_gridIntP[ nodeIndex ] = & * ip;
            //SetOnShape( _nodes[ nodeIndex ], *ip );
          }
          if ( _gridIntP[ nodeIndex ] )
            _gridIntP[ nodeIndex ]->Add( ip->_faceIDs );
          else
            _gridIntP[ nodeIndex ] = & (*ip);
          // ip->_node        = _nodes[ nodeIndex ]; -- to differ from ip on links
          ip->_indexOnLine = nodeCoord-coord0;
          if ( ++nodeCoord < coordEnd )
            nodeParam = *nodeCoord - *coord0;
        }
      }
      // set OUT state to nodes after the last ip
      for ( ; nodeCoord < coordEnd; ++nodeCoord )
        shapeIDVec[ nIndex0 + nShift * ( nodeCoord-coord0 ) ] = 0;
    }
  }

  // Create mesh nodes at !OUT nodes of the grid

  for ( size_t z = 0; z < _coords[2].size(); ++z )
    for ( size_t y = 0; y < _coords[1].size(); ++y )
      for ( size_t x = 0; x < _coords[0].size(); ++x )
      {
        size_t nodeIndex = NodeIndex( x, y, z );
        if ( !_nodes[ nodeIndex ] &&
              0 < shapeIDVec[ nodeIndex ] && shapeIDVec[ nodeIndex ] < theUndefID )
        {
          gp_XYZ xyz = ( _coords[0][x] * _axes[0] +
                          _coords[1][y] * _axes[1] +
                          _coords[2][z] * _axes[2] );
          _nodes[ nodeIndex ] = mesh->AddNode( xyz.X(), xyz.Y(), xyz.Z() );
          mesh->SetNodeInVolume( _nodes[ nodeIndex ], shapeIDVec[ nodeIndex ]);
        }
        else if ( _nodes[ nodeIndex ] && _gridIntP[ nodeIndex ] /*&&
                  !_nodes[ nodeIndex]->GetShapeID()*/ )
        {
          TopoDS_Vertex v;
          SetOnShape( _nodes[ nodeIndex ], *_gridIntP[ nodeIndex ], & v );
          UpdateFacesOfVertex( *_gridIntP[ nodeIndex ], v );
        }
        else if ( _toUseQuanta && !_allBorderNodes[ nodeIndex ] /*add all nodes outside the body. Used to reconstruct the hexahedrals when polys are not desired!*/)
        {
          gp_XYZ xyz = ( _coords[0][x] * _axes[0] +
                          _coords[1][y] * _axes[1] +
                          _coords[2][z] * _axes[2] );
          _allBorderNodes[ nodeIndex ] = mesh->AddNode( xyz.X(), xyz.Y(), xyz.Z() );
          mesh->SetNodeInVolume( _allBorderNodes[ nodeIndex ], shapeIDVec[ nodeIndex ]);
        }
      }
#ifdef _MY_DEBUG_
  // check validity of transitions
  const char* trName[] = { "TANGENT", "IN", "OUT", "APEX" };
  for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
  {
    LineIndexer li = GetLineIndexer( iDir );
    for ( ; li.More(); ++li )
    {
      multiset< F_IntersectPoint >& intPnts = _lines[ iDir ][ li.LineIndex() ]._intPoints;
      if ( intPnts.empty() ) continue;
      if ( intPnts.size() == 1 )
      {
        if ( intPnts.begin()->_transition != Trans_TANGENT &&
              intPnts.begin()->_transition != Trans_APEX )
          throw SMESH_ComputeError (COMPERR_ALGO_FAILED,
                                    SMESH_Comment("Wrong SOLE transition of GridLine (")
                                    << li._curInd[li._iVar1] << ", " << li._curInd[li._iVar2]
                                    << ") along " << li._nameConst
                                    << ": " << trName[ intPnts.begin()->_transition] );
      }
      else
      {
        if ( intPnts.begin()->_transition == Trans_OUT )
          throw SMESH_ComputeError (COMPERR_ALGO_FAILED,
                                    SMESH_Comment("Wrong START transition of GridLine (")
                                    << li._curInd[li._iVar1] << ", " << li._curInd[li._iVar2]
                                    << ") along " << li._nameConst
                                    << ": " << trName[ intPnts.begin()->_transition ]);
        if ( intPnts.rbegin()->_transition == Trans_IN )
          throw SMESH_ComputeError (COMPERR_ALGO_FAILED,
                                    SMESH_Comment("Wrong END transition of GridLine (")
                                    << li._curInd[li._iVar1] << ", " << li._curInd[li._iVar2]
                                    << ") along " << li._nameConst
                                    << ": " << trName[ intPnts.rbegin()->_transition ]);
      }
    }
  }
#endif
  return;
}

bool Grid::GridInitAndInterserctWithShape( const TopoDS_Shape& theShape,
                                           TEdge2faceIDsMap& edge2faceIDsMap, 
                                           const StdMeshers_CartesianParameters3D* hyp,
                                           const int /*numOfThreads*/, bool computeCanceled )
{
  InitGeometry( theShape );

  vector< TopoDS_Shape > faceVec;
  {
    TopTools_MapOfShape faceMap;
    TopExp_Explorer fExp;
    for ( fExp.Init( theShape, TopAbs_FACE ); fExp.More(); fExp.Next() )
    {
      bool isNewFace = faceMap.Add( fExp.Current() );
      if ( !_toConsiderInternalFaces )
        if ( !isNewFace || fExp.Current().Orientation() == TopAbs_INTERNAL )
          // remove an internal face
          faceMap.Remove( fExp.Current() );
    }
    faceVec.reserve( faceMap.Extent() );
    faceVec.assign( faceMap.cbegin(), faceMap.cend() );
  }
  vector<FaceGridIntersector> facesItersectors( faceVec.size() );
  Bnd_Box shapeBox;
  for ( size_t i = 0; i < faceVec.size(); ++i )
  {
    facesItersectors[i]._face   = TopoDS::Face( faceVec[i] );
    facesItersectors[i]._faceID = ShapeID( faceVec[i] );
    facesItersectors[i]._grid   = this;
    shapeBox.Add( facesItersectors[i].GetFaceBndBox() );
  }
  Tools::GetExactBndBox( faceVec, hyp->GetAxisDirs(), shapeBox );

  vector<double> xCoords, yCoords, zCoords;
  hyp->GetCoordinates( xCoords, yCoords, zCoords, shapeBox );
  
  SetCoordinates( xCoords, yCoords, zCoords, hyp->GetAxisDirs(), shapeBox );

  if ( computeCanceled ) return false;

#ifdef WITH_TBB
  { // copy partner faces and curves of not thread-safe types
    set< const Standard_Transient* > tshapes;
    BRepBuilderAPI_Copy copier;
    for ( size_t i = 0; i < facesItersectors.size(); ++i )
    {
      if ( !facesItersectors[i].IsThreadSafe( tshapes ))
      {
        copier.Perform( facesItersectors[i]._face );
        facesItersectors[i]._face = TopoDS::Face( copier );
      }
    }
  }
  // Intersection of grid lines with the geometry boundary.
  tbb::parallel_for ( tbb::blocked_range<size_t>( 0, facesItersectors.size() ),
                      ParallelIntersector( facesItersectors ),
                      tbb::simple_partitioner());
  
  // TODO SOLVE SEGFAULT WHEN USING THREAD PARALLEL FUNCTION

  // parallel_for(facesItersectors.begin(), facesItersectors.end(), computeGridIntersection<FaceGridIntersector>, numOfThreads );                      
#else
  for ( size_t i = 0; i < facesItersectors.size(); ++i )
    facesItersectors[i].Intersect();
#endif

  // put intersection points onto the GridLine's; this is done after intersection
  // to avoid contention of facesItersectors for writing into the same GridLine
  // in case of parallel work of facesItersectors
  for ( size_t i = 0; i < facesItersectors.size(); ++i )
    facesItersectors[i].StoreIntersections();

  if ( computeCanceled ) return false;

  // create nodes on the geometry
  ComputeNodes( *_helper );

  if ( computeCanceled ) return false;

  // get EDGEs to take into account
  // map< TGeomID, vector< TGeomID > > edge2faceIDsMap;
  GetEdgesToImplement( edge2faceIDsMap, theShape, faceVec );
  return true;
}

void Tools::GetExactBndBox( const vector< TopoDS_Shape >& faceVec, const double* axesDirs, Bnd_Box& shapeBox )
{
  BRep_Builder b;
  TopoDS_Compound allFacesComp;
  b.MakeCompound( allFacesComp );
  for ( size_t iF = 0; iF < faceVec.size(); ++iF )
    b.Add( allFacesComp, faceVec[ iF ] );

  double sP[6]; // aXmin, aYmin, aZmin, aXmax, aYmax, aZmax
  shapeBox.Get(sP[0],sP[1],sP[2],sP[3],sP[4],sP[5]);
  double farDist = 0;
  for ( int i = 0; i < 6; ++i )
    farDist = Max( farDist, 10 * sP[i] );

  gp_XYZ axis[3] = { gp_XYZ( axesDirs[0], axesDirs[1], axesDirs[2] ),
                      gp_XYZ( axesDirs[3], axesDirs[4], axesDirs[5] ),
                      gp_XYZ( axesDirs[6], axesDirs[7], axesDirs[8] ) };
  axis[0].Normalize();
  axis[1].Normalize();
  axis[2].Normalize();

  gp_Mat basis( axis[0], axis[1], axis[2] );
  gp_Mat bi = basis.Inverted();

  gp_Pnt pMin, pMax;
  for ( int iDir = 0; iDir < 3; ++iDir )
  {
    gp_XYZ axis0 = axis[ iDir ];
    gp_XYZ axis1 = axis[ ( iDir + 1 ) % 3 ];
    gp_XYZ axis2 = axis[ ( iDir + 2 ) % 3 ];
    for ( int isMax = 0; isMax < 2; ++isMax )
    {
      double shift = isMax ? farDist : -farDist;
      gp_XYZ orig = shift * axis0;
      gp_XYZ norm = axis1 ^ axis2;
      gp_Pln pln( orig, norm );
      norm = pln.Axis().Direction().XYZ();
      BRepBuilderAPI_MakeFace plane( pln, -farDist, farDist, -farDist, farDist );

      gp_Pnt& pAxis = isMax ? pMax : pMin;
      gp_Pnt pPlane, pFaces;
      double dist = GEOMUtils::GetMinDistance( plane, allFacesComp, pPlane, pFaces );
      if ( dist < 0 )
      {
        Bnd_B3d bb;
        gp_XYZ corner;
        for ( int i = 0; i < 2; ++i ) {
          corner.SetCoord( 1, sP[ i*3 ]);
          for ( int j = 0; j < 2; ++j ) {
            corner.SetCoord( 2, sP[ i*3 + 1 ]);
            for ( int k = 0; k < 2; ++k )
            {
              corner.SetCoord( 3, sP[ i*3 + 2 ]);
              corner *= bi;
              bb.Add( corner );
            }
          }
        }
        corner = isMax ? bb.CornerMax() : bb.CornerMin();
        pAxis.SetCoord( iDir+1, corner.Coord( iDir+1 ));
      }
      else
      {
        gp_XYZ pf = pFaces.XYZ() * bi;
        pAxis.SetCoord( iDir+1, pf.Coord( iDir+1 ) );
      }
    }
  } // loop on 3 axes

  shapeBox.SetVoid();
  shapeBox.Add( pMin );
  shapeBox.Add( pMax );

  return;
}

//=============================================================================
/*
 * Intersects TopoDS_Face with all GridLine's
 */
void FaceGridIntersector::Intersect()
{
  FaceLineIntersector intersector;
  intersector._surfaceInt = GetCurveFaceIntersector();
  intersector._tol        = _grid->_tol;
  intersector._transOut   = _face.Orientation() == TopAbs_REVERSED ? Trans_IN : Trans_OUT;
  intersector._transIn    = _face.Orientation() == TopAbs_REVERSED ? Trans_OUT : Trans_IN;

  typedef void (FaceLineIntersector::* PIntFun )(const GridLine& gridLine);
  PIntFun interFunction;

  bool isDirect = true;
  BRepAdaptor_Surface surf( _face );
  switch ( surf.GetType() ) {
  case GeomAbs_Plane:
    intersector._plane = surf.Plane();
    interFunction = &FaceLineIntersector::IntersectWithPlane;
    isDirect = intersector._plane.Direct();
    break;
  case GeomAbs_Cylinder:
    intersector._cylinder = surf.Cylinder();
    interFunction = &FaceLineIntersector::IntersectWithCylinder;
    isDirect = intersector._cylinder.Direct();
    break;
  case GeomAbs_Cone:
    intersector._cone = surf.Cone();
    interFunction = &FaceLineIntersector::IntersectWithCone;
    //isDirect = intersector._cone.Direct();
    break;
  case GeomAbs_Sphere:
    intersector._sphere = surf.Sphere();
    interFunction = &FaceLineIntersector::IntersectWithSphere;
    isDirect = intersector._sphere.Direct();
    break;
  case GeomAbs_Torus:
    intersector._torus = surf.Torus();
    interFunction = &FaceLineIntersector::IntersectWithTorus;
    //isDirect = intersector._torus.Direct();
    break;
  default:
    interFunction = &FaceLineIntersector::IntersectWithSurface;
  }
  if ( !isDirect )
    std::swap( intersector._transOut, intersector._transIn );

  _intersections.clear();
  for ( int iDir = 0; iDir < 3; ++iDir ) // loop on 3 line directions
  {
    if ( surf.GetType() == GeomAbs_Plane )
    {
      // check if all lines in this direction are parallel to a plane
      if ( intersector._plane.Axis().IsNormal( _grid->_lines[iDir][0]._line.Position(),
                                               Precision::Angular()))
        continue;
      // find out a transition, that is the same for all lines of a direction
      gp_Dir plnNorm = intersector._plane.Axis().Direction();
      gp_Dir lineDir = _grid->_lines[iDir][0]._line.Direction();
      intersector._transition =
        ( plnNorm * lineDir < 0 ) ? intersector._transIn : intersector._transOut;
    }
    if ( surf.GetType() == GeomAbs_Cylinder )
    {
      // check if all lines in this direction are parallel to a cylinder
      if ( intersector._cylinder.Axis().IsParallel( _grid->_lines[iDir][0]._line.Position(),
                                                    Precision::Angular()))
        continue;
    }

    // intersect the grid lines with the face
    for ( size_t iL = 0; iL < _grid->_lines[iDir].size(); ++iL )
    {
      GridLine& gridLine = _grid->_lines[iDir][iL];
      if ( _bndBox.IsOut( gridLine._line )) continue;

      intersector._intPoints.clear();
      (intersector.*interFunction)( gridLine ); // <- intersection with gridLine
      for ( size_t i = 0; i < intersector._intPoints.size(); ++i )
        _intersections.push_back( std::make_pair( &gridLine, intersector._intPoints[i] ));
    }
  }

  if ( _face.Orientation() == TopAbs_INTERNAL )
  {
    for ( size_t i = 0; i < _intersections.size(); ++i )
      if ( _intersections[i].second._transition == Trans_IN ||
           _intersections[i].second._transition == Trans_OUT )
      {
        _intersections[i].second._transition = Trans_INTERNAL;
      }
  }
  return;
}

#ifdef WITH_TBB
//================================================================================
/*
 * check if its face can be safely intersected in a thread
 */
bool FaceGridIntersector::IsThreadSafe(std::set< const Standard_Transient* >& noSafeTShapes) const
{
  bool isSafe = true;

  // check surface
  TopLoc_Location loc;
  Handle(Geom_Surface) surf = BRep_Tool::Surface( _face, loc );
  Handle(Geom_RectangularTrimmedSurface) ts =
    Handle(Geom_RectangularTrimmedSurface)::DownCast( surf );
  while( !ts.IsNull() ) {
    surf = ts->BasisSurface();
    ts = Handle(Geom_RectangularTrimmedSurface)::DownCast(surf);
  }
  if ( surf->IsKind( STANDARD_TYPE(Geom_BSplineSurface )) ||
       surf->IsKind( STANDARD_TYPE(Geom_BezierSurface )))
    if ( !noSafeTShapes.insert( _face.TShape().get() ).second )
      isSafe = false;

  double f, l;
  TopExp_Explorer exp( _face, TopAbs_EDGE );
  for ( ; exp.More(); exp.Next() )
  {
    bool edgeIsSafe = true;
    const TopoDS_Edge& e = TopoDS::Edge( exp.Current() );
    // check 3d curve
    {
      Handle(Geom_Curve) c = BRep_Tool::Curve( e, loc, f, l);
      if ( !c.IsNull() )
      {
        Handle(Geom_TrimmedCurve) tc = Handle(Geom_TrimmedCurve)::DownCast(c);
        while( !tc.IsNull() ) {
          c = tc->BasisCurve();
          tc = Handle(Geom_TrimmedCurve)::DownCast(c);
        }
        if ( c->IsKind( STANDARD_TYPE(Geom_BSplineCurve )) ||
             c->IsKind( STANDARD_TYPE(Geom_BezierCurve )))
          edgeIsSafe = false;
      }
    }
    // check 2d curve
    if ( edgeIsSafe )
    {
      Handle(Geom2d_Curve) c2 = BRep_Tool::CurveOnSurface( e, surf, loc, f, l);
      if ( !c2.IsNull() )
      {
        Handle(Geom2d_TrimmedCurve) tc = Handle(Geom2d_TrimmedCurve)::DownCast(c2);
        while( !tc.IsNull() ) {
          c2 = tc->BasisCurve();
          tc = Handle(Geom2d_TrimmedCurve)::DownCast(c2);
        }
        if ( c2->IsKind( STANDARD_TYPE(Geom2d_BSplineCurve )) ||
             c2->IsKind( STANDARD_TYPE(Geom2d_BezierCurve )))
          edgeIsSafe = false;
      }
    }
    if ( !edgeIsSafe && !noSafeTShapes.insert( e.TShape().get() ).second )
      isSafe = false;
  }
  return isSafe;
}
#endif

//================================================================================
/*
 * Store an intersection if it is IN or ON the face
 */
void FaceLineIntersector::addIntPoint(const bool toClassify)
{
  if ( !toClassify || UVIsOnFace() )
  {
    F_IntersectPoint p;
    p._paramOnLine = _w;
    p._u           = _u;
    p._v           = _v;
    p._transition  = _transition;
    _intPoints.push_back( p );
  }
}

//================================================================================
/*
 * Intersect a line with a plane
 */
void FaceLineIntersector::IntersectWithPlane(const GridLine& gridLine)
{
  IntAna_IntConicQuad linPlane( gridLine._line, _plane, Precision::Angular());
  _w = linPlane.ParamOnConic(1);
  if ( isParamOnLineOK( gridLine._length ))
  {
    ElSLib::Parameters(_plane, linPlane.Point(1) ,_u,_v);
    addIntPoint();
  }
}

//================================================================================
/*
 * Intersect a line with a cylinder
 */
void FaceLineIntersector::IntersectWithCylinder(const GridLine& gridLine)
{
  IntAna_IntConicQuad linCylinder( gridLine._line, _cylinder );
  if ( linCylinder.IsDone() && linCylinder.NbPoints() > 0 )
  {
    _w = linCylinder.ParamOnConic(1);
    if ( linCylinder.NbPoints() == 1 )
      _transition = Trans_TANGENT;
    else
      _transition = _w < linCylinder.ParamOnConic(2) ? _transIn : _transOut;
    if ( isParamOnLineOK( gridLine._length ))
    {
      ElSLib::Parameters(_cylinder, linCylinder.Point(1) ,_u,_v);
      addIntPoint();
    }
    if ( linCylinder.NbPoints() > 1 )
    {
      _w = linCylinder.ParamOnConic(2);
      if ( isParamOnLineOK( gridLine._length ))
      {
        ElSLib::Parameters(_cylinder, linCylinder.Point(2) ,_u,_v);
        _transition = ( _transition == Trans_OUT ) ? Trans_IN : Trans_OUT;
        addIntPoint();
      }
    }
  }
}

//================================================================================
/*
 * Intersect a line with a cone
 */
void FaceLineIntersector::IntersectWithCone (const GridLine& gridLine)
{
  IntAna_IntConicQuad linCone(gridLine._line,_cone);
  if ( !linCone.IsDone() ) return;
  gp_Pnt P;
  gp_Vec du, dv, norm;
  for ( int i = 1; i <= linCone.NbPoints(); ++i )
  {
    _w = linCone.ParamOnConic( i );
    if ( !isParamOnLineOK( gridLine._length )) continue;
    ElSLib::Parameters(_cone, linCone.Point(i) ,_u,_v);
    if ( UVIsOnFace() )
    {
      ElSLib::D1( _u, _v, _cone, P, du, dv );
      norm = du ^ dv;
      double normSize2 = norm.SquareMagnitude();
      if ( normSize2 > Precision::Angular() * Precision::Angular() )
      {
        double cos = norm.XYZ() * gridLine._line.Direction().XYZ();
        cos /= sqrt( normSize2 );
        if ( cos < -Precision::Angular() )
          _transition = _transIn;
        else if ( cos > Precision::Angular() )
          _transition = _transOut;
        else
          _transition = Trans_TANGENT;
      }
      else
      {
        _transition = Trans_APEX;
      }
      addIntPoint( /*toClassify=*/false);
    }
  }
}

//================================================================================
/*
 * Intersect a line with a sphere
 */
void FaceLineIntersector::IntersectWithSphere  (const GridLine& gridLine)
{
  IntAna_IntConicQuad linSphere(gridLine._line,_sphere);
  if ( linSphere.IsDone() && linSphere.NbPoints() > 0 )
  {
    _w = linSphere.ParamOnConic(1);
    if ( linSphere.NbPoints() == 1 )
      _transition = Trans_TANGENT;
    else
      _transition = _w < linSphere.ParamOnConic(2) ? _transIn : _transOut;
    if ( isParamOnLineOK( gridLine._length ))
    {
      ElSLib::Parameters(_sphere, linSphere.Point(1) ,_u,_v);
      addIntPoint();
    }
    if ( linSphere.NbPoints() > 1 )
    {
      _w = linSphere.ParamOnConic(2);
      if ( isParamOnLineOK( gridLine._length ))
      {
        ElSLib::Parameters(_sphere, linSphere.Point(2) ,_u,_v);
        _transition = ( _transition == Trans_OUT ) ? Trans_IN : Trans_OUT;
        addIntPoint();
      }
    }
  }
}

//================================================================================
/*
 * Intersect a line with a torus
 */
void FaceLineIntersector::IntersectWithTorus (const GridLine& gridLine)
{
  IntAna_IntLinTorus linTorus(gridLine._line,_torus);
  if ( !linTorus.IsDone()) return;
  gp_Pnt P;
  gp_Vec du, dv, norm;
  for ( int i = 1; i <= linTorus.NbPoints(); ++i )
  {
    _w = linTorus.ParamOnLine( i );
    if ( !isParamOnLineOK( gridLine._length )) continue;
    linTorus.ParamOnTorus( i, _u,_v );
    if ( UVIsOnFace() )
    {
      ElSLib::D1( _u, _v, _torus, P, du, dv );
      norm = du ^ dv;
      double normSize = norm.Magnitude();
      double cos = norm.XYZ() * gridLine._line.Direction().XYZ();
      cos /= normSize;
      if ( cos < -Precision::Angular() )
        _transition = _transIn;
      else if ( cos > Precision::Angular() )
        _transition = _transOut;
      else
        _transition = Trans_TANGENT;
      addIntPoint( /*toClassify=*/false);
    }
  }
}

//================================================================================
/*
 * Intersect a line with a non-analytical surface
 */
void FaceLineIntersector::IntersectWithSurface (const GridLine& gridLine)
{
//#define OLD_INTERSECTOR
#ifdef OLD_INTERSECTOR
  _surfaceInt->Perform( gridLine._line, 0.0, gridLine._length );
  if ( !_surfaceInt->IsDone() ) return;
  for ( int i = 1; i <= _surfaceInt->NbPnt(); ++i )
  {
    _transition = Transition( _surfaceInt->Transition( i ) );
    _w = _surfaceInt->WParameter( i );
    addIntPoint(/*toClassify=*/false);
  }
#else
  double tol = 1e-6;
  //double tol = Precision::Confusion();
  BRepIntCurveSurface_Inter theAlg;
  theAlg.Init(_surfaceInt->Face(), gridLine._line, tol);

  for (; theAlg.More(); theAlg.Next())
  {
    if (theAlg.Transition() == IntCurveSurface_In)
      _transition = _transIn;
    else if (theAlg.Transition() == IntCurveSurface_Out)
      _transition = _transOut;
    else
      _transition = Transition( theAlg.Transition() );

    _u = theAlg.U();
    _v = theAlg.V();
    _w = theAlg.W();
    addIntPoint(/*toClassify=*/false);
  }
#endif
}
