using namespace std;
//=============================================================================
// File      : SMESH_2D_Algo.cxx
// Created   : sam mai 18 09:23:44 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================
using namespace std;

#include "SMESH_2D_Algo.hxx"
#include "SMESH_Gen.hxx"

#include "utilities.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_2D_Algo::SMESH_2D_Algo(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_Algo(hypId, studyId, gen)
{
//   _compatibleHypothesis.push_back("hypothese_2D_bidon");
  _type = ALGO_2D;
  gen->_map2D_Algo[hypId] = this;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_2D_Algo::~SMESH_2D_Algo()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & SMESH_2D_Algo::SaveTo(ostream & save)
{
  return save << this;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & SMESH_2D_Algo::LoadFrom(istream & load)
{
  return load >> (*this);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream& operator << (ostream & save, SMESH_2D_Algo & hyp)
{
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream& operator >> (istream & load, SMESH_2D_Algo & hyp)
{
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

int SMESH_2D_Algo::NumberOfWires(const TopoDS_Shape& S)
{
  int i = 0;
  for (TopExp_Explorer exp(S,TopAbs_WIRE); exp.More(); exp.Next())
    i++;
  return i;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

int SMESH_2D_Algo::NumberOfPoints(SMESH_Mesh& aMesh, const TopoDS_Wire& W)
{
  int nbPoints = 0;
  for (TopExp_Explorer exp(W,TopAbs_EDGE); exp.More(); exp.Next())
    {
      const TopoDS_Edge& E = TopoDS::Edge(exp.Current());
      int nb = aMesh.GetSubMesh(E)->GetSubMeshDS()->NbNodes();
      //SCRUTE(nb);
      nbPoints += nb +1; // internal points plus 1 vertex of 2 (last point ?)
    }
  //SCRUTE(nbPoints);
  return nbPoints;
}
