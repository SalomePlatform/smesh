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

// File   : SMESHGUI_AddNodeOnFaceDlg.cxx
// Author : Edward AGAPOV, Open CASCADE S.A.S.
//
#include "SMESHGUI_AddNodeOnFaceDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Filter.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_MeshEditPreview.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_VTKUtils.h"

#include <SMDS_Mesh.hxx>
#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMESH_TypeDefs.hxx>
#include <SMESH_MeshAlgos.hxx>

// SALOME GUI includes
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SalomeApp_Tools.h>
#include <SalomeApp_TypeFilter.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_MessageBox.h>
#include <SVTK_RenderWindowInteractor.h>
#include <SVTK_Renderer.h>
#include <SVTK_Event.h>

// Qt includes
#include <QApplication>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QButtonGroup>

// VTK includes
#include <vtkProperty.h>
#include <vtkCellPicker.h>
#include <vtkInteractorStyle.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkInteractorObserver.h>


// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

#define TOLERANCE 1e-3

//=======================================================================
/*!
 * \brief Dialog to publish a sub-shape of the mesh main shape
 *        by selecting mesh elements
 */
//=======================================================================

SMESHGUI_AddNodeOnFaceDlg::SMESHGUI_AddNodeOnFaceDlg()
  : SMESHGUI_Dialog( 0, false, true )
{
  setWindowTitle(tr("CAPTION"));

  QVBoxLayout* aDlgLay = new QVBoxLayout (mainFrame());
  aDlgLay->setMargin(0);
  aDlgLay->setSpacing(SPACING);
  myMainFrame = createMainFrame(mainFrame());

  aDlgLay->addWidget(myMainFrame);

  aDlgLay->setStretchFactor(myMainFrame, 1);
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's input fields
//=======================================================================

QWidget* SMESHGUI_AddNodeOnFaceDlg::createMainFrame (QWidget* theParent)
{
  QWidget* aFrame = new QWidget(theParent);

  SUIT_ResourceMgr* rm = SMESH::GetResourceMgr( SMESHGUI::GetSMESHGUI() );
  QPixmap iconSelect( rm->loadPixmap("SMESH", tr("ICON_SELECT")));

  // Face to split

  QGroupBox* faceGrp = new QGroupBox(tr("FACE_GROUP"), aFrame);
  faceGrp->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  QLabel* idLabel = new QLabel(tr("FACE_ID"), faceGrp);
  myIdBtn = new QPushButton(faceGrp);
  myIdBtn->setIcon(iconSelect);
  myIdBtn->setCheckable(true);
  myId = new QLineEdit(faceGrp);
  myId->setValidator(new SMESHGUI_IdValidator(this, 1));

  QGridLayout* faceGrpLayout = new QGridLayout(faceGrp);
  faceGrpLayout->setSpacing(SPACING);
  faceGrpLayout->setMargin(MARGIN);

  faceGrpLayout->addWidget( idLabel, 0, 0 );
  faceGrpLayout->addWidget( myIdBtn, 0, 1 );
  faceGrpLayout->addWidget( myId,    0, 2 );

  // Node location

  QGroupBox* xyzGrp = new QGroupBox(tr("XYZ_GROUP"), aFrame);
  xyzGrp->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myDestBtn = new QPushButton(xyzGrp);
  myDestBtn->setIcon(iconSelect);
  myDestBtn->setCheckable(true);

  QLabel* locationXLabel = new QLabel(tr("SMESH_X"), xyzGrp);
  myDestinationX = new SMESHGUI_SpinBox(xyzGrp);
  QLabel* locationYLabel = new QLabel(tr("SMESH_Y"), xyzGrp);
  myDestinationY = new SMESHGUI_SpinBox(xyzGrp);
  QLabel* locationZLabel = new QLabel(tr("SMESH_Z"), xyzGrp);
  myDestinationZ = new SMESHGUI_SpinBox(xyzGrp);

  myDestinationX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myDestinationY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myDestinationZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");

  myPointOnFace = new QCheckBox(tr("XYZ_NODE_ON_FACE"), xyzGrp);

  QGridLayout* aDestLayout = new QGridLayout(xyzGrp);
  aDestLayout->setMargin(MARGIN);
  aDestLayout->setSpacing(SPACING);
  aDestLayout->addWidget(myDestBtn, 0, 0);
  aDestLayout->addWidget(locationXLabel, 0, 1);
  aDestLayout->addWidget(myDestinationX, 0, 2);
  aDestLayout->addWidget(locationYLabel, 0, 3);
  aDestLayout->addWidget(myDestinationY, 0, 4);
  aDestLayout->addWidget(locationZLabel, 0, 5);
  aDestLayout->addWidget(myDestinationZ, 0, 6);
  aDestLayout->setColumnStretch(2, 1);
  aDestLayout->setColumnStretch(4, 1);
  aDestLayout->setColumnStretch(6, 1);
  aDestLayout->addWidget(myPointOnFace, 1, 0, 1, 6);

  // Preview
  myPreviewChkBox = new QCheckBox( tr("PREVIEW"), aFrame);
  myPreviewChkBox->setChecked( true );

  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->addWidget(faceGrp);
  aLay->addWidget(xyzGrp);
  aLay->addWidget(myPreviewChkBox);

  connect(myDestBtn, SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));
  connect(myIdBtn,   SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));

  myIdBtn->setChecked(true);

  return aFrame;
}

//================================================================================
/*!
 * \brief SLOT called when any button is toggled
  * \param bool - on or off
 */
//================================================================================

void SMESHGUI_AddNodeOnFaceDlg::ButtonToggled (bool on)
{
  const QObject* aSender = sender();
  if ( on ) {
    if ( aSender == myDestBtn ) // button to set coord by node selection
    {
      if ( myIdBtn->isEnabled() )
        myIdBtn->setChecked( !on );
    }
    else if ( aSender == myIdBtn ) // button to select a node to move
    {
      myDestBtn->setChecked( !on );
    }
  }
  emit selTypeChanged();
}

//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_AddNodeOnFaceOp::SMESHGUI_AddNodeOnFaceOp() : 
  SMESHGUI_InteractiveOp()
{
  mySimulation = 0;
  mySMESHGUI = 0;
  myDlg = new SMESHGUI_AddNodeOnFaceDlg;
  myHelpFileName = "add_node_on_face.html";

  myNoPreview = false;
  myUpdateDestination = false;

  // connect signals and slots
  connect(myDlg->myDestinationX,  SIGNAL (valueChanged(double)), this, SLOT(onDestCoordChanged()));
  connect(myDlg->myDestinationY,  SIGNAL (valueChanged(double)), this, SLOT(onDestCoordChanged()));
  connect(myDlg->myDestinationZ,  SIGNAL (valueChanged(double)), this, SLOT(onDestCoordChanged()));
  connect(myDlg->myId,            SIGNAL (textChanged(const QString&)),SLOT(redisplayPreview()));
  connect(myDlg->myPreviewChkBox, SIGNAL (toggled(bool)),              SLOT(redisplayPreview()));
  connect(myDlg,                  SIGNAL (selTypeChanged() ),          SLOT(onSelTypeChange()));
  connect(myDlg->myId,            SIGNAL (textChanged(const QString&)),SLOT(onTextChange(const QString&)));
  connect(myDlg->myPointOnFace,   SIGNAL(toggled(bool)),               SLOT(pointLocationChanged(bool)));

  myFacePicker = vtkCellPicker::New();
  myFacePicker->SetTolerance(0.005);
  myFacePicker->PickFromListOn();

}

//================================================================================
/*!
 * \brief SLOT. Called upon change of selection type
 */
//================================================================================

void SMESHGUI_AddNodeOnFaceOp::onSelTypeChange()
{
  if ( myDlg->myIdBtn->isChecked() )
  {
    setSelectionMode( FaceSelection );
  }
  else if ( myDlg->myDestBtn->isChecked() )
  {
    QString msg;
    if (isValid(msg)) {
      //Disconnect selectionChanged to keep selected element
      disconnect(selectionMgr(), SIGNAL(selectionChanged()), this, SLOT(onSelectionDone()));
      // Set selection mode to ActorSelection to avoid element's prehighlight during interactive selection
      setSelectionMode(ActorSelection);
      connect(selectionMgr(), SIGNAL(selectionChanged()), SLOT(onSelectionDone()));
    }
  }
  else
  {
    setSelectionMode( ActorSelection );
  }
}

//=======================================================================
// function : startOperation()
// purpose  : Init dialog fields, connect signals and slots, show dialog
//=======================================================================

void SMESHGUI_AddNodeOnFaceOp::startOperation()
{
  myNoPreview = false;
  myMeshActor = 0;

  // init simulation with a current View
  if ( mySimulation ) delete mySimulation;
  mySMESHGUI = getSMESHGUI();
  mySimulation = new SMESHGUI_MeshEditPreview(SMESH::GetViewWindow( mySMESHGUI ) );
  connect(mySMESHGUI, SIGNAL (SignalActivatedViewManager()), this, SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL (SignalCloseView()), this, SLOT(onCloseView()));
  vtkProperty* aProp = vtkProperty::New();
  aProp->SetRepresentationToWireframe();
  aProp->SetColor(250, 0, 250);
  aProp->SetPointSize(5);
  aProp->SetLineWidth( SMESH::GetFloat("SMESH:element_width",1) + 1);
  mySimulation->GetActor()->SetProperty(aProp);
  aProp->Delete();

  SMESHGUI_SelectionOp::startOperation();
  SMESHGUI_InteractiveOp::startOperation();
  myDlg->myId->setText("");
  myUpdateDestination = true;
  myDlg->myDestinationX->SetValue(0);
  myDlg->myDestinationY->SetValue(0);
  myDlg->myDestinationZ->SetValue(0);
  myUpdateDestination = false;

  addObserver();

  myDlg->show();

  onSelectionDone(); // init myMeshActor
}

//================================================================================
/*!
 * \brief Stops operation
 */
//================================================================================

void SMESHGUI_AddNodeOnFaceOp::stopOperation()
{
  myNoPreview = true;
  if ( mySimulation )
  {
    mySimulation->SetVisibility(false);
    delete mySimulation;
    mySimulation = 0;
  }
  if ( myMeshActor ) {
    myMeshActor = 0;
  }
  // SMESH::SetPointRepresentation( false );
  // SMESH::RepaintCurrentView();

  disconnect(mySMESHGUI, SIGNAL (SignalActivatedViewManager()), this, SLOT(onOpenView()));
  disconnect(mySMESHGUI, SIGNAL (SignalCloseView()),            this, SLOT(onCloseView()));
  SMESHGUI_SelectionOp::stopOperation();
  removeObserver();
}

//================================================================================
/*!
 * \brief perform it's intention action: split a face
 */
//================================================================================

bool SMESHGUI_AddNodeOnFaceOp::onApply()
{
  if( SMESHGUI::isStudyLocked() )
    return false;

  if ( !myMeshActor ) {
    SUIT_MessageBox::warning( dlg(), tr( "SMESH_WRN_WARNING" ), tr("INVALID_MESH") );
    dlg()->show();
    return false;
  }

  QString msg;
  if ( !isValid( msg ) ) { // node id is invalid
    if( !msg.isEmpty() )
      SUIT_MessageBox::warning( dlg(), tr( "SMESH_WRN_WARNING" ), tr("INVALID_ID") );
    dlg()->show();
    return false;
  }

  QStringList aParameters;
  aParameters << myDlg->myDestinationX->text();
  aParameters << myDlg->myDestinationY->text();
  aParameters << myDlg->myDestinationZ->text();

  try {
    SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(myMeshActor->getIO());
    if (aMesh->_is_nil()) {
      SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"), tr("SMESHG_NO_MESH") );
      return true;
    }
    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
    if (aMeshEditor->_is_nil())
      return true;

    aMesh->SetParameters( aParameters.join(":").toUtf8().constData() );

    bool ok;
    SMESH::smIdType anId = myDlg->myId->text().toLong( &ok );

    aMeshEditor->AddNodeOnFace( anId,
                                myDlg->myDestinationX->GetValue(),
                                myDlg->myDestinationY->GetValue(),
                                myDlg->myDestinationZ->GetValue() );
    SALOME_ListIO aList;
    selectionMgr()->setSelectedObjects(aList,false);
    aList.Append(myMeshActor->getIO());
    selectionMgr()->setSelectedObjects(aList,false);
    onSelectionDone();
    SMESH::UpdateView();
    SMESHGUI::Modified();
  }
  catch (const SALOME_CMOD::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }
  catch (...) {
  }

  return true;
}

//================================================================================
/*!
 * \brief Check selected face id validity
 */
//================================================================================

bool SMESHGUI_AddNodeOnFaceOp::isValid( QString& msg )
{
  bool ok = false;
  if ( myMeshActor )
  {
    SMESH::smIdType id = myDlg->myId->text().toLong();
    if ( id > 0 )
      if (SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh())
        if ( const SMDS_MeshElement* face = aMesh->FindElement( id ))
        {
          if ( face->GetType() == SMDSAbs_Face )
            ok = true;
          else
            msg += tr("NOT_FACE") + "\n";
        }
    if ( !ok )
      msg += tr("INVALID_ID") + "\n";
  }

  ok = myDlg->myDestinationX->isValid( msg, !myNoPreview ) && ok;
  ok = myDlg->myDestinationY->isValid( msg, !myNoPreview ) && ok;
  ok = myDlg->myDestinationZ->isValid( msg, !myNoPreview ) && ok;

  return ok;
}

//================================================================================
/*!
 * \brief SLOT called when selection changed
 */
//================================================================================

void SMESHGUI_AddNodeOnFaceOp::onSelectionDone()
{
  if ( !myDlg->isVisible() || !myDlg->isEnabled() )
    return;

  myNoPreview = true;
  QString idStr;
  try {
    SALOME_ListIO aList;
    selectionMgr()->selectedObjects(aList, SVTK_Viewer::Type());
    if (aList.Extent() != 1)
      return;
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    myMeshActor = SMESH::FindActorByEntry(anIO->getEntry());
    myFacePicker->InitializePickList();
    myFacePicker->AddPickList(myMeshActor);

    QString aString;
    int nbElems = SMESH::GetNameOfSelectedElements(selector(),anIO, aString);
    if (nbElems == 1)
      if ( SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh())
        if ( const SMDS_MeshElement* face = aMesh->FindElement( aString.toLong() ))
        {
          idStr = aString;
          // set coordinates to face gravity center
          gp_XYZ faceGC( 0,0,0 );
          for ( int i = 0; i < face->NbCornerNodes(); ++i )
            faceGC += SMESH_NodeXYZ( face->GetNode( i ));
          faceGC /= face->NbCornerNodes();
          myUpdateDestination = true;
          myDlg->myDestinationX->SetValue(faceGC.X());
          myDlg->myDestinationY->SetValue(faceGC.Y());
          myDlg->myDestinationZ->SetValue(faceGC.Z());
          myUpdateDestination = false;
        }
  } catch (...) {
  }
  myDlg->myId->setText( idStr );

  myNoPreview = false;
  redisplayPreview();
}

//================================================================================
/*!
 * \brief update preview
 */
//================================================================================

void SMESHGUI_AddNodeOnFaceOp::redisplayPreview()
{
  if ( myNoPreview )
    return;
  myNoPreview = true;

  SMESH::MeshPreviewStruct_var aMeshPreviewStruct;

  if ( myMeshActor && myDlg->myPreviewChkBox->isChecked() )
  {
    QString msg;
    if ( isValid( msg ))
    {
      try {
        SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(myMeshActor->getIO());
        if (!aMesh->_is_nil()) {
          SMESH::SMESH_MeshEditor_var aPreviewer = aMesh->GetMeshEditPreviewer();
          if (!aPreviewer->_is_nil())
          {
            SUIT_OverrideCursor aWaitCursor;

            SMESH::smIdType anId = myDlg->myId->text().toLong();
            aPreviewer->AddNodeOnFace(anId,
                                      myDlg->myDestinationX->GetValue(),
                                      myDlg->myDestinationY->GetValue(),
                                      myDlg->myDestinationZ->GetValue());
            aMeshPreviewStruct = aPreviewer->GetPreviewData();
          }
        }
      }
      catch (...) {
      }
    }
  }
  if (!mySimulation)
    mySimulation = new SMESHGUI_MeshEditPreview(SMESH::GetViewWindow( mySMESHGUI ));
  // display data
  if ( & aMeshPreviewStruct.in() )
  {
    mySimulation->SetData( aMeshPreviewStruct.in() );
  }
  else
  {
    mySimulation->SetVisibility(false);
  }

  myNoPreview = false;
}

//=================================================================================
/*!
 * \brief SLOT called when the viewer opened
 */
//=================================================================================

void SMESHGUI_AddNodeOnFaceOp::onOpenView()
{
  if ( mySimulation ) {
    mySimulation->SetVisibility(false);
    SMESH::SetPointRepresentation(false);
  }
  else {
    mySimulation = new SMESHGUI_MeshEditPreview(SMESH::GetViewWindow( mySMESHGUI ));
  }
}

//=================================================================================
/*!
 * \brief SLOT called when the viewer closed
 */
//=================================================================================

void SMESHGUI_AddNodeOnFaceOp::onCloseView()
{
  delete mySimulation;
  mySimulation = 0;
}

//================================================================================
/*!
 * \brief SLOT called when the face id is manually changed
 */
//================================================================================

void SMESHGUI_AddNodeOnFaceOp::onTextChange( const QString& theText )
{
  if( myMeshActor )
  {
    if( SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh() )
    {
      Handle(SALOME_InteractiveObject) anIO = myMeshActor->getIO();
      SALOME_ListIO aList;
      aList.Append( anIO );
      selectionMgr()->setSelectedObjects( aList, false );

      if( const SMDS_MeshElement* face = aMesh->FindElement( theText.toLong() ) )
      {
        SVTK_TVtkIDsMap aListInd;
        aListInd.Add( FromSmIdType<int>( face->GetID()) );
        selector()->AddOrRemoveIndex( anIO, aListInd, false );
        if( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( SMESHGUI::GetSMESHGUI() ) )
          aViewWindow->highlight( anIO, true, true );
      }
    }
  }
}

//================================================================================
/*!
 * \brief Activate selection
 */
//================================================================================

void SMESHGUI_AddNodeOnFaceOp::activateSelection()
{
  selectionMgr()->clearFilters();
  SMESH::SetPointRepresentation( false );
  onSelTypeChange();
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================

SMESHGUI_AddNodeOnFaceOp::~SMESHGUI_AddNodeOnFaceOp()
{
  if ( myDlg )        delete myDlg;
  if ( mySimulation ) delete mySimulation;
  myFacePicker->Delete();
}

//================================================================================
/*!
 * \brief SLOT called when destination coordinates are changed
*/
//================================================================================
void SMESHGUI_AddNodeOnFaceOp::onDestCoordChanged() 
{
  if (myUpdateDestination)
    return;
  pointLocationChanged(myDlg->myPointOnFace->isChecked());
  redisplayPreview();
}

//================================================================================
/*!
 * \brief SLOT called when 'Node on face' checkbox is changed
*/
//================================================================================
void SMESHGUI_AddNodeOnFaceOp::pointLocationChanged(bool onFace) {
  if (onFace) {
    QString msg;
    if (myMeshActor && isValid(msg)) {
      SMESH::smIdType id = myDlg->myId->text().toLong();
      if (id > 0) {
        if (SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh()) {
          if (const SMDS_MeshElement* face = aMesh->FindElement(id))
          {
            if (face->GetType() == SMDSAbs_Face) {
              gp_Pnt point(myDlg->myDestinationX->GetValue(), myDlg->myDestinationY->GetValue(),myDlg->myDestinationZ->GetValue());
              gp_XYZ closestPnt;
              double dist = SMESH_MeshAlgos::GetDistance( face, point, &closestPnt);
              if (dist > TOLERANCE) {
                myUpdateDestination = true;
                myDlg->myDestinationX->SetValue(closestPnt.X());
                myDlg->myDestinationY->SetValue(closestPnt.Y());
                myDlg->myDestinationZ->SetValue(closestPnt.Z());
                myUpdateDestination = false;
                redisplayPreview();
              }
            }
          }
        }
      }
    }
  }
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
 * \retval LightApp_Dialog* - pointer to dialog of this operation
 */
//================================================================================

LightApp_Dialog* SMESHGUI_AddNodeOnFaceOp::dlg() const
{
  return myDlg;
}

//================================================================================
/*
* \brief Process InteractiveSelectionChanged event
*/
//================================================================================
void SMESHGUI_AddNodeOnFaceOp::processStyleEvents(unsigned long theEvent, void* theCallData)
{
  (void*)theCallData;
  QString msg;
  if (isValid(msg)) {
    if (theEvent == SVTK::InteractiveSelectionChanged) {
      if (myDlg->myPointOnFace->isChecked()) {
        // Pick point on a mesh surface
        QString msg;
        if (myMeshActor && isValid(msg)) {
          SMESH::smIdType id = myDlg->myId->text().toLong();
          if (id > 0) {
            if (SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh()) {
              if (const SMDS_MeshElement* face = aMesh->FindElement(id))
              {
                if (face->GetType() == SMDSAbs_Face) {
                  int xClick, yClick; // Last event (move or left button bown) position
                  myRWInteractor->GetDevice()->GetEventPosition(xClick, yClick);
                  gp_XYZ faceNode(0, 0, 0);
                  SMDS_Mesh tmp;
                  double Ni[3];
                  std::vector<const SMDS_MeshNode*> tmpNodes;
                  for (int i = 0; i < face->NbCornerNodes(); ++i) {
                    faceNode = SMESH_NodeXYZ(face->GetNode(i));
                    vtkInteractorObserver::ComputeWorldToDisplay(myRWInteractor->GetRenderer()->GetDevice(),
                      faceNode.X(), faceNode.Y(), faceNode.Z(), Ni);
                    tmpNodes.push_back(tmp.AddNode(Ni[0], Ni[1], 0));
                  }
                  SMDS_MeshFace* face2D = tmp.AddPolygonalFace(tmpNodes);
                  gp_Pnt point(double(xClick), double(yClick), 0);
                  gp_XYZ closestPnt;
                  double dist = SMESH_MeshAlgos::GetDistance(face2D, point, &closestPnt);
                  double xPick = 0, yPick = 0;
                  if (dist < TOLERANCE) {
                    xPick = xClick;
                    yPick = yClick;
                  }
                  else {
                    xPick = closestPnt.X();
                    yPick = closestPnt.Y();
                  }
                  myFacePicker->Pick(xPick, yPick, 0.0, myRWInteractor->GetRenderer()->GetDevice());
                  double closest[3];
                  myFacePicker->GetPickPosition(closest);
                  myUpdateDestination = true;
                  myDlg->myDestinationX->SetValue(closest[0]);
                  myDlg->myDestinationY->SetValue(closest[1]);
                  myDlg->myDestinationZ->SetValue(closest[2]);
                  myUpdateDestination = false;
                  redisplayPreview();
                }
              }
            }
          }
        }
      }
      else {
        double* aCoord = (double*)theCallData;
        myUpdateDestination = true;
        myDlg->myDestinationX->SetValue(aCoord[0]);
        myDlg->myDestinationY->SetValue(aCoord[1]);
        myDlg->myDestinationZ->SetValue(aCoord[2]);
        myUpdateDestination = false;
        redisplayPreview();
      }
    }
  }
}

//================================================================================
/*
* \brief Process LeftButtonPressEvent event: activate interactive selection
*/
//================================================================================
void SMESHGUI_AddNodeOnFaceOp::processInteractorEvents(unsigned long theEvent, void* theCallData)
{
  (void*)theCallData;
  if (theEvent == vtkCommand::LeftButtonPressEvent && myDlg->myDestBtn->isChecked()) {
    bool control = myRWInteractor->GetDevice()->GetControlKey();
    bool shift = myRWInteractor->GetDevice()->GetControlKey();
    SVTK_ViewWindow* svtkViewWindow = SMESH::GetViewWindow(mySMESHGUI);
    if (svtkViewWindow && !shift && !control) {
      QString msg;
      if (isValid(msg)) {
        svtkViewWindow->activateInteractiveSelection();
      }
    }
  }
}
