//  SMESH SMESHDS : management of mesh data and SMESH document
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
//  File   : SMESHDS_Mesh.hxx
//  Module : SMESH

#ifndef _SMESHDS_Mesh_HeaderFile
#define _SMESHDS_Mesh_HeaderFile

#include "SMDS_Mesh.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_MeshEdge.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"
#include "SMESHDS_Hypothesis.hxx"
#include "SMESHDS_SubMesh.hxx"
#include "SMESHDS_Script.hxx"

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <map>
#ifdef WNT
#include <hash_map>
#endif

//Not portable see http://gcc.gnu.org/onlinedocs/libstdc++/faq/index.html#5_4 to know more.
#ifdef __GNUC__
  #if __GNUC__ < 3
    #include <hash_map.h>
    namespace gstd { using ::hash_map; }; // inherit globals
  #elif __GNUC__ == 3
    #include <ext/hash_map>
    #if __GNUC_MINOR__ == 0
      namespace gstd = std;               // GCC 3.0
    #else
      namespace gstd = ::__gnu_cxx;       // GCC 3.1 and later
    #endif
  #else                                   // GCC 4.0 and later
    #include <ext/hash_map>
    namespace gstd = ::__gnu_cxx;
  #endif
#else      // ...  there are other compilers, right?
  namespace gstd = std;
#endif

#if defined WNT && defined WIN32 && defined SMESHDS_EXPORTS
#define SMESHDS_WNT_EXPORT __declspec( dllexport )
#else
#define SMESHDS_WNT_EXPORT
#endif

class SMESHDS_GroupBase;

class SMESHDS_WNT_EXPORT SMESHDS_Mesh:public SMDS_Mesh{
public:
  SMESHDS_Mesh(int MeshID);
  void ShapeToMesh(const TopoDS_Shape & S);
  bool AddHypothesis(const TopoDS_Shape & SS, const SMESHDS_Hypothesis * H);
  bool RemoveHypothesis(const TopoDS_Shape & S, const SMESHDS_Hypothesis * H);
  
  virtual SMDS_MeshNode* AddNodeWithID(double x, double y, double z, int ID);
  virtual SMDS_MeshNode * AddNode(double x, double y, double z);
  
  virtual SMDS_MeshEdge* AddEdgeWithID(int n1, int n2, int ID);
  virtual SMDS_MeshEdge* AddEdgeWithID(const SMDS_MeshNode * n1,
				       const SMDS_MeshNode * n2, 
				       int ID);
  virtual SMDS_MeshEdge* AddEdge(const SMDS_MeshNode * n1,
				 const SMDS_MeshNode * n2);
  
  virtual SMDS_MeshFace* AddFaceWithID(int n1, int n2, int n3, int ID);
  virtual SMDS_MeshFace* AddFaceWithID(const SMDS_MeshNode * n1,
				       const SMDS_MeshNode * n2,
				       const SMDS_MeshNode * n3, 
				       int ID);
  virtual SMDS_MeshFace* AddFace(const SMDS_MeshNode * n1,
				 const SMDS_MeshNode * n2,
				 const SMDS_MeshNode * n3);

  virtual SMDS_MeshFace* AddFaceWithID(int n1, int n2, int n3, int n4, int ID);
  virtual SMDS_MeshFace* AddFaceWithID(const SMDS_MeshNode * n1,
				       const SMDS_MeshNode * n2,
				       const SMDS_MeshNode * n3,
				       const SMDS_MeshNode * n4, 
				       int ID);
  virtual SMDS_MeshFace* AddFace(const SMDS_MeshNode * n1,
				 const SMDS_MeshNode * n2,
				 const SMDS_MeshNode * n3,
				 const SMDS_MeshNode * n4);

  virtual SMDS_MeshVolume* AddVolumeWithID(int n1, int n2, int n3, int n4, int ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
					   const SMDS_MeshNode * n2,
					   const SMDS_MeshNode * n3,
					   const SMDS_MeshNode * n4, 
					   int ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
				     const SMDS_MeshNode * n2,
				     const SMDS_MeshNode * n3,
				     const SMDS_MeshNode * n4);

  virtual SMDS_MeshVolume* AddVolumeWithID(int n1, int n2, int n3, int n4, int n5, int ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
					   const SMDS_MeshNode * n2,
					   const SMDS_MeshNode * n3,
					   const SMDS_MeshNode * n4,
					   const SMDS_MeshNode * n5, 
					   int ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
				     const SMDS_MeshNode * n2,
				     const SMDS_MeshNode * n3,
				     const SMDS_MeshNode * n4,
				     const SMDS_MeshNode * n5);

  virtual SMDS_MeshVolume* AddVolumeWithID(int n1, int n2, int n3, int n4, int n5, int n6, int ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
					   const SMDS_MeshNode * n2,
					   const SMDS_MeshNode * n3,
					   const SMDS_MeshNode * n4,
					   const SMDS_MeshNode * n5,
					   const SMDS_MeshNode * n6, 
					   int ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
				     const SMDS_MeshNode * n2,
				     const SMDS_MeshNode * n3,
				     const SMDS_MeshNode * n4,
				     const SMDS_MeshNode * n5,
				     const SMDS_MeshNode * n6);

  virtual SMDS_MeshVolume* AddVolumeWithID(int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8, int ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
					   const SMDS_MeshNode * n2,
					   const SMDS_MeshNode * n3,
					   const SMDS_MeshNode * n4,
					   const SMDS_MeshNode * n5,
					   const SMDS_MeshNode * n6,
					   const SMDS_MeshNode * n7,
					   const SMDS_MeshNode * n8, 
					   int ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
				     const SMDS_MeshNode * n2,
				     const SMDS_MeshNode * n3,
				     const SMDS_MeshNode * n4,
				     const SMDS_MeshNode * n5,
				     const SMDS_MeshNode * n6,
				     const SMDS_MeshNode * n7,
				     const SMDS_MeshNode * n8);
  
  virtual SMDS_MeshFace* AddPolygonalFaceWithID (std::vector<int> nodes_ids,
                                                 const int        ID);

  virtual SMDS_MeshFace* AddPolygonalFaceWithID (std::vector<const SMDS_MeshNode*> nodes,
                                                 const int                         ID);

  virtual SMDS_MeshFace* AddPolygonalFace (std::vector<const SMDS_MeshNode*> nodes);

  virtual SMDS_MeshVolume* AddPolyhedralVolumeWithID
                           (std::vector<int> nodes_ids,
                            std::vector<int> quantities,
                            const int        ID);

  virtual SMDS_MeshVolume* AddPolyhedralVolumeWithID
                           (std::vector<const SMDS_MeshNode*> nodes,
                            std::vector<int>                  quantities,
                            const int                         ID);

  virtual SMDS_MeshVolume* AddPolyhedralVolume
                           (std::vector<const SMDS_MeshNode*> nodes,
                            std::vector<int>                  quantities);

  void MoveNode(const SMDS_MeshNode *, double x, double y, double z);
  virtual void RemoveNode(const SMDS_MeshNode *);
  void RemoveElement(const SMDS_MeshElement *);
  bool ChangeElementNodes(const SMDS_MeshElement * elem,
                          const SMDS_MeshNode    * nodes[],
                          const int                nbnodes);
  bool ChangePolygonNodes(const SMDS_MeshElement * elem,
                          std::vector<const SMDS_MeshNode*> nodes);
  bool ChangePolyhedronNodes(const SMDS_MeshElement * elem,
                             std::vector<const SMDS_MeshNode*> nodes,
                             std::vector<int>                  quantities);
  void Renumber (const bool isNodes, const int startID=1, const int deltaID=1);

  void SetNodeInVolume(SMDS_MeshNode * aNode, const TopoDS_Shell & S);
  void SetNodeInVolume(SMDS_MeshNode * aNode, const TopoDS_Solid & S);
  void SetNodeOnFace(SMDS_MeshNode * aNode, const TopoDS_Face & S, double u=0., double v=0.);
  void SetNodeOnEdge(SMDS_MeshNode * aNode, const TopoDS_Edge & S, double u=0.);
  void SetNodeOnVertex(SMDS_MeshNode * aNode, const TopoDS_Vertex & S);
  void UnSetNodeOnShape(const SMDS_MeshNode * aNode);
  void SetMeshElementOnShape(const SMDS_MeshElement * anElt,
			     const TopoDS_Shape & S);
  void UnSetMeshElementOnShape(const SMDS_MeshElement * anElt,
			       const TopoDS_Shape & S);
  TopoDS_Shape ShapeToMesh() const;
  bool HasMeshElements(const TopoDS_Shape & S);
  SMESHDS_SubMesh * MeshElements(const TopoDS_Shape & S) const;
  SMESHDS_SubMesh * MeshElements(const int Index);
  std::list<int> SubMeshIndices();
  const std::map<int,SMESHDS_SubMesh*>& SubMeshes()
  { return myShapeIndexToSubMesh; }

  bool HasHypothesis(const TopoDS_Shape & S);
  const std::list<const SMESHDS_Hypothesis*>& GetHypothesis(const TopoDS_Shape & S) const;
  SMESHDS_Script * GetScript();
  void ClearScript();
  int ShapeToIndex(const TopoDS_Shape & aShape) const;
  TopoDS_Shape IndexToShape(int ShapeIndex);

  SMESHDS_SubMesh * NewSubMesh(int Index);
  int AddCompoundSubmesh(const TopoDS_Shape& S, TopAbs_ShapeEnum type = TopAbs_SHAPE);
  void SetNodeInVolume(const SMDS_MeshNode * aNode, int Index);
  void SetNodeOnFace(SMDS_MeshNode * aNode, int Index , double u=0., double v=0.);
  void SetNodeOnEdge(SMDS_MeshNode * aNode, int Index , double u=0.);
  void SetNodeOnVertex(SMDS_MeshNode * aNode, int Index);
  void SetMeshElementOnShape(const SMDS_MeshElement * anElt, int Index);

  void AddGroup (SMESHDS_GroupBase* theGroup)      { myGroups.insert(theGroup); }
  void RemoveGroup (SMESHDS_GroupBase* theGroup)   { myGroups.erase(theGroup); }
  int GetNbGroups() const                      { return myGroups.size(); }
  const std::set<SMESHDS_GroupBase*>& GetGroups() const { return myGroups; }

  bool IsGroupOfSubShapes (const TopoDS_Shape& aSubShape) const;

  ~SMESHDS_Mesh();
  
private:
#ifndef WNT
  struct HashTopoDS_Shape{
    size_t operator()(const TopoDS_Shape& S) const {
      return S.HashCode(2147483647);
    }
  };
#else
  typedef gstd::hash_compare< TopoDS_Shape, less<TopoDS_Shape> > HashTopoDS;

  class HashTopoDS_Shape : public HashTopoDS {
  public:
  
    size_t operator()(const TopoDS_Shape& S) const {
      return S.HashCode(2147483647);
    }

	bool operator()(const TopoDS_Shape& S1,const TopoDS_Shape& S2) const {
		return S1==S2;
	}
  };



#endif

  void addNodeToSubmesh( const SMDS_MeshNode* aNode, int Index )
  {
    //Update or build submesh
    map<int,SMESHDS_SubMesh*>::iterator it = myShapeIndexToSubMesh.find( Index );
    if ( it == myShapeIndexToSubMesh.end() )
      it = myShapeIndexToSubMesh.insert( make_pair(Index, new SMESHDS_SubMesh() )).first;
    it->second->AddNode( aNode ); // add aNode to submesh
  }

  typedef std::list<const SMESHDS_Hypothesis*> THypList;

#ifndef WNT
  typedef gstd::hash_map<TopoDS_Shape,THypList,HashTopoDS_Shape> ShapeToHypothesis;
#else
  typedef gstd::hash_map<TopoDS_Shape,THypList,HashTopoDS_Shape> ShapeToHypothesis;
#endif

  ShapeToHypothesis          myShapeToHypothesis;

  int                        myMeshID;
  TopoDS_Shape               myShape;

  typedef std::map<int,SMESHDS_SubMesh*> TShapeIndexToSubMesh;
  TShapeIndexToSubMesh myShapeIndexToSubMesh;

  TopTools_IndexedMapOfShape myIndexToShape;

  typedef std::set<SMESHDS_GroupBase*> TGroups;
  TGroups myGroups;

  SMESHDS_Script*            myScript;
};


#endif
