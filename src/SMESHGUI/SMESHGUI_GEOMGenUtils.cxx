// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_GEOMGenUtils.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_Utils.h"

// SALOME GEOM includes
#include <GeometryGUI.h>

// SALOME KERNEL includes
#include <SALOMEDS_SObject.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SMESH_Mesh)

namespace SMESH
{
  GEOM::GEOM_Gen_var GetGEOMGen()
  {
    static GEOM::GEOM_Gen_var aGEOMGen;

    if(CORBA::is_nil(aGEOMGen)) {
      if ( GeometryGUI::GetGeomGen()->_is_nil() )
        GeometryGUI::InitGeomGen();
      aGEOMGen = GeometryGUI::GetGeomGen();
    }
    return aGEOMGen;
  }

  GEOM::GEOM_Object_var GetShapeOnMeshOrSubMesh(_PTR(SObject) theMeshOrSubmesh,
                                                bool*         isMesh)
  {
    SALOMEDS_SObject* aMeshOrSubmesh = _CAST(SObject,theMeshOrSubmesh);
    if(aMeshOrSubmesh) {
      CORBA::Object_var Obj = aMeshOrSubmesh->GetObject();
      if ( !CORBA::is_nil( Obj ) ) {
        SMESH::SMESH_Mesh_var aMesh =
          SObjectToInterface<SMESH::SMESH_Mesh>( theMeshOrSubmesh );
        if ( !aMesh->_is_nil() )
        {
          if ( isMesh ) *isMesh = true;
          return aMesh->GetShapeToMesh();
        }
        SMESH::SMESH_subMesh_var aSubmesh =
          SObjectToInterface<SMESH::SMESH_subMesh>( theMeshOrSubmesh );
        if ( !aSubmesh->_is_nil() )
        {
          if ( isMesh ) *isMesh = false;
          return aSubmesh->GetSubShape();
        }
      }
    }
    return GEOM::GEOM_Object::_nil();
  }

  GEOM::GEOM_Object_ptr GetGeom (_PTR(SObject) theSO)
  {
    if (!theSO)
      return GEOM::GEOM_Object::_nil();

    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    if (!aStudy)
      return GEOM::GEOM_Object::_nil();

    _PTR(ChildIterator) anIter (aStudy->NewChildIterator(theSO));
    for ( ; anIter->More(); anIter->Next()) {
      _PTR(SObject) aSObject = anIter->Value();
      _PTR(SObject) aRefSOClient;
      GEOM::GEOM_Object_var aMeshShape;

      if (aSObject->ReferencedObject(aRefSOClient)) {
        SALOMEDS_SObject* aRefSO = _CAST(SObject,aRefSOClient);
        aMeshShape = GEOM::GEOM_Object::_narrow(aRefSO->GetObject());
      } else {
        SALOMEDS_SObject* aSO = _CAST(SObject,aSObject);
        aMeshShape = GEOM::GEOM_Object::_narrow(aSO->GetObject());
      }

      if (!aMeshShape->_is_nil())
        return aMeshShape._retn();
    }
    return GEOM::GEOM_Object::_nil();
  }

  GEOM::GEOM_Object_ptr GetSubShape (GEOM::GEOM_Object_ptr theMainShape,
                                     long                  theID)
  {
    GEOM::GEOM_Gen_var geomGen = SMESH::GetGEOMGen();
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    if (!aStudy || geomGen->_is_nil())
      return GEOM::GEOM_Object::_nil();
    GEOM::GEOM_IShapesOperations_var aShapesOp = geomGen->GetIShapesOperations(aStudy->StudyId());
    if (aShapesOp->_is_nil())
      return GEOM::GEOM_Object::_nil();
    GEOM::GEOM_Object_var subShape = aShapesOp->GetSubShape (theMainShape,theID);
    return subShape._retn();
  }
} // end of namespace SMESH
