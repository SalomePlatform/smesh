//  SMESH NETGENPlugin : implementaion of SMESH idl descriptions
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
//  File   : NETGENPlugin.cxx
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESH_Hypothesis_i.hxx"
#include "SMESH_Gen_i.hxx"

#include "utilities.h"

#include "NETGENPlugin_NETGEN_3D_i.hxx"

//=============================================================================
/*!
 *
 */
//=============================================================================

extern "C"
{
  GenericHypothesisCreator_i* GetHypothesisCreator (const char* aHypName)
  {
    MESSAGE("GetHypothesisCreator " << aHypName);

    GenericHypothesisCreator_i* aCreator = 0;

    // Hypotheses

    // Algorithms
    if (strcmp(aHypName, "NETGEN_3D") == 0)
      aCreator = new HypothesisCreator_i<NETGENPlugin_NETGEN_3D_i>;
    else ;

    return aCreator;
  }
}
