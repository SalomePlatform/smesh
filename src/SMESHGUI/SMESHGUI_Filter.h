//  SMESHGUI_Filter : Filters for VTK viewer
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
//  File   : SMESHGUI_Filter.h
//  Author : Sergey LITONIN
//  Module : SMESH

#ifndef SMESHGUI_Filter_HeaderFile
#define SMESHGUI_Filter_HeaderFile

#include "VTKViewer_Filter.h"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Filter)

class SALOME_Actor;


DEFINE_STANDARD_HANDLE(SMESHGUI_Filter, VTKViewer_Filter)

/*
  Class       : SMESHGUI_Filter
  Description : Selection filter for VTK viewer
*/

class SMESHGUI_Filter : public VTKViewer_Filter
{

public:
                              SMESHGUI_Filter();
  virtual                     ~SMESHGUI_Filter();

  virtual bool                IsValid( const int theCellId ) const;

  virtual void                SetActor( SALOME_Actor* );
  SALOME_Actor*               GetActor() const;

  void                        SetPredicate( SMESH::Predicate_ptr );

private:

  SMESH::Predicate_var        myPred;

public:  
  DEFINE_STANDARD_RTTI(SMESHGUI_Filter)
};

#endif
