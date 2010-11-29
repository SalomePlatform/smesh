#include "SMDS_MeshCell.hxx"
#include "utilities.h"

using namespace std;

int SMDS_MeshCell::nbCells = 0;

SMDS_MeshCell::SMDS_MeshCell() :
  SMDS_MeshElement(-1)
{
  nbCells++;
  myVtkID = -1;
}

SMDS_MeshCell::~SMDS_MeshCell()
{
  nbCells--;
}
