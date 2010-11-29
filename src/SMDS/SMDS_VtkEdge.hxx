#ifndef _SMDS_VTKEDGE_HXX_
#define _SMDS_VTKEDGE_HXX_

#include "SMESH_SMDS.hxx"

#include "SMDS_MeshEdge.hxx"
#include <vtkUnstructuredGrid.h>
#include <vector>

class SMDS_EXPORT SMDS_VtkEdge: public SMDS_MeshEdge
{

public:
  SMDS_VtkEdge();
  SMDS_VtkEdge(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh);
  ~SMDS_VtkEdge();
  void init(std::vector<vtkIdType> nodeIds, SMDS_Mesh* mesh);
  bool ChangeNodes(const SMDS_MeshNode * node1, const SMDS_MeshNode * node2);
  virtual bool ChangeNodes(const SMDS_MeshNode* nodes[], const int nbNodes);
  virtual bool IsMediumNode(const SMDS_MeshNode* node) const;

  void Print(std::ostream & OS) const;
  int NbNodes() const;
  int NbEdges() const;

  virtual vtkIdType GetVtkType() const;
  virtual SMDSAbs_EntityType GetEntityType() const;
  virtual const SMDS_MeshNode* GetNode(const int ind) const;
  virtual bool IsQuadratic() const;

  virtual SMDS_ElemIteratorPtr elementsIterator(SMDSAbs_ElementType type) const;
  virtual SMDS_ElemIteratorPtr nodesIteratorToUNV() const;
  virtual SMDS_ElemIteratorPtr interlacedNodesElemIterator() const;
protected:
};
#endif
