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
//  File   : DriverMED_W_SMESHDS_Mesh.cxx
//  Module : SMESH

using namespace std;
#include "DriverMED_W_SMESHDS_Mesh.h"
#include "DriverMED_W_SMDS_Mesh.h"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include <TopExp.hxx>
#include <vector>
#include "utilities.h"

extern "C"
{
/**
 * Factory function which will be called by SMESHDriver
 */
void * SMESH_createMEDMeshWriter()
{
	return new DriverMED_W_SMESHDS_Mesh();
}

}

DriverMED_W_SMESHDS_Mesh::DriverMED_W_SMESHDS_Mesh()
{
	;
}

DriverMED_W_SMESHDS_Mesh::~DriverMED_W_SMESHDS_Mesh()
{
	;
}

void DriverMED_W_SMESHDS_Mesh::SetMesh(SMDS_Mesh * aMesh)
{
	myMesh = aMesh;
}

void DriverMED_W_SMESHDS_Mesh::SetFile(string aFile)
{
	myFile = aFile;
}

void DriverMED_W_SMESHDS_Mesh::SetFileId(med_idt aFileId)
{
	myFileId = aFileId;
}

void DriverMED_W_SMESHDS_Mesh::SetMeshId(int aMeshId)
{
	myMeshId = aMeshId;
}

void DriverMED_W_SMESHDS_Mesh::Write()
{
	Add();
}

void DriverMED_W_SMESHDS_Mesh::Add()
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
	// PN : Initilialisation de nomcoo et unicoo pour lisibilite du maillage
	char nomcoo[3 * MED_TAILLE_PNOM + 1] = "x       y        z      ";
	char unicoo[3 * MED_TAILLE_PNOM + 1] = "m       m        m     ";
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

	typ_con = MED_NOD;
	mode_coo = MED_FULL_INTERLACE;
	numero = myMeshId;

	//---- provisoire : switch pour ecrire les familles de mailles
	int besoinfamilledemaille = 1;
	//---- provisoire : switch pour ecrire les familles de mailles

  /****************************************************************************
   *                      OUVERTURE DU FICHIER EN ECRITURE                    *
   ****************************************************************************/
	char *file2Read = (char *)myFile.c_str();

	MESSAGE(" file2Read " << file2Read)
		myFileId = MEDouvrir(file2Read, MED_REMP);
	if (myFileId < 0)
	{
		fprintf(stderr, ">> ERREUR : ouverture du fichier %s \n", file2Read);
		exit(EXIT_FAILURE);
	}

  /****************************************************************************
   *                       NOMBRES D'OBJETS MED                               *
   ****************************************************************************/
	MESSAGE("(****************************)");
	MESSAGE("(* INFORMATIONS GENERALES : *)");
	MESSAGE("(****************************)");

	/* calcul de la dimension */
	mdim = 2;
	double epsilon = 0.00001;
	double nodeRefX;
	double nodeRefY;
	double nodeRefZ;

	bool dimX = true;
	bool dimY = true;
	bool dimZ = true;

	int inode = 0;
	SMDS_Iterator<const SMDS_MeshNode *> * myItNodes=myMesh->nodesIterator();
	while(myItNodes->more())
	{
		const SMDS_MeshNode * node = myItNodes->next();
		if (inode == 0)
		{
			nodeRefX = fabs(node->X());
			nodeRefY = fabs(node->Y());
			nodeRefZ = fabs(node->Z());
		}
		SCRUTE(inode);
		SCRUTE(nodeRefX);
		SCRUTE(nodeRefY);
		SCRUTE(nodeRefZ);

		if (inode != 0)
		{
			if ((fabs(fabs(node->X()) - nodeRefX) > epsilon) && dimX)
				dimX = false;
			if ((fabs(fabs(node->Y()) - nodeRefY) > epsilon) && dimY)
				dimY = false;
			if ((fabs(fabs(node->Z()) - nodeRefZ) > epsilon) && dimZ)
				dimZ = false;
		}
		if (!dimX && !dimY && !dimZ)
		{
			mdim = 3;
			break;
		}
		inode++;
	}

	if (mdim != 3)
	{
		if (dimX && dimY && dimZ)
			mdim = 0;
		else if (!dimX)
		{
			if (dimY && dimZ)
				mdim = 1;
			else if ((dimY && !dimZ) || (!dimY && dimZ))
				mdim = 2;
		}
		else if (!dimY)
		{
			if (dimX && dimZ)
				mdim = 1;
			else if ((dimX && !dimZ) || (!dimX && dimZ))
				mdim = 2;
		}
		else if (!dimZ)
		{
			if (dimY && dimX)
				mdim = 1;
			else if ((dimY && !dimX) || (!dimY && dimX))
				mdim = 2;
		}
	}

	MESSAGE(" mdim " << mdim);

	/* creation du maillage */
	//mdim=3;
	sprintf(nommaa, "Mesh %d", numero);
	SCRUTE(nommaa);
	ret = MEDmaaCr(myFileId, nommaa, mdim);

	ASSERT(ret == 0);
	SCRUTE(ret);

	/* Combien de noeuds ? */
	nnoe = myMesh->NbNodes();
	//SCRUTE(nnoe);
	/* Combien de mailles, faces ou aretes ? */
	for (i = 0; i < MED_NBR_GEOMETRIE_MAILLE; i++)
		nmailles[i] = 0;

	int nb_of_nodes, nb_of_faces, nb_of_edges;
	vector < int >elem_Id[MED_NBR_GEOMETRIE_MAILLE];

	nb_of_edges = myMesh->NbEdges();
	SMDS_Iterator<const SMDS_MeshEdge *> * itEdges=myMesh->edgesIterator();
	while(itEdges->more())
	{
		const SMDS_MeshEdge * elem = itEdges->next();

		nb_of_nodes = elem->NbNodes();

		switch (nb_of_nodes)
		{
		case 2:
		{
			elem_Id[1].push_back(elem->GetID());
			nmailles[1]++;
			break;
		}
		case 3:
		{
			elem_Id[2].push_back(elem->GetID());
			nmailles[2]++;
			break;
		}
		}
	}

	nb_of_faces = myMesh->NbFaces();
	SMDS_Iterator<const SMDS_MeshFace *> * itFaces=myMesh->facesIterator();
	while(itFaces->more())
	{
		const SMDS_MeshElement * elem = itFaces->next();

		nb_of_nodes = elem->NbNodes();

		switch (nb_of_nodes)
		{
		case 3:
		{
			elem_Id[3].push_back(elem->GetID());
			nmailles[3]++;
			break;
		}
		case 4:
		{
			elem_Id[5].push_back(elem->GetID());
			nmailles[5]++;
			break;
		}
		case 6:
		{
			elem_Id[4].push_back(elem->GetID());
			nmailles[4]++;
			break;
		}
		}

	}

	SMDS_Iterator<const SMDS_MeshVolume *> * itVolumes=myMesh->volumesIterator();
	while(itVolumes->more())
	{
		const SMDS_MeshElement * elem = itVolumes->next();

		nb_of_nodes = elem->NbNodes();
		switch (nb_of_nodes)
		{
		case 8:
		{
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
   *                       ECRITURE DES NOEUDS                                *
   ****************************************************************************/
	MESSAGE("(************************)");
	MESSAGE("(* NOEUDS DU MAILLAGE : *)");
	MESSAGE("(************************)");

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

	/* PN  pour aster, il faut une famille 0 pour les noeuds et une autre pour les elements */
	/* PN : Creation de la famille 0 */
	char *nomfam = "FAMILLE_0";
	char *attdes = "";
	char *gro = 0;
	med_int ngro = 0;
	med_int natt = 1;
	med_int attide = 0;
	med_int attval = 0;
	med_int numfam = 0;
	med_int attvalabs = 1;
	ret =
		MEDfamCr(myFileId, nommaa, nomfam, numfam, &attide, &attval, attdes,
		natt, gro, ngro);
	ASSERT(ret == 0);

	/* PN : FIN Creation de la famille 0 */

	map < int, int >mapNoeud;
	typedef pair < set < int >::iterator, bool > IsFamily;
	int nbFamillesNoeud;

	i = 0;
	set < int >FamilySet;
	nbFamillesNoeud = 0;
	int verifienbnoeuds = 0;
	med_int *rien = 0;

	SMDS_Iterator<const SMDS_MeshNode *> * itNodes=myMesh->nodesIterator();
	while(itNodes->more())
	{
		const SMDS_MeshNode * node = itNodes->next();

		if (mdim == 3)
		{
			coo[i * 3] = node->X();
			coo[i * 3 + 1] = node->Y();
			coo[i * 3 + 2] = node->Z();
		}
		else if (mdim == 2)
		{
			if (dimX)
			{
				coo[i * 2] = node->Y();
				coo[i * 2 + 1] = node->Z();
			}
			if (dimY)
			{
				coo[i * 2] = node->X();
				coo[i * 2 + 1] = node->Z();
			}
			if (dimZ)
			{
				coo[i * 2] = node->X();
				coo[i * 2 + 1] = node->Y();
			}
		}
		else
		{
			if (dimX)
			{
				coo[i * 2] = node->Y();
				coo[i * 2 + 1] = node->Z();
			}
			if (dimY)
			{
				coo[i * 2] = node->X();
				coo[i * 2 + 1] = node->Z();
			}
			if (dimZ)
			{
				coo[i * 2] = node->X();
				coo[i * 2 + 1] = node->Y();
			}
		}
		mapNoeud[node->GetID()] = i + 1;

		// renvoie 0 pour les noeuds internes du volume
		int numfamille = node->GetPosition()->GetShapeId();
		nufano[i] = numfamille;

		//SCRUTE(i);
		//SCRUTE(nufano[i]);
		//SCRUTE(coo[i*3]);
		//SCRUTE(coo[i*3+1]);
		//SCRUTE(coo[i*3+2]);
		if (nufano[i] != 0)
		{
			IsFamily deja = FamilySet.insert(nufano[i]);	// insert if new, or gives existant
			if (deja.second)	// actually inserted
			{
				char famille[MED_TAILLE_NOM + 1];
				sprintf(famille, "F%d", nufano[i]);
				//          CreateFamily(strdup(nommaa),strdup(famille),nufano[i],attvalabs++);
				attvalabs++;
				CreateFamily(strdup(nommaa), strdup(famille), nufano[i],
					numfamille);
				//MESSAGE("---famille-noeud--- "<<nbFamillesNoeud<<" "<<nufano[i]);
				nbFamillesNoeud++;
			}
		}

		i++;
		verifienbnoeuds++;
	}
	ret = MEDnoeudsEcr(myFileId, nommaa, mdim, coo, mode_coo, MED_CART,
		nomcoo, unicoo, nomnoe, MED_FAUX, rien, MED_FAUX,
		nufano, nnoe, MED_REMP);
	ASSERT(ret == 0);
	MESSAGE("--- Creation de " << verifienbnoeuds << " noeuds");
	ASSERT(verifienbnoeuds == nnoe);
	MESSAGE("--- Creation de " << nbFamillesNoeud << " familles de noeuds");

	/* liberation memoire */
	free(coo);
	free(numnoe);
	free(nufano);

  /****************************************************************************
   *                       ECRITURE DES ELEMENTS                              *
   ****************************************************************************/
	MESSAGE("(**************************)");
	MESSAGE("(* ELEMENTS DU MAILLAGE : *)");
	MESSAGE("(**************************)");

	/* Ecriture des connectivites, noms, numeros des mailles */

	if (ret == 0)
	{
		int nbFamillesElts = 0;
		SMESHDS_Mesh * mySMESHDSMesh = dynamic_cast<SMESHDS_Mesh *>(myMesh);
		map<int,int> mapFamille;

		if(!mySMESHDSMesh->ShapeToMesh().IsNull())
		{
			TopTools_IndexedMapOfShape myIndexToShape;
			TopExp::MapShapes(mySMESHDSMesh->ShapeToMesh(), myIndexToShape);


			if (besoinfamilledemaille == 1)
			{
				int t;
				for (t = 1; t <= myIndexToShape.Extent(); t++)
				{
					const TopoDS_Shape S = myIndexToShape(t);
					if (mySMESHDSMesh->HasMeshElements(S))
					{
						//MESSAGE ("********* Traitement de la Famille "<<-t);
	
						SMESHDS_SubMesh * SM = mySMESHDSMesh->MeshElements(S);
						SMDS_Iterator<const SMDS_MeshElement*> * ite=SM->GetElements();
						bool plein = false;
						while(ite->more())
						{
							mapFamille[ite->next()->GetID()] = -t;
							plein = true;
						}
						if (plein)
						{
							nbFamillesElts++;
							char famille[MED_TAILLE_NOM + 1];
							sprintf(famille, "E%d", t);
							CreateFamily(strdup(nommaa), strdup(famille), -t,
								attvalabs++);
						}
					}
				}
			}
		}
		else besoinfamilledemaille = 0;

		int indice = 1;
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
				//SCRUTE(nsup);

				taille = nsup + typmai[i] % 100;
				//SCRUTE(taille);

				/* allocation memoire */
				connectivite =
					(med_int *) malloc(sizeof(med_int) * taille * nmailles[i]);
				/* nomele =
					(char *)malloc(sizeof(char) * MED_TAILLE_PNOM *
					nmailles[i] + 1);*/
				nomele = "";
				numele = (med_int *) malloc(sizeof(med_int) * nmailles[i]);
				nufael = (med_int *) malloc(sizeof(med_int) * nmailles[i]);
				nbNodes = typmai[i] % 100;

				for (j = 0; j < nmailles[i]; j++)
				{
					myId = elem_Id[i][j];
					const SMDS_MeshElement * elem =
						myMesh->FindElement(myId);
					//*(numele+j) = myId;
					*(numele + j) = indice++;

					SMDS_Iterator<const SMDS_MeshElement *> * itk=elem->nodesIterator();
					for (k = 0; itk->more(); k++)
					{
						*(connectivite + j * taille + k) =
							mapNoeud[itk->next()->GetID()];
					}
					delete itk;

					if (nsup)
						*(connectivite + j * taille + nbNodes) = 0;

					if (besoinfamilledemaille == 1)
					{
						if (mapFamille.find(myId) != mapFamille.end())
						{
							nufael[j] = mapFamille[myId];
						}
						else
						{
							nufael[j] = 0;
						}
					}
					else
					{
						nufael[j] = 0;
					}

					//SCRUTE(myId);
					//SCRUTE(j);
					//SCRUTE(nufael[j]);
				}

				/* ecriture des données */

				med_int *rien = 0;
				ret =
					MEDelementsEcr(myFileId, nommaa, mdim, connectivite,
					mode_coo, nomele, MED_FAUX, numele, MED_VRAI, nufael,
					nmailles[i], MED_MAILLE, typmai[i], typ_con, MED_REMP);
				ASSERT(ret == 0);
				//SCRUTE(ret);

				if (ret < 0)
					MESSAGE(">> ERREUR : ecriture des mailles \n");

				/* liberation memoire */
				free(connectivite);
				free(numele);
				free(nufael);
				MESSAGE(" End " << typmai[i]);
			}
		};
		MESSAGE("--- Creation de " << nbFamillesElts << " familles d elements");

	}

	  /****************************************************************************
      *                      FERMETURE DU FICHIER                                *
      ****************************************************************************/

	ret = MEDfermer(myFileId);

	if (ret != 0)
		fprintf(stderr, ">> ERREUR : erreur a la fermeture du fichier %s\n",
			file2Read);
	MESSAGE("fichier ferme");

}

void DriverMED_W_SMESHDS_Mesh::CreateFamily(char *nommaa, char *famille, int i,
	med_int k)
{

	med_int ngro = 0;
	med_int natt;

	natt = 1;
	char attdes[MED_TAILLE_DESC + 1];
	char gro[MED_TAILLE_LNOM + 1];
	char fam2[MED_TAILLE_LNOM + 1];

	strcpy(attdes, "");
	strcpy(gro, "");
	strcpy(fam2, famille);

	med_int *attide = new med_int[1];
	med_int *attval = new med_int[1];
	attide[0] = k;
	attval[0] = k;

	//MESSAGE("-------- Creation de la Famille : "<< famille << "numero " << i << " --------------");
	med_int ret =
		MEDfamCr(myFileId, nommaa, fam2, i, attide, attval, attdes, natt, gro,
		ngro);
	ASSERT(ret == 0);
	delete[]attide;
	delete[]attval;

}
