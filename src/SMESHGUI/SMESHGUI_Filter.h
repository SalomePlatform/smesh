// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// SMESHGUI_Filter : Filters for VTK viewer
// File   : SMESHGUI_Filter.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_FILTER_H
#define SMESHGUI_FILTER_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMDSAbs_ElementType.hxx"

// SALOME GUI includes
#include <VTKViewer_Filter.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Filter)

class SALOME_Actor;

namespace SMESH
{
  enum SMESHGUI_FilterType {
    UnknownFilter           = -1,
    NodeFilter              =  0,
    EdgeFilter              =  1,
    FaceFilter              =  2,
    VolumeFilter            =  3,
    AllElementsFilter       =  4,
    QuadFilter              =  5,
    TriaFilter              =  6,
    FirstGeometryTypeFilter,
    FirstEntityTypeFilter   = FirstGeometryTypeFilter + SMDSGeom_NONE,
    LastFilter              = FirstEntityTypeFilter   + SMDSEntity_Last
  };
};

/*
  Class       : SMESHGUI_Filter
  Description : Base class for SMESH selection filters for VTK viewer.
*/

DEFINE_STANDARD_HANDLE(SMESHGUI_Filter, VTKViewer_Filter)

class SMESHGUI_Filter : public VTKViewer_Filter
{
public:
  Standard_EXPORT virtual bool IsObjValid( const int ) const = 0;

public:
  OCCT_DEFINE_STANDARD_RTTIEXT(SMESHGUI_Filter,VTKViewer_Filter)
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
  Standard_EXPORT SMESHGUI_PredicateFilter();
  Standard_EXPORT virtual ~SMESHGUI_PredicateFilter();

  Standard_EXPORT virtual bool IsValid( const int ) const;
  Standard_EXPORT virtual bool IsObjValid( const int ) const;
  Standard_EXPORT virtual int  GetId() const;
  Standard_EXPORT virtual bool IsNodeFilter() const;
  Standard_EXPORT void         SetPredicate( SMESH::Predicate_ptr );

  Standard_EXPORT void         SetActor( SALOME_Actor* );

private:
  SMESH::Predicate_var        myPred;

public:
  OCCT_DEFINE_STANDARD_RTTIEXT(SMESHGUI_PredicateFilter,SMESHGUI_Filter)
};

/*
  Class       : SMESHGUI_QuadrangleFilter
  Description : Verify whether selected cell is quadranle
*/

DEFINE_STANDARD_HANDLE(SMESHGUI_QuadrangleFilter, SMESHGUI_Filter)

class SMESHGUI_QuadrangleFilter : public SMESHGUI_Filter
{
public:
  Standard_EXPORT SMESHGUI_QuadrangleFilter();
  Standard_EXPORT virtual ~SMESHGUI_QuadrangleFilter();

  Standard_EXPORT virtual bool IsValid( const int ) const;
  Standard_EXPORT virtual bool IsObjValid( const int ) const;
  Standard_EXPORT virtual int  GetId() const;
  Standard_EXPORT virtual bool IsNodeFilter() const;

public:
  OCCT_DEFINE_STANDARD_RTTIEXT(SMESHGUI_QuadrangleFilter,SMESHGUI_Filter)
};

/*
  Class       : SMESHGUI_TriangleFilter
  Description : Verify whether selected cell is triangle
*/

DEFINE_STANDARD_HANDLE(SMESHGUI_TriangleFilter, SMESHGUI_Filter)

class SMESHGUI_TriangleFilter : public SMESHGUI_Filter
{
public:
  Standard_EXPORT SMESHGUI_TriangleFilter();
  Standard_EXPORT virtual ~SMESHGUI_TriangleFilter();

  Standard_EXPORT virtual bool IsValid( const int ) const;
  Standard_EXPORT virtual bool IsObjValid( const int ) const;
  Standard_EXPORT virtual int  GetId() const;
  Standard_EXPORT virtual bool IsNodeFilter() const;  

public:
  OCCT_DEFINE_STANDARD_RTTIEXT(SMESHGUI_TriangleFilter,SMESHGUI_Filter)
};

/*
  Class       : SMESHGUI_FacesFilter
  Description : Verify whether selected cell is any face
*/

DEFINE_STANDARD_HANDLE(SMESHGUI_FacesFilter, SMESHGUI_Filter)

class SMESHGUI_FacesFilter : public SMESHGUI_Filter
{
public:
  Standard_EXPORT SMESHGUI_FacesFilter();
  Standard_EXPORT virtual ~SMESHGUI_FacesFilter();

  Standard_EXPORT virtual bool IsValid( const int ) const;
  Standard_EXPORT virtual bool IsObjValid( const int ) const;
  Standard_EXPORT virtual int  GetId() const;
  Standard_EXPORT virtual bool IsNodeFilter() const;  

public:
  OCCT_DEFINE_STANDARD_RTTIEXT(SMESHGUI_FacesFilter,SMESHGUI_Filter)
};

/*
  Class       : SMESHGUI_VolumesFilter
  Description : Verify whether selected cell is any volume
*/

DEFINE_STANDARD_HANDLE(SMESHGUI_VolumesFilter, SMESHGUI_Filter)

class SMESHGUI_VolumesFilter : public SMESHGUI_Filter
{
public:
  Standard_EXPORT SMESHGUI_VolumesFilter();
  Standard_EXPORT virtual ~SMESHGUI_VolumesFilter();

  Standard_EXPORT virtual bool IsValid( const int ) const;
  Standard_EXPORT virtual bool IsObjValid( const int ) const;
  Standard_EXPORT virtual int  GetId() const;
  Standard_EXPORT virtual bool IsNodeFilter() const;

public:
  OCCT_DEFINE_STANDARD_RTTIEXT(SMESHGUI_VolumesFilter,SMESHGUI_Filter)
};

/*
  Class       : SMESHGUI_VolumeShapeFilter
  Description : Verify whether selected cell is a volume of a certain shape
*/

DEFINE_STANDARD_HANDLE(SMESHGUI_VolumeShapeFilter, SMESHGUI_Filter)

class SMESHGUI_VolumeShapeFilter : public SMESHGUI_Filter
{
  SMDSAbs_GeometryType myGeometryType;
public:
  Standard_EXPORT SMESHGUI_VolumeShapeFilter(const SMDSAbs_GeometryType shape);

  Standard_EXPORT virtual bool IsValid( const int ) const;
  Standard_EXPORT virtual bool IsObjValid( const int ) const;
  Standard_EXPORT virtual int  GetId() const;
  Standard_EXPORT virtual bool IsNodeFilter() const;

  Standard_EXPORT static int GetId( SMDSAbs_GeometryType geom );

public:
  OCCT_DEFINE_STANDARD_RTTIEXT(SMESHGUI_VolumeShapeFilter,SMESHGUI_Filter)
};

#endif // SMESHGUI_FILTER_H
