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

enum SMESHGUI_FilterType
{
  SMESHGUI_UnknownFilter      = -1,
  SMESHGUI_NodeFilter         =  0,
  SMESHGUI_EdgeFilter         =  1,
  SMESHGUI_FaceFilter         =  2,
  SMESHGUI_VolumeFilter       =  3,
  SMESHGUI_AllElementsFilter  =  4,
  SMESHGUI_QuadFilter         =  5,
  SMESHGUI_TriaFilter         =  6
};

/*
  Class       : SMESHGUI_Filter
  Description : Base class for SMESH selection filters for VTK viewer. 
*/

DEFINE_STANDARD_HANDLE(SMESHGUI_Filter, VTKViewer_Filter)

class SMESHGUI_Filter : public VTKViewer_Filter
{

public:

  virtual bool                IsObjValid( const int theObjId ) const = 0;
public:
  DEFINE_STANDARD_RTTI(SMESHGUI_Filter)
};

/*
  Class       : SMESHGUI_PredicateFilter
  Description : Selection filter for VTK viewer. This class aggregate object
                of SMESH_Predicate class and uses it for verification of criterion
*/

DEFINE_STANDARD_HANDLE(SMESHGUI_PredicateFilter, SMESHGUI_Filter)

class SMESHGUI_PredicateFilter : public SMESHGUI_Filter
{

public:
                              SMESHGUI_PredicateFilter();
  virtual                     ~SMESHGUI_PredicateFilter();

  virtual bool                IsValid( const int theCellId ) const;
  virtual bool                IsObjValid( const int theObjId ) const;
  virtual int                 GetId() const;
  virtual bool                IsNodeFilter() const;
  void                        SetPredicate( SMESH::Predicate_ptr );

  void                        SetActor( SALOME_Actor* );

private:

  SMESH::Predicate_var        myPred;

public:
  DEFINE_STANDARD_RTTI(SMESHGUI_PredicateFilter)
};

/*
  Class       : SMESHGUI_QuadrangleFilter
  Description : Verify whether selected cell is quadranle
*/

DEFINE_STANDARD_HANDLE(SMESHGUI_QuadrangleFilter, SMESHGUI_Filter)

class SMESHGUI_QuadrangleFilter : public SMESHGUI_Filter
{

public:
                              SMESHGUI_QuadrangleFilter();
  virtual                     ~SMESHGUI_QuadrangleFilter();

  virtual bool                IsValid( const int theCellId ) const;
  virtual bool                IsObjValid( const int theObjId ) const;
  virtual int                 GetId() const;
  virtual bool                IsNodeFilter() const;

public:
  DEFINE_STANDARD_RTTI(SMESHGUI_QuadrangleFilter)
};

/*
  Class       : SMESHGUI_TriangleFilter
  Description : Verify whether selected cell is triangle
*/

DEFINE_STANDARD_HANDLE(SMESHGUI_TriangleFilter, SMESHGUI_Filter)

class SMESHGUI_TriangleFilter : public SMESHGUI_Filter
{

public:
                              SMESHGUI_TriangleFilter();
  virtual                     ~SMESHGUI_TriangleFilter();

  virtual bool                IsValid( const int theCellId ) const;
  virtual bool                IsObjValid( const int theObjId ) const;
  virtual int                 GetId() const;
  virtual bool                IsNodeFilter() const;  

public:
  DEFINE_STANDARD_RTTI(SMESHGUI_TriangleFilter)
};

#endif
