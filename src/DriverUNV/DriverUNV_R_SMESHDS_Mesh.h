#ifndef _INCLUDE_DRIVERUNV_R_SMESHDS_MESH
#define _INCLUDE_DRIVERUNV_R_SMESHDS_MESH

#include <stdio.h>

#include "SMESHDS_Mesh.hxx"
#include "Mesh_Reader.h"

class DriverUNV_R_SMESHDS_Mesh:public Mesh_Reader
{

  public:DriverUNV_R_SMESHDS_Mesh();
	~DriverUNV_R_SMESHDS_Mesh();

	void Add();
	void Read();
	void SetMesh(SMDS_Mesh * aMesh);
	void SetFile(string);

	void SetFileId(FILE *);
	void SetMeshId(int);

  private:  SMDS_Mesh * myMesh;
	string myFile;
	FILE *myFileId;
	int myMeshId;

};
#endif
