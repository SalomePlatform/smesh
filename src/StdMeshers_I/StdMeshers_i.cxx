//  SMESH StdMeshers : implementaion of SMESH idl descriptions
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
//  File   : StdMeshers_i.cxx
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESH_Gen_i.hxx"

#include "utilities.h"

#include "StdMeshers_LocalLength_i.hxx"
#include "StdMeshers_AutomaticLength_i.hxx"
#include "StdMeshers_StartEndLength_i.hxx"
#include "StdMeshers_Arithmetic1D_i.hxx"
#include "StdMeshers_NumberOfSegments_i.hxx"
#include "StdMeshers_Deflection1D_i.hxx"
#include "StdMeshers_Propagation_i.hxx"
#include "StdMeshers_LengthFromEdges_i.hxx"
#include "StdMeshers_QuadranglePreference_i.hxx"
#include "StdMeshers_MaxElementArea_i.hxx"
#include "StdMeshers_MaxElementVolume_i.hxx"
#include "StdMeshers_NotConformAllowed_i.hxx"

#include "StdMeshers_Regular_1D_i.hxx"
#include "StdMeshers_MEFISTO_2D_i.hxx"
#include "StdMeshers_Quadrangle_2D_i.hxx"
#include "StdMeshers_Hexa_3D_i.hxx"

//=============================================================================
/*!
 *
 */
//=============================================================================

extern "C"
{
  GenericHypothesisCreator_i* GetHypothesisCreator (const char* aHypName)
  {
    MESSAGE("Get HypothesisCreator for " << aHypName);

    GenericHypothesisCreator_i* aCreator = 0;

    // Hypotheses
    if      (strcmp(aHypName, "LocalLength") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_LocalLength_i>;
    else if (strcmp(aHypName, "NumberOfSegments") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_NumberOfSegments_i>;
    else if (strcmp(aHypName, "LengthFromEdges") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_LengthFromEdges_i>;
    else if (strcmp(aHypName, "NotConformAllowed") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_NotConformAllowed_i>;
    else if (strcmp(aHypName, "Propagation") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_Propagation_i>;
    else if (strcmp(aHypName, "MaxElementArea") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_MaxElementArea_i>;
    else if (strcmp(aHypName, "MaxElementVolume") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_MaxElementVolume_i>;
    else if (strcmp(aHypName, "StartEndLength") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_StartEndLength_i>;
    else if (strcmp(aHypName, "Deflection1D") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_Deflection1D_i>;
    else if (strcmp(aHypName, "Arithmetic1D") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_Arithmetic1D_i>;
    else if (strcmp(aHypName, "AutomaticLength") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_AutomaticLength_i>;
    else if (strcmp(aHypName, "QuadranglePreference") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_QuadranglePreference_i>;

    // Algorithms
    else if (strcmp(aHypName, "Regular_1D") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_Regular_1D_i>;
    else if (strcmp(aHypName, "MEFISTO_2D") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_MEFISTO_2D_i>;
    else if (strcmp(aHypName, "Quadrangle_2D") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_Quadrangle_2D_i>;
    else if (strcmp(aHypName, "Hexa_3D") == 0)
      aCreator = new HypothesisCreator_i<StdMeshers_Hexa_3D_i>;
    else ;

    return aCreator;
  }
}
