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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org
//
//
//
//  File   : SMESHGUI_ShapeByMeshDlg.cxx
//  Author : Edward AGAPOV
//  Module : SMESH

#include "SMESHGUI_ShapeByMeshDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_GEOMGenUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include "SMDS_Mesh.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMESH_Actor.h"

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

#define SPACING 5
#define MARGIN  10

enum { EDGE = 0, FACE, VOLUME };

/*!
 * \brief Dialog to publish a sub-shape of the mesh main shape
 *        by selecting mesh elements
 */
SMESHGUI_ShapeByMeshDlg::SMESHGUI_ShapeByMeshDlg( SMESHGUI*   theModule,
                                                  const char* theName)
     : QDialog( SMESH::GetDesktop( theModule ), theName, false,
                WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu),
     mySMESHGUI( theModule ),
     mySelectionMgr( SMESH::GetSelectionMgr( theModule ) )
{
  setCaption(tr("CAPTION"));

  QVBoxLayout* aDlgLay = new QVBoxLayout (this, MARGIN, SPACING);

  QFrame* aMainFrame = createMainFrame  (this);
  QFrame* aBtnFrame  = createButtonFrame(this);

  aDlgLay->addWidget(aMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  aDlgLay->setStretchFactor(aMainFrame, 1);

  myViewWindow = SMESH::GetViewWindow( mySMESHGUI );

  Init();
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_ShapeByMeshDlg::createMainFrame (QWidget* theParent)
{
  QFrame* aMainGrp = new QFrame(theParent, "main frame");
  QGridLayout* aLayout = new QGridLayout(aMainGrp, 3, 2);
  aLayout->setSpacing(6);
  aLayout->setAutoAdd(false);

  // elem type
  myElemTypeGroup = new QButtonGroup(1, Qt::Vertical, aMainGrp, "Group types");
  myElemTypeGroup->setTitle(tr("SMESH_ELEMENT_TYPE"));
  myElemTypeGroup->setExclusive(true);

  (new QRadioButton( tr("SMESH_EDGE")  , myElemTypeGroup))->setChecked(true);
  new QRadioButton( tr("SMESH_FACE")  , myElemTypeGroup);
  new QRadioButton( tr("SMESH_VOLUME"), myElemTypeGroup);

  // element id
  QLabel* anIdLabel = new QLabel( aMainGrp, "element id label");
  anIdLabel->setText( tr("ELEMENT_ID") );
  myElementId = new QLineEdit( aMainGrp, "element id");
  myElementId->setValidator( new SMESHGUI_IdValidator( theParent, "id validator", 1 ));

  // shape name
  QLabel* aNameLabel = new QLabel( aMainGrp, "geom name label");
  aNameLabel->setText( tr("GEOMETRY_NAME") );
  myGeomName = new QLineEdit( aMainGrp, "geom name");

  aLayout->addMultiCellWidget(myElemTypeGroup, 0, 0, 0, 1);
  aLayout->addWidget(anIdLabel,   1, 0);
  aLayout->addWidget(myElementId, 1, 1);
  aLayout->addWidget(aNameLabel,  2, 0);
  aLayout->addWidget(myGeomName,  2, 1);

  connect(myElemTypeGroup, SIGNAL(clicked(int)), SLOT(onTypeChanged(int)));
  connect(myElementId, SIGNAL(textChanged(const QString&)), SLOT(onElemIdChanged(const QString&)));

  return aMainGrp;
}

//=======================================================================
// function : createButtonFrame()
// purpose  : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_ShapeByMeshDlg::createButtonFrame (QWidget* theParent)
{
  QFrame* aFrame = new QFrame(theParent);
  aFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);

  myOkBtn    = new QPushButton(tr("SMESH_BUT_OK"   ), aFrame);
  myCloseBtn = new QPushButton(tr("SMESH_BUT_CLOSE"), aFrame);

  QSpacerItem* aSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

  QHBoxLayout* aLay = new QHBoxLayout(aFrame, MARGIN, SPACING);

  aLay->addWidget(myOkBtn);
  aLay->addItem(aSpacer);
  aLay->addWidget(myCloseBtn);

  connect(myOkBtn,    SIGNAL(clicked()), SLOT(onOk()));
  connect(myCloseBtn, SIGNAL(clicked()), SLOT(onClose()));

  return aFrame;
}

//=======================================================================
// function : ~SMESHGUI_ShapeByMeshDlg()
// purpose  : Destructor
//=======================================================================
SMESHGUI_ShapeByMeshDlg::~SMESHGUI_ShapeByMeshDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=======================================================================
// function : Init()
// purpose  : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_ShapeByMeshDlg::Init()
{
  SetMesh( SMESH::SMESH_Mesh::_nil() );
  myIsManualIdEnter = false;

  //erasePreview();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // selection and SMESHGUI
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(onClose()));

  setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
  qApp->processEvents();
  updateGeometry();
  adjustSize();
  resize(minimumSize());

  activateSelection();
  onSelectionDone();
  
  this->show();
}

//=======================================================================
// function : SetMesh()
// purpose  : Set mesh to dialog
//=======================================================================

void SMESHGUI_ShapeByMeshDlg::SetMesh (SMESH::SMESH_Mesh_ptr thePtr)
{
  myMesh    = SMESH::SMESH_Mesh::_duplicate(thePtr);
  myGeomObj = GEOM::GEOM_Object::_nil();
  myHasSolids = false;

  vector< bool > hasElement (myElemTypeGroup->count(), false);
  if (!myMesh->_is_nil() && myViewWindow )
  {
    _PTR(SObject) aSobj = SMESH::FindSObject(myMesh.in());
    SUIT_DataOwnerPtr anIObj (new LightApp_DataOwner(aSobj->GetID().c_str()));

    vector< int > nbShapes( TopAbs_SHAPE, 0 );
    int shapeDim = 0; // max dim with several shapes
    if ( mySelectionMgr->isOk(anIObj) ) // check that the mesh has a valid shape
    {
      _PTR(SObject) aSO = SMESH::FindSObject(myMesh.in());
      GEOM::GEOM_Object_var mainShape = SMESH::GetGeom(aSO);
      if ( !mainShape->_is_nil() ) 
      {
        if ( GeometryGUI::GetGeomGen()->_is_nil() )// check that GEOM_Gen exists
          GeometryGUI::InitGeomGen();
        TopoDS_Shape aShape;
        if ( GEOMBase::GetShape(mainShape, aShape))
        {
          TopAbs_ShapeEnum types[4] = { TopAbs_EDGE, TopAbs_FACE, TopAbs_SHELL, TopAbs_SOLID };
          for ( int dim = 4; dim > 0; --dim ) {
            TopAbs_ShapeEnum type = types[ dim - 1 ];
            TopAbs_ShapeEnum avoid = ( type == TopAbs_SHELL ) ? TopAbs_SOLID : TopAbs_SHAPE;
            TopExp_Explorer exp( aShape, type, avoid );
            for ( ; nbShapes[ type ] < 2 && exp.More(); exp.Next() )
              ++nbShapes[ type ];
            if ( nbShapes[ type ] > 1 ) {
              shapeDim = dim;
              break;
            }
          }
        }
      }
    }
    if (shapeDim > 0)
    {
      if ( nbShapes[ TopAbs_SHELL ] + nbShapes[ TopAbs_SOLID ] > 1 )
        shapeDim = 3;
      hasElement[ EDGE ]   = shapeDim > 0 && myMesh->NbEdges()  ;
      hasElement[ FACE ]   = shapeDim > 1 && myMesh->NbFaces()  ;
      hasElement[ VOLUME ] = shapeDim > 2 && myMesh->NbVolumes();

      if ( hasElement[ EDGE ] && myViewWindow->GetSelector() )
      {
        connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
      }
    }
    myHasSolids = nbShapes[ TopAbs_SOLID ];
  }

  // disable inexistant elem types
  for ( int i = 0; i < myElemTypeGroup->count(); ++i ) {
    if ( QButton* button = myElemTypeGroup->find( i ) )
      button->setEnabled( hasElement[ i ] );
  }
  myElementId->setEnabled( hasElement[ EDGE ] );
  myGeomName-> setEnabled( hasElement[ EDGE ] );

  setElementID("");
}

//=======================================================================
// function : GetShape()
// purpose  : Get published sub-shape
//=======================================================================
GEOM::GEOM_Object_ptr SMESHGUI_ShapeByMeshDlg::GetShape()
{
  return myGeomObj.in();
}

//=======================================================================
// function : onOk()
// purpose  : SLOT called when "Ok" button pressed.
//=======================================================================
void SMESHGUI_ShapeByMeshDlg::onOk()
{
  try {
    int elemID = myElementId->text().toInt();
    myGeomObj = SMESHGUI::GetSMESHGen()->GetGeometryByMeshElement
      ( myMesh.in(), elemID, myGeomName->text().latin1());

    accept();
    emit PublishShape();
  }
  catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }
  catch (...) {
  }
  myViewWindow->SetSelectionMode( ActorSelection );
  disconnect(mySelectionMgr, 0, this, 0);
  disconnect(mySMESHGUI, 0, this, 0);
  mySMESHGUI->ResetState();
}

//=======================================================================
// function : onClose()
// purpose  : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_ShapeByMeshDlg::onClose()
{
  myViewWindow->SetSelectionMode( ActorSelection );
  disconnect(mySelectionMgr, 0, this, 0);
  disconnect(mySMESHGUI, 0, this, 0);
  mySMESHGUI->ResetState();
  reject();
  emit Close();
}

//=======================================================================
// function : onSelectionDone()
// purpose  : SLOT called when selection changed
//=======================================================================
void SMESHGUI_ShapeByMeshDlg::onSelectionDone()
{
  myOkBtn->setEnabled( false );
  setElementID("");

  try {
    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects(aList, SVTK_Viewer::Type());
    if (aList.Extent() != 1)
      return;

    SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(aList.First());
    if (aMesh->_is_nil() || myMesh->_is_nil() || aMesh->GetId() != myMesh->GetId() )
      return;

    QString aString;
    int nbElems = SMESH::GetNameOfSelectedElements(myViewWindow->GetSelector(),
                                                   aList.First(), aString);
    if ( nbElems == 1 ) {
      setElementID( aString );
      myOkBtn->setEnabled( true );
    }
  } catch (...) {
  }
}

//=======================================================================
// function : onDeactivate()
// purpose  : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_ShapeByMeshDlg::onDeactivate()
{
  if ( isEnabled() ) {
    //disconnect(mySelectionMgr, 0, this, 0);
    myViewWindow->SetSelectionMode( ActorSelection );
    setEnabled(false);
  }
}

//=======================================================================
// function : enterEvent()
// purpose  : Event filter
//=======================================================================
void SMESHGUI_ShapeByMeshDlg::enterEvent (QEvent*)
{
  // there is a stange problem that enterEvent() comes after onSave()
  if ( isVisible () && !isEnabled() ) {
    mySMESHGUI->EmitSignalDeactivateDialog();
    setEnabled(true);
    activateSelection();
    //connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
  }
}

//=================================================================================
// function : closeEvent()
// purpose  : Close dialog box
//=================================================================================
void SMESHGUI_ShapeByMeshDlg::closeEvent (QCloseEvent*)
{
  onClose();
}

//=======================================================================
// function : activateSelection()
// purpose  : Activate selection in accordance with current pattern type
//=======================================================================
void SMESHGUI_ShapeByMeshDlg::activateSelection()
{
  mySelectionMgr->clearFilters();
  SMESH::SetPointRepresentation(false);

  myGeomName->setText("");

  if ( myViewWindow )
  {
    QString geomName;
    Selection_Mode mode = EdgeSelection;
    switch ( myElemTypeGroup->id( myElemTypeGroup->selected() )) {
    case EDGE  :
      mode = EdgeSelection;   geomName = tr("GEOM_EDGE"); break;
    case FACE  :
      mode = FaceSelection;   geomName = tr("GEOM_FACE"); break;
    case VOLUME:
      mode = VolumeSelection; geomName = tr(myHasSolids ? "GEOM_SOLID" : "GEOM_SHELL"); break;
    default: return;
    }
    if ( myViewWindow->SelectionMode() != mode )
      myViewWindow->SetSelectionMode( mode );

    myGeomName->setText( GEOMBase::GetDefaultName( geomName ));
  }
}

//=======================================================================
//function : onTypeChanged
//purpose  : SLOT. Called when element type changed.
//=======================================================================

void SMESHGUI_ShapeByMeshDlg::onTypeChanged (int theType)
{
  setElementID("");
  activateSelection();
}

//=======================================================================
//function : onTypeChanged
//purpose  : SLOT. Called when element id is entered
//           Highlight the element whose Ids the user entered manually
//=======================================================================

void SMESHGUI_ShapeByMeshDlg::onElemIdChanged(const QString& theNewText)
{
  myOkBtn->setEnabled( false );

  if ( myIsManualIdEnter && !myMesh->_is_nil() && myViewWindow )
    if ( SMESH_Actor* actor = SMESH::FindActorByObject(myMesh) )
      if ( SMDS_Mesh* aMesh = actor->GetObject()->GetMesh() )
      {
        SMDSAbs_ElementType type = SMDSAbs_Edge;
        switch ( myElemTypeGroup->id( myElemTypeGroup->selected() )) {
        case EDGE  : type = SMDSAbs_Edge;   break;
        case FACE  : type = SMDSAbs_Face;   break;
        case VOLUME: type = SMDSAbs_Volume; break;
        default: return;
        }
        TColStd_MapOfInteger newIndices;
        QStringList aListId = QStringList::split( " ", theNewText, false);
        for ( int i = 0; i < aListId.count(); i++ ) {
          if ( const SMDS_MeshElement * e = aMesh->FindElement( aListId[ i ].toInt() ))
            if ( e->GetType() == type )
              newIndices.Add( e->GetID() );
        }

        if ( !newIndices.IsEmpty() && newIndices.Extent() == 1 )
          if ( SVTK_Selector* s = myViewWindow->GetSelector() ) {
            s->AddOrRemoveIndex( actor->getIO(), newIndices, false );
            myViewWindow->highlight( actor->getIO(), true, true );
            myOkBtn->setEnabled( true );
          }
      }
}

//=======================================================================
//function : setElementID
//purpose  : programmatically set element id
//=======================================================================

void SMESHGUI_ShapeByMeshDlg::setElementID(const QString& theText)
{
  myIsManualIdEnter = false;
  myElementId->setText(theText);
  myIsManualIdEnter = true;
}
