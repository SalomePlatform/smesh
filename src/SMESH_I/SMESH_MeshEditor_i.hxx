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
//  File   : SMESH_MeshEditor_i.hxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef _SMESH_MESHEDITOR_I_HXX_
#define _SMESH_MESHEDIOTR_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include "SMESH_Mesh.hxx"

class SMESH_MeshEditor_i: public POA_SMESH::SMESH_MeshEditor
{
 public:
  SMESH_MeshEditor_i(SMESH_Mesh * theMesh);

  virtual ~ SMESH_MeshEditor_i()
  {
  };

  // --- CORBA
  CORBA::Boolean RemoveElements(const SMESH::long_array & IDsOfElements);
  CORBA::Boolean RemoveNodes(const SMESH::long_array & IDsOfNodes);

  CORBA::Boolean AddNode(CORBA::Double x, CORBA::Double y, CORBA::Double z);
  CORBA::Boolean AddEdge(const SMESH::long_array & IDsOfNodes);
  CORBA::Boolean AddFace(const SMESH::long_array & IDsOfNodes);
  CORBA::Boolean AddVolume(const SMESH::long_array & IDsOfNodes);
  CORBA::Boolean MoveNode(CORBA::Long NodeID,
                          CORBA::Double x, CORBA::Double y, CORBA::Double z);

  CORBA::Boolean InverseDiag(CORBA::Long NodeID1, CORBA::Long NodeID2);
  CORBA::Boolean DeleteDiag(CORBA::Long NodeID1, CORBA::Long NodeID2);
  CORBA::Boolean Reorient(const SMESH::long_array & IDsOfElements);
  CORBA::Boolean ReorientObject(SMESH::SMESH_IDSource_ptr theObject);

  CORBA::Boolean TriToQuad(const SMESH::long_array &   IDsOfElements,
                           SMESH::NumericalFunctor_ptr Criterion,
                           CORBA::Double               MaxAngle);
  CORBA::Boolean TriToQuadObject(SMESH::SMESH_IDSource_ptr theObject,
				 SMESH::NumericalFunctor_ptr Criterion,
				 CORBA::Double               MaxAngle);
  CORBA::Boolean QuadToTri(const SMESH::long_array &   IDsOfElements,
                           SMESH::NumericalFunctor_ptr Criterion);
  CORBA::Boolean SplitQuad(const SMESH::long_array & IDsOfElements,
                           CORBA::Boolean            Diag13);
  CORBA::Boolean SplitQuadObject(SMESH::SMESH_IDSource_ptr theObject,
				 CORBA::Boolean            Diag13);

  CORBA::Boolean Smooth(const SMESH::long_array &              IDsOfElements,
                        const SMESH::long_array &              IDsOfFixedNodes,
                        CORBA::Long                            MaxNbOfIterations,
                        CORBA::Double                          MaxAspectRatio,
                        SMESH::SMESH_MeshEditor::Smooth_Method Method);
  CORBA::Boolean SmoothObject(SMESH::SMESH_IDSource_ptr              theObject,
			      const SMESH::long_array &              IDsOfFixedNodes,
			      CORBA::Long                            MaxNbOfIterations,
			      CORBA::Double                          MaxAspectRatio,
			      SMESH::SMESH_MeshEditor::Smooth_Method Method);


  void RenumberNodes();
  void RenumberElements();

  void RotationSweep(const SMESH::long_array & IDsOfElements,
                     const SMESH::AxisStruct & Axis,
                     CORBA::Double             AngleInRadians,
                     CORBA::Long               NbOfSteps,
                     CORBA::Double             Tolerance);
  void RotationSweepObject(SMESH::SMESH_IDSource_ptr theObject,
			   const SMESH::AxisStruct & Axis,
			   CORBA::Double             AngleInRadians,
			   CORBA::Long               NbOfSteps,
			   CORBA::Double             Tolerance);

  void ExtrusionSweep(const SMESH::long_array & IDsOfElements,
                      const SMESH::DirStruct &  StepVector,
                      CORBA::Long               NbOfSteps);
  void ExtrusionSweepObject(SMESH::SMESH_IDSource_ptr theObject,
			    const SMESH::DirStruct &  StepVector,
			    CORBA::Long               NbOfSteps);
  
  void ExtrusionAlongPath(const SMESH::long_array &   IDsOfElements,
                          SMESH::SMESH_Mesh_ptr       PathMesh,
                          GEOM::GEOM_Object_ptr       PathShape,
                          CORBA::Long                 NodeStart,
                          CORBA::Boolean              HasAngles,
                          const SMESH::double_array & Angles,
                          CORBA::Boolean              HasRefPoint,
                          const SMESH::PointStruct &  RefPoint);

  void ExtrusionAlongPathObject(SMESH::SMESH_IDSource_ptr   theObject,
                                SMESH::SMESH_Mesh_ptr       PathMesh,
                                GEOM::GEOM_Object_ptr       PathShape,
                                CORBA::Long                 NodeStart,
                                CORBA::Boolean              HasAngles,
                                const SMESH::double_array & Angles,
                                CORBA::Boolean              HasRefPoint,
                                const SMESH::PointStruct &  RefPoint);

  void Mirror(const SMESH::long_array &           IDsOfElements,
              const SMESH::AxisStruct &           Axis,
              SMESH::SMESH_MeshEditor::MirrorType MirrorType,
              CORBA::Boolean                      Copy);
  void MirrorObject(SMESH::SMESH_IDSource_ptr           theObject,
		    const SMESH::AxisStruct &           Axis,
		    SMESH::SMESH_MeshEditor::MirrorType MirrorType,
		    CORBA::Boolean                      Copy);
  void Translate(const SMESH::long_array & IDsOfElements,
                 const SMESH::DirStruct &   Vector,
                 CORBA::Boolean            Copy);
  void TranslateObject(SMESH::SMESH_IDSource_ptr  theObject,
		       const SMESH::DirStruct &   Vector,
		       CORBA::Boolean             Copy);
  void Rotate(const SMESH::long_array & IDsOfElements,
              const SMESH::AxisStruct &  Axis,
              CORBA::Double             Angle,
              CORBA::Boolean            Copy);
  void RotateObject(SMESH::SMESH_IDSource_ptr  theObject,
		    const SMESH::AxisStruct &  Axis,
		    CORBA::Double              Angle,
		    CORBA::Boolean             Copy);

  void FindCoincidentNodes (CORBA::Double                  Tolerance,
                            SMESH::array_of_long_array_out GroupsOfNodes);
  void MergeNodes (const SMESH::array_of_long_array& GroupsOfNodes);
  void MergeEqualElements();

  SMESH::SMESH_MeshEditor::Sew_Error
    SewFreeBorders(CORBA::Long FirstNodeID1,
                   CORBA::Long SecondNodeID1,
                   CORBA::Long LastNodeID1,
                   CORBA::Long FirstNodeID2,
                   CORBA::Long SecondNodeID2,
                   CORBA::Long LastNodeID2);
  SMESH::SMESH_MeshEditor::Sew_Error
    SewConformFreeBorders(CORBA::Long FirstNodeID1,
                          CORBA::Long SecondNodeID1,
                          CORBA::Long LastNodeID1,
                          CORBA::Long FirstNodeID2,
                          CORBA::Long SecondNodeID2);
  SMESH::SMESH_MeshEditor::Sew_Error
    SewBorderToSide(CORBA::Long FirstNodeIDOnFreeBorder,
                    CORBA::Long SecondNodeIDOnFreeBorder,
                    CORBA::Long LastNodeIDOnFreeBorder,
                    CORBA::Long FirstNodeIDOnSide,
                    CORBA::Long LastNodeIDOnSide);
  SMESH::SMESH_MeshEditor::Sew_Error
    SewSideElements(const SMESH::long_array& IDsOfSide1Elements,
                    const SMESH::long_array& IDsOfSide2Elements,
                    CORBA::Long NodeID1OfSide1ToMerge,
                    CORBA::Long NodeID1OfSide2ToMerge,
                    CORBA::Long NodeID2OfSide1ToMerge,
                    CORBA::Long NodeID2OfSide2ToMerge);

 private:
  SMESHDS_Mesh * GetMeshDS() { return _myMesh->GetMeshDS(); }
  SMESH_Mesh   *_myMesh;
};

#endif
