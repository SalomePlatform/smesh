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

#ifndef SMESH_ACTORUTILS_H
#define SMESH_ACTORUTILS_H

#include "SMESH_Object.h"

#include <qcolor.h>
#include <qstring.h>

#include "VTKViewer.h"

class vtkUnstructuredGrid;

namespace SMESH
{
SMESHOBJECT_EXPORT  
  vtkFloatingPointType 
  GetFloat( const QString& theValue, 
	    vtkFloatingPointType theDefault = 0 );

SMESHOBJECT_EXPORT
  vtkFloatingPointType 
  GetFloat( const QString& theName, 
	    const QString& theSection, 
	    vtkFloatingPointType theDefault = 0 );

SMESHOBJECT_EXPORT
  QColor 
  GetColor( const QString& theSect, 
	    const QString& theName, 
	    const QColor&t = QColor() );

SMESHOBJECT_EXPORT
  void
  GetColor( const QString& theSect, 
	    const QString& theName, 
	    int&, 
	    int&, 
	    int&, 
	    const QColor&t = QColor() );

SMESHOBJECT_EXPORT
  void
  GetColor( const QString& theSect, 
	    const QString& theName, 
	    vtkFloatingPointType&, 
	    vtkFloatingPointType&, 
	    vtkFloatingPointType&, 
	    const QColor&t = QColor() );

SMESHOBJECT_EXPORT
  void 
  WriteUnstructuredGrid(vtkUnstructuredGrid* theGrid, 
			const char* theFileName);

}

#endif
