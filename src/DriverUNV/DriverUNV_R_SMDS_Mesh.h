#ifndef _INCLUDE_DRIVERUNV_R_SMDS_MESH
#define _INCLUDE_DRIVERUNV_R_SMDS_MESH

#include <stdio.h>

#include "SMDS_Mesh.hxx"
#include "Mesh_Reader.h"

class DriverUNV_R_SMDS_Mesh:public Mesh_Reader
{

  public:DriverUNV_R_SMDS_Mesh();
	~DriverUNV_R_SMDS_Mesh();

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
