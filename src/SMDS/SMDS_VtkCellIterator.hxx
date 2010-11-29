#ifndef _SMDS_VTKCELLITERATOR_HXX_
#define _SMDS_VTKCELLITERATOR_HXX_

#include "SMDS_ElemIterator.hxx"
#include "SMDS_Mesh.hxx"
#include "SMDSAbs_ElementType.hxx"

#include <vtkCell.h>
#include <vtkIdList.h>

class SMDS_VtkCellIterator: public SMDS_ElemIterator
{
public:
  SMDS_VtkCellIterator(SMDS_Mesh* mesh, int vtkCellId, SMDSAbs_EntityType aType);
  virtual ~SMDS_VtkCellIterator();
  virtual bool more();
  virtual const SMDS_MeshElement* next();
  inline void exchange(vtkIdType a, vtkIdType b)
  {
    vtkIdType t = _vtkIdList->GetId(a);
    _vtkIdList->SetId(a, _vtkIdList->GetId(b));
    _vtkIdList->SetId(b, t);
  }

protected:
  SMDS_VtkCellIterator() {};

  SMDS_Mesh* _mesh;
  int _cellId;
  int _index;
  int _nbNodes;
  SMDSAbs_EntityType _type;
  vtkIdList* _vtkIdList;
};

class SMDS_VtkCellIteratorToUNV: public SMDS_VtkCellIterator
{
public:
  SMDS_VtkCellIteratorToUNV(SMDS_Mesh* mesh, int vtkCellId, SMDSAbs_EntityType aType);
  virtual ~SMDS_VtkCellIteratorToUNV();
};

class SMDS_VtkCellIteratorPolyH: public SMDS_VtkCellIterator
{
public:
  SMDS_VtkCellIteratorPolyH(SMDS_Mesh* mesh, int vtkCellId, SMDSAbs_EntityType aType);
  virtual ~SMDS_VtkCellIteratorPolyH();
  virtual bool more();
protected:
  int _nbNodesInFaces;
};

#endif
