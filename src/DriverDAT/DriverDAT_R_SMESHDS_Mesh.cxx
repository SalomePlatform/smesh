using namespace std;
#include "DriverDAT_R_SMESHDS_Mesh.h"
#include "DriverDAT_R_SMDS_Mesh.h"

#include "utilities.h"

DriverDAT_R_SMESHDS_Mesh::DriverDAT_R_SMESHDS_Mesh() {
;
}

DriverDAT_R_SMESHDS_Mesh::~DriverDAT_R_SMESHDS_Mesh() {
;
}

void DriverDAT_R_SMESHDS_Mesh::SetMesh(Handle(SMDS_Mesh)& aMesh) {
  //myMesh = Handle(SMESHDS_Mesh)::DownCast(aMesh);
  myMesh = aMesh;
}

void DriverDAT_R_SMESHDS_Mesh::SetFile(string aFile) {
  myFile = aFile;
}

void DriverDAT_R_SMESHDS_Mesh::SetFileId(FILE* aFileId) {
  myFileId = aFileId;
}

void DriverDAT_R_SMESHDS_Mesh::SetMeshId(int aMeshId) {
  myMeshId = aMeshId;
}

void DriverDAT_R_SMESHDS_Mesh::Add() {
  ;
}

void DriverDAT_R_SMESHDS_Mesh::Read() {
  string myClass = string("SMDS_Mesh");
  string myExtension = string("DAT");

  MESSAGE("in DriverDAT_R_SMESHDS_Mesh::Read() 1");
  DriverDAT_R_SMDS_Mesh* myReader = new DriverDAT_R_SMDS_Mesh;

  MESSAGE("in DriverDAT_R_SMESHDS_Mesh::Read() 2");
  myReader->SetMesh(myMesh);
  MESSAGE("in DriverDAT_R_SMESHDS_Mesh::Read() 3");
  myReader->SetFile(myFile);
  //myReader->SetFileId(myFileId);

  MESSAGE("in DriverDAT_R_SMESHDS_Mesh::Read() 4");
  myReader->Read();

}
