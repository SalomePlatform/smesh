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
//  File   : DriverMED_R_SMESHDS_Mesh.cxx
//  Module : SMESH

using namespace std;
#include "DriverMED_R_SMESHDS_Mesh.h"
#include "DriverMED_R_SMDS_Mesh.h"
#include "utilities.h"

#include <stdlib.h>

extern "C"
{
/**
 * Factory function which will be called by SMESHDriver
 */
void * SMESH_createMEDMeshReader()
{
	return new DriverMED_R_SMESHDS_Mesh();
}

}

DriverMED_R_SMESHDS_Mesh::DriverMED_R_SMESHDS_Mesh()
{
	myFileId = -1;
}

DriverMED_R_SMESHDS_Mesh::~DriverMED_R_SMESHDS_Mesh()
{
	;
}

void DriverMED_R_SMESHDS_Mesh::SetMesh(SMDS_Mesh * aMesh)
{
	//myMesh = SMESHDS_Mesh *::DownCast(aMesh);
	myMesh = aMesh;
}

void DriverMED_R_SMESHDS_Mesh::SetFile(string aFile)
{
	myFile = aFile;
}

void DriverMED_R_SMESHDS_Mesh::SetFileId(med_idt aFileId)
{
	myFileId = aFileId;
}

void DriverMED_R_SMESHDS_Mesh::SetMeshId(int aMeshId)
{
	myMeshId = aMeshId;
}

void DriverMED_R_SMESHDS_Mesh::Read()
{

	string myClass = string("SMDS_Mesh");
	string myExtension = string("MED");

	DriverMED_R_SMDS_Mesh *myReader = new DriverMED_R_SMDS_Mesh;

	myReader->SetMesh(myMesh);
	myReader->SetMeshId(myMeshId);
	myReader->SetFile(myFile);
	myReader->SetFileId(-1);

	myReader->Read();

}

void DriverMED_R_SMESHDS_Mesh::Add()
{

	string myClass = string("SMDS_Mesh");
	string myExtension = string("MED");

	DriverMED_R_SMDS_Mesh *myReader = new DriverMED_R_SMDS_Mesh;

	myReader->SetMesh(myMesh);
	myReader->SetMeshId(myMeshId);

	SCRUTE(myFileId);
	myReader->SetFileId(myFileId);

	myReader->Read();

}

void DriverMED_R_SMESHDS_Mesh::ReadMySelf()
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
	/* elements */
	med_int nsup;
	med_int edim;
	med_int taille;
	med_int elem_id;
	med_int cmpt = 0;
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
	string fam;
	string fam_type;
	string fam_id;

	char *file2Read;
	bool locally_managed;

	if (myFileId == -1)
		locally_managed = true;
	else
		locally_managed = false;

	if (locally_managed)
	{
		file2Read = (char *)myFile.c_str();
		myFileId = MEDouvrir(file2Read, MED_LECT);
		if (myFileId < 0)
		{
			fprintf(stderr, ">> ERREUR : ouverture du fichier %s \n",
				file2Read);
			exit(EXIT_FAILURE);
		}
		numero = 1;
	}
	else
		numero = myMeshId;
	sprintf(nommaa, "Mesh %d", myMeshId);	//pour load
	SCRUTE(nommaa);

	typ_con = MED_NOD;
	mode_coo = MED_FULL_INTERLACE;
	mdim = 3;

	SMESHDS_Mesh * mySMESHDSMesh = dynamic_cast<SMESHDS_Mesh *>(myMesh);

	//TopoDS_Shape myShape = mySMESHDSMesh->ShapeToMesh();

  /****************************************************************************
  *                       NOMBRES D'OBJETS MED                                *
  ****************************************************************************/
	fprintf(stdout, "\n(****************************)\n");
	fprintf(stdout, "(* INFORMATIONS GENERALES : *)\n");
	fprintf(stdout, "(****************************)\n");

	/* lecture du nom et de la dimension du maillage */
	/*!  fprintf(stdout,"%d %d\n",myFileId,numero);
	 * ret = MEDmaaInfo(myFileId,numero,nommaa,&mdim);
	 * fprintf(stdout,"%d\n",ret);
	 * if (ret < 0)
	 * {
	 * fprintf(stderr,">> ERREUR : lecture du nom du maillage \n");
	 * exit(EXIT_FAILURE);
	 * }
	 * fprintf(stdout,"- Nom du maillage : <<%s>>\n",nommaa);
	 * fprintf(stdout,"- Dimension du maillage : %d\n",mdim);
	 */
	/* Combien de noeuds ? */
	nnoe =
		MEDnEntMaa(myFileId, nommaa, MED_COOR, MED_NOEUD, MED_POINT1, typ_con);
	if (nnoe < 0)
	{
		fprintf(stderr, ">> ERREUR : lecture du nombre de noeuds \n");
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "- Nombre de noeuds : %d \n", nnoe);

	/* Combien de mailles, faces ou aretes ? */
	for (i = 0; i < MED_NBR_GEOMETRIE_MAILLE; i++)
	{
		nmailles[i] =
			MEDnEntMaa(myFileId, nommaa, MED_CONN, MED_MAILLE, typmai[i],
			typ_con);
		if (nmailles[i] < 0)
		{
			fprintf(stderr, ">> ERREUR : lecture du nombre de mailles \n");
			exit(EXIT_FAILURE);
		}
		fprintf(stdout, "- Nombre de mailles de type %s : %d \n", nommai[i],
			nmailles[i]);
	}

	for (i = 0; i < MED_NBR_GEOMETRIE_FACE; i++)
	{
		nfaces[i] = MEDnEntMaa(myFileId, nommaa, MED_CONN, MED_FACE, typfac[i],
			typ_con);
		if (nfaces[i] < 0)
		{
			fprintf(stderr, ">> ERREUR : lecture du nombre de faces \n");
			exit(EXIT_FAILURE);
		}
		fprintf(stdout, "- Nombre de faces de type %s : %d \n", nomfac[i],
			nfaces[i]);
	}

	for (i = 0; i < MED_NBR_GEOMETRIE_ARETE; i++)
	{
		naretes[i] =
			MEDnEntMaa(myFileId, nommaa, MED_CONN, MED_ARETE, typare[i],
			typ_con);
		if (naretes[i] < 0)
		{
			fprintf(stderr, ">> ERREUR : lecture du nombre d'aretes \n");
			exit(EXIT_FAILURE);
		}
		fprintf(stdout, "- Nombre d'aretes de type %s : %d \n", nomare[i],
			naretes[i]);
	}

	/* nombre de familles */
	nfam = MEDnFam(myFileId, nommaa, 0, MED_FAMILLE);
	if (nfam < 0)
	{
		fprintf(stderr, ">> ERREUR : lecture du nombre de familles \n");
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "- Nombre de familles : %d \n", nfam);

	vector < int >family[nfam];

  /****************************************************************************
  *                       LECTURE DES NOEUDS                                  *
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
	nomnoe = (char *)malloc(MED_TAILLE_PNOM * nnoe + 1);

	/* lecture des noeuds : 
	 * - coordonnees
	 * - noms (optionnel dans un fichier MED) 
	 * - numeros (optionnel dans un fichier MED) 
	 * - numeros des familles */
	ret = MEDnoeudsLire(myFileId, nommaa, mdim, coo, mode_coo, &rep,
		nomcoo, unicoo, nomnoe, &inonoe, numnoe, &inunoe, nufano, nnoe);
	if (ret < 0)
		strcpy(message, ">> ERREUR : lecture des noeuds \n");

	if (inunoe)
	{
		for (int i = 0; i < nnoe; i++)
		{
			ok = mySMESHDSMesh->AddNodeWithID(coo[i * 3], coo[i * 3 + 1],
				coo[i * 3 + 2], numnoe[i]);
			//fprintf(Out,"%d %f %f %f\n",numnoe[i],coo[i*3],coo[i*3+1],coo[i*3+2]);
		}
	}
	else
	{
		for (int i = 0; i < nnoe; i++)
		{
			ok = mySMESHDSMesh->AddNodeWithID(coo[i * 3], coo[i * 3 + 1],
				coo[i * 3 + 2], i + 1);
			//fprintf(Out,"%d %f %f %f\n",numnoe[i],coo[i*3],coo[i*3+1],i);
			family[*(nufano + i)].push_back(numnoe[i]);
		}
	}

	fprintf(stdout, "\n- Numeros des familles des noeuds : \n");
	for (i = 0; i < nnoe; i++)
		fprintf(stdout, " %d ", *(nufano + i));
	fprintf(stdout, "\n");

	/* liberation memoire */
	free(coo);
	free(nomnoe);
	free(numnoe);
	free(nufano);

  /****************************************************************************
  *                       LECTURE DES ELEMENTS                                *
  ****************************************************************************/
	fprintf(stdout, "\n(**************************)\n");
	fprintf(stdout, "(* ELEMENTS DU MAILLAGE : *)\n");
	fprintf(stdout, "(**************************)");
	//fprintf(Out,"CELLS\n");
	/* Lecture des connectivites, noms, numeros des mailles */
	//printf("%d %d %d %d\n",nmailles[3],nmailles[4],nmailles[5],nmailles[9]);

	if (ret == 0)
		for (i = 0; i < MED_NBR_GEOMETRIE_MAILLE; i++)
		{
			if (nmailles[i] > 0 && ret == 0)
			{
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

				/* lecture des données */
				ret =
					MEDelementsLire(myFileId, nommaa, mdim, connectivite,
					mode_coo, nomele, &inoele, numele, &inuele, nufael,
					nmailles[i], MED_MAILLE, typmai[i], typ_con);

				switch (typmai[i])
				{
				case MED_TRIA3:
				{
					if (inuele)
					{
						for (j = 0; j < nmailles[i]; j++)
						{
							elem_id = *(numele + j);
							ok = mySMESHDSMesh->AddFaceWithID(*(connectivite +
									j * (taille - nsup)),
								*(connectivite + j * (taille - nsup) + 1),
								*(connectivite + j * (taille - nsup) + 2),
								elem_id);
							//fprintf(Out,"%d %d %d %d\n",elem_id,*(connectivite+j*(taille-nsup)),*(connectivite+j*(taille-nsup)+1),*(connectivite+j*(taille-nsup)+2));
						}
					}
					else
					{
						for (j = 0; j < nmailles[i]; j++)
						{
							cmpt++;
							ok = mySMESHDSMesh->AddFaceWithID(*(connectivite +
									j * (taille)),
								*(connectivite + j * (taille) + 1),
								*(connectivite + j * (taille) + 2), cmpt);
							//fprintf(Out,"%d %d %d %d\n",j,*(connectivite+j*(taille)),*(connectivite+j*(taille)+1),*(connectivite+j*(taille)+2));
						}
					}

					break;
				}
				case MED_QUAD4:
				{
					if (inuele)
					{
						for (j = 0; j < nmailles[i]; j++)
						{
							elem_id = *(numele + j);
							ok = mySMESHDSMesh->AddFaceWithID(*(connectivite +
									j * (taille - nsup)),
								*(connectivite + j * (taille - nsup) + 1),
								*(connectivite + j * (taille - nsup) + 2),
								*(connectivite + j * (taille - nsup) + 3),
								elem_id);
							//fprintf(Out,"%d %d %d %d\n",elem_id,*(connectivite+j*(taille-nsup)),*(connectivite+j*(taille-nsup)+1),*(connectivite+j*(taille-nsup)+2),*(connectivite+j*(taille-nsup)+3));
						}
					}
					else
					{
						for (j = 0; j < nmailles[i]; j++)
						{
							cmpt++;
							ok = myMesh->AddFaceWithID(*(connectivite +
									j * (taille)),
								*(connectivite + j * (taille) + 1),
								*(connectivite + j * (taille) + 2),
								*(connectivite + j * (taille) + 3), cmpt);
							//fprintf(Out,"%d %d %d %d\n",j,*(connectivite+j*(taille)),*(connectivite+j*(taille)+1),*(connectivite+j*(taille)+2),*(connectivite+j*(taille)+3));
						}
					}
					break;
				}
				case MED_TETRA4:
				{
					if (inuele)
					{
						for (j = 0; j < nmailles[i]; j++)
						{
							elem_id = *(numele + j);
							ok = mySMESHDSMesh->AddVolumeWithID(*(connectivite +
									j * (taille - nsup)),
								*(connectivite + j * (taille - nsup) + 1),
								*(connectivite + j * (taille - nsup) + 2),
								*(connectivite + j * (taille - nsup) + 3),
								elem_id);
							//fprintf(Out,"%d %d %d %d\n",elem_id,*(connectivite+j*(taille-nsup)),*(connectivite+j*(taille-nsup)+1),*(connectivite+j*(taille-nsup)+2),*(connectivite+j*(taille-nsup)+3));
						}
					}
					else
					{
						for (j = 0; j < nmailles[i]; j++)
						{
							cmpt++;
							ok = mySMESHDSMesh->AddVolumeWithID(*(connectivite +
									j * (taille)),
								*(connectivite + j * (taille) + 1),
								*(connectivite + j * (taille) + 2),
								*(connectivite + j * (taille) + 3), cmpt);
							//fprintf(Out,"%d %d %d %d\n",j,*(connectivite+j*(taille)),*(connectivite+j*(taille)+1),*(connectivite+j*(taille)+2),*(connectivite+j*(taille)+3));
						}
					}
					break;
				}
				case MED_HEXA8:
				{
					if (inuele)
					{
						for (j = 0; j < nmailles[i]; j++)
						{
							elem_id = *(numele + j);
							ok = mySMESHDSMesh->AddVolumeWithID(*(connectivite +
									j * (taille - nsup)),
								*(connectivite + j * (taille - nsup) + 1),
								*(connectivite + j * (taille - nsup) + 2),
								*(connectivite + j * (taille - nsup) + 3),
								*(connectivite + j * (taille - nsup) + 4),
								*(connectivite + j * (taille - nsup) + 5),
								*(connectivite + j * (taille - nsup) + 6),
								*(connectivite + j * (taille - nsup) + 7),
								elem_id);
							//fprintf(Out,"%d %d %d %d\n",elem_id,*(connectivite+j*(taille-nsup)),*(connectivite+j*(taille-nsup)+1),*(connectivite+j*(taille-nsup)+2),*(connectivite+j*(taille-nsup)+3),*(connectivite+j*(taille-nsup)+4),*(connectivite+j*(taille-nsup)+5),*(connectivite+j*(taille-nsup)+6),*(connectivite+j*(taille-nsup)+7));
						}
					}
					else
					{
						for (j = 0; j < nmailles[i]; j++)
						{
							cmpt++;
							ok = mySMESHDSMesh->AddVolumeWithID(*(connectivite +
									j * (taille)),
								*(connectivite + j * (taille) + 1),
								*(connectivite + j * (taille) + 2),
								*(connectivite + j * (taille) + 3),
								*(connectivite + j * (taille) + 4),
								*(connectivite + j * (taille) + 5),
								*(connectivite + j * (taille) + 6),
								*(connectivite + j * (taille) + 7), cmpt);
							//fprintf(Out,"%d %d %d %d\n",j,*(connectivite+j*(taille)),*(connectivite+j*(taille)+1),*(connectivite+j*(taille)+2),*(connectivite+j*(taille)+3),*(connectivite+j*(taille)+4),*(connectivite+j*(taille)+5),*(connectivite+j*(taille)+6),*(connectivite+j*(taille)+7));
						}
					}
					break;
				}
				default:
				{
					break;
				}
				}

				fprintf(stdout, "\n  - Numéros de familles : \n");
				for (j = 0; j < nmailles[i]; j++)
					fprintf(stdout, " %d ", *(nufael + j));

				/* liberation memoire */
				free(connectivite);
				free(nomele);
				free(numele);
				free(nufael);
			}
		}

  /****************************************************************************
   *                       LECTURE DES FAMILLES                                *
   ****************************************************************************/
	printf("\n(*************************)\n");
	printf("(* FAMILLES DU MAILLAGE : *)\n");
	printf("(*************************)\n");
	if (ret == 0)
		for (i = 0; i < nfam; i++)
		{

			/* nombre de groupes */
			ngro = MEDnFam(myFileId, nommaa, i + 1, MED_GROUPE);
			if (ngro < 0)
			{
				ret = -1;
				strcpy(message,
					">> ERREUR : lecture du nombre de groupes d'une famille \n");
			}

			/* nombre d'attributs */
			if (ret == 0)
			{
				natt = MEDnFam(myFileId, nommaa, i + 1, MED_ATTR);
				if (natt < 0)
				{
					ret = -1;
					strcpy(message,
						">> ERREUR : lecture du nombre d'attributs d'une famille\n");
				}
			}

			if (ret == 0)
				fprintf(stdout, "- Famille %d a %d attributs et %d groupes \n",
					i + 1, natt, ngro);

			/* nom,numero,attributs,groupes */
			if (ret == 0)
			{
				attide = (med_int *) malloc(sizeof(med_int) * natt);
				attval = (med_int *) malloc(sizeof(med_int) * natt);
				attdes = (char *)malloc(MED_TAILLE_DESC * natt + 1);
				gro = (char *)malloc(MED_TAILLE_LNOM * ngro + 1);
				ret =
					MEDfamInfo(myFileId, nommaa, i + 1, nomfam, &numfam, attide,
					attval, attdes, &natt, gro, &ngro);

				fam = string(nomfam);
				fam_type = fam.substr(1, 1);
				fam_id = fam.substr(2, 1);
				if ((fam_type == string("V")) || (fam_type == string("A")) ||
					(fam_type == string("F")))
					LinkMeshToShape(fam_type, fam_id, family[i]);

				fprintf(stdout, "  - Famille de nom %s et de numero %d : \n",
					nomfam, numfam);
				fprintf(stdout, "  - Attributs : \n");
				for (j = 0; j < natt; j++)
				{
					strncpy(str1, attdes + j * MED_TAILLE_DESC,
						MED_TAILLE_DESC);
					str1[MED_TAILLE_DESC] = '\0';
					fprintf(stdout, "   ide = %d - val = %d - des = %s\n",
						*(attide + j), *(attval + j), str1);
				}
				free(attide);
				free(attval);
				free(attdes);
				fprintf(stdout, "  - Groupes :\n");
				for (j = 0; j < ngro; j++)
				{
					strncpy(str2, gro + j * MED_TAILLE_LNOM, MED_TAILLE_LNOM);
					str2[MED_TAILLE_LNOM] = '\0';
					fprintf(stdout, "   gro = %s\n", str2);
				}
				free(gro);
			}
		}

	if (locally_managed)
		ret = MEDfermer(myFileId);

}

void DriverMED_R_SMESHDS_Mesh::LinkMeshToShape(string fam_type, string fam_id,
	vector < int >myNodes)
{

	SMESHDS_Mesh * mySMESHDSMesh = dynamic_cast<SMESHDS_Mesh *>(myMesh);

	int id = atoi(fam_id.c_str());
	if (fam_type == string("V"))
	{							//Linked to a vertex
		for (int i = 0; i < myNodes.size(); i++)
		{
			const SMDS_MeshNode * node = mySMESHDSMesh->FindNode(myNodes[i]);
			//const TopoDS_Vertex& S;//le recuperer !!!
			//mySMESHDSMesh->SetNodeOnVertex (node,S);
		}
	}
	else if (fam_type == string("E"))
	{							//Linked to an edge
		for (int i = 0; i < myNodes.size(); i++)
		{
			const SMDS_MeshNode * node = mySMESHDSMesh->FindNode(myNodes[i]);
			//const TopoDS_Edge& S;//le recuperer !!!
			//mySMESHDSMesh->SetNodeOnEdge (node,S);
		}
	}
	else if (fam_type == string("F"))
	{							//Linked to a face
		for (int i = 0; i < myNodes.size(); i++)
		{
			const SMDS_MeshNode * node = mySMESHDSMesh->FindNode(myNodes[i]);
			//const TopoDS_Face& S;//le recuperer !!!
			//mySMESHDSMesh->SetNodeOnFace (node,S);
		}
	}

}
