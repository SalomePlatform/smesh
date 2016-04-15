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
// File   : SMESHGUI_ConvToQuadOp.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_ConvToQuadOp.h"

#include "SMESHGUI.h"
#include "SMESHGUI_ConvToQuadDlg.h"
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
#include <SALOME_Actor.h>

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
SMESHGUI_ConvToQuadOp::SMESHGUI_ConvToQuadOp()
  : SMESHGUI_SelectionOp(), 
    myDlg( 0 ),
    myBadElemsPreview(0)
{
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================
SMESHGUI_ConvToQuadOp::~SMESHGUI_ConvToQuadOp()
{
  if ( myDlg ) delete myDlg;
  if ( myBadElemsPreview ) delete myBadElemsPreview;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
  * \retval LightApp_Dialog* - pointer to dialog of this operation
*/
//================================================================================
LightApp_Dialog* SMESHGUI_ConvToQuadOp::dlg() const
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
void SMESHGUI_ConvToQuadOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_ConvToQuadDlg( );
  }
  connect( myDlg, SIGNAL( onClicked( int ) ), SLOT( ConnectRadioButtons( int ) ) );

  myHelpFileName = "convert_to_from_quadratic_mesh_page.html";

  SMESHGUI_SelectionOp::startOperation();

  myDlg->SetMediumNdsOnGeom( true );
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
void SMESHGUI_ConvToQuadOp::selectionDone()
{
  if ( !dlg()->isVisible() )
    return;

  SMESHGUI_SelectionOp::selectionDone();
  try
  {
    QString anObjEntry = myDlg->selectedObject( 0 );
    _PTR(SObject) pObj = studyDS()->FindObjectID( anObjEntry.toLatin1().data() );
    if ( !pObj ) return;

    SMESH::SMESH_IDSource_var idSource = 
      SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( pObj );  

    myDlg->setButtonEnabled( true, QtxDialog::OK | QtxDialog::Apply );
    if( idSource->_is_nil() )
    {
      myDlg->SetEnabledControls( false );
      myDlg->setButtonEnabled( false, QtxDialog::OK | QtxDialog::Apply );
      return;
    }
    SMESH::SMESH_Mesh_var      mesh = idSource->GetMesh();
    bool                    hasGeom = mesh->HasShapeToMesh();
    MeshDestinationType meshTgtType = DestinationMesh( idSource );
    myDlg->SetEnabledRB( meshTgtType, hasGeom && ( meshTgtType & ( BiQuadratic | Quadratic )));

    // show warning on non-conformal result mesh
    if ( ! idSource->_is_nil() )
    {
      SMESH::SMESH_subMesh_var subMesh = 
        SMESH::SObjectToInterface<SMESH::SMESH_subMesh>( pObj );
      bool toShow = false;
      if ( !subMesh->_is_nil() )
      {
        SMESH::SMESH_Mesh_var mesh = idSource->GetMesh();
        idSource = SMESH::SMESH_IDSource::_narrow( mesh );
        bool isMixOrder;
        DestinationMesh( idSource, &isMixOrder );
        toShow = !isMixOrder;
      }
      myDlg->ShowWarning( toShow );
    }
  }
  catch ( const SALOME::SALOME_Exception& S_ex )
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
SUIT_SelectionFilter* SMESHGUI_ConvToQuadOp::createFilter( const int theId ) const
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
bool SMESHGUI_ConvToQuadOp::onApply()
{
  SUIT_OverrideCursor aWaitCursor;

  QString aMess;

  QString anObjEntry = myDlg->selectedObject( 0 );
  _PTR(SObject) pObj = studyDS()->FindObjectID( anObjEntry.toLatin1().data() );
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

  try
  {
    SMESH::SMESH_MeshEditor_var aEditor = mesh->GetMeshEditor();
    aResult = true; 
    SMESH::SMESH_Mesh_var sourceMesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( pObj );  
    if( myDlg->CurrentRB()==1 || myDlg->CurrentRB()==2)
    {
      bool force3d = true;
      if( myDlg->IsEnabledCheck() )
        force3d = myDlg->IsMediumNdsOnGeom();
      bool theToBiQuad = myDlg->IsBiQuadratic();
      if ( sourceMesh->_is_nil() ) {
        if ( theToBiQuad ) aEditor->ConvertToBiQuadratic    ( force3d, idSource );
        else               aEditor->ConvertToQuadraticObject( force3d, idSource );
      }
      else {
        if ( theToBiQuad ) aEditor->ConvertToBiQuadratic( force3d, sourceMesh );
        else               aEditor->ConvertToQuadratic  ( force3d );
      }
      if ( !force3d )
      {
        SMESH::ComputeError_var error = aEditor->GetLastError();
        if ( error->hasBadMesh )
        {
          if ( myBadElemsPreview ) delete myBadElemsPreview; // viewWindow may change
          myBadElemsPreview = new SMESHGUI_MeshEditPreview( viewWindow() );

          double aPointSize = SMESH::GetFloat("SMESH:node_size",3);
          double aLineWidth = SMESH::GetFloat("SMESH:element_width",1);
          vtkProperty* prop = vtkProperty::New();
          prop->SetLineWidth( aLineWidth * 3 );
          prop->SetPointSize( aPointSize * 3 );
          prop->SetColor( 250, 0, 250 );
          myBadElemsPreview->GetActor()->SetProperty( prop );
          prop->Delete();

          SMESH::MeshPreviewStruct_var previewData = aEditor->GetPreviewData();
          myBadElemsPreview->SetData( & previewData.in() );
          myBadElemsPreview->SetVisibility(true);

          SUIT_MessageBox* mb = new SUIT_MessageBox(SUIT_MessageBox::Warning,
                                                    tr( "SMESH_WRN_WARNING" ),
                                                    tr("EDITERR_NO_MEDIUM_ON_GEOM"),
                                                    SUIT_MessageBox::Ok, myDlg);
          mb->setWindowModality( Qt::NonModal );
          mb->setAttribute( Qt::WA_DeleteOnClose );
          mb->show();
          connect ( mb, SIGNAL( finished(int) ), this, SLOT( onWarningWinFinished() ));
          //connect ( mb, SIGNAL( rejected() ), this, SLOT( onWarningWinFinished() ));
        }
      }
    }
    else
    {
      if ( sourceMesh->_is_nil() )
        aEditor->ConvertFromQuadraticObject( idSource );
      else
        aEditor->ConvertFromQuadratic();
    }
  }
  catch ( const SALOME::SALOME_Exception& S_ex )
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
    update( UF_ObjBrowser | UF_Model | UF_Viewer );
    selectionDone();
  }
  return aResult;
}

//================================================================================
/*!
 * \brief SLOT called when a warning window is closed
 */
//================================================================================

void SMESHGUI_ConvToQuadOp::onWarningWinFinished()
{
  if ( myBadElemsPreview )
    myBadElemsPreview->SetVisibility(false);
}

//================================================================================
/*! ConsistMesh
 *  Determines, what elements this mesh does not contain.
 */
//================================================================================

SMESHGUI_ConvToQuadOp::MeshDestinationType
SMESHGUI_ConvToQuadOp::DestinationMesh( const SMESH::SMESH_IDSource_var& idSource,
                                        bool*                            isMixOrder)
{
  SMESH::long_array_var nbElemOfType = idSource->GetMeshInfo();

  bool hasBiQuad     = ( nbElemOfType[SMDSEntity_BiQuad_Triangle   ] ||
                         nbElemOfType[SMDSEntity_BiQuad_Quadrangle ] ||
                         nbElemOfType[SMDSEntity_TriQuad_Hexa      ] );
  bool hasLinBiQuad  = ( nbElemOfType[SMDSEntity_Triangle   ] ||
                         nbElemOfType[SMDSEntity_Quadrangle ] ||
                         nbElemOfType[SMDSEntity_Hexa       ] );
  bool hasQuadBiQuad = ( nbElemOfType[SMDSEntity_Quad_Triangle   ] ||
                         nbElemOfType[SMDSEntity_Quad_Quadrangle ] ||
                         nbElemOfType[SMDSEntity_Quad_Hexa       ] );

  bool hasQuad = ( nbElemOfType[SMDSEntity_Quad_Edge      ] ||
                   nbElemOfType[SMDSEntity_Quad_Triangle  ] ||
                   nbElemOfType[SMDSEntity_Quad_Quadrangle] ||
                   nbElemOfType[SMDSEntity_Quad_Tetra     ] ||
                   nbElemOfType[SMDSEntity_Quad_Hexa      ] ||
                   nbElemOfType[SMDSEntity_Quad_Pyramid   ] ||
                   nbElemOfType[SMDSEntity_Quad_Polygon   ] ||
                   nbElemOfType[SMDSEntity_Quad_Penta     ] );

  bool hasLin  = ( nbElemOfType[SMDSEntity_Edge      ] ||
                   nbElemOfType[SMDSEntity_Triangle  ] ||
                   nbElemOfType[SMDSEntity_Quadrangle] ||
                   nbElemOfType[SMDSEntity_Tetra     ] ||
                   nbElemOfType[SMDSEntity_Hexa      ] ||
                   nbElemOfType[SMDSEntity_Pyramid   ] ||
                   nbElemOfType[SMDSEntity_Polygon   ] ||
                   nbElemOfType[SMDSEntity_Penta     ] );

  int tgtType = 0;
  if ( hasBiQuad )
    tgtType |= ( Quadratic | Linear );
  if ( hasLinBiQuad )
    tgtType |= ( BiQuadratic | Quadratic );
  if ( hasQuadBiQuad )
    tgtType |= ( BiQuadratic | Linear );
  if ( hasQuad )
    tgtType |= Linear;
  if ( hasLin )
    tgtType |= Quadratic;

  if ( tgtType == 0 )
    tgtType = Quadratic;

  if ( isMixOrder )
    *isMixOrder = ( hasLin && ( hasQuad || hasBiQuad ));

  return MeshDestinationType( tgtType );
}

void SMESHGUI_ConvToQuadOp::ConnectRadioButtons( int id )
{
  QString anObjEntry = myDlg->selectedObject( 0 );
  _PTR(SObject) pObj = studyDS()->FindObjectID( anObjEntry.toLatin1().data() );
  if ( !pObj ) return;

  SMESH::SMESH_IDSource_var idSource = 
    SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( pObj );  
  SMESH::SMESH_Mesh_var mesh = idSource->GetMesh();

  bool hasGeom = mesh->HasShapeToMesh();

  if( id==0 || !hasGeom )
    myDlg->SetEnabledCheck( false );
  else
    myDlg->SetEnabledCheck( true );
}
