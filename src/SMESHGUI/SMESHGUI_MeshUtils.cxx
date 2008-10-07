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


#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_Utils.h"
#include "SALOMEDSClient_Study.hxx"

#include "SALOMEconfig.h"
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

  QString UniqueMeshName(const char* theBaseName, const char* thePostfix)
  {
    QString baseName = theBaseName;
    if ( thePostfix/* && !name.contains( postfix )*/) { // add postfix
      baseName += "_";
      baseName += thePostfix;
    }
    if(_PTR(Study) aStudy = GetActiveStudyDocument()) {
      QString name = baseName;
      while ( !aStudy->FindObjectByName( name.latin1(), "SMESH" ).empty() ) {
        int nb = 0;
        if ( name.at( name.length()-1 ).isNumber() ) {
          int nbBeg = name.findRev("_");
          nb = name.right( name.length() - nbBeg - 1 ).toInt();
          name = name.left( nbBeg );
        }
        name += QString("_%1").arg( nb+1 );
      }
      return name;
    }
    return QString("");
  }

}
