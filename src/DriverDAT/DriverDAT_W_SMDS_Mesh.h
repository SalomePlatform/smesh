#ifndef _INCLUDE_DRIVERDAT_W_SMDS_MESH
#define _INCLUDE_DRIVERDAT_W_SMDS_MESH

#include <stdio.h>
#include <string>

#include "SMDS_Mesh.hxx"
#include "Mesh_Writer.h"

class DriverDAT_W_SMDS_Mesh : public Mesh_Writer {

  public :
    DriverDAT_W_SMDS_Mesh();
    ~DriverDAT_W_SMDS_Mesh();

    void Add();
    void Write();
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
