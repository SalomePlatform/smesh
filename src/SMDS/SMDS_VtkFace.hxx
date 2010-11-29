#ifndef _SMDS_VTKFACE_HXX_
#define _SMDS_VTKFACE_HXX_

#include "SMESH_SMDS.hxx"

#include "SMDS_MeshFace.hxx"
#include <vtkUnstructuredGrid.h>
#include <vector>

class SMDS_EXPORT SMDS_VtkFace: public SMDS_MeshFace
{
public:
  SMDS_VtkFace();
  SMDS_VtkFace(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh);
  ~SMDS_VtkFace();
  void init(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh);
  void initPoly(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh);
  bool ChangeNodes(const SMDS_MeshNode* nodes[], const int nbNodes);
  void ChangeApex(const SMDS_MeshNode* node); // to use only for tmp triangles
  void Print(std::ostream & OS) const;
  int NbEdges() const;
  int NbFaces() const;
  int NbNodes() const;

  virtual vtkIdType GetVtkType() const;
  virtual SMDSAbs_EntityType GetEntityType() const;
  virtual const SMDS_MeshNode* GetNode(const int ind) const;
  virtual bool IsQuadratic() const;
  virtual bool IsPoly() const;
  virtual bool IsMediumNode(const SMDS_MeshNode* node) const;

  virtual SMDS_ElemIteratorPtr elementsIterator(SMDSAbs_ElementType type) const;
  virtual SMDS_ElemIteratorPtr nodesIteratorToUNV() const;
  virtual SMDS_ElemIteratorPtr interlacedNodesElemIterator() const;
protected:
};

#endif
