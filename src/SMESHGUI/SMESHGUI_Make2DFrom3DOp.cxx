//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : SMESHGUI_Make2DFrom3DOp.cxx
// Author : Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_Make2DFrom3DOp.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_MeshInfosBox.h"

// SALOME GUI includes
#include <LightApp_SelectionMgr.h>

#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_OverrideCursor.h>

#include <SALOME_ListIO.hxx>

// SALOME KERNEL includes
#include <SALOMEDS_SObject.hxx>
#include <SALOMEDSClient_SObject.hxx>

// Qt includes
// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>

// MESH includes
#include "SMDSAbs_ElementType.hxx"
#include "SMDSAbs_ElementType.hxx"


#define SPACING 6
#define MARGIN  11

// =========================================================================================
/*!
 * \brief Dialog to show creted mesh statistic
 */
//=======================================================================

SMESHGUI_Make2DFrom3DDlg::SMESHGUI_Make2DFrom3DDlg( QWidget* parent )
 : SMESHGUI_Dialog( parent, false, true, Close/* | Help*/ )
{
  setWindowTitle( tr("CAPTION") );
  QVBoxLayout* aDlgLay = new QVBoxLayout (mainFrame());
  aDlgLay->setMargin( 0 );
  aDlgLay->setSpacing( SPACING );
  QFrame* aMainFrame = createMainFrame(mainFrame());
  aDlgLay->addWidget(aMainFrame);
  aDlgLay->setStretchFactor(aMainFrame, 1);
}

// =========================================================================================
/*!
 * \brief Dialog destructor
 */
//=======================================================================

SMESHGUI_Make2DFrom3DDlg::~SMESHGUI_Make2DFrom3DDlg()
{
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's fields
//=======================================================================

QFrame* SMESHGUI_Make2DFrom3DDlg::createMainFrame (QWidget* theParent)
{
  QFrame* aFrame = new QFrame(theParent);

  SUIT_ResourceMgr* rm = resourceMgr();
  QPixmap iconCompute (rm->loadPixmap("SMESH", tr("ICON_2D_FROM_3D")));

  // Mesh name
  QGroupBox* nameBox = new QGroupBox(tr("SMESH_MESHINFO_NAME"), aFrame );
  QHBoxLayout* nameBoxLayout = new QHBoxLayout(nameBox);
  nameBoxLayout->setMargin(MARGIN); nameBoxLayout->setSpacing(SPACING);
  myMeshName = new QLabel(nameBox);
  nameBoxLayout->addWidget(myMeshName);

  // Mesh Info

  myFullInfo = new SMESHGUI_MeshInfosBox(true,  aFrame);

  // add all widgets to aFrame
  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->setMargin( 0 );
  aLay->setSpacing( 0 );
  aLay->addWidget( nameBox );
  aLay->addWidget( myFullInfo );

  ((QPushButton*) button( OK ))->setDefault( true );
  return aFrame;
}

//================================================================================
/*!
 * \brief set name of the mesh
*/
//================================================================================

void SMESHGUI_Make2DFrom3DDlg::SetMeshName(const QString& theName)
{
  myMeshName->setText( theName );
}

//================================================================================
/*!
 * \brief set mesh info
*/
//================================================================================

void SMESHGUI_Make2DFrom3DDlg::SetMeshInfo(const SMESH::long_array& theInfo)
{
  myFullInfo->SetMeshInfo( theInfo );
}

//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_Make2DFrom3DOp::SMESHGUI_Make2DFrom3DOp()
 : SMESHGUI_Operation()
{
  myDlg = new SMESHGUI_Make2DFrom3DDlg(desktop());
}

//================================================================================
/*!
 * \brief Desctructor
*/
//================================================================================

SMESHGUI_Make2DFrom3DOp::~SMESHGUI_Make2DFrom3DOp()
{
}

//================================================================================
/*!
 * \brief perform it's intention action: compute 2D mesh on 3D
 */
//================================================================================

void SMESHGUI_Make2DFrom3DOp::startOperation()
{
  myMesh = SMESH::SMESH_Mesh::_nil();
  
  // check selection
  LightApp_SelectionMgr *Sel = selectionMgr();
  SALOME_ListIO selected; Sel->selectedObjects( selected );

  int nbSel = selected.Extent();
  if (nbSel != 1) {
    SUIT_MessageBox::warning(desktop(),
                             tr("SMESH_WRN_WARNING"),
                             tr("SMESH_WRN_NO_AVAILABLE_DATA"));
    onCancel();
    return;
  }

  Handle(SALOME_InteractiveObject) anIO = selected.First();
  myMesh = SMESH::GetMeshByIO(anIO);
  if (myMesh->_is_nil()) {
    SUIT_MessageBox::warning(desktop(),
                             tr("SMESH_WRN_WARNING"),
                             tr("SMESH_WRN_NO_AVAILABLE_DATA"));
    onCancel();
    return;
  }

  SMESHGUI_Operation::startOperation();


  // backup mesh info before 2D mesh computation
  SMESH::long_array_var anOldInfo = myMesh->GetMeshInfo();
  

  if (!compute2DMesh()) {
    SUIT_MessageBox::warning(desktop(),
                             tr("SMESH_WRN_WARNING"),
                             tr("SMESH_WRN_COMPUTE_FAILED"));
    onCancel();
    return;
  }
  // get new mesh statistic
  SMESH::long_array_var aNewInfo = myMesh->GetMeshInfo();
  // get difference in mesh statistic from old to new
  for ( int i = SMDSEntity_Node; i < SMDSEntity_Last; i++ )
    aNewInfo[i] -= anOldInfo[i];

  // update presentation
  SMESH::Update(anIO, SMESH::eDisplay);

  // show computated result
  _PTR(SObject) aMeshSObj = SMESH::FindSObject(myMesh);
  if ( aMeshSObj )
    myDlg->SetMeshName( aMeshSObj->GetName().c_str() );
  myDlg->SetMeshInfo( aNewInfo );
  myDlg->show(); /*exec();*/
  commit();
  SMESHGUI::Modified();
}

//================================================================================
/*!
 * \brief compute 2D mesh on initial 3D
 */
//================================================================================

bool SMESHGUI_Make2DFrom3DOp::compute2DMesh()
{
  SUIT_OverrideCursor wc;
  SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
  return aMeshEditor->Make2DMeshFrom3D();
}
