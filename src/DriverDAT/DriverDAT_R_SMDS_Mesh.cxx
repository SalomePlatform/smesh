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
//  File   : DriverDAT_R_SMDS_Mesh.cxx
//  Module : SMESH

#include "DriverDAT_R_SMDS_Mesh.h"
#include "utilities.h"

extern "C"
{

/**
 * Factory function which will be called by SMESHDriver
 */
void * SMESH_createDATMeshReader()
{
	return new DriverDAT_R_SMDS_Mesh();
}
}

DriverDAT_R_SMDS_Mesh::DriverDAT_R_SMDS_Mesh()
{
	;
}

DriverDAT_R_SMDS_Mesh::~DriverDAT_R_SMDS_Mesh()
{
	;
}

void DriverDAT_R_SMDS_Mesh::SetMesh(SMDS_Mesh * aMesh)
{
	myMesh = aMesh;
}

void DriverDAT_R_SMDS_Mesh::SetFile(string aFile)
{
	myFile = aFile;
}

void DriverDAT_R_SMDS_Mesh::SetFileId(FILE * aFileId)
{
	myFileId = aFileId;
}

void DriverDAT_R_SMDS_Mesh::SetMeshId(int aMeshId)
{
	myMeshId = aMeshId;
}

void DriverDAT_R_SMDS_Mesh::Add()
{
	;
}

void DriverDAT_R_SMDS_Mesh::Read()
{

	int i, j;
	int nbNodes, nbCells;
	int intNumPoint;
	float coordX, coordY, coordZ;
	int nbNoeuds;

	int intNumMaille, Degre;
	int ValElement;
	int ValDegre;
	int NoeudsMaille[20];
	int NoeudMaille;

	bool ok;

	MESSAGE("in DriverDAT_R_SMDS_Mesh::Read()");
  /****************************************************************************
  *                      OUVERTURE DU FICHIER EN LECTURE                      *
  ****************************************************************************/
	char *file2Read = (char *)myFile.c_str();
	myFileId = fopen(file2Read, "r");
	if (myFileId < 0)
	{
		fprintf(stderr, ">> ERREUR : ouverture du fichier %s \n", file2Read);
		exit(EXIT_FAILURE);
	}

	fscanf(myFileId, "%d %d\n", &nbNodes, &nbCells);

  /****************************************************************************
  *                       LECTURE DES NOEUDS                                  *
  ****************************************************************************/
	fprintf(stdout, "\n(************************)\n");
	fprintf(stdout, "(* NOEUDS DU MAILLAGE : *)\n");
	fprintf(stdout, "(************************)\n");

	for (i = 0; i < nbNodes; i++)
	{
		fscanf(myFileId, "%d %e %e %e\n", &intNumPoint, &coordX, &coordY,
			&coordZ);
		ok = myMesh->AddNodeWithID(coordX, coordY, coordZ, intNumPoint);
	}

	fprintf(stdout, "%d noeuds\n", myMesh->NbNodes());
  /****************************************************************************
  *                       LECTURE DES ELEMENTS                                *
  ****************************************************************************/
	fprintf(stdout, "\n(**************************)\n");
	fprintf(stdout, "(* ELEMENTS DU MAILLAGE : *)\n");
	fprintf(stdout, "(**************************)");

	fprintf(stdout, "%d elements\n", nbCells);

	for (i = 0; i < nbCells; i++)
	{
		fscanf(myFileId, "%d %d", &intNumMaille, &ValElement);
		Degre = abs(ValElement / 100);
		nbNoeuds = ValElement - (Degre * 100);

		// Recuperation des noeuds de la maille
		for (j = 0; j < nbNoeuds; j++)
		{
			fscanf(myFileId, "%d", &NoeudMaille);
			NoeudsMaille[j] = NoeudMaille;
		}

		// Analyse des cas de cellules
		switch (ValElement)
		{
		case 102:;
		case 103:;
			{
				ValDegre = 3;
				nbNoeuds = 2;
				ok = myMesh->AddEdgeWithID(NoeudsMaille[0], NoeudsMaille[1],
					intNumMaille);
				break;
			}
		case 204:;
		case 208:;
			{
				ValDegre = 9;
				nbNoeuds = 4;
				ok = myMesh->AddFaceWithID(NoeudsMaille[0], NoeudsMaille[1],
					NoeudsMaille[2], NoeudsMaille[3], intNumMaille);
				break;
			}
		case 203:;
		case 206:;
			{
				ValDegre = 5;
				nbNoeuds = 3;
				ok = myMesh->AddFaceWithID(NoeudsMaille[0], NoeudsMaille[1],
					NoeudsMaille[2], intNumMaille);
				break;
			}
		case 308:;
		case 320:;
			{
				ValDegre = 12;
				nbNoeuds = 8;
				if (ValElement == 320)
				{
					//A voir, correspondance VTK
					NoeudsMaille[4] = NoeudsMaille[8];
					NoeudsMaille[5] = NoeudsMaille[9];
					NoeudsMaille[6] = NoeudsMaille[10];
					NoeudsMaille[7] = NoeudsMaille[11];
				}
				ok = myMesh->AddVolumeWithID(NoeudsMaille[0], NoeudsMaille[1],
					NoeudsMaille[2], NoeudsMaille[3], NoeudsMaille[4],
					NoeudsMaille[5], NoeudsMaille[6], NoeudsMaille[7],
					intNumMaille);
				break;
			}
		case 304:;
		case 310:;
			{
				ValDegre = 10;
				nbNoeuds = 4;
				if (ValElement == 310)
					NoeudsMaille[3] = NoeudsMaille[6];
				ok = myMesh->AddVolumeWithID(NoeudsMaille[0], NoeudsMaille[1],
					NoeudsMaille[2], NoeudsMaille[3], intNumMaille);
				break;
			}
		case 306:;
		case 315:;
			{
				ValDegre = 12;
				nbNoeuds = 8;
				if (ValElement == 315)
				{
					NoeudsMaille[3] = NoeudsMaille[6];
					NoeudsMaille[4] = NoeudsMaille[7];
					NoeudsMaille[5] = NoeudsMaille[8];
				}
				NoeudsMaille[7] = NoeudsMaille[5];
				NoeudsMaille[6] = NoeudsMaille[5];
				NoeudsMaille[5] = NoeudsMaille[4];
				NoeudsMaille[4] = NoeudsMaille[3];
				NoeudsMaille[3] = NoeudsMaille[2];
				ok = myMesh->AddVolumeWithID(NoeudsMaille[0], NoeudsMaille[1],
					NoeudsMaille[2], NoeudsMaille[3], NoeudsMaille[4],
					NoeudsMaille[5], intNumMaille);
				break;
			}

		}
	}

  /****************************************************************************
  *                      FERMETURE DU FICHIER                      *
  ****************************************************************************/
	fclose(myFileId);

}
