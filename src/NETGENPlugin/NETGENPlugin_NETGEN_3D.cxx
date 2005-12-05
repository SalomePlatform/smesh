// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
//=============================================================================
// File      : NETGENPlugin_NETGEN_3D.cxx
//             Moved here from SMESH_NETGEN_3D.cxx
// Created   : lundi 27 Janvier 2003
// Author    : Nadir BOUHAMOU (CEA)
// Project   : SALOME
// Copyright : CEA 2003
// $Header$
//=============================================================================
using namespace std;

#include "NETGENPlugin_NETGEN_3D.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_FacePosition.hxx"

#include <TopExp.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TColStd_ListIteratorOfListOfInteger.hxx>

#include <BRep_Tool.hxx>
#include <Geom_Surface.hxx>
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>

#include "utilities.h"

/*
  Netgen include files
*/

#include "nglib.h"

//=============================================================================
/*!
 *  
 */
//=============================================================================

NETGENPlugin_NETGEN_3D::NETGENPlugin_NETGEN_3D(int hypId, int studyId,
			     SMESH_Gen* gen)
  : SMESH_3D_Algo(hypId, studyId, gen)
{
  MESSAGE("NETGENPlugin_NETGEN_3D::NETGENPlugin_NETGEN_3D");
  _name = "NETGEN_3D";
//   _shapeType = TopAbs_SOLID;
  _shapeType = (1 << TopAbs_SHELL) | (1 << TopAbs_SOLID);// 1 bit /shape type
//   MESSAGE("_shapeType octal " << oct << _shapeType);
  _compatibleHypothesis.push_back("MaxElementVolume");

  _maxElementVolume = 0.;

  _hypMaxElementVolume = NULL;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

NETGENPlugin_NETGEN_3D::~NETGENPlugin_NETGEN_3D()
{
  MESSAGE("NETGENPlugin_NETGEN_3D::~NETGENPlugin_NETGEN_3D");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool NETGENPlugin_NETGEN_3D::CheckHypothesis
                         (SMESH_Mesh& aMesh,
                          const TopoDS_Shape& aShape,
                          SMESH_Hypothesis::Hypothesis_Status& aStatus)
{
  MESSAGE("NETGENPlugin_NETGEN_3D::CheckHypothesis");

  _hypMaxElementVolume = NULL;

  list<const SMESHDS_Hypothesis*>::const_iterator itl;
  const SMESHDS_Hypothesis* theHyp;

  const list<const SMESHDS_Hypothesis*>& hyps = GetUsedHypothesis(aMesh, aShape);
  int nbHyp = hyps.size();
  if (!nbHyp)
  {
    aStatus = SMESH_Hypothesis::HYP_MISSING;
    return false;  // can't work with no hypothesis
  }

  itl = hyps.begin();
  theHyp = (*itl); // use only the first hypothesis

  string hypName = theHyp->GetName();
  int hypId = theHyp->GetID();
  SCRUTE(hypName);

  bool isOk = false;

  if (hypName == "MaxElementVolume")
  {
    _hypMaxElementVolume = static_cast<const StdMeshers_MaxElementVolume*> (theHyp);
    ASSERT(_hypMaxElementVolume);
    _maxElementVolume = _hypMaxElementVolume->GetMaxVolume();
    isOk =true;
    aStatus = SMESH_Hypothesis::HYP_OK;
  }
  else
    aStatus = SMESH_Hypothesis::HYP_INCOMPATIBLE;

  return isOk;
}

//=============================================================================
/*!
 *Here we are going to use the NETGEN mesher
 */
//=============================================================================

bool NETGENPlugin_NETGEN_3D::Compute(SMESH_Mesh& aMesh,
			     const TopoDS_Shape& aShape)
{
  MESSAGE("NETGENPlugin_NETGEN_3D::Compute with maxElmentsize = " << _maxElementVolume);

  bool isOk = false;
  SMESHDS_Mesh* meshDS = aMesh.GetMeshDS();
  SMESH_subMesh* theSubMesh = aMesh.GetSubMesh(aShape);
  //const Handle(SMESHDS_SubMesh)& subMeshDS = theSubMesh->GetSubMeshDS();

  map<int, const SMDS_MeshNode*> netgenToDS;

  MESSAGE("NETGENPlugin_NETGEN_3D::Compute Checking the mesh Faces");

  // check if all faces were meshed by a triangle mesher (here MESFISTO_2D)

  vector<SMESH_subMesh*> meshFaces;
  vector<TopoDS_Shape> shapeFaces;

  for (TopExp_Explorer exp(aShape,TopAbs_FACE);exp.More();exp.Next())
    {
      TopoDS_Shape aShapeFace = exp.Current();
      SMESH_subMesh* aSubMesh = aMesh.GetSubMeshContaining(exp.Current());
      ASSERT (aSubMesh);
      int internal_size = meshFaces.size();
      int index = 0;
      for (int i = 0;i<internal_size;i++)
	{
	  if (aSubMesh == meshFaces[i]) index = 1;
	}
      if (index == 0) meshFaces.push_back(aSubMesh);

      internal_size = shapeFaces.size();
      index = 0;
      for (int i = 0;i<internal_size;i++)
	{
	  if (aShapeFace == shapeFaces[i]) index = 1;
	}
      if (index == 0) shapeFaces.push_back(aShapeFace);
    }

  int numberOfFaces = meshFaces.size();
  int numberOfShapeFaces = shapeFaces.size();

  SCRUTE(numberOfFaces);
  SCRUTE(numberOfShapeFaces);

  MESSAGE("---");

  int NbTotOfTria = 0;
  int NbTotOfNodesFaces = 0;

  for (int i=0; i<numberOfFaces; i++)
    {
      TopoDS_Shape aShapeFace = meshFaces[i]->GetSubShape();
      TopoDS_Shape aFace = shapeFaces[i];
      SMESH_Algo* algoFace = _gen->GetAlgo(aMesh, aShapeFace);
      string algoFaceName = algoFace->GetName();
      SCRUTE(algoFaceName);
      if (algoFaceName != "MEFISTO_2D")
	{
	  SCRUTE(algoFaceName);
	  ASSERT(0);
	  return false;
	}

      bool orientationMeshFace = (aFace.Orientation() == aShapeFace.Orientation());

      const SMESHDS_SubMesh* aSubMeshDSFace = meshFaces[i]->GetSubMeshDS();
      SCRUTE(aSubMeshDSFace);

      int nbNodes = aSubMeshDSFace->NbNodes();
      NbTotOfNodesFaces += nbNodes;
      int nbTria = aSubMeshDSFace->NbElements();
      NbTotOfTria += nbTria;
      int index = 0;

      MESSAGE("NETGENPlugin_NETGEN_3D::Compute The mesh Face " << (i+1) << " has " << nbNodes << " face internal Nodes, " << nbTria << " triangles");

      SCRUTE(orientationMeshFace);

      if (orientationMeshFace)
	{
	  MESSAGE("The mesh and face have the same orientation");
	}
      else
	{
	  MESSAGE("The mesh and face have different orientations");
	}

      SMDS_NodeIteratorPtr iteratorNodes = aSubMeshDSFace->GetNodes();
      SCRUTE(nbNodes);
      index = 0;
      while(iteratorNodes->more())
	{
	  index++;
	  const SMDS_MeshNode * node = iteratorNodes->next();
// 	  int nodeId = node->GetID();
// 	  double nodeX = node->X();
// 	  double nodeY = node->Y();
// 	  double nodeZ = node->Z();
// 	  MESSAGE("NODE -> ID = " << nodeId << " X = " << nodeX << " Y = " << nodeY << " Z = " << nodeZ);
	}

      SCRUTE(index);

      SMDS_ElemIteratorPtr iteratorTriangle = aSubMeshDSFace->GetElements();

      SCRUTE(nbTria);
      index = 0;
      int numberOfDegeneratedTriangle = 0;
      while(iteratorTriangle->more())
	{
	  index++;
	  const SMDS_MeshElement * triangle = iteratorTriangle->next();
	  int triangleId = triangle->GetID();

	  SMDS_ElemIteratorPtr triangleNodesIt = triangle->nodesIterator();

	  const SMDS_MeshNode * node1 = static_cast<const SMDS_MeshNode *>(triangleNodesIt->next());
	  double node1X = node1->X();
	  double node1Y = node1->Y();
	  double node1Z = node1->Z();

	  const SMDS_MeshNode * node2 = static_cast<const SMDS_MeshNode *>(triangleNodesIt->next());
	  double node2X = node2->X();
	  double node2Y = node2->Y();
	  double node2Z = node2->Z();

	  const SMDS_MeshNode * node3 = static_cast<const SMDS_MeshNode *>(triangleNodesIt->next());
	  double node3X = node3->X();
	  double node3Y = node3->Y();
	  double node3Z = node3->Z();

	  int triangleNode1 = node1->GetID();
	  int triangleNode2 = node2->GetID();
	  int triangleNode3 = node3->GetID();

	  // Compute the triangle surface

	  double vect1 = ((node2Y - node1Y)*(node3Z - node1Z) - (node2Z - node1Z)*(node3Y - node1Y));
	  double vect2 = - ((node2X - node1X)*(node3Z - node1Z) - (node2Z - node1Z)*(node3X - node1X));
	  double vect3 = ((node2X - node1X)*(node3Y - node1Y) - (node2Y - node1Y)*(node3X - node1X));
	  double epsilon = 1.0e-6;

	  bool triangleIsDegenerated = ((abs(vect1)<epsilon) && (abs(vect2)<epsilon) && (abs(vect3)<epsilon));

	  if (triangleIsDegenerated)
	    {
// 	      MESSAGE("TRIANGLE -> ID = " << triangleId << " N1 = " << triangleNode1 << " N2 = " << triangleNode2 << " N3 = " << triangleNode3 << " is degenerated");
// 	      MESSAGE("NODE -> ID = " << triangleNode1 << " X = " << node1X << " Y = " << node1Y << " Z = " << node1Z);
// 	      MESSAGE("NODE -> ID = " << triangleNode2 << " X = " << node2X << " Y = " << node2Y << " Z = " << node2Z);
// 	      MESSAGE("NODE -> ID = " << triangleNode3 << " X = " << node3X << " Y = " << node3Y << " Z = " << node3Z);
	      numberOfDegeneratedTriangle++;
	    }
	  else
	    {
// 	      MESSAGE("TRIANGLE -> ID = " << triangleId << " N1 = " << triangleNode1 << " N2 = " << triangleNode2 << " N3 = " << triangleNode3 << " is normal");
	    }
	}

      if (numberOfDegeneratedTriangle > 0)
	MESSAGE("WARNING THERE IS(ARE) " << numberOfDegeneratedTriangle << " degenerated triangle on this face");

      SCRUTE(index);
    }



  SCRUTE(NbTotOfTria);
  SCRUTE(NbTotOfNodesFaces);

  MESSAGE("NETGENPlugin_NETGEN_3D::Compute Checking the mesh Edges");

  // check if all edges were meshed by a edge mesher (here Regular_1D)

  vector<SMESH_subMesh*> meshEdges;
  for (TopExp_Explorer exp(aShape,TopAbs_EDGE);exp.More();exp.Next())
    {
      SMESH_subMesh* aSubMesh = aMesh.GetSubMeshContaining(exp.Current());
      ASSERT (aSubMesh);
      int internal_size = meshEdges.size();
      int index = 0;
      for (int i = 0;i<internal_size;i++)
	{
	  if (aSubMesh == meshEdges[i]) index = 1;
	}
      if (index == 0) meshEdges.push_back(aSubMesh);
    }

  int numberOfEdges = meshEdges.size();
  SCRUTE(numberOfEdges);

  MESSAGE("---");

  int NbTotOfNodesEdges = 0;
  int NbTotOfSegs = 0;

  for (int i=0; i<numberOfEdges; i++)
    {
      TopoDS_Shape aShapeEdge = meshEdges[i]->GetSubShape();
      SMESH_Algo* algoEdge = _gen->GetAlgo(aMesh, aShapeEdge);
      string algoEdgeName = algoEdge->GetName();
      SCRUTE(algoEdgeName);
      if (algoEdgeName != "Regular_1D")
	{
	  SCRUTE(algoEdgeName);
	  ASSERT(0);
	  return false;
	}

      const SMESHDS_SubMesh* aSubMeshDSEdge = meshEdges[i]->GetSubMeshDS();
      SCRUTE(aSubMeshDSEdge);

      int nbNodes = aSubMeshDSEdge->NbNodes();
      NbTotOfNodesEdges += nbNodes;
      int nbSegs = aSubMeshDSEdge->NbElements();
      NbTotOfSegs += nbSegs;

      MESSAGE("NETGENPlugin_NETGEN_3D::Compute The mesh Edge " << (i+1) << " has " << nbNodes << " edge internal Nodes, " << nbSegs << " segments");

      SMDS_NodeIteratorPtr iteratorNodes = aSubMeshDSEdge->GetNodes();
      SCRUTE(nbNodes);
      int index = 0;
      while(iteratorNodes->more())
	{
	  index++;
 	  const SMDS_MeshNode * node = iteratorNodes->next();
// 	  int nodeId = node->GetID();
// 	  double nodeX = node->X();
// 	  double nodeY = node->Y();
// 	  double nodeZ = node->Z();
// 	  MESSAGE("NODE -> ID = " << nodeId << " X = " << nodeX << " Y = " << nodeY << " Z = " << nodeZ);
	}

      SCRUTE(index);
    }

  SCRUTE(NbTotOfNodesEdges);
  SCRUTE(NbTotOfSegs);

  MESSAGE("NETGENPlugin_NETGEN_3D::Compute Checking the mesh Vertices");

  vector<SMESH_subMesh*> meshVertices;
  for (TopExp_Explorer exp(aShape,TopAbs_VERTEX);exp.More();exp.Next())
    {
      SMESH_subMesh* aSubMesh = aMesh.GetSubMeshContaining(exp.Current());
      ASSERT (aSubMesh);
      int internal_size = meshVertices.size();
      int index = 0;
      for (int i = 0;i<internal_size;i++)
	{
	  if (aSubMesh == meshVertices[i]) index = 1;
	}
      if (index == 0) meshVertices.push_back(aSubMesh);
    }

  int numberOfVertices = meshVertices.size();
  SCRUTE(numberOfVertices);

  MESSAGE("---");

  int NbTotOfNodesVertices = 0;

  for (int i=0; i<numberOfVertices; i++)
    {
      TopoDS_Shape aShapeVertex = meshVertices[i]->GetSubShape();

      const SMESHDS_SubMesh * aSubMeshDSVertex = meshVertices[i]->GetSubMeshDS();
      SCRUTE(aSubMeshDSVertex);

      int nbNodes = aSubMeshDSVertex->NbNodes();
      NbTotOfNodesVertices += nbNodes;

      MESSAGE("NETGENPlugin_NETGEN_3D::Compute The mesh Vertex " << (i+1) << " has " << nbNodes << " Nodes");

      SMDS_NodeIteratorPtr iteratorNodes = aSubMeshDSVertex->GetNodes();
      SCRUTE(nbNodes);
      int index = 0;
      while(iteratorNodes->more())
	{
	  index++;
 	  const SMDS_MeshNode * node = iteratorNodes->next();
// 	  int nodeId = node->GetID();
// 	  double nodeX = node->X();
// 	  double nodeY = node->Y();
// 	  double nodeZ = node->Z();
// 	  MESSAGE("NODE -> ID = " << nodeId << " X = " << nodeX << " Y = " << nodeY << " Z = " << nodeZ);
	}

      SCRUTE(index);
    }

  SCRUTE(NbTotOfNodesVertices);

  MESSAGE("NETGENPlugin_NETGEN_3D::Compute --> Analysis of all shell mesh");

  vector<SMESH_subMesh*> meshShells;
  TopoDS_Shell aShell;

  for (TopExp_Explorer exp(aShape,TopAbs_SHELL);exp.More();exp.Next())
    {
      SMESH_subMesh* aSubMesh = aMesh.GetSubMesh(exp.Current());
      ASSERT(aSubMesh);
      SCRUTE(aSubMesh);
      aShell = TopoDS::Shell(exp.Current());
      meshShells.push_back(aSubMesh);
    }

  int numberOfShells = meshShells.size();
  SCRUTE(numberOfShells);

  if (numberOfShells == 1)
    {
      MESSAGE("NETGENPlugin_NETGEN_3D::Compute Only one shell --> generation of the mesh using directly Netgen");

      /*
	Prepare the Netgen surface mesh from the SMESHDS
      */

      MESSAGE("NETGENPlugin_NETGEN_3D::Compute Prepare the Netgen surface mesh from the SMESHDS");

      int spaceDimension = 3;
      int nbNodesByTri = 3;
      int nbNodesByTetra = 4;

      int Netgen_NbOfNodes = NbTotOfNodesFaces +
	                     NbTotOfNodesEdges +
	                     NbTotOfNodesVertices;
      int Netgen_NbOfTria = NbTotOfTria;
      int Netgen_param2ndOrder = 0;
      double Netgen_paramFine = 1.;
      double Netgen_paramSize = _maxElementVolume;

      SCRUTE(Netgen_NbOfNodes);
      SCRUTE(Netgen_NbOfTria);

      double * Netgen_Coordinates = new double [spaceDimension*
						Netgen_NbOfNodes];
      int * listNodeCoresNetgenSmesh = new int [Netgen_NbOfNodes];
      int * Netgen_Connectivity = new int [nbNodesByTri*Netgen_NbOfTria];
      double * Netgen_point = new double [spaceDimension];
      int * Netgen_triangle = new int [nbNodesByTri];
      int * Netgen_tetrahedron = new int [nbNodesByTetra];

      for (int i=0; i<Netgen_NbOfTria; i++)
	{
	  for (int j=0; j<nbNodesByTri; j++)
	    Netgen_Connectivity[i*nbNodesByTri+j] = 0;
	}

      double bigNumber = 1.e20;

      for (int i=0; i<Netgen_NbOfNodes; i++)
	{
	  listNodeCoresNetgenSmesh[i] = 0;
	  for (int j=0; j<spaceDimension; j++)
	    Netgen_Coordinates[i*spaceDimension+j] = bigNumber;
	}

      int indexNodes = 0;
      for (int i=0; i<numberOfVertices; i++)
	{
	  const SMESHDS_SubMesh * aSubMeshDSVertex =
	    meshVertices[i]->GetSubMeshDS();

	  SMDS_NodeIteratorPtr iteratorNodes = aSubMeshDSVertex->GetNodes();

	  while(iteratorNodes->more())
	    {
	      const SMDS_MeshNode * node = iteratorNodes->next();
	      int nodeId = node->GetID();
	      double nodeX = node->X();
	      double nodeY = node->Y();
	      double nodeZ = node->Z();
// 	      MESSAGE("NODE -> ID = " << nodeId << " X = " << nodeX << " Y = " << nodeY << " Z = " << nodeZ);
	      listNodeCoresNetgenSmesh[indexNodes] = nodeId;
	      int index = indexNodes*spaceDimension;
	      Netgen_Coordinates[index] = nodeX;
	      Netgen_Coordinates[index+1] = nodeY;
	      Netgen_Coordinates[index+2] = nodeZ;
	      netgenToDS[indexNodes] = node;
	      indexNodes++;
	    }
	}

      for (int i=0; i<numberOfEdges; i++)
	{
	  const SMESHDS_SubMesh *  aSubMeshDSEdge =
	    meshEdges[i]->GetSubMeshDS();

	  SMDS_NodeIteratorPtr iteratorNodes = aSubMeshDSEdge->GetNodes();

	  while(iteratorNodes->more())
	    {
	      const SMDS_MeshNode * node = iteratorNodes->next();
	      int nodeId = node->GetID();
	      double nodeX = node->X();
	      double nodeY = node->Y();
	      double nodeZ = node->Z();
// 	      MESSAGE("NODE -> ID = " << nodeId << " X = " << nodeX << " Y = " << nodeY << " Z = " << nodeZ);
	      listNodeCoresNetgenSmesh[indexNodes] = node->GetID();
	      int index = indexNodes*spaceDimension;
	      Netgen_Coordinates[index] = node->X();
	      Netgen_Coordinates[index+1] = node->Y();
	      Netgen_Coordinates[index+2] = node->Z();
	      netgenToDS[indexNodes] = node;
	      indexNodes++;
	    }
	}

      for (int i=0; i<numberOfFaces; i++)
	{
	  const SMESHDS_SubMesh * aSubMeshDSFace =
	    meshFaces[i]->GetSubMeshDS();

	  SMDS_NodeIteratorPtr iteratorNodes = aSubMeshDSFace->GetNodes();

	  while(iteratorNodes->more())
	    {
	      const SMDS_MeshNode * node = iteratorNodes->next();
	      int nodeId = node->GetID();
	      double nodeX = node->X();
	      double nodeY = node->Y();
	      double nodeZ = node->Z();
// 	      MESSAGE("NODE -> ID = " << nodeId << " X = " << nodeX << " Y = " << nodeY << " Z = " << nodeZ);
	      listNodeCoresNetgenSmesh[indexNodes] = nodeId;
	      int index = indexNodes*spaceDimension;
	      Netgen_Coordinates[index] = nodeX;
	      Netgen_Coordinates[index+1] = nodeY;
	      Netgen_Coordinates[index+2] = nodeZ;
	      netgenToDS[indexNodes] = node;
	      indexNodes++;
	    }
	}

      SCRUTE(indexNodes);

      for (int i=0; i<Netgen_NbOfNodes; i++)
	{
	  ASSERT(listNodeCoresNetgenSmesh[i] != 0);

	  for (int j=0; j<Netgen_NbOfNodes && j!=i; j++)
	    ASSERT(listNodeCoresNetgenSmesh[i] != listNodeCoresNetgenSmesh[j]);

	  for (int j=0; j<spaceDimension; j++)
	    ASSERT(Netgen_Coordinates[i*spaceDimension+j] != bigNumber);
	}

      int indexTrias = 0;
      for (int i=0; i<numberOfFaces; i++)
	{
	  const SMESHDS_SubMesh * aSubMeshDSFace =
	    meshFaces[i]->GetSubMeshDS();

	  TopoDS_Shape aFace = shapeFaces[i];

	  SMDS_ElemIteratorPtr iteratorTriangle = aSubMeshDSFace->GetElements();

	  TopoDS_Shape aShapeFace = meshFaces[i]->GetSubShape();

	  bool orientationMeshFace = (aFace.Orientation() == aShapeFace.Orientation());

	  SCRUTE(orientationMeshFace);

	  if (orientationMeshFace)
	    {
	      MESSAGE("The mesh and face have the same orientation");

	      while(iteratorTriangle->more())
		{
		  const SMDS_MeshElement * triangle = iteratorTriangle->next();
		  int triangleId = triangle->GetID();

		  SMDS_ElemIteratorPtr triangleNodesIt = triangle->nodesIterator();

		  int triangleNode1 = (triangleNodesIt->next())->GetID();
		  int triangleNode2 = (triangleNodesIt->next())->GetID();
		  int triangleNode3 = (triangleNodesIt->next())->GetID();

// 		  MESSAGE("TRIANGLE -> ID = " << triangleId << " N1 = " << triangleNode1 << " N2 = " << triangleNode2 << " N3 = " << triangleNode3);

		  int N1New = 0;
		  int N2New = 0;
		  int N3New = 0;
		  int index = indexTrias*nbNodesByTri;

		  for (int j=0; j<Netgen_NbOfNodes; j++)
		    {
		      int jp1 = j+1;

		      if (triangleNode1 == listNodeCoresNetgenSmesh[j])
			N1New = jp1;
		      else if (triangleNode2 == listNodeCoresNetgenSmesh[j])
			N2New = jp1;
		      else if (triangleNode3 == listNodeCoresNetgenSmesh[j])
			N3New = jp1;
		    }

		  triangleNode1 = N1New;
		  triangleNode2 = N2New;
		  triangleNode3 = N3New;

		  Netgen_Connectivity[index] = triangleNode1;
		  Netgen_Connectivity[index+1] = triangleNode2;
		  Netgen_Connectivity[index+2] = triangleNode3;

		  indexTrias++;
		}
	    }
	  else
	    {
	      MESSAGE("The mesh and face have different orientations");

	      while(iteratorTriangle->more())
		{
		  const SMDS_MeshElement * triangle = iteratorTriangle->next();
		  int triangleId = triangle->GetID();

		  SMDS_ElemIteratorPtr triangleNodesIt = triangle->nodesIterator();

		  int triangleNode1 = (triangleNodesIt->next())->GetID();
		  int triangleNode3 = (triangleNodesIt->next())->GetID();
		  int triangleNode2 = (triangleNodesIt->next())->GetID();

// 		  MESSAGE("TRIANGLE -> ID = " << triangleId << " N1 = " << triangleNode1 << " N2 = " << triangleNode2 << " N3 = " << triangleNode3);

		  int N1New = 0;
		  int N2New = 0;
		  int N3New = 0;
		  int index = indexTrias*nbNodesByTri;

		  for (int j=0; j<Netgen_NbOfNodes; j++)
		    {
		      int jp1 = j+1;

		      if (triangleNode1 == listNodeCoresNetgenSmesh[j])
			N1New = jp1;
		      else if (triangleNode2 == listNodeCoresNetgenSmesh[j])
			N2New = jp1;
		      else if (triangleNode3 == listNodeCoresNetgenSmesh[j])
			N3New = jp1;
		    }

		  triangleNode1 = N1New;
		  triangleNode2 = N2New;
		  triangleNode3 = N3New;

		  Netgen_Connectivity[index] = triangleNode1;
		  Netgen_Connectivity[index+1] = triangleNode2;
		  Netgen_Connectivity[index+2] = triangleNode3;

		  indexTrias++;
		}
	    }
	}

      SCRUTE(indexTrias);

      int * nodesUsed = new int[Netgen_NbOfNodes];

      for (int i=0; i<Netgen_NbOfNodes; i++) nodesUsed[i] = 0;

      for (int i=0; i<Netgen_NbOfTria; i++)
	for (int j=0; j<nbNodesByTri; j++)
	  {
	    int Nij = Netgen_Connectivity[i*nbNodesByTri+j];

	    ASSERT((Nij>=1) && (Nij<=Netgen_NbOfNodes));

	    nodesUsed[Nij-1] = 1;
	    Netgen_Connectivity[i*nbNodesByTri+j] = Nij;
	  }

      for (int i=0; i<Netgen_NbOfNodes; i++)
	{
	  ASSERT(nodesUsed[i] != 0);
	}

      delete [] nodesUsed;

      /*
	Feed the Netgen surface mesh
      */

      MESSAGE("NETGENPlugin_NETGEN_3D::Compute Feed the Netgen surface mesh");

      Ng_Mesh * Netgen_mesh;

      Ng_Init();

      Netgen_mesh = Ng_NewMesh();

      Ng_Meshing_Parameters Netgen_param;

      for (int i=0; i<Netgen_NbOfNodes; i++)
	{
	  for (int j=0; j<spaceDimension; j++)
	    Netgen_point[j] = Netgen_Coordinates[i*spaceDimension+j];

	  Ng_AddPoint(Netgen_mesh, Netgen_point);
	}

      for (int i=0; i<Netgen_NbOfTria; i++)
	{
	  for (int j=0; j<nbNodesByTri; j++)
	    Netgen_triangle[j] = Netgen_Connectivity[i*nbNodesByTri+j];

	  Ng_AddSurfaceElement(Netgen_mesh, NG_TRIG, Netgen_triangle);
	}

      SCRUTE(Netgen_paramSize);

      Netgen_param.secondorder = Netgen_param2ndOrder;
      Netgen_param.fineness = Netgen_paramFine;
      Netgen_param.maxh = Netgen_paramSize;

      /*
	Generate the volume mesh
      */

      MESSAGE("NETGENPlugin_NETGEN_3D::Compute Generate the volume mesh");

      SCRUTE(Netgen_NbOfNodes);
      SCRUTE(Netgen_NbOfTria);

      SCRUTE(Ng_GetNP(Netgen_mesh));
      SCRUTE(Ng_GetNE(Netgen_mesh));
      SCRUTE(Ng_GetNSE(Netgen_mesh));

      ASSERT(Netgen_NbOfNodes == Ng_GetNP(Netgen_mesh));
      ASSERT(Ng_GetNE(Netgen_mesh) == 0);
      ASSERT(Netgen_NbOfTria == Ng_GetNSE(Netgen_mesh));

      Ng_Result status;

      status = Ng_GenerateVolumeMesh(Netgen_mesh, &Netgen_param);

      SCRUTE(status);

      int Netgen_NbOfNodesNew = Ng_GetNP(Netgen_mesh);

      int Netgen_NbOfTetra = Ng_GetNE(Netgen_mesh);

      SCRUTE(Netgen_NbOfNodesNew);

      SCRUTE(Netgen_NbOfTetra);

      if ((status != NG_OK) ||
	  (Netgen_NbOfNodesNew <= Netgen_NbOfNodes) ||
	  (Netgen_NbOfTetra <= 0))
	{
	  MESSAGE("NETGENPlugin_NETGEN_3D::Compute The Volume Mesh Generation has failed ...");
	  SCRUTE(status);

	  /*
	    Free the memory needed by to generate the Netgen Mesh
	  */

	  MESSAGE("NETGENPlugin_NETGEN_3D::Compute Free the memory needed by to generate the Netgen Mesh");

	  delete [] Netgen_Coordinates;
	  delete [] Netgen_Connectivity;
	  delete [] Netgen_point;
	  delete [] Netgen_triangle;
	  delete [] Netgen_tetrahedron;

	  delete [] listNodeCoresNetgenSmesh;

	  Ng_DeleteMesh(Netgen_mesh);
	  Ng_Exit();

	  return false;
	}

      MESSAGE("NETGENPlugin_NETGEN_3D::Compute End of Volume Mesh Generation");
      SCRUTE(status);

      double * Netgen_CoordinatesNew = new double [spaceDimension*Netgen_NbOfNodesNew];
      int * Netgen_ConnectivityNew = new int [nbNodesByTetra*Netgen_NbOfTetra];

      for (int i=0; i<Netgen_NbOfNodesNew; i++)
	{
	  Ng_GetPoint(Netgen_mesh, (i+1), Netgen_point);

	  for (int j=0; j<spaceDimension; j++)
	    Netgen_CoordinatesNew[i*spaceDimension+j] = Netgen_point[j];
	}

      for (int i=0; i<Netgen_NbOfNodes; i++)
	for (int j=0; j<spaceDimension; j++)
	  ASSERT(Netgen_CoordinatesNew[i*spaceDimension+j] == Netgen_Coordinates[i*spaceDimension+j])

      for (int i=0; i<Netgen_NbOfTetra; i++)
	{
	  Ng_GetVolumeElement(Netgen_mesh, (i+1), Netgen_tetrahedron);

	  for (int j=0; j<nbNodesByTetra; j++)
	    Netgen_ConnectivityNew[i*nbNodesByTetra+j] = Netgen_tetrahedron[j];
	}

      /*
	Feed back the SMESHDS with the generated Nodes and Volume Elements
      */

      MESSAGE("NETGENPlugin_NETGEN_3D::Compute Feed back the SMESHDS with the generated Nodes and Volume Elements");

      int NbTotOfNodesShell = Netgen_NbOfNodesNew - Netgen_NbOfNodes;

      SCRUTE(NbTotOfNodesShell);

      int * listNodeShellCoresNetgenSmesh = new int [NbTotOfNodesShell];

      for (int i=0; i<NbTotOfNodesShell; i++)
	listNodeShellCoresNetgenSmesh[i] = 0;

      MESSAGE("NETGENPlugin_NETGEN_3D::Compute --> Adding the New Nodes to SMESHDS");

      for (int i=0; i<NbTotOfNodesShell; i++)
	{
	  int index = (i+Netgen_NbOfNodes)*spaceDimension;

	  SMDS_MeshNode * node =
	    meshDS->AddNode(Netgen_CoordinatesNew[index],
			    Netgen_CoordinatesNew[index+1],
			    Netgen_CoordinatesNew[index+2]);

	  meshDS->SetNodeInVolume(node, aShell);

	  index = i+Netgen_NbOfNodes;
	  netgenToDS[index] = node;

	  listNodeShellCoresNetgenSmesh[i] = node->GetID();
	}

      SCRUTE(Netgen_NbOfNodesNew);
      
      SCRUTE(netgenToDS.size());

      for (int i=0; i<NbTotOfNodesShell; i++)
	{
	  ASSERT(listNodeShellCoresNetgenSmesh[i] != 0);

	  for (int j=0; j<NbTotOfNodesShell && j!=i; j++)
	    ASSERT(listNodeShellCoresNetgenSmesh[i] != listNodeShellCoresNetgenSmesh[j]);
	}

      MESSAGE("NETGENPlugin_NETGEN_3D::Compute --> Adding the New elements (Tetrahedrons) to the SMESHDS");

      for (int i=0; i<Netgen_NbOfTetra; i++)
	{
	  int index = i*nbNodesByTetra;
	  int tetraNode1 = Netgen_ConnectivityNew[index];
	  int tetraNode2 = Netgen_ConnectivityNew[index+1];
	  int tetraNode3 = Netgen_ConnectivityNew[index+2];
	  int tetraNode4 = Netgen_ConnectivityNew[index+3];

	  const SMDS_MeshNode * node1 = netgenToDS[tetraNode1-1];
	  const SMDS_MeshNode * node2 = netgenToDS[tetraNode2-1];
	  const SMDS_MeshNode * node3 = netgenToDS[tetraNode3-1];
	  const SMDS_MeshNode * node4 = netgenToDS[tetraNode4-1];

	  index = tetraNode1;
	  if (index <= Netgen_NbOfNodes)
	    tetraNode1 = listNodeCoresNetgenSmesh[index-1];
	  else
	    tetraNode1 = listNodeShellCoresNetgenSmesh[index-Netgen_NbOfNodes-1];

	  index = tetraNode2;
	  if (index <= Netgen_NbOfNodes)
	    tetraNode2 = listNodeCoresNetgenSmesh[index-1];
	  else
	    tetraNode2 = listNodeShellCoresNetgenSmesh[index-Netgen_NbOfNodes-1];

	  index = tetraNode3;
	  if (index <= Netgen_NbOfNodes)
	    tetraNode3 = listNodeCoresNetgenSmesh[index-1];
	  else
	    tetraNode3 = listNodeShellCoresNetgenSmesh[index-Netgen_NbOfNodes-1];

	  index = tetraNode4;
	  if (index <= Netgen_NbOfNodes)
	    tetraNode4 = listNodeCoresNetgenSmesh[index-1];
	  else
	    tetraNode4 = listNodeShellCoresNetgenSmesh[index-Netgen_NbOfNodes-1];

	  SMDS_MeshVolume * elt =
	    meshDS->AddVolume(node1,node2,node3,node4);

	  meshDS->SetMeshElementOnShape(elt, aShell);
	}

      /*
	Free the memory needed by to generate the Netgen Mesh
      */

      MESSAGE("NETGENPlugin_NETGEN_3D::Compute Free the memory needed by to generate the Netgen Mesh");

      delete [] Netgen_Coordinates;
      delete [] Netgen_Connectivity;
      delete [] Netgen_CoordinatesNew;
      delete [] Netgen_ConnectivityNew;
      delete [] Netgen_point;
      delete [] Netgen_triangle;
      delete [] Netgen_tetrahedron;

      delete [] listNodeCoresNetgenSmesh;
      delete [] listNodeShellCoresNetgenSmesh;

      Ng_DeleteMesh(Netgen_mesh);
      Ng_Exit();

      /*
	Verification
      */

      {
	MESSAGE("NETGENPlugin_NETGEN_3D::Compute Verification of the Shell mesh");

	TopoDS_Shape aShapeShell = meshShells[0]->GetSubShape();
	SMESH_Algo* algoShell = _gen->GetAlgo(aMesh, aShapeShell);
	string algoShellName = algoShell->GetName();
	SCRUTE(algoShellName);
	if (algoShellName != "NETGEN_3D")
	  {
	    SCRUTE(algoShellName);
	    ASSERT(0);
	    return false;
	  }

	const SMESHDS_SubMesh * aSubMeshDSShell = meshShells[0]->GetSubMeshDS();
	SCRUTE(&aSubMeshDSShell);

	int nbNodes = aSubMeshDSShell->NbNodes();
	int nbTetra = aSubMeshDSShell->NbElements();

	MESSAGE("NETGENPlugin_NETGEN_3D::Compute The mesh Shell has " << nbNodes << " shell internal Nodes, " << nbTetra << " tetrahedrons");

	SMDS_NodeIteratorPtr iteratorNodes = aSubMeshDSShell->GetNodes();

	SCRUTE(nbNodes);

	int index;

	index = 0;

	while(iteratorNodes->more())
	  {
	    index++;
	    const SMDS_MeshNode * node = iteratorNodes->next();
	    int nodeId = node->GetID();
	    double nodeX = node->X();
	    double nodeY = node->Y();
	    double nodeZ = node->Z();
// 	    MESSAGE("NODE -> ID = " << nodeId << " X = " << nodeX << " Y = " << nodeY << " Z = " << nodeZ);
	  }

	SCRUTE(index);

	SMDS_ElemIteratorPtr iteratorTetra = aSubMeshDSShell->GetElements();

	SCRUTE(nbTetra);

	index = 0;
	while(iteratorTetra->more())
	  {
	    index++;
	    const SMDS_MeshElement * tetra = iteratorTetra->next();
	    int tetraId = tetra->GetID();

	    SMDS_ElemIteratorPtr tetraNodesIt = tetra->nodesIterator();

	    int tetraNode1 = (tetraNodesIt->next())->GetID();
	    int tetraNode2 = (tetraNodesIt->next())->GetID();
	    int tetraNode3 = (tetraNodesIt->next())->GetID();
	    int tetraNode4 = (tetraNodesIt->next())->GetID();

// 	    MESSAGE("TETRAHEDRON -> ID = " << tetraId << " N1 = " << tetraNode1 << " N2 = " << tetraNode2 << " N3 = " << tetraNode3 << " N4 = " << tetraNode4);

	  }

	SCRUTE(index);
      }
    }
  else
    {
      SCRUTE(numberOfShells);
      MESSAGE("NETGENPlugin_NETGEN_3D::Compute ERROR More than one shell ????? ");
      return false;
    }

  return true;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & NETGENPlugin_NETGEN_3D::SaveTo(ostream & save)
{
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & NETGENPlugin_NETGEN_3D::LoadFrom(istream & load)
{
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator << (ostream & save, NETGENPlugin_NETGEN_3D & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >> (istream & load, NETGENPlugin_NETGEN_3D & hyp)
{
  return hyp.LoadFrom( load );
}
