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
// File:      SMESH_MesherHelper.hxx
// Created:   15.02.06 14:48:09
// Author:    Sergey KUUL
// Copyright: Open CASCADE 2006


#ifndef SMESH_MesherHelper_HeaderFile
#define SMESH_MesherHelper_HeaderFile

#include <SMESH_Mesh.hxx>
#include <TopoDS_Shape.hxx>
#include <SMDS_MeshNode.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Pnt2d.hxx>
#include <SMDS_QuadraticEdge.hxx>

#include <map>

typedef pair<const SMDS_MeshNode*, const SMDS_MeshNode*> NLink;
typedef map<NLink, const SMDS_MeshNode*> NLinkNodeMap;
typedef map<NLink, const SMDS_MeshNode*>::iterator ItNLinkNode;

/*!
 * \brief It helps meshers to add elements
 *
 * It allow meshers not to care about creation of medium nodes
 * when filling a quadratic mesh. Helper does it itself.
 * It defines degree of elements to create when IsQuadraticSubMesh()
 * is called.
 */

class SMESH_MesherHelper
{
 public:
  // ---------- PUBLIC METHODS ----------

  /// Empty constructor
  SMESH_MesherHelper(SMESH_Mesh& theMesh)
    { myMesh=(void *)&theMesh; myCreateQuadratic = false; myShapeID=-1;}

  SMESH_Mesh* GetMesh() const
    { return (SMESH_Mesh*)myMesh; }
    
  /// Copy constructor
  //Standard_EXPORT SMESH_MesherHelper (const SMESH_MesherHelper& theOther);

  /// Destructor
  //Standard_EXPORT virtual ~SMESH_MesherHelper ();

  /**
   * Check submesh for given shape
   * Check if all elements on this shape
   * are quadratic, if yes => set true to myCreateQuadratic 
   * (default value is false). Also fill myNLinkNodeMap
   * Returns myCreateQuadratic
   */
  bool IsQuadraticSubMesh(const TopoDS_Shape& theShape);

  /*!
   * \brief Returns true if given node is medium
    * \param n - node to check
    * \param typeToCheck - type of elements containing the node to ask about node status
    * \retval bool - check result
   */
  static bool IsMedium(const SMDS_MeshNode*      node,
                       const SMDSAbs_ElementType typeToCheck = SMDSAbs_All);

  /**
   * Auxilary function for filling myNLinkNodeMap
   */
  void AddNLinkNode(const SMDS_MeshNode* n1,
                    const SMDS_MeshNode* n2,
                    const SMDS_MeshNode* n12);

  /**
   * Auxilary function for filling myNLinkNodeMap
   */
  void AddNLinkNodeMap(const NLinkNodeMap& aMap)
    { myNLinkNodeMap.insert(aMap.begin(), aMap.end()); }

  /**
   * Returns myNLinkNodeMap
   */
  const NLinkNodeMap& GetNLinkNodeMap() { return myNLinkNodeMap; }

  /*!
   * \brief Return node UV on face
    * \param F - the face
    * \param n - the node
    * \param inFaceNode - a node of element being created located inside a face
    * \retval gp_XY - resulting UV
   * 
   * Auxilary function called form GetMediumNode()
   */
  gp_XY GetNodeUV(const TopoDS_Face&   F,
                  const SMDS_MeshNode* n,
                  const SMDS_MeshNode* inFaceNode=0);

  /*!
   * \brief Check if inFaceNode argument is necessary for call GetNodeUV(F,..)
    * \param F - the face
    * \retval bool - return true if the face is periodic
    *
    * if F is Null, answer about subshape set through IsQuadraticSubMesh() or
    * SetSubShape()
   */
  bool GetNodeUVneedInFaceNode(const TopoDS_Face& F = TopoDS_Face()) const;

  /*!
   * \brief Return  U on edge
    * \param F - the edge
    * \param n - the node
    * \retval double - resulting U
   * 
   * Auxilary function called from GetMediumNode()
   */
  double GetNodeU(const TopoDS_Edge&  E,
                  const SMDS_MeshNode* n);


  /**
   * Special function for search or creation medium node
   */
  const SMDS_MeshNode* GetMediumNode(const SMDS_MeshNode* n1,
                                     const SMDS_MeshNode* n2,
                                     const bool force3d);

  /**
   * Special function for creation quadratic edge
   */
  SMDS_QuadraticEdge* AddQuadraticEdge(const SMDS_MeshNode* n1,
                                       const SMDS_MeshNode* n2,
                                       const int id = 0, 
				       const bool force3d = true);

  /**
   * Special function for creation quadratic triangle
   */
  SMDS_MeshFace* AddFace(const SMDS_MeshNode* n1,
                         const SMDS_MeshNode* n2,
                         const SMDS_MeshNode* n3,
                         const int id=0, 
			 const bool force3d = false);

  /**
   * Special function for creation quadratic quadrangle
   */
  SMDS_MeshFace* AddFace(const SMDS_MeshNode* n1,
                         const SMDS_MeshNode* n2,
                         const SMDS_MeshNode* n3,
                         const SMDS_MeshNode* n4,
                         const int id = 0,
			 const bool force3d = false);

  /**
   * Special function for creation quadratic tetraahedron
   */
  SMDS_MeshVolume* AddVolume(const SMDS_MeshNode* n1,
                             const SMDS_MeshNode* n2,
                             const SMDS_MeshNode* n3,
                             const SMDS_MeshNode* n4,
                             const int id = 0,
			     const bool force3d = true);

  /**
   * Special function for creation quadratic pentahedron
   */
  SMDS_MeshVolume* AddVolume(const SMDS_MeshNode* n1,
                             const SMDS_MeshNode* n2,
                             const SMDS_MeshNode* n3,
                             const SMDS_MeshNode* n4,
                             const SMDS_MeshNode* n5,
                             const SMDS_MeshNode* n6,
                             const int id = 0, 
			     const bool force3d = true);

  /**
   * Special function for creation quadratic hexahedron
   */
  SMDS_MeshVolume* AddVolume(const SMDS_MeshNode* n1,
                             const SMDS_MeshNode* n2,
                             const SMDS_MeshNode* n3,
                             const SMDS_MeshNode* n4,
                             const SMDS_MeshNode* n5,
                             const SMDS_MeshNode* n6,
                             const SMDS_MeshNode* n7,
                             const SMDS_MeshNode* n8,
                             const int id = 0, 
			     bool force3d = true);

  
  /*!
   * \brief Set order of elements to create
    * \param theBuildQuadratic - to build quadratic or not
   * 
   * To be used for quadratic elements creation without preceding
   * IsQuadraticSubMesh() or AddQuadraticEdge() call
   */
  void SetKeyIsQuadratic(const bool theBuildQuadratic)
  { myCreateQuadratic = theBuildQuadratic; }

  /*!
   * \brief Set shape to make elements on
    * \param subShape, subShapeID - shape or its ID (==SMESHDS_Mesh::ShapeToIndex(shape))
   */
  void SetSubShape(const int           subShapeID);
  void SetSubShape(const TopoDS_Shape& subShape);

  /*!
   * \brief Return shape or its ID, on which created elements are added
    * \retval TopoDS_Shape, int - shape or its ID
   */
  int          GetSubShapeID() { return myShapeID; }
  TopoDS_Shape GetSubShape()   { return myShape; }

 protected:

  /*!
   * \brief Select UV on either of 2 pcurves of a seam edge, closest to the given UV
    * \param uv1 - UV on the seam
    * \param uv2 - UV within a face
    * \retval gp_Pnt2d - selected UV
   */
  gp_Pnt2d GetUVOnSeam( const gp_Pnt2d& uv1, const gp_Pnt2d& uv2 ) const;

 private:

  void* myMesh;

  int myShapeID;

  // Key for creation quadratic faces
  bool myCreateQuadratic;

  // special map for using during creation quadratic faces
  NLinkNodeMap myNLinkNodeMap;

  std::set< int > mySeamShapeIds;
  double          myPar1, myPar2; // bounds of a closed periodic surface
  int             myParIndex;     // bounds' index (1-U, 2-V)
  TopoDS_Shape    myShape;

};


#endif
