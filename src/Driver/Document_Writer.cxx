//  SMESH Driver : implementaion of driver for reading and writing  
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

#include "Document_Writer.h"
#include "utilities.h"

void Document_Writer::SetFile(string aFile)
{
	myFile = aFile;
}

void Document_Writer::SetDocument(SMESHDS_Document * aDoc)
{
	myDocument = aDoc;
}

void Document_Writer::Write()
{
	SCRUTE(myFile);
	SMESHDS_Mesh * myMesh;
	int nb_of_meshes = myDocument->NbMeshes();	//voir avec Yves
	SCRUTE(nb_of_meshes);

	int numero = 0;

	myDocument->InitMeshesIterator();
	while(myDocument->MoreMesh())
	{
		numero++;
		myMesh = myDocument->NextMesh();
		myWriter->SetMesh(myMesh);
		myWriter->SetFile(myFile);
		myWriter->SetMeshId(numero);
		myWriter->Add();
	}
}

Document_Writer::Document_Writer(Mesh_Writer* writer): myWriter(writer)
{
}
