#ifndef _INCLUDE_DRIVERMED_R_SMESHDS_MESH
#define _INCLUDE_DRIVERMED_R_SMESHDS_MESH

#include "SMESHDS_Mesh.hxx"
#include "Mesh_Reader.h"

#include <vector>
extern "C"
{
#include <med.h>
}

class DriverMED_R_SMESHDS_Mesh : public Mesh_Reader {

  public :
    DriverMED_R_SMESHDS_Mesh();
    ~DriverMED_R_SMESHDS_Mesh();

    void Read();
    void ReadMySelf();
    void Add();

    void SetMesh(Handle(SMDS_Mesh)& aMesh);
    void SetFile(string);
    void SetFileId(med_idt);
    void SetMeshId(int);

    void LinkMeshToShape(string , string , vector<int>);

private :
    Handle_SMDS_Mesh myMesh;
    string myFile; 
    med_idt myFileId;
    int myMeshId;

};
#endif
