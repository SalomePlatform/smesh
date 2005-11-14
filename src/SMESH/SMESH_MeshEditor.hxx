//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
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
// File      : SMESH_MeshEditor.hxx
// Created   : Mon Apr 12 14:56:19 2004
// Author    : Edward AGAPOV (eap)
// Module    : SMESH


#ifndef SMESH_MeshEditor_HeaderFile
#define SMESH_MeshEditor_HeaderFile

#include "SMESH_Mesh.hxx"
#include "SMESH_Controls.hxx"
#include "SMESH_SequenceOfNode.hxx"
#include "gp_Dir.hxx"
#include "TColStd_HSequenceOfReal.hxx"

#include <list>
#include <map>

typedef map<const SMDS_MeshElement*,
            list<const SMDS_MeshElement*> > TElemOfElemListMap;

class SMDS_MeshElement;
class SMDS_MeshFace;
class SMDS_MeshNode;
class gp_Ax1;
class gp_Vec;
class gp_Pnt;

class SMESH_MeshEditor {
 public:

  SMESH_MeshEditor( SMESH_Mesh* theMesh );

  bool Remove (const std::list< int >& theElemIDs, const bool isNodes);
  // Remove a node or an element.
  // Modify a compute state of sub-meshes which become empty

  bool InverseDiag (const SMDS_MeshElement * theTria1,
                    const SMDS_MeshElement * theTria2 );
  // Replace two neighbour triangles with ones built on the same 4 nodes
  // but having other common link.
  // Return False if args are improper

  bool InverseDiag (const SMDS_MeshNode * theNode1,
                    const SMDS_MeshNode * theNode2 );
  // Replace two neighbour triangles sharing theNode1-theNode2 link
  // with ones built on the same 4 nodes but having other common link.
  // Return false if proper faces not found

  bool DeleteDiag (const SMDS_MeshNode * theNode1,
                   const SMDS_MeshNode * theNode2 );
  // Replace two neighbour triangles sharing theNode1-theNode2 link
  // with a quadrangle built on the same 4 nodes.
  // Return false if proper faces not found

  bool Reorient (const SMDS_MeshElement * theElement);
  // Reverse theElement orientation


  /*!
   * \brief Fuse neighbour triangles into quadrangles.
   * \param theElems     - The triangles to be fused.
   * \param theCriterion - Is used to choose a neighbour to fuse with.
   * \param theMaxAngle  - Is a max angle between element normals at which fusion
   *                       is still performed; theMaxAngle is mesured in radians.
   * \retval bool - Success or not.
   */
  bool TriToQuad (std::set<const SMDS_MeshElement*> &  theElems,
                  SMESH::Controls::NumericalFunctorPtr theCriterion,
                  const double                         theMaxAngle);

  /*!
   * \brief Split quadrangles into triangles.
   * \param theElems     - The faces to be splitted.
   * \param theCriterion - Is used to choose a diagonal for splitting.
   * \retval bool - Success or not.
   */
  bool QuadToTri (std::set<const SMDS_MeshElement*> &  theElems,
                  SMESH::Controls::NumericalFunctorPtr theCriterion);

  /*!
   * \brief Split quadrangles into triangles.
   * \param theElems  - The faces to be splitted.
   * \param the13Diag - Is used to choose a diagonal for splitting.
   * \retval bool - Success or not.
   */
  bool QuadToTri (std::set<const SMDS_MeshElement*> & theElems,
                  const bool                          the13Diag);

  /*!
   * \brief Find better diagonal for splitting.
   * \param theQuad      - The face to find better splitting of.
   * \param theCriterion - Is used to choose a diagonal for splitting.
   * \retval int - 1 for 1-3 diagonal, 2 for 2-4, -1 - for errors.
   */
  int BestSplit (const SMDS_MeshElement*              theQuad,
                 SMESH::Controls::NumericalFunctorPtr theCriterion);


  enum SmoothMethod { LAPLACIAN = 0, CENTROIDAL };

  void Smooth (std::set<const SMDS_MeshElement*> & theElements,
               std::set<const SMDS_MeshNode*> &    theFixedNodes,
               const SmoothMethod                  theSmoothMethod,
               const int                           theNbIterations,
               double                              theTgtAspectRatio = 1.0,
               const bool                          the2D = true);
  // Smooth theElements using theSmoothMethod during theNbIterations
  // or until a worst element has aspect ratio <= theTgtAspectRatio.
  // Aspect Ratio varies in range [1.0, inf].
  // If theElements is empty, the whole mesh is smoothed.
  // theFixedNodes contains additionally fixed nodes. Nodes built
  // on edges and boundary nodes are always fixed.
  // If the2D, smoothing is performed using UV parameters of nodes
  // on geometrical faces


  void RotationSweep (std::set<const SMDS_MeshElement*> & theElements,
                      const gp_Ax1&                       theAxis,
                      const double                        theAngle,
                      const int                           theNbSteps,
                      const double                        theToler);
  // Generate new elements by rotation of theElements around theAxis
  // by theAngle by theNbSteps

  /*!
   * Auxilary flag for advanced extrusion.
   * BOUNDARY: create or not boundary for result of extrusion
   * SEW:      try to use existing nodes or create new nodes in any case
   */
  enum ExtrusionFlags {
    EXTRUSION_FLAG_BOUNDARY = 0x01,
    EXTRUSION_FLAG_SEW = 0x02
  };
  
  /*!
   * special structire for control of extrusion functionality
   */
  struct ExtrusParam {
    gp_Dir myDir; // direction of extrusion
    Handle(TColStd_HSequenceOfReal) mySteps; // magnitudes for each step
    SMESH_SequenceOfNode myNodes; // nodes for using in sewing
  };

  /*!
   * Create new node in the mesh with given coordinates
   * (auxilary for advanced extrusion)
   */
  const SMDS_MeshNode* CreateNode(const double x,
                                  const double y,
                                  const double z,
                                  const double tolnode,
                                  SMESH_SequenceOfNode& aNodes);

  /*!
   * Generate new elements by extrusion of theElements
   * It is a method used in .idl file. All functionality
   * is implemented in the next method (see below) which
   * is used in the cuurent method.
   * param theElems - list of elements for extrusion
   * param newElemsMap returns history of extrusion
   * param theFlags set flags for performing extrusion (see description
   *   of enum ExtrusionFlags for additional information)
   * param theTolerance - uses for comparing locations of nodes if flag
   *   EXTRUSION_FLAG_SEW is set
   */
  void ExtrusionSweep
           (set<const SMDS_MeshElement*> & theElems,
            const gp_Vec&                  theStep,
            const int                      theNbSteps,
            TElemOfElemListMap&            newElemsMap,
            const int                      theFlags = EXTRUSION_FLAG_BOUNDARY,
            const double                   theTolerance = 1.e-6);
  
  /*!
   * Generate new elements by extrusion of theElements
   * param theElems - list of elements for extrusion
   * param newElemsMap returns history of extrusion
   * param theFlags set flags for performing extrusion (see description
   *   of enum ExtrusionFlags for additional information)
   * param theTolerance - uses for comparing locations of nodes if flag
   *   EXTRUSION_FLAG_SEW is set
   * param theParams - special structure for manage of extrusion
   */
  void ExtrusionSweep (set<const SMDS_MeshElement*> & theElems,
                       ExtrusParam&                   theParams,
                       TElemOfElemListMap&            newElemsMap,
                       const int                      theFlags,
                       const double                   theTolerance);


  // Generate new elements by extrusion of theElements 
  // by theStep by theNbSteps

  enum Extrusion_Error {
    EXTR_OK,
    EXTR_NO_ELEMENTS, 
    EXTR_PATH_NOT_EDGE,
    EXTR_BAD_PATH_SHAPE,
    EXTR_BAD_STARTING_NODE,
    EXTR_BAD_ANGLES_NUMBER,
    EXTR_CANT_GET_TANGENT
    };
  
  Extrusion_Error ExtrusionAlongTrack (std::set<const SMDS_MeshElement*> & theElements,
                                       SMESH_subMesh*                      theTrackPattern,
                                       const SMDS_MeshNode*                theNodeStart,
                                       const bool                          theHasAngles,
                                       std::list<double>&                  theAngles,
                                       const bool                          theHasRefPoint,
                                       const gp_Pnt&                       theRefPoint);
  // Generate new elements by extrusion of theElements along path given by theTrackPattern,
  // theHasAngles are the rotation angles, base point can be given by theRefPoint

  void Transform (std::set<const SMDS_MeshElement*> & theElements,
                  const gp_Trsf&                      theTrsf,
                  const bool                          theCopy);
  // Move or copy theElements applying theTrsf to their nodes

  typedef std::list< std::list< const SMDS_MeshNode* > > TListOfListOfNodes;

  void FindCoincidentNodes (std::set<const SMDS_MeshNode*> & theNodes,
                            const double                     theTolerance,
                            TListOfListOfNodes &             theGroupsOfNodes);
  // Return list of group of nodes close to each other within theTolerance.
  // Search among theNodes or in the whole mesh if theNodes is empty.

  int SimplifyFace (const vector<const SMDS_MeshNode *> faceNodes,
                    vector<const SMDS_MeshNode *>&      poly_nodes,
                    vector<int>&                        quantities) const;
  // Split face, defined by <faceNodes>, into several faces by repeating nodes.
  // Is used by MergeNodes()

  void MergeNodes (TListOfListOfNodes & theNodeGroups);
  // In each group, the cdr of nodes are substituted by the first one
  // in all elements.

  void MergeEqualElements();
  // Remove all but one of elements built on the same nodes.
  // Return nb of successfully merged groups.

  static bool CheckFreeBorderNodes(const SMDS_MeshNode* theNode1,
                                   const SMDS_MeshNode* theNode2,
                                   const SMDS_MeshNode* theNode3 = 0);
  // Return true if the three nodes are on a free border

  enum Sew_Error {
    SEW_OK,
    // for SewFreeBorder()
    SEW_BORDER1_NOT_FOUND,
    SEW_BORDER2_NOT_FOUND,
    SEW_BOTH_BORDERS_NOT_FOUND,
    SEW_BAD_SIDE_NODES,
    SEW_VOLUMES_TO_SPLIT,
    // for SewSideElements()
    SEW_DIFF_NB_OF_ELEMENTS,
    SEW_TOPO_DIFF_SETS_OF_ELEMENTS,
    SEW_BAD_SIDE1_NODES,
    SEW_BAD_SIDE2_NODES
    };
    

  Sew_Error SewFreeBorder (const SMDS_MeshNode* theBorderFirstNode,
                           const SMDS_MeshNode* theBorderSecondNode,
                           const SMDS_MeshNode* theBorderLastNode,
                           const SMDS_MeshNode* theSide2FirstNode,
                           const SMDS_MeshNode* theSide2SecondNode,
                           const SMDS_MeshNode* theSide2ThirdNode = 0,
                           const bool           theSide2IsFreeBorder = true,
                           const bool           toCreatePolygons = false,
                           const bool           toCreatePolyedrs = false);
  // Sew the free border to the side2 by replacing nodes in
  // elements on the free border with nodes of the elements
  // of the side 2. If nb of links in the free border and
  // between theSide2FirstNode and theSide2LastNode are different,
  // additional nodes are inserted on a link provided that no
  // volume elements share the splitted link.
  // The side 2 is a free border if theSide2IsFreeBorder == true.
  // Sewing is peformed between the given first, second and last
  // nodes on the sides.
  // theBorderFirstNode is merged with theSide2FirstNode.
  // if (!theSide2IsFreeBorder) then theSide2SecondNode gives
  // the last node on the side 2, which will be merged with
  // theBorderLastNode.
  // if (theSide2IsFreeBorder) then theSide2SecondNode will
  // be merged with theBorderSecondNode.
  // if (theSide2IsFreeBorder && theSide2ThirdNode == 0) then
  // the 2 free borders are sewn link by link and no additional
  // nodes are inserted.
  // Return false, if sewing failed.

  Sew_Error SewSideElements (std::set<const SMDS_MeshElement*>& theSide1,
                             std::set<const SMDS_MeshElement*>& theSide2,
                             const SMDS_MeshNode*               theFirstNode1ToMerge,
                             const SMDS_MeshNode*               theFirstNode2ToMerge,
                             const SMDS_MeshNode*               theSecondNode1ToMerge,
                             const SMDS_MeshNode*               theSecondNode2ToMerge);
  // Sew two sides of a mesh. Nodes belonging to theSide1 are
  // merged with nodes of elements of theSide2.
  // Number of elements in theSide1 and in theSide2 must be
  // equal and they should have similar node connectivity.
  // The nodes to merge should belong to side s borders and
  // the first node should be linked to the second.

  void InsertNodesIntoLink(const SMDS_MeshElement*          theFace,
                           const SMDS_MeshNode*             theBetweenNode1,
                           const SMDS_MeshNode*             theBetweenNode2,
                           std::list<const SMDS_MeshNode*>& theNodesToInsert,
                           const bool                       toCreatePoly = false);
  // insert theNodesToInsert into theFace between theBetweenNode1 and theBetweenNode2.
  // If toCreatePoly is true, replace theFace by polygon, else split theFace.

  void UpdateVolumes (const SMDS_MeshNode*             theBetweenNode1,
                      const SMDS_MeshNode*             theBetweenNode2,
                      std::list<const SMDS_MeshNode*>& theNodesToInsert);
  // insert theNodesToInsert into all volumes, containing link
  // theBetweenNode1 - theBetweenNode2, between theBetweenNode1 and theBetweenNode2.

//  static int SortQuadNodes (const SMDS_Mesh * theMesh,
//                            int               theNodeIds[] );
//  // Set 4 nodes of a quadrangle face in a good order.
//  // Swap 1<->2 or 2<->3 nodes and correspondingly return
//  // 1 or 2 else 0.
//
//  static bool SortHexaNodes (const SMDS_Mesh * theMesh,
//                             int               theNodeIds[] );
//  // Set 8 nodes of a hexahedron in a good order.
//  // Return success status

  static void AddToSameGroups (const SMDS_MeshElement* elemToAdd,
                               const SMDS_MeshElement* elemInGroups,
                               SMESHDS_Mesh *          aMesh);
  // Add elemToAdd to the groups the elemInGroups belongs to

  static const SMDS_MeshElement*
    FindFaceInSet(const SMDS_MeshNode*                     n1,
                  const SMDS_MeshNode*                     n2,
                  const std::set<const SMDS_MeshElement*>& elemSet,
                  const std::set<const SMDS_MeshElement*>& avoidSet);
  // Return a face having linked nodes n1 and n2 and which is
  // - not in avoidSet,
  // - in elemSet provided that !elemSet.empty()

  int FindShape (const SMDS_MeshElement * theElem);
  // Return an index of the shape theElem is on
  // or zero if a shape not found


  SMESH_Mesh * GetMesh() { return myMesh; }

  SMESHDS_Mesh * GetMeshDS() { return myMesh->GetMeshDS(); }

 private:

  SMESH_Mesh * myMesh;

};

#endif
