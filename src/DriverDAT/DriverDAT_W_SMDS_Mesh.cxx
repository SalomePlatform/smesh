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

#include <stdio.h>

#include "DriverDAT_W_SMDS_Mesh.h"

#include "SMDS_Mesh.hxx"

#include "utilities.h"

#include <Basics_Utils.hxx>

using namespace std;

Driver_Mesh::Status DriverDAT_W_SMDS_Mesh::Perform()
{
  Kernel_Utils::Localizer loc;
  Status aResult = DRS_OK;

  smIdType nbNodes, nbCells;
#if defined(WIN32) && defined(UNICODE)
  std::wstring file2Read = Kernel_Utils::utf8_decode_s(myFile);
  FILE* aFileId = _wfopen(file2Read.c_str(), L"w+");

#else
  char *file2Read = (char *)myFile.c_str();
  FILE* aFileId = fopen(file2Read, "w+");
#endif
  if ( !aFileId )
  {
    fprintf(stderr, ">> ERREUR : ouverture du fichier %s \n", file2Read);
    return DRS_FAIL;
  }
  SCRUTE(myMesh);
  /****************************************************************************
   *                       NOMBRES D'OBJETS                                    *
   ****************************************************************************/

  /* Combien de noeuds ? */
  nbNodes = myMesh->NbNodes();

  /* Combien de mailles, faces ou aretes ? */
  smIdType nb_of_edges, nb_of_faces, nb_of_volumes;
  nb_of_edges = myMesh->NbEdges();
  nb_of_faces = myMesh->NbFaces();
  nb_of_volumes = myMesh->NbVolumes();
  nbCells = nb_of_edges + nb_of_faces + nb_of_volumes;
  SCRUTE(nb_of_edges);
  SCRUTE(nb_of_faces);
  SCRUTE(nb_of_volumes);

  //fprintf(stdout, "%d %d\n", nbNodes, nbCells);
  fprintf(aFileId, "%ld %ld\n", nbNodes, nbCells);

  /****************************************************************************
   *                       ECRITURE DES NOEUDS                                 *
   ****************************************************************************/

  std::vector< size_t > nodeNumByID;
  if ( myMesh->HasNumerationHoles() )
    nodeNumByID.resize( myMesh->MaxNodeID() + 1 );

  int num;
  SMDS_NodeIteratorPtr itNodes=myMesh->nodesIterator();
  for ( num = 1; itNodes->more(); ++num )
  {
    const SMDS_MeshNode * node = itNodes->next();
    fprintf(aFileId, "%d %.14e %.14e %.14e\n", num, node->X(), node->Y(), node->Z());

    if ( !nodeNumByID.empty() )
      nodeNumByID[ node->GetID() ] = num;
  }

  /****************************************************************************
   *                       ECRITURE DES ELEMENTS                                *
   ****************************************************************************/
  /* Ecriture des connectivites, noms, numeros des mailles */

  num = 1;
  for ( SMDS_EdgeIteratorPtr itEdges = myMesh->edgesIterator(); itEdges->more(); ++num )
  {
    const SMDS_MeshElement * elem = itEdges->next();
    fprintf(aFileId, "%d %d ", num, 100 + elem->NbNodes());

    for ( SMDS_ElemIteratorPtr it = elem->nodesIterator(); it->more(); )
    {
      smIdType nodeID = it->next()->GetID();
      if ( !nodeNumByID.empty() )
        nodeID = nodeNumByID[ nodeID ];
      fprintf(aFileId, "%ld ", nodeID );
    }
    fprintf(aFileId, "\n");
  }

  for ( SMDS_FaceIteratorPtr itFaces = myMesh->facesIterator(); itFaces->more(); ++num )
  {
    const SMDS_MeshElement * elem = itFaces->next();

    fprintf(aFileId, "%d %d ", num, (elem->IsPoly() ? 400 : 200 ) + elem->NbNodes() );

    for( SMDS_ElemIteratorPtr it = elem->nodesIterator(); it->more(); )
    {
      smIdType nodeID = it->next()->GetID();
      if ( !nodeNumByID.empty() )
        nodeID = nodeNumByID[ nodeID ];
      fprintf(aFileId, "%ld ", nodeID );
    }
    fprintf(aFileId, "\n");
  }


  const SMDS_MeshVolume* v;
  for ( SMDS_VolumeIteratorPtr itVolumes=myMesh->volumesIterator(); itVolumes->more(); ++num )
  {
    const SMDS_MeshElement * elem = itVolumes->next();
    if ( elem->IsPoly() )
    {
      fprintf(aFileId, "%d %d ", num, 500 + elem->NbNodes());

      if (( v = myMesh->DownCast< SMDS_MeshVolume >( elem )))
      {
        std::vector<int> quant = v->GetQuantities();
        if ( !quant.empty() )
        {
          fprintf(aFileId, "%d %d ", (int)quant.size(), quant[0]);
          for ( size_t i = 1; i < quant.size(); ++i )
            fprintf(aFileId, "%d ", quant[i]);
        }
      }
    }
    else
    {
      fprintf(aFileId, "%d %d ", num, 300 + elem->NbNodes());
    }

    for( SMDS_ElemIteratorPtr it = elem->nodesIterator(); it->more(); )
    {
      smIdType nodeID = it->next()->GetID();
      if ( !nodeNumByID.empty() )
        nodeID = nodeNumByID[ nodeID ];
      fprintf(aFileId, "%ld ", nodeID );
    }

    fprintf(aFileId, "\n");
  }

  fclose(aFileId);

  return aResult;
}
