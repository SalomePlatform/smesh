//  SMESH StdMeshersGUI : GUI for standard meshers
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
//  File   : StdMeshersGUI_Parameters.h
//  Module : SMESH
//  $Header$

#ifndef STDMESHERSGUI_PARAMETERS_H
#define STDMESHERSGUI_PARAMETERS_H

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)

#include "SMESHGUI_aParameter.h"

#include <list>

class StdMeshersGUI_Parameters
{
 public:
  static bool HasParameters (const QString& hypType);
  
  static void GetParameters (const QString&                      hypType,
			     std::list<SMESHGUI_aParameterPtr> & params );
  
  static void GetParameters (SMESH::SMESH_Hypothesis_ptr         hyp,
			     std::list<SMESHGUI_aParameterPtr> & params );
  static void GetParameters (SMESH::SMESH_Hypothesis_ptr         hyp,
			     std::list<SMESHGUI_aParameterPtr> & paramList,
			     QString&                            params);
  
  static bool SetParameters(SMESH::SMESH_Hypothesis_ptr               hyp,
			    const std::list<SMESHGUI_aParameterPtr> & params );
};
#endif
