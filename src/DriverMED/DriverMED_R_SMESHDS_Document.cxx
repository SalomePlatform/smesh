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
//  File   : DriverMED_R_SMESHDS_Document.cxx
//  Module : SMESH

using namespace std;
#include "DriverMED_R_SMESHDS_Document.h"
#include "DriverMED_R_SMESHDS_Mesh.h"
#include "utilities.h"

extern "C"
{
	void * SMESH_createMEDDocumentReader()
	{
		return new DriverMED_R_SMESHDS_Document;
	}
}

DriverMED_R_SMESHDS_Document::DriverMED_R_SMESHDS_Document()
	:Document_Reader(new DriverMED_R_SMESHDS_Mesh())
{
	;
}

DriverMED_R_SMESHDS_Document::~DriverMED_R_SMESHDS_Document()
{
	;
}

//void DriverMED_R_SMESHDS_Document::SetFile(string aFile) {
//  myFile = aFile;
//}

void DriverMED_R_SMESHDS_Document::Read()
{

	med_err ret = 0;
	med_idt fid;
	med_int nmaa;

	int myMeshId;

/****************************************************************************
  *                      OUVERTURE DU FICHIER EN LECTURE                      *
  ****************************************************************************/
	char *file2Read = (char *)myFile.c_str();
	fid = MEDouvrir(file2Read, MED_LECT);
	if (fid < 0)
	{
		fprintf(stderr, ">> ERREUR : ouverture du fichier %s \n", file2Read);
		exit(EXIT_FAILURE);
	}

  /****************************************************************************
  *                      COMBIEN DE MAILLAGES ?                               *
  ****************************************************************************/
	nmaa = MEDnMaa(fid);
	if (nmaa < 0)
	{
		fprintf(stderr, ">> ERREUR : lecture du nombre de maillages \n");
		exit(EXIT_FAILURE);
	}
	printf("Nombre de maillages = %d\n", nmaa);

	string myClass = string("SMESHDS_Mesh");
	string myExtension = string("MED");

	for (int meshIt = 1; meshIt <= nmaa; meshIt++)
	{
		myMeshId = myDocument->NewMesh();
		printf("MeshId = %d\n", myMeshId);

		SMDS_Mesh * myMesh = myDocument->GetMesh(myMeshId);
		SCRUTE(myMesh);

		// Handle (SMDS_Mesh) MMM =.........
		//ex SMESHDS_Mesh * myMesh2 =SMESHDS_Mesh *::DownCast(MMM);

		DriverMED_R_SMESHDS_Mesh *myReader = new DriverMED_R_SMESHDS_Mesh;

		myReader->SetMesh(myMesh);
		myReader->SetMeshId(myMeshId);

		//myReader->SetFile(myFile);
		SCRUTE(fid);
		myReader->SetFileId(fid);

		myReader->Add();

	}

  /****************************************************************************
  *                      FERMETURE DU FICHIER                                 *
  ****************************************************************************/
	ret = MEDfermer(fid);

	if (ret != 0)
		fprintf(stderr, ">> ERREUR : erreur a la fermeture du fichier %s\n",
			file2Read);

}
