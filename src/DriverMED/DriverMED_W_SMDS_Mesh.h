#ifndef _INCLUDE_DRIVERMED_W_SMDS_MESH
#define _INCLUDE_DRIVERMED_W_SMDS_MESH

#include "SMDS_Mesh.hxx"
#include "Mesh_Writer.h"

#include <string>

extern "C"
{
#include <med.h>
}

class DriverMED_W_SMDS_Mesh : public Mesh_Writer {

  public :
    DriverMED_W_SMDS_Mesh();
    ~DriverMED_W_SMDS_Mesh();

    void Add(); 
    void Write();
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
