//  SMESH SMESH : implementaion of SMESH idl descriptions
//
//  Copyright (C) 2003  CEA
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : StdMeshers_Propagation.cxx
//  Module : SMESH
//  $Header$

#include "StdMeshers_Propagation.hxx"

#include "utilities.h"

using namespace std;

//=============================================================================
/*!
 *
 */
//=============================================================================
StdMeshers_Propagation::StdMeshers_Propagation (int hypId, int studyId,
                                                SMESH_Gen * gen)
     : SMESH_Hypothesis(hypId, studyId, gen)
{
  _name = GetName();
  _param_algo_dim = -1; // 1D auxiliary
}

//=============================================================================
/*!
 *
 */
//=============================================================================
StdMeshers_Propagation::~StdMeshers_Propagation()
{
}

//=============================================================================
/*!
 *
 */
//=============================================================================

ostream & StdMeshers_Propagation::SaveTo (ostream & save)
{
  return save;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
istream & StdMeshers_Propagation::LoadFrom (istream & load)
{
  return load;
}

//=============================================================================
/*!
 *
 */
//=============================================================================
ostream & operator << (ostream & save, StdMeshers_Propagation & hyp)
{
  return hyp.SaveTo(save);
}

//=============================================================================
/*!
 *
 */
//=============================================================================
istream & operator >> (istream & load, StdMeshers_Propagation & hyp)
{
  return hyp.LoadFrom(load);
}

//=============================================================================
/*!
 *  GetName
 */
//=============================================================================
std::string StdMeshers_Propagation::GetName ()
{
  return "Propagation";
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

bool StdMeshers_Propagation::SetParametersByMesh(const SMESH_Mesh* /*theMesh*/,
                                                 const TopoDS_Shape& /*theShape*/)
{
  return false;
}
