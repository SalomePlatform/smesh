using namespace std;
#include "DriverUNV_R_SMESHDS_Mesh.h"
#include "DriverUNV_R_SMDS_Mesh.h"

#include "utilities.h"

DriverUNV_R_SMESHDS_Mesh::DriverUNV_R_SMESHDS_Mesh()
{
	;
}

DriverUNV_R_SMESHDS_Mesh::~DriverUNV_R_SMESHDS_Mesh()
{
	;
}

void DriverUNV_R_SMESHDS_Mesh::SetMesh(SMDS_Mesh * aMesh)
{
	//myMesh = SMESHDS_Mesh *::DownCast(aMesh);
	myMesh = aMesh;
}

void DriverUNV_R_SMESHDS_Mesh::SetFile(string aFile)
{
	myFile = aFile;
}

void DriverUNV_R_SMESHDS_Mesh::SetFileId(FILE * aFileId)
{
	myFileId = aFileId;
}

void DriverUNV_R_SMESHDS_Mesh::SetMeshId(int aMeshId)
{
	myMeshId = aMeshId;
}

void DriverUNV_R_SMESHDS_Mesh::Add()
{
	;
}

void DriverUNV_R_SMESHDS_Mesh::Read()
{
	string myClass = string("SMDS_Mesh");
	string myExtension = string("UNV");

	DriverUNV_R_SMDS_Mesh *myReader = new DriverUNV_R_SMDS_Mesh;

	myReader->SetMesh(myMesh);
	myReader->SetFile(myFile);
	//myReader->SetFileId(myFileId);

	myReader->Read();

}
