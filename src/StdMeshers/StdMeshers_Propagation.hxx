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
//  File   : StdMeshers_Propagation.hxx
//  Module : SMESH
//  $Header$

#ifndef _SMESH_PROPAGATION_HXX_
#define _SMESH_PROPAGATION_HXX_

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

class StdMeshers_Propagation:public SMESH_Hypothesis
{
 public:
  StdMeshers_Propagation(int hypId, int studyId, SMESH_Gen * gen);
  virtual ~ StdMeshers_Propagation();

  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream & operator <<(ostream & save, StdMeshers_Propagation & hyp);
  friend istream & operator >>(istream & load, StdMeshers_Propagation & hyp);

  static std::string GetName ();
};

#endif
