//  SMESH SMESH : implementaion of SMESH idl descriptions
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
//  File   : SMESH_Hypothesis.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_HYPOTHESIS_HXX_
#define _SMESH_HYPOTHESIS_HXX_

#include "SMESHDS_Hypothesis.hxx"

class SMESH_Gen;
class TopoDS_Shape;
class SMESH_Mesh;

class SMESH_Hypothesis: public SMESHDS_Hypothesis
{
public:
  enum Hypothesis_Status // in the order of severity
  {
    HYP_OK,
    HYP_MISSING,      // algo misses a hypothesis
    HYP_CONCURENT,    // several applicable hypotheses
    HYP_BAD_PARAMETER,// hypothesis has a bad parameter value
    HYP_UNKNOWN_FATAL,//  --- all statuses below should be considered as fatal
                      //      for Add/RemoveHypothesis operations
    HYP_INCOMPATIBLE, // hypothesis does not fit algo
    HYP_NOTCONFORM,   // not conform mesh is produced appling a hypothesis
    HYP_ALREADY_EXIST,// such hypothesis already exist
    HYP_BAD_DIM,      // bad dimension
    HYP_BAD_SUBSHAPE  // shape is neither the main one, nor its subshape, nor a group
  };
  static bool IsStatusFatal(Hypothesis_Status theStatus)
  { return theStatus >= HYP_UNKNOWN_FATAL; }

  SMESH_Hypothesis(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~SMESH_Hypothesis();
  virtual int GetDim() const;
  int GetStudyId() const;
  virtual void NotifySubMeshesHypothesisModification();
  virtual int GetShapeType() const;
  virtual const char* GetLibName() const;
  void  SetLibName(const char* theLibName);

  /*!
   * \brief Initialize my parameter values by the mesh built on the geometry
    * \param theMesh - the built mesh
    * \param theShape - the geometry of interest
    * \retval bool - true if parameter values have been successfully defined
   */
  virtual bool SetParametersByMesh(const SMESH_Mesh* theMesh, const TopoDS_Shape& theShape)=0;

  /*!
   * \brief Return true if me is an auxiliary hypothesis
    * \retval bool - auxiliary or not
   * 
   * An auxiliary hypothesis is optional, i.e. an algorithm
   * can work without it and another hypothesis of the same
   * dimention can be assigned to the shape
   */
  virtual bool IsAuxiliary() const
  { return GetType() == PARAM_ALGO && _param_algo_dim <= 0; }

protected:
  SMESH_Gen* _gen;
  int _studyId;
  int _shapeType;
  int _param_algo_dim;

private:
  std::string _libName;
};

#endif
