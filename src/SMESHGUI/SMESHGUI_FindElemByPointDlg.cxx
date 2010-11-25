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

// File   : SMESHGUI_FindElemByPointDlg.cxx
// Author : Edward AGAPOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_FindElemByPointDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_MeshEditPreview.h"

#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMESH_TypeFilter.hxx>

// SALOME GUI includes
#include <LightApp_SelectionMgr.h>
#include <QtxComboBox.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SalomeApp_Tools.h>
#include <SalomeApp_TypeFilter.h>

// Qt includes
#include <QAbstractButton>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

// VTK includes
#include <vtkProperty.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

//=======================================================================
/*!
 * \brief Dialog to find elements by a point coordinates
 */
//=======================================================================

SMESHGUI_FindElemByPointDlg::SMESHGUI_FindElemByPointDlg()
  : SMESHGUI_Dialog( 0, false, true, OK | Help )
{
  setWindowTitle(tr("CAPTION"));

  QVBoxLayout* aDlgLay = new QVBoxLayout (mainFrame());
  aDlgLay->setMargin(0);
  aDlgLay->setSpacing(SPACING);

  QWidget* aMainFrame = createMainFrame  (mainFrame());

  aDlgLay->addWidget(aMainFrame);
  aDlgLay->setStretchFactor(aMainFrame, 1);
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's input fields
//=======================================================================
QWidget* SMESHGUI_FindElemByPointDlg::createMainFrame (QWidget* theParent)
{
  QWidget* aFrame = new QWidget(theParent);

  //mesh name

  QGroupBox* aMeshGrp = new QGroupBox(tr("SMESH_MESH"), aFrame);
  QHBoxLayout* aMeshGrpLayout = new QHBoxLayout(aMeshGrp);
  aMeshGrpLayout->setMargin(MARGIN);
  aMeshGrpLayout->setSpacing(SPACING);

  myMeshName = new QLineEdit(aMeshGrp);
  aMeshGrpLayout->addWidget(myMeshName);

  // coordinates

  QGroupBox* aCoordGrp = new QGroupBox(tr("SMESH_COORDINATES"), aFrame);
  QHBoxLayout* aCoordGrpLayout = new QHBoxLayout(aCoordGrp);
  aCoordGrpLayout->setMargin(MARGIN);
  aCoordGrpLayout->setSpacing(SPACING);

  QLabel* aXLabel = new QLabel(tr("SMESH_X"), aCoordGrp);
  myX = new SMESHGUI_SpinBox(aCoordGrp);

  QLabel* aYLabel = new QLabel(tr("SMESH_Y"), aCoordGrp);
  myY = new SMESHGUI_SpinBox(aCoordGrp);

  QLabel* aZLabel = new QLabel(tr("SMESH_Z"), aCoordGrp);
  myZ = new SMESHGUI_SpinBox(aCoordGrp);

  myX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myX->SetValue(0);
  myY->SetValue(0);
  myZ->SetValue(0);

  aCoordGrpLayout->addWidget(aXLabel);
  aCoordGrpLayout->addWidget(myX);
  aCoordGrpLayout->addWidget(aYLabel);
  aCoordGrpLayout->addWidget(myY);
  aCoordGrpLayout->addWidget(aZLabel);
  aCoordGrpLayout->addWidget(myZ);

  // Elements

  QGroupBox* elementGrp = new QGroupBox(tr("Elements"), aFrame);
  QGridLayout* elementGrpLayout = new QGridLayout(elementGrp);
  elementGrpLayout->setSpacing(SPACING);
  elementGrpLayout->setMargin(MARGIN);


  myFindBtn = new QPushButton(elementGrp);
  myFindBtn->setText(tr("Find"));
  //myFindBtn->setCheckable(false);

  myElemTypeCombo = new QtxComboBox(elementGrp);
  myElemTypeCombo->addItem( tr( "MEN_ALL" ));    myElemTypeCombo->setId( 0, int( SMESH::ALL   ));
  myElemTypeCombo->addItem( tr( "MEN_NODE" ));   myElemTypeCombo->setId( 1, int( SMESH::NODE  ));
  myElemTypeCombo->addItem( tr( "MEN_EDGE" ));   myElemTypeCombo->setId( 2, int( SMESH::EDGE  ));
  myElemTypeCombo->addItem( tr( "MEN_FACE" ));   myElemTypeCombo->setId( 3, int( SMESH::FACE  ));
  myElemTypeCombo->addItem( tr("MEN_VOLUME_3D"));myElemTypeCombo->setId( 4, int( SMESH::VOLUME));
  myElemTypeCombo->addItem( tr( "MEN_ELEM0D" )); myElemTypeCombo->setId( 5, int( SMESH::ELEM0D));

  myFoundList = new QListWidget(elementGrp);

  elementGrpLayout->addWidget( myFindBtn,       0, 0 );
  elementGrpLayout->addWidget( myElemTypeCombo, 0, 1 );
  elementGrpLayout->addWidget( myFoundList,     1, 0, 2, 2 );


  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->setMargin( 0 );
  aLay->setSpacing( SPACING );
  aLay->addWidget(aMeshGrp);
  aLay->addWidget(aCoordGrp);
  aLay->addWidget(elementGrp);

  // OK instead of "Apply and Close"
  if ( QAbstractButton* but = button(OK) )
    but->setText( tr("SMESH_BUT_OK"));

  return aFrame;
}

//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_FindElemByPointOp::SMESHGUI_FindElemByPointOp()
  :SMESHGUI_SelectionOp()
{
  mySimulation = 0;
  myDlg = new SMESHGUI_FindElemByPointDlg;
  myHelpFileName = "find_element_by_point_page.html";
  myFilter = new SMESH_TypeFilter( MESH );

  myPreview = new SMESH::MeshPreviewStruct();

  myPreview->nodesXYZ.length(1);
  myPreview->nodesXYZ[0].x = myDlg->myX->GetValue();
  myPreview->nodesXYZ[0].y = myDlg->myY->GetValue();
  myPreview->nodesXYZ[0].z = myDlg->myZ->GetValue();

  myPreview->elementTypes.length(1);
  myPreview->elementTypes[0].SMDS_ElementType = SMESH::NODE;
  myPreview->elementTypes[0].isPoly = false;
  myPreview->elementTypes[0].nbNodesInElement = 1;

  myPreview->elementConnectivities.length(1);
  myPreview->elementConnectivities[0] = 0;

  // connect signals and slots
  connect(myDlg->myFindBtn,      SIGNAL(clicked()),               this, SLOT(onFind()));
  connect(myDlg->myX,            SIGNAL(valueChanged(double)),    this, SLOT(redisplayPreview()));
  connect(myDlg->myY,            SIGNAL(valueChanged(double)),    this, SLOT(redisplayPreview()));
  connect(myDlg->myZ,            SIGNAL(valueChanged(double)),    this, SLOT(redisplayPreview()));
  connect(myDlg->myFoundList,    SIGNAL(itemSelectionChanged()),  this, SLOT(onElemSelected()));
  connect(myDlg->myElemTypeCombo,SIGNAL(currentIndexChanged(int)),this, SLOT(onElemTypeChange(int)));
}

//=======================================================================
// function : startOperation()
// purpose  : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_FindElemByPointOp::startOperation()
{
  // init simulation with a current View
  if ( mySimulation ) delete mySimulation;
  mySimulation = new SMESHGUI_MeshEditPreview(SMESH::GetViewWindow( getSMESHGUI() ));
  vtkProperty* aProp = vtkProperty::New();
  aProp->SetRepresentationToWireframe();
  aProp->SetColor(250, 0, 250);
  aProp->SetPointSize(5);
  aProp->SetLineWidth( SMESH::GetFloat("SMESH:element_width",1) + 1);
  mySimulation->GetActor()->SetProperty(aProp);
  aProp->Delete();

  myDlg->myElemTypeCombo->setCurrentId( int(SMESH::ALL));

  SMESHGUI_SelectionOp::startOperation();
  myDlg->show();
  redisplayPreview();

  onSelectionDone(); // init myMesh
}

//================================================================================
/*!
 * \brief Stops operation
 */
//================================================================================

void SMESHGUI_FindElemByPointOp::stopOperation()
{
  if ( mySimulation )
  {
    mySimulation->SetVisibility(false);
    delete mySimulation;
    mySimulation = 0;
  }
  selectionMgr()->removeFilter( myFilter );
  SMESHGUI_SelectionOp::stopOperation();
}

//================================================================================
/*!
 * \brief hilight found selected elements
 */
//================================================================================

void SMESHGUI_FindElemByPointOp::onElemSelected()
{
  if ( !myMeshIO.IsNull() )
  {
    Selection_Mode selMode =
      myDlg->myElemTypeCombo->currentId() == int(SMESH::NODE) ? NodeSelection : CellSelection;
    if ( selectionMode() != selMode )
      setSelectionMode( selMode );

    QList<QListWidgetItem *> ids = myDlg->myFoundList->selectedItems();
    QList<QListWidgetItem*>::iterator id = ids.begin();
    TColStd_MapOfInteger idMap;
    for ( ; id != ids.end(); ++id )
      idMap.Add( (*id)->text().toInt() );

    addOrRemoveIndex( myMeshIO, idMap, false );

    SALOME_ListIO aList;
    aList.Append(myMeshIO);
    selectionMgr()->setSelectedObjects(aList,false);
  }
}

//================================================================================
/*!
 * \brief Set selection mode according to element type
 */
//================================================================================

void SMESHGUI_FindElemByPointOp::onElemTypeChange(int index)
{
  Selection_Mode newMode = (index == 1) ? NodeSelection : CellSelection;
  if ( selectionMode() != newMode )
  {
    selectionMgr()->clearFilters();
    setSelectionMode( newMode );
  }
  myDlg->myFoundList->clear();
}

//================================================================================
/*!
 * \brief perform it's intention action: find elements
 */
//================================================================================

void SMESHGUI_FindElemByPointOp::onFind()
{
  if ( myMesh->_is_nil() ) {
    return;
  }

  try {
    SUIT_OverrideCursor wc;

    SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
    if (aMeshEditor->_is_nil())
      return;

    SMESH::long_array_var foundIds =
      aMeshEditor->FindElementsByPoint( myDlg->myX->GetValue(),
                                        myDlg->myY->GetValue(),
                                        myDlg->myZ->GetValue(),
                                        SMESH::ElementType( myDlg->myElemTypeCombo->currentId() ));
    myDlg->myFoundList->clear();
    for ( int i = 0; i < foundIds->length(); ++i )
      myDlg->myFoundList->addItem( QString::number( foundIds[i] ));

    if ( foundIds->length() > 0 )
      myDlg->myFoundList->setCurrentRow(0);
  }
  catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }
  catch (...) {
  }
}

//================================================================================
/*!
 * \brief Method needed for internal cuisine
 */
//================================================================================

bool SMESHGUI_FindElemByPointOp::onApply()
{
  return true;
}

//================================================================================
/*!
 * \brief SLOT called when selection changed
 */
//================================================================================

void SMESHGUI_FindElemByPointOp::onSelectionDone()
{
  if ( !myDlg->isVisible() || !myDlg->isEnabled() )
    return;

  QString oldMeshEntry, newMeshEntry;
  if ( !myMeshIO.IsNull() && myMeshIO->hasEntry() )
    oldMeshEntry = myMeshIO->getEntry();

  myDlg->myMeshName->setText("");
  myMeshIO.Nullify();

  try {
    SALOME_ListIO aList;
    selectionMgr()->selectedObjects(aList, SVTK_Viewer::Type());
    if (aList.Extent() == 1 && aList.First()->hasEntry())
    {
      Handle(SALOME_InteractiveObject) anIO = aList.First();
      _PTR(SObject) pObj = studyDS()->FindObjectID(anIO->getEntry());
      myMesh = SMESH::GetMeshByIO( anIO );
      if ( pObj && !myMesh->_is_nil() )
      {
        myMeshIO = anIO;
        myDlg->myMeshName->setText( pObj->GetName().c_str() );
        newMeshEntry = anIO->getEntry();
      }
    }
  }
  catch (...) {
  }

  if ( oldMeshEntry != newMeshEntry || newMeshEntry.isEmpty() )
    myDlg->myFoundList->clear();

  myDlg->myFindBtn->setEnabled( !myMeshIO.IsNull() );
}

//================================================================================
/*!
 * \brief show point by coordinates
 */
//================================================================================

void SMESHGUI_FindElemByPointOp::redisplayPreview()
{
  myDlg->myFoundList->clear();

  myPreview->nodesXYZ[0].x = myDlg->myX->GetValue();
  myPreview->nodesXYZ[0].y = myDlg->myY->GetValue();
  myPreview->nodesXYZ[0].z = myDlg->myZ->GetValue();

  mySimulation->SetData(&myPreview.in());
}

//================================================================================
/*!
 * \brief install filter on meshes
 */
//================================================================================

void SMESHGUI_FindElemByPointOp::activateSelection()
{
  selectionMgr()->clearFilters();
  selectionMgr()->installFilter( myFilter );
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================

SMESHGUI_FindElemByPointOp::~SMESHGUI_FindElemByPointOp()
{
  if ( myDlg )         { delete myDlg;        myDlg = 0; }
  if ( mySimulation )  { delete mySimulation; mySimulation = 0; }
  if ( myFilter )      { delete myFilter;     myFilter = 0; }
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
 * \retval LightApp_Dialog* - pointer to dialog of this operation
 */
//================================================================================

LightApp_Dialog* SMESHGUI_FindElemByPointOp::dlg() const
{
  return myDlg;
}

