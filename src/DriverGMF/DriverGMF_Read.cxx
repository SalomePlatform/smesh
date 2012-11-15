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
// License along with this library; if not, Read to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// File      : DriverGMF_Read.cxx
// Created   : Mon Sep 17 17:03:02 2012
// Author    : Edward AGAPOV (eap)

#include "DriverGMF_Read.hxx"
#include "DriverGMF.hxx"

#include "SMESHDS_Group.hxx"
#include "SMESHDS_Mesh.hxx"
#include "SMESH_Comment.hxx"

#include <Basics_Utils.hxx>

extern "C"
{
#include "libmesh5.h"
}

#include <stdarg.h>

// --------------------------------------------------------------------------------
DriverGMF_Read::DriverGMF_Read():
  Driver_SMESHDS_Mesh(),
  _makeRequiredGroups( true )
{
}
// --------------------------------------------------------------------------------
DriverGMF_Read::~DriverGMF_Read()
{
}

//================================================================================
/*!
 * \brief Read a GMF file
 */
//================================================================================

Driver_Mesh::Status DriverGMF_Read::Perform()
{
  Kernel_Utils::Localizer loc;

  Status status = DRS_OK;

  int dim, version;

  // open the file
  int meshID = GmfOpenMesh( myFile.c_str(), GmfRead, &version, &dim );
  if ( !meshID )
  {
    if ( DriverGMF::isExtensionCorrect( myFile ))
      return addMessage( SMESH_Comment("Can't open for reading ") << myFile, /*fatal=*/true );
    else
      return addMessage( SMESH_Comment("Not '.mesh' or '.meshb' extension of file ") << myFile, /*fatal=*/true );
  }
  DriverGMF::MeshCloser aMeshCloser( meshID ); // An object closing GMF mesh at destruction

  // Read nodes

  int nbNodes = GmfStatKwd(meshID, GmfVertices);
  if ( nbNodes < 1 )
    return addMessage( "No nodes in the mesh", /*fatal=*/true );

  GmfGotoKwd(meshID, GmfVertices);

  int ref;

  const int nodeIDShift = myMesh->GetMeshInfo().NbNodes();
  if ( version != GmfFloat )
  {
    double x, y, z;
    for ( int i = 1; i <= nbNodes; ++i )
    {
      GmfGetLin(meshID, GmfVertices, &x, &y, &z, &ref);
      myMesh->AddNodeWithID( x,y,z, nodeIDShift + i);
    }
  }
  else
  {
    float x, y, z;
    for ( int i = 1; i <= nbNodes; ++i )
    {
      GmfGetLin(meshID, GmfVertices, &x, &y, &z, &ref);
      myMesh->AddNodeWithID( x,y,z, nodeIDShift + i);
    }
  }

  // Read elements

  int iN[28];

  /* Read edges */
  const int edgeIDShift = myMesh->GetMeshInfo().NbElements();
  if ( int nbEdges = GmfStatKwd(meshID, GmfEdges))
  {
    GmfGotoKwd(meshID, GmfEdges);
    for ( int i = 1; i <= nbEdges; ++i )
    {
      GmfGetLin(meshID, GmfEdges, &iN[0], &iN[1], &ref);
      if ( !myMesh->AddEdgeWithID( iN[0], iN[1], edgeIDShift + i ))
        status = storeBadNodeIds( "GmfEdges",i, 2, iN[0], iN[1] );
    }
  }
  /* Read quadratic edges */
  const int edge2IDShift = myMesh->GetMeshInfo().NbElements();
  if ( int nbEdges = GmfStatKwd(meshID, GmfEdgesP2))
  {
    GmfGotoKwd(meshID, GmfEdgesP2);
    for ( int i = 1; i <= nbEdges; ++i )
    {
      GmfGetLin(meshID, GmfEdgesP2, &iN[0], &iN[1], &iN[2], &ref);
      if ( !myMesh->AddEdgeWithID( iN[0], iN[1], iN[2], edge2IDShift + i ))
        status = storeBadNodeIds( "GmfEdgesP2",i, 3, iN[0], iN[1], iN[2] );
    }
  }
  /* Read triangles */
  const int triaIDShift = myMesh->GetMeshInfo().NbElements();
  if ( int nbTria = GmfStatKwd(meshID, GmfTriangles))
  {
    GmfGotoKwd(meshID, GmfTriangles);
    for ( int i = 1; i <= nbTria; ++i )
    {
      GmfGetLin(meshID, GmfTriangles, &iN[0], &iN[1], &iN[2], &ref);
      if ( !myMesh->AddFaceWithID( iN[0], iN[1], iN[2], triaIDShift + i ))
        status = storeBadNodeIds( "GmfTriangles",i, 3, iN[0], iN[1], iN[2] );
    }
  }
  /* Read quadratic triangles */
  const int tria2IDShift = myMesh->GetMeshInfo().NbElements();
  if ( int nbTria = GmfStatKwd(meshID, GmfTrianglesP2))
  {
    GmfGotoKwd(meshID, GmfTrianglesP2);
    for ( int i = 1; i <= nbTria; ++i )
    {
      GmfGetLin(meshID, GmfTrianglesP2,
                &iN[0], &iN[1], &iN[2], &iN[3], &iN[4], &iN[5], &ref);
      if ( !myMesh->AddFaceWithID( iN[0],iN[1],iN[2],iN[3],iN[4],iN[5],
                                   tria2IDShift + i ))
        status = storeBadNodeIds( "GmfTrianglesP2",i, 6, iN[0],iN[1],iN[2],iN[3],iN[4],iN[5] );
    }
  }
  /* Read quadrangles */
  const int quadIDShift = myMesh->GetMeshInfo().NbElements();
  if ( int nbQuad = GmfStatKwd(meshID, GmfQuadrilaterals))
  {
    GmfGotoKwd(meshID, GmfQuadrilaterals);
    for ( int i = 1; i <= nbQuad; ++i )
    {
      GmfGetLin(meshID, GmfQuadrilaterals, &iN[0], &iN[1], &iN[2], &iN[3], &ref);
      if ( !myMesh->AddFaceWithID( iN[0], iN[1], iN[2], iN[3], quadIDShift + i ))
        status = storeBadNodeIds( "GmfQuadrilaterals",i, 4, iN[0], iN[1],iN[2], iN[3] );
    }
  }
  /* Read bi-quadratic quadrangles */
  const int quad2IDShift = myMesh->GetMeshInfo().NbElements();
  if ( int nbQuad = GmfStatKwd(meshID, GmfQuadrilateralsQ2))
  {
    GmfGotoKwd(meshID, GmfQuadrilateralsQ2);
    for ( int i = 1; i <= nbQuad; ++i )
    {
      GmfGetLin(meshID, GmfQuadrilateralsQ2,
                &iN[0], &iN[1], &iN[2], &iN[3], &iN[4], &iN[5], &iN[6], &iN[7], &iN[8], &ref);
      if ( !myMesh->AddFaceWithID( iN[0],iN[1],iN[2],iN[3],iN[4],iN[5],iN[6],iN[7],iN[8],
                                   quad2IDShift + i ))
        status = storeBadNodeIds( "GmfQuadrilateralsQ2",i,
                                  9, iN[0],iN[1],iN[2],iN[3],iN[4],iN[5],iN[6],iN[7],iN[8] );
    }
  }
  /* Read terahedra */
  const int tetIDShift = myMesh->GetMeshInfo().NbElements();
  if ( int nbTet = GmfStatKwd(meshID, GmfTetrahedra))
  {
    GmfGotoKwd(meshID, GmfTetrahedra);
    for ( int i = 1; i <= nbTet; ++i )
    {
      GmfGetLin(meshID, GmfTetrahedra, &iN[0], &iN[1], &iN[2], &iN[3], &ref);
      if ( !myMesh->AddVolumeWithID( iN[0], iN[2], iN[1], iN[3], tetIDShift + i ))
        status = storeBadNodeIds( "GmfTetrahedra",i, 4, iN[0], iN[1],iN[2], iN[3] );
    }
  }
  /* Read quadratic terahedra */
  const int tet2IDShift = myMesh->GetMeshInfo().NbElements();
  if ( int nbTet = GmfStatKwd(meshID, GmfTetrahedraP2))
  {
    GmfGotoKwd(meshID, GmfTetrahedraP2);
    for ( int i = 1; i <= nbTet; ++i )
    {
      GmfGetLin(meshID, GmfTetrahedraP2, &iN[0], &iN[1], &iN[2],
                &iN[3], &iN[4], &iN[5], &iN[6], &iN[7], &iN[8], &iN[9], &ref);
      if ( !myMesh->AddVolumeWithID( iN[0],iN[2],iN[1],iN[3],
                                     iN[6],iN[5],iN[4],
                                     iN[7],iN[9],iN[8], tet2IDShift + i ))
        status = storeBadNodeIds( "GmfTetrahedraP2",i, 10, iN[0],iN[1],iN[2],iN[3],
                                  iN[4],iN[5],iN[6],iN[7],iN[8],iN[9] );
    }
  }
  /* Read pyramids */
  const int pyrIDShift = myMesh->GetMeshInfo().NbElements();
  if ( int nbPyr = GmfStatKwd(meshID, GmfPyramids))
  {
    GmfGotoKwd(meshID, GmfPyramids);
    for ( int i = 1; i <= nbPyr; ++i )
    {
      GmfGetLin(meshID, GmfPyramids, &iN[0], &iN[1], &iN[2], &iN[3], &iN[4], &ref);
      if ( !myMesh->AddVolumeWithID( iN[0], iN[2], iN[1], iN[3], iN[4], pyrIDShift + i ))
        status = storeBadNodeIds( "GmfPyramids",i, 5, iN[0], iN[1],iN[2], iN[3], iN[4] );
    }
  }
  /* Read hexahedra */
  const int hexIDShift = myMesh->GetMeshInfo().NbElements();
  if ( int nbHex = GmfStatKwd(meshID, GmfHexahedra))
  {
    GmfGotoKwd(meshID, GmfHexahedra);
    for ( int i = 1; i <= nbHex; ++i )
    {
      GmfGetLin(meshID, GmfHexahedra,
                &iN[0], &iN[1], &iN[2], &iN[3], &iN[4], &iN[5], &iN[6], &iN[7], &ref);
      if ( !myMesh->AddVolumeWithID( iN[0], iN[3], iN[2], iN[1], iN[4], iN[7], iN[6], iN[5],
                                     hexIDShift + i))
        status = storeBadNodeIds( "GmfHexahedra",i,
                                  8, iN[0], iN[1],iN[2], iN[3], iN[4], iN[7], iN[6], iN[5] );
    }
  }
  /* Read tri-quadratic hexahedra */
  const int hex2IDShift = myMesh->GetMeshInfo().NbElements();
  if ( int nbHex = GmfStatKwd(meshID, GmfHexahedraQ2))
  {
    GmfGotoKwd(meshID, GmfHexahedraQ2);
    for ( int i = 1; i <= nbHex; ++i )
    {
      GmfGetLin(meshID, GmfHexahedraQ2, &iN[0], &iN[1], &iN[2], &iN[3], &iN[4], &iN[5],
                &iN[6], &iN[7], &iN[8],&iN[9],&iN[10],&iN[11],&iN[12],&iN[13],&iN[14],
                &iN[15],&iN[16],&iN[17],&iN[18],&iN[19],&iN[20],&iN[21],&iN[22],&iN[23],
                &iN[24],&iN[25],&iN[26], &ref);
      if ( !myMesh->AddVolumeWithID( iN[0],iN[3],iN[2],iN[1],iN[4],iN[7],iN[6],iN[5],iN[11],iN[10],
                                     iN[9],iN[8],iN[12],iN[15],iN[14], iN[13],iN[19],iN[18],iN[17],
                                     iN[16],iN[20],iN[24],iN[23],iN[22],iN[21], iN[25],iN[26],
                                     hex2IDShift + i ))
        status = storeBadNodeIds( "GmfHexahedraQ2",i, 27,
                                  iN[0],iN[3],iN[2],iN[1],iN[4], iN[7],iN[6],iN[5],iN[11],iN[10],
                                  iN[9],iN[8],iN[12],iN[15],iN[14], iN[13],iN[19],iN[18],iN[17],
                                  iN[16],iN[20],iN[24],iN[23],iN[22],iN[21], iN[25],iN[26]);
    }
  }
  /* Read prism */
  const int prismIDShift = myMesh->GetMeshInfo().NbElements();
  if ( int nbPrism = GmfStatKwd(meshID, GmfPrisms))
  {
    GmfGotoKwd(meshID, GmfPrisms);
    for ( int i = 1; i <= nbPrism; ++i )
    {
      GmfGetLin(meshID, GmfPrisms,
                &iN[0], &iN[1], &iN[2], &iN[3], &iN[4], &iN[5], &ref);
      if ( !myMesh->AddVolumeWithID( iN[0], iN[2], iN[1], iN[3], iN[5], iN[4], prismIDShift + i))
        status = storeBadNodeIds( "GmfPrisms",i,
                                  6, iN[0], iN[1],iN[2], iN[3], iN[4], iN[5] );
    }
  }

  // Read required entities into groups

  if ( _makeRequiredGroups )
  {
    // get ids of existing groups
    std::set< int > groupIDs;
    const std::set<SMESHDS_GroupBase*>& groups = myMesh->GetGroups();
    std::set<SMESHDS_GroupBase*>::const_iterator grIter = groups.begin();
    for ( ; grIter != groups.end(); ++grIter )
      groupIDs.insert( (*grIter)->GetID() );
    if ( groupIDs.empty() ) groupIDs.insert( 0 );

    const int kes[4][3] = { { GmfRequiredVertices,      SMDSAbs_Node, nodeIDShift },
                            { GmfRequiredEdges,         SMDSAbs_Edge, edgeIDShift },
                            { GmfRequiredTriangles,     SMDSAbs_Face, triaIDShift },
                            { GmfRequiredQuadrilaterals,SMDSAbs_Face, quadIDShift }};
    const char* names[4] = { "_required_Vertices"      ,
                             "_required_Edges"         ,
                             "_required_Triangles"     ,
                             "_required_Quadrilaterals" };
    for ( int i = 0; i < 4; ++i )
    {
      int                 gmfKwd = kes[i][0];
      SMDSAbs_ElementType entity = (SMDSAbs_ElementType) kes[i][1];
      int                 shift  = kes[i][2];
      if ( int nb = GmfStatKwd(meshID, gmfKwd))
      {
        const int newID = *groupIDs.rbegin() + 1;
        groupIDs.insert( newID );
        SMESHDS_Group* group = new SMESHDS_Group( newID, myMesh, entity );
        group->SetStoreName( names[i] );
        myMesh->AddGroup( group );

        GmfGotoKwd(meshID, gmfKwd);
        for ( int i = 0; i < nb; ++i )
        {
          GmfGetLin(meshID, gmfKwd, &iN[0] );
          group->Add( shift + iN[0] );
        }
      }
    }
  }

  return status;
}

//================================================================================
/*!
 * \brief Store a message about invalid IDs of nodes
 */
//================================================================================

Driver_Mesh::Status DriverGMF_Read::storeBadNodeIds(const char* gmfKwd, int elemNb, int nb, ...)
{
  if ( myStatus != DRS_OK )
    return myStatus;

  SMESH_Comment msg;

  va_list VarArg;
  va_start(VarArg, nb);

  for ( int i = 0; i < nb; ++i )
  {
    int id = va_arg(VarArg, int );
    if ( !myMesh->FindNode( id ))
      msg << " " << id;
  }
  va_end(VarArg);

  if ( !msg.empty() )
  {
    std::string nbStr;
    const char* nbNames[] = { "1-st ", "2-nd ", "3-d " };
    if ( elemNb < 3 ) nbStr = nbNames[ elemNb-1 ];
    else              nbStr = SMESH_Comment(elemNb) << "-th ";

    return addMessage
      ( SMESH_Comment("Wrong node IDs of ")<< nbStr << gmfKwd << ":" << msg,
        /*fatal=*/false );
  }
  return DRS_OK;
}
