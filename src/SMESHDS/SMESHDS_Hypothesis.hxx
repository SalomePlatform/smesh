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
//  File   : SMESHDS_Hypothesis.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESHDS_HYPOTHESIS_HXX_
#define _SMESHDS_HYPOTHESIS_HXX_

#if (__GNUC__>2)
#include <string>
#include <istream>
#include <ostream>
#else 
#include <string>
#include <istream.h>
#include <ostream.h>
#endif
using namespace std;  

class SMESHDS_Hypothesis
{
public:
  SMESHDS_Hypothesis(int hypId);
  virtual ~SMESHDS_Hypothesis();

  const char* GetName() const;
  int GetID() const;
  void SetID(int id);
  int GetType() const;

  virtual ostream & SaveTo(ostream & save)=0;
  virtual istream & LoadFrom(istream & load)=0;

enum hypothesis_type {PARAM_ALGO, ALGO, ALGO_1D, ALGO_2D, ALGO_3D};

protected:
  string _name;
  int _hypId;
  int _type;
};

#endif
