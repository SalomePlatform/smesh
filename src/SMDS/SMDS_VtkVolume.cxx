#include "SMDS_VtkVolume.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDS_VtkCellIterator.hxx"

#include "utilities.h"

#include <vector>

SMDS_VtkVolume::SMDS_VtkVolume()
{
}

SMDS_VtkVolume::SMDS_VtkVolume(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh)
{
  init(nodeIds, mesh);
}
/*!
 * typed used are vtk types (@see vtkCellType.h)
 * see GetEntityType() for conversion in SMDS type (@see SMDSAbs_ElementType.hxx)
 */
void SMDS_VtkVolume::init(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh)
{
  vtkUnstructuredGrid* grid = mesh->getGrid();
  myIdInShape = -1;
  myMeshId = mesh->getMeshId();
  vtkIdType aType = VTK_TETRA;
  switch (nodeIds.size())
  {
    case 4:
      aType = VTK_TETRA;
      break;
    case 5:
      aType = VTK_PYRAMID;
      break;
    case 6:
      aType = VTK_WEDGE;
      break;
    case 8:
      aType = VTK_HEXAHEDRON;
      break;
    case 10:
      aType = VTK_QUADRATIC_TETRA;
      break;
    case 13:
      aType = VTK_QUADRATIC_PYRAMID;
      break;
    case 15:
      aType = VTK_QUADRATIC_WEDGE;
      break;
    case 20:
      aType = VTK_QUADRATIC_HEXAHEDRON;
      break;
    default:
      aType = VTK_HEXAHEDRON;
      break;
  }
  myVtkID = grid->InsertNextLinkedCell(aType, nodeIds.size(), &nodeIds[0]);
  mesh->setMyModified();
  //MESSAGE("SMDS_VtkVolume::init myVtkID " << myVtkID);
}

//#ifdef VTK_HAVE_POLYHEDRON
void SMDS_VtkVolume::initPoly(std::vector<vtkIdType> nodeIds, std::vector<int> nbNodesPerFace, SMDS_Mesh* mesh)
{
  //MESSAGE("SMDS_VtkVolume::initPoly");
  SMDS_UnstructuredGrid* grid = mesh->getGrid();
  double center[3];
  this->gravityCenter(grid, &nodeIds[0], nodeIds.size(), &center[0]);
  vector<vtkIdType> ptIds;
  ptIds.clear();
  vtkIdType nbFaces = nbNodesPerFace.size();
  int k = 0;
  for (int i = 0; i < nbFaces; i++)
    {
      int nf = nbNodesPerFace[i];
      ptIds.push_back(nf);
      double a[3];
      double b[3];
      double c[3];
      grid->GetPoints()->GetPoint(nodeIds[k], a);
      grid->GetPoints()->GetPoint(nodeIds[k + 1], b);
      grid->GetPoints()->GetPoint(nodeIds[k + 2], c);
      bool isFaceForward = this->isForward(a, b, c, center);
      //MESSAGE("isFaceForward " << i << " " << isFaceForward);
      vtkIdType *facePts = &nodeIds[k];
      if (isFaceForward)
        for (int n = 0; n < nf; n++)
          ptIds.push_back(facePts[n]);
      else
        for (int n = nf - 1; n >= 0; n--)
          ptIds.push_back(facePts[n]);
      k += nf;
    }
  myVtkID = grid->InsertNextLinkedCell(VTK_POLYHEDRON, nbFaces, &ptIds[0]);
  mesh->setMyModified();
}
//#endif

bool SMDS_VtkVolume::ChangeNodes(const SMDS_MeshNode* nodes[], const int nbNodes)
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

/*!
 * Reorder in VTK order a list of nodes given in SMDS order.
 * To be used before ChangeNodes: lists are given or computed in SMDS order.
 */
bool SMDS_VtkVolume::vtkOrder(const SMDS_MeshNode* nodes[], const int nbNodes)
{
  if (nbNodes != this->NbNodes())
    {
      MESSAGE("vtkOrder, wrong number of nodes " << nbNodes << " instead of "<< this->NbNodes());
      return false;
    }
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  switch (aVtkType)
  {
    case VTK_TETRA:
      this->exchange(nodes, 1, 2);
      break;
    case VTK_QUADRATIC_TETRA:
      this->exchange(nodes, 1, 2);
      this->exchange(nodes, 4, 6);
      this->exchange(nodes, 8, 9);
      break;
    case VTK_PYRAMID:
      this->exchange(nodes, 1, 3);
      break;
    case VTK_WEDGE:
      break;
    case VTK_QUADRATIC_PYRAMID:
      this->exchange(nodes, 1, 3);
      this->exchange(nodes, 5, 8);
      this->exchange(nodes, 6, 7);
      this->exchange(nodes, 10, 12);
      break;
    case VTK_QUADRATIC_WEDGE:
      break;
    case VTK_HEXAHEDRON:
      this->exchange(nodes, 1, 3);
      this->exchange(nodes, 5, 7);
      break;
    case VTK_QUADRATIC_HEXAHEDRON:
      this->exchange(nodes, 1, 3);
      this->exchange(nodes, 5, 7);
      this->exchange(nodes, 8, 11);
      this->exchange(nodes, 9, 10);
      this->exchange(nodes, 12, 15);
      this->exchange(nodes, 13, 14);
      this->exchange(nodes, 17, 19);
      break;
    case VTK_POLYHEDRON:
    default:
      break;
  }
  return true;
}

SMDS_VtkVolume::~SMDS_VtkVolume()
{
}

void SMDS_VtkVolume::Print(ostream & OS) const
{
  OS << "volume <" << GetID() << "> : ";
}

int SMDS_VtkVolume::NbFaces() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  int nbFaces = 4;
  switch (aVtkType)
  {
    case VTK_TETRA:
    case VTK_QUADRATIC_TETRA:
      nbFaces = 4;
      break;
    case VTK_PYRAMID:
    case VTK_WEDGE:
    case VTK_QUADRATIC_PYRAMID:
    case VTK_QUADRATIC_WEDGE:
      nbFaces = 5;
      break;
    case VTK_HEXAHEDRON:
    case VTK_QUADRATIC_HEXAHEDRON:
      nbFaces = 6;
      break;
    case VTK_POLYHEDRON:
      {
        vtkIdType nFaces = 0;
        vtkIdType* ptIds = 0;
        grid->GetFaceStream(this->myVtkID, nFaces, ptIds);
        nbFaces = nFaces;
        break;
      }
    default:
      MESSAGE("invalid volume type")
      ;
      nbFaces = 0;
      break;
  }
  return nbFaces;
}

int SMDS_VtkVolume::NbNodes() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  int nbPoints = 0;
  if (aVtkType != VTK_POLYHEDRON)
    {
      nbPoints = grid->GetCell(myVtkID)->GetNumberOfPoints();
    }
  else
    {
      vtkIdType nFaces = 0;
      vtkIdType* ptIds = 0;
      grid->GetFaceStream(this->myVtkID, nFaces, ptIds);
      int id = 0;
      for (int i = 0; i < nFaces; i++)
        {
          int nodesInFace = ptIds[id];
          nbPoints += nodesInFace;
          id += (nodesInFace + 1);
        }
    }
  return nbPoints;
}

int SMDS_VtkVolume::NbEdges() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  int nbEdges = 6;
  switch (aVtkType)
  {
    case VTK_TETRA:
    case VTK_QUADRATIC_TETRA:
      nbEdges = 6;
      break;
    case VTK_PYRAMID:
    case VTK_QUADRATIC_PYRAMID:
      nbEdges = 8;
      break;
    case VTK_WEDGE:
    case VTK_QUADRATIC_WEDGE:
      nbEdges = 9;
      break;
    case VTK_HEXAHEDRON:
    case VTK_QUADRATIC_HEXAHEDRON:
      nbEdges = 12;
      break;
    case VTK_POLYHEDRON:
      {
        vtkIdType nFaces = 0;
        vtkIdType* ptIds = 0;
        grid->GetFaceStream(this->myVtkID, nFaces, ptIds);
        nbEdges = 0;
        int id = 0;
        for (int i = 0; i < nFaces; i++)
          {
            int edgesInFace = ptIds[id];
            id += (edgesInFace + 1);
            nbEdges += edgesInFace;
          }
        nbEdges = nbEdges / 2;
        break;
      }
    default:
      MESSAGE("invalid volume type")
      ;
      nbEdges = 0;
      break;
  }
  return nbEdges;
}

/*! polyhedron only,
 *  1 <= face_ind <= NbFaces()
 */
int SMDS_VtkVolume::NbFaceNodes(const int face_ind) const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  int nbNodes = 0;
  if (aVtkType == VTK_POLYHEDRON)
    {
      vtkIdType nFaces = 0;
      vtkIdType* ptIds = 0;
      grid->GetFaceStream(this->myVtkID, nFaces, ptIds);
      int id = 0;
      for (int i = 0; i < nFaces; i++)
        {
          int nodesInFace = ptIds[id];
          id += (nodesInFace + 1);
          if (i == face_ind - 1)
            {
              nbNodes = nodesInFace;
              break;
            }
        }
    }
  return nbNodes;
}

/*! polyhedron only,
 *  1 <= face_ind <= NbFaces()
 *  1 <= node_ind <= NbFaceNodes()
 */
const SMDS_MeshNode* SMDS_VtkVolume::GetFaceNode(const int face_ind, const int node_ind) const
{
  SMDS_Mesh *mesh = SMDS_Mesh::_meshList[myMeshId];
  vtkUnstructuredGrid* grid = mesh->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  const SMDS_MeshNode* node = 0;
  if (aVtkType == VTK_POLYHEDRON)
    {
      vtkIdType nFaces = 0;
      vtkIdType* ptIds = 0;
      grid->GetFaceStream(this->myVtkID, nFaces, ptIds);
      int id = 0;
      for (int i = 0; i < nFaces; i++)
        {
          int nodesInFace = ptIds[id]; // nodeIds in ptIds[id+1 .. id+nodesInFace]
          if (i == face_ind - 1) // first face is number 1
            {
              if ((node_ind > 0) && (node_ind <= nodesInFace))
                node = mesh->FindNodeVtk(ptIds[id + node_ind]); // ptIds[id+1] : first node
              break;
            }
          id += (nodesInFace + 1);
        }
    }
  return node;
}

/*! polyhedron only,
 *  return number of nodes for each face
 */
const std::vector<int> SMDS_VtkVolume::GetQuantities() const
{
  vector<int> quantities;
  quantities.clear();
  SMDS_Mesh *mesh = SMDS_Mesh::_meshList[myMeshId];
  vtkUnstructuredGrid* grid = mesh->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  if (aVtkType == VTK_POLYHEDRON)
    {
      vtkIdType nFaces = 0;
      vtkIdType* ptIds = 0;
      grid->GetFaceStream(this->myVtkID, nFaces, ptIds);
      int id = 0;
      for (int i = 0; i < nFaces; i++)
        {
          int nodesInFace = ptIds[id]; // nodeIds in ptIds[id+1 .. id+nodesInFace]
          quantities.push_back(nodesInFace);
          id += (nodesInFace + 1);
        }
    }
  return quantities;
}

SMDS_ElemIteratorPtr SMDS_VtkVolume::elementsIterator(SMDSAbs_ElementType type) const
{
  switch (type)
  {
    case SMDSAbs_Node:
      {
        SMDSAbs_EntityType aType = this->GetEntityType();
        if (aType == SMDSEntity_Polyhedra)
          return SMDS_ElemIteratorPtr(new SMDS_VtkCellIteratorPolyH(SMDS_Mesh::_meshList[myMeshId], myVtkID, aType));
        else
          return SMDS_ElemIteratorPtr(new SMDS_VtkCellIterator(SMDS_Mesh::_meshList[myMeshId], myVtkID, aType));
      }
    default:
      MESSAGE("ERROR : Iterator not implemented");
      return SMDS_ElemIteratorPtr((SMDS_ElemIterator*) NULL);
  }
}

SMDS_ElemIteratorPtr SMDS_VtkVolume::nodesIteratorToUNV() const
{
  return SMDS_ElemIteratorPtr(new SMDS_VtkCellIteratorToUNV(SMDS_Mesh::_meshList[myMeshId], myVtkID, GetEntityType()));
}

SMDS_ElemIteratorPtr SMDS_VtkVolume::interlacedNodesElemIterator() const
{
  return SMDS_ElemIteratorPtr(new SMDS_VtkCellIteratorToUNV(SMDS_Mesh::_meshList[myMeshId], myVtkID, GetEntityType()));
}

SMDSAbs_ElementType SMDS_VtkVolume::GetType() const
{
  return SMDSAbs_Volume;
}

/*!
 * \brief Return node by its index
 * \param ind - node index
 * \retval const SMDS_MeshNode* - the node
 */
const SMDS_MeshNode* SMDS_VtkVolume::GetNode(const int ind) const
{
  // TODO optimize if possible (vtkCellIterator)
  return SMDS_MeshElement::GetNode(ind);
}

bool SMDS_VtkVolume::IsQuadratic() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  // TODO quadratic polyhedrons ?
  switch (aVtkType)
  {
    case VTK_QUADRATIC_TETRA:
    case VTK_QUADRATIC_PYRAMID:
    case VTK_QUADRATIC_WEDGE:
    case VTK_QUADRATIC_HEXAHEDRON:
      return true;
      break;
    default:
      return false;
  }
}

bool SMDS_VtkVolume::IsPoly() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  return (aVtkType == VTK_POLYHEDRON);
}

bool SMDS_VtkVolume::IsMediumNode(const SMDS_MeshNode* node) const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);
  int rankFirstMedium = 0;
  switch (aVtkType)
  {
    case VTK_QUADRATIC_TETRA:
      rankFirstMedium = 4; // medium nodes are of rank 4 to 9
      break;
    case VTK_QUADRATIC_PYRAMID:
      rankFirstMedium = 5; // medium nodes are of rank 5 to 12
      break;
    case VTK_QUADRATIC_WEDGE:
      rankFirstMedium = 6; // medium nodes are of rank 6 to 14
      break;
    case VTK_QUADRATIC_HEXAHEDRON:
      rankFirstMedium = 8; // medium nodes are of rank 8 to 19
      break;
    default:
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

SMDSAbs_EntityType SMDS_VtkVolume::GetEntityType() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aVtkType = grid->GetCellType(this->myVtkID);

  SMDSAbs_EntityType aType = SMDSEntity_Tetra;
  switch (aVtkType)
  {
    case VTK_TETRA:
      aType = SMDSEntity_Tetra;
      break;
    case VTK_PYRAMID:
      aType = SMDSEntity_Pyramid;
      break;
    case VTK_WEDGE:
      aType = SMDSEntity_Penta;
      break;
    case VTK_HEXAHEDRON:
      aType = SMDSEntity_Hexa;
      break;
    case VTK_QUADRATIC_TETRA:
      aType = SMDSEntity_Quad_Tetra;
      break;
    case VTK_QUADRATIC_PYRAMID:
      aType = SMDSEntity_Quad_Pyramid;
      break;
    case VTK_QUADRATIC_WEDGE:
      aType = SMDSEntity_Quad_Penta;
      break;
    case VTK_QUADRATIC_HEXAHEDRON:
      aType = SMDSEntity_Quad_Hexa;
      break;
//#ifdef VTK_HAVE_POLYHEDRON
    case VTK_POLYHEDRON:
      aType = SMDSEntity_Polyhedra;
      break;
//#endif
    default:
      aType = SMDSEntity_Polyhedra;
      break;
  }
  return aType;
}

vtkIdType SMDS_VtkVolume::GetVtkType() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  vtkIdType aType = grid->GetCellType(myVtkID);
  return aType;
}

void SMDS_VtkVolume::gravityCenter(SMDS_UnstructuredGrid* grid, vtkIdType *nodeIds, int nbNodes, double* result)
{
  for (int j = 0; j < 3; j++)
    result[j] = 0;
  if (nbNodes <= 0)
    return;
  for (int i = 0; i < nbNodes; i++)
    {
      double *coords = grid->GetPoint(nodeIds[i]);
      for (int j = 0; j < 3; j++)
        result[j] += coords[j];
    }
  for (int j = 0; j < 3; j++)
    result[j] = result[j] / nbNodes;
  //MESSAGE("center " << result[0] << " " << result[1] << " "  << result[2]);
  return;
}

bool SMDS_VtkVolume::isForward(double* a, double* b, double* c, double* d)
{
  double u[3], v[3], w[3];
  for (int j = 0; j < 3; j++)
    {
      //MESSAGE("a,b,c,d " << a[j] << " " << b[j] << " " << c[j] << " " << d[j]);
      u[j] = b[j] - a[j];
      v[j] = c[j] - a[j];
      w[j] = d[j] - a[j];
      //MESSAGE("u,v,w " << u[j] << " " << v[j] << " " << w[j]);
    }
  double prodmixte = (u[1]*v[2] - u[2]*v[1]) * w[0]
                   + (u[2]*v[0] - u[0]*v[2]) * w[1]
                   + (u[0]*v[1] - u[1]*v[0]) * w[2];
  return (prodmixte < 0);
}

/*! For polyhedron only
 *  @return actual number of nodes (not the sum of nodes of all faces)
 */
int SMDS_VtkVolume::NbUniqueNodes() const
{
  vtkUnstructuredGrid* grid = SMDS_Mesh::_meshList[myMeshId]->getGrid();
  return grid->GetCell(myVtkID)->GetNumberOfPoints();
}

/*! For polyhedron use only
 *  @return iterator on actual nodes (not through the faces)
 */
SMDS_ElemIteratorPtr SMDS_VtkVolume::uniqueNodesIterator() const
{
  MESSAGE("uniqueNodesIterator");
  return SMDS_ElemIteratorPtr(new SMDS_VtkCellIterator(SMDS_Mesh::_meshList[myMeshId], myVtkID, GetEntityType()));
}
