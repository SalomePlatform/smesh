using namespace std;
#include "DriverMED_W_SMESHDS_Document.h"
#include "DriverMED_W_SMESHDS_Mesh.h"
#include "utilities.h"

#include <stdio.h>

extern "C" 
{
  Document_Writer* Wmaker() {
    return new DriverMED_W_SMESHDS_Document;
  }
}

DriverMED_W_SMESHDS_Document::DriverMED_W_SMESHDS_Document() {
;
}

DriverMED_W_SMESHDS_Document::~DriverMED_W_SMESHDS_Document() {
;
}

//void DriverMED_W_SMESHDS_Document::SetFile(string aFile) {
//myFile = aFile;
//}

//void DriverMED_W_SMESHDS_Document::SetDocument(Handle(SMESHDS_Document)& aDocument) {
//myDocument = aDocument;
//}

void DriverMED_W_SMESHDS_Document::Write() {

  med_err ret = 0;
  med_idt fid;
  med_int nmaa,numero,nb_of_meshes;
  med_mode_switch mode_coo;
  med_connectivite typ_con;

  SCRUTE(myFile);
  Handle(SMESHDS_Mesh) myMesh;

  /******** Nombre de maillages ********/
  nb_of_meshes = myDocument->NbMeshes(); //voir avec Yves
  MESSAGE("nb_of_meshes = "<<nb_of_meshes<<"\n");
  SCRUTE(nb_of_meshes);
  //nb_of_meshes = 1;
  numero = 0;

  string myClass = string("SMESHDS_Mesh");
  string myExtension = string("MED");

  //while (numero<nb_of_meshes) {
  //numero++;
  //myMesh = myDocument->GetMesh(numero);

  myDocument->InitMeshesIterator();
  if (nb_of_meshes!=0)
  for (;myDocument->MoreMesh();myDocument->NextMesh()) {
    numero++;
    myMesh = myDocument->CurrentMesh();

    DriverMED_W_SMESHDS_Mesh* myWriter = new DriverMED_W_SMESHDS_Mesh;

    myWriter->SetMesh(myMesh);
    myWriter->SetFile(myFile);
    //myWriter->SetFileId(fid);
    myWriter->SetMeshId(numero);

    //myWriter->Write();
    myWriter->Add();
  }


}
