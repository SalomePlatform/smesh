using namespace std;
#include "DriverUNV_R_SMDS_Mesh.h"

#include "utilities.h"


extern "C"
{

/**
 * Factory function which will be called by SMESHDriver
 */
void * SMESH_createUNVMeshReader()
{
	return new DriverUNV_R_SMDS_Mesh();
}

}

DriverUNV_R_SMDS_Mesh::DriverUNV_R_SMDS_Mesh()
{
	;
}

DriverUNV_R_SMDS_Mesh::~DriverUNV_R_SMDS_Mesh()
{
	;
}

void DriverUNV_R_SMDS_Mesh::SetMesh(SMDS_Mesh * aMesh)
{
	myMesh = aMesh;
}

void DriverUNV_R_SMDS_Mesh::SetFile(string aFile)
{
	myFile = aFile;
}

void DriverUNV_R_SMDS_Mesh::SetFileId(FILE * aFileId)
{
	myFileId = aFileId;
}

void DriverUNV_R_SMDS_Mesh::SetMeshId(int aMeshId)
{
	myMeshId = aMeshId;
}

void DriverUNV_R_SMDS_Mesh::Add()
{
	;
}

void DriverUNV_R_SMDS_Mesh::Read()
{

	int cell = 0, node = 0, n1, n2, n3, n4, n_nodes, nodes[6], blockId, i;
	char *s1, *s2, *s3;
	string str1, str2, str3;
	int i1 = 0;
	bool ok, found_block2411, found_block2412;

  /****************************************************************************
  *                      OUVERTURE DU FICHIER EN LECTURE                      *
  ****************************************************************************/
	char *file2Read = (char *)myFile.c_str();
	myFileId = fopen(file2Read, "r");
	if (myFileId < 0)
	{
		fprintf(stderr, ">> ERREUR : ouverture du fichier %s \n", file2Read);
		exit(EXIT_FAILURE);
	}

	s1 = (char *)malloc(sizeof(char) * 100);
	s2 = (char *)malloc(sizeof(char) * 100);
	s3 = (char *)malloc(sizeof(char) * 100);

	found_block2411 = false;
	found_block2412 = false;

	do
	{

		while (i1 == -1)
		{
			fscanf(myFileId, "%d\n", &blockId);
			switch (blockId)
			{
			case 2411:
			{
				MESSAGE("BlockId " << blockId);
				fscanf(myFileId, "%d", &node);
				//MESSAGE("Node "<<node);
				while (node != -1)
				{
					fscanf(myFileId, "%d %d %d\n", &n1, &n2, &n3);
					fscanf(myFileId, "%s %s %s\n", s1, s2, s3);
					str1 = string(s1);
					str2 = string(s2);
					str3 = string(s3);
					if (str1.find("D") != string::npos)
						str1.replace(str1.find("D"), 1, "E");
					if (str2.find("D") != string::npos)
						str2.replace(str2.find("D"), 1, "E");
					if (str3.find("D") != string::npos)
						str3.replace(str3.find("D"), 1, "E");
					ok = myMesh->AddNodeWithID(atof(str1.c_str()),
						atof(str2.c_str()), atof(str3.c_str()), node);
					fscanf(myFileId, "%d", &node);
				}
				i1 = 0;
				found_block2411 = true;
				break;
			}
			case 2412:
			{
				MESSAGE("BlockId " << blockId);
				fscanf(myFileId, "%d", &cell);
				//MESSAGE("Cell "<<cell);
				while (cell != -1)
				{
					fscanf(myFileId, "%d %d %d %d %d\n", &n1, &n2, &n3, &n4,
						&n_nodes);

					if ((n1 == 71) || (n1 == 72) || (n1 == 74) || (n1 == 91) ||
						(n1 == 92))
					{			//203
						if (n_nodes == 3)
						{
							for (i = 1; i <= n_nodes; i++)
								fscanf(myFileId, "%d", &nodes[i - 1]);
							ok = myMesh->AddFaceWithID(nodes[0], nodes[1],
								nodes[2], cell);
						}
						else if (n_nodes == 6)
						{		//206
							for (i = 1; i <= n_nodes; i++)
								fscanf(myFileId, "%d", &nodes[i - 1]);
							ok = myMesh->AddFaceWithID(nodes[0], nodes[2],
								nodes[4], cell);
						}
					}

					else if ((n1 == 11) || (n1 == 21) || (n1 == 24) ||
						(n1 == 25))
					{			//103
						fgets(s2, 100, myFileId);
						if (n_nodes == 3)
						{
							for (i = 1; i <= n_nodes; i++)
								fscanf(myFileId, "%d", &nodes[i - 1]);
							ok = myMesh->AddEdgeWithID(nodes[0], nodes[1],
								cell);
							//MESSAGE("in 103 "<<cell);
						}
						else if (n_nodes == 2)
						{		//102
							for (i = 1; i <= n_nodes; i++)
								fscanf(myFileId, "%d", &nodes[i - 1]);
							ok = myMesh->AddEdgeWithID(nodes[0], nodes[1],
								cell);
							//MESSAGE("in 102 "<<cell);
						}
					}

					fscanf(myFileId, "\n");
					fscanf(myFileId, "%d", &cell);
				}

				i1 = 0;
				found_block2412 = true;
				break;
			}
			case -1:
			{
				break;
			}
			default:
				MESSAGE("BlockId " << blockId);
				i1 = 0;
				break;
			}
		}

		fscanf(myFileId, "%s\n", s1);
		i1 = atoi(s1);

	}
	while ((!feof(myFileId)) && ((!found_block2411) || (!found_block2412)));

  /****************************************************************************
  *                      FERMETURE DU FICHIER                      *
  ****************************************************************************/
	free(s1);
	free(s2);
	free(s3);
	fclose(myFileId);

}
