using namespace std;
#include "DriverMED_R_SMESHDS_Document.h"
#include "DriverMED_R_SMESHDS_Mesh.h"
#include "utilities.h"

extern "C" {
  Document_Reader* maker() {
    return new DriverMED_R_SMESHDS_Document;
  }
}

DriverMED_R_SMESHDS_Document::DriverMED_R_SMESHDS_Document() {
;
}

DriverMED_R_SMESHDS_Document::~DriverMED_R_SMESHDS_Document() {
;
}

//void DriverMED_R_SMESHDS_Document::SetFile(string aFile) {
//  myFile = aFile;
//}

void DriverMED_R_SMESHDS_Document::Read() {

  med_err ret = 0;
  med_idt fid;
  med_int nmaa;

  Standard_Integer myMeshId;

  //string myFile = string("/home/home_users/cai/projects/salome_prev04/SALOME_ROOT/data/fra1.med");

/****************************************************************************
  *                      OUVERTURE DU FICHIER EN LECTURE                      *
  ****************************************************************************/
  char* file2Read = (char*)myFile.c_str();
  fid = MEDouvrir(file2Read,MED_LECT);
  if (fid < 0)
    {
      fprintf(stderr,">> ERREUR : ouverture du fichier %s \n",file2Read);
      exit(EXIT_FAILURE);
    }

  /****************************************************************************
  *                      COMBIEN DE MAILLAGES ?                               *
  ****************************************************************************/
  nmaa = MEDnMaa(fid);
  if (nmaa < 0)
    {
      fprintf(stderr,">> ERREUR : lecture du nombre de maillages \n");
      exit(EXIT_FAILURE);
    }
  printf("Nombre de maillages = %d\n",nmaa);

  string myClass = string("SMESHDS_Mesh");
  string myExtension = string("MED");


  for (int meshIt=1;meshIt<=nmaa;meshIt++) {
    myMeshId = myDocument->NewMesh();
    printf("MeshId = %d\n",myMeshId);

    Handle(SMDS_Mesh) myMesh = myDocument->GetMesh(myMeshId);
  SCRUTE(myMesh);

    // Handle (SMDS_Mesh) MMM =.........
    //ex Handle(SMESHDS_Mesh) myMesh2 =Handle(SMESHDS_Mesh)::DownCast(MMM);

    DriverMED_R_SMESHDS_Mesh* myReader = new DriverMED_R_SMESHDS_Mesh;

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
    fprintf(stderr,">> ERREUR : erreur a la fermeture du fichier %s\n",file2Read);


}
