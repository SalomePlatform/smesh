//  SMESH StdMeshers_Penta_3D implementaion of SMESH idl descriptions
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : StdMeshers_Penta_3D.cxx
//  Module : SMESH

using namespace std;

#include <StdMeshers_Penta_3D.hxx>

#include <stdio.h>

#include <algorithm>

#include "utilities.h"
#include "Utils_ExceptHandlers.hxx"

#include <TopAbs_ShapeEnum.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <SMESH_Mesh.hxx>
#include <SMESH_subMesh.hxx>
#include <SMESHDS_SubMesh.hxx>

#include <SMDS_MeshElement.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <gp_Pnt.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS_Shell.hxx>

typedef map < int, int, less<int> >::iterator   \
  StdMeshers_IteratorOfDataMapOfIntegerInteger;

//=======================================================================
//
//           StdMeshers_Penta_3D 
//
//=======================================================================
//function : StdMeshers_Penta_3D
//purpose  : 
//=======================================================================
StdMeshers_Penta_3D::StdMeshers_Penta_3D()
: myErrorStatus(1)
{
  myTol3D=0.1;
}
//=======================================================================
//function : Compute
//purpose  : 
//=======================================================================
bool StdMeshers_Penta_3D::Compute(SMESH_Mesh& aMesh, 
				  const TopoDS_Shape& aShape)
{
  myErrorStatus=0;
  //
  bool bOK=false;
  //
  myShape=aShape;
  SetMesh(aMesh);
  //
  CheckData();
  if (myErrorStatus){
    return bOK;
  }
  //
  MakeBlock();
    if (myErrorStatus){
    return bOK;
  }
  //
  MakeNodes();
  if (myErrorStatus){
    return bOK;
  }
  //
  MakeConnectingMap();
  //
  ClearMeshOnFxy1();
  if (myErrorStatus) {
    return bOK;
  }
  //
  MakeMeshOnFxy1();
  if (myErrorStatus) {
    return bOK;
  }
  //
  MakeVolumeMesh();
  //
  return !bOK;
}
//=======================================================================
//function : MakeNodes
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::MakeNodes()
{
  myErrorStatus=0;
  //
  const int aNbSIDs=9;
  int i, j, k, ij, iNbN, aNodeID, aSize, iErr;
  double aX, aY, aZ;
  SMESH_Block::TShapeID aSID, aSIDs[aNbSIDs]={
    SMESH_Block::ID_V000, SMESH_Block::ID_V100, 
    SMESH_Block::ID_V110, SMESH_Block::ID_V010,
    SMESH_Block::ID_Ex00, SMESH_Block::ID_E1y0, 
    SMESH_Block::ID_Ex10, SMESH_Block::ID_E0y0,
    SMESH_Block::ID_Fxy0
  }; 
  //
  SMESH_Mesh* pMesh=GetMesh();
  //
  // 1. Define the sizes of mesh
  //
  // 1.1 Horizontal size
  myJSize=0;
  for (i=0; i<aNbSIDs; ++i) {
    const TopoDS_Shape& aS=myBlock.Shape(aSIDs[i]);
    SMESH_subMesh *aSubMesh = pMesh->GetSubMeshContaining(aS);
    ASSERT(aSubMesh);
    SMESHDS_SubMesh *aSM=aSubMesh->GetSubMeshDS();
    iNbN=aSM->NbNodes();
    myJSize+=iNbN;
  }
  //printf("***  Horizontal: number of nodes summary=%d\n", myJSize);
  //
  // 1.2 Vertical size
  myISize=2;
  {
    const TopoDS_Shape& aS=myBlock.Shape(SMESH_Block::ID_E00z);
    SMESH_subMesh *aSubMesh = pMesh->GetSubMeshContaining(aS);
    ASSERT(aSubMesh);
    SMESHDS_SubMesh *aSM=aSubMesh->GetSubMeshDS();
    iNbN=aSM->NbNodes();
    myISize+=iNbN;
  }
  //printf("***  Vertical: number of nodes on edges and vertices=%d\n", myISize);
  //
  aSize=myISize*myJSize;
  myTNodes.resize(aSize);
  //
  StdMeshers_TNode aTNode;
  gp_XYZ aCoords;
  gp_Pnt aP3D;
  //
  // 2. Fill the repers on base face (Z=0)
  i=0; j=0;
  // vertices
  for (k=0; k<aNbSIDs; ++k) {
    aSID=aSIDs[k];
    const TopoDS_Shape& aS=myBlock.Shape(aSID);
    SMDS_NodeIteratorPtr ite =pMesh->GetSubMeshContaining(aS)->GetSubMeshDS()->GetNodes();
    while(ite->more()) {
      const SMDS_MeshNode* aNode = ite->next();
      aNodeID=aNode->GetID();
      //
      aTNode.SetNode(aNode);
      aTNode.SetShapeSupportID(aSID);
      aTNode.SetBaseNodeID(aNodeID);
      //
      switch (aSID){
        case SMESH_Block::ID_V000:
	  aCoords.SetCoord(0., 0., 0.);
	  break;
	case SMESH_Block::ID_V100:
	  aCoords.SetCoord(1., 0., 0.);
	  break; 
	case SMESH_Block::ID_V110:
	  aCoords.SetCoord(1., 1., 0.);
	  break; 
	case SMESH_Block::ID_V010:
	  aCoords.SetCoord(0., 1., 0.);
	  break;
	case SMESH_Block::ID_Ex00:  
	case SMESH_Block::ID_E1y0:
	case SMESH_Block::ID_Ex10:
	case SMESH_Block::ID_E0y0:
	case SMESH_Block::ID_Fxy0:{
	  aX=aNode->X();
	  aY=aNode->Y();
	  aZ=aNode->Z();
	  aP3D.SetCoord(aX, aY, aZ);
	  myBlock.ComputeParameters(aP3D, aS, aCoords);
	  iErr=myBlock.ErrorStatus();
	  if (iErr) {
            MESSAGE("StdMeshers_Penta_3D::MakeNodes()," <<
                    "SMESHBlock: ComputeParameters operation failed");
	    myErrorStatus=101; // SMESHBlock: ComputeParameters operation failed
	    return;
	  }
	}
	  break;
	default:
	  break;
      }
      aTNode.SetNormCoord(aCoords);
      ij=i*myJSize+j;
      myTNodes[ij]=aTNode;
      ++j;
    }
  }
  /*
  //DEB
  {
    int iShapeSupportID, iBaseNodeID;
    //
    //printf("\n\n*** Base Face\n");
    i=0;
    for (j=0; j<myJSize; ++j) {
      ij=i*myJSize+j;
      const StdMeshers_TNode& aTNode=myTNodes[ij];
      iShapeSupportID=aTNode.ShapeSupportID();
      iBaseNodeID=aTNode.BaseNodeID();
      const gp_XYZ& aXYZ=aTNode.NormCoord();
      printf("*** j:%d bID#%d iSS:%d { %lf %lf %lf }\n",
	     j,  iBaseNodeID, iShapeSupportID, aXYZ.X(),  aXYZ.Y(), aXYZ.Z());
    }
  }
  */
  //DEB
  //return; //zz
  //
  // 3. Finding of Z-layers
  vector<double> aZL(myISize);
  vector<double>::iterator aItZL1, aItZL2 ;
  //
  const TopoDS_Shape& aE00z=myBlock.Shape(SMESH_Block::ID_E00z);
  SMDS_NodeIteratorPtr aItaE00z =
    pMesh->GetSubMeshContaining(aE00z)->GetSubMeshDS()->GetNodes();
  //
  aZL[0]=0.;
  i=1;
  while (aItaE00z->more()) {
    const SMDS_MeshNode* aNode=aItaE00z->next();
    aX=aNode->X(); aY=aNode->Y(); aZ=aNode->Z();
    aP3D.SetCoord(aX, aY, aZ);
    myBlock.ComputeParameters(aP3D, aE00z, aCoords);
    iErr=myBlock.ErrorStatus();
    if (iErr) {
      MESSAGE("StdMeshers_Penta_3D::MakeNodes()," <<
              "SMESHBlock: ComputeParameters operation failed");
      myErrorStatus=101; // SMESHBlock: ComputeParameters operation failed
      return;
    }
    aZL[i]=aCoords.Z();
    ++i;
  }
  aZL[i]=1.;
  //
  aItZL1=aZL.begin();
  aItZL2=aZL.end();
  //
  // Sorting the layers
  sort(aItZL1, aItZL2);
  //DEB
  /*
  printf("** \n\n Layers begin\n");
  for(i=0, aItZL=aItZL1; aItZL!=aItZL2; ++aItZL, ++i) {
    printf(" #%d : %lf\n", i, *aItZL);
  } 
  printf("** Layers end\n");
  */
  //DEB
  //
  //
  // 4. Fill the rest repers
  bool bIsUpperLayer;
  int iBNID;
  SMESH_Block::TShapeID aSSID, aBNSSID;
  StdMeshers_TNode aTN;
  //
  for (j=0; j<myJSize; ++j) {
    for (i=1; i<myISize; ++i) {
      //
      // base node info
      const StdMeshers_TNode& aBN=myTNodes[j];
      aBNSSID=(SMESH_Block::TShapeID)aBN.ShapeSupportID();
      iBNID=aBN.BaseNodeID();
      const gp_XYZ& aBNXYZ=aBN.NormCoord();
      //
      // fill current node info
      //   -index in aTNodes
      ij=i*myJSize+j; 
      //   -normalized coordinates  
      aX=aBNXYZ.X();  
      aY=aBNXYZ.Y();
      aZ=aZL[i];
      aCoords.SetCoord(aX, aY, aZ); 
      //
      //   suporting shape ID
      bIsUpperLayer=(i==(myISize-1));
      ShapeSupportID(bIsUpperLayer, aBNSSID, aSSID);
      if (myErrorStatus) {
        MESSAGE("StdMeshers_Penta_3D::MakeNodes() ");
	return;
      }
      //
      aTN.SetShapeSupportID(aSSID);
      aTN.SetNormCoord(aCoords);
      aTN.SetBaseNodeID(iBNID);
      //
      if (aSSID!=SMESH_Block::ID_NONE){
	// try to find the node
	const TopoDS_Shape& aS=myBlock.Shape((int)aSSID);
	FindNodeOnShape(aS, aCoords, aTN);
      }
      else{
	// create node and get it id
	CreateNode (bIsUpperLayer, aCoords, aTN);
      }
      if (myErrorStatus) {
        MESSAGE("StdMeshers_Penta_3D::MakeNodes() ");
	return;
      }
      //
      myTNodes[ij]=aTN;
    }
  }
  //DEB
  /*
  {
    int iSSID, iBNID, aID;
    //
    for (i=0; i<myISize; ++i) {
      printf(" Layer# %d\n", i);
      for (j=0; j<myJSize; ++j) {
	ij=i*myJSize+j; 
	const StdMeshers_TNode& aTN=myTNodes[ij];
	//const StdMeshers_TNode& aTN=aTNodes[ij];
	const gp_XYZ& aXYZ=aTN.NormCoord();
	iSSID=aTN.ShapeSupportID();
	iBNID=aTN.BaseNodeID();
	//
	const SMDS_MeshNode* aNode=aTN.Node();
	aID=aNode->GetID(); 
	aX=aNode->X();
	aY=aNode->Y();
	aZ=aNode->Z();
	printf("*** j:%d BNID#%d iSSID:%d ID:%d { %lf %lf %lf },  { %lf %lf %lf }\n",
	       j,  iBNID, iSSID, aID, aXYZ.X(),  aXYZ.Y(), aXYZ.Z(), aX, aY, aZ);
      }
    }
  }
  */
  //DEB t
}
//=======================================================================
//function : FindNodeOnShape
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::FindNodeOnShape(const TopoDS_Shape& aS,
					  const gp_XYZ& aParams,
					  StdMeshers_TNode& aTN)
{
  myErrorStatus=0;
  //
  double aX, aY, aZ, aD, aTol2;
  gp_Pnt aP1, aP2;
  //
  SMESH_Mesh* pMesh=GetMesh();
  aTol2=myTol3D*myTol3D;
  SMDS_MeshNode* pNode=NULL;
  //
  myBlock.Point(aParams, aS, aP1);
  //
  SMDS_NodeIteratorPtr ite=
    pMesh->GetSubMeshContaining(aS)->GetSubMeshDS()->GetNodes();
  while(ite->more()) {
    const SMDS_MeshNode* aNode = ite->next();
    aX=aNode->X();
    aY=aNode->Y();
    aZ=aNode->Z();
    aP2.SetCoord(aX, aY, aZ);
    aD=(double)aP1.SquareDistance(aP2);
    //printf("** D=%lf ", aD, aTol2);
    if (aD<aTol2) {
      pNode=(SMDS_MeshNode*)aNode;
      aTN.SetNode(pNode);
      //printf(" Ok\n");
      return; 
    }
  }
  //
  //printf(" KO\n");
  aTN.SetNode(pNode);
  MESSAGE("StdMeshers_Penta_3D::FindNodeOnShape(), can not find the node");
  myErrorStatus=11; // can not find the node;
}

//=======================================================================
//function : MakeVolumeMesh
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::MakeVolumeMesh()
{
  myErrorStatus=0;
  //
  int i, j, ij, ik, i1, i2, aSSID; 
  //
  TopoDS_Shell aShell;
  TopExp_Explorer aExp;
  //
  SMESH_Mesh*   pMesh =GetMesh();
  SMESHDS_Mesh* meshDS=pMesh->GetMeshDS();
  //
  aExp.Init(myShape, TopAbs_SHELL);
  for (; aExp.More(); aExp.Next()){
    aShell=TopoDS::Shell(aExp.Current());
    break;
  }
  //
  // 1. Set Node In Volume
  ik=myISize-1;
  for (i=1; i<ik; ++i){
    for (j=0; j<myJSize; ++j){
      ij=i*myJSize+j;
      const StdMeshers_TNode& aTN=myTNodes[ij];
      aSSID=aTN.ShapeSupportID();
      if (aSSID==SMESH_Block::ID_NONE) {
	SMDS_MeshNode* aNode=(SMDS_MeshNode*)aTN.Node();
	meshDS->SetNodeInVolume(aNode, aShell);
      }
    }
  }
  //
  // 2. Make pentahedrons
  int aID0, k , aJ[3];
  vector<const SMDS_MeshNode*> aN;
  //
  SMDS_ElemIteratorPtr itf, aItNodes;
  //
  const TopoDS_Face& aFxy0=
    TopoDS::Face(myBlock.Shape(SMESH_Block::ID_Fxy0));
  SMESH_subMesh *aSubMesh0 = pMesh->GetSubMeshContaining(aFxy0);
  SMESHDS_SubMesh *aSM0=aSubMesh0->GetSubMeshDS();
  //
  itf=aSM0->GetElements();
  while(itf->more()) {
    const SMDS_MeshElement* pE0=itf->next();
    //
    int nbFaceNodes = pE0->NbNodes();
    if ( aN.size() < nbFaceNodes * 2 )
      aN.resize( nbFaceNodes * 2 );
    //
    k=0;
    aItNodes=pE0->nodesIterator();
    while (aItNodes->more()) {
      const SMDS_MeshElement* pNode=aItNodes->next();
      aID0=pNode->GetID();
      aJ[k]=GetIndexOnLayer(aID0);
      if (myErrorStatus) {
        MESSAGE("StdMeshers_Penta_3D::MakeVolumeMesh");
	return;
      }
      //
      ++k;
    }
    //
    for (i=0; i<ik; ++i){
      i1=i;
      i2=i+1;
      for(j=0; j<nbFaceNodes; ++j) {
	ij=i1*myJSize+aJ[j];
	const StdMeshers_TNode& aTN1=myTNodes[ij];
	const SMDS_MeshNode* aN1=aTN1.Node();
	aN[j]=aN1;
	//
	ij=i2*myJSize+aJ[j];
	const StdMeshers_TNode& aTN2=myTNodes[ij];
	const SMDS_MeshNode* aN2=aTN2.Node();
	aN[j+nbFaceNodes]=aN2;
      }
      //
      SMDS_MeshVolume* aV = 0;
      switch ( nbFaceNodes ) {
      case 3:
	aV = meshDS->AddVolume(aN[0], aN[1], aN[2],
                               aN[3], aN[4], aN[5]);
        break;
      case 4:
	aV = meshDS->AddVolume(aN[0], aN[1], aN[2], aN[3],
                               aN[4], aN[5], aN[6], aN[7]);
        break;
      default:
        continue;
      }
      meshDS->SetMeshElementOnShape(aV, aShell);
    }
  }
}

//=======================================================================
//function : MakeMeshOnFxy1
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::MakeMeshOnFxy1()
{
  myErrorStatus=0;
  //
  int aID0, aJ, aLevel, ij, aNbNodes, k;
  //
  SMDS_NodeIteratorPtr itn;
  SMDS_ElemIteratorPtr itf, aItNodes;
  SMDSAbs_ElementType aElementType;
  //
  const TopoDS_Face& aFxy0=
    TopoDS::Face(myBlock.Shape(SMESH_Block::ID_Fxy0));
  const TopoDS_Face& aFxy1=
    TopoDS::Face(myBlock.Shape(SMESH_Block::ID_Fxy1));
  //
  SMESH_Mesh* pMesh=GetMesh();
  SMESHDS_Mesh * meshDS = pMesh->GetMeshDS();
  //
  SMESH_subMesh *aSubMesh0 = pMesh->GetSubMeshContaining(aFxy0);
  SMESHDS_SubMesh *aSM0=aSubMesh0->GetSubMeshDS();
  //
  // set nodes on aFxy1
  aLevel=myISize-1;
  itn=aSM0->GetNodes();
  aNbNodes=aSM0->NbNodes();
  //printf("** aNbNodes=%d\n", aNbNodes);
  while(itn->more()) {
    const SMDS_MeshNode* aN0=itn->next();
    aID0=aN0->GetID();
    aJ=GetIndexOnLayer(aID0);
    if (myErrorStatus) {
      MESSAGE("StdMeshers_Penta_3D::MakeMeshOnFxy1() ");
      return;
    }
    //
    ij=aLevel*myJSize+aJ;
    const StdMeshers_TNode& aTN1=myTNodes[ij];
    SMDS_MeshNode* aN1=(SMDS_MeshNode*)aTN1.Node();
    //
    meshDS->SetNodeOnFace(aN1, aFxy1);
  }
  //
  // set elements on aFxy1
  vector<const SMDS_MeshNode*> aNodes1;
  //
  itf=aSM0->GetElements();
  while(itf->more()) {
    const SMDS_MeshElement * pE0=itf->next();
    aElementType=pE0->GetType();
    if (!aElementType==SMDSAbs_Face) {
      continue;
    }
    aNbNodes=pE0->NbNodes();
//     if (aNbNodes!=3) {
//       continue;
//     }
    if ( aNodes1.size() < aNbNodes )
      aNodes1.resize( aNbNodes );
    //
    k=aNbNodes-1; // reverse a face
    aItNodes=pE0->nodesIterator();
    while (aItNodes->more()) {
      const SMDS_MeshElement* pNode=aItNodes->next();
      aID0=pNode->GetID();
      aJ=GetIndexOnLayer(aID0);
      if (myErrorStatus) {
        MESSAGE("StdMeshers_Penta_3D::MakeMeshOnFxy1() ");
	return;
      }
      //
      ij=aLevel*myJSize+aJ;
      const StdMeshers_TNode& aTN1=myTNodes[ij];
      const SMDS_MeshNode* aN1=aTN1.Node();
      aNodes1[k]=aN1;
      --k;
    }
    SMDS_MeshFace * face = 0;
    switch ( aNbNodes ) {
    case 3:
      face = meshDS->AddFace(aNodes1[0], aNodes1[1], aNodes1[2]);
      break;
    case 4:
      face = meshDS->AddFace(aNodes1[0], aNodes1[1], aNodes1[2], aNodes1[3]);
      break;
    default:
      continue;
    }
    meshDS->SetMeshElementOnShape(face, aFxy1);
  }
}
//=======================================================================
//function : ClearMeshOnFxy1
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::ClearMeshOnFxy1()
{
  myErrorStatus=0;
  //
  SMESH_subMesh* aSubMesh;
  SMESH_Mesh* pMesh=GetMesh();
  //
  const TopoDS_Shape& aFxy1=myBlock.Shape(SMESH_Block::ID_Fxy1);
  aSubMesh = pMesh->GetSubMeshContaining(aFxy1);
  if (aSubMesh)
    aSubMesh->ComputeStateEngine( SMESH_subMesh::CLEAN );
}

//=======================================================================
//function : GetIndexOnLayer
//purpose  : 
//=======================================================================
int StdMeshers_Penta_3D::GetIndexOnLayer(const int aID)
{
  myErrorStatus=0;
  //
  int j=-1;
  StdMeshers_IteratorOfDataMapOfIntegerInteger aMapIt;
  //
  aMapIt=myConnectingMap.find(aID);
  if (aMapIt==myConnectingMap.end()) {
    myErrorStatus=200;
    return j;
  }
  j=(*aMapIt).second;
  return j;
}
//=======================================================================
//function : MakeConnectingMap
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::MakeConnectingMap()
{
  int j, aBNID;
  //
  for (j=0; j<myJSize; ++j) {
    const StdMeshers_TNode& aBN=myTNodes[j];
    aBNID=aBN.BaseNodeID();
    myConnectingMap[aBNID]=j;
  }
}
//=======================================================================
//function : CreateNode
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::CreateNode(const bool bIsUpperLayer,
				     const gp_XYZ& aParams,
				     StdMeshers_TNode& aTN)
{
  myErrorStatus=0;
  //
  int iErr;
  double aX, aY, aZ;
  //
  gp_Pnt aP;
  //
  SMDS_MeshNode* pNode=NULL; 
  aTN.SetNode(pNode);  
  //
  if (bIsUpperLayer) {
    // point on face Fxy1
    const TopoDS_Shape& aS=myBlock.Shape(SMESH_Block::ID_Fxy1);
    myBlock.Point(aParams, aS, aP);
  }
  else {
    // point inside solid
    myBlock.Point(aParams, aP);
  }
  //
  iErr=myBlock.ErrorStatus();
  if (iErr) {
    myErrorStatus=12; // can not find the node point;
    return;
  }
  //
  aX=aP.X(); aY=aP.Y(); aZ=aP.Z(); 
  //
  SMESH_Mesh* pMesh=GetMesh();
  SMESHDS_Mesh* pMeshDS=pMesh->GetMeshDS();
  //
  pNode = pMeshDS->AddNode(aX, aY, aZ);
  aTN.SetNode(pNode);
}
//=======================================================================
//function : ShapeSupportID
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::ShapeSupportID(const bool bIsUpperLayer,
					 const SMESH_Block::TShapeID aBNSSID,
					 SMESH_Block::TShapeID& aSSID)
{
  myErrorStatus=0;
  //
  switch (aBNSSID) {
    case SMESH_Block::ID_V000:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_V001 : SMESH_Block::ID_E00z;
      break;
    case SMESH_Block::ID_V100:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_V101 : SMESH_Block::ID_E10z;
      break; 
    case SMESH_Block::ID_V110:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_V111 : SMESH_Block::ID_E11z;
      break;
    case SMESH_Block::ID_V010:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_V011 : SMESH_Block::ID_E01z;
      break;
    case SMESH_Block::ID_Ex00:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_Ex01 : SMESH_Block::ID_Fx0z;
      break;
    case SMESH_Block::ID_Ex10:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_Ex11 : SMESH_Block::ID_Fx1z;
      break; 
    case SMESH_Block::ID_E0y0:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_E0y1 : SMESH_Block::ID_F0yz;
      break; 
    case SMESH_Block::ID_E1y0:
      aSSID=(bIsUpperLayer) ?  SMESH_Block::ID_E1y1 : SMESH_Block::ID_F1yz;
      break; 
    case SMESH_Block::ID_Fxy0:
      aSSID=SMESH_Block::ID_NONE;//(bIsUpperLayer) ?  Shape_ID_Fxy1 : Shape_ID_NONE;
      break;   
    default:
      aSSID=SMESH_Block::ID_NONE;
      myErrorStatus=10; // Can not find supporting shape ID
      break;
  }
  return;
}
//=======================================================================
//function : MakeBlock
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::MakeBlock()
{
  myErrorStatus=0;
  //
  bool bFound;
  int i, j, iNbEV, iNbE, iErr, iCnt, iNbNodes, iNbF;
  //
  TopoDS_Vertex aV000, aV001;
  TopoDS_Shape aFTr;
  TopTools_IndexedDataMapOfShapeListOfShape aMVES;
  TopTools_IndexedMapOfShape aME ,aMEV, aM;
  TopTools_ListIteratorOfListOfShape aIt;
  //
  TopExp::MapShapes(myShape, TopAbs_FACE, aM);
  //
  // 0. Find triangulated face aFTr
  SMDSAbs_ElementType aElementType;
  SMESH_Mesh* pMesh=GetMesh();
  //
  iCnt=0;
  iNbF=aM.Extent();
  for (i=1; i<=iNbF; ++i) {
    const TopoDS_Shape& aF=aM(i);
    SMESH_subMesh *aSubMesh = pMesh->GetSubMeshContaining(aF);
    ASSERT(aSubMesh);
    SMESHDS_SubMesh *aSM=aSubMesh->GetSubMeshDS();
    SMDS_ElemIteratorPtr itf=aSM->GetElements();
    while(itf->more()) {
      const SMDS_MeshElement * pElement=itf->next();
      aElementType=pElement->GetType();
      if (aElementType==SMDSAbs_Face) {
	iNbNodes=pElement->NbNodes();
	if (iNbNodes==3) {
	  aFTr=aF;
	  ++iCnt;
	  if (iCnt>1) {
            MESSAGE("StdMeshers_Penta_3D::MakeBlock() ");
	    myErrorStatus=5; // more than one face has triangulation
	    return;
	  }
	  break; // next face
	}
      }
    }
  }
  // 
  // 1. Vetrices V00, V001;
  //
  TopExp::MapShapes(aFTr, TopAbs_EDGE, aME);
  TopExp::MapShapesAndAncestors(myShape, TopAbs_VERTEX, TopAbs_EDGE, aMVES);
  //
  // 1.1 Base vertex V000
  iNbE=aME.Extent();
  if (iNbE!=4){
    MESSAGE("StdMeshers_Penta_3D::MakeBlock() ");
    myErrorStatus=7; // too few edges are in base face aFTr 
    return;
  }
  const TopoDS_Edge& aE1=TopoDS::Edge(aME(1));
  aV000=TopExp::FirstVertex(aE1);
  //
  const TopTools_ListOfShape& aLE=aMVES.FindFromKey(aV000);
  aIt.Initialize(aLE);
  for (; aIt.More(); aIt.Next()) {
    const TopoDS_Shape& aEx=aIt.Value();
    aMEV.Add(aEx);
  }
  iNbEV=aMEV.Extent();
  if (iNbEV!=3){
    MESSAGE("StdMeshers_Penta_3D::MakeBlock() ");
    myErrorStatus=7; // too few edges meet in base vertex 
    return;
  }
  //
  // 1.2 Vertex V001
  bFound=false;
  for (j=1; j<=iNbEV; ++j) {
    const TopoDS_Shape& aEx=aMEV(j);
    if (!aME.Contains(aEx)) {
      TopoDS_Vertex aV[2];
      //
      const TopoDS_Edge& aE=TopoDS::Edge(aEx);
      TopExp::Vertices(aE, aV[0], aV[1]);
      for (i=0; i<2; ++i) {
	if (!aV[i].IsSame(aV000)) {
	  aV001=aV[i];
	  bFound=!bFound;
	  break;
	}
      }
    }
  }
  //
  if (!bFound) {
    MESSAGE("StdMeshers_Penta_3D::MakeBlock() ");
    myErrorStatus=8; // can not find reper V001 
    return;
  }
  //DEB
  //gp_Pnt aP000, aP001;
  //
  //aP000=BRep_Tool::Pnt(TopoDS::Vertex(aV000));
  //printf("*** aP000 { %lf, %lf, %lf }\n", aP000.X(), aP000.Y(), aP000.Z());
  //aP001=BRep_Tool::Pnt(TopoDS::Vertex(aV001));
  //printf("*** aP001 { %lf, %lf, %lf }\n", aP001.X(), aP001.Y(), aP001.Z());
  //DEB
  //
  aME.Clear();
  TopExp::MapShapes(myShape, TopAbs_SHELL, aME);
  iNbE=aME.Extent();
  if (iNbE!=1) {
    MESSAGE("StdMeshers_Penta_3D::MakeBlock() ");
    myErrorStatus=9; // number of shells in source shape !=1 
    return;
  }
  //
  // 2. Load Block
  const TopoDS_Shell& aShell=TopoDS::Shell(aME(1));
  myBlock.Load(aShell, aV000, aV001);
  iErr=myBlock.ErrorStatus();
  if (iErr) {
    MESSAGE("StdMeshers_Penta_3D::MakeBlock() ");
    myErrorStatus=100; // SMESHBlock: Load operation failed
    return;
  }
}
//=======================================================================
//function : CheckData
//purpose  : 
//=======================================================================
void StdMeshers_Penta_3D::CheckData()
{
  myErrorStatus=0;
  //
  int i, iNb;
  int iNbEx[]={8, 12, 6};
  //
  TopAbs_ShapeEnum aST;
  TopAbs_ShapeEnum aSTEx[]={
    TopAbs_VERTEX, TopAbs_EDGE, TopAbs_FACE
  }; 
  TopTools_IndexedMapOfShape aM;
  //
  if (myShape.IsNull()){
    MESSAGE("StdMeshers_Penta_3D::CheckData() ");
    myErrorStatus=2; // null shape
    return;
  }
  //
  aST=myShape.ShapeType();
  if (!(aST==TopAbs_SOLID || aST==TopAbs_SHELL)) {
    MESSAGE("StdMeshers_Penta_3D::CheckData() ");
    myErrorStatus=3; // not compatible type of shape
    return;
  }
  //
  for (i=0; i<3; ++i) {
    aM.Clear();
    TopExp::MapShapes(myShape, aSTEx[i], aM);
    iNb=aM.Extent();
    if (iNb!=iNbEx[i]){
      MESSAGE("StdMeshers_Penta_3D::CheckData() ");
      myErrorStatus=4; // number of subshape is not compatible
      return;
    }
  }
}
//////////////////////////////////////////////////////////////////////////
//
//   StdMeshers_SMESHBlock
//
//
#include <TopTools_IndexedMapOfOrientedShape.hxx>
#include <TopoDS_Vertex.hxx>

//=======================================================================
//function : StdMeshers_SMESHBlock
//purpose  : 
//=======================================================================
StdMeshers_SMESHBlock::StdMeshers_SMESHBlock()
{
  myErrorStatus=1;
}
//=======================================================================
//function : ErrorStatus
//purpose  : 
//=======================================================================
int StdMeshers_SMESHBlock::ErrorStatus() const
{
  return myErrorStatus;
}
//=======================================================================
//function : Load
//purpose  : 
//=======================================================================
void StdMeshers_SMESHBlock::Load(const TopoDS_Shell& theShell)
{
  
  TopoDS_Vertex aV000, aV001;
  //
  Load(theShell, aV000, aV001);
}
//=======================================================================
//function : Load
//purpose  : 
//=======================================================================
void StdMeshers_SMESHBlock::Load(const TopoDS_Shell& theShell,
				 const TopoDS_Vertex& theV000,
				 const TopoDS_Vertex& theV001)
{
  myErrorStatus=0;
  //
  myShell=theShell;
  //
  bool bOk;
  //
  myShapeIDMap.Clear();  
  bOk=myTBlock.LoadBlockShapes(myShell, theV000, theV001, myShapeIDMap);
  if (!bOk) {
    myErrorStatus=2;
    return;
  }
}
//=======================================================================
//function : ComputeParameters
//purpose  : 
//=======================================================================
void StdMeshers_SMESHBlock::ComputeParameters(const gp_Pnt& thePnt, 
					      gp_XYZ& theXYZ)
{
  ComputeParameters(thePnt, myShell, theXYZ);
}
//=======================================================================
//function : ComputeParameters
//purpose  : 
//=======================================================================
void StdMeshers_SMESHBlock::ComputeParameters(const gp_Pnt& thePnt,
					      const TopoDS_Shape& theShape,
					      gp_XYZ& theXYZ)
{
  myErrorStatus=0;
  //
  int aID;
  bool bOk;
  //
  aID=ShapeID(theShape);
  if (myErrorStatus) {
    return;
  }
  bOk=myTBlock.ComputeParameters(thePnt, theXYZ, aID);
  if (!bOk) {
    myErrorStatus=4; // problems with computation Parameters 
    return;
  }
}
//=======================================================================
//function : Point
//purpose  : 
//=======================================================================
 void StdMeshers_SMESHBlock::Point(const gp_XYZ& theParams,
				   gp_Pnt& aP3D)
{
  TopoDS_Shape aS;
  //
  Point(theParams, aS, aP3D);
}
//=======================================================================
//function : Point
//purpose  : 
//=======================================================================
 void StdMeshers_SMESHBlock::Point(const gp_XYZ& theParams,
				   const TopoDS_Shape& theShape,
				   gp_Pnt& aP3D)
{
  myErrorStatus=0;
  //
  int aID;
  bool bOk=false;
  gp_XYZ aXYZ(99.,99.,99.);
  aP3D.SetXYZ(aXYZ);
  //
  if (theShape.IsNull()) {
    bOk=myTBlock.ShellPoint(theParams, aXYZ);
  }
  //
  else {
    aID=ShapeID(theShape);
    if (myErrorStatus) {
      return;
    }
    //
    if (SMESH_Block::IsVertexID(aID)) {
      bOk=myTBlock.VertexPoint(aID, aXYZ);
    }
    else if (SMESH_Block::IsEdgeID(aID)) {
      bOk=myTBlock.EdgePoint(aID, theParams, aXYZ);
    }
    //
    else if (SMESH_Block::IsFaceID(aID)) {
      bOk=myTBlock.FacePoint(aID, theParams, aXYZ);
    }
  }
  if (!bOk) {
    myErrorStatus=4; // problems with point computation 
    return;
  }
  aP3D.SetXYZ(aXYZ);
}
//=======================================================================
//function : ShapeID
//purpose  : 
//=======================================================================
int StdMeshers_SMESHBlock::ShapeID(const TopoDS_Shape& theShape)
{
  myErrorStatus=0;
  //
  int aID=-1;
  TopoDS_Shape aSF, aSR;
  //
  aSF=theShape;
  aSF.Orientation(TopAbs_FORWARD);
  aSR=theShape;
  aSR.Orientation(TopAbs_REVERSED);
  //
  if (myShapeIDMap.Contains(aSF)) {
    aID=myShapeIDMap.FindIndex(aSF);
    return aID;
  }
  if (myShapeIDMap.Contains(aSR)) {
    aID=myShapeIDMap.FindIndex(aSR);
    return aID;
  }
  myErrorStatus=2; // unknown shape;
  return aID;
}
//=======================================================================
//function : Shape
//purpose  : 
//=======================================================================
const TopoDS_Shape& StdMeshers_SMESHBlock::Shape(const int theID)
{
  myErrorStatus=0;
  //
  int aNb;
  //
  aNb=myShapeIDMap.Extent();
  if (theID<1 || theID>aNb) {
    myErrorStatus=3; // ID is out of range
    return myEmptyShape;
  }
  //
  const TopoDS_Shape& aS=myShapeIDMap.FindKey(theID);
  return aS;
}
