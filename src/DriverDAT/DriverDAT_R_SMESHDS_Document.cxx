using namespace std;
#include "DriverDAT_R_SMESHDS_Document.h"
#include "DriverDAT_R_SMESHDS_Mesh.h"

#include "utilities.h"

int getOne() {
  printf("in getOne");
  return (1);
}

extern "C" {
  //  Document_Reader* maker() {
  DriverDAT_R_SMESHDS_Document* maker() {
    fprintf(stdout,"here in maker\n");
    return new DriverDAT_R_SMESHDS_Document;
  }
}

DriverDAT_R_SMESHDS_Document::DriverDAT_R_SMESHDS_Document() {
  myFile = string("");
}

DriverDAT_R_SMESHDS_Document::~DriverDAT_R_SMESHDS_Document() {
;
}

//void DriverDAT_R_SMESHDS_Document::SetFile(string aFile) {
//myFile = aFile;
//}

//void DriverDAT_R_SMESHDS_Document::SetDocument(Handle(SMESHDS_Document)& aDoc) {
//myDocument = aDoc;
//}

void DriverDAT_R_SMESHDS_Document::Read() {

  int myMeshId;
  MESSAGE("in read");
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
  string myExtension = string("DAT");

  for (int meshIt=1;meshIt<=nmaa;meshIt++) {
    myMeshId = myDocument->NewMesh();

    Handle(SMDS_Mesh) myMesh = myDocument->GetMesh(myMeshId);

    DriverDAT_R_SMESHDS_Mesh* myReader = new DriverDAT_R_SMESHDS_Mesh;


    myReader->SetMesh(myMesh);
    myReader->SetFile(myFile);
    //myReader->SetFileId(fid);

    myReader->Read();

  }


}
