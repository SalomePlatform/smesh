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

#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Handle_SMESHDS_Mesh_HeaderFile
#include "Handle_SMESHDS_Mesh.hxx"
#endif

#ifndef _Standard_Integer_HeaderFile
#include <Standard_Integer.hxx>
#endif
#ifndef _TopoDS_Shape_HeaderFile
#include <TopoDS_Shape.hxx>
#endif
#ifndef _TopoDS_Shell_HeaderFile
#include <TopoDS_Shell.hxx>
#endif
#ifndef _TopTools_IndexedMapOfShape_HeaderFile
#include <TopTools_IndexedMapOfShape.hxx>
#endif
#ifndef _SMESHDS_DataMapOfIntegerSubMesh_HeaderFile
#include "SMESHDS_DataMapOfIntegerSubMesh.hxx"
#endif
#ifndef _SMESHDS_DataMapOfShapeListOfPtrHypothesis_HeaderFile
#include "SMESHDS_DataMapOfShapeListOfPtrHypothesis.hxx"
#endif
#ifndef _Handle_SMESHDS_Script_HeaderFile
#include "Handle_SMESHDS_Script.hxx"
#endif
#ifndef _SMDS_Mesh_HeaderFile
#include "SMDS_Mesh.hxx"
#endif
#ifndef _Standard_Boolean_HeaderFile
#include <Standard_Boolean.hxx>
#endif
#ifndef _SMESHDS_PtrHypothesis_HeaderFile
#include "SMESHDS_PtrHypothesis.hxx"
#endif
#ifndef _Standard_Real_HeaderFile
#include <Standard_Real.hxx>
#endif
#ifndef _Handle_SMDS_MeshNode_HeaderFile
#include "Handle_SMDS_MeshNode.hxx"
#endif
#ifndef _Handle_SMDS_MeshElement_HeaderFile
#include "Handle_SMDS_MeshElement.hxx"
#endif
#ifndef _Handle_SMESHDS_SubMesh_HeaderFile
#include "Handle_SMESHDS_SubMesh.hxx"
#endif
class SMESHDS_Script;
class TopoDS_Shape;
class SMDS_MeshNode;
class TopoDS_Shell;
class TopoDS_Face;
class TopoDS_Edge;
class TopoDS_Vertex;
class SMDS_MeshElement;
class SMESHDS_SubMesh;
class SMESHDS_ListOfPtrHypothesis;


class SMESHDS_Mesh : public SMDS_Mesh {

public:

    inline void* operator new(size_t,void* anAddress) 
      {
        return anAddress;
      }
    inline void* operator new(size_t size) 
      { 
        return Standard::Allocate(size); 
      }
    inline void  operator delete(void *anAddress) 
      { 
        if (anAddress) Standard::Free((Standard_Address&)anAddress); 
      }
//    inline void  operator delete(void *anAddress, size_t size) 
//      { 
//        if (anAddress) Standard::Free((Standard_Address&)anAddress,size); 
//      }
 // Methods PUBLIC
 // 
Standard_EXPORT SMESHDS_Mesh(const Standard_Integer MeshID);
Standard_EXPORT   void ShapeToMesh(const TopoDS_Shape& S) ;
Standard_EXPORT   Standard_Boolean AddHypothesis(const TopoDS_Shape& SS,const SMESHDS_PtrHypothesis& H) ;
Standard_EXPORT   Standard_Boolean RemoveHypothesis(const TopoDS_Shape& S,const SMESHDS_PtrHypothesis& H) ;
Standard_EXPORT virtual  Standard_Integer AddNode(const Standard_Real x,const Standard_Real y,const Standard_Real z) ;
Standard_EXPORT virtual  void RemoveNode(const Standard_Integer ID) ;
Standard_EXPORT   void MoveNode(const Standard_Integer ID,const Standard_Real x,const Standard_Real y,const Standard_Real z) ;
Standard_EXPORT virtual  Standard_Integer AddEdge(const Standard_Integer idnode1,const Standard_Integer idnode2) ;
Standard_EXPORT virtual  Standard_Integer AddFace(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3) ;
Standard_EXPORT virtual  Standard_Integer AddFace(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4) ;
Standard_EXPORT virtual  Standard_Integer AddVolume(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4) ;
Standard_EXPORT virtual  Standard_Integer AddVolume(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5) ;
Standard_EXPORT virtual  Standard_Integer AddVolume(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5,const Standard_Integer idnode6) ;
Standard_EXPORT virtual  Standard_Integer AddVolume(const Standard_Integer idnode1,const Standard_Integer idnode2,const Standard_Integer idnode3,const Standard_Integer idnode4,const Standard_Integer idnode5,const Standard_Integer idnode6,const Standard_Integer idnode7,const Standard_Integer idnode8) ;
Standard_EXPORT virtual  void RemoveElement(const Standard_Integer IDelem) ;
Standard_EXPORT   void SetNodeInVolume(const Handle(SMDS_MeshNode)& aNode,const TopoDS_Shell& S) ;
Standard_EXPORT   void SetNodeOnFace(const Handle(SMDS_MeshNode)& aNode,const TopoDS_Face& S) ;
Standard_EXPORT   void SetNodeOnEdge(const Handle(SMDS_MeshNode)& aNode,const TopoDS_Edge& S) ;
Standard_EXPORT   void SetNodeOnVertex(const Handle(SMDS_MeshNode)& aNode,const TopoDS_Vertex& S) ;
Standard_EXPORT   void UnSetNodeOnShape(const Handle(SMDS_MeshNode)& aNode) ;
Standard_EXPORT   void SetMeshElementOnShape(const Handle(SMDS_MeshElement)& anElt,const TopoDS_Shape& S) ;
Standard_EXPORT   void UnSetMeshElementOnShape(const Handle(SMDS_MeshElement)& anElt,const TopoDS_Shape& S) ;
Standard_EXPORT   TopoDS_Shape ShapeToMesh() ;
Standard_EXPORT   Standard_Boolean HasMeshElements(const TopoDS_Shape& S) ;
Standard_EXPORT   Handle_SMESHDS_SubMesh MeshElements(const TopoDS_Shape& S) ;
Standard_EXPORT   Standard_Boolean HasHypothesis(const TopoDS_Shape& S) ;
Standard_EXPORT  const SMESHDS_ListOfPtrHypothesis& GetHypothesis(const TopoDS_Shape& S) ;
Standard_EXPORT  const Handle_SMESHDS_Script& GetScript() ;
Standard_EXPORT   void ClearScript() ;
Standard_EXPORT   Standard_Integer ShapeToIndex(const TopoDS_Shape& aShape) ;
Standard_EXPORT   TopoDS_Shape IndexToShape(const Standard_Integer ShapeIndex) ;
Standard_EXPORT   void NewSubMesh(const Standard_Integer Index) ;
Standard_EXPORT   void SetNodeInVolume(const Handle(SMDS_MeshNode)& aNode,const Standard_Integer Index) ;
Standard_EXPORT   void SetNodeOnFace(const Handle(SMDS_MeshNode)& aNode,const Standard_Integer Index) ;
Standard_EXPORT   void SetNodeOnEdge(const Handle(SMDS_MeshNode)& aNode,const Standard_Integer Index) ;
Standard_EXPORT   void SetNodeOnVertex(const Handle(SMDS_MeshNode)& aNode,const Standard_Integer Index) ;
Standard_EXPORT   void SetMeshElementOnShape(const Handle(SMDS_MeshElement)& anElt,const Standard_Integer Index) ;
Standard_EXPORT ~SMESHDS_Mesh();




 // Type management
 //
 Standard_EXPORT friend Handle_Standard_Type& SMESHDS_Mesh_Type_();
 Standard_EXPORT const Handle(Standard_Type)& DynamicType() const;
 Standard_EXPORT Standard_Boolean	       IsKind(const Handle(Standard_Type)&) const;

protected:

 // Methods PROTECTED
 // 


 // Fields PROTECTED
 //


private: 

 // Methods PRIVATE
 // 


 // Fields PRIVATE
 //
Standard_Integer myMeshID;
TopoDS_Shape myShape;
TopTools_IndexedMapOfShape myIndexToShape;
SMESHDS_DataMapOfIntegerSubMesh myShapeIndexToSubMesh;
SMESHDS_DataMapOfShapeListOfPtrHypothesis myShapeToHypothesis;
Handle_SMESHDS_Script myScript;


};





// other inline functions and methods (like "C++: function call" methods)
//


#endif
