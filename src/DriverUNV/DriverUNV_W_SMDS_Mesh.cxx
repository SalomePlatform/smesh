//  SMESH DriverUNV : driver to read and write 'unv' files
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : DriverUNV_W_SMDS_Mesh.cxx
//  Module : SMESH

using namespace std;
#include "DriverUNV_W_SMDS_Mesh.h"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_MeshEdgesIterator.hxx"
#include "SMDS_MeshFacesIterator.hxx"
#include "SMDS_MeshNodesIterator.hxx"
#include "SMDS_MeshVolumesIterator.hxx"

#include <utilities.h>

#define sNODE_UNV_ID "  2411"
#define sELT_UNV_ID  "  2412"
#define sUNV_SEPARATOR "    -1"
#define sNODE_UNV_DESCR "%10d         1         1        11\n"
#define sELT_SURF_DESC  "%10d        %2d         1         1        11         %1d\n"
#define sELT_VOLU_DESC  "%10d        %2d         1         1         9         %1d\n"
#define sELT_BEAM_DESC1 "%10d        %2d         1         1         7         %1d\n"
#define sELT_BEAM_DESC2 "         0         1         1\n"

DriverUNV_W_SMDS_Mesh::DriverUNV_W_SMDS_Mesh() {
;
}

DriverUNV_W_SMDS_Mesh::~DriverUNV_W_SMDS_Mesh() {
;
}

void DriverUNV_W_SMDS_Mesh::SetMesh(Handle(SMDS_Mesh)& aMesh) {
  myMesh = aMesh;
}

void DriverUNV_W_SMDS_Mesh::SetFile(string aFile) {
  myFile = aFile;
}

void DriverUNV_W_SMDS_Mesh::SetFileId(FILE* aFileId) {
  myFileId = aFileId;
}

void DriverUNV_W_SMDS_Mesh::SetMeshId(int aMeshId) {
  myMeshId = aMeshId;
}

void DriverUNV_W_SMDS_Mesh::Add() {
  ;
}

void DriverUNV_W_SMDS_Mesh::Write() {

  int nbNodes,nbCells;
  int i;

  char* file2Read = (char*)myFile.c_str();
  myFileId = fopen(file2Read,"w+");
  if (myFileId < 0)
    {
      fprintf(stderr,">> ERREUR : ouverture du fichier %s \n",file2Read);
      exit(EXIT_FAILURE);
    }
  SCRUTE(myMesh);
  /****************************************************************************
  *                       NOMBRES D'OBJETS                                    *
  ****************************************************************************/
  fprintf(stdout,"\n(****************************)\n");
  fprintf(stdout,"(* INFORMATIONS GENERALES : *)\n");
  fprintf(stdout,"(****************************)\n");

  /* Combien de noeuds ? */
  nbNodes = myMesh->NbNodes();

  /* Combien de mailles, faces ou aretes ? */
  Standard_Integer nb_of_nodes, nb_of_edges,nb_of_faces, nb_of_volumes;
  nb_of_edges = myMesh->NbEdges();
  nb_of_faces = myMesh->NbFaces();
  nb_of_volumes = myMesh->NbVolumes();
  nbCells = nb_of_edges + nb_of_faces + nb_of_volumes;
  SCRUTE(nb_of_edges);
  SCRUTE(nb_of_faces);
  SCRUTE(nb_of_volumes);

  fprintf(stdout,"%d %d\n",nbNodes,nbCells);
  fprintf(myFileId,"%d %d\n",nbNodes,nbCells);

  /****************************************************************************
  *                       ECRITURE DES NOEUDS                                 *
  ****************************************************************************/
  fprintf(stdout,"\n(************************)\n");
  fprintf(stdout,"(* NOEUDS DU MAILLAGE : *)\n");
  fprintf(stdout,"(************************)\n");

  SMDS_MeshNodesIterator itNodes(myMesh);

  fprintf(myFileId,"%s\n", sUNV_SEPARATOR);
  fprintf(myFileId,"%s\n", sNODE_UNV_ID  );

  for (;itNodes.More();itNodes.Next()) {
    const Handle(SMDS_MeshElement)& elem = itNodes.Value();
    const Handle(SMDS_MeshNode   )& node = myMesh->GetNode(1, elem);

    fprintf(myFileId, sNODE_UNV_DESCR, node->GetID());
    fprintf(myFileId, "%25.16E%25.16E%25.16E\n", node->X(), node->Y(), node->Z());
  }
  fprintf(myFileId,"%s\n", sUNV_SEPARATOR);

  /****************************************************************************
  *                       ECRITURE DES ELEMENTS                                *
  ****************************************************************************/
  fprintf(stdout,"\n(**************************)\n");
  fprintf(stdout,"(* ELEMENTS DU MAILLAGE : *)\n");
  fprintf(stdout,"(**************************)");
  /* Ecriture des connectivites, noms, numeros des mailles */

  fprintf(myFileId,"%s\n", sUNV_SEPARATOR);
  fprintf(myFileId,"%s\n", sELT_UNV_ID   );

  SMDS_MeshEdgesIterator itEdges(myMesh);
  for (;itEdges.More();itEdges.Next()) {
    const Handle(SMDS_MeshElement)& elem = itEdges.Value();

    switch (elem->NbNodes()) {
      case 2 : {
        fprintf(myFileId, sELT_BEAM_DESC1, elem->GetID(), 21, elem->NbNodes());
        fprintf(myFileId, sELT_BEAM_DESC2);
        fprintf(myFileId, "%10d%10d\n", elem->GetConnection(1), elem->GetConnection(2));
        break;
      } 
      case 3 : {
        fprintf(myFileId, sELT_BEAM_DESC1, elem->GetID(), 24, elem->NbNodes());
        fprintf(myFileId, sELT_BEAM_DESC2);
        fprintf(myFileId, "%10d%10d%10d\n",elem->GetConnection(1), elem->GetConnection(2), elem->GetConnection(3));
        break;
      }
    }
  }

  SMDS_MeshFacesIterator itFaces(myMesh);
  for (;itFaces.More();itFaces.Next()) {
    const Handle(SMDS_MeshElement)& elem = itFaces.Value();

    switch (elem->NbNodes()) {
      case 3 :
        // linear triangle
        fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 74, elem->NbNodes());
        break;
      case 4 :
        // linear quadrilateral
        fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 71, elem->NbNodes());
        break;
      case 6 :
        // parabolic triangle
        fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 72, elem->NbNodes());
        break;
      case 8 :
        // parabolic quadrilateral
        fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 75, elem->NbNodes());
        break;
      default:
        fprintf(myFileId, "element not registered\n");
    }

    for (i=0;i<elem->NbNodes();i++)
      fprintf(myFileId,"%10d",elem->GetConnection(i+1));
    
    fprintf(myFileId,"\n");
  }

  SMDS_MeshVolumesIterator itVolumes(myMesh);
  for (;itVolumes.More();itVolumes.Next()) {
    const Handle(SMDS_MeshElement)& elem = itVolumes.Value();

    switch (elem->NbNodes()) {
      case 4 : 
        // linear tetrahedron
        fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 111, elem->NbNodes());
        break;
      case 6 : 
        // linear tetrahedron
        fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 112, elem->NbNodes());
        break;
      case 8 : 
        // linear brick
        fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 115, elem->NbNodes());
        break;
    }

    for (i=0;i<elem->NbNodes();i++)
      fprintf(myFileId,"%10d",elem->GetConnection(i+1));
    
    fprintf(myFileId,"\n");
  }
  fprintf(myFileId,"%s\n", sUNV_SEPARATOR);

  fclose (myFileId);
}
