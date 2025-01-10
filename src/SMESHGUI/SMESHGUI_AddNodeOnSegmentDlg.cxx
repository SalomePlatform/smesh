// Copyright (C) 2007-2025  CEA, EDF, OPEN CASCADE
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

// File   : SMESHGUI_AddNodeOnSegmentDlg.cxx
// Author : Edward AGAPOV, Open CASCADE S.A.S.
//
#include "SMESHGUI_AddNodeOnSegmentDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_MeshEditPreview.h"

#include <SMDS_Mesh.hxx>
#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMESH_TypeDefs.hxx>

#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SVTK_RenderWindowInteractor.h>
#include <SVTK_Renderer.h>
#include <SVTK_Event.h>
#include <SalomeApp_Tools.h>

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
#include <vtkInteractorStyle.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkInteractorObserver.h>
#include <vtkLine.h>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

#define SPIN_TOLERANCE 1e-3

//=======================================================================
/*!
 * \brief Dialog to split a diagonal of a quadrangle formed by two adjacent triangles
 */
//=======================================================================

SMESHGUI_AddNodeOnSegmentDlg::SMESHGUI_AddNodeOnSegmentDlg()
: SMESHGUI_Dialog( 0, false, true )
{
  setWindowTitle(tr("CAPTION"));

  QVBoxLayout* aDlgLay = new QVBoxLayout (mainFrame());
  aDlgLay->setMargin(0);
  aDlgLay->setSpacing(SPACING);
  QWidget* mainFr = createMainFrame(mainFrame());

  aDlgLay->addWidget( mainFr );

  aDlgLay->setStretchFactor( mainFr, 1);
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's input fields
//=======================================================================

QWidget* SMESHGUI_AddNodeOnSegmentDlg::createMainFrame (QWidget* theParent)
{
  QWidget* aFrame = new QWidget(theParent);

  SUIT_ResourceMgr* rm = SMESH::GetResourceMgr( SMESHGUI::GetSMESHGUI() );
  QPixmap iconSelect( rm->loadPixmap("SMESH", tr("ICON_SELECT")));

  // Segment

  QGroupBox* segmentGrp = new QGroupBox(tr("SEGMENT_GROUP"), aFrame);
  segmentGrp->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  QLabel* segmentLabel = new QLabel(tr("SEGMENT"), segmentGrp);
  mySegmentBtn = new QPushButton(segmentGrp);
  mySegmentBtn->setIcon(iconSelect);
  mySegmentBtn->setCheckable(true);
  mySegment = new QLineEdit(segmentGrp);
  mySegment->setValidator(new QRegExpValidator(QRegExp("[\\d]*-[\\d]*"), this));

  QGridLayout* segmentGrpLayout = new QGridLayout(segmentGrp);
  segmentGrpLayout->setSpacing(SPACING);
  segmentGrpLayout->setMargin(MARGIN);

  segmentGrpLayout->addWidget( segmentLabel, 0, 0 );
  segmentGrpLayout->addWidget( mySegmentBtn, 0, 1 );
  segmentGrpLayout->addWidget( mySegment,    0, 2 );

  // Position on segment

  QGroupBox* positionGrp = new QGroupBox(tr("POSITION_GROUP"), aFrame);
  positionGrp->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myPositionBtn = new QPushButton(positionGrp);
  myPositionBtn->setIcon(iconSelect);
  myPositionBtn->setCheckable(true);

  QLabel* positionLbl = new QLabel(tr("POSITION"), positionGrp);

  myPositionSpin = new SMESHGUI_SpinBox(positionGrp);
  myPositionSpin->setReadOnly(false);
  myPositionSpin->RangeStepAndValidator(SPIN_TOLERANCE, 1- SPIN_TOLERANCE, 0.1, "length_precision");

  QGridLayout* positionLayout = new QGridLayout(positionGrp);
  positionLayout->setMargin(MARGIN);
  positionLayout->setSpacing(SPACING);
  positionLayout->addWidget(positionLbl, 0, 0);
  positionLayout->addWidget(myPositionBtn, 0, 1);
  positionLayout->addWidget(myPositionSpin, 0, 2);
  positionLayout->setColumnStretch(2, 1);

  // Preview

  myPreviewChkBox = new QCheckBox( tr("PREVIEW"), aFrame);
  myPreviewChkBox->setChecked( true );

  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->addWidget(segmentGrp);
  aLay->addWidget(positionGrp);
  aLay->addWidget(myPreviewChkBox);

  connect(myPositionBtn, SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));
  connect(mySegmentBtn,  SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));

  mySegmentBtn->setChecked(true);

  return aFrame;
}

//================================================================================
/*!
 * \brief SLOT called when any button is toggled
 * \param bool - on or off
 */
//================================================================================

void SMESHGUI_AddNodeOnSegmentDlg::ButtonToggled (bool on)
{
  const QObject* aSender = sender();
  if ( on ) {
    if ( aSender == myPositionBtn )
    {
      mySegmentBtn->setChecked( !on );
    }
    else if ( aSender == mySegmentBtn )
    {
      myPositionBtn->setChecked( !on );
    }
  }
  emit selTypeChanged();
}

//================================================================================
/*!
 * \brief Constructor
 */
//================================================================================

SMESHGUI_AddNodeOnSegmentOp::SMESHGUI_AddNodeOnSegmentOp() :
  SMESHGUI_InteractiveOp()
{
  mySimulation = 0;
  mySMESHGUI = 0;
  myDlg = new SMESHGUI_AddNodeOnSegmentDlg;
  myHelpFileName = "add_node_on_segment.html";

  myNoPreview = false;

  // connect signals and slots
  connect(myDlg->myPreviewChkBox, SIGNAL (toggled(bool)),              SLOT(redisplayPreview()));
  connect(myDlg->myPositionSpin,  SIGNAL (valueChanged(double)),       SLOT(redisplayPreview()));
  connect(myDlg->myPositionSpin,  SIGNAL (textChanged(const QString&)),SLOT(redisplayPreview()));
  connect(myDlg,                  SIGNAL (selTypeChanged() ),          SLOT(onSelTypeChange()));
  connect(myDlg->mySegment,       SIGNAL (textChanged(const QString&)),SLOT(onTextChange(const QString&)));
}

//================================================================================
/*!
 * \brief SLOT. Called upon change of selection type
 */
//================================================================================

void SMESHGUI_AddNodeOnSegmentOp::onSelTypeChange()
{
  if ( myDlg->mySegmentBtn->isChecked() )
  {
    setSelectionMode( EdgeOfCellSelection );
  }
  else if ( myDlg->myPositionBtn->isChecked() )
  {
    if (SVTK_ViewWindow* svtkViewWindow = SMESH::GetViewWindow(mySMESHGUI)) {
      QString msg;
      SMESH::smIdType node1 = 0, node2 = 0;
      if (isValid(msg, node1, node2)) {
        //Disconnect selectionChanged to keep selected element
        disconnect(selectionMgr(), SIGNAL(selectionChanged()), this, SLOT(onSelectionDone()));
        // Set selection mode to ActorSelection to avoid element's prehighlight during interactive selection
        setSelectionMode(ActorSelection);
        connect(selectionMgr(), SIGNAL(selectionChanged()), SLOT(onSelectionDone()));
      }      
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

void SMESHGUI_AddNodeOnSegmentOp::startOperation()
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

  SMESHGUI_SelectionOp::startOperation(); // this method should be called only after filter creation
  SMESHGUI_InteractiveOp::startOperation();
  myDlg->mySegment->setText("");
  myDlg->myPositionSpin->SetValue(0.5);
  myDlg->myPositionSpin->setReadOnly(false);

  addObserver();

  myDlg->show();

  onSelectionDone(); // init myMeshActor
}

//================================================================================
/*!
 * \brief Stops operation
 */
//================================================================================

void SMESHGUI_AddNodeOnSegmentOp::stopOperation()
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
  SMESH::SetPointRepresentation( false );
  SMESH::RepaintCurrentView();

  disconnect(mySMESHGUI, SIGNAL (SignalActivatedViewManager()), this, SLOT(onOpenView()));
  disconnect(mySMESHGUI, SIGNAL (SignalCloseView()),            this, SLOT(onCloseView()));
  //selectionMgr()->removeFilter( myFilter );
  SMESHGUI_SelectionOp::stopOperation();
  removeObserver();
}

//================================================================================
/*!
 * \brief perform it's intention action: create a node on a segment
 */
//================================================================================

bool SMESHGUI_AddNodeOnSegmentOp::onApply()
{
  if( SMESHGUI::isStudyLocked() )
    return false;

  QString msg;
  SMESH::smIdType node1= 0, node2 = 0;
  if ( !isValid( msg, node1, node2 ))
  {
    SUIT_MessageBox::warning( dlg(), tr( "SMESH_WRN_WARNING" ),
                              msg.isEmpty() ? tr("INVALID_ID") : msg );
    dlg()->show();
    return false;
  }

  QStringList aParameters;
  aParameters << myDlg->myPositionSpin->text();

  try {
    SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO( myMeshActor->getIO() );
    if (aMesh->_is_nil()) {
      SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"), tr("INVALID_MESH") );
      return true;
    }
    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
    if (aMeshEditor->_is_nil())
      return true;

    aMesh->SetParameters( aParameters.join(":").toUtf8().constData() );

    aMeshEditor->AddNodeOnSegment( node1, node2, myDlg->myPositionSpin->GetValue() );

    selector()->ClearIndex();
    selector()->ClearCompositeIndex();
    SALOME_ListIO aList;
    aList.Append( myMeshActor->getIO() );
    selectionMgr()->setSelectedObjects(aList,false);
    onSelectionDone();
    SMESH::UpdateView();
    SMESHGUI::Modified();
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
 * \brief Check selected node id validity
 */
//================================================================================

bool SMESHGUI_AddNodeOnSegmentOp::isValid( QString&          msg,
                                           SMESH::smIdType & node1,
                                           SMESH::smIdType & node2 )
{
  bool ok = false;
  if ( !myMeshActor )
  {
    msg = tr("INVALID_MESH");
  }
  else
  {
    if ( SMDS_Mesh* mesh = myMeshActor->GetObject()->GetMesh() )
    {
      QString txt = myDlg->mySegment->text();
      if ( txt.contains('-'))
      {
        QString str1 = txt.section('-', 0, 0, QString::SectionSkipEmpty);
        QString str2 = txt.section('-', 1, 1, QString::SectionSkipEmpty);
        node1 = str1.toLong();
        node2 = str2.toLong();
        const SMDS_MeshNode* n1 = mesh->FindNode( node1 );
        const SMDS_MeshNode* n2 = mesh->FindNode( node2 );
        std::vector<const SMDS_MeshNode *> nodes = { n1, n2 };
        std::vector<const SMDS_MeshElement *> foundElems;
        if ( !mesh->GetElementsByNodes( nodes, foundElems ))
          msg = tr("NO_ELEMENTS");
        else
        {
          for ( const SMDS_MeshElement * elem : foundElems )
          {
            if ( elem->GetGeomType() == SMDSGeom_TRIANGLE )
              ok = true;
            else
            {
              if ( elem->GetType() == SMDSAbs_Volume )
                msg = tr("VOLUME_FOUND");
              if ( elem->GetType() == SMDSAbs_Face )
                msg = tr("NOT_TRIANGLE_FACE_FOUND");
            }
          }
          if ( !msg.isEmpty() )
            ok = false;
        }
      }
    }
  }
  if ( !ok && msg.isEmpty() )
  {
    node1 = node2 = 0;
    msg += tr("INVALID_EDGE") + "\n";
  }

  if ( ok && ! myDlg->myPositionSpin->isValid( msg, /*toCorrect=*/false ))
  {
    msg = tr("BAD_POSITION");
    ok = false;
  }

  return ok;
}

//================================================================================
/*!
 * \brief SLOT called when selection changed
 */
//================================================================================

void SMESHGUI_AddNodeOnSegmentOp::onSelectionDone()
{
  if ( !myDlg->isVisible() || !myDlg->isEnabled() )
    return;

  myNoPreview = true;
  QString segmentStr;
  try {
    SALOME_ListIO aList;
    selectionMgr()->selectedObjects(aList, SVTK_Viewer::Type());
    if (aList.Extent() != 1)
      return;
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    if (( myMeshActor = SMESH::FindActorByEntry(anIO->getEntry()) ))
    {
      SVTK_IndexedMapOfVtkIds IDs;
      selector()->GetCompositeIndex( anIO, IDs );
      if ( IDs.Extent() == 1 && IDs(1).size() == 2 )
      {
        SMESH::smIdType id1 = IDs(1)[0];
        SMESH::smIdType id2 = IDs(1)[1];
        segmentStr = QString("%1-%2").arg( id1 ).arg( id2 );
      }
    }
  } catch (...) {
  }
  myDlg->mySegment->setText( segmentStr );

  myNoPreview = false;
  redisplayPreview();
}

//================================================================================
/*!
 * \brief update preview
 */
//================================================================================

void SMESHGUI_AddNodeOnSegmentOp::redisplayPreview()
{
  if ( myNoPreview || !myDlg->myPreviewChkBox->isChecked() )
  {
    if ( mySimulation )
      mySimulation->SetVisibility(false);
    return;
  }
  myNoPreview = true;

  SMESH::MeshPreviewStruct_var aMeshPreviewStruct;

  QString msg;
  SMESH::smIdType node1, node2;
  try {
    if ( isValid( msg, node1, node2 ))
    {
      SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(myMeshActor->getIO());
      if ( !aMesh->_is_nil() )
      {
        SMESH::SMESH_MeshEditor_var aPreviewer = aMesh->GetMeshEditPreviewer();
        if ( !aPreviewer->_is_nil() )
        {
          SUIT_OverrideCursor aWaitCursor;
          double pos = myDlg->myPositionSpin->value();
          aPreviewer->AddNodeOnSegment( node1, node2, pos );

          aMeshPreviewStruct = aPreviewer->GetPreviewData();
        }
      }
    }
  }
  catch (...) {
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

void SMESHGUI_AddNodeOnSegmentOp::onOpenView()
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

void SMESHGUI_AddNodeOnSegmentOp::onCloseView()
{
  delete mySimulation;
  mySimulation = 0;
}

//================================================================================
/*!
 * \brief SLOT called when the node ids are manually changed
 */
//================================================================================

void SMESHGUI_AddNodeOnSegmentOp::onTextChange( const QString& /*theText*/ )
{
  QString msg;
  SMESH::smIdType node1= 0, node2 = 0;

  if (( isValid( msg, node1, node2 )) ||
      ( node1 && node2 )) // position only can be invalid
  {
    // highlight entered segment

    Handle(SALOME_InteractiveObject) anIO = myMeshActor->getIO();
    SALOME_ListIO aList;
    aList.Append( anIO );
    selectionMgr()->setSelectedObjects( aList, false );

    SVTK_ListOfVtk newIndices = { node1, node2 };
    selector()->AddOrRemoveCompositeIndex( anIO, newIndices, false );
    SMESH::GetViewWindow(mySMESHGUI)->highlight( anIO, true, true );
  }
}

//================================================================================
/*!
 * \brief Activate Node selection
 */
//================================================================================

void SMESHGUI_AddNodeOnSegmentOp::activateSelection()
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

SMESHGUI_AddNodeOnSegmentOp::~SMESHGUI_AddNodeOnSegmentOp()
{
  if ( myDlg )        delete myDlg;
  if ( mySimulation ) delete mySimulation;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
 * \retval LightApp_Dialog* - pointer to dialog of this operation
 */
//================================================================================

LightApp_Dialog* SMESHGUI_AddNodeOnSegmentOp::dlg() const
{
  return myDlg;
}

//================================================================================
/*
* \brief Process InteractiveSelectionChanged event
*/
//================================================================================
void SMESHGUI_AddNodeOnSegmentOp::processStyleEvents(unsigned long theEvent, void* theCallData) 
{
  (void*)theCallData;
  QString msg;
  SMESH::smIdType node1 = 0, node2 = 0;
  if (isValid(msg, node1, node2)) {
    if (theEvent == SVTK::InteractiveSelectionChanged) {
      if (SMDS_Mesh* mesh = myMeshActor->GetObject()->GetMesh())
        if(myRWInteractor && myRWInteractor->GetDevice() && myInteractorStyle) {
        {
          double N1[3];
          double N2[3];
          double pos;
          double N1_SC[3];
          double N2_SC[3];
          double xyz[3];
          double closest[3];

          const SMDS_MeshNode* n1 = mesh->FindNode(node1);
          const SMDS_MeshNode* n2 = mesh->FindNode(node2);
          int xClick, yClick; // Last event (move or left button down) position
          myRWInteractor->GetDevice()->GetEventPosition(xClick, yClick);

          n1->GetXYZ(N1);
          n2->GetXYZ(N2);
          // Get 2D screen coordinates of each node
          vtkInteractorObserver::ComputeWorldToDisplay(myRWInteractor->GetRenderer()->GetDevice(),
            N1[0], N1[1], N1[2], N1_SC);
          vtkInteractorObserver::ComputeWorldToDisplay(myRWInteractor->GetRenderer()->GetDevice(),
            N2[0], N2[1], N2[2], N2_SC);
          N1_SC[2] = N2_SC[2] = xyz[2] = 0;
          xyz[0] = static_cast<double>(xClick);
          xyz[1] = static_cast<double>(yClick);
          // Parametric position of selected point on a line
          vtkLine::DistanceToLine(xyz, N1_SC, N2_SC, pos, closest);
          if (pos < 0)
            pos = SPIN_TOLERANCE;
          else if (pos > 1.0)
            pos = 1.0 - SPIN_TOLERANCE;
          myDlg->myPositionSpin->SetValue(pos);
          redisplayPreview();
        }
      }
    }
  }
}

//================================================================================
/*
* \brief Process LeftButtonPressEvent event: activate interactive selection
*/
//================================================================================
void SMESHGUI_AddNodeOnSegmentOp::processInteractorEvents(unsigned long theEvent, void* theCallData) 
{
  (void*)theCallData;
  if (theEvent == vtkCommand::LeftButtonPressEvent && myDlg->myPositionBtn->isChecked()) {
    bool control = myRWInteractor->GetDevice()->GetControlKey();
    bool shift = myRWInteractor->GetDevice()->GetControlKey();
    SVTK_ViewWindow* svtkViewWindow = SMESH::GetViewWindow(mySMESHGUI);
    if (svtkViewWindow  && !shift && ! control) {
      QString msg;
      SMESH::smIdType node1 = 0, node2 = 0;
      if (isValid(msg, node1, node2)) {
        svtkViewWindow->activateInteractiveSelection();
      }
    }
  }
}