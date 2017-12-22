// Copyright (C) 2010-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

#include "SMDS_MeshCell.hxx"

#include "SMDS_Mesh.hxx"
#include "SMDS_VtkCellIterator.hxx"

#include <utilities.h>

#include <vtkCell.h>

#include <cstdarg>

#include <boost/make_shared.hpp>

namespace
{
  /*!
   * \brief Cell type features
   */
  struct CellProps
  {
    SMDSAbs_EntityType   myEntity;
    SMDSAbs_ElementType  myType;
    SMDSAbs_GeometryType myGeom;
    bool                 myIsPoly;
    int                  myNbCornerNodes;
    int                  myNbNodes;
    int                  myNbEdges;
    int                  myNbFaces;

    CellProps() :
      myEntity( SMDSEntity_Last ), myType( SMDSAbs_All ), myGeom( SMDSGeom_NONE ),
      myIsPoly( 0 ), myNbCornerNodes( 0 ),
      myNbNodes( 0 ), myNbEdges( 0 ), myNbFaces ( 0 )
    {
    }
    void Set( SMDSAbs_EntityType   Entity,
              SMDSAbs_ElementType  Type,
              SMDSAbs_GeometryType Geom,
              bool                 IsPoly,
              int                  NbCornerNodes,
              int                  NbNodes,
              int                  NbEdges,
              int                  NbFaces)
    {
      myEntity        = Entity;
      myType          = Type;
      myGeom          = Geom;
      myIsPoly        = IsPoly;
      myNbCornerNodes = NbCornerNodes;
      myNbNodes       = NbNodes;
      myNbEdges       = NbEdges;
      myNbFaces       = NbFaces;
    }
    bool IsQuadratic() const { return myNbNodes > myNbCornerNodes; }
  };

  //! return vector a CellProps
  const CellProps& getCellProps( VTKCellType vtkType )
  {
    static std::vector< CellProps > theCellProps;
    if ( theCellProps.empty() )
    {
      theCellProps.resize( VTK_NUMBER_OF_CELL_TYPES );
      CellProps* p = & theCellProps[0];
      p[ VTK_VERTEX ].
        Set( SMDSEntity_0D, SMDSAbs_0DElement, SMDSGeom_POINT,
             /*isPoly=*/0,/*nbCN=*/1,/*nbN=*/1,/*nbE=*/0,/*nbF=*/0 );
      p[ VTK_LINE ].
        Set( SMDSEntity_Edge, SMDSAbs_Edge, SMDSGeom_EDGE,
             /*isPoly=*/0,/*nbCN=*/2,/*nbN=*/2,/*nbE=*/1,/*nbF=*/0 );
      p[ VTK_QUADRATIC_EDGE ].
        Set( SMDSEntity_Quad_Edge, SMDSAbs_Edge, SMDSGeom_EDGE,
             /*isPoly=*/0,/*nbCN=*/2,/*nbN=*/3,/*nbE=*/1,/*nbF=*/0 );
      p[ VTK_TRIANGLE ].
        Set( SMDSEntity_Triangle, SMDSAbs_Face, SMDSGeom_TRIANGLE,
             /*isPoly=*/0,/*nbCN=*/3,/*nbN=*/3,/*nbE=*/3,/*nbF=*/1 );
      p[ VTK_QUADRATIC_TRIANGLE ].
        Set( SMDSEntity_Quad_Triangle, SMDSAbs_Face, SMDSGeom_TRIANGLE,
             /*isPoly=*/0,/*nbCN=*/3,/*nbN=*/6,/*nbE=*/3,/*nbF=*/1 );
      p[ VTK_BIQUADRATIC_TRIANGLE ].
        Set( SMDSEntity_BiQuad_Triangle, SMDSAbs_Face, SMDSGeom_TRIANGLE,
             /*isPoly=*/0,/*nbCN=*/3,/*nbN=*/7,/*nbE=*/3,/*nbF=*/1 );
      p[ VTK_QUAD].
        Set( SMDSEntity_Quadrangle, SMDSAbs_Face, SMDSGeom_QUADRANGLE,
             /*isPoly=*/0,/*nbCN=*/4,/*nbN=*/4,/*nbE=*/4,/*nbF=*/1 );
      p[ VTK_QUADRATIC_QUAD].
        Set( SMDSEntity_Quad_Quadrangle, SMDSAbs_Face, SMDSGeom_QUADRANGLE,
             /*isPoly=*/0,/*nbCN=*/4,/*nbN=*/8,/*nbE=*/4,/*nbF=*/1 );
      p[ VTK_BIQUADRATIC_QUAD].
        Set( SMDSEntity_BiQuad_Quadrangle, SMDSAbs_Face, SMDSGeom_QUADRANGLE,
             /*isPoly=*/0,/*nbCN=*/4,/*nbN=*/9,/*nbE=*/4,/*nbF=*/1 );
      p[ VTK_POLYGON ].
        Set( SMDSEntity_Polygon, SMDSAbs_Face, SMDSGeom_POLYGON,
             /*isPoly=*/1,/*nbCN=*/-1,/*nbN=*/-1,/*nbE=*/-1,/*nbF=*/1 );
      p[ VTK_QUADRATIC_POLYGON ].
        Set( SMDSEntity_Quad_Polygon, SMDSAbs_Face, SMDSGeom_POLYGON,
             /*isPoly=*/1,/*nbCN=*/-2,/*nbN=*/-1,/*nbE=*/-1,/*nbF=*/1 );
      p[ VTK_TETRA ].
        Set( SMDSEntity_Tetra, SMDSAbs_Volume, SMDSGeom_TETRA,
             /*isPoly=*/0,/*nbCN=*/4,/*nbN=*/4,/*nbE=*/6,/*nbF=*/4 );
      p[ VTK_QUADRATIC_TETRA ].
        Set( SMDSEntity_Quad_Tetra, SMDSAbs_Volume, SMDSGeom_TETRA,
             /*isPoly=*/0,/*nbCN=*/4,/*nbN=*/10,/*nbE=*/6,/*nbF=*/4 );
      p[ VTK_PYRAMID ].
        Set( SMDSEntity_Pyramid, SMDSAbs_Volume, SMDSGeom_PYRAMID,
             /*isPoly=*/0,/*nbCN=*/5,/*nbN=*/5,/*nbE=*/8,/*nbF=*/5 );
      p[ VTK_QUADRATIC_PYRAMID].
        Set( SMDSEntity_Quad_Pyramid, SMDSAbs_Volume, SMDSGeom_PYRAMID,
             /*isPoly=*/0,/*nbCN=*/5,/*nbN=*/13,/*nbE=*/8,/*nbF=*/5 );
      p[ VTK_HEXAHEDRON ].
        Set( SMDSEntity_Hexa, SMDSAbs_Volume, SMDSGeom_HEXA,
             /*isPoly=*/0,/*nbCN=*/8,/*nbN=*/8,/*nbE=*/12,/*nbF=*/6 );
      p[ VTK_QUADRATIC_HEXAHEDRON ].
        Set( SMDSEntity_Quad_Hexa, SMDSAbs_Volume, SMDSGeom_HEXA,
             /*isPoly=*/0,/*nbCN=*/8,/*nbN=*/20,/*nbE=*/12,/*nbF=*/6 );
      p[ VTK_TRIQUADRATIC_HEXAHEDRON ].
        Set( SMDSEntity_TriQuad_Hexa, SMDSAbs_Volume, SMDSGeom_HEXA,
             /*isPoly=*/0,/*nbCN=*/8,/*nbN=*/27,/*nbE=*/12,/*nbF=*/6 );
      p[ VTK_WEDGE ].
        Set( SMDSEntity_Penta, SMDSAbs_Volume, SMDSGeom_PENTA,
             /*isPoly=*/0,/*nbCN=*/6,/*nbN=*/6,/*nbE=*/9,/*nbF=*/5 );
      p[ VTK_QUADRATIC_WEDGE ].
        Set( SMDSEntity_Quad_Penta, SMDSAbs_Volume, SMDSGeom_PENTA,
             /*isPoly=*/0,/*nbCN=*/6,/*nbN=*/15,/*nbE=*/9,/*nbF=*/5 );
      p[ VTK_BIQUADRATIC_QUADRATIC_WEDGE ].
        Set( SMDSEntity_BiQuad_Penta, SMDSAbs_Volume, SMDSGeom_PENTA,
             /*isPoly=*/0,/*nbCN=*/6,/*nbN=*/21,/*nbE=*/9,/*nbF=*/5 );
      p[ VTK_HEXAGONAL_PRISM].
        Set( SMDSEntity_Hexagonal_Prism, SMDSAbs_Volume, SMDSGeom_HEXAGONAL_PRISM,
             /*isPoly=*/0,/*nbCN=*/12,/*nbN=*/12,/*nbE=*/18,/*nbF=*/8 );
      p[ VTK_POLYHEDRON ].
        Set( SMDSEntity_Polyhedra, SMDSAbs_Volume, SMDSGeom_POLYHEDRA,
             /*isPoly=*/1,/*nbCN=*/-1,/*nbN=*/-1,/*nbE=*/-1,/*nbF=*/-1 );
      p[ VTK_POLY_VERTEX].
        Set( SMDSEntity_Ball, SMDSAbs_Ball, SMDSGeom_BALL,
             /*isPoly=*/0,/*nbCN=*/1,/*nbN=*/1,/*nbE=*/0,/*nbF=*/0 );
    }
    return theCellProps[ vtkType ];

  } // getCellProps()

  //! return vector a CellProps
  const CellProps& getCellProps( SMDSAbs_EntityType entity )
  {
    return getCellProps( SMDS_MeshCell::toVtkType( entity ));
  }
  
} // namespace

void SMDS_MeshCell::InitStaticMembers()
{
  getCellProps( SMDSEntity_Ball );
  toVtkOrder( SMDSEntity_Ball );
  reverseSmdsOrder( SMDSEntity_Ball, 1 );
  interlacedSmdsOrder( SMDSEntity_Ball, 1 );
  fromVtkOrder( SMDSEntity_Ball );
}

void SMDS_MeshCell::init( SMDSAbs_EntityType theEntity, int theNbNodes, ... )
{
  ASSERT( getCellProps( theEntity ).myNbNodes == theNbNodes ||
          getCellProps( theEntity ).myIsPoly);

  va_list vl;
  va_start( vl, theNbNodes );

  vtkIdType vtkIds[ VTK_CELL_SIZE ];
  typedef const SMDS_MeshNode* node_t;

  const std::vector<int>& interlace = toVtkOrder( theEntity );
  if ((int) interlace.size() == theNbNodes )
  {
    const SMDS_MeshNode* nodes[ VTK_CELL_SIZE ];
    for ( int i = 0; i < theNbNodes; i++ )
      nodes[i] = va_arg( vl, node_t );

    for ( int i = 0; i < theNbNodes; i++ )
      vtkIds[i] = nodes[ interlace[i] ]->GetVtkID();
  }
  else
  {
    for ( int i = 0; i < theNbNodes; i++ )
      vtkIds[i] = va_arg( vl, node_t )->GetVtkID();
  }
  va_end( vl );

  int vtkType = toVtkType( theEntity );
  int   vtkID = getGrid()->InsertNextLinkedCell( vtkType, theNbNodes, vtkIds );
  setVtkID( vtkID );
}

void SMDS_MeshCell::init( SMDSAbs_EntityType                       theEntity,
                          const std::vector<const SMDS_MeshNode*>& nodes )
{
  std::vector< vtkIdType > vtkIds( nodes.size() );
  for ( size_t i = 0; i < nodes.size(); ++i )
    vtkIds[i] = nodes[i]->GetVtkID();

  int vtkType = toVtkType( theEntity );
  int   vtkID = getGrid()->InsertNextLinkedCell( vtkType, nodes.size(), &vtkIds[0] );
  setVtkID( vtkID );
}

void SMDS_MeshCell::init( SMDSAbs_EntityType            theEntity,
                          const std::vector<vtkIdType>& vtkNodeIds )
{
  int vtkType = toVtkType( theEntity );
  int   vtkID = getGrid()->InsertNextLinkedCell( vtkType, vtkNodeIds.size(),
                                                 const_cast< vtkIdType* > ( &vtkNodeIds[0] ));
  setVtkID( vtkID );
}

bool SMDS_MeshCell::ChangeNodes(const SMDS_MeshNode* nodes[], const int theNbNodes)
{
  vtkIdType npts = 0;
  vtkIdType* pts = 0;
  getGrid()->GetCellPoints( GetVtkID(), npts, pts );
  if ( theNbNodes != npts )
  {
    MESSAGE("ChangeNodes problem: not the same number of nodes " << npts << " -> " << theNbNodes);
    return false;
  }
  const std::vector<int>& interlace = toVtkOrder((VTKCellType) GetVtkType() );
  if ((int) interlace.size() != theNbNodes )
    for ( int i = 0; i < theNbNodes; i++ )
    {
      pts[i] = nodes[i]->GetVtkID();
    }
  else
    for ( int i = 0; i < theNbNodes; i++ )
    {
      pts[i] = nodes[ interlace[i] ]->GetVtkID();
    }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
///Return The number of nodes owned by the current element
///////////////////////////////////////////////////////////////////////////////
int SMDS_MeshCell::NbNodes() const
{
  if ( GetEntityType() == SMDSEntity_Polyhedra )
    return static_cast< const SMDS_MeshVolume* >( this )->SMDS_MeshVolume::NbNodes();
  vtkIdType *pts, npts;
  getGrid()->GetCellPoints( GetVtkID(), npts, pts );
  return npts;
}

int SMDS_MeshCell::NbFaces() const
{
  if ( GetEntityType() == SMDSEntity_Polyhedra )
    return static_cast< const SMDS_MeshVolume* >( this )->SMDS_MeshVolume::NbFaces();
  return getCellProps( GetVtkType() ).myNbFaces;
}

int SMDS_MeshCell::NbEdges() const
{
  switch ( GetEntityType() )
  {
  case SMDSEntity_Polyhedra:
    return static_cast< const SMDS_MeshVolume* >( this )->SMDS_MeshVolume::NbEdges();
  case SMDSEntity_Polygon:
    return NbNodes();
  case SMDSEntity_Quad_Polygon:
    return NbNodes() / 2;
  default:;
  }
  return getCellProps( GetVtkType() ).myNbEdges;
}

int SMDS_MeshCell::NbCornerNodes() const
{
  switch ( GetEntityType() )
  {
  case SMDSEntity_Polyhedra:
    return static_cast< const SMDS_MeshVolume* >( this )->SMDS_MeshVolume::NbCornerNodes();
  case SMDSEntity_Polygon:
    return NbNodes();
  case SMDSEntity_Quad_Polygon:
    return NbNodes() / 2;
  default:;
  }
  return getCellProps( GetVtkType() ).myNbCornerNodes;
}

///////////////////////////////////////////////////////////////////////////////
/// Create an iterator which iterate on nodes owned by the element.
///////////////////////////////////////////////////////////////////////////////
SMDS_ElemIteratorPtr SMDS_MeshCell::nodesIterator() const
{
  if ( GetEntityType() == SMDSEntity_Polyhedra )
    return static_cast< const SMDS_MeshVolume* >( this )->SMDS_MeshVolume::nodesIterator();

  return boost::make_shared< SMDS_VtkCellIterator<> >( GetMesh(), GetVtkID(), GetEntityType());
}

///////////////////////////////////////////////////////////////////////////////
/// Create an iterator which iterate on nodes owned by the element.
///////////////////////////////////////////////////////////////////////////////
SMDS_NodeIteratorPtr SMDS_MeshCell::nodeIterator() const
{
  if ( GetEntityType() == SMDSEntity_Polyhedra )
    return static_cast< const SMDS_MeshVolume* >( this )->SMDS_MeshVolume::nodeIterator();

  return SMDS_NodeIteratorPtr
    (new SMDS_VtkCellIterator<SMDS_NodeIterator>( GetMesh(), GetVtkID(), GetEntityType()));
}

SMDS_NodeIteratorPtr SMDS_MeshCell::interlacedNodesIterator() const
{
  bool canInterlace = ( GetType() == SMDSAbs_Face || GetType() == SMDSAbs_Edge );
  return canInterlace ? nodesIteratorToUNV() : nodeIterator();
}

SMDS_NodeIteratorPtr SMDS_MeshCell::nodesIteratorToUNV() const
{
  return SMDS_NodeIteratorPtr
    (new SMDS_VtkCellIteratorToUNV<SMDS_NodeIterator>( GetMesh(), GetVtkID(), GetEntityType()));
}

SMDSAbs_ElementType SMDS_MeshCell::GetType() const
{
  return ElemType( GetEntityType() );
}

SMDSAbs_EntityType SMDS_MeshCell::GetEntityType() const
{
  return toSmdsType( (VTKCellType) GetVtkType() );
}

SMDSAbs_GeometryType SMDS_MeshCell::GetGeomType() const
{
  return getCellProps( GetVtkType() ).myGeom;
}

VTKCellType SMDS_MeshCell::GetVtkType() const
{
  return (VTKCellType) getGrid()->GetCellType( GetVtkID() );
}

bool SMDS_MeshCell::IsPoly() const
{
  return getCellProps( GetVtkType() ).myIsPoly;
}

bool SMDS_MeshCell::IsQuadratic() const
{
  return getCellProps( GetVtkType() ).IsQuadratic();
}

const SMDS_MeshNode* SMDS_MeshCell::GetNode(const int ind) const
{
  if ( GetEntityType() == SMDSEntity_Polyhedra )
    return static_cast< const SMDS_MeshVolume* >( this )->SMDS_MeshVolume::GetNode( ind );

  vtkIdType npts, *pts;
  getGrid()->GetCellPoints( GetVtkID(), npts, pts );
  const std::vector<int>& interlace = SMDS_MeshCell::fromVtkOrder( VTKCellType( GetVtkType() ));
  return GetMesh()->FindNodeVtk( pts[ interlace.empty() ? ind : interlace[ ind ]]);
}

int SMDS_MeshCell::GetNodeIndex( const SMDS_MeshNode* node ) const
{
  if ( GetEntityType() == SMDSEntity_Polyhedra )
    return static_cast< const SMDS_MeshVolume* >( this )->SMDS_MeshVolume::GetNodeIndex( node );

  vtkIdType npts, *pts;
  getGrid()->GetCellPoints( GetVtkID(), npts, pts );
  for ( vtkIdType i = 0; i < npts; ++i )
    if ( pts[i] == node->GetVtkID() )
    {
      const std::vector<int>& interlace = SMDS_MeshCell::toVtkOrder( VTKCellType( GetVtkType() ));
      return interlace.empty() ? i : interlace[i];
    }
  return -1;
}

//================================================================================
/*!
 * \brief Return VTKCellType corresponding to SMDSAbs_EntityType
 */
//================================================================================

VTKCellType SMDS_MeshCell::toVtkType (SMDSAbs_EntityType smdsType)
{
  static std::vector< VTKCellType > vtkTypes;
  if ( vtkTypes.empty() )
  {
    vtkTypes.resize( SMDSEntity_Last+1, VTK_EMPTY_CELL );
    vtkTypes[ SMDSEntity_Node ]              = VTK_VERTEX;
    vtkTypes[ SMDSEntity_0D ]                = VTK_VERTEX;
    vtkTypes[ SMDSEntity_Edge ]              = VTK_LINE;
    vtkTypes[ SMDSEntity_Quad_Edge ]         = VTK_QUADRATIC_EDGE;
    vtkTypes[ SMDSEntity_Triangle ]          = VTK_TRIANGLE;
    vtkTypes[ SMDSEntity_Quad_Triangle ]     = VTK_QUADRATIC_TRIANGLE;
    vtkTypes[ SMDSEntity_BiQuad_Triangle ]   = VTK_BIQUADRATIC_TRIANGLE;
    vtkTypes[ SMDSEntity_Quadrangle ]        = VTK_QUAD;
    vtkTypes[ SMDSEntity_Quad_Quadrangle ]   = VTK_QUADRATIC_QUAD;
    vtkTypes[ SMDSEntity_BiQuad_Quadrangle ] = VTK_BIQUADRATIC_QUAD;
    vtkTypes[ SMDSEntity_Polygon ]           = VTK_POLYGON;
    vtkTypes[ SMDSEntity_Quad_Polygon ]      = VTK_QUADRATIC_POLYGON;
    vtkTypes[ SMDSEntity_Tetra ]             = VTK_TETRA;
    vtkTypes[ SMDSEntity_Quad_Tetra ]        = VTK_QUADRATIC_TETRA;
    vtkTypes[ SMDSEntity_Pyramid ]           = VTK_PYRAMID;
    vtkTypes[ SMDSEntity_Quad_Pyramid ]      = VTK_QUADRATIC_PYRAMID;
    vtkTypes[ SMDSEntity_Hexa ]              = VTK_HEXAHEDRON;
    vtkTypes[ SMDSEntity_Quad_Hexa ]         = VTK_QUADRATIC_HEXAHEDRON;
    vtkTypes[ SMDSEntity_TriQuad_Hexa ]      = VTK_TRIQUADRATIC_HEXAHEDRON;
    vtkTypes[ SMDSEntity_Penta ]             = VTK_WEDGE;
    vtkTypes[ SMDSEntity_Quad_Penta ]        = VTK_QUADRATIC_WEDGE;
    vtkTypes[ SMDSEntity_BiQuad_Penta ]      = VTK_BIQUADRATIC_QUADRATIC_WEDGE;
    vtkTypes[ SMDSEntity_Hexagonal_Prism ]   = VTK_HEXAGONAL_PRISM;
    vtkTypes[ SMDSEntity_Polyhedra ]         = VTK_POLYHEDRON;
    //vtkTypes[ SMDSEntity_Quad_Polyhedra ]    = ;
    vtkTypes[ SMDSEntity_Ball ]              = VTK_POLY_VERTEX;
  }
  return vtkTypes[ smdsType ];
}

//================================================================================
/*!
 * \brief Return indices to transform cell connectivity from SMDS to VTK
 * Usage: vtkIDs[i] = smdsIDs[ indices[ i ]]
 */
//================================================================================

const std::vector< int >& SMDS_MeshCell::toVtkOrder(SMDSAbs_EntityType smdsType)
{
  static std::vector< std::vector< int > > toVtkInterlaces;
  if ( toVtkInterlaces.empty() )
  {
    toVtkInterlaces.resize( SMDSEntity_Last+1 );
    // {
    //   const int ids[] = {0};
    //   toVtkInterlaces[SMDSEntity_0D].assign( &ids[0], &ids[0]+1 );
    //   toVtkInterlaces[SMDSEntity_Node].assign( &ids[0], &ids[0]+1 );
    // }
    // {
    //   const int ids[] = {0,1};
    //   toVtkInterlaces[SMDSEntity_Edge].assign( &ids[0], &ids[0]+2 );
    // }
    // {
    //   const int ids[] = {0,1,2};
    //   toVtkInterlaces[SMDSEntity_Quad_Edge].assign( &ids[0], &ids[0]+3 );
    // }
    // {
    //   const int ids[] = {0,1,2};
    //   toVtkInterlaces[SMDSEntity_Triangle].assign( &ids[0], &ids[0]+3 );
    // }
    // {
    //   const int ids[] = {0,1,2,3,4,5};
    //   toVtkInterlaces[SMDSEntity_Quad_Triangle].assign( &ids[0], &ids[0]+6 );
    // }
    // {
    //   const int ids[] = {0,1,2,3};
    //   toVtkInterlaces[SMDSEntity_Quadrangle].assign( &ids[0], &ids[0]+4 );
    // }
    // {
    //   const int ids[] = {0,1,2,3,4,5,6,7};
    //   toVtkInterlaces[SMDSEntity_Quad_Quadrangle].assign( &ids[0], &ids[0]+8 );
    // }
    // {
    //   const int ids[] = {0,1,2,3,4,5,6,7,8};
    //   toVtkInterlaces[SMDSEntity_BiQuad_Quadrangle].assign( &ids[0], &ids[0]+9 );
    // }
    {
      const int ids[] = {0,2,1,3};
      toVtkInterlaces[SMDSEntity_Tetra].assign( &ids[0], &ids[0]+4 );
    }
    {
      const int ids[] = {0,2,1,3,6,5,4,7,9,8};
      toVtkInterlaces[SMDSEntity_Quad_Tetra].assign( &ids[0], &ids[0]+10 );
    }
    {
      const int ids[] = {0,3,2,1,4};
      toVtkInterlaces[SMDSEntity_Pyramid].assign( &ids[0], &ids[0]+5 );
    }
    {
      const int ids[] = {0,3,2,1,4,8,7,6,5,9,12,11,10};
      toVtkInterlaces[SMDSEntity_Quad_Pyramid].assign( &ids[0], &ids[0]+13 );
    }
    {
      const int ids[] = {0,3,2,1,4,7,6,5};
      toVtkInterlaces[SMDSEntity_Hexa].assign( &ids[0], &ids[0]+8 );
    }
    {
      const int ids[] = {0,3,2,1,4,7,6,5,11,10,9,8,15,14,13,12,16,19,18,17};
      toVtkInterlaces[SMDSEntity_Quad_Hexa].assign( &ids[0], &ids[0]+20 );
    }
    {
      const int ids[] = {0,3,2,1,4,7,6,5,11,10,9,8,15,14,13,12,16,19,18,17, 21,23,24,22,20,25,26};
      toVtkInterlaces[SMDSEntity_TriQuad_Hexa].assign( &ids[0], &ids[0]+27 );
    }
    {
      const int ids[] = {0,1,2,3,4,5};
      toVtkInterlaces[SMDSEntity_Penta].assign( &ids[0], &ids[0]+6 );
    }
    {
      const int ids[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14}; // TODO: check
      toVtkInterlaces[SMDSEntity_Quad_Penta].assign( &ids[0], &ids[0]+15 );
    }
    {
      const int ids[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};// TODO: check
      toVtkInterlaces[SMDSEntity_BiQuad_Penta].assign( &ids[0], &ids[0]+18 );
    }
    {
      const int ids[] = {0,5,4,3,2,1,6,11,10,9,8,7};
      toVtkInterlaces[SMDSEntity_Hexagonal_Prism].assign( &ids[0], &ids[0]+12 );
    }
  }
  return toVtkInterlaces[smdsType];
}

//================================================================================
/*!
 * \brief Return indices to reverse an SMDS cell of given type.
 *        nbNodes is useful for polygons
 * Usage: reverseIDs[i] = forwardIDs[ indices[ i ]]
 */
//================================================================================

const std::vector<int>& SMDS_MeshCell::reverseSmdsOrder(SMDSAbs_EntityType smdsType,
                                                        const size_t       nbNodes)
{
  static std::vector< std::vector< int > > reverseInterlaces;
  if ( reverseInterlaces.empty() )
  {
    reverseInterlaces.resize( SMDSEntity_Last+1 );
    {
      const int ids[] = {0};
      reverseInterlaces[SMDSEntity_0D].assign( &ids[0], &ids[0]+1 );
      reverseInterlaces[SMDSEntity_Node].assign( &ids[0], &ids[0]+1 );
      reverseInterlaces[SMDSEntity_Ball].assign( &ids[0], &ids[0]+1 );
    }
    {
      const int ids[] = {1,0};
      reverseInterlaces[SMDSEntity_Edge].assign( &ids[0], &ids[0]+2 );
    }
    {
      const int ids[] = {1,0,2};
      reverseInterlaces[SMDSEntity_Quad_Edge].assign( &ids[0], &ids[0]+3 );
    }
    {
      const int ids[] = {0,2,1};
      reverseInterlaces[SMDSEntity_Triangle].assign( &ids[0], &ids[0]+3 );
    }
    {
      const int ids[] = {0,2,1,5,4,3};
      reverseInterlaces[SMDSEntity_Quad_Triangle].assign( &ids[0], &ids[0]+6 );
    }
    {
      const int ids[] = {0,2,1,5,4,3,6};
      reverseInterlaces[SMDSEntity_BiQuad_Triangle].assign( &ids[0], &ids[0]+7 );
    }
    {
      const int ids[] = {0,3,2,1};
      reverseInterlaces[SMDSEntity_Quadrangle].assign( &ids[0], &ids[0]+4 );
    }
    {
      const int ids[] = {0,3,2,1,7,6,5,4};
      reverseInterlaces[SMDSEntity_Quad_Quadrangle].assign( &ids[0], &ids[0]+8 );
    }
    {
      const int ids[] = {0,3,2,1,7,6,5,4,8};
      reverseInterlaces[SMDSEntity_BiQuad_Quadrangle].assign( &ids[0], &ids[0]+9 );
    }
    {
      const int ids[] = {0,2,1,3};
      reverseInterlaces[SMDSEntity_Tetra].assign( &ids[0], &ids[0]+4 );
    }
    {
      const int ids[] = {0,2,1,3,6,5,4,7,9,8};
      reverseInterlaces[SMDSEntity_Quad_Tetra].assign( &ids[0], &ids[0]+10 );
    }
    {
      const int ids[] = {0,3,2,1,4};
      reverseInterlaces[SMDSEntity_Pyramid].assign( &ids[0], &ids[0]+5 );
    }
    {
      const int ids[] = {0,3,2,1,4,8,7,6,5,9,12,11,10};
      reverseInterlaces[SMDSEntity_Quad_Pyramid].assign( &ids[0], &ids[0]+13 );
    }
    {
      const int ids[] = {0,3,2,1,4,7,6,5};
      reverseInterlaces[SMDSEntity_Hexa].assign( &ids[0], &ids[0]+8 );
    }
    {
      const int ids[] = {0,3,2,1,4,7,6,5,11,10,9,8,15,14,13,12,16,19,18,17};
      reverseInterlaces[SMDSEntity_Quad_Hexa].assign( &ids[0], &ids[0]+20 );
    }
    {
      const int ids[] = {0,3,2,1,4,7,6,5,11,10,9,8,15,14,13,12,16,19,18,17, 20,24,23,22,21,25,26};
      reverseInterlaces[SMDSEntity_TriQuad_Hexa].assign( &ids[0], &ids[0]+27 );
    }
    {
      const int ids[] = {0,2,1,3,5,4};
      reverseInterlaces[SMDSEntity_Penta].assign( &ids[0], &ids[0]+6 );
    }
    {
      const int ids[] = {0,2,1,3,5,4, 8,7,6,11,10,9,12,14,13};
      reverseInterlaces[SMDSEntity_Quad_Penta].assign( &ids[0], &ids[0]+15 );
    }
    {
      const int ids[] = {0,2,1,3,5,4, 8,7,6,11,10,9,12,14,13,15,16,17};
      reverseInterlaces[SMDSEntity_BiQuad_Penta].assign( &ids[0], &ids[0]+18 );
    }
    {
      const int ids[] = {0,5,4,3,2,1,6,11,10,9,8,7};
      reverseInterlaces[SMDSEntity_Hexagonal_Prism].assign( &ids[0], &ids[0]+12 );
    }
  }

  if ( smdsType == SMDSEntity_Polygon )
  {
    if ( reverseInterlaces[ smdsType ].size() != nbNodes )
    {
      reverseInterlaces[ smdsType ].resize( nbNodes );
      for ( size_t i = 0; i < nbNodes; ++i )
        reverseInterlaces[ smdsType ][i] = nbNodes - i - 1;
    }
  }
  else if ( smdsType == SMDSEntity_Quad_Polygon )
  {
    if ( reverseInterlaces[ smdsType ].size() != nbNodes )
    {
      // e.g. for 8 nodes: [ 0, 3,2,1, 7,6,5,4 ]
      reverseInterlaces[ smdsType ].resize( nbNodes );
      size_t pos = 0;
      reverseInterlaces[ smdsType ][pos++] = 0;
      for ( int i = nbNodes / 2 - 1; i > 0 ; --i ) // 3,2,1
        reverseInterlaces[ smdsType ][pos++] = i;
      for ( int i = nbNodes - 1, nb = nbNodes / 2; i >= nb; --i ) // 7,6,5,4
        reverseInterlaces[ smdsType ][pos++] = i;
    }
  }
  
  return reverseInterlaces[smdsType];
}

//================================================================================
/*!
 * \brief Return indices to set nodes of a quadratic 1D or 2D element in interlaced order
 * Usage: interlacedIDs[i] = smdsIDs[ indices[ i ]]
 */
//================================================================================

const std::vector<int>& SMDS_MeshCell::interlacedSmdsOrder(SMDSAbs_EntityType smdsType,
                                                           const size_t       nbNodes)
{
  static std::vector< std::vector< int > > interlace;
  if ( interlace.empty() )
  {
    interlace.resize( SMDSEntity_Last+1 );
    {
      const int ids[] = {0,2,1};
      interlace[SMDSEntity_Quad_Edge].assign( &ids[0], &ids[0]+3 );
    }
    {
      const int ids[] = {0,3,1,4,2,5,6};
      interlace[SMDSEntity_Quad_Triangle  ].assign( &ids[0], &ids[0]+6 );
      interlace[SMDSEntity_BiQuad_Triangle].assign( &ids[0], &ids[0]+7 );
    }
    {
      const int ids[] = {0,4,1,5,2,6,3,7,8};
      interlace[SMDSEntity_Quad_Quadrangle  ].assign( &ids[0], &ids[0]+8 );
      interlace[SMDSEntity_BiQuad_Quadrangle].assign( &ids[0], &ids[0]+9 );
    }
  }

  if ( smdsType == SMDSEntity_Quad_Polygon )
  {
    if ( interlace[smdsType].size() != nbNodes )
    {
      interlace[smdsType].resize( nbNodes );
      for ( size_t i = 0; i < nbNodes / 2; ++i )
      {
        interlace[smdsType][i*2+0] = i;
        interlace[smdsType][i*2+1] = i + nbNodes / 2;
      }
    }
  }
  return interlace[smdsType];
}

//================================================================================
/*!
 * \brief Return SMDSAbs_EntityType corresponding to VTKCellType
 */
//================================================================================

SMDSAbs_EntityType SMDS_MeshCell::toSmdsType(VTKCellType vtkType)
{
  return getCellProps( vtkType ).myEntity;
}

//================================================================================
/*!
 * \brief Return SMDSAbs_ElementType by SMDSAbs_GeometryType
 */
//================================================================================

SMDSAbs_ElementType SMDS_MeshCell::ElemType(SMDSAbs_GeometryType geomType)
{
  switch ( geomType ) {
  case SMDSGeom_POINT:     return SMDSAbs_0DElement;

  case SMDSGeom_EDGE:      return SMDSAbs_Edge; 

  case SMDSGeom_TRIANGLE:
  case SMDSGeom_QUADRANGLE:
  case SMDSGeom_POLYGON:   return SMDSAbs_Face;

  case SMDSGeom_TETRA:
  case SMDSGeom_PYRAMID:
  case SMDSGeom_HEXA:
  case SMDSGeom_PENTA:
  case SMDSGeom_HEXAGONAL_PRISM:
  case SMDSGeom_POLYHEDRA: return SMDSAbs_Volume;

  case SMDSGeom_BALL:      return SMDSAbs_Ball;

  case SMDSGeom_NONE: ;
  }
  return SMDSAbs_All;
}

//================================================================================
/*!
 * \brief Return SMDSAbs_ElementType by SMDSAbs_EntityType
 */
//================================================================================

SMDSAbs_ElementType SMDS_MeshCell::ElemType(SMDSAbs_EntityType entityType)
{
  return getCellProps( entityType ).myType;
}

SMDSAbs_GeometryType SMDS_MeshCell::GeomType( SMDSAbs_EntityType entityType )
{
  return getCellProps( entityType ).myGeom;
}

bool SMDS_MeshCell::IsPoly( SMDSAbs_EntityType entityType )
{
  return getCellProps( entityType ).myIsPoly;
}

bool SMDS_MeshCell::IsQuadratic( SMDSAbs_EntityType entityType )
{
  return getCellProps( entityType ).IsQuadratic();
}

int SMDS_MeshCell::NbCornerNodes( SMDSAbs_EntityType entityType )
{
  return getCellProps( entityType ).myNbCornerNodes;
}

int SMDS_MeshCell::NbNodes( SMDSAbs_EntityType entityType )
{
  return getCellProps( entityType ).myNbNodes;
}

int SMDS_MeshCell::NbEdges( SMDSAbs_EntityType entityType )
{
  return getCellProps( entityType ).myNbEdges;
}

int SMDS_MeshCell::NbFaces( SMDSAbs_EntityType entityType )
{
  return getCellProps( entityType ).myNbFaces;
}

//================================================================================
/*!
 * \brief Return indices to transform cell connectivity from VTK to SMDS
 * Usage: smdsIDs[i] = vtkIDs[ indices[ i ]]
 */
//================================================================================

const std::vector<int>& SMDS_MeshCell::fromVtkOrder(SMDSAbs_EntityType smdsType)
{
  static std::vector< std::vector<int> > fromVtkInterlaces;
  if ( fromVtkInterlaces.empty() )
  {
    fromVtkInterlaces.resize( SMDSEntity_Last+1 );
    for ( int iSMDS = 0; iSMDS < SMDSEntity_Last; ++iSMDS )
    {
      const std::vector<int> & toVtk = toVtkOrder( SMDSAbs_EntityType( iSMDS ));
      std::vector<int> &      toSmds = fromVtkInterlaces[ iSMDS ];
      toSmds.resize( toVtk.size() );
      for ( size_t i = 0; i < toVtk.size(); ++i )
        toSmds[ toVtk[i] ] = i;
    }
  }
  return fromVtkInterlaces[ smdsType ];
}

//================================================================================
/*!
 * \brief Return indices to transform cell connectivity from SMDS to VTK
 * Usage: vtkIDs[i] = smdsIDs[ indices[ i ]]
 */
//================================================================================

const std::vector<int>& SMDS_MeshCell::toVtkOrder(VTKCellType vtkType)
{
  return toVtkOrder( toSmdsType( vtkType ));
}

//================================================================================
/*!
 * \brief Return indices to transform cell connectivity from VTK to SMDS
 * Usage: smdsIDs[i] = vtkIDs[ indices[ i ]]
 */
//================================================================================

const std::vector<int>& SMDS_MeshCell::fromVtkOrder(VTKCellType vtkType)
{
  return fromVtkOrder( toSmdsType( vtkType ));
}
