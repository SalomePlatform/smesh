using namespace std;
#include "DriverDAT_W_SMESHDS_Mesh.h"
#include "DriverDAT_W_SMDS_Mesh.h"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_MeshEdgesIterator.hxx"
#include "SMDS_MeshFacesIterator.hxx"
#include "SMDS_MeshNodesIterator.hxx"
#include "SMDS_MeshVolumesIterator.hxx"

#include "utilities.h"

DriverDAT_W_SMESHDS_Mesh::DriverDAT_W_SMESHDS_Mesh() {
;
}

DriverDAT_W_SMESHDS_Mesh::~DriverDAT_W_SMESHDS_Mesh() {
;
}

void DriverDAT_W_SMESHDS_Mesh::SetMesh(Handle(SMDS_Mesh)& aMesh) {
  //myMesh = Handle(SMESHDS_Mesh)::DownCast(aMesh);
  myMesh = aMesh;
}

void DriverDAT_W_SMESHDS_Mesh::SetFile(string aFile) {
  myFile = aFile;
}

void DriverDAT_W_SMESHDS_Mesh::SetFileId(FILE* aFileId) {
  myFileId = aFileId;
}

void DriverDAT_W_SMESHDS_Mesh::SetMeshId(int aMeshId) {
  myMeshId = aMeshId;
}

void DriverDAT_W_SMESHDS_Mesh::Write() {

  string myClass = string("SMDS_Mesh");
  string myExtension = string("DAT");

  DriverDAT_W_SMDS_Mesh* myWriter = new DriverDAT_W_SMDS_Mesh;

  myWriter->SetMesh(myMesh);
  myWriter->SetFile(myFile);
  myWriter->SetMeshId(myMeshId);
  //myWriter->SetFileId(myFileId);

  myWriter->Write();


}
void DriverDAT_W_SMESHDS_Mesh::Add() {
  int nbNodes,nbCells;
  int i;

  char* file2Read = (char*)myFile.c_str();
  myFileId = fopen(file2Read,"w+");
  if (myFileId < 0)
    {
      fprintf(stderr,">> ERREUR : ouverture du fichier %s \n",file2Read);
      exit(EXIT_FAILURE);
    }
  
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

  fprintf(stdout,"%d %d\n",nbNodes,nbCells);
  fprintf(myFileId,"%d %d\n",nbNodes,nbCells);

  /****************************************************************************
  *                       ECRITURE DES NOEUDS                                 *
  ****************************************************************************/
  fprintf(stdout,"\n(************************)\n");
  fprintf(stdout,"(* NOEUDS DU MAILLAGE : *)\n");
  fprintf(stdout,"(************************)\n");

  SMDS_MeshNodesIterator itNodes(myMesh);
  for (;itNodes.More();itNodes.Next()) {
    const Handle(SMDS_MeshElement)& elem = itNodes.Value();
    const Handle(SMDS_MeshNode)& node = myMesh->GetNode(1,elem);
 
    fprintf(myFileId,"%d %e %e %e\n",node->GetID(),node->X(),node->Y(),node->Z());
  }

  /****************************************************************************
  *                       ECRITURE DES ELEMENTS                                *
  ****************************************************************************/
  fprintf(stdout,"\n(**************************)\n");
  fprintf(stdout,"(* ELEMENTS DU MAILLAGE : *)\n");
  fprintf(stdout,"(**************************)");
  /* Ecriture des connectivites, noms, numeros des mailles */

  SMDS_MeshEdgesIterator itEdges(myMesh);
  for (;itEdges.More();itEdges.Next()) {
    const Handle(SMDS_MeshElement)& elem = itEdges.Value();

    switch (elem->NbNodes()) {
    case 2 : {
      fprintf(myFileId,"%d %d ",elem->GetID(),102);
      break;
    } 
    case 3 : {
      fprintf(myFileId,"%d %d ",elem->GetID(),103);
      break;
    }
    }

    for (i=0;i<elem->NbNodes();i++)
      fprintf(myFileId,"%d ",elem->GetConnection(i+1));
    
    fprintf(myFileId,"\n");
  }

  SMDS_MeshFacesIterator itFaces(myMesh);
  for (;itFaces.More();itFaces.Next()) {
    const Handle(SMDS_MeshElement)& elem = itFaces.Value();

    switch (elem->NbNodes()) {
    case 3 : {
      fprintf(myFileId,"%d %d ",elem->GetID(),203);
      break;
    }
    case 4 : {
      fprintf(myFileId,"%d %d ",elem->GetID(),204);
      break;
    }
    case 6 : {
      fprintf(myFileId,"%d %d ",elem->GetID(),206);
      break;
    }
    }

    for (i=0;i<elem->NbNodes();i++)
      fprintf(myFileId,"%d ",elem->GetConnection(i+1));
    
    fprintf(myFileId,"\n");
  }

  SMDS_MeshVolumesIterator itVolumes(myMesh);
  for (;itVolumes.More();itVolumes.Next()) {
    const Handle(SMDS_MeshElement)& elem = itVolumes.Value();

    switch (elem->NbNodes()) {
    case 8 : {
      fprintf(myFileId,"%d %d ",elem->GetID(),308);
      break;
    }
    }

    for (i=0;i<elem->NbNodes();i++)
      fprintf(myFileId,"%d ",elem->GetConnection(i+1));
    
    fprintf(myFileId,"\n");
  }

  fclose (myFileId);
}

