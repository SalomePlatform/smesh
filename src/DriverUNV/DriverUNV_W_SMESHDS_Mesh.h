#ifndef _INCLUDE_DRIVERUNV_W_SMESHDS_MESH
#define _INCLUDE_DRIVERUNV_W_SMESHDS_MESH

#include <stdio.h>
#include <string>

#include "SMESHDS_Mesh.hxx"
#include "Mesh_Writer.h"

class DriverUNV_W_SMESHDS_Mesh:public Mesh_Writer
{

  public:DriverUNV_W_SMESHDS_Mesh();
	~DriverUNV_W_SMESHDS_Mesh();

	void Add();
	void Write();
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
