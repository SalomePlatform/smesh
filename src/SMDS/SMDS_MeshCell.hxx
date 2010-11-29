#ifndef _SMDS_MESHCELL_HXX_
#define _SMDS_MESHCELL_HXX_

#include "SMDS_MeshElement.hxx"

/*!
 * \brief Base class for all cells
 */

class SMDS_EXPORT SMDS_MeshCell: public SMDS_MeshElement
{
public:
  SMDS_MeshCell();
  virtual ~SMDS_MeshCell();

  virtual bool ChangeNodes(const SMDS_MeshNode* nodes[], const int nbNodes)= 0;
  virtual bool vtkOrder(const SMDS_MeshNode* nodes[], const int nbNodes) {return true; };

  static int nbCells;

protected:
  inline void exchange(const SMDS_MeshNode* nodes[],int a, int b)
  {
    const SMDS_MeshNode* noda = nodes[a];
    nodes[a] = nodes[b];
    nodes[b] = noda;
  }
};

#endif
