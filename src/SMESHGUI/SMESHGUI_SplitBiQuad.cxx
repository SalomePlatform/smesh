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
// File   : SMESHGUI_SplitBiQuad.h
// Author : Open CASCADE S.A.S.
//

#include "SMESHGUI_SplitBiQuad.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESH_LogicalFilter.hxx"
#include "SMESH_TypeFilter.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#include <LightApp_UpdateFlags.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SalomeApp_Tools.h>

#include <QStringList>
#include <QGridLayout>

//================================================================================
/*!
 * \brief Dialog constructor
 */
//================================================================================

SMESHGUI_SplitBiQuadDlg::SMESHGUI_SplitBiQuadDlg()
  : SMESHGUI_Dialog( 0, /*modal=*/false, /*allowResize=*/true )
{
  setWindowTitle( tr( "CAPTION" ) );
  setObjectPixmap( "SMESH", tr( "ICON_SELECT" ) );
  createObject( tr( "MESH" ), mainFrame(), 0 );

  QGridLayout* aLay = new QGridLayout( mainFrame() );
  aLay->setMargin( 5 );
  aLay->setSpacing( 5 );

  aLay->addWidget( objectWg( 0,  Label ),   0, 0 );
  //aLay->addWidget( objectWg( 0,  Btn ),     0, 1 );
  aLay->addWidget( objectWg( 0,  Control ), 0, 1 );
  objectWg( 0,  Btn )->hide();
}

//================================================================================
/*!
 * \brief Dialog destructor
 */
//================================================================================

SMESHGUI_SplitBiQuadDlg::~SMESHGUI_SplitBiQuadDlg()
{
}

//================================================================================
/*!
 * \brief SMESHGUI_SplitBiQuadOp constructor
 */
//================================================================================

SMESHGUI_SplitBiQuadOp::SMESHGUI_SplitBiQuadOp()
  : SMESHGUI_SelectionOp(), myDlg( 0 )
{
}

//================================================================================
/*!
 * \brief SMESHGUI_SplitBiQuadOp destructor
 */
//================================================================================

SMESHGUI_SplitBiQuadOp::~SMESHGUI_SplitBiQuadOp()
{
  if ( myDlg ) delete myDlg;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
  * \retval LightApp_Dialog* - pointer to dialog of this operation
*/
//================================================================================

LightApp_Dialog* SMESHGUI_SplitBiQuadOp::dlg() const
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

void SMESHGUI_SplitBiQuadOp::startOperation()
{
  if( !myDlg )
  {
    myDlg = new SMESHGUI_SplitBiQuadDlg();
  }
  myHelpFileName = "split_biquad_to_linear_page.html";

  SMESHGUI_SelectionOp::startOperation();

  myDlg->activateObject( 0 );
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

// void SMESHGUI_SplitBiQuadOp::selectionDone()
// {
//   if ( !dlg()->isVisible() )
//     return;

//   SMESHGUI_SelectionOp::selectionDone();
// }

//================================================================================
/*!
 * \brief Creates selection filter
  * \param theId - identifier of current selection widget
  * \retval SUIT_SelectionFilter* - pointer to the created filter or null
 *
 * Creates selection filter in accordance with identifier of current selection widget
 */
//================================================================================

SUIT_SelectionFilter* SMESHGUI_SplitBiQuadOp::createFilter( const int theId ) const
{
  if ( theId != 0 )
    return 0;

  QList<SUIT_SelectionFilter*> filters;
  filters << new SMESH_TypeFilter( SMESH::IDSOURCE_FACE );
  filters << new SMESH_TypeFilter( SMESH::IDSOURCE_VOLUME );
  return new SMESH_LogicalFilter( filters,
                                  SMESH_LogicalFilter::LO_OR,
                                  /*takeOwnership=*/true );
}

//================================================================================
/*!
 * \brief Edits mesh
 *
 * Virtual slot redefined from the base class called when "Apply" button is clicked
 */
//================================================================================

bool SMESHGUI_SplitBiQuadOp::onApply()
{
  SUIT_OverrideCursor aWaitCursor;

  LightApp_Dialog::SelectedObjects selection;
  myDlg->objectSelection( selection );
  if ( selection.empty() || selection[0].empty() )
  {
    SUIT_MessageBox::warning( myDlg, tr( "SMESH_WRN_WARNING" ), tr("MESH_IS_NOT_SELECTED") );
    return false;
  }
  QStringList& entries = selection[0];

  SMESH::SMESH_Mesh_var mesh;
  SMESH::ListOfIDSources_var idSource = new SMESH::ListOfIDSources();
  idSource->length( entries.count() );

  int nbObj = 0;
  for ( int i = 0; i < entries.count() ; ++i )
  {
    _PTR(SObject) pObj = studyDS()->FindObjectID( entries[i].toLatin1().data() );
    SMESH::SMESH_IDSource_var obj = SMESH::SObjectToInterface<SMESH::SMESH_IDSource>( pObj );
    if( !CORBA::is_nil( obj ))
    {
      idSource[ nbObj++ ] = obj;
      SMESH::SMESH_Mesh_var m = obj->GetMesh();
      if ( !mesh->_is_nil() && mesh->GetId() != m->GetId() )
      {
        SUIT_MessageBox::warning( myDlg, tr( "SMESH_WRN_WARNING" ), tr("DIFFERENT_MESHES") );
        return false;
      }
      mesh = m;
    }
  }
  if ( CORBA::is_nil( mesh ))
  {
    SUIT_MessageBox::warning( myDlg, tr( "SMESH_WRN_WARNING" ), tr("REF_IS_NULL") );
    return false;
  }
  if ( nbObj == 0 )
  {
    SUIT_MessageBox::warning( myDlg, tr( "SMESH_WRN_WARNING" ), tr("MESH_IS_NOT_SELECTED") );
    return false;
  }
  idSource->length( nbObj );

  bool aResult = false;

  try
  {
    SMESH::SMESH_MeshEditor_var aEditor = mesh->GetMeshEditor();
    aResult = true;
    aEditor->SplitBiQuadraticIntoLinear( idSource );
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
