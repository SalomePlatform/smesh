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
//  File   : DriverUNV_W_SMESHDS_Document.cxx
//  Module : SMESH

using namespace std;
#include "DriverUNV_W_SMESHDS_Document.h"
#include "DriverUNV_W_SMESHDS_Mesh.h"

#include "utilities.h"

extern "C" 
{
  Document_Writer* Wmaker() {
    return new DriverUNV_W_SMESHDS_Document;
  }
}

DriverUNV_W_SMESHDS_Document::DriverUNV_W_SMESHDS_Document() {
;
}

DriverUNV_W_SMESHDS_Document::~DriverUNV_W_SMESHDS_Document() {
;
}

//void DriverUNV_W_SMESHDS_Document::SetFile(string aFile) {
//myFile = aFile;
//}

//void DriverUNV_W_SMESHDS_Document::SetDocument(Handle(SMESHDS_Document)& aDocument) {
//myDocument = aDocument;
//}

void DriverUNV_W_SMESHDS_Document::Write() {

  Handle(SMESHDS_Mesh) myMesh;

  /****************************************************************************
  *                      OUVERTURE DU FICHIER EN ECRITURE                      *
  ****************************************************************************/
  char* file2Write = (char*)myFile.c_str();
  FILE* fid = fopen(file2Write,"w+");
  if (fid < 0)
    {
      fprintf(stderr,">> ERREUR : ouverture du fichier %s \n",file2Write);
      exit(EXIT_FAILURE);
    }

  /****************************************************************************
  *                      FERMETURE DU FICHIER                                 *
  ****************************************************************************/

  fclose(fid);

  /******** Nombre de maillages ********/
  int nb_of_meshes = myDocument->NbMeshes(); //voir avec Yves
  //nb_of_meshes = 1;
  int numero = 0;

  string myClass = string("SMESHDS_Mesh");
  string myExtension = string("UNV");

  //while (numero<nb_of_meshes) {
  //numero++;
  //myMesh = myDocument->GetMesh(numero);
  myDocument->InitMeshesIterator();
  for (;myDocument->MoreMesh();myDocument->NextMesh()) {
    numero++;
    myMesh = myDocument->CurrentMesh();

    DriverUNV_W_SMESHDS_Mesh* myWriter = new DriverUNV_W_SMESHDS_Mesh;
    //Mesh_Writer* myWriter = Driver::GetMeshWriter(myExtension, myClass);

    myWriter->SetMesh(myMesh);
    myWriter->SetFile(myFile);
  SCRUTE(myMesh);
    //myWriter->SetFileId(fid);
    myWriter->SetMeshId(numero);
    myWriter->Write();
  }

}
