#include "SMDS_VtkCellIterator.hxx"
#include "utilities.h"

SMDS_VtkCellIterator::SMDS_VtkCellIterator(SMDS_Mesh* mesh, int vtkCellId, SMDSAbs_EntityType aType) :
  _mesh(mesh), _cellId(vtkCellId), _index(0), _type(aType)
{
  //MESSAGE("SMDS_VtkCellIterator " << _type);
  _vtkIdList = vtkIdList::New();
  vtkUnstructuredGrid* grid = _mesh->getGrid();
  grid->GetCellPoints(_cellId, _vtkIdList);
  _nbNodes = _vtkIdList->GetNumberOfIds();
  switch (_type)
  {
    case SMDSEntity_Tetra:
      {
        this->exchange(1, 2);
        break;
      }
    case SMDSEntity_Pyramid:
      {
        this->exchange(1, 3);
        break;
      }
    case SMDSEntity_Penta:
      {
        //this->exchange(1, 2);
        //this->exchange(4, 5);
        break;
      }
    case SMDSEntity_Hexa:
      {
        this->exchange(1, 3);
        this->exchange(5, 7);
        break;
      }
    case SMDSEntity_Quad_Tetra:
      {
        this->exchange(1, 2);
        this->exchange(4, 6);
        this->exchange(8, 9);
        break;
      }
    case SMDSEntity_Quad_Pyramid:
      {
        this->exchange(1, 3);
        this->exchange(5, 8);
        this->exchange(6, 7);
        this->exchange(10, 12);
        break;
      }
    case SMDSEntity_Quad_Penta:
      {
        //this->exchange(1, 2);
        //this->exchange(4, 5);
        //this->exchange(6, 8);
        //this->exchange(9, 11);
        //this->exchange(13, 14);
        break;
      }
    case SMDSEntity_Quad_Hexa:
      {
        MESSAGE("SMDS_VtkCellIterator Quad_Hexa");
        this->exchange(1, 3);
        this->exchange(5, 7);
        this->exchange(8, 11);
        this->exchange(9, 10);
        this->exchange(12, 15);
        this->exchange(13, 14);
        this->exchange(17, 19);
        break;
      }
    case SMDSEntity_Polyhedra:
      MESSAGE("SMDS_VtkCellIterator Polyhedra (iterate on actual nodes)");
      break;
    default:
      break;
  }
}

SMDS_VtkCellIterator::~SMDS_VtkCellIterator()
{
  _vtkIdList->Delete();
}

bool SMDS_VtkCellIterator::more()
{
  return (_index < _nbNodes);
}

const SMDS_MeshElement* SMDS_VtkCellIterator::next()
{
  vtkIdType id = _vtkIdList->GetId(_index++);
  return _mesh->FindNodeVtk(id);
}

SMDS_VtkCellIteratorToUNV::SMDS_VtkCellIteratorToUNV(SMDS_Mesh* mesh, int vtkCellId, SMDSAbs_EntityType aType) :
  SMDS_VtkCellIterator()
{
  _mesh = mesh;
  _cellId = vtkCellId;
  _index = 0;
  _type = aType;
  //MESSAGE("SMDS_VtkCellInterlacedIterator (UNV)" << _type);

  _vtkIdList = vtkIdList::New();
  vtkIdType* pts;
  vtkUnstructuredGrid* grid = _mesh->getGrid();
  grid->GetCellPoints(_cellId, _nbNodes, pts);
  _vtkIdList->SetNumberOfIds(_nbNodes);
  int *ids = 0;
  switch (_type)
  {
    case SMDSEntity_Quad_Edge:
      {
        static int id[] = { 0, 2, 1 };
        ids = id;
        break;
      }
    case SMDSEntity_Quad_Triangle:
      {
        static int id[] = { 0, 3, 1, 4, 2, 5 };
        ids = id;
        break;
      }
    case SMDSEntity_Quad_Quadrangle:
      {
        static int id[] = { 0, 4, 1, 5, 2, 6, 3, 7 };
        ids = id;
        break;
      }
    case SMDSEntity_Quad_Tetra:
      {
        static int id[] = { 0, 4, 1, 5, 2, 6, 7, 8, 9, 3 };
        ids = id;
        break;
      }
    case SMDSEntity_Quad_Pyramid:
      {
        static int id[] = { 0, 5, 1, 6, 2, 7, 3, 8, 9, 10, 11, 12, 4 };
        ids = id;
        break;
      }
    case SMDSEntity_Penta:
      {
        static int id[] = { 0, 2, 1, 3, 5, 4 };
        ids = id;
        break;
      }
    case SMDSEntity_Quad_Penta:
      {
        static int id[] = { 0, 8, 2, 7, 1, 6, 12, 14, 13, 3, 11, 5, 10, 4, 9 };
        ids = id;
        break;
      }
    case SMDSEntity_Quad_Hexa:
      {
        static int id[] = { 0, 8, 1, 9, 2, 10, 3, 11, 16, 17, 18, 19, 4, 12, 5, 13, 6, 14, 7, 15 };
        ids = id;
        break;
      }
    case SMDSEntity_Polygon:
    case SMDSEntity_Quad_Polygon:
    case SMDSEntity_Polyhedra:
    case SMDSEntity_Quad_Polyhedra:
    default:
      {
        static int id[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
                            25, 26, 27, 28, 29 };
        ids = id;
        break;
      }
  }
  //MESSAGE("_nbNodes " << _nbNodes);
  for (int i = 0; i < _nbNodes; i++)
    _vtkIdList->SetId(i, pts[ids[i]]);
}

SMDS_VtkCellIteratorToUNV::~SMDS_VtkCellIteratorToUNV()
{
}

SMDS_VtkCellIteratorPolyH::SMDS_VtkCellIteratorPolyH(SMDS_Mesh* mesh, int vtkCellId, SMDSAbs_EntityType aType) :
  SMDS_VtkCellIterator()
{
  _mesh = mesh;
  _cellId = vtkCellId;
  _index = 0;
  _type = aType;
  //MESSAGE("SMDS_VtkCellIteratorPolyH " << _type);
  _vtkIdList = vtkIdList::New();
  vtkUnstructuredGrid* grid = _mesh->getGrid();
  grid->GetCellPoints(_cellId, _vtkIdList);
  _nbNodes = _vtkIdList->GetNumberOfIds();
  switch (_type)
  {
    case SMDSEntity_Polyhedra:
      {
        //MESSAGE("SMDS_VtkCellIterator Polyhedra");
        vtkIdType nFaces = 0;
        vtkIdType* ptIds = 0;
        grid->GetFaceStream(_cellId, nFaces, ptIds);
        int id = 0;
        _nbNodesInFaces = 0;
        for (int i = 0; i < nFaces; i++)
          {
            int nodesInFace = ptIds[id]; // nodeIds in ptIds[id+1 .. id+nodesInFace]
            _nbNodesInFaces += nodesInFace;
            id += (nodesInFace + 1);
          }
        _vtkIdList->SetNumberOfIds(_nbNodesInFaces);
        id = 0;
        int n = 0;
        for (int i = 0; i < nFaces; i++)
          {
            int nodesInFace = ptIds[id]; // nodeIds in ptIds[id+1 .. id+nodesInFace]
            for (int k = 1; k <= nodesInFace; k++)
              _vtkIdList->SetId(n++, ptIds[id + k]);
            id += (nodesInFace + 1);
          }
        break;
      }
    default:
      assert(0);
  }
}

SMDS_VtkCellIteratorPolyH::~SMDS_VtkCellIteratorPolyH()
{
}

bool SMDS_VtkCellIteratorPolyH::more()
{
  return (_index < _nbNodesInFaces);
}
