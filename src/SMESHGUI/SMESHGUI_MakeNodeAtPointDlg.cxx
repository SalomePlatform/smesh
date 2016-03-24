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

// File   : SMESHGUI_MakeNodeAtPointDlg.cxx
// Author : Edward AGAPOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_MakeNodeAtPointDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_MeshEditPreview.h"

#include <SMDS_Mesh.hxx>
#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>
#include <SMESH_NumberFilter.hxx>
#include <SMESH_LogicalFilter.hxx>

// SALOME GEOM includes
#include <GEOMBase.h>

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

// OCCT includes
#include <TColStd_MapOfInteger.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRep_Tool.hxx>
#include <gp_Pnt.hxx>

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

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)

#define SPACING 6
#define MARGIN  11

namespace
{
  enum { MANUAL_MODE = 0, SEARCH_MODE }; // how a node to move is specified
}

/*!
 * \brief Dialog to publish a sub-shape of the mesh main shape
 *        by selecting mesh elements
 */
SMESHGUI_MakeNodeAtPointDlg::SMESHGUI_MakeNodeAtPointDlg()
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
QWidget* SMESHGUI_MakeNodeAtPointDlg::createMainFrame (QWidget* theParent)
{
  QWidget* aFrame = new QWidget(theParent);

  SUIT_ResourceMgr* rm = SMESH::GetResourceMgr( SMESHGUI::GetSMESHGUI() );
  QPixmap iconMoveNode        (rm->loadPixmap("SMESH", tr("ICON_DLG_MOVE_NODE")));
  QPixmap iconMoveWithoutNode (rm->loadPixmap("SMESH", tr("ICON_DLG_MOVE_WITHOUT_NODE")));
  QPixmap iconSelect          (rm->loadPixmap("SMESH", tr("ICON_SELECT")));

  // constructor
  QGroupBox* aPixGrp = new QGroupBox(tr("MOVE_NODE"), this);
  aPixGrp->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  myButtonGroup = new QButtonGroup(this);
  QHBoxLayout* aPixGrpLayout = new QHBoxLayout(aPixGrp);
  aPixGrpLayout->setMargin(MARGIN);
  aPixGrpLayout->setSpacing(SPACING);

  myRButNodeToMove = new QRadioButton(aPixGrp);
  myRButMoveWithoutNode = new QRadioButton(aPixGrp);
  myRButNodeToMove->setIcon(iconMoveNode);
  myRButMoveWithoutNode->setIcon(iconMoveWithoutNode);
  myRButNodeToMove->setChecked(true);

  aPixGrpLayout->addWidget(myRButNodeToMove);
  aPixGrpLayout->addWidget(myRButMoveWithoutNode);
  myButtonGroup->addButton(myRButNodeToMove, 0);
  myButtonGroup->addButton(myRButMoveWithoutNode, 1);

  // Node to move

  myNodeToMoveGrp = new QGroupBox(tr("NODE_2MOVE"), aFrame);
  myNodeToMoveGrp->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  QLabel* idLabel = new QLabel(tr("NODE_2MOVE_ID"), myNodeToMoveGrp);
  myIdBtn = new QPushButton(myNodeToMoveGrp);
  myIdBtn->setIcon(iconSelect);
  myIdBtn->setCheckable(true);
  myId = new QLineEdit(myNodeToMoveGrp);
  myId->setValidator(new SMESHGUI_IdValidator(this, 1));

  myUpdateBtn = new QPushButton(tr("UPDATE_DESTINATION"), myNodeToMoveGrp);
  myUpdateBtn->setAutoDefault(true);

  QWidget* aCoordWidget = new QWidget(myNodeToMoveGrp);

  QLabel* aCurrentXLabel = new QLabel(tr("SMESH_X"), aCoordWidget);
  myCurrentX = new SMESHGUI_SpinBox(aCoordWidget);
  myCurrentX->setButtonSymbols(QAbstractSpinBox::NoButtons);
  myCurrentX->setReadOnly(true);

  QLabel* aCurrentYLabel = new QLabel(tr("SMESH_Y"), aCoordWidget);
  myCurrentY = new SMESHGUI_SpinBox(aCoordWidget);
  myCurrentY->setButtonSymbols(QAbstractSpinBox::NoButtons);
  myCurrentY->setReadOnly(true);

  QLabel* aCurrentZLabel = new QLabel(tr("SMESH_Z"), aCoordWidget);
  myCurrentZ = new SMESHGUI_SpinBox(aCoordWidget);
  myCurrentZ->setButtonSymbols(QAbstractSpinBox::NoButtons);
  myCurrentZ->setReadOnly(true);

  myCurrentX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myCurrentY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myCurrentZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");

  QGridLayout* aCoordLayout = new QGridLayout(aCoordWidget);
  aCoordLayout->setMargin(0);
  aCoordLayout->setSpacing(SPACING);
  aCoordLayout->addWidget(aCurrentXLabel, 0, 0);
  aCoordLayout->addWidget(myCurrentX,     0, 1);
  aCoordLayout->addWidget(aCurrentYLabel, 0, 2);
  aCoordLayout->addWidget(myCurrentY,     0, 3);
  aCoordLayout->addWidget(aCurrentZLabel, 0, 4);
  aCoordLayout->addWidget(myCurrentZ,     0, 5);
  aCoordLayout->setColumnStretch(1, 1);
  aCoordLayout->setColumnStretch(3, 1);
  aCoordLayout->setColumnStretch(5, 1);

  QGridLayout* myNodeToMoveGrpLayout = new QGridLayout(myNodeToMoveGrp);
  myNodeToMoveGrpLayout->setSpacing(SPACING);
  myNodeToMoveGrpLayout->setMargin(MARGIN);

  myNodeToMoveGrpLayout->addWidget( idLabel, 0, 0 );
  myNodeToMoveGrpLayout->addWidget( myIdBtn, 0, 1 );
  myNodeToMoveGrpLayout->addWidget( myId,    0, 2 );
  myNodeToMoveGrpLayout->addWidget( myUpdateBtn, 0, 3 );
  myNodeToMoveGrpLayout->addWidget( aCoordWidget,       1, 0, 1, 4 );

  // Destination

  myDestinationGrp = new QGroupBox(tr("DESTINATION"), aFrame);
  myDestinationGrp->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  myDestBtn = new QPushButton(myDestinationGrp);
  myDestBtn->setIcon(iconSelect);
  myDestBtn->setCheckable(true);

  QLabel* aDestinationXLabel = new QLabel(tr("SMESH_X"), myDestinationGrp);
  myDestinationX = new SMESHGUI_SpinBox(myDestinationGrp);

  QLabel* aDestinationYLabel = new QLabel(tr("SMESH_Y"), myDestinationGrp);
  myDestinationY = new SMESHGUI_SpinBox(myDestinationGrp);

  QLabel* aDestinationZLabel = new QLabel(tr("SMESH_Z"), myDestinationGrp);
  myDestinationZ = new SMESHGUI_SpinBox(myDestinationGrp);

  myDestDXLabel = new QLabel(tr("SMESH_DX"), myDestinationGrp);
  myDestDX = new SMESHGUI_SpinBox(myDestinationGrp);
  myDestDX->setReadOnly(true);

  myDestDYLabel = new QLabel(tr("SMESH_DY"), myDestinationGrp);
  myDestDY = new SMESHGUI_SpinBox(myDestinationGrp);
  myDestDY->setReadOnly(true);

  myDestDZLabel = new QLabel(tr("SMESH_DZ"), myDestinationGrp);
  myDestDZ = new SMESHGUI_SpinBox(myDestinationGrp);
  myDestDZ->setReadOnly(true);

  myDestinationX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myDestinationY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myDestinationZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myDestDX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myDestDY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myDestDZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");

  QGridLayout* aDestLayout = new QGridLayout(myDestinationGrp);
  aDestLayout->setMargin(MARGIN);
  aDestLayout->setSpacing(SPACING);
  aDestLayout->addWidget(myDestBtn, 0, 0);
  aDestLayout->addWidget(aDestinationXLabel, 0, 1);
  aDestLayout->addWidget(myDestinationX,     0, 2);
  aDestLayout->addWidget(aDestinationYLabel, 0, 3);
  aDestLayout->addWidget(myDestinationY,     0, 4);
  aDestLayout->addWidget(aDestinationZLabel, 0, 5);
  aDestLayout->addWidget(myDestinationZ,     0, 6);
  aDestLayout->addWidget(myDestDXLabel,      1, 1);
  aDestLayout->addWidget(myDestDX,           1, 2);
  aDestLayout->addWidget(myDestDYLabel,      1, 3);
  aDestLayout->addWidget(myDestDY,           1, 4);
  aDestLayout->addWidget(myDestDZLabel,      1, 5);
  aDestLayout->addWidget(myDestDZ,           1, 6);
  aDestLayout->setColumnStretch(2, 1);
  aDestLayout->setColumnStretch(4, 1);
  aDestLayout->setColumnStretch(6, 1);

  // Preview

  myPreviewChkBox = new QCheckBox( tr("PREVIEW"), aFrame);

  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->addWidget(aPixGrp);
  aLay->addWidget(myNodeToMoveGrp);
  aLay->addWidget(myDestinationGrp);
  aLay->addWidget(myPreviewChkBox);

  connect(myDestBtn,          SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));
  connect(myIdBtn,            SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));
  connect(myButtonGroup,      SIGNAL (buttonClicked(int)),  SLOT(ConstructorsClicked(int)));

  myIdBtn->setChecked(true);

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
}
//================================================================================
/*!
 * \brief SLOT called when clicked radio button
  * \param int - number of the button
 */
//================================================================================
void SMESHGUI_MakeNodeAtPointDlg::ConstructorsClicked (int constructorId)
{
  switch (constructorId) {
  case 0:
    {
      myDestDXLabel->show();
      myDestDYLabel->show();
      myDestDZLabel->show();
      myDestDX->show();
      myDestDY->show();
      myDestDZ->show();
      myCurrentX->SetValue(0);
      myCurrentY->SetValue(0);
      myCurrentZ->SetValue(0);
      if (!myNodeToMoveGrp->isVisible()) myNodeToMoveGrp->show();
      break;
    }
  case 1:
    {
      myId->setText("");
      myCurrentX->SetValue(0);
      myCurrentY->SetValue(0);
      myCurrentZ->SetValue(0);
      myDestDXLabel->hide();
      myDestDYLabel->hide();
      myDestDZLabel->hide();
      myDestDX->hide();
      myDestDY->hide();
      myDestDZ->hide();
      if (myNodeToMoveGrp->isVisible()) myNodeToMoveGrp->hide();
      myDestBtn->setChecked( true );
      break;
    }
  }
  QApplication::instance()->processEvents();
  myMainFrame->hide();
  myMainFrame->show();
  updateGeometry();
  resize(minimumSizeHint());
}

//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_MakeNodeAtPointOp::SMESHGUI_MakeNodeAtPointOp()
{
  mySimulation = 0;
  mySMESHGUI = 0;
  myDlg = new SMESHGUI_MakeNodeAtPointDlg;
  myFilter = 0;
  myHelpFileName = "mesh_through_point_page.html";

  myNoPreview = false;
  myUpdateDestination = false;
  myDestCoordChanged = true;

  // connect signals and slots
  connect(myDlg->myDestinationX,  SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  connect(myDlg->myDestinationY,  SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  connect(myDlg->myDestinationZ,  SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  connect(myDlg->myDestDX,        SIGNAL (valueChanged(double)), this, SLOT(onDestCoordChanged()));
  connect(myDlg->myDestDY,        SIGNAL (valueChanged(double)), this, SLOT(onDestCoordChanged()));
  connect(myDlg->myDestDZ,        SIGNAL (valueChanged(double)), this, SLOT(onDestCoordChanged()));
  connect(myDlg->myId,            SIGNAL (textChanged(const QString&)),SLOT(redisplayPreview()));
  connect(myDlg->myPreviewChkBox, SIGNAL (toggled(bool)),              SLOT(redisplayPreview()));
  connect(myDlg->myButtonGroup,   SIGNAL (buttonClicked(int)),         SLOT(redisplayPreview()));

  // IPAL22913: TC6.5.0: selected in "Move node" dialog box node is not highlighted
  // note: this slot seems to be lost together with removed obsolete SMESHGUI_MoveNodesDlg class
  connect(myDlg->myId,SIGNAL (textChanged(const QString&)),SLOT(onTextChange(const QString&)));
  connect(myDlg->myUpdateBtn, SIGNAL (clicked()), this, SLOT(onUpdateDestination()));
}

void SMESHGUI_MakeNodeAtPointOp::onUpdateDestination()
{
  myUpdateDestination = true;
  redisplayPreview();
  myUpdateDestination = false;
}

void SMESHGUI_MakeNodeAtPointOp::onDestCoordChanged()
{
  myDestCoordChanged = false;
  redisplayPreview();
  myDestCoordChanged = true;
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

  // SalomeApp_TypeFilter depends on a current study
  if ( myFilter ) delete myFilter;
  QList<SUIT_SelectionFilter*> filters;
  filters.append( new SalomeApp_TypeFilter((SalomeApp_Study*)study(), "SMESH" ));
  TColStd_MapOfInteger vertexType;
  vertexType.Add( TopAbs_VERTEX );
  filters.append( new SMESH_NumberFilter("GEOM", TopAbs_VERTEX, 1, vertexType ));
  myFilter = new SMESH_LogicalFilter( filters, SMESH_LogicalFilter::LO_OR );

  // IPAL19360
  SMESHGUI_SelectionOp::startOperation(); // this method should be called only after filter creation
  //activateSelection(); // set filters   // called inside of previous statement
  myDlg->myId->setText("");
  myDlg->myDestinationX->SetValue(0);
  myDlg->myDestinationY->SetValue(0);
  myDlg->myDestinationZ->SetValue(0);
  myDlg->myDestDX->SetValue(0);
  myDlg->myDestDY->SetValue(0);
  myDlg->myDestDZ->SetValue(0);
  myDlg->myCurrentX->SetValue(0);
  myDlg->myCurrentY->SetValue(0);
  myDlg->myCurrentZ->SetValue(0);
  myDlg->myDestDX->setReadOnly(true);
  myDlg->myDestDY->setReadOnly(true);
  myDlg->myDestDZ->setReadOnly(true);
  myDlg->myRButNodeToMove->setChecked(true);

  myDlg->ConstructorsClicked( GetConstructorId() );

  myDlg->show();

  onSelectionDone(); // init myMeshActor
}

//=================================================================================
// function : GetConstructorId()
// purpose  :
//=================================================================================
int SMESHGUI_MakeNodeAtPointOp::GetConstructorId()
{
  return myDlg->myButtonGroup->checkedId();
}

//================================================================================
/*!
 * \brief Stops operation
 */
//================================================================================

void SMESHGUI_MakeNodeAtPointOp::stopOperation()
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
  selectionMgr()->removeFilter( myFilter );
  SMESHGUI_SelectionOp::stopOperation();
}

//================================================================================
/*!
 * \brief perform it's intention action: move or create a node
 */
//================================================================================

bool SMESHGUI_MakeNodeAtPointOp::onApply()
{
  if( isStudyLocked() )
    return false;

  if ( !myMeshActor ) {
    SUIT_MessageBox::warning( dlg(), tr( "SMESH_WRN_WARNING" ),
                              tr("INVALID_MESH") );
    dlg()->show();
    return false;
  }

  QString msg;
  if ( !isValid( msg ) ) { // node id is invalid
    if( !msg.isEmpty() )
      SUIT_MessageBox::warning( dlg(), tr( "SMESH_WRN_WARNING" ),
                                tr("INVALID_ID") );
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
      SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_ERROR"),
                                   tr("SMESHG_NO_MESH") );
      return true;
    }
    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
    if (aMeshEditor->_is_nil())
      return true;

    aMesh->SetParameters( aParameters.join(":").toLatin1().constData() );

    bool ok;
    int anId = myDlg->myId->text().toInt( &ok );
    if( !ok || anId < 1 )
      anId = aMeshEditor->FindNodeClosestTo(myDlg->myDestinationX->GetValue(),
                                            myDlg->myDestinationY->GetValue(),
                                            myDlg->myDestinationZ->GetValue());

    int aResult = aMeshEditor->MoveNode(anId,
                                        myDlg->myDestinationX->GetValue(),
                                        myDlg->myDestinationY->GetValue(),
                                        myDlg->myDestinationZ->GetValue() );

    if (aResult)
    {
      myDlg->myCurrentX->SetValue(0);
      myDlg->myCurrentY->SetValue(0);
      myDlg->myCurrentZ->SetValue(0);
      myDlg->myDestDX->SetValue(0);
      myDlg->myDestDY->SetValue(0);
      myDlg->myDestDZ->SetValue(0);

      SALOME_ListIO aList;
      selectionMgr()->setSelectedObjects(aList,false);
      aList.Append(myMeshActor->getIO());
      selectionMgr()->setSelectedObjects(aList,false);
      SMESH::UpdateView();
      SMESHGUI::Modified();
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
 * \brief Check selected node id validity
 */
//================================================================================

bool SMESHGUI_MakeNodeAtPointOp::isValid( QString& msg )
{
  bool ok = true;
  if ( myMeshActor && myDlg->myRButNodeToMove->isChecked() )
  {
    ok = false;
    int id = myDlg->myId->text().toInt();
    if ( id > 0 )
      if (SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh())
        ok = aMesh->FindNode( id );
    if( !ok )
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

void SMESHGUI_MakeNodeAtPointOp::onSelectionDone()
{
  if ( !myDlg->isVisible() || !myDlg->isEnabled() )
    return;

  myNoPreview = true;
  try {
    SALOME_ListIO aList;
    selectionMgr()->selectedObjects(aList, SVTK_Viewer::Type());
    if (aList.Extent() != 1)
      return;
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    SMESH_Actor* aMeshActor = SMESH::FindActorByEntry(anIO->getEntry());

    if (( myDlg->myIdBtn->isChecked() && myDlg->myIdBtn->isEnabled() ) ||
        ( !myDlg->myNodeToMoveGrp->isVisible() ))
      myMeshActor = aMeshActor;

    if (!aMeshActor) { // coord by geom
      if ( myDlg->myDestBtn->isChecked() ) {
        GEOM::GEOM_Object_var geom = SMESH::IObjectToInterface<GEOM::GEOM_Object>(anIO);
        if ( !geom->_is_nil() ) {
          TopoDS_Vertex aShape;
          if ( GEOMBase::GetShape(geom, aShape) &&
               aShape.ShapeType() == TopAbs_VERTEX ) {
            gp_Pnt P = BRep_Tool::Pnt(aShape);
            myDlg->myDestinationX->SetValue(P.X());
            myDlg->myDestinationY->SetValue(P.Y());
            myDlg->myDestinationZ->SetValue(P.Z());
          }
        }
        myNoPreview = false;
        redisplayPreview();
        return;
      }
    }

    QString aString;
    int nbElems = SMESH::GetNameOfSelectedElements(selector(),anIO, aString);
    if (nbElems == 1) {
      if (SMDS_Mesh* aMesh = aMeshActor->GetObject()->GetMesh()) {
        if (const SMDS_MeshNode* aNode = aMesh->FindNode(aString.toInt())) {
          if ( myDlg->myDestBtn->isChecked() ) { // set coord
            myDlg->myDestinationX->SetValue(aNode->X());
            myDlg->myDestinationY->SetValue(aNode->Y());
            myDlg->myDestinationZ->SetValue(aNode->Z());
          }
          else if ( myDlg->myIdBtn->isChecked() &&
                    myDlg->myIdBtn->isEnabled() ) { // set node to move
            myDlg->myId->setText(aString);
            myDlg->myCurrentX->SetValue( aNode->X() );
            myDlg->myCurrentY->SetValue( aNode->Y() );
            myDlg->myCurrentZ->SetValue( aNode->Z() );
            redisplayPreview();
          }

          double x = myDlg->myCurrentX->GetValue();
          double y = myDlg->myCurrentY->GetValue();
          double z = myDlg->myCurrentZ->GetValue();
          double dx = myDlg->myDestinationX->GetValue() - x;
          double dy = myDlg->myDestinationY->GetValue() - y;
          double dz = myDlg->myDestinationZ->GetValue() - z;
          myDlg->myDestDX->SetValue(dx);
          myDlg->myDestDY->SetValue(dy);
          myDlg->myDestDZ->SetValue(dz);
          myDlg->myDestDX->setReadOnly(false);
          myDlg->myDestDY->setReadOnly(false);
          myDlg->myDestDZ->setReadOnly(false);
        }
      }
    }
  } catch (...) {
  }

  myNoPreview = false;
  redisplayPreview();
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

  if ( !myMeshActor && GetConstructorId() == SEARCH_MODE )
    onSelectionDone();

  SMESH::MeshPreviewStruct_var aMeshPreviewStruct;

  bool moveShown = false;
  if ( myMeshActor)
  {
    const bool  isPreview = myDlg->myPreviewChkBox->isChecked();
    const bool isMoveNode = myDlg->myRButMoveWithoutNode->isChecked();
    QString msg;
    if ( isValid( msg ) )
    {
      try {
        SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(myMeshActor->getIO());
        if (!aMesh->_is_nil()) {
          SMESH::SMESH_MeshEditor_var aPreviewer = aMesh->GetMeshEditPreviewer();
          if (!aPreviewer->_is_nil())
          {
            SUIT_OverrideCursor aWaitCursor;

            int anId = myDlg->myId->text().toInt();

            SMESH::double_array_var aXYZ = aMesh->GetNodeXYZ( anId );
            if( &aXYZ.in() && aXYZ->length() >= 3 )
            {
              double x = aXYZ[0];
              double y = aXYZ[1];
              double z = aXYZ[2];
              double dx = 0;
              double dy = 0;
              double dz = 0;

              if ( myUpdateDestination ) {
                myDlg->myDestinationX->SetValue(x);
                myDlg->myDestinationY->SetValue(y);
                myDlg->myDestinationZ->SetValue(z);
                myDlg->myDestDX->SetValue(dx);
                myDlg->myDestDY->SetValue(dy);
                myDlg->myDestDZ->SetValue(dz);
              }
              else
              {
                if ( myDestCoordChanged ) {
                  dx = myDlg->myDestinationX->GetValue() - x;
                  dy = myDlg->myDestinationY->GetValue() - y;
                  dz = myDlg->myDestinationZ->GetValue() - z;
                  myDlg->myDestDX->SetValue(dx);
                  myDlg->myDestDY->SetValue(dy);
                  myDlg->myDestDZ->SetValue(dz);
                }
                else {
                  dx = myDlg->myDestDX->GetValue() + x;
                  dy = myDlg->myDestDY->GetValue() + y;
                  dz = myDlg->myDestDZ->GetValue() + z;
                  myDlg->myDestinationX->SetValue(dx);
                  myDlg->myDestinationY->SetValue(dy);
                  myDlg->myDestinationZ->SetValue(dz);
                }
              }
              myDlg->myCurrentX->SetValue(x);
              myDlg->myCurrentY->SetValue(y);
              myDlg->myCurrentZ->SetValue(z);
              myDlg->myDestDX->setReadOnly(false);
              myDlg->myDestDY->setReadOnly(false);
              myDlg->myDestDZ->setReadOnly(false);
            }
            else {
              myDlg->myCurrentX->SetValue(0);
              myDlg->myCurrentY->SetValue(0);
              myDlg->myCurrentZ->SetValue(0);
              myDlg->myDestDX->SetValue(0);
              myDlg->myDestDY->SetValue(0);
              myDlg->myDestDZ->SetValue(0);
              myDlg->myDestDX->setReadOnly(true);
              myDlg->myDestDY->setReadOnly(true);
              myDlg->myDestDZ->setReadOnly(true);
            }
            if ( isPreview && isMoveNode && anId == 0 )
              anId = aPreviewer->FindNodeClosestTo(myDlg->myDestinationX->GetValue(),
                                                   myDlg->myDestinationY->GetValue(),
                                                   myDlg->myDestinationZ->GetValue());
            // find id and/or just compute preview
            aPreviewer->MoveNode(anId,
                                 myDlg->myDestinationX->GetValue(),
                                 myDlg->myDestinationY->GetValue(),
                                 myDlg->myDestinationZ->GetValue());

            if ( isPreview ) { // fill preview data
              aMeshPreviewStruct = aPreviewer->GetPreviewData();
              moveShown = ( anId > 0 );
            }
          }
        }
      }
      catch (...) {
      }
    }
  }

  if ( !moveShown )
  {
    aMeshPreviewStruct = new SMESH::MeshPreviewStruct();

    aMeshPreviewStruct->nodesXYZ.length(1);
    aMeshPreviewStruct->nodesXYZ[0].x = myDlg->myDestinationX->GetValue();
    aMeshPreviewStruct->nodesXYZ[0].y = myDlg->myDestinationY->GetValue();
    aMeshPreviewStruct->nodesXYZ[0].z = myDlg->myDestinationZ->GetValue();

    aMeshPreviewStruct->elementTypes.length(1);
    aMeshPreviewStruct->elementTypes[0].SMDS_ElementType = SMESH::NODE;
    aMeshPreviewStruct->elementTypes[0].isPoly = false;
    aMeshPreviewStruct->elementTypes[0].nbNodesInElement = 1;

    aMeshPreviewStruct->elementConnectivities.length(1);
    aMeshPreviewStruct->elementConnectivities[0] = 0;
  }
  if (!mySimulation)
    mySimulation = new SMESHGUI_MeshEditPreview(SMESH::GetViewWindow( mySMESHGUI ));
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

//=================================================================================
/*!
 * \brief SLOT called when the viewer opened
 */
//=================================================================================
void SMESHGUI_MakeNodeAtPointOp::onOpenView()
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
void SMESHGUI_MakeNodeAtPointOp::onCloseView()
{
  delete mySimulation;
  mySimulation = 0;
}

//================================================================================
/*!
 * \brief SLOT called when the node id is manually changed
 */
//================================================================================

void SMESHGUI_MakeNodeAtPointOp::onTextChange( const QString& theText )
{
  if( myMeshActor )
  {
    if( SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh() )
    {
      Handle(SALOME_InteractiveObject) anIO = myMeshActor->getIO();
      SALOME_ListIO aList;
      aList.Append( anIO );
      selectionMgr()->setSelectedObjects( aList, false );

      if( const SMDS_MeshNode* aNode = aMesh->FindNode( theText.toInt() ) )
      {
        TColStd_MapOfInteger aListInd;
        aListInd.Add( aNode->GetID() );
        selector()->AddOrRemoveIndex( anIO, aListInd, false );
        if( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( SMESHGUI::GetSMESHGUI() ) )
          aViewWindow->highlight( anIO, true, true );
      }
    }
  }
}

//================================================================================
/*!
 * \brief Activate Node selection
 */
//================================================================================

void SMESHGUI_MakeNodeAtPointOp::activateSelection()
{
  selectionMgr()->clearFilters();
  SMESH::SetPointRepresentation( true );
  selectionMgr()->installFilter( myFilter );
  setSelectionMode( NodeSelection );
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================

SMESHGUI_MakeNodeAtPointOp::~SMESHGUI_MakeNodeAtPointOp()
{
  if ( myDlg )        delete myDlg;
  if ( mySimulation ) delete mySimulation;
  if ( myFilter )     delete myFilter;
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

