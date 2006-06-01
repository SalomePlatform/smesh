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


#include "SMESHGUI_GroupUtils.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"

#include "SUIT_Session.h"
#include "SalomeApp_Application.h"
#include "SalomeApp_Tools.h"

#include "OB_Browser.h"

namespace SMESH
{

  SMESH::SMESH_Group_var AddGroup( SMESH::SMESH_Mesh_ptr theMesh,
				   SMESH::ElementType theType,
				   const char* theGroupName )
  {
    SMESH::SMESH_Group_var aGroup;
    try {
      if ( !theMesh->_is_nil() )
	aGroup = theMesh->CreateGroup( theType, theGroupName );
    }
    catch( const SALOME::SALOME_Exception& S_ex ) {
      SalomeApp_Tools::QtCatchCorbaException( S_ex );
    }
    //SalomeApp_Application* app =
    //  dynamic_cast<SalomeApp_Application*>(SUIT_Session::session()->activeApplication());
    //if (app) app->objectBrowser()->updateTree();
    SMESHGUI::GetSMESHGUI()->updateObjBrowser();
    return aGroup._retn();
  }
}
