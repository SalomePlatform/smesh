#ifndef _INCLUDE_DRIVERDAT_R_SMDS_MESH
#define _INCLUDE_DRIVERDAT_R_SMDS_MESH

#include <stdio.h>

#include "SMDS_Mesh.hxx"
#include "Mesh_Reader.h"

class DriverDAT_R_SMDS_Mesh : public Mesh_Reader {

  public :
    DriverDAT_R_SMDS_Mesh();
    ~DriverDAT_R_SMDS_Mesh();

    void Add();
    void Read();
    void SetMesh(Handle(SMDS_Mesh)& aMesh);
    void SetFile(string);

    void SetFileId(FILE*);
    void SetMeshId(int);

  private :
    Handle_SMDS_Mesh myMesh;
    string myFile; 
    FILE* myFileId;
    int myMeshId;

};
#endif
