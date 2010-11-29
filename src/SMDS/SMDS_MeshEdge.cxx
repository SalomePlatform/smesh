#include "SMDS_MeshEdge.hxx"

SMDSAbs_ElementType SMDS_MeshEdge::GetType() const
{
	return SMDSAbs_Edge;
}

vtkIdType SMDS_MeshEdge::GetVtkType() const
{
  return VTK_POLY_VERTEX; // --- must be reimplemented in derived classes
}
