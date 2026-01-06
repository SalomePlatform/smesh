// Copyright (C) 2007-2026  CEA, EDF, OPEN CASCADE
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
// File   : SMESHGUI_CreateDualMeshOp.cxx
// Author : Yoann AUDOUIN (EDF)
// SMESH includes
//
#include "SMESHGUI_CreateDualMeshOp.h"

#include "SMESHGUI.h"
#include "SMESHGUI_CreateDualMeshDlg.h"
#include "SMESHGUI_MeshEditPreview.h"
#include "SMESHGUI_Utils.h"
#include "SMESH_ActorUtils.h"
#include "SMESH_TypeFilter.hxx"
#include "SMDSAbs_ElementType.hxx"

// SALOME GUI includes
#include <LightApp_UpdateFlags.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SalomeApp_Tools.h>
#include <SalomeApp_Application.h>
#include <SALOME_Actor.h>

// Qt includes
#include <QLineEdit>
#include <QCheckBox>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

// VTK includes
#include <vtkProperty.h>

//================================================================================
/*!
 * \brief Constructor
 *
 * Initialize operation
*/
//================================================================================
SMESHGUI_CreateDualMeshOp::SMESHGUI_CreateDualMeshOp()
  : SMESHGUI_SelectionOp(),
    myDlg( 0 )
{
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================
SMESHGUI_CreateDualMeshOp::~SMESHGUI_CreateDualMeshOp()
{
  if ( myDlg ) delete myDlg;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
  * \retval LightApp_Dialog* - pointer to dialog of this operation
*/
//================================================================================
LightApp_Dialog* SMESHGUI_CreateDualMeshOp::dlg() const
{
  return myDlg;
}

//================================================================================
/*!
 * \brief Creates dialog if necessary and shows it
 *
 * Virtual method redefined from base class called when operation is started creates
 * dialog if necessary and shows it, activates selection
 */
//================================================================================
void SMESHGUI_CreateDualMeshOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_CreateDualMeshDlg( );
  }
  connect( myDlg, SIGNAL( onClicked( int ) ), SLOT( ConnectRadioButtons( int ) ) );

  myHelpFileName = "create_dual_mesh.html";

  SMESHGUI_SelectionOp::startOperation();

  myDlg->activateObject( 0 );
  myDlg->ShowWarning( false );
  myDlg->show();

  selectionDone();
}

//================================================================================
/*!
 * \brief Updates dialog's look and feel
 *
 * Virtual method redefined from the base class updates dialog's look and feel
 */
//================================================================================
void SMESHGUI_CreateDualMeshOp::selectionDone()
{
  if ( !dlg()->isVisible() )
    return;

  SMESHGUI_SelectionOp::selectionDone();
  try
  {
    QString anObjEntry = myDlg->selectedObject( 0 );
    _PTR(SObject) pObj = SMESH::getStudy()->FindObjectID( anObjEntry.toUtf8().data() );
    if ( !pObj ) return;

    SMESH::SMESH_IDSource_var idSource =
      SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( pObj );

    myDlg->setButtonEnabled( true, QtxDialog::OK | QtxDialog::Apply );
    if( idSource->_is_nil() )
    {
      myDlg->setButtonEnabled( false, QtxDialog::OK | QtxDialog::Apply );
      return;
    }
    SMESH::SMESH_Mesh_var      mesh = idSource->GetMesh();

    // show warning on non-conformal result mesh
    if ( ! idSource->_is_nil() )
    {
      SMESH::SMESH_subMesh_var subMesh =
        SMESH::SObjectToInterface<SMESH::SMESH_subMesh>( pObj );
      // Check that mesh is only tetra
      if (!checkMesh(idSource)){
        myDlg->ShowWarning( true );
        myDlg->setButtonEnabled(false, QtxDialog::OK|QtxDialog::Apply);
      }
    }
    std::string mesh_name = "dual_" + pObj->GetName();
    myDlg->myMeshName->setText(QString(mesh_name.c_str()));

  }
  catch ( const SALOME_CMOD::SALOME_Exception& S_ex )
  {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
  }
  catch ( ... )
  {
  }

}

//================================================================================
/*!
 * \brief Creates selection filter
  * \param theId - identifier of current selection widget
  * \retval SUIT_SelectionFilter* - pointer to the created filter or null
 *
 * Creates selection filter in accordance with identifier of current selection widget
 */
//================================================================================
SUIT_SelectionFilter* SMESHGUI_CreateDualMeshOp::createFilter( const int theId ) const
{
  if ( theId == 0 )
    return new SMESH_TypeFilter( SMESH::MESHorSUBMESH );
  else
    return 0;
}

//================================================================================
/*!
 * \brief Edits mesh
 *
 * Virtual slot redefined from the base class called when "Apply" button is clicked
 */
//================================================================================
bool SMESHGUI_CreateDualMeshOp::onApply()
{
  SUIT_OverrideCursor aWaitCursor;

  QString aMess;
  QStringList anEntryList;

  QString anObjEntry = myDlg->selectedObject( 0 );
  _PTR(SObject) pObj = SMESH::getStudy()->FindObjectID( anObjEntry.toUtf8().data() );
  if ( !pObj )
  {
    dlg()->show();
    SUIT_MessageBox::warning( myDlg,
                              tr( "SMESH_WRN_WARNING" ), tr("MESH_IS_NOT_SELECTED") );
    return false;
  }

  SMESH::SMESH_Mesh_var mesh;
  SMESH::SMESH_IDSource_var idSource =
    SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( pObj );
  if( !CORBA::is_nil(idSource) )
    mesh = idSource->GetMesh();

  if( CORBA::is_nil(mesh) )
  {
    SUIT_MessageBox::warning( myDlg,
                              tr( "SMESH_WRN_WARNING" ), tr("REF_IS_NULL") );
    return false;
  }

  bool aResult = false;
  SMESH::SMESH_Gen_var gen = SMESHGUI::GetSMESHGen();
  SMESH::SMESH_Mesh_var newMesh;
  QByteArray newMeshName=myDlg->myMeshName->text().toUtf8();
  bool adapt_to_shape=myDlg->myProjShape->isChecked();
  try
  {
    newMesh = gen->CreateDualMesh(mesh, newMeshName.constData(), adapt_to_shape);

    if ( !newMesh->_is_nil() )
      if ( _PTR(SObject) aSObject = SMESH::ObjectToSObject( newMesh ) )
      {
        anEntryList.append( aSObject->GetID().c_str() );

        SMESH::SetName( aSObject, newMeshName );
      }
    aResult = true;
  }
  catch ( const SALOME_CMOD::SALOME_Exception& S_ex )
  {
    SalomeApp_Tools::QtCatchCorbaException( S_ex );
    aResult = false;
  }
  catch ( ... )
  {
    aResult = false;
  }
  if( aResult )
  {
    SMESHGUI::Modified();
    selectionDone();
    update( UF_ObjBrowser | UF_Model | UF_Viewer );

  }
  SMESHGUI::GetSMESHGUI()->getApp()->updateObjectBrowser();

  // updateObjBrowser(true);
  // SMESHGUI::Modified();

  // if( LightApp_Application* anApp =
  //     dynamic_cast<LightApp_Application*>( SUIT_Session::session()->activeApplication() ) )
  //   anApp->browseObjects( anEntryList, true );

  return true;

}

//================================================================================
/*! checkMesh
 *  Verify that mesh as only tetraheadrons as 3D elements
 */
//================================================================================

bool
SMESHGUI_CreateDualMeshOp::checkMesh( const SMESH::SMESH_IDSource_var& idSource)
{
  SMESH::smIdType_array_var nbElemOfType = idSource->GetMeshInfo();
  // Checking that the mesh only has Tetrahedron
  bool hasOnlyTetra  = (
                    nbElemOfType[SMDSEntity_Tetra     ] &&
                   !nbElemOfType[SMDSEntity_Hexa      ] &&
                   !nbElemOfType[SMDSEntity_Pyramid   ] &&
                   !nbElemOfType[SMDSEntity_Polygon   ] &&
                   !nbElemOfType[SMDSEntity_Penta     ] );

  return hasOnlyTetra;
}
