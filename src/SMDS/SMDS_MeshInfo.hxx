// File      : SMDS_MeshInfo.hxx
// Created   : Mon Sep 24 18:32:41 2007
// Author    : Edward AGAPOV (eap)


#ifndef SMDS_MeshInfo_HeaderFile
#define SMDS_MeshInfo_HeaderFile

#include "SMDS_MeshElement.hxx"

class SMDS_MeshInfo
{
public:

  inline SMDS_MeshInfo();

  int NbNodes() const { return myNbNodes; }

  inline int NbEdges      (SMDSAbs_ElementOrder order = ORDER_ANY) const;
  inline int NbFaces      (SMDSAbs_ElementOrder order = ORDER_ANY) const;
  inline int NbTriangles  (SMDSAbs_ElementOrder order = ORDER_ANY) const;
  inline int NbQuadrangles(SMDSAbs_ElementOrder order = ORDER_ANY) const;
  int NbPolygons() const { return myNbPolygons; }

  inline int NbVolumes (SMDSAbs_ElementOrder order = ORDER_ANY) const;
  inline int NbTetras  (SMDSAbs_ElementOrder order = ORDER_ANY) const;
  inline int NbHexas   (SMDSAbs_ElementOrder order = ORDER_ANY) const;
  inline int NbPyramids(SMDSAbs_ElementOrder order = ORDER_ANY) const;
  inline int NbPrisms  (SMDSAbs_ElementOrder order = ORDER_ANY) const;
  int NbPolyhedrons() const { return myNbPolyhedrons; }

private:
  friend class SMDS_Mesh;

  inline void RemoveEdge(const SMDS_MeshElement* el);
  inline void RemoveFace(const SMDS_MeshElement* el);
  inline void RemoveVolume(const SMDS_MeshElement* el);

  int myNbNodes;

  int myNbEdges      , myNbQuadEdges      ;
  int myNbTriangles  , myNbQuadTriangles  ;
  int myNbQuadrangles, myNbQuadQuadrangles;
  int myNbPolygons;

  int myNbTetras  , myNbQuadTetras  ;
  int myNbHexas   , myNbQuadHexas   ;
  int myNbPyramids, myNbQuadPyramids;
  int myNbPrisms  , myNbQuadPrisms  ;
  int myNbPolyhedrons;
  
};

inline SMDS_MeshInfo::SMDS_MeshInfo():
  myNbNodes(0),
  myNbEdges      (0), myNbQuadEdges      (0),
  myNbTriangles  (0), myNbQuadTriangles  (0),
  myNbQuadrangles(0), myNbQuadQuadrangles(0),
  myNbPolygons(0),
  myNbTetras  (0), myNbQuadTetras  (0),
  myNbHexas   (0), myNbQuadHexas   (0),
  myNbPyramids(0), myNbQuadPyramids(0),
  myNbPrisms  (0), myNbQuadPrisms  (0),
  myNbPolyhedrons(0)
{}

inline int // NbEdges
SMDS_MeshInfo::NbEdges      (SMDSAbs_ElementOrder order) const
{ return order == ORDER_ANY ? myNbEdges+myNbQuadEdges : order == ORDER_LINEAR ? myNbEdges : myNbQuadEdges; }

inline int // NbFaces
SMDS_MeshInfo::NbFaces      (SMDSAbs_ElementOrder order) const
{ return NbTriangles(order)+NbQuadrangles(order)+(order == ORDER_QUADRATIC ? 0 : myNbPolygons); }

inline int // NbTriangles
SMDS_MeshInfo::NbTriangles  (SMDSAbs_ElementOrder order) const
{ return order == ORDER_ANY ? myNbTriangles+myNbQuadTriangles : order == ORDER_LINEAR ? myNbTriangles : myNbQuadTriangles; }

inline int // NbQuadrangles
SMDS_MeshInfo::NbQuadrangles(SMDSAbs_ElementOrder order) const
{ return order == ORDER_ANY ? myNbQuadrangles+myNbQuadQuadrangles : order == ORDER_LINEAR ? myNbQuadrangles : myNbQuadQuadrangles; }

inline int // NbVolumes
SMDS_MeshInfo::NbVolumes (SMDSAbs_ElementOrder order) const
{ return NbTetras(order) + NbHexas(order) + NbPyramids(order) + NbPrisms(order) + (order == ORDER_QUADRATIC ? 0 : myNbPolyhedrons); }

inline int // NbTetras
SMDS_MeshInfo::NbTetras  (SMDSAbs_ElementOrder order) const
{ return order == ORDER_ANY ? myNbTetras+myNbQuadTetras : order == ORDER_LINEAR ? myNbTetras : myNbQuadTetras; }

inline int // NbHexas
SMDS_MeshInfo::NbHexas   (SMDSAbs_ElementOrder order) const
{ return order == ORDER_ANY ? myNbHexas+myNbQuadHexas : order == ORDER_LINEAR ? myNbHexas : myNbQuadHexas; }

inline int // NbPyramids
SMDS_MeshInfo::NbPyramids(SMDSAbs_ElementOrder order) const
{ return order == ORDER_ANY ? myNbPyramids+myNbQuadPyramids : order == ORDER_LINEAR ? myNbPyramids : myNbQuadPyramids; }

inline int // NbPrisms
SMDS_MeshInfo::NbPrisms  (SMDSAbs_ElementOrder order) const
{ return order == ORDER_ANY ? myNbPrisms+myNbQuadPrisms : order == ORDER_LINEAR ? myNbPrisms : myNbQuadPrisms; }

// RemoveEdge
inline void SMDS_MeshInfo::RemoveEdge(const SMDS_MeshElement* el)
{
  if ( el->IsQuadratic() ) --myNbQuadEdges; else --myNbEdges;
}

// RemoveFace
inline void SMDS_MeshInfo::RemoveFace(const SMDS_MeshElement* el)
{
  int nbnode = el->NbNodes();
  if ( el->IsPoly() )   --myNbPolygons;
  else if (nbnode == 3) --myNbTriangles;
  else if (nbnode == 4) --myNbQuadrangles;
  else if (nbnode == 6) --myNbQuadTriangles;
  else if (nbnode == 8) --myNbQuadQuadrangles;
}

// RemoveVolume
inline void SMDS_MeshInfo::RemoveVolume(const SMDS_MeshElement* el)
{
  int nbnode = el->NbNodes();
  if ( el->IsPoly() )    --myNbPolyhedrons;
  else if (nbnode == 4)  --myNbTetras;  
  else if (nbnode == 5)  --myNbPyramids;
  else if (nbnode == 6)  --myNbPrisms;
  else if (nbnode == 8)  --myNbHexas;
  else if (nbnode == 10) --myNbQuadTetras;  
  else if (nbnode == 13) --myNbQuadPyramids;
  else if (nbnode == 15) --myNbQuadPrisms;  
  else if (nbnode == 20) --myNbQuadHexas;   
}

#endif
