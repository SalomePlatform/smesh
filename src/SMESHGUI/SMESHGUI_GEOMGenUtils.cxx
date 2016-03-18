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

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_GEOMGenUtils.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI.h"

// SALOME GEOM includes
#include <GeometryGUI.h>
#include <GEOM_wrap.hxx>

// SALOME KERNEL includes
#include <SALOMEDS_SObject.hxx>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SMESH_Mesh)

#include <QString>

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

  GEOM::GEOM_Object_var GetGeom (_PTR(SObject) theSO)
  {
    GEOM::GEOM_Object_var aMeshShape;
    if (!theSO)
      return aMeshShape;

    CORBA::Object_var obj = _CAST( SObject,theSO )->GetObject();
    aMeshShape = GEOM::GEOM_Object::_narrow( obj );
    if ( !aMeshShape->_is_nil() )
      return aMeshShape;

    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    if (!aStudy)
      return aMeshShape;

    _PTR(ChildIterator) anIter (aStudy->NewChildIterator(theSO));
    for ( ; anIter->More(); anIter->Next()) {
      _PTR(SObject) aSObject = anIter->Value();
      _PTR(SObject) aRefSOClient;

      if (aSObject->ReferencedObject(aRefSOClient)) {
        SALOMEDS_SObject* aRefSO = _CAST(SObject,aRefSOClient);
        aMeshShape = GEOM::GEOM_Object::_narrow(aRefSO->GetObject());
      } else {
        SALOMEDS_SObject* aSO = _CAST(SObject,aSObject);
        aMeshShape = GEOM::GEOM_Object::_narrow(aSO->GetObject());
      }
      if ( !aMeshShape->_is_nil() )
        return aMeshShape;
    }
    return aMeshShape;
  }

  SMESHGUI_EXPORT char* GetGeomName( _PTR(SObject) smeshSO )
  {
    if (!smeshSO)
      return 0;

    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    if (!aStudy)
      return 0;

    _PTR(ChildIterator) anIter (aStudy->NewChildIterator( smeshSO ));
    for ( ; anIter->More(); anIter->Next()) {
      _PTR(SObject) aSObject = anIter->Value();
      _PTR(SObject) aRefSOClient;
      GEOM::GEOM_Object_var aMeshShape;

      if (aSObject->ReferencedObject(aRefSOClient)) {
        SALOMEDS_SObject* aRefSO = _CAST(SObject,aRefSOClient);
        aMeshShape = GEOM::GEOM_Object::_narrow(aRefSO->GetObject());
        aSObject = aRefSOClient;
      }
      else {
        SALOMEDS_SObject* aSO = _CAST(SObject,aSObject);
        aMeshShape = GEOM::GEOM_Object::_narrow(aSO->GetObject());
      }

      if (!aMeshShape->_is_nil())
      {
        std::string name = aSObject->GetName();
        return CORBA::string_dup( name.c_str() );
      }
    }
    return 0;
  }

  GEOM::GEOM_Object_ptr GetSubShape (GEOM::GEOM_Object_ptr theMainShape,
                                     long                  theID)
  {
    GEOM::GEOM_Gen_var geomGen = SMESH::GetGEOMGen();
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    if (!aStudy || geomGen->_is_nil())
      return GEOM::GEOM_Object::_nil();
    GEOM::GEOM_IShapesOperations_wrap aShapesOp =
      geomGen->GetIShapesOperations(aStudy->StudyId());
    if (aShapesOp->_is_nil())
      return GEOM::GEOM_Object::_nil();
    GEOM::GEOM_Object_wrap subShape = aShapesOp->GetSubShape (theMainShape,theID);
    return subShape._retn();
  }

  //================================================================================
  /*!
   * \brief Return entries of sub-mesh geometry and mesh geometry by an IO of assigned
   *        hypothesis
   *  \param [in] hypIO - IO of hyp which is a reference SO to a hyp SO
   *  \param [out] subGeom - found entry of a sub-mesh if any
   *  \param [out] meshGeom - found entry of a mesh
   *  \return bool - \c true if any geometry has been found
   */
  //================================================================================

  bool GetGeomEntries( Handle(SALOME_InteractiveObject)& hypIO,
                       QString&                          subGeom,
                       QString&                          meshGeom )
  {
    subGeom.clear();
    meshGeom.clear();
    if ( hypIO.IsNull() ) return false;

    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    if ( !aStudy ) return false;

    _PTR(SObject) hypSO = aStudy->FindObjectID( hypIO->getEntry() );
    if ( !hypSO ) return false;

    // Depth() is a number of fathers
    if ( hypSO->Depth() == 4 ) // hypSO is not a reference to a hyp but a hyp it-self
    {
      SMESH::SMESH_Hypothesis_var hyp =
        SMESH::SObjectToInterface< SMESH::SMESH_Hypothesis >( hypSO );
      SMESH::SMESH_Mesh_var mesh;
      GEOM::GEOM_Object_var geom;
      SMESH::SMESH_Gen_var  gen = SMESHGUI::GetSMESHGUI()->GetSMESHGen();
      if ( !gen || !gen->GetSoleSubMeshUsingHyp( hyp, mesh.out(), geom.out() ))
        return false;

      subGeom = toQStr( geom->GetStudyEntry() );

      geom  = mesh->GetShapeToMesh();
      if ( geom->_is_nil() )
        return false;
      meshGeom = toQStr( geom->GetStudyEntry() );
    }
    else
    {
      _PTR(SObject) appliedSO = hypSO->GetFather(); // "Applied hypotheses" folder
      if ( !appliedSO ) return false;

      _PTR(SObject) subOrMeshSO = appliedSO->GetFather(); // mesh or sub-mesh SO
      if ( !subOrMeshSO ) return false;

      bool isMesh;
      GEOM::GEOM_Object_var geom = GetShapeOnMeshOrSubMesh( subOrMeshSO, &isMesh );
      if ( geom->_is_nil() )
        return false;

      if ( isMesh )
      {
        meshGeom = toQStr( geom->GetStudyEntry() );
        return !meshGeom.isEmpty();
      }

      subGeom = toQStr( geom->GetStudyEntry() );

      _PTR(SObject) subFolderSO = subOrMeshSO->GetFather(); // "SubMeshes on ..." folder
      if ( !subFolderSO ) return false;

      _PTR(SObject) meshSO = subFolderSO->GetFather(); // mesh SO
      if ( !meshSO ) return false;

      geom = GetShapeOnMeshOrSubMesh( meshSO );
      if ( geom->_is_nil() )
        return false;

      meshGeom = toQStr( geom->GetStudyEntry() );
    }

    return !meshGeom.isEmpty() && !subGeom.isEmpty();
  }


} // end of namespace SMESH
