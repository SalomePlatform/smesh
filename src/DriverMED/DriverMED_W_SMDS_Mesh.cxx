//  SMESH DriverMED : driver to read and write 'med' files
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
//  File   : DriverMED_W_SMDS_Mesh.cxx
//  Module : SMESH

using namespace std;
#include "DriverMED_W_SMDS_Mesh.h"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"




#include "utilities.h"
#include <vector>

DriverMED_W_SMDS_Mesh::DriverMED_W_SMDS_Mesh()
{
	;
}

DriverMED_W_SMDS_Mesh::~DriverMED_W_SMDS_Mesh()
{
	;
}

void DriverMED_W_SMDS_Mesh::SetMesh(SMDS_Mesh * aMesh)
{
	myMesh = aMesh;
}

void DriverMED_W_SMDS_Mesh::SetFile(string aFile)
{
	myFile = aFile;
}

void DriverMED_W_SMDS_Mesh::SetFileId(med_idt aFileId)
{
	myFileId = aFileId;
}

void DriverMED_W_SMDS_Mesh::SetMeshId(int aMeshId)
{
	myMeshId = aMeshId;
}

void DriverMED_W_SMDS_Mesh::Add()
{
	;
}

void DriverMED_W_SMDS_Mesh::Write()
{

	med_err ret = 0;
	int i, j, k, l;
	int numero;
	char message[200];
	bool ok;
	/* nombre d'objets MED */
	char nom_universel[MED_TAILLE_LNOM + 1];
	med_int long_fichier_en_tete;
	char *fichier_en_tete;
	char version_hdf[10];
	char version_med[10];
	med_int nmaa, mdim, nnoe;
	med_int nmai[MED_NBR_GEOMETRIE_MAILLE], nfac[MED_NBR_GEOMETRIE_FACE];
	med_int nare[MED_NBR_GEOMETRIE_ARETE];
	/* nom du maillage */
	char nommaa[MED_TAILLE_NOM + 1];
	/* noeuds */
	med_float *coo;
	char nomcoo[3 * MED_TAILLE_PNOM + 1];
	char unicoo[3 * MED_TAILLE_PNOM + 1];
	char *nomnoe;
	med_int *numnoe;
	med_int *nufano;
	med_repere rep;
	med_booleen inonoe, inunoe;
	med_mode_switch mode_coo;
	char str[MED_TAILLE_PNOM + 1];
	med_int nbNodes;
	/* elements */
	med_int nsup;
	med_int edim;
	med_int taille;
	med_int elem_id, myId;
	med_int *connectivite;
	char *nomele;
	med_int *numele;
	med_int *nufael;
	med_booleen inoele, inuele;
	med_connectivite typ_con;
	med_geometrie_element typgeo;
	med_geometrie_element typmai[MED_NBR_GEOMETRIE_MAILLE] =
		{ MED_POINT1, MED_SEG2,
		MED_SEG3, MED_TRIA3,
		MED_TRIA6, MED_QUAD4,
		MED_QUAD8, MED_TETRA4,
		MED_TETRA10, MED_HEXA8,
		MED_HEXA20, MED_PENTA6,
		MED_PENTA15, MED_PYRA5,
		MED_PYRA13
	};
	med_int desmai[MED_NBR_GEOMETRIE_MAILLE] =
		{ 0, 2, 3, 3, 3, 4, 4, 4, 4, 6, 6, 5, 5, 5, 5 };
	med_int nmailles[MED_NBR_GEOMETRIE_MAILLE];
	char nommai[MED_NBR_GEOMETRIE_MAILLE][MED_TAILLE_NOM + 1] = { "MED_POINT1",
		"MED_SEG2",
		"MED_SEG3",
		"MED_TRIA3",
		"MED_TRIA6",
		"MED_QUAD4",
		"MED_QUAD8",
		"MED_TETRA4",
		"MED_TETRA10",
		"MED_HEXA8",
		"MED_HEXA20",
		"MED_PENTA6",
		"MED_PENTA15",
		"MED_PYRA5",
		"MED_PYRA13"
	};
	med_geometrie_element typfac[MED_NBR_GEOMETRIE_FACE] =
		{ MED_TRIA3, MED_TRIA6,
		MED_QUAD4, MED_QUAD8
	};
	med_int desfac[MED_NBR_GEOMETRIE_FACE] = { 3, 3, 4, 4 };
	med_int nfaces[MED_NBR_GEOMETRIE_FACE];
	char nomfac[MED_NBR_GEOMETRIE_FACE][MED_TAILLE_NOM + 1] =
		{ "MED_TRIA3", "MED_TRIA6",
		"MED_QUAD4", "MED_QUAD8"
	};
	med_geometrie_element typare[MED_NBR_GEOMETRIE_ARETE] =
		{ MED_SEG2, MED_SEG3 };
	med_int desare[MED_NBR_GEOMETRIE_ARETE] = { 2, 3 };
	med_int naretes[MED_NBR_GEOMETRIE_ARETE];
	char nomare[MED_NBR_GEOMETRIE_ARETE][MED_TAILLE_NOM + 1] =
		{ "MED_SEG2", "MED_SEG3" };
	/* familles */
	med_int nfam;
	med_int natt, ngro;
	char *attdes, *gro;
	med_int *attval, *attide;
	char nomfam[MED_TAILLE_NOM + 1];
	med_int numfam;
	char str1[MED_TAILLE_DESC + 1];
	char str2[MED_TAILLE_LNOM + 1];

  /****************************************************************************
  *                      OUVERTURE DU FICHIER EN ECRITURE                      *
  ****************************************************************************/
	/*!  char* file2Read = (char*)myFile.c_str();
	 * myFileId = MEDouvrir(file2Read,MED_REMP);
	 * if (myFileId < 0)
	 * {
	 * fprintf(stderr,">> ERREUR : ouverture du fichier %s \n",file2Read);
	 * exit(EXIT_FAILURE);
	 * }
	 */
	typ_con = MED_NOD;
	mode_coo = MED_FULL_INTERLACE;
	numero = myMeshId;

  /****************************************************************************
  *                       NOMBRES D'OBJETS MED                                *
  ****************************************************************************/
	fprintf(stdout, "\n(****************************)\n");
	fprintf(stdout, "(* INFORMATIONS GENERALES : *)\n");
	fprintf(stdout, "(****************************)\n");

	/* creation du maillage */
	mdim = 3;
	//nommaa = QString("Mesh "+myMeshId).latin1();
	//nommaa = "";
	//plutot recuperer le nom du maillage dans l'etude
	if (ret == 0)
		ret = MEDmaaCr(myFileId, nommaa, mdim);
	printf("%d\n", ret);

	/* Combien de noeuds ? */
	nnoe = myMesh->NbNodes();
	//SCRUTE(nnoe);
	/* Combien de mailles, faces ou aretes ? */
	for (i = 0; i < MED_NBR_GEOMETRIE_MAILLE; i++)
		nmailles[i] = 0;

	SMDS_Iterator<const SMDS_MeshFace *> * itFaces=myMesh->facesIterator();
	int nb_of_nodes, nb_of_faces;
	nb_of_faces = myMesh->NbFaces();
	//SCRUTE(nb_of_faces);

	//med_int elem_Id[MED_NBR_GEOMETRIE_FACE][nb_of_faces];
	vector<int> elem_Id[MED_NBR_GEOMETRIE_MAILLE];
	//typedef vector<int> Integer_vector;
	//vector<Integer_vector> elem_Id;
	//elem_Id.resize(MED_NBR_GEOMETRIE_MAILLE);

	while(itFaces->more())
	{
		const SMDS_MeshFace * elem = itFaces->next();

		nb_of_nodes = elem->NbNodes();

		switch (nb_of_nodes)
		{
		case 3:
		{
			//elem_Id[3][nmailles[3]] = elem->GetID();
			elem_Id[3].push_back(elem->GetID());
			nmailles[3]++;
			break;
		}
		case 4:
		{
			//elem_Id[5][nmailles[5]] = elem->GetID();
			elem_Id[5].push_back(elem->GetID());
			nmailles[5]++;
			break;
		}
		case 6:
		{
			//elem_Id[4][nmailles[4]] = elem->GetID();
			elem_Id[4].push_back(elem->GetID());
			nmailles[4]++;
			break;
		}
		}

	}

	SMDS_Iterator<const SMDS_MeshVolume*> * itVolumes=myMesh->volumesIterator();
	while(itVolumes->more())
	{
		const SMDS_MeshVolume * elem = itVolumes->next();

		nb_of_nodes = elem->NbNodes();
		switch (nb_of_nodes)
		{
		case 8:
		{
			//elem_Id[9][nmailles[9]] = elem->GetID();
			elem_Id[9].push_back(elem->GetID());
			nmailles[9]++;
			break;
		}
		case 4 :
		{
		        elem_Id[7].push_back(elem->GetID());
			nmailles[7]++;
			break;
		}
		}
	}

  /****************************************************************************
  *                       ECRITURE DES NOEUDS                                  *
  ****************************************************************************/
	fprintf(stdout, "\n(************************)\n");
	fprintf(stdout, "(* NOEUDS DU MAILLAGE : *)\n");
	fprintf(stdout, "(************************)\n");

	/* Allocations memoires */
	/* table des coordonnees 
	 * profil : (dimension * nombre de noeuds ) */
	coo = (med_float *) malloc(sizeof(med_float) * nnoe * mdim);
	/* table  des numeros, des numeros de familles des noeuds
	 * profil : (nombre de noeuds) */
	numnoe = (med_int *) malloc(sizeof(med_int) * nnoe);
	nufano = (med_int *) malloc(sizeof(med_int) * nnoe);
	/* table des noms des noeuds 
	 * profil : (nnoe*MED_TAILLE_PNOM+1) */
	nomnoe = "";

	i = 0;
	SMDS_Iterator<const SMDS_MeshNode *> * itNodes=myMesh->nodesIterator();
	while(itNodes->more())
	{		
		const SMDS_MeshNode * node = itNodes->next();
		coo[i * 3] = node->X();
		coo[i * 3 + 1] = node->Y();
		coo[i * 3 + 2] = node->Z();
		numnoe[i] = node->GetID();
		nufano[i] = 0;
		i++;
	}

	/* ecriture des noeuds : 
	 * - coordonnees
	 * - noms (optionnel dans un fichier MED) 
	 * - numeros (optionnel dans un fichier MED) 
	 * - numeros des familles */
	ret = MEDnoeudsEcr(myFileId, nommaa, mdim, coo, mode_coo, MED_CART,
		nomcoo, unicoo, nomnoe, MED_FAUX, numnoe, MED_VRAI,
		nufano, nnoe, MED_REMP);

	/* liberation memoire */
	free(coo);
	//free(nomnoe);
	free(numnoe);
	free(nufano);

  /****************************************************************************
  *                       ECRITURE DES ELEMENTS                                *
  ****************************************************************************/
	fprintf(stdout, "\n(**************************)\n");
	fprintf(stdout, "(* ELEMENTS DU MAILLAGE : *)\n");
	fprintf(stdout, "(**************************)");
	//fprintf(Out,"CELLS\n");
	/* Ecriture des connectivites, noms, numeros des mailles */

	if (ret == 0)
		for (i = 0; i < MED_NBR_GEOMETRIE_MAILLE; i++)
		{
			if (nmailles[i] > 0 && ret == 0)
			{
				MESSAGE(" Start " << typmai[i]);
				/* dimension de la maille */
				edim = typmai[i] / 100;
				nsup = 0;
				if (mdim == 2 || mdim == 3)
					if (edim == 1)
						nsup = 1;
				if (mdim == 3)
					if (edim == 2)
						nsup = 1;

				taille = nsup + typmai[i] % 100;
				//taille = typmai[i]%100;

				/* allocation memoire */
				connectivite = (med_int *) malloc(sizeof(med_int) *
					taille * nmailles[i]);
				nomele = (char *)malloc(sizeof(char) * MED_TAILLE_PNOM *
					nmailles[i] + 1);
				numele = (med_int *) malloc(sizeof(med_int) * nmailles[i]);
				nufael = (med_int *) malloc(sizeof(med_int) * nmailles[i]);
				nomele = "";
				nbNodes = typmai[i] % 100;

				//penser a mater les (taille-nsup)
				for (j = 0; j < nmailles[i]; j++)
				{
					myId = elem_Id[i][j];
					const SMDS_MeshElement * elem =
						myMesh->FindElement(myId);
					*(numele + j) = myId;
					//elem_id=*(numele+j);
					//fprintf(stdout,"%d \n",myId);

					SMDS_Iterator<const SMDS_MeshElement *> * itNode=
						elem->nodesIterator();

					while(itNode->more())
					{
						//*(connectivite+j*(taille-1)+k)=cnx[k];
						*(connectivite + j * (taille - nsup) + k) =
							itNode->next()->GetID();
						//fprintf(stdout,"%d ",*(connectivite+j*(taille-nsup)+k));
					}
					nufael[j] = 0;
					//fprintf(stdout,"\n");
				}

				/* ecriture des données */

				ret =
					MEDelementsEcr(myFileId, nommaa, mdim, connectivite,
					mode_coo, nomele, MED_FAUX, numele, MED_VRAI, nufael,
					nmailles[i], MED_MAILLE, typmai[i], typ_con, MED_REMP);

				if (ret < 0)
					MESSAGE(">> ERREUR : lecture des mailles \n");

				/* liberation memoire */
				free(connectivite);
				//free(nomele);
				free(numele);
				free(nufael);
				MESSAGE(" End " << typmai[i]);
			}
		}

  /****************************************************************************
  *                      FERMETURE DU FICHIER                                 *
  ****************************************************************************/

	/*!  ret = MEDfermer(myFileId);
	 * 
	 * if (ret != 0)
	 * fprintf(stderr,">> ERREUR : erreur a la fermeture du fichier %s\n",file2Read);
	 */

}
