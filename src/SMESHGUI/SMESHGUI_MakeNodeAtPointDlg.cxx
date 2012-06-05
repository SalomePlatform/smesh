// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

  QWidget* aMainFrame = createMainFrame  (mainFrame());

  aDlgLay->addWidget(aMainFrame);

  aDlgLay->setStretchFactor(aMainFrame, 1);
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's input fields
//=======================================================================
QWidget* SMESHGUI_MakeNodeAtPointDlg::createMainFrame (QWidget* theParent)
{
  QWidget* aFrame = new QWidget(theParent);

  SUIT_ResourceMgr* rm = SMESH::GetResourceMgr( SMESHGUI::GetSMESHGUI() );
  QPixmap iconMoveNode (rm->loadPixmap("SMESH", tr("ICON_DLG_MOVE_NODE")));
  QPixmap iconSelect   (rm->loadPixmap("SMESH", tr("ICON_SELECT")));

  // constructor

  QGroupBox* aPixGrp = new QGroupBox(tr("MOVE_NODE"), aFrame);
  QButtonGroup* aBtnGrp = new QButtonGroup(this);
  QHBoxLayout* aPixGrpLayout = new QHBoxLayout(aPixGrp);
  aPixGrpLayout->setMargin(MARGIN);
  aPixGrpLayout->setSpacing(SPACING);

  QRadioButton* aRBut = new QRadioButton(aPixGrp);
  aRBut->setIcon(iconMoveNode);
  aRBut->setChecked(true);
  aPixGrpLayout->addWidget(aRBut);
  aBtnGrp->addButton(aRBut, 0);

  // coordinates

  QGroupBox* aCoordGrp = new QGroupBox(tr("DESTINATION"), aFrame);
  QHBoxLayout* aCoordGrpLayout = new QHBoxLayout(aCoordGrp);
  aCoordGrpLayout->setMargin(MARGIN);
  aCoordGrpLayout->setSpacing(SPACING);

  myCoordBtn = new QPushButton(aCoordGrp);
  myCoordBtn->setIcon(iconSelect);
  myCoordBtn->setCheckable(true);

  QLabel* aXLabel = new QLabel(tr("SMESH_X"), aCoordGrp);
  myX = new SMESHGUI_SpinBox(aCoordGrp);

  QLabel* aYLabel = new QLabel(tr("SMESH_Y"), aCoordGrp);
  myY = new SMESHGUI_SpinBox(aCoordGrp);

  QLabel* aZLabel = new QLabel(tr("SMESH_Z"), aCoordGrp);
  myZ = new SMESHGUI_SpinBox(aCoordGrp);

  myX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");

  aCoordGrpLayout->addWidget(myCoordBtn);
  aCoordGrpLayout->addWidget(aXLabel);
  aCoordGrpLayout->addWidget(myX);
  aCoordGrpLayout->addWidget(aYLabel);
  aCoordGrpLayout->addWidget(myY);
  aCoordGrpLayout->addWidget(aZLabel);
  aCoordGrpLayout->addWidget(myZ);
  aCoordGrpLayout->setStretchFactor(myX, 1);
  aCoordGrpLayout->setStretchFactor(myY, 1);
  aCoordGrpLayout->setStretchFactor(myZ, 1);

  // node ID

  myNodeToMoveGrp = new QGroupBox(tr("NODE_2MOVE"), aFrame);

  QLabel* idLabel = new QLabel(tr("NODE_2MOVE_ID"), myNodeToMoveGrp);
  myIdBtn = new QPushButton(myNodeToMoveGrp);
  myIdBtn->setIcon(iconSelect);
  myIdBtn->setCheckable(true);
  myId = new QLineEdit(myNodeToMoveGrp);
  myId->setValidator(new SMESHGUI_IdValidator(this, 1));

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

  QLabel* aDXLabel = new QLabel(tr("SMESH_DX"), aCoordWidget);
  myDX = new SMESHGUI_SpinBox(aCoordWidget);
  myDX->setButtonSymbols(QAbstractSpinBox::NoButtons);
  myDX->setReadOnly(true);

  QLabel* aDYLabel = new QLabel(tr("SMESH_DY"), aCoordWidget);
  myDY = new SMESHGUI_SpinBox(aCoordWidget);
  myDY->setButtonSymbols(QAbstractSpinBox::NoButtons);
  myDY->setReadOnly(true);

  QLabel* aDZLabel = new QLabel(tr("SMESH_DZ"), aCoordWidget);
  myDZ = new SMESHGUI_SpinBox(aCoordWidget);
  myDZ->setButtonSymbols(QAbstractSpinBox::NoButtons);
  myDZ->setReadOnly(true);

  myCurrentX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myCurrentY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myCurrentZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myDX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myDY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myDZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");

  QGridLayout* aCoordLayout = new QGridLayout(aCoordWidget);
  aCoordLayout->setMargin(0);
  aCoordLayout->setSpacing(SPACING);
  aCoordLayout->addWidget(aCurrentXLabel, 0, 0);
  aCoordLayout->addWidget(myCurrentX,     0, 1);
  aCoordLayout->addWidget(aCurrentYLabel, 0, 2);
  aCoordLayout->addWidget(myCurrentY,     0, 3);
  aCoordLayout->addWidget(aCurrentZLabel, 0, 4);
  aCoordLayout->addWidget(myCurrentZ,     0, 5);
  aCoordLayout->addWidget(aDXLabel,       1, 0);
  aCoordLayout->addWidget(myDX,           1, 1);
  aCoordLayout->addWidget(aDYLabel,       1, 2);
  aCoordLayout->addWidget(myDY,           1, 3);
  aCoordLayout->addWidget(aDZLabel,       1, 4);
  aCoordLayout->addWidget(myDZ,           1, 5);
  aCoordLayout->setColumnStretch(1, 1);
  aCoordLayout->setColumnStretch(3, 1);
  aCoordLayout->setColumnStretch(5, 1);

  myAutoSearchChkBox = new QCheckBox( tr("AUTO_SEARCH"), myNodeToMoveGrp);
  myPreviewChkBox = new QCheckBox( tr("PREVIEW"), myNodeToMoveGrp);

  QGridLayout* myNodeToMoveGrpLayout = new QGridLayout(myNodeToMoveGrp);
  myNodeToMoveGrpLayout->setSpacing(SPACING);
  myNodeToMoveGrpLayout->setMargin(MARGIN);

  myNodeToMoveGrpLayout->addWidget( idLabel, 0, 0 );
  myNodeToMoveGrpLayout->addWidget( myIdBtn, 0, 1 );
  myNodeToMoveGrpLayout->addWidget( myId,    0, 2 );
  myNodeToMoveGrpLayout->addWidget( aCoordWidget,       1, 0, 1, 3 );
  myNodeToMoveGrpLayout->addWidget( myAutoSearchChkBox, 2, 0, 1, 3 );
  myNodeToMoveGrpLayout->addWidget( myPreviewChkBox,    3, 0, 1, 3 );

  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->addWidget(aPixGrp);
  aLay->addWidget(aCoordGrp);
  aLay->addWidget(myNodeToMoveGrp);

  connect(myCoordBtn,         SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));
  connect(myIdBtn,            SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));
  connect(myAutoSearchChkBox, SIGNAL (toggled(bool)), this, SLOT(ButtonToggled(bool)));

  myIdBtn->setChecked(true);
  myAutoSearchChkBox->setChecked(true);

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
        myIdBtn->setChecked( !on );
    }
    else if ( aSender == myIdBtn ) // button to select a node to move
    {
      myCoordBtn->setChecked( !on );
    }
  }      
  if ( aSender == myAutoSearchChkBox ) // automatic node search
  {
    if ( on ) {
      myCurrentX->SetValue(0);
      myCurrentY->SetValue(0);
      myCurrentZ->SetValue(0);
      myDX->SetValue(0);
      myDY->SetValue(0);
      myDZ->SetValue(0);
      myId->setText("");
      myId->setReadOnly ( true );
      myIdBtn->setChecked( false );
      myIdBtn->setEnabled( false );
      myCoordBtn->setChecked( true );
    }
    else {
      myId->setReadOnly ( false );
      myIdBtn->setEnabled( true );
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
  myFilter = 0;
  myHelpFileName = "mesh_through_point_page.html";

  // connect signals and slots
  connect(myDlg->myX, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  connect(myDlg->myY, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  connect(myDlg->myZ, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  connect(myDlg->myId,SIGNAL (textChanged(const QString&)),SLOT(redisplayPreview()));
  connect(myDlg->myPreviewChkBox,   SIGNAL (toggled(bool)),SLOT(redisplayPreview()));
  connect(myDlg->myAutoSearchChkBox,SIGNAL (toggled(bool)),SLOT(redisplayPreview()));

  // IPAL22913: TC6.5.0: selected in "Move node" dialog box node is not highlighted
  // note: this slot seems to be lost together with removed obsolete SMESHGUI_MoveNodesDlg class
  connect(myDlg->myId,SIGNAL (textChanged(const QString&)),SLOT(onTextChange(const QString&)));
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

  myDlg->myX->SetValue(0);
  myDlg->myY->SetValue(0);
  myDlg->myZ->SetValue(0);
  myDlg->myCurrentX->SetValue(0);
  myDlg->myCurrentY->SetValue(0);
  myDlg->myCurrentZ->SetValue(0);
  myDlg->myDX->SetValue(0);
  myDlg->myDY->SetValue(0);
  myDlg->myDZ->SetValue(0);
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
    myMeshActor = 0;
  }
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
  aParameters << myDlg->myX->text();
  aParameters << myDlg->myY->text();
  aParameters << myDlg->myZ->text();

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
      anId = aMeshEditor->FindNodeClosestTo(myDlg->myX->GetValue(),
                                            myDlg->myY->GetValue(),
                                            myDlg->myZ->GetValue());

    int aResult = aMeshEditor->MoveNode(anId,
                                        myDlg->myX->GetValue(),
                                        myDlg->myY->GetValue(),
                                        myDlg->myZ->GetValue() );

    if (aResult)
    {
      myDlg->myCurrentX->SetValue(0);
      myDlg->myCurrentY->SetValue(0);
      myDlg->myCurrentZ->SetValue(0);
      myDlg->myDX->SetValue(0);
      myDlg->myDY->SetValue(0);
      myDlg->myDZ->SetValue(0);
      myDlg->myId->setText("");

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
  if ( myMeshActor &&
       !myDlg->myAutoSearchChkBox->isChecked() )
  {
    ok = false;
    int id = myDlg->myId->text().toInt();
    if ( id > 0 )
      if (SMDS_Mesh* aMesh = myMeshActor->GetObject()->GetMesh())
        ok = aMesh->FindNode( id );
    if( !ok )
      msg += tr("INVALID_ID") + "\n";
  }

  ok = myDlg->myX->isValid( msg, !myNoPreview ) && ok;
  ok = myDlg->myY->isValid( msg, !myNoPreview ) && ok;
  ok = myDlg->myZ->isValid( msg, !myNoPreview ) && ok;

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
  try {
    SALOME_ListIO aList;
    selectionMgr()->selectedObjects(aList, SVTK_Viewer::Type());
    if (aList.Extent() != 1)
      return;
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    SMESH_Actor* aMeshActor = SMESH::FindActorByEntry(anIO->getEntry());

    if (!aMeshActor) { // coord by geom
      if ( myDlg->myCoordBtn->isChecked() ) {
        GEOM::GEOM_Object_var geom = SMESH::IObjectToInterface<GEOM::GEOM_Object>(anIO);
        if ( !geom->_is_nil() ) {
          TopoDS_Vertex aShape;
          if ( GEOMBase::GetShape(geom, aShape) &&
               aShape.ShapeType() == TopAbs_VERTEX ) {
            gp_Pnt P = BRep_Tool::Pnt(aShape);
            myNoPreview = true;
            myDlg->myX->SetValue(P.X());
            myDlg->myY->SetValue(P.Y());
            myDlg->myZ->SetValue(P.Z());
            myNoPreview = false;
            redisplayPreview();
          }
        }
        return;
      }
    }

    if ( !myMeshActor )
      myMeshActor = aMeshActor;

    QString aString;
    int nbElems = SMESH::GetNameOfSelectedElements(selector(),anIO, aString);
    if (nbElems == 1) {
      if (SMDS_Mesh* aMesh = aMeshActor->GetObject()->GetMesh()) {
        if (const SMDS_MeshNode* aNode = aMesh->FindNode(aString.toInt())) {
          myNoPreview = true;
          if ( myDlg->myCoordBtn->isChecked() ) { // set coord
            myDlg->myX->SetValue(aNode->X());
            myDlg->myY->SetValue(aNode->Y());
            myDlg->myZ->SetValue(aNode->Z());
            myNoPreview = false;
            redisplayPreview();
          }
          else if ( myDlg->myIdBtn->isChecked() &&
                    myDlg->myIdBtn->isEnabled() ) { // set node to move
            myDlg->myId->setText(aString);
            myNoPreview = false;
            redisplayPreview();
          }

          if (const SMDS_MeshNode* aCurrentNode = aMesh->FindNode(myDlg->myId->text().toInt())) {
            double x = aCurrentNode->X();
            double y = aCurrentNode->Y();
            double z = aCurrentNode->Z();
            double dx = myDlg->myX->GetValue() - x;
            double dy = myDlg->myY->GetValue() - y;
            double dz = myDlg->myZ->GetValue() - z;
            myDlg->myCurrentX->SetValue(x);
            myDlg->myCurrentY->SetValue(y);
            myDlg->myCurrentZ->SetValue(z);
            myDlg->myDX->SetValue(dx);
            myDlg->myDY->SetValue(dy);
            myDlg->myDZ->SetValue(dz);
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
  if ( myMeshActor)
  {
    const bool autoSearch = myDlg->myAutoSearchChkBox->isChecked();
    const bool preview    = myDlg->myPreviewChkBox->isChecked();
    if ( autoSearch )
    {
      myDlg->myCurrentX->SetValue(0);
      myDlg->myCurrentY->SetValue(0);
      myDlg->myCurrentZ->SetValue(0);
      myDlg->myDX->SetValue(0);
      myDlg->myDY->SetValue(0);
      myDlg->myDZ->SetValue(0);
      myDlg->myId->setText("");
    }
    QString msg;
    if ( autoSearch || isValid( msg ) )
    {
      try {
        SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(myMeshActor->getIO());
        if (!aMesh->_is_nil()) {
          SMESH::SMESH_MeshEditor_var aPreviewer = aMesh->GetMeshEditPreviewer();
          if (!aPreviewer->_is_nil())
          {
            SUIT_OverrideCursor aWaitCursor;

            int anId = 0;
            if ( autoSearch )
              anId = aPreviewer->FindNodeClosestTo(myDlg->myX->GetValue(),
                                                   myDlg->myY->GetValue(),
                                                   myDlg->myZ->GetValue());
            else
              anId = myDlg->myId->text().toInt();

            // find id and/or just compute preview
            aPreviewer->MoveNode(anId,
                                 myDlg->myX->GetValue(),
                                 myDlg->myY->GetValue(),
                                 myDlg->myZ->GetValue());
            if ( autoSearch ) { // set found id
              QString idTxt("%1");
              if ( anId > 0 )
                idTxt = idTxt.arg( anId );
              else
                idTxt = "";
              myDlg->myId->setText( idTxt );
            }

            SMESH::double_array* aXYZ = aMesh->GetNodeXYZ( anId );
            if( aXYZ && aXYZ->length() >= 3 )
            {
              double x = aXYZ->operator[](0);
              double y = aXYZ->operator[](1);
              double z = aXYZ->operator[](2);
              double dx = myDlg->myX->GetValue() - x;
              double dy = myDlg->myY->GetValue() - y;
              double dz = myDlg->myZ->GetValue() - z;
              myDlg->myCurrentX->SetValue(x);
              myDlg->myCurrentY->SetValue(y);
              myDlg->myCurrentZ->SetValue(z);
              myDlg->myDX->SetValue(dx);
              myDlg->myDY->SetValue(dy);
              myDlg->myDZ->SetValue(dz);
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
  SMESH::SetPointRepresentation(false);
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

