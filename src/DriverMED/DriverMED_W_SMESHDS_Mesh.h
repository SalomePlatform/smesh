#ifndef _INCLUDE_DRIVERMED_W_SMESHDS_MESH
#define _INCLUDE_DRIVERMED_W_SMESHDS_MESH

#include "SMESHDS_Mesh.hxx"
#include "Mesh_Writer.h"

#include <string>

extern "C"
{
#include <med.h>
}

class DriverMED_W_SMESHDS_Mesh : public Mesh_Writer {

  public :
    DriverMED_W_SMESHDS_Mesh();
    ~DriverMED_W_SMESHDS_Mesh();

    void Add(); 
    void Write();
    void SetMesh(Handle(SMDS_Mesh)& aMesh);
    void SetFile(string);

    void SetFileId(med_idt);
    void SetMeshId(int);

    void CreateFamily(char*, char*, int, med_int);

private :
    Handle_SMDS_Mesh myMesh;
    string myFile; 
    med_idt myFileId;
    int myMeshId;

};
#endif
