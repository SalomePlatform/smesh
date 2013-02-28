// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
// File      : DriverGMF_Write.cxx
// Created   : Mon Sep 17 17:03:02 2012
// Author    : Edward AGAPOV (eap)

#include "DriverGMF_Write.hxx"
#include "DriverGMF.hxx"

#include "SMESHDS_GroupBase.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESH_Comment.hxx"

#include <Basics_Utils.hxx>

extern "C"
{
#include "libmesh5.h"
}

#include <vector>

#define BEGIN_ELEM_WRITE( SMDSEntity, GmfKwd, elem )                    \
  elemIt = myMesh->elementEntityIterator( SMDSEntity );                 \
  if ( elemIt->more() )                                                 \
  {                                                                     \
  GmfSetKwd(meshID, GmfKwd, myMesh->GetMeshInfo().NbEntities( SMDSEntity )); \
  for ( int gmfID = 1; elemIt->more(); ++gmfID )                        \
  {                                                                     \
  const SMDS_MeshElement* elem = elemIt->next();                        \
  GmfSetLin(meshID, GmfKwd,

#define BEGIN_EXTRA_VERTICES_WRITE( SMDSEntity, GmfKwd, elem, nbVertices ) \
  elemIt = myMesh->elementEntityIterator( SMDSEntity );                 \
  if ( elemIt->more() )                                                 \
  {                                                                     \
  GmfSetKwd(meshID, GmfKwd, myMesh->GetMeshInfo().NbEntities( SMDSEntity )); \
  for ( int gmfID = 1; elemIt->more(); ++gmfID )                        \
  {                                                                     \
  const SMDS_MeshElement* elem = elemIt->next();                        \
  GmfSetLin(meshID, GmfKwd, gmfID, nbVertices,

#define END_ELEM_WRITE( elem )                  \
  elem->getshapeId() );                         \
  }}                                            \

#define END_ELEM_WRITE_ADD_TO_MAP( elem, e2id )         \
  elem->getshapeId() );                                 \
  e2id.insert( e2id.end(), make_pair( elem, gmfID ));   \
  }}                                                    \

#define END_EXTRA_VERTICES_WRITE()           \
  );                                         \
  }}                                         \


DriverGMF_Write::DriverGMF_Write():
  Driver_SMESHDS_Mesh(), _exportRequiredGroups( true )
{
}
DriverGMF_Write::~DriverGMF_Write()
{
}

//================================================================================
/*!
 * \brief Reads a GMF file
 */
//================================================================================

Driver_Mesh::Status DriverGMF_Write::Perform()
{
  Kernel_Utils::Localizer loc;

  const int dim = 3, version = sizeof(long) == 4 ? 2 : 3;

  int meshID = GmfOpenMesh( myFile.c_str(), GmfWrite, version, dim );
  if ( !meshID )
  {
    if ( DriverGMF::isExtensionCorrect( myFile ))
      return addMessage( SMESH_Comment("Can't open for writing ") << myFile, /*fatal=*/true );
    else
      return addMessage( SMESH_Comment("Not '.mesh' or '.meshb' extension of file ") << myFile, /*fatal=*/true );
  }

  DriverGMF::MeshCloser aMeshCloser( meshID ); // An object closing GMF mesh at destruction

  // nodes
  std::map< const SMDS_MeshNode* , int > node2IdMap;
  int iN = 0, nbNodes = myMesh->NbNodes();
  GmfSetKwd( meshID, GmfVertices, nbNodes );
  double xyz[3];
  SMDS_NodeIteratorPtr nodeIt = myMesh->nodesIterator();
  while ( nodeIt->more() )
  {
    const SMDS_MeshNode* n = nodeIt->next();
    n->GetXYZ( xyz );
    GmfSetLin( meshID, GmfVertices, xyz[0], xyz[1], xyz[2], n->getshapeId() );
    node2IdMap.insert( node2IdMap.end(), make_pair( n, ++iN ));
  }
  if ( iN != nbNodes )
    return addMessage("Wrong nb of nodes returned by nodesIterator", /*fatal=*/true);


  SMDS_ElemIteratorPtr elemIt;
  typedef std::map< const SMDS_MeshElement*, size_t, TIDCompare > TElem2IDMap;

  // edges
  TElem2IDMap edge2IDMap;
  BEGIN_ELEM_WRITE( SMDSEntity_Edge, GmfEdges, edge )
    node2IdMap[ edge->GetNode( 0 )],
    node2IdMap[ edge->GetNode( 1 )],
    END_ELEM_WRITE_ADD_TO_MAP( edge, edge2IDMap );

  // quadratic edges
  BEGIN_ELEM_WRITE( SMDSEntity_Quad_Edge, GmfEdges, edge )
    node2IdMap[ edge->GetNode( 0 )],
    node2IdMap[ edge->GetNode( 1 )],
    END_ELEM_WRITE( edge );

  BEGIN_EXTRA_VERTICES_WRITE( SMDSEntity_Quad_Edge, GmfExtraVerticesAtEdges, edge, 1 )
    node2IdMap[ edge->GetNode( 2 )]
    END_EXTRA_VERTICES_WRITE();

  // triangles
  TElem2IDMap tria2IDMap;
  BEGIN_ELEM_WRITE( SMDSEntity_Triangle, GmfTriangles, tria )
    node2IdMap[ tria->GetNode( 0 )],
    node2IdMap[ tria->GetNode( 1 )],
    node2IdMap[ tria->GetNode( 2 )],
    END_ELEM_WRITE_ADD_TO_MAP( tria, tria2IDMap );

  // quadratic triangles
  BEGIN_ELEM_WRITE( SMDSEntity_Quad_Triangle, GmfTriangles, tria )
    node2IdMap[ tria->GetNode( 0 )],
    node2IdMap[ tria->GetNode( 1 )],
    node2IdMap[ tria->GetNode( 2 )],
    END_ELEM_WRITE( tria );

  BEGIN_EXTRA_VERTICES_WRITE( SMDSEntity_Quad_Triangle, GmfExtraVerticesAtTriangles, tria, 3 )
    node2IdMap[ tria->GetNode( 3 )],
    node2IdMap[ tria->GetNode( 4 )],
    node2IdMap[ tria->GetNode( 5 )]
    END_EXTRA_VERTICES_WRITE();

  // quadrangles
  TElem2IDMap quad2IDMap;
  BEGIN_ELEM_WRITE( SMDSEntity_Quadrangle, GmfQuadrilaterals, quad )
    node2IdMap[ quad->GetNode( 0 )],
    node2IdMap[ quad->GetNode( 1 )],
    node2IdMap[ quad->GetNode( 2 )],
    node2IdMap[ quad->GetNode( 3 )],
    END_ELEM_WRITE_ADD_TO_MAP( quad, quad2IDMap );

  // quadratic quadrangles
  BEGIN_ELEM_WRITE( SMDSEntity_Quad_Quadrangle, GmfQuadrilaterals, quad )
    node2IdMap[ quad->GetNode( 0 )],
    node2IdMap[ quad->GetNode( 1 )],
    node2IdMap[ quad->GetNode( 2 )],
    node2IdMap[ quad->GetNode( 3 )],
    END_ELEM_WRITE( quad );

  BEGIN_EXTRA_VERTICES_WRITE( SMDSEntity_Quad_Quadrangle, GmfExtraVerticesAtQuadrilaterals, quad, 4 )
    node2IdMap[ quad->GetNode( 4 )],
    node2IdMap[ quad->GetNode( 5 )],
    node2IdMap[ quad->GetNode( 6 )],
    node2IdMap[ quad->GetNode( 7 )]
    END_EXTRA_VERTICES_WRITE();

  // bi-quadratic quadrangles
  BEGIN_ELEM_WRITE( SMDSEntity_BiQuad_Quadrangle, GmfQuadrilaterals, quad )
    node2IdMap[ quad->GetNode( 0 )],
    node2IdMap[ quad->GetNode( 1 )],
    node2IdMap[ quad->GetNode( 2 )],
    node2IdMap[ quad->GetNode( 3 )],
    END_ELEM_WRITE( quad );

  BEGIN_EXTRA_VERTICES_WRITE( SMDSEntity_BiQuad_Quadrangle, GmfExtraVerticesAtQuadrilaterals, quad, 5 )
    node2IdMap[ quad->GetNode( 4 )],
    node2IdMap[ quad->GetNode( 5 )],
    node2IdMap[ quad->GetNode( 6 )],
    node2IdMap[ quad->GetNode( 7 )],
    node2IdMap[ quad->GetNode( 8 )]
    END_EXTRA_VERTICES_WRITE();

  // terahedra
  BEGIN_ELEM_WRITE( SMDSEntity_Tetra, GmfTetrahedra, tetra )
    node2IdMap[ tetra->GetNode( 0 )],
    node2IdMap[ tetra->GetNode( 2 )],
    node2IdMap[ tetra->GetNode( 1 )],
    node2IdMap[ tetra->GetNode( 3 )],
    END_ELEM_WRITE( tetra );

  // quadratic terahedra
  BEGIN_ELEM_WRITE( SMDSEntity_Quad_Tetra, GmfTetrahedra, tetra )
    node2IdMap[ tetra->GetNode( 0 )],
    node2IdMap[ tetra->GetNode( 2 )],
    node2IdMap[ tetra->GetNode( 1 )],
    node2IdMap[ tetra->GetNode( 3 )],
    END_ELEM_WRITE( tetra );
    
  BEGIN_EXTRA_VERTICES_WRITE( SMDSEntity_Quad_Tetra, GmfExtraVerticesAtTetrahedra, tetra, 6 )
    node2IdMap[ tetra->GetNode( 6 )],
    node2IdMap[ tetra->GetNode( 5 )],
    node2IdMap[ tetra->GetNode( 4 )],
    node2IdMap[ tetra->GetNode( 7 )],
    node2IdMap[ tetra->GetNode( 9 )],
    node2IdMap[ tetra->GetNode( 8 )]
    END_EXTRA_VERTICES_WRITE();
    
  // pyramids
  BEGIN_ELEM_WRITE( SMDSEntity_Pyramid, GmfPyramids, pyra )
    node2IdMap[ pyra->GetNode( 0 )],
    node2IdMap[ pyra->GetNode( 2 )],
    node2IdMap[ pyra->GetNode( 1 )],
    node2IdMap[ pyra->GetNode( 3 )],
    node2IdMap[ pyra->GetNode( 4 )],
    END_ELEM_WRITE( pyra );

  // hexahedra
  BEGIN_ELEM_WRITE( SMDSEntity_Hexa, GmfHexahedra, hexa )
    node2IdMap[ hexa->GetNode( 0 )],
    node2IdMap[ hexa->GetNode( 3 )],
    node2IdMap[ hexa->GetNode( 2 )],
    node2IdMap[ hexa->GetNode( 1 )],
    node2IdMap[ hexa->GetNode( 4 )],
    node2IdMap[ hexa->GetNode( 7 )],
    node2IdMap[ hexa->GetNode( 6 )],
    node2IdMap[ hexa->GetNode( 5 )],
    END_ELEM_WRITE( hexa );

  // quadratic hexahedra
  BEGIN_ELEM_WRITE( SMDSEntity_Quad_Hexa, GmfHexahedra, hexa )
    node2IdMap[ hexa->GetNode( 0 )],
    node2IdMap[ hexa->GetNode( 3 )],
    node2IdMap[ hexa->GetNode( 2 )],
    node2IdMap[ hexa->GetNode( 1 )],
    node2IdMap[ hexa->GetNode( 4 )],
    node2IdMap[ hexa->GetNode( 7 )],
    node2IdMap[ hexa->GetNode( 6 )],
    node2IdMap[ hexa->GetNode( 5 )],
    END_ELEM_WRITE( hexa );
    
  BEGIN_EXTRA_VERTICES_WRITE( SMDSEntity_Quad_Hexa, GmfExtraVerticesAtHexahedra, hexa, 12 )
    node2IdMap[ hexa->GetNode( 11 )],
    node2IdMap[ hexa->GetNode( 10 )],
    node2IdMap[ hexa->GetNode( 9 )],
    node2IdMap[ hexa->GetNode( 8 )],
    node2IdMap[ hexa->GetNode( 15 )],
    node2IdMap[ hexa->GetNode( 14 )],
    node2IdMap[ hexa->GetNode( 13 )],
    node2IdMap[ hexa->GetNode( 12 )],
    node2IdMap[ hexa->GetNode( 16 )],
    node2IdMap[ hexa->GetNode( 19 )],
    node2IdMap[ hexa->GetNode( 18 )],
    node2IdMap[ hexa->GetNode( 17 )]
    END_EXTRA_VERTICES_WRITE();
    
  // tri-quadratic hexahedra
  BEGIN_ELEM_WRITE( SMDSEntity_TriQuad_Hexa, GmfHexahedra, hexa )
    node2IdMap[ hexa->GetNode( 0 )],
    node2IdMap[ hexa->GetNode( 3 )],
    node2IdMap[ hexa->GetNode( 2 )],
    node2IdMap[ hexa->GetNode( 1 )],
    node2IdMap[ hexa->GetNode( 4 )],
    node2IdMap[ hexa->GetNode( 7 )],
    node2IdMap[ hexa->GetNode( 6 )],
    node2IdMap[ hexa->GetNode( 5 )],
    END_ELEM_WRITE( hexa );
    
  BEGIN_EXTRA_VERTICES_WRITE( SMDSEntity_TriQuad_Hexa, GmfExtraVerticesAtHexahedra, hexa, 19 )
    node2IdMap[ hexa->GetNode( 11 )],
    node2IdMap[ hexa->GetNode( 10 )],
    node2IdMap[ hexa->GetNode( 9 )],
    node2IdMap[ hexa->GetNode( 8 )],
    node2IdMap[ hexa->GetNode( 15 )],
    node2IdMap[ hexa->GetNode( 14 )],
    node2IdMap[ hexa->GetNode( 13 )],
    node2IdMap[ hexa->GetNode( 12 )],
    node2IdMap[ hexa->GetNode( 16 )],
    node2IdMap[ hexa->GetNode( 19 )],
    node2IdMap[ hexa->GetNode( 18 )],
    node2IdMap[ hexa->GetNode( 17 )],
    node2IdMap[ hexa->GetNode( 20 )],
    node2IdMap[ hexa->GetNode( 24 )],
    node2IdMap[ hexa->GetNode( 23 )],
    node2IdMap[ hexa->GetNode( 22 )],
    node2IdMap[ hexa->GetNode( 21 )],
    node2IdMap[ hexa->GetNode( 25 )],
    node2IdMap[ hexa->GetNode( 26 )]
    END_EXTRA_VERTICES_WRITE();

  // prism
  BEGIN_ELEM_WRITE( SMDSEntity_Penta, GmfPrisms, prism )
    node2IdMap[ prism->GetNode( 0 )],
    node2IdMap[ prism->GetNode( 2 )],
    node2IdMap[ prism->GetNode( 1 )],
    node2IdMap[ prism->GetNode( 3 )],
    node2IdMap[ prism->GetNode( 5 )],
    node2IdMap[ prism->GetNode( 4 )],
    END_ELEM_WRITE( prism );


  if ( _exportRequiredGroups )
  {
    // required entities
    SMESH_Comment badGroups;
    const std::set<SMESHDS_GroupBase*>&      groupSet = myMesh->GetGroups();
    std::set<SMESHDS_GroupBase*>::const_iterator grIt = groupSet.begin();
    for ( ; grIt != groupSet.end(); ++grIt )
    {
      const SMESHDS_GroupBase* group = *grIt;
      std::string          groupName = group->GetStoreName();
      std::string::size_type     pos = groupName.find( "_required_" );
      if ( pos == std::string::npos ) continue;

      int                    gmfKwd;
      SMDSAbs_EntityType smdsEntity;
      std::string entity = groupName.substr( pos + strlen("_required_"));
      if      ( entity == "Vertices" ) {
        gmfKwd   = GmfRequiredVertices;
        smdsEntity = SMDSEntity_Node;
      }
      else if ( entity == "Edges" ) {
        gmfKwd   = GmfRequiredEdges;
        smdsEntity = SMDSEntity_Edge;
      }
      else if ( entity == "Triangles" ) {
        gmfKwd   = GmfRequiredTriangles;
        smdsEntity = SMDSEntity_Triangle;
      }
      else if ( entity == "Quadrilaterals" ) {
        gmfKwd   = GmfRequiredQuadrilaterals;
        smdsEntity = SMDSEntity_Quadrangle;
      }
      else {
        addMessage( SMESH_Comment("Invalig gmf entity name: ") << entity, /*fatal=*/false );
        continue;
      }

      // check elem type in the group
      int nbOkElems = 0;
      SMDS_ElemIteratorPtr elemIt = group->GetElements();
      while ( elemIt->more() )
        nbOkElems += ( elemIt->next()->GetEntityType() == smdsEntity );

      if ( nbOkElems != group->Extent() && nbOkElems == 0 )
      {
        badGroups << " " << groupName;
        continue;
      }

      // choose a TElem2IDMap
      TElem2IDMap* elem2IDMap = 0;
      if ( smdsEntity == SMDSEntity_Quadrangle && nbOkElems != myMesh->NbFaces() )
        elem2IDMap = & quad2IDMap;
      else if ( smdsEntity == SMDSEntity_Triangle && nbOkElems != myMesh->NbFaces() )
        elem2IDMap = & tria2IDMap;
      else if ( smdsEntity == SMDSEntity_Edge && nbOkElems != myMesh->NbEdges() )
        elem2IDMap = & edge2IDMap;

      // write the group
      GmfSetKwd( meshID, gmfKwd, nbOkElems );
      elemIt = group->GetElements();
      if ( elem2IDMap )
        for ( ; elemIt->more(); )
        {
          const SMDS_MeshElement* elem = elemIt->next();
          if ( elem->GetEntityType() == smdsEntity )
            GmfSetLin( meshID, gmfKwd, (*elem2IDMap)[ elem ] );
        }
      else
        for ( int gmfID = 1; elemIt->more(); ++gmfID)
        {
          const SMDS_MeshElement* elem = elemIt->next();
          if ( elem->GetEntityType() == smdsEntity )
            GmfSetLin( meshID, gmfKwd, gmfID );
        }

    } // loop on groups

    if ( !badGroups.empty() )
      addMessage( SMESH_Comment("Groups of elements of inappropriate geometry:")
                  << badGroups, /*fatal=*/false );
  }

  return DRS_OK;
}
