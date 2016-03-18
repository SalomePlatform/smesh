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

// File   : SMESHGUI_ReorientFacesDlg.cxx
// Author : Edward AGAPOV, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_ReorientFacesDlg.h"

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
#include <SMESH_TypeFilter.hxx>

// SALOME GEOM includes
#include <GEOMBase.h>

// SALOME GUI includes
#include <LightApp_SelectionMgr.h>
#include <SALOME_ListIO.hxx>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SVTK_ViewModel.h>
#include <SVTK_ViewWindow.h>
#include <SalomeApp_Tools.h>
#include <SalomeApp_TypeFilter.h>

// SALOME KERNEL includes
#include <SALOMEDS_SObject.hxx>

// OCCT includes
#include <BRep_Tool.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TColgp_SequenceOfXYZ.hxx>
#include <TopoDS_Vertex.hxx>
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

// std
#include <limits>

#define SPACING 6
#define MARGIN  11

enum { CONSTRUCTOR_POINT=0, CONSTRUCTOR_FACE, CONSTRUCTOR_VOLUME,
       EObject, EPoint, EFace, EDirection, EVolumes };

//=======================================================================
/*!
 * \brief Dialog to reorient faces acoording to vector
 */
//=======================================================================

SMESHGUI_ReorientFacesDlg::SMESHGUI_ReorientFacesDlg()
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

//================================================================================
/*!
 * \brief Create frame containing dialog's input fields
 */
//================================================================================

QWidget* SMESHGUI_ReorientFacesDlg::createMainFrame (QWidget* theParent)
{
  QWidget* aFrame = new QWidget(theParent);

  // constructors

  QPixmap iconReoriPoint (resMgr()->loadPixmap("SMESH", tr("ICON_DLG_REORIENT2D_POINT")));
  QPixmap iconReoriFace  (resMgr()->loadPixmap("SMESH", tr("ICON_DLG_REORIENT2D_FACE")));
  QPixmap iconReoriVolum (resMgr()->loadPixmap("SMESH", tr("ICON_DLG_REORIENT2D_VOLUME")));

  QGroupBox* aConstructorBox = new QGroupBox(tr("REORIENT_FACES"), aFrame);
  myConstructorGrp = new QButtonGroup(aConstructorBox);
  QHBoxLayout* aConstructorGrpLayout = new QHBoxLayout(aConstructorBox);
  aConstructorGrpLayout->setMargin(MARGIN);
  aConstructorGrpLayout->setSpacing(SPACING);

  QRadioButton* aPntBut = new QRadioButton(aConstructorBox);
  aPntBut->setIcon(iconReoriPoint);
  aPntBut->setChecked(true);
  aConstructorGrpLayout->addWidget(aPntBut);
  myConstructorGrp->addButton(aPntBut, CONSTRUCTOR_POINT);

  QRadioButton* aFaceBut= new QRadioButton(aConstructorBox);
  aFaceBut->setIcon(iconReoriFace);
  aConstructorGrpLayout->addWidget(aFaceBut);
  myConstructorGrp->addButton(aFaceBut, CONSTRUCTOR_FACE);

  QRadioButton* aVolBut= new QRadioButton(aConstructorBox);
  aVolBut->setIcon(iconReoriVolum);
  aConstructorGrpLayout->addWidget(aVolBut);
  myConstructorGrp->addButton(aVolBut, CONSTRUCTOR_VOLUME);

  // Create other controls

  setObjectPixmap( "SMESH", tr( "ICON_SELECT" ) );

  createObject( tr("OBJECT")   , aFrame, EObject );
  createObject( tr("POINT")    , aFrame, EPoint );
  createObject( tr("FACE")     , aFrame, EFace );
  createObject( tr("DIRECTION"), aFrame, EDirection );
  createObject( tr("VOLUMES"),   aFrame, EVolumes );
  setNameIndication( EObject, OneName );
  setNameIndication( EFace, OneName );
  setReadOnly( EFace, false );
  if ( QLineEdit* le = qobject_cast<QLineEdit*>( objectWg( EFace, Control ) ))
    le->setValidator( new SMESHGUI_IdValidator( this,1 ));

  int width = aFaceBut->fontMetrics().width( tr("DIRECTION"));
  objectWg( EDirection, Label )->setFixedWidth( width );
  objectWg( EObject   , Label )->setFixedWidth( width );
  objectWg( EPoint    , Label )->setFixedWidth( width );
  objectWg( EFace     , Label )->setFixedWidth( width );
  objectWg( EVolumes  , Label )->setFixedWidth( width );

  myOutsideChk = new QCheckBox( tr("OUTSIDE_VOLUME_NORMAL"), aFrame);
  myOutsideChk->setChecked( true );

  QLabel* aXLabel = new QLabel(tr("SMESH_X"), aFrame);
  myX = new SMESHGUI_SpinBox(aFrame);
  QLabel* aYLabel = new QLabel(tr("SMESH_Y"), aFrame);
  myY = new SMESHGUI_SpinBox(aFrame);
  QLabel* aZLabel = new QLabel(tr("SMESH_Z"), aFrame);
  myZ = new SMESHGUI_SpinBox(aFrame);

  myX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myX->SetValue(0);
  myY->SetValue(0);
  myZ->SetValue(0);

  QLabel* aDXLabel = new QLabel(tr("SMESH_DX"), aFrame);
  myDX = new SMESHGUI_SpinBox(aFrame);
  QLabel* aDYLabel = new QLabel(tr("SMESH_DY"), aFrame);
  myDY = new SMESHGUI_SpinBox(aFrame);
  QLabel* aDZLabel = new QLabel(tr("SMESH_DZ"), aFrame);
  myDZ = new SMESHGUI_SpinBox(aFrame);
  myDX->SetValue(1);
  myDY->SetValue(0);
  myDZ->SetValue(0);

  myDX->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myDY->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");
  myDZ->RangeStepAndValidator(COORD_MIN, COORD_MAX, 10.0, "length_precision");

  width = Max( aFaceBut->fontMetrics().width( tr("SMESH_X")),
               aFaceBut->fontMetrics().width( tr("SMESH_DX")));
  aXLabel->setFixedWidth( width );
  aYLabel->setFixedWidth( width );
  aZLabel->setFixedWidth( width );
  aDXLabel->setFixedWidth( width );
  aDYLabel->setFixedWidth( width );
  aDZLabel->setFixedWidth( width );

  // Layouting

  QGroupBox* anObjectGrp = new QGroupBox(tr("FACES"), aFrame);
  QHBoxLayout* anObjectGrpLayout = new QHBoxLayout(anObjectGrp);
  anObjectGrpLayout->setMargin(MARGIN);
  anObjectGrpLayout->setSpacing(SPACING);
  anObjectGrpLayout->addWidget( objectWg( EObject, Label ));
  anObjectGrpLayout->addWidget( objectWg( EObject, Btn ));
  anObjectGrpLayout->addWidget( objectWg( EObject, Control ));

  myPointFrm = new QFrame(aFrame);
  QHBoxLayout* aPointGrpLayout = new QHBoxLayout(myPointFrm);
  aPointGrpLayout->setMargin(0);
  objectWg( EPoint, Control )->hide();
  aPointGrpLayout->addWidget( objectWg( EPoint, Label ) );
  aPointGrpLayout->addWidget( objectWg( EPoint, Btn ) );
  aPointGrpLayout->addWidget( aXLabel, 0 );
  aPointGrpLayout->addWidget( myX,     1 );
  aPointGrpLayout->addWidget( aYLabel, 0 );
  aPointGrpLayout->addWidget( myY,     1 );
  aPointGrpLayout->addWidget( aZLabel, 0 );
  aPointGrpLayout->addWidget( myZ,     1 );

  myFaceFrm = new QFrame(aFrame);
  QHBoxLayout* aFaceGrpLayout = new QHBoxLayout(myFaceFrm);
  aFaceGrpLayout->setMargin(0);
  aFaceGrpLayout->addWidget( objectWg( EFace, Label ) );
  aFaceGrpLayout->addWidget( objectWg( EFace, Btn ) );
  aFaceGrpLayout->addWidget( objectWg( EFace, Control ) );

  myVolumFrm = new QFrame(aFrame);
  QGridLayout* aVolumGrpLayout = new QGridLayout(myVolumFrm);
  aVolumGrpLayout->setMargin(0);
  aVolumGrpLayout->setSpacing(SPACING);
  aVolumGrpLayout->addWidget( objectWg( EVolumes, Label ),   0, 0 );
  aVolumGrpLayout->addWidget( objectWg( EVolumes, Btn ),     0, 1 );
  aVolumGrpLayout->addWidget( objectWg( EVolumes, Control ), 0, 2 );
  aVolumGrpLayout->addWidget( myOutsideChk,                  1, 0, 1, 3 );

  myDirFrm = new QFrame(aFrame);
  QHBoxLayout* aDirectGrpLayout = new QHBoxLayout(myDirFrm);
  aDirectGrpLayout->setMargin(0);
  objectWg( EDirection, Control )->hide();
  aDirectGrpLayout->addWidget( objectWg( EDirection, Label ) );
  aDirectGrpLayout->addWidget( objectWg( EDirection, Btn ) );
  aDirectGrpLayout->addWidget( aDXLabel, 0 );
  aDirectGrpLayout->addWidget( myDX,     1 );
  aDirectGrpLayout->addWidget( aDYLabel, 0 );
  aDirectGrpLayout->addWidget( myDY,     1 );
  aDirectGrpLayout->addWidget( aDZLabel, 0 );
  aDirectGrpLayout->addWidget( myDZ,     1 );
  

  QGroupBox* anOrientGrp = new QGroupBox(tr("ORIENTATION"), aFrame);
  QVBoxLayout* anOrientGrpLayout = new QVBoxLayout ( anOrientGrp );
  anOrientGrpLayout->addWidget(myPointFrm);
  anOrientGrpLayout->addWidget(myFaceFrm);
  anOrientGrpLayout->addWidget(myVolumFrm);
  anOrientGrpLayout->addWidget(myDirFrm);
  

  QVBoxLayout* aLay = new QVBoxLayout(aFrame);
  aLay->addWidget(aConstructorBox);
  aLay->addWidget(anObjectGrp);
  aLay->addWidget(anOrientGrp);

  connect( myConstructorGrp, SIGNAL(buttonClicked (int)), this, SLOT(constructorChange(int)));

  return aFrame;
}

//================================================================================
/*!
 * \brief Show point or face
 */
//================================================================================

void SMESHGUI_ReorientFacesDlg::constructorChange(int id)
{
  if ( id == CONSTRUCTOR_FACE )
  {
    myPointFrm->hide();
    myVolumFrm->hide();
    myFaceFrm->show();
    myDirFrm->show();
    activateObject( EFace );
  }
  else if ( id == CONSTRUCTOR_POINT )
  {
    myFaceFrm->hide();
    myVolumFrm->hide();
    myPointFrm->show();
    myDirFrm->show();
    activateObject( EPoint );
  }
  else // CONSTRUCTOR_VOLUME
  {
    myFaceFrm->hide();
    myPointFrm->hide();
    myDirFrm->hide();
    myVolumFrm->show();
    activateObject( EVolumes );
  }
}

//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_ReorientFacesOp::SMESHGUI_ReorientFacesOp()
  :SMESHGUI_SelectionOp( ActorSelection )
{
  //myVectorPreview = 0;
  myHelpFileName = "reorient_faces_page.html";

  myDlg = new SMESHGUI_ReorientFacesDlg;
  myDlg->constructorChange( CONSTRUCTOR_POINT );

  // connect signals and slots
  connect( myDlg->objectWg( EFace, LightApp_Dialog::Control ), SIGNAL(textChanged(const QString&)),
           this, SLOT(onTextChange(const QString&)));
  // connect(myDlg->myX, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  // connect(myDlg->myY, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  // connect(myDlg->myZ, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  // connect(myDlg->myDX, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  // connect(myDlg->myDY, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));
  // connect(myDlg->myDZ, SIGNAL (valueChanged(double)), this, SLOT(redisplayPreview()));

}

//=======================================================================
// function : startOperation()
// purpose  : Init dialog fields, connect signals and slots, show dialog
//=======================================================================

void SMESHGUI_ReorientFacesOp::startOperation()
{
  myObjectActor = 0;

  // init simulation with a current View
  //if ( myVectorPreview ) delete myVectorPreview;
  // myVectorPreview = new SMESHGUI_MeshEditPreview(SMESH::GetViewWindow( getSMESHGUI() ));
  // vtkProperty* aProp = vtkProperty::New();
  // aProp->SetRepresentationToWireframe();
  // aProp->SetColor(250, 0, 250);
  // aProp->SetPointSize(5);
  // aProp->SetLineWidth( SMESH::GetFloat("SMESH:element_width",1) + 1);
  // myVectorPreview->GetActor()->SetProperty(aProp);
  // aProp->Delete();

  SMESHGUI_SelectionOp::startOperation();

  myDlg->show();

  mySelectionMode = 0;
  myDlg->activateObject( EObject );
}

//================================================================================
/*!
 * \brief Stops operation
 */
//================================================================================

void SMESHGUI_ReorientFacesOp::stopOperation()
{
  //myVectorPreview->SetVisibility(false);
  if ( myObjectActor ) {
    myObjectActor->SetPointRepresentation(false);
    SMESH::RepaintCurrentView();
    myObjectActor = 0;
  }
  SMESHGUI_SelectionOp::stopOperation();
  myDlg->deactivateAll();
}

//================================================================================
/*!
 * \brief Set selection mode corresponding to a pressed selection button
 */
//================================================================================

void SMESHGUI_ReorientFacesOp::onActivateObject( int what )
{
  if ( what == mySelectionMode )
    return;
  mySelectionMode = what;
  switch ( mySelectionMode )
  {
  case EPoint:
  case EDirection:
    SMESH::SetPointRepresentation(true);
    setSelectionMode( NodeSelection );
    SMESH::SetPickable();
    break;
  case EObject:
  case EVolumes:
    SMESH::SetPointRepresentation(false);
    setSelectionMode( ActorSelection );
    break;
  case EFace:
    SMESH::SetPointRepresentation(false);
    setSelectionMode( FaceSelection );
    if ( myObjectActor )
      SMESH::SetPickable( myObjectActor );
    else
      SMESH::SetPickable();
    break;
  }
  SMESHGUI_SelectionOp::onActivateObject( what );
}

//================================================================================
/*!
 * \brief Creates a filter corresponding to a pressed selection button
 */
//================================================================================

SUIT_SelectionFilter* SMESHGUI_ReorientFacesOp::createFilter( const int what ) const
{
  switch ( what )
  {
  case EObject:
    {
      QList<SUIT_SelectionFilter*> filters;
      filters.append( new SMESH_TypeFilter( SMESH::MESH ));
      filters.append( new SMESH_TypeFilter( SMESH::SUBMESH_FACE ));
      filters.append( new SMESH_TypeFilter( SMESH::GROUP_FACE ));
      return new SMESH_LogicalFilter( filters, SMESH_LogicalFilter::LO_OR );
    }
  case EVolumes:
    {
      QList<SUIT_SelectionFilter*> filters;
      filters.append( new SMESH_TypeFilter( SMESH::MESH ));
      filters.append( new SMESH_TypeFilter( SMESH::SUBMESH_SOLID ));
      filters.append( new SMESH_TypeFilter( SMESH::GROUP_VOLUME ));
      return new SMESH_LogicalFilter( filters, SMESH_LogicalFilter::LO_OR );
    }
  case EPoint:
    {
      QList<SUIT_SelectionFilter*> filters;
      filters.append( new SMESH_TypeFilter( SMESH::IDSOURCE ));
      filters.append( new SMESH_NumberFilter( "GEOM",TopAbs_VERTEX, 1, TopAbs_VERTEX ));
      return new SMESH_LogicalFilter( filters, SMESH_LogicalFilter::LO_OR );
    }
  case EFace:
  case EDirection:
    {
      return new SMESH_TypeFilter( SMESH::IDSOURCE );
    }
  }
  return NULL;
}

//================================================================================
/*!
 * \brief get data from selection
 */
//================================================================================

void SMESHGUI_ReorientFacesOp::selectionDone()
{
  if ( !myDlg->isVisible() || !myDlg->isEnabled() )
    return;

  if ( mySelectionMode == EVolumes )
  {
    SMESHGUI_SelectionOp::selectionDone();
    return;
  }

  myDlg->clearSelection( mySelectionMode );

  SALOME_ListIO aList;
  selectionMgr()->selectedObjects(aList);
  const int nbSelected = aList.Extent();
  if ( nbSelected == 0 )
    return;

  Handle(SALOME_InteractiveObject) anIO = aList.First();

  try
  {
    switch ( mySelectionMode )
    {
    case EObject: { // get an actor of object

      if ( nbSelected == 1 )
      {
        myDlg->selectObject( EObject, anIO->getName(), 0, anIO->getEntry(), true );
        // typeById( aList.First()->getEntry(),
        //           SMESHGUI_SelectionOp::Object ),
        myObjectActor = SMESH::FindActorByEntry( anIO->getEntry() );
      }
      break;
    }
    case EFace: {  // get a face ID

      if ( nbSelected == 1 )
      {
        TColStd_IndexedMapOfInteger faceIndices;
        selector()->GetIndex( anIO, faceIndices );
        if ( faceIndices.Extent() == 1 )
        {
          SMESH_Actor* savedActor = myObjectActor;
          myObjectActor = 0; // to prevent work of onTextChange()
          myDlg->setObjectText( EFace, QString("%1").arg( faceIndices(1) ));
          myObjectActor = savedActor;

          if ( !myObjectActor )
          {
            myDlg->selectObject( EObject, anIO->getName(), 0, anIO->getEntry(), true );
            // typeById( aList.First()->getEntry(),
            //           SMESHGUI_SelectionOp::Object ),
            myObjectActor = SMESH::FindActorByEntry( anIO->getEntry() );
          }
        }
      }
      break;
    }
    case EPoint:
    case EDirection: {  // set XYZ by selected nodes or vertices

      if ( mySelectionMode == EPoint && aList.Extent() > 1 )
        return;

      TColgp_SequenceOfXYZ points;
      for( SALOME_ListIteratorOfListIO anIt( aList ); anIt.More(); anIt.Next() )
      {
        anIO = anIt.Value();
        GEOM::GEOM_Object_var geom = SMESH::IObjectToInterface<GEOM::GEOM_Object>(anIO);
        if ( !geom->_is_nil() ) {
          TopoDS_Vertex aShape;
          if ( GEOMBase::GetShape(geom, aShape) && aShape.ShapeType() == TopAbs_VERTEX ) {
            gp_Pnt P = BRep_Tool::Pnt(aShape);
            points.Append( P.XYZ() );
          }
        }
        else
        {
          TColStd_IndexedMapOfInteger nodeIndices;
          selector()->GetIndex( anIO, nodeIndices );
          if ( nodeIndices.Extent() > 0 && nodeIndices.Extent() <=2 )
          {
            if ( SMESH_Actor* aMeshActor = SMESH::FindActorByEntry(anIO->getEntry()))
              if (SMDS_Mesh* aMesh = aMeshActor->GetObject()->GetMesh())
              {
                if (const SMDS_MeshNode* aNode = aMesh->FindNode( nodeIndices(1)))
                  points.Append( gp_XYZ( aNode->X(), aNode->Y(), aNode->Z()));
                if ( nodeIndices.Extent() == 2 )
                  if (const SMDS_MeshNode* aNode = aMesh->FindNode( nodeIndices(2)))
                    points.Append( gp_XYZ( aNode->X(), aNode->Y(), aNode->Z()));
              }
          }
        }
      }
      gp_XYZ xyz;
      if ( points.Length() == 1 )
        xyz = points(1);
      else if ( points.Length() == 2 )
        xyz = points(2) - points(1);
      else
        return;
      if ( points.Length() == 1 && mySelectionMode == EPoint )
      {
        myDlg->myX->SetValue( xyz.X() );
        myDlg->myY->SetValue( xyz.Y() );
        myDlg->myZ->SetValue( xyz.Z() );
        redisplayPreview();
      }
      if ( mySelectionMode == EDirection )
      {
        myDlg->myDX->SetValue( xyz.X() );
        myDlg->myDY->SetValue( xyz.Y() );
        myDlg->myDZ->SetValue( xyz.Z() );
        redisplayPreview();
      }
      break;
    } // case EPoint || EDirection
    } // switch
  }
  catch (...)
  {
  }
}

//================================================================================
/*!
 * \brief SLOT called when the face id is changed
 */
//================================================================================

void SMESHGUI_ReorientFacesOp::onTextChange( const QString& theText )
{
  if( myObjectActor )
  {
    sender()->blockSignals( true );
    if ( mySelectionMode != EFace )
    {
      myDlg->activateObject( EFace );
      myDlg->setObjectText( EFace, theText );
    }
    TColStd_MapOfInteger ids;
    if ( !theText.isEmpty() && theText.toInt() > 0 )
      ids.Add( theText.toInt() );

    SMESHGUI_SelectionOp::addOrRemoveIndex( myObjectActor->getIO(), ids, false );
    SMESHGUI_SelectionOp::highlight( myObjectActor->getIO(), true, true );
    sender()->blockSignals( false );
  }
}

//================================================================================
/*!
 * \brief perform it's intention action: reorient faces of myObject
 */
//================================================================================

bool SMESHGUI_ReorientFacesOp::onApply()
{
  if( isStudyLocked() )
    return false;

  QString msg;
  if ( !isValid( msg ) ) { // node id is invalid
    if( !msg.isEmpty() )
      SUIT_MessageBox::warning( dlg(), tr( "SMESH_WRN_WARNING" ), msg );
    dlg()->show();
    return false;
  }

  QStringList aParameters;
  aParameters << myDlg->myDX->text();
  aParameters << myDlg->myDY->text();
  aParameters << myDlg->myDZ->text();
  aParameters << myDlg->myX->text();
  aParameters << myDlg->myY->text();
  aParameters << myDlg->myZ->text();

  try {
    SUIT_OverrideCursor wc;

    SMESH::SMESH_Mesh_var aMesh = myObject->GetMesh();
    if ( aMesh->_is_nil() ) return false;

    SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
    if (aMeshEditor->_is_nil()) return false;

    int aResult = 0;
    if ( myDlg->myConstructorGrp->checkedId() == CONSTRUCTOR_VOLUME )
    {
      SMESH::ListOfIDSources_var faceGroups = new SMESH::ListOfIDSources;
      faceGroups->length(1);
      faceGroups[0] = myObject;

      bool outsideNormal = myDlg->myOutsideChk->isChecked();

      aResult = aMeshEditor->Reorient2DBy3D( faceGroups, myVolumeObj, outsideNormal );
    }
    else
    {
      SMESH::DirStruct direction;
      direction.PS.x = myDlg->myDX->GetValue();
      direction.PS.y = myDlg->myDY->GetValue();
      direction.PS.z = myDlg->myDZ->GetValue();

      long face = myDlg->objectText( EFace ).toInt();
      if ( myDlg->myConstructorGrp->checkedId() == CONSTRUCTOR_POINT )
        face = -1;

      SMESH::PointStruct point;
      point.x = myDlg->myX->GetValue();
      point.y = myDlg->myY->GetValue();
      point.z = myDlg->myZ->GetValue();

      aMesh->SetParameters( aParameters.join(":").toLatin1().constData() );

      aResult = aMeshEditor->Reorient2D( myObject, direction, face, point );
    }

    if (aResult)
    {
      SALOME_ListIO aList;
      selectionMgr()->setSelectedObjects(aList,false);
      SMESH::UpdateView();
      SMESHGUI::Modified();
    }
    wc.suspend();
    SUIT_MessageBox::information(SMESHGUI::desktop(), tr("SMESH_INFORMATION"),
                                 tr("NB_REORIENTED").arg(aResult));
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

bool SMESHGUI_ReorientFacesOp::isValid( QString& msg )
{
  // check object
  QString objectEntry = myDlg->selectedObject( EObject );
  _PTR(SObject) pSObject = studyDS()->FindObjectID( objectEntry.toLatin1().data() );
  myObject = SMESH::SMESH_IDSource::_narrow( _CAST( SObject,pSObject )->GetObject() );
  if ( myObject->_is_nil() )
  {
    msg = tr("NO_OBJECT_SELECTED");
    return false;
  }
  bool hasFaces = false;
  SMESH::array_of_ElementType_var types = myObject->GetTypes();
  for ( size_t i = 0; i < types->length() && !hasFaces; ++i )
    hasFaces = ( types[i] == SMESH::FACE );
  if ( !hasFaces )
  {
    msg = tr("NO_FACES");
    return false;
  }

  // check volume object
  if ( myDlg->myConstructorGrp->checkedId() == CONSTRUCTOR_VOLUME )
  {
    objectEntry = myDlg->selectedObject( EVolumes );
    _PTR(SObject) pSObject = studyDS()->FindObjectID( objectEntry.toLatin1().data() );
    myVolumeObj = SMESH::SObjectToInterface< SMESH::SMESH_IDSource>( pSObject );
    if ( myVolumeObj->_is_nil() )
    {
      msg = tr("NO_VOLUME_OBJECT_SELECTED");
      return false;
    }
    bool hasVolumes = false;
    types = myVolumeObj->GetTypes();
    for ( size_t i = 0; i < types->length() && !hasVolumes; ++i )
      hasVolumes = ( types[i] == SMESH::VOLUME );
    if ( !hasVolumes )
    {
      msg = tr("NO_VOLUMES");
      return false;
    }
  }
  // check vector
  gp_Vec vec( myDlg->myDX->GetValue(),
              myDlg->myDY->GetValue(),
              myDlg->myDZ->GetValue() );
  if ( vec.Magnitude() < std::numeric_limits<double>::min() )
  {
    msg = tr("ZERO_SIZE_VECTOR");
    return false;
  }

  // check face ID
  if ( myDlg->myConstructorGrp->checkedId() == CONSTRUCTOR_FACE )
  {
    int faceID = myDlg->objectText( EFace ).toInt();
    bool faceOK = ( faceID > 0 );
    if ( faceOK )
    {
      if ( myObjectActor )
      {
        faceOK = ( myObjectActor->GetObject()->GetElemDimension( faceID ) == 2 );
      }
      else
      {
        SMESH::SMESH_Mesh_var aMesh = myObject->GetMesh();
        if ( !aMesh->_is_nil() ) 
          faceOK = ( aMesh->GetElementType( faceID, true ) == SMESH::FACE );
      }
    }
    if ( !faceOK )
    {
      msg = tr("INVALID_FACE");
      return false;
    }
  }

  return true;
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================

SMESHGUI_ReorientFacesOp::~SMESHGUI_ReorientFacesOp()
{
  if ( myDlg )        delete myDlg;
  //if ( myVectorPreview ) delete myVectorPreview;
}

//================================================================================
/*!
 * \brief Gets dialog of this operation
 * \retval LightApp_Dialog* - pointer to dialog of this operation
 */
//================================================================================

LightApp_Dialog* SMESHGUI_ReorientFacesOp::dlg() const
{
  return myDlg;
}

//================================================================================
/*!
 * \brief update preview
 */
//================================================================================

void SMESHGUI_ReorientFacesOp::redisplayPreview()
{
//   SMESH::MeshPreviewStruct_var aMeshPreviewStruct;

//   bool moveShown = false;
//   if ( myObjectActor)
//   {
//     const bool autoSearch = myDlg->myAutoSearchChkBox->isChecked();
//     const bool preview    = myDlg->myPreviewChkBox->isChecked();
//     if ( autoSearch )
//     {
//       myDlg->myCurrentX->SetValue(0);
//       myDlg->myCurrentY->SetValue(0);
//       myDlg->myCurrentZ->SetValue(0);
//       myDlg->myDX->SetValue(0);
//       myDlg->myDY->SetValue(0);
//       myDlg->myDZ->SetValue(0);
//       myDlg->myId->setText("");
//     }
//     QString msg;
//     if ( autoSearch || isValid( msg ) )
//     {
//       try {
//         SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(myObjectActor->getIO());
//         if (!aMesh->_is_nil()) {
//           SMESH::SMESH_MeshEditor_var aPreviewer = aMesh->GetMeshEditPreviewer();
//           if (!aPreviewer->_is_nil())
//           {
//             SUIT_OverrideCursor aWaitCursor;

//             int anId = 0;
//             if ( autoSearch )
//               anId = aPreviewer->FindNodeClosestTo(myDlg->myX->GetValue(),
//                                                    myDlg->myY->GetValue(),
//                                                    myDlg->myZ->GetValue());
//             else
//               anId = myDlg->myId->text().toInt();

//             // find id and/or just compute preview
//             aPreviewer->MoveNode(anId,
//                                  myDlg->myX->GetValue(),
//                                  myDlg->myY->GetValue(),
//                                  myDlg->myZ->GetValue());
//             if ( autoSearch ) { // set found id
//               QString idTxt("%1");
//               if ( anId > 0 )
//                 idTxt = idTxt.arg( anId );
//               else
//                 idTxt = "";
//               myDlg->myId->setText( idTxt );
//             }

//             SMESH::double_array* aXYZ = aMesh->GetNodeXYZ( anId );
//             if( aXYZ && aXYZ->length() >= 3 )
//             {
//               double x = aXYZ->operator[](0);
//               double y = aXYZ->operator[](1);
//               double z = aXYZ->operator[](2);
//               double dx = myDlg->myX->GetValue() - x;
//               double dy = myDlg->myY->GetValue() - y;
//               double dz = myDlg->myZ->GetValue() - z;
//               myDlg->myCurrentX->SetValue(x);
//               myDlg->myCurrentY->SetValue(y);
//               myDlg->myCurrentZ->SetValue(z);
//               myDlg->myDX->SetValue(dx);
//               myDlg->myDY->SetValue(dy);
//               myDlg->myDZ->SetValue(dz);
//             }

//             if ( preview ) { // fill preview data
//               aMeshPreviewStruct = aPreviewer->GetPreviewData();
//               moveShown = ( anId > 0 );
//             }
//           }
//         }
//       }catch (...) {
//       }
//     }
//   }

//   if ( !moveShown )
//   {
//     aMeshPreviewStruct = new SMESH::MeshPreviewStruct();

//     aMeshPreviewStruct->nodesXYZ.length(1);
//     aMeshPreviewStruct->nodesXYZ[0].x = myDlg->myX->GetValue();
//     aMeshPreviewStruct->nodesXYZ[0].y = myDlg->myY->GetValue();
//     aMeshPreviewStruct->nodesXYZ[0].z = myDlg->myZ->GetValue();

//     aMeshPreviewStruct->elementTypes.length(1);
//     aMeshPreviewStruct->elementTypes[0].SMDS_ElementType = SMESH::NODE;
//     aMeshPreviewStruct->elementTypes[0].isPoly = false;
//     aMeshPreviewStruct->elementTypes[0].nbNodesInElement = 1;

//     aMeshPreviewStruct->elementConnectivities.length(1);
//     aMeshPreviewStruct->elementConnectivities[0] = 0;
//   }

//   // display data
//   if ( aMeshPreviewStruct.operator->() )
//   {
//     myVectorPreview->SetData(aMeshPreviewStruct._retn());
//   }
//   else
//   {
//     myVectorPreview->SetVisibility(false);
//   }

}
