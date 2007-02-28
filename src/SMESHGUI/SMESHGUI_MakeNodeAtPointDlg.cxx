//  SMESH SMESHGUI : GUI for SMESH component
//
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
//
//
//
//  File   : SMESHGUI_MakeNodeAtPointDlg.cxx
//  Author : Edward AGAPOV
//  Module : SMESH

#include "SMESHGUI_MakeNodeAtPointDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_MeshEditPreview.h"

#include "SMDS_Mesh.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMESH_Actor.h"
#include "SMESH_ActorUtils.h"

#include "GEOMBase.h"
#include "GeometryGUI.h"

#include "LightApp_DataOwner.h"
#include "LightApp_SelectionMgr.h"
#include "SALOMEDSClient_SObject.hxx"
#include "SALOME_ListIO.hxx"
#include "SUIT_Desktop.h"
#include "SVTK_Selector.h"
#include "SVTK_ViewWindow.h"
#include "SVTK_ViewModel.h"
#include "SalomeApp_Tools.h"
#include "SUIT_ResourceMgr.h"
#include "SUIT_OverrideCursor.h"

// OCCT Includes
#include <TColStd_MapOfInteger.hxx>
#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>

// QT Includes
#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qapplication.h>
#include <qstringlist.h>
#include <qcheckbox.h>
#include <qmessagebox.h>

#include <vtkProperty.h>

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 5
#define MARGIN  10

/*!
 * \brief Dialog to publish a sub-shape of the mesh main shape
 *        by selecting mesh elements
 */
SMESHGUI_MakeNodeAtPointDlg::SMESHGUI_MakeNodeAtPointDlg()
  : SMESHGUI_Dialog( 0, false, true )
{
  setCaption(tr("CAPTION"));

  QVBoxLayout* aDlgLay = new QVBoxLayout (mainFrame(), MARGIN, SPACING);

  QFrame* aMainFrame = createMainFrame  (mainFrame());

  aDlgLay->addWidget(aMainFrame);

  aDlgLay->setStretchFactor(aMainFrame, 1);
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_MakeNodeAtPointDlg::createMainFrame (QWidget* theParent)
{
  QFrame* aFrame = new QFrame(theParent);

  SUIT_ResourceMgr* rm = SMESH::GetResourceMgr( SMESHGUI::GetSMESHGUI() );
  QPixmap iconMoveNode (rm->loadPixmap("SMESH", tr("ICON_DLG_MOVE_NODE")));
  QPixmap iconSelect   (rm->loadPixmap("SMESH", tr("ICON_SELECT")));

  // constructor

  QButtonGroup* aPixGrp = new QButtonGroup(1, Qt::Vertical, tr("MESH_PASS_THROUGH_POINT"), aFrame);
  aPixGrp->setExclusive(TRUE);
  QRadioButton* aRBut = new QRadioButton(aPixGrp);
  aRBut->setPixmap(iconMoveNode);
  aRBut->setChecked(TRUE);

  // coordinates

  QGroupBox* aCoordGrp = new QGroupBox(1, Qt::Vertical, tr("SMESH_COORDINATES"), aFrame);
  myCoordBtn = new QPushButton(aCoordGrp);
  myCoordBtn->setPixmap(iconSelect);
  myCoordBtn->setToggleButton(TRUE);

  QLabel* aXLabel = new QLabel(tr("SMESH_X"), aCoordGrp);
  aXLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  myX = new SMESHGUI_SpinBox(aCoordGrp);

  QLabel* aYLabel = new QLabel(tr("SMESH_Y"), aCoordGrp);
  aYLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::ExpandTabs );
  myY = new SMESHGUI_SpinBox(aCoordGrp);

  QLabel* aZLabel = new QLabel(tr("SMESH_Z"), aCoordGrp);
  aZLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::ExpandTabs );
  myZ = new SMESHGUI_SpinBox(aCoordGrp);

  myX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 25.0, 3);
  myY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 25.0, 3);
  myZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 25.0, 3);

  // Method selection

  QButtonGroup* aMethodGrp = new QButtonGroup(1, Qt::Vertical, tr("METHOD"), aFrame);
  aMethodGrp->setExclusive(TRUE);
  myMoveRBtn = new QRadioButton(tr("MOVE_EXISTING_METHOD"), aMethodGrp);
  myCreateRBtn = new QRadioButton(tr("CREATE_NEW_METHOD"), aMethodGrp);

  // node ID

  myNodeToMoveGrp = new QGroupBox(tr("NODE_2MOVE"), aFrame, "anIdGrp");

  QLabel* idLabel = new QLabel(tr("NODE_2MOVE_ID"), myNodeToMoveGrp, "idLabel");
  myIdBtn = new QPushButton(myNodeToMoveGrp);
  myIdBtn->setPixmap(iconSelect);
  myIdBtn->setToggleButton(TRUE);
  myId = new QLineEdit(myNodeToMoveGrp,"myId");
  myId->setValidator(new SMESHGUI_IdValidator(this, "validator", 1));
  myAutoSearchChkBox = new QCheckBox( tr("AUTO_SEARCH"), myNodeToMoveGrp, "myAutoSearchChkBox");
  myPreviewChkBox = new QCheckBox( tr("PREVIEW"), myNodeToMoveGrp, "myPreviewChkBox");

  myNodeToMoveGrp->setColumnLayout(0, Qt::Vertical);
  myNodeToMoveGrp->layout()->setSpacing(0);
  myNodeToMoveGrp->layout()->setMargin(0);
  QGridLayout* myNodeToMoveGrpLayout = new QGridLayout(myNodeToMoveGrp->layout());
  myNodeToMoveGrpLayout->setAlignment(Qt::AlignTop);
  myNodeToMoveGrpLayout->setSpacing(SPACING);
  myNodeToMoveGrpLayout->setMargin(MARGIN);
  myNodeToMoveGrpLayout->addWidget( idLabel, 0, 0 );
  myNodeToMoveGrpLayout->addWidget( myIdBtn, 0, 1 );
  myNodeToMoveGrpLayout->addWidget( myId,    0, 2 );
  myNodeToMoveGrpLayout->addMultiCellWidget( myAutoSearchChkBox, 1, 1, 0, 2 );
  myNodeToMoveGrpLayout->addMultiCellWidget( myPreviewChkBox,    2, 2, 0, 2 );

  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->addWidget(aPixGrp);
  aLay->addWidget(aCoordGrp);
  aLay->addWidget(aMethodGrp);
  aLay->addWidget(myNodeToMoveGrp);

  connect(myCoordBtn,         SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));
  connect(myMoveRBtn,         SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));
  connect(myCreateRBtn,       SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));
  connect(myIdBtn,            SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));
  connect(myAutoSearchChkBox, SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));

  myMoveRBtn->setChecked(TRUE);
  myIdBtn->setOn(TRUE);
  myAutoSearchChkBox->setChecked(TRUE);

  return aFrame;
}

//================================================================================
/*!
 * \brief SLOT called when any button is toggled
  * \param bool - on or off
 */
//================================================================================

void SMESHGUI_MakeNodeAtPointDlg::ButtonToggled (bool on)
{
  const QObject* aSender = sender();
  if ( on ) {
    if ( aSender == myCoordBtn ) // button to set coord by node selection
    {
      if ( myIdBtn->isEnabled() )
        myIdBtn->setOn( !on );
    }
    else if ( aSender == myIdBtn ) // button to select a node to move
    {
      myCoordBtn->setOn( !on );
    }
    else if ( aSender == myMoveRBtn ) // move node method
    {
      myNodeToMoveGrp->setEnabled( TRUE );
    }
    else if ( aSender == myCreateRBtn ) // create node method
    {
      myNodeToMoveGrp->setEnabled( FALSE );
      myCoordBtn->setOn( TRUE ); 
    }
  }      
  if ( aSender == myAutoSearchChkBox ) // automatic node search
  {
    if ( on ) {
      myId->setText("");
      myId->setReadOnly ( TRUE );
      myIdBtn->setOn( FALSE );
      myIdBtn->setEnabled( FALSE );
      myCoordBtn->setOn( TRUE );
    }
    else {
      myId->setReadOnly ( FALSE );
      myIdBtn->setEnabled( TRUE );
    }
  }
}

//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_MakeNodeAtPointOp::SMESHGUI_MakeNodeAtPointOp()
{
  mySimulation = 0;
  myDlg = new SMESHGUI_MakeNodeAtPointDlg;

  // connect signals and slots
  connect(myDlg->myX, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  connect(myDlg->myY, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  connect(myDlg->myZ, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  connect(myDlg->myId,SIGNAL (textChanged(const QString&)),SLOT(redisplayPreview()));
  connect(myDlg->myPreviewChkBox,   SIGNAL (toggled(bool)),SLOT(redisplayPreview()));
  connect(myDlg->myAutoSearchChkBox,SIGNAL (toggled(bool)),SLOT(redisplayPreview()));
  connect(myDlg->myMoveRBtn,        SIGNAL (toggled(bool)),SLOT(redisplayPreview()));
  connect(myDlg->myCreateRBtn,      SIGNAL (toggled(bool)),SLOT(redisplayPreview()));
}

//=======================================================================
// function : startOperation()
// purpose  : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_MakeNodeAtPointOp::startOperation()
{
  myNoPreview = false;
  myMeshActor = 0;

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

  SMESHGUI_SelectionOp::startOperation();

  activateSelection(); // set filters

  myDlg->myX->SetValue(0);
  myDlg->myY->SetValue(0);
  myDlg->myZ->SetValue(0);
  myDlg->myId->setText("");
  myDlg->show();

  onSelectionDone(); // init myMeshActor

  if ( myMeshActor ) {
//     myMeshOldDisplayMode = myMeshActor->GetRepresentation();
//     myMeshActor->SetRepresentation( VTK_WIREFRAME );
    myMeshActor->SetPointRepresentation(true);
    SMESH::RepaintCurrentView();
    redisplayPreview();
  }
}

//================================================================================
/*!
 * \brief Stops operation
 */
//================================================================================

void SMESHGUI_MakeNodeAtPointOp::stopOperation()
{
  myNoPreview = true;
  mySimulation->SetVisibility(false);
  if ( myMeshActor ) {
//     myMeshActor->SetRepresentation( myMeshOldDisplayMode );
    myMeshActor->SetPointRepresentation(false);
    SMESH::RepaintCurrentView();
  }
  SMESHGUI_SelectionOp::stopOperation();
}

//================================================================================
/*!
 * \brief perform it's intention action: move or create a node
 */
//================================================================================

bool SMESHGUI_MakeNodeAtPointOp::onApply()
{
  if ( !isValid() ) return false; // node id is invalid

  if ( !myMeshActor ) return true;

  try {
    SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(myMeshActor->getIO());
    if (aMesh->_is_nil()) {
      QMessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                               tr("SMESHG_NO_MESH"), QMessageBox::Ok);
      return true;
    }
    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
    if (aMeshEditor->_is_nil())
      return true;

    int aResult = 0;
    if ( myDlg->myCreateRBtn->isOn() )
    {
      aResult = aMeshEditor->AddNode(myDlg->myX->GetValue(),
                                     myDlg->myY->GetValue(),
                                     myDlg->myZ->GetValue());
    }
    else
    {
      int anId = myDlg->myId->text().toInt();
      aResult = aMeshEditor->MoveClosestNodeToPoint(myDlg->myX->GetValue(),
                                                    myDlg->myY->GetValue(),
                                                    myDlg->myZ->GetValue(),
                                                    anId);
    }
    if (aResult)
    {
      myDlg->myId->setText("");
      
      SALOME_ListIO aList;
      aList.Append(myMeshActor->getIO());
      selectionMgr()->setSelectedObjects(aList,false);
      SMESH::UpdateView();
    }
  }
  catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }
  catch (...) {
  }

  return true;
}

//================================================================================
/*!
 * \brief Check data validity
 */
//================================================================================

bool SMESHGUI_MakeNodeAtPointOp::isValid()
{
  bool ok = true;

  if ( myMeshActor &&
       myDlg->myMoveRBtn->isOn() &&
       !myDlg->myAutoSearchChkBox->isChecked() )
  {
    ok = false;
    int id = myDlg->myId->text().toInt();
    if ( id > 0 )
      if (SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh())
        ok = aMesh->FindNode( id );
  }
  return ok;
}

//================================================================================
/*!
 * \brief SLOT called when selection changed
 */
//================================================================================

void SMESHGUI_MakeNodeAtPointOp::onSelectionDone()
{
  if ( !myDlg->isShown() || !myDlg->isEnabled() )
    return;
  try {
    SALOME_ListIO aList;
    selectionMgr()->selectedObjects(aList, SVTK_Viewer::Type());
    if (aList.Extent() != 1)
      return;
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    SMESH_Actor* aMeshActor = SMESH::FindActorByEntry(anIO->getEntry());
    if (!aMeshActor)
      return;
    if ( !myMeshActor )
      myMeshActor = aMeshActor;
    QString aString;
    int nbElems = SMESH::GetNameOfSelectedElements(selector(),anIO, aString);
    if (nbElems == 1) {
      if (SMDS_Mesh* aMesh = aMeshActor->GetObject()->GetMesh()) {
        if (const SMDS_MeshNode* aNode = aMesh->FindNode(aString.toInt())) {
          myNoPreview = true;
          if ( myDlg->myCoordBtn->isOn() ) { // set coord
            myDlg->myX->SetValue(aNode->X());
            myDlg->myY->SetValue(aNode->Y());
            myDlg->myZ->SetValue(aNode->Z());
            myNoPreview = false;
            redisplayPreview();
          }
          else if ( myDlg->myIdBtn->isOn() &&
                    myDlg->myIdBtn->isEnabled() ) { // set node to move
            myDlg->myId->setText(aString);
            myNoPreview = false;
            redisplayPreview();
          }
        }
      }
    }
  } catch (...) {
  }
}

//================================================================================
/*!
 * \brief update preview
 */
//================================================================================

void SMESHGUI_MakeNodeAtPointOp::redisplayPreview()
{
  if ( myNoPreview )
    return;
  myNoPreview = true;

  SMESH::MeshPreviewStruct_var aMeshPreviewStruct;

  bool moveShown = false;
  if ( myDlg->myMoveRBtn->isOn() && // Move method
       myMeshActor)
  {
    const bool autoSearch = myDlg->myAutoSearchChkBox->isChecked();
    const bool preview    = myDlg->myPreviewChkBox->isChecked();
    if ( autoSearch )
      myDlg->myId->setText("");
    if ( preview && ( autoSearch || isValid() ))
    {
      try {
        SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(myMeshActor->getIO());
        if (!aMesh->_is_nil()) {
          SMESH::SMESH_MeshEditor_var aPreviewer = aMesh->GetMeshEditPreviewer();
          if (!aPreviewer->_is_nil())
          {
            SUIT_OverrideCursor aWaitCursor;

            // find id and/or just compute preview
            int anId = aPreviewer->MoveClosestNodeToPoint(myDlg->myX->GetValue(),
                                                          myDlg->myY->GetValue(),
                                                          myDlg->myZ->GetValue(),
                                                          myDlg->myId->text().toInt());
            if ( autoSearch ) { // set found id
              QString idTxt("%1");
              if ( anId > 0 )
                idTxt = idTxt.arg( anId );
              else
                idTxt = "";
              myDlg->myId->setText( idTxt );
            }
            if ( preview ) { // fill preview data
              aMeshPreviewStruct = aPreviewer->GetPreviewData();
              moveShown = ( anId > 0 );
            }
          }
        }
      }catch (...) {
      }
    }
  }

  if ( !moveShown )
  {
    aMeshPreviewStruct = new SMESH::MeshPreviewStruct();

    aMeshPreviewStruct->nodesXYZ.length(1);
    aMeshPreviewStruct->nodesXYZ[0].x = myDlg->myX->GetValue();
    aMeshPreviewStruct->nodesXYZ[0].y = myDlg->myY->GetValue();
    aMeshPreviewStruct->nodesXYZ[0].z = myDlg->myZ->GetValue();

    aMeshPreviewStruct->elementTypes.length(1);
    aMeshPreviewStruct->elementTypes[0].SMDS_ElementType = SMESH::NODE;
    aMeshPreviewStruct->elementTypes[0].isPoly = false;
    aMeshPreviewStruct->elementTypes[0].nbNodesInElement = 1;

    aMeshPreviewStruct->elementConnectivities.length(1);
    aMeshPreviewStruct->elementConnectivities[0] = 0;
  }

  // display data
  if ( aMeshPreviewStruct.operator->() )
  {
    mySimulation->SetData(aMeshPreviewStruct._retn());
  }
  else
  {
    mySimulation->SetVisibility(false);
  }

  myNoPreview = false;
}

//================================================================================
/*!
 * \brief Activate Node selection
 */
//================================================================================

void SMESHGUI_MakeNodeAtPointOp::activateSelection()
{
  selectionMgr()->clearFilters();
  SMESH::SetPointRepresentation(false);
  setSelectionMode( NodeSelection );
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================

SMESHGUI_MakeNodeAtPointOp::~SMESHGUI_MakeNodeAtPointOp()
{
  if ( myDlg )
    delete myDlg;
  delete mySimulation;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
 * \retval LightApp_Dialog* - pointer to dialog of this operation
 */
//================================================================================

LightApp_Dialog* SMESHGUI_MakeNodeAtPointOp::dlg() const
{
  return myDlg;
}

