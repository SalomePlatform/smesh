//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_MeshUtils.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_MeshUtils.h"

#include "SMESHGUI_Utils.h"

// SALOME KERNEL includes
#include <SALOMEDSClient_Study.hxx>

// Qt includes
#include <QStringList>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)

namespace SMESH
{
  SMESH_Mesh_var GetMeshByIO(const Handle(SALOME_InteractiveObject)& theIO)
  {
    CORBA::Object_var anObj = IObjectToObject(theIO);
    if(!CORBA::is_nil(anObj)){
      SMESH_Mesh_var aMesh = SMESH_Mesh::_narrow(anObj);
      if(!CORBA::is_nil(aMesh))
	return aMesh;
      SMESH_GroupBase_var aGroup = SMESH_GroupBase::_narrow(anObj);
      if(!CORBA::is_nil(aGroup))
	return aGroup->GetMesh();
      SMESH_subMesh_var aSubMesh = SMESH_subMesh::_narrow(anObj);
      if(!CORBA::is_nil(aSubMesh))
	return aSubMesh->GetFather();
    }
    return SMESH_Mesh::_nil();
  }

  QString UniqueMeshName(const QString& theBaseName, const QString& thePostfix)
  {
    QString baseName = thePostfix.isEmpty() ? 
      theBaseName : theBaseName + "_" + thePostfix;
    if ( _PTR(Study) aStudy = GetActiveStudyDocument() ) {
      QString name = baseName;
      while ( !aStudy->FindObjectByName( name.toLatin1().data(), "SMESH" ).empty() ) {
        int nb = 0;
	QStringList names = name.split("_", QString::KeepEmptyParts);
	if ( names.count() > 0 ) {
	  bool ok;
	  int index = names.last().toInt( &ok );
	  if ( ok ) {
	    nb = index;
	    names.removeLast();
	  }
	}
	names.append( QString::number( nb+1 ) );
	name = names.join( "_" );
      }
      return name;
    }
    return baseName;
  }
} // end of namespace SMESH
