using namespace std;
#include "DriverUNV_W_SMESHDS_Document.h"
#include "DriverUNV_W_SMESHDS_Mesh.h"

#include "utilities.h"

extern "C"
{
	Document_Writer *Wmaker()
	{
		return new DriverUNV_W_SMESHDS_Document;
	}
}

DriverUNV_W_SMESHDS_Document::DriverUNV_W_SMESHDS_Document()
{
	;
}

DriverUNV_W_SMESHDS_Document::~DriverUNV_W_SMESHDS_Document()
{
	;
}

//void DriverUNV_W_SMESHDS_Document::SetFile(string aFile) {
//myFile = aFile;
//}

//void DriverUNV_W_SMESHDS_Document::SetDocument(Handle(SMESHDS_Document)& aDocument) {
//myDocument = aDocument;
//}

void DriverUNV_W_SMESHDS_Document::Write()
{

	SMESHDS_Mesh * myMesh;

  /****************************************************************************
  *                      OUVERTURE DU FICHIER EN ECRITURE                      *
  ****************************************************************************/
	char *file2Write = (char *)myFile.c_str();
	FILE *fid = fopen(file2Write, "w+");
	if (fid < 0)
	{
		fprintf(stderr, ">> ERREUR : ouverture du fichier %s \n", file2Write);
		exit(EXIT_FAILURE);
	}

  /****************************************************************************
  *                      FERMETURE DU FICHIER                                 *
  ****************************************************************************/

	fclose(fid);

  /******** Nombre de maillages ********/
	int nb_of_meshes = myDocument->NbMeshes();	//voir avec Yves
	//nb_of_meshes = 1;
	int numero = 0;

	string myClass = string("SMESHDS_Mesh");
	string myExtension = string("UNV");

	//while (numero<nb_of_meshes) {
	//numero++;
	//myMesh = myDocument->GetMesh(numero);
	myDocument->InitMeshesIterator();
	while(myDocument->MoreMesh())
	{
		numero++;
		myMesh = myDocument->NextMesh();

		DriverUNV_W_SMESHDS_Mesh *myWriter = new DriverUNV_W_SMESHDS_Mesh;
		//Mesh_Writer* myWriter = Driver::GetMeshWriter(myExtension, myClass);

		myWriter->SetMesh(myMesh);
		myWriter->SetFile(myFile);
		SCRUTE(myMesh);
		//myWriter->SetFileId(fid);
		myWriter->SetMeshId(numero);
		myWriter->Write();
	}

}
