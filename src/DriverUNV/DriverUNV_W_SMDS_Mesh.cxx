#include "DriverUNV_W_SMDS_Mesh.h"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"

#include <utilities.h>

#define sNODE_UNV_ID "  2411"
#define sELT_UNV_ID  "  2412"
#define sUNV_SEPARATOR "    -1"
#define sNODE_UNV_DESCR "%10d         1         1        11\n"
#define sELT_SURF_DESC  "%10d        %2d         1         1        11         %1d\n"
#define sELT_VOLU_DESC  "%10d        %2d         1         1         9         %1d\n"
#define sELT_BEAM_DESC1 "%10d        %2d         1         1         7         %1d\n"
#define sELT_BEAM_DESC2 "         0         1         1\n"

extern "C"
{

/**
 * Factory function which will be called by SMESHDriver
 */
void * SMESH_createUNVMeshWriter()
{
	return new DriverUNV_W_SMDS_Mesh();
}

}

DriverUNV_W_SMDS_Mesh::DriverUNV_W_SMDS_Mesh()
{
	;
}

DriverUNV_W_SMDS_Mesh::~DriverUNV_W_SMDS_Mesh()
{
	;
}

void DriverUNV_W_SMDS_Mesh::SetMesh(SMDS_Mesh * aMesh)
{
	myMesh = aMesh;
}

void DriverUNV_W_SMDS_Mesh::SetFile(string aFile)
{
	myFile = aFile;
}

void DriverUNV_W_SMDS_Mesh::SetFileId(FILE * aFileId)
{
	myFileId = aFileId;
}

void DriverUNV_W_SMDS_Mesh::SetMeshId(int aMeshId)
{
	myMeshId = aMeshId;
}

void DriverUNV_W_SMDS_Mesh::Add()
{
	;
}

void DriverUNV_W_SMDS_Mesh::Write()
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
	SCRUTE(myMesh);
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
	SCRUTE(nb_of_edges);
	SCRUTE(nb_of_faces);
	SCRUTE(nb_of_volumes);

	fprintf(stdout, "%d %d\n", nbNodes, nbCells);

  /****************************************************************************
  *                       ECRITURE DES NOEUDS                                 *
  ****************************************************************************/
	fprintf(stdout, "\n(************************)\n");
	fprintf(stdout, "(* NOEUDS DU MAILLAGE : *)\n");
	fprintf(stdout, "(************************)\n");

	fprintf(myFileId, "%s\n", sUNV_SEPARATOR);
	fprintf(myFileId, "%s\n", sNODE_UNV_ID);

	SMDS_Iterator<const SMDS_MeshNode *> * itNodes=myMesh->nodesIterator();
	while(itNodes->more())
	{
		const SMDS_MeshNode * node = itNodes->next();

		fprintf(myFileId, sNODE_UNV_DESCR, node->GetID());
		fprintf(myFileId, "%25.16E%25.16E%25.16E\n", node->X(), node->Y(),
			node->Z());
	}
	delete itNodes;
	fprintf(myFileId, "%s\n", sUNV_SEPARATOR);

  /****************************************************************************
  *                       ECRITURE DES ELEMENTS                                *
  ****************************************************************************/
	fprintf(stdout, "\n(**************************)\n");
	fprintf(stdout, "(* ELEMENTS DU MAILLAGE : *)\n");
	fprintf(stdout, "(**************************)");
	/* Ecriture des connectivites, noms, numeros des mailles */

	fprintf(myFileId, "%s\n", sUNV_SEPARATOR);
	fprintf(myFileId, "%s\n", sELT_UNV_ID);

	SMDS_Iterator<const SMDS_MeshEdge *> * itEdges=myMesh->edgesIterator();
	while(itEdges->more())
	{
		const SMDS_MeshElement * elem = itEdges->next();
		SMDS_Iterator<const SMDS_MeshElement*> *itn=elem->nodesIterator();

		switch (elem->NbNodes())
		{
		case 2:
			fprintf(myFileId, sELT_BEAM_DESC1, elem->GetID(), 21,
				elem->NbNodes());
			fprintf(myFileId, sELT_BEAM_DESC2);
			fprintf(myFileId, "%10d%10d\n", itn->next()->GetID(),
				itn->next()->GetID());
			break;

		case 3:
			fprintf(myFileId, sELT_BEAM_DESC1, elem->GetID(), 24,
				elem->NbNodes());
			fprintf(myFileId, sELT_BEAM_DESC2);
			fprintf(myFileId, "%10d%10d%10d\n", itn->next()->GetID(),
				itn->next()->GetID(), itn->next()->GetID());

			break;
		}
		delete itn;
	}
	delete itEdges;

	SMDS_Iterator<const SMDS_MeshFace*> * itFaces=myMesh->facesIterator();
	while(itFaces->more())
	{
		const SMDS_MeshElement * elem = itFaces->next();

		switch (elem->NbNodes())
		{
		case 3:
			// linear triangle
			fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 74,
				elem->NbNodes());
			break;
		case 4:
			// linear quadrilateral
			fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 71,
				elem->NbNodes());
			break;
		case 6:
			// parabolic triangle
			fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 72,
				elem->NbNodes());
			break;
		case 8:
			// parabolic quadrilateral
			fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 75,
				elem->NbNodes());
			break;
		default:
			fprintf(myFileId, "element not registered\n");
		}

		SMDS_Iterator<const SMDS_MeshElement*> *itn=elem->nodesIterator();
		while(itn->more()) fprintf(myFileId, "%10d", itn->next()->GetID());
		delete itn;

		fprintf(myFileId, "\n");
	}
	delete itFaces;

	SMDS_Iterator<const SMDS_MeshVolume*> * itVolumes=myMesh->volumesIterator();
	while(itVolumes->more())
	{
		const SMDS_MeshElement * elem = itVolumes->next();

		switch (elem->NbNodes())
		{
		case 4:
			// linear tetrahedron
			fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 111,
				elem->NbNodes());
			break;
		case 6:
			// linear tetrahedron
			fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 112,
				elem->NbNodes());
			break;
		case 8:
			// linear brick
			fprintf(myFileId, sELT_SURF_DESC, elem->GetID(), 115,
				elem->NbNodes());
			break;
		}

		SMDS_Iterator<const SMDS_MeshElement*> *itn=elem->nodesIterator();
		while(itn->more()) fprintf(myFileId, "%10d", itn->next()->GetID());
		delete itn;

		fprintf(myFileId, "\n");
	}
	delete itVolumes;

	fprintf(myFileId, "%s\n", sUNV_SEPARATOR);

	fclose(myFileId);
}
