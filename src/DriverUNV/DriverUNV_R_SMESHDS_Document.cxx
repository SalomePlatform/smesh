//  SMESH DriverUNV : driver to read and write 'unv' files
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
//  File   : DriverUNV_R_SMESHDS_Document.cxx
//  Module : SMESH

using namespace std;
#include "DriverUNV_R_SMESHDS_Document.h"
#include "DriverUNV_R_SMESHDS_Mesh.h"

#include "utilities.h"

int getOne() {
  printf("in getOne");
  return (1);
}

extern "C" {
  //  Document_Reader* maker() {
  DriverUNV_R_SMESHDS_Document* maker() {
    fprintf(stdout,"here in maker\n");
    return new DriverUNV_R_SMESHDS_Document;
  }
}

DriverUNV_R_SMESHDS_Document::DriverUNV_R_SMESHDS_Document() {
  myFile = string("");
}

DriverUNV_R_SMESHDS_Document::~DriverUNV_R_SMESHDS_Document() {
;
}

//void DriverUNV_R_SMESHDS_Document::SetFile(string aFile) {
//myFile = aFile;
//}

//void DriverUNV_R_SMESHDS_Document::SetDocument(Handle(SMESHDS_Document)& aDoc) {
//myDocument = aDoc;
//}

void DriverUNV_R_SMESHDS_Document::Read() {

  int myMeshId;
  SCRUTE(myFile);
  //Handle(SMESHDS_Document) myDocument = new SMESHDS_Document(1);

  /****************************************************************************
  *                      OUVERTURE DU FICHIER EN LECTURE                      *
  ****************************************************************************/
  char* file2Read = (char*)myFile.c_str();
  FILE* fid = fopen(file2Read,"r");
  if (fid < 0)
    {
      fprintf(stderr,">> ERREUR : ouverture du fichier %s \n",file2Read);
      exit(EXIT_FAILURE);
    }

  /****************************************************************************
  *                      COMBIEN DE MAILLAGES ?                               *
  ****************************************************************************/
  int nmaa = 1;

  /****************************************************************************
  *                      FERMETURE DU FICHIER                                 *
  ****************************************************************************/
  fclose(fid);
  
  printf("Nombre de maillages = %d\n",nmaa);

  string myClass = string("SMESHDS_Mesh");
  string myExtension = string("UNV");

  for (int meshIt=1;meshIt<=nmaa;meshIt++) {
    myMeshId = myDocument->NewMesh();

    Handle(SMDS_Mesh) myMesh = myDocument->GetMesh(myMeshId);

    DriverUNV_R_SMESHDS_Mesh* myReader = new DriverUNV_R_SMESHDS_Mesh;


    myReader->SetMesh(myMesh);
    myReader->SetFile(myFile);
    //myReader->SetFileId(fid);

    myReader->Read();

  }


}
