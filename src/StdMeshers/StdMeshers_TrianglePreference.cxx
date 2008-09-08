//  SMESH StdMeshers_TrianglePreference 
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : StdMeshers_TrianglePreference.cxx
//  Module : SMESH
//  $Header$

#include "StdMeshers_TrianglePreference.hxx"
#include "utilities.h"

using namespace std;

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_TrianglePreference::StdMeshers_TrianglePreference(int         hypId,
                                                                 int         studyId,
                                                                 SMESH_Gen * gen)
     :SMESH_Hypothesis(hypId, studyId, gen)
{
  _name = "TrianglePreference";
  _param_algo_dim = -2; // auxiliary used by StdMeshers_Quadrangle_2D
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_TrianglePreference::~StdMeshers_TrianglePreference()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & StdMeshers_TrianglePreference::SaveTo(ostream & save)
{
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & StdMeshers_TrianglePreference::LoadFrom(istream & load)
{
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator <<(ostream & save, StdMeshers_TrianglePreference & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, StdMeshers_TrianglePreference & hyp)
{
  return hyp.LoadFrom( load );
}
//================================================================================
/*!
 * \brief Initialize my parameter values by the mesh built on the geometry
 * \param theMesh - the built mesh
 * \param theShape - the geometry of interest
 * \retval bool - true if parameter values have been successfully defined
 *
 * Just return false as this hypothesis does not have parameters values
 */
//================================================================================

bool StdMeshers_TrianglePreference::SetParametersByMesh(const SMESH_Mesh* /*theMesh*/,
                                                          const TopoDS_Shape& /*theShape*/)
{
  return false;
}
