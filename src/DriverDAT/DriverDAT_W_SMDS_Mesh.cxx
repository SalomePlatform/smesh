//  SMESH DriverDAT : driver to read and write 'dat' files
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
//  File   : DriverDAT_W_SMDS_Mesh.cxx
//  Module : SMESH

#include "DriverDAT_W_SMDS_Mesh.h"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "utilities.h"

extern "C"
{

/**
 * Factory function which will be called by SMESHDriver
 */
void * SMESH_createDATMeshWriter()
{
	return new DriverDAT_W_SMDS_Mesh();
}
}

DriverDAT_W_SMDS_Mesh::DriverDAT_W_SMDS_Mesh()
{
}

DriverDAT_W_SMDS_Mesh::~DriverDAT_W_SMDS_Mesh()
{
	;
}

void DriverDAT_W_SMDS_Mesh::SetMesh(SMDS_Mesh * aMesh)
{
	myMesh = aMesh;
}

void DriverDAT_W_SMDS_Mesh::SetFile(string aFile)
{
	myFile = aFile;
}

void DriverDAT_W_SMDS_Mesh::SetFileId(FILE * aFileId)
{
	myFileId = aFileId;
}

void DriverDAT_W_SMDS_Mesh::SetMeshId(int aMeshId)
{
	myMeshId = aMeshId;
}

void DriverDAT_W_SMDS_Mesh::Add()
{
	MESSAGE("Adding a mesh to a DAT document. As DAT do not support more than one mesh in a file, the previous mesh is deleted");
	Write();
}

void DriverDAT_W_SMDS_Mesh::Write()
{
	int nbNodes, nbCells;
	int i;

	char *file2Read = (char *)myFile.c_str();
	myFileId = fopen(file2Read, "w+");
	if (myFileId < 0)
	{
		fprintf(stderr, ">> ERREUR : ouverture du fichier %s \n", file2Read);
		exit(EXIT_FAILURE);
	}

  /****************************************************************************
  *                       NOMBRES D'OBJETS                                    *
  ****************************************************************************/
	fprintf(stdout, "\n(****************************)\n");
	fprintf(stdout, "(* INFORMATIONS GENERALES : *)\n");
	fprintf(stdout, "(****************************)\n");

	/* Combien de noeuds ? */
	nbNodes = myMesh->NbNodes();

	/* Combien de mailles, faces ou aretes ? */
	int nb_of_nodes, nb_of_edges, nb_of_faces, nb_of_volumes;
	nb_of_edges = myMesh->NbEdges();
	nb_of_faces = myMesh->NbFaces();
	nb_of_volumes = myMesh->NbVolumes();
	nbCells = nb_of_edges + nb_of_faces + nb_of_volumes;

	fprintf(stdout, "%d %d\n", nbNodes, nbCells);
	fprintf(myFileId, "%d %d\n", nbNodes, nbCells);

  /****************************************************************************
  *                       ECRITURE DES NOEUDS                                 *
  ****************************************************************************/
	fprintf(stdout, "\n(************************)\n");
	fprintf(stdout, "(* NOEUDS DU MAILLAGE : *)\n");
	fprintf(stdout, "(************************)\n");

	SMDS_Iterator<const SMDS_MeshNode *> * itNodes=myMesh->nodesIterator();
	while(itNodes->more())
	{		
		const SMDS_MeshNode * node = itNodes->next();
		fprintf(myFileId, "%d %e %e %e\n", node->GetID(), node->X(),
			node->Y(), node->Z());
	}
	delete itNodes;

  /****************************************************************************
  *                       ECRITURE DES ELEMENTS                                *
  ****************************************************************************/
	fprintf(stdout, "\n(**************************)\n");
	fprintf(stdout, "(* ELEMENTS DU MAILLAGE : *)\n");
	fprintf(stdout, "(**************************)");
	/* Ecriture des connectivites, noms, numeros des mailles */

	SMDS_Iterator<const SMDS_MeshEdge*> * itEdges=myMesh->edgesIterator();
	while(itEdges->more())
	{
		const SMDS_MeshEdge * elem = itEdges->next();

		switch (elem->NbNodes())
		{
		case 2:
		{
			fprintf(myFileId, "%d %d ", elem->GetID(), 102);
			break;
		}
		case 3:
		{
			fprintf(myFileId, "%d %d ", elem->GetID(), 103);
			break;
		}
		}

		SMDS_Iterator<const SMDS_MeshElement *> * itNodes=elem->nodesIterator();
		while(itNodes->more())
			fprintf(myFileId, "%d ", itNodes->next()->GetID());
		
		fprintf(myFileId, "\n");
	}
	delete itEdges;

	SMDS_Iterator<const SMDS_MeshFace *> * itFaces=myMesh->facesIterator();
	while(itFaces->more())
	{
		const SMDS_MeshElement * elem = itFaces->next();

		switch (elem->NbNodes())
		{
		case 3:
		{
			fprintf(myFileId, "%d %d ", elem->GetID(), 203);
			break;
		}
		case 4:
		{
			fprintf(myFileId, "%d %d ", elem->GetID(), 204);
			break;
		}
		case 6:
		{
			fprintf(myFileId, "%d %d ", elem->GetID(), 206);
			break;
		}
		}

		SMDS_Iterator<const SMDS_MeshElement *> * itNodes=elem->nodesIterator();
		while(itNodes->more())
			fprintf(myFileId, "%d ", itNodes->next()->GetID());
		delete itNodes;
	
		fprintf(myFileId, "\n");
	}
	delete itFaces;

	SMDS_Iterator<const SMDS_MeshVolume*> * itVolumes=myMesh->volumesIterator();
	while(itVolumes->more())
	{
		const SMDS_MeshElement * elem = itVolumes->next();
		
		switch (elem->NbNodes())
		{
		case 8:
		{
			fprintf(myFileId, "%d %d ", elem->GetID(), 308);
			break;
		}
		}

		SMDS_Iterator<const SMDS_MeshElement *> * itNodes=elem->nodesIterator();
		while(itNodes->more())
			fprintf(myFileId, "%d ", itNodes->next()->GetID());
		delete itNodes;	

		fprintf(myFileId, "\n");
	}
	delete itVolumes;

	fclose(myFileId);
}
