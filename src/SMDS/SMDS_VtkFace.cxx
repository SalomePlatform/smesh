#include "SMDS_VtkFace.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDS_VtkCellIterator.hxx"

#include "utilities.h"

#include <vector>

using namespace std;

SMDS_VtkFace::SMDS_VtkFace()
{
}

SMDS_VtkFace::SMDS_VtkFace(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh)
{
  init(nodeIds, mesh);
}

SMDS_VtkFace::~SMDS_VtkFace()
{
}

void SMDS_VtkFace::init(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh)
{
  vtkUnstructuredGrid* grid = mesh->getGrid();
  myIdInShape = -1;
  myMeshId = mesh->getMeshId();
  vtkIdType aType = VTK_TRIANGLE;
  switch (nodeIds.size())
  {
    case 3:
      aType = VTK_TRIANGLE;
      break;
    case 4:
      aType = VTK_QUAD;
      break;
    case 6:
      aType = VTK_QUADRATIC_TRIANGLE;
      break;
    case 8:
      aType = VTK_QUADRATIC_QUAD;
      break;
    default:
      aType = VTK_POLYGON;
      break;
  }
  myVtkID = grid->InsertNextLinkedCell(aType, nodeIds.size(), &nodeIds[0]);
  mesh->setMyModified();
  //MESSAGE("SMDS_VtkFace::init myVtkID " << myVtkID);
}

void SMDS_VtkFace::initPoly(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh)
{
  vtkUnstructuredGrid* grid = mesh->getGrid();
  myIdInShape = -1;
  myMeshId = mesh->getMeshId();
  myVtkID = grid->InsertNextLinkedCell(VTK_POLYGON, nodeIds.size(), &nodeIds[0]);
  mesh->setMyModified();
}

bool SMDS_VtkFace::ChangeNodes(const SMDS_MeshNode* nodes[], const int nbNodes)
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType npts = 0;
  vtkIdType* pts = 0;
  grid->GetCellPoints(myVtkID, npts, pts);
  if (nbNodes != npts)
    {
      MESSAGE("ChangeNodes problem: not the same number of nodes " << npts << " -> " << nbNodes);
      return false;
    }
  for (int i = 0; i < nbNodes; i++)
    {
      pts[i] = nodes[i]->getVtkId();
    }
  SMDS_Mesh::_meshList[myMeshId]->setMyModified();
  return true;
}

void SMDS_VtkFace::Print(std::ostream & OS) const
{
  OS << "face <" << GetID() << "> : ";
}

int SMDS_VtkFace::NbEdges() const
{
  // TODO quadratic polygons ?
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  int nbEdges = 3;
  switch (aVtkType)
  {
    case VTK_TRIANGLE:
    case VTK_QUADRATIC_TRIANGLE:
      nbEdges = 3;
      break;
    case VTK_QUAD:
    case VTK_QUADRATIC_QUAD:
      nbEdges = 4;
      break;
    case VTK_POLYGON:
    default:
      nbEdges = grid->GetCell(myVtkID)->GetNumberOfPoints();
      break;
  }
  return nbEdges;
}

int SMDS_VtkFace::NbFaces() const
{
  return 1;
}

int SMDS_VtkFace::NbNodes() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  int nbPoints = grid->GetCell(myVtkID)->GetNumberOfPoints();
  return nbPoints;
}

/*!
 * \brief Return node by its index
 * \param ind - node index
 * \retval const SMDS_MeshNode* - the node
 */
const SMDS_MeshNode*
SMDS_VtkFace::GetNode(const int ind) const
{
  return SMDS_MeshElement::GetNode(ind); // --- a optimiser !
}

bool SMDS_VtkFace::IsQuadratic() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  // TODO quadratic polygons ?
  switch (aVtkType)
  {
    case VTK_QUADRATIC_TRIANGLE:
    case VTK_QUADRATIC_QUAD:
      return true;
      break;
    default:
      return false;
  }
}

bool SMDS_VtkFace::IsPoly() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  return (aVtkType == VTK_POLYGON);
}

bool SMDS_VtkFace::IsMediumNode(const SMDS_MeshNode* node) const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  int rankFirstMedium = 0;
  switch (aVtkType)
  {
    case VTK_QUADRATIC_TRIANGLE:
      rankFirstMedium = 3; // medium nodes are of rank 3,4,5
      break;
    case VTK_QUADRATIC_QUAD:
      rankFirstMedium = 4; // medium nodes are of rank 4,5,6,7
      break;
    default:
      //MESSAGE("wrong element type " << aVtkType);
      return false;
  }
  vtkIdType npts = 0;
  vtkIdType* pts = 0;
  grid->GetCellPoints(myVtkID, npts, pts);
  vtkIdType nodeId = node->getVtkId();
  for (int rank = 0; rank < npts; rank++)
    {
      if (pts[rank] == nodeId)
        {
          //MESSAGE("rank " << rank << " is medium node " << (rank < rankFirstMedium));
          if (rank < rankFirstMedium)
            return false;
          else
            return true;
        }
    }
  //throw SALOME_Exception(LOCALIZED("node does not belong to this element"));
  MESSAGE("======================================================");
  MESSAGE("= IsMediumNode: node does not belong to this element =");
  MESSAGE("======================================================");
  return false;
}

SMDSAbs_EntityType SMDS_VtkFace::GetEntityType() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  SMDSAbs_EntityType aType = SMDSEntity_Polygon;
  switch (aVtkType)
  {
    case VTK_TRIANGLE:
      aType = SMDSEntity_Triangle;
      break;
    case VTK_QUAD:
      aType = SMDSEntity_Quadrangle;
      break;
    case VTK_QUADRATIC_TRIANGLE:
      aType = SMDSEntity_Quad_Triangle;
      break;
    case VTK_QUADRATIC_QUAD:
      aType = SMDSEntity_Quad_Quadrangle;
      break;
    default:
      aType = SMDSEntity_Polygon;
  }
  return aType;
}

vtkIdType SMDS_VtkFace::GetVtkType() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  return aVtkType;
}

SMDS_ElemIteratorPtr SMDS_VtkFace::elementsIterator(SMDSAbs_ElementType type) const
{
  switch (type)
  {
    case SMDSAbs_Node:
      return SMDS_ElemIteratorPtr(new SMDS_VtkCellIterator(SMDS_Mesh::_meshList[myMeshId], myVtkID, GetEntityType()));
    default:
      MESSAGE("ERROR : Iterator not implemented")
      ;
      return SMDS_ElemIteratorPtr((SMDS_ElemIterator*) NULL);
  }
}

SMDS_ElemIteratorPtr SMDS_VtkFace::nodesIteratorToUNV() const
{
  return SMDS_ElemIteratorPtr(new SMDS_VtkCellIteratorToUNV(SMDS_Mesh::_meshList[myMeshId], myVtkID, GetEntityType()));
}

SMDS_ElemIteratorPtr SMDS_VtkFace::interlacedNodesElemIterator() const
{
  return SMDS_ElemIteratorPtr(new SMDS_VtkCellIteratorToUNV(SMDS_Mesh::_meshList[myMeshId], myVtkID, GetEntityType()));
}

//! change only the first node, used for temporary triangles in quadrangle to triangle adaptor
void SMDS_VtkFace::ChangeApex(const SMDS_MeshNode* node)
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType npts = 0;
  vtkIdType* pts = 0;
  grid->GetCellPoints(myVtkID, npts, pts);
  grid->RemoveReferenceToCell(pts[0], myVtkID);
  pts[0] = node->getVtkId();
  grid->AddReferenceToCell(pts[0], myVtkID);
  SMDS_Mesh::_meshList[myMeshId]->setMyModified();
}
