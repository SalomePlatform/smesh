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
//
//
//
//  File   : Document_Reader.cxx
//  Module : SMESH

using namespace std;
#include "Document_Reader.h"

#include "utilities.h"

void Document_Reader::SetFile(string aFile)
{
	myFile = aFile;
}

void Document_Reader::SetDocument(SMESHDS_Document * aDoc)
{
	myDocument = aDoc;
}

void Document_Reader::Read()
{
	int myMeshId = myDocument->NewMesh();
	SMDS_Mesh * myMesh = myDocument->GetMesh(myMeshId);
	myReader->SetMesh(myMesh);
	myReader->SetFile(myFile);
	myReader->Read();
}

Document_Reader::Document_Reader(Mesh_Reader* reader): myReader(reader)
{
}
