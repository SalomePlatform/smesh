#ifndef _INCLUDE_DRIVERMED_R_SMDS_MESH
#define _INCLUDE_DRIVERMED_R_SMDS_MESH

#include "SMDS_Mesh.hxx"
#include "Mesh_Reader.h"

extern "C"
{
#include <med.h>
}

class DriverMED_R_SMDS_Mesh : public Mesh_Reader {

  public :
    DriverMED_R_SMDS_Mesh();
    ~DriverMED_R_SMDS_Mesh();

    void Add();
    void Read();
    void SetMesh(Handle(SMDS_Mesh)& aMesh);
    void SetFile(string);

    void SetFileId(med_idt);
    void SetMeshId(int);

private :
    Handle_SMDS_Mesh myMesh;
    string myFile; 
    med_idt myFileId;
    int myMeshId;

};
#endif
