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
//  File   : SMESHGUI_SingleEditDlg.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI_SingleEditDlg.h"

#include "QAD_Desktop.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"

#include "SMESHGUI_SpinBox.h"

#include "utilities.h"
#include "SALOME_Selection.h"
#include "SMESH_Actor.h"
#include "SMDS_Mesh.hxx"

#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qvalidator.h>


#define SPACING 5
#define MARGIN  10


/*
  Class       : SMESHGUI_DiagValidator
  Description : validate munual input of edge like "id1-id2"
*/
class SMESHGUI_DiagValidator: public QValidator
{
 public:

  SMESHGUI_DiagValidator(QWidget * parent, const char * name = 0):
    QValidator(parent,name) {}

  State validate ( QString & text, int & pos) const
  {
    text.stripWhiteSpace();
    text.replace( QRegExp("[^0-9]+"), "-" );
    if ( text == "-" )
      text = "";
    int ind = text.find( QRegExp("-[0-9]+-"));
    if ( ind > 0 ) { // leave only two ids
      ind = text.find( '-', ind + 1 );
      if ( ind > 0 )
        text.truncate( ind );
    }
    if ( pos > text.length() )
      pos = text.length();
    return Acceptable;
  }
};

/*
  Class       : SMESHGUI_SingleEditDlg
  Description : Inversion of the diagonal of a pseudo-quadrangle formed by 
                2 neighboring triangles with 1 common edge
*/

//=======================================================================
// name    : SMESHGUI_SingleEditDlg::SMESHGUI_SingleEditDlg
// Purpose : Constructor
//=======================================================================
SMESHGUI_SingleEditDlg::SMESHGUI_SingleEditDlg( QWidget*          theParent, 
                                                      SALOME_Selection* theSelection,
                                                      const char*       theName )
: QDialog( theParent, theName, false, 
           WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
  //setCaption( tr( "CAPTION" ) );

  QVBoxLayout* aDlgLay = new QVBoxLayout( this, MARGIN, SPACING );

  QFrame* aMainFrame = createMainFrame  ( this );
  QFrame* aBtnFrame  = createButtonFrame( this );

  aDlgLay->addWidget( aMainFrame );
  aDlgLay->addWidget( aBtnFrame );

  aDlgLay->setStretchFactor( aMainFrame, 1 );

  Init( theSelection ) ; 
}

//=======================================================================
// name    : SMESHGUI_SingleEditDlg::createMainFrame
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_SingleEditDlg::createMainFrame( QWidget* theParent )
{
  QGroupBox* aMainGrp = new QGroupBox( 1, Qt::Vertical, tr( "EDGE_BETWEEN" ), theParent );

  QPixmap aPix( QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr( "ICON_SELECT" ) ) );
  
  new QLabel( tr( "SMESH_EDGE" ), aMainGrp );
  ( new QPushButton( aMainGrp ) )->setPixmap( aPix );
  myEdge = new QLineEdit( aMainGrp );
  myEdge->setValidator( new SMESHGUI_DiagValidator( this, "validator" ));

  return aMainGrp;
}

//=======================================================================
// name    : SMESHGUI_SingleEditDlg::createButtonFrame
// Purpose : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_SingleEditDlg::createButtonFrame( QWidget* theParent )
{
  QFrame* aFrame = new QFrame( theParent );
  aFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

  myOkBtn     = new QPushButton( tr( "SMESH_BUT_OK"    ), aFrame );
  myApplyBtn  = new QPushButton( tr( "SMESH_BUT_APPLY" ), aFrame );
  myCloseBtn  = new QPushButton( tr( "SMESH_BUT_CLOSE" ), aFrame );

  QSpacerItem* aSpacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );

  QHBoxLayout* aLay = new QHBoxLayout( aFrame, MARGIN, SPACING );

  aLay->addWidget( myOkBtn );
  aLay->addWidget( myApplyBtn );
  aLay->addItem( aSpacer);
  aLay->addWidget( myCloseBtn );

  return aFrame;
}

//=======================================================================
// name    : SMESHGUI_SingleEditDlg::isValid
// Purpose : Verify validity of input data
//=======================================================================
bool SMESHGUI_SingleEditDlg::isValid( const bool theMess ) const
{
  int id1, id2;
  return getNodeIds( myEdge->text(), id1, id2 );
}

//=======================================================================
// name    : SMESHGUI_SingleEditDlg::getNodeIds
// Purpose : Retrieve node ids from string
//=======================================================================
bool SMESHGUI_SingleEditDlg::getNodeIds( const QString& theStr, int& theId1, int&  theId2 ) const
{
  if ( !theStr.contains( '-' ) )
    return false;

  bool ok1, ok2;
  QString str1 = theStr.section( '-', 0, 0, QString::SectionSkipEmpty );
  QString str2 = theStr.section( '-', 1, 1, QString::SectionSkipEmpty );
  theId1 = str1.toInt( &ok1 );
  theId2 = str2.toInt( &ok2 );
  
  return ok1 & ok2;
}

//=======================================================================
// name    : SMESHGUI_SingleEditDlg::~SMESHGUI_SingleEditDlg
// Purpose : Destructor
//=======================================================================
SMESHGUI_SingleEditDlg::~SMESHGUI_SingleEditDlg()
{
}

//=======================================================================
// name    : SMESHGUI_SingleEditDlg::Init
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_SingleEditDlg::Init( SALOME_Selection* theSelection )
{
  mySelection = theSelection;  
  SMESHGUI* aSMESHGUI = SMESHGUI::GetSMESHGUI();
  aSMESHGUI->SetActiveDialogBox( ( QDialog* )this ) ;
  myBusy = false;
  myActor = 0;
 
  // main buttons
  connect( myOkBtn,    SIGNAL( clicked() ), SLOT( onOk() ) );
  connect( myCloseBtn, SIGNAL( clicked() ), SLOT( onClose() ) ) ;
  connect( myApplyBtn, SIGNAL( clicked() ), SLOT( onApply() ) );

  // selection and SMESHGUI
  connect( mySelection, SIGNAL( currentSelectionChanged() ), SLOT( onSelectionDone() ) );
  connect( aSMESHGUI, SIGNAL( SignalDeactivateActiveDialog() ), SLOT( onDeactivate() ) );
  connect( aSMESHGUI, SIGNAL( SignalCloseAllDialogs() ), SLOT( onClose() ) );
  connect( myEdge, SIGNAL( textChanged(const QString&)), SLOT( onTextChange(const QString&)));

  myOkBtn->setEnabled( false );
  myApplyBtn->setEnabled( false );
  setEnabled( true );

  int x, y ;
  aSMESHGUI->DefineDlgPosition( this, x, y );
  this->move( x, y );
  this->show(); 

  // set selection mode
  QAD_Application::getDesktop()->SetSelectionMode( EdgeOfCellSelection, true );

  onSelectionDone();

  return;
}

//=======================================================================
// name    : SMESHGUI_SingleEditDlg::onOk
// Purpose : SLOT called when "Ok" button pressed. 
//           Assign filters VTK viewer and close dialog
//=======================================================================
void SMESHGUI_SingleEditDlg::onOk()
{
  if ( onApply() )
    onClose();
}

//=======================================================================
// name    : SMESHGUI_SingleEditDlg::onClose
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_SingleEditDlg::onClose()
{
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
  mySelection->ClearIObjects();
  disconnect( mySelection, 0, this, 0 );
  disconnect( SMESHGUI::GetSMESHGUI(), 0, this, 0 );
  SMESHGUI::GetSMESHGUI()->ResetState() ;
  reject();
}

//=======================================================================
//function : findTriangles
//purpose  : find triangles sharing theNode1-theNode2 link
//    THIS IS A PIECE OF SMESH_MeshEditor.cxx
//    TO DO: make it available in SMDS for ex.
//=======================================================================

static bool findTriangles(const SMDS_MeshNode *    theNode1,
                          const SMDS_MeshNode *    theNode2,
                          const SMDS_MeshElement*& theTria1,
                          const SMDS_MeshElement*& theTria2)
{
  if ( !theNode1 || !theNode2 ) return false;

  theTria1 = theTria2 = 0;

  set< const SMDS_MeshElement* > emap;
  SMDS_ElemIteratorPtr it = theNode1->GetInverseElementIterator();
  while (it->more()) {
    const SMDS_MeshElement* elem = it->next();
    if ( elem->GetType() == SMDSAbs_Face && elem->NbNodes() == 3 )
      emap.insert( elem );
  }
  it = theNode2->GetInverseElementIterator();
  while (it->more()) {
    const SMDS_MeshElement* elem = it->next();
    if ( elem->GetType() == SMDSAbs_Face &&
         emap.find( elem ) != emap.end() )
      if ( theTria1 ) {
        theTria2 = elem;
        break;
      } else {
        theTria1 = elem;
      }
  }
  return ( theTria1 && theTria2 );
}

//=======================================================================
//function : onTextChange
//purpose  : 
//=======================================================================

void SMESHGUI_SingleEditDlg::onTextChange( const QString& theNewText )
{
  if ( myBusy ) return;

  myOkBtn->setEnabled( false );
  myApplyBtn->setEnabled( false );

  // hilight entered edge
  SMDS_Mesh* aMesh = 0;
  if ( myActor )
    aMesh = myActor->GetObject()->GetMesh();
  if ( aMesh ) {

    myBusy = true; // block onSelectionDone()
    mySelection->ClearIObjects();
    mySelection->AddIObject( myActor->getIO() );
    myBusy = false;

    QStringList aListId = QStringList::split( "-", theNewText, false);
    if ( aListId.count() != 2 )
      return;
    const SMDS_MeshNode* a2Nodes[2];
    bool allOk = true;
    int i;
    for ( i = 0; i < aListId.count(); i++ ) {
      const SMDS_MeshNode * n = aMesh->FindNode( aListId[ i ].toInt() );
      if ( n )
        a2Nodes[ i ] = n;
      else
        allOk = false;
    }

    // find a triangle and an edge nb
    const SMDS_MeshElement* tria[2];
    if (allOk &&
        a2Nodes[0] != a2Nodes[1] &&
        findTriangles( a2Nodes[0],
                      a2Nodes[1],
                      tria[0],
                      tria[1])) {
      myBusy = true; // block onSelectionDone()
      mySelection->AddOrRemoveIndex (myActor->getIO(), tria[0]->GetID(), true, false);

      const SMDS_MeshNode* a3Nodes [3];
      SMDS_ElemIteratorPtr it;
      int edgeInd = 2;
      for (i = 0, it = tria[0]->nodesIterator(); it->more(); i++ ) {
        a3Nodes[ i ] = static_cast<const SMDS_MeshNode*>( it->next() );
        if ( i > 0 )
          if (( a3Nodes[ i ] == a2Nodes[ 0 ] && a3Nodes[ i - 1] == a2Nodes[ 1 ] ) ||
              ( a3Nodes[ i ] == a2Nodes[ 1 ] && a3Nodes[ i - 1] == a2Nodes[ 0 ] ) ) {
            edgeInd = i - 1;
            break;
          }
      }

      mySelection->AddOrRemoveIndex( myActor->getIO(), -edgeInd-1, true, true );
      myBusy = false;

      myOkBtn->setEnabled( true );
      myApplyBtn->setEnabled( true );
    }
  }
}
//=======================================================================
// name    : SMESHGUI_SingleEditDlg::onSelectionDone
// Purpose : SLOT called when selection changed
//=======================================================================
void SMESHGUI_SingleEditDlg::onSelectionDone()
{
  if ( myBusy ) return;

  int anId1 = 0, anId2 = 0;

  myOkBtn->setEnabled( false );
  myApplyBtn->setEnabled( false );

  if ( mySelection->IObjectCount() != 1 )
  {
    myEdge->clear();
    return;
  }

  myActor = SMESH::FindActorByEntry( mySelection->firstIObject()->getEntry() );
  SMDS_Mesh* aMesh = 0;
  if ( myActor )
    aMesh = myActor->GetObject()->GetMesh();
  if ( !aMesh )
    return;

  if (SMESH::GetEdgeNodes( mySelection, anId1, anId2 ) >= 1 ) {
    QString aText = QString( "%1-%2" ).arg( anId1 ).arg( anId2 );
    myBusy = true;
    myEdge->setText( aText );    
    myBusy = false;

    const SMDS_MeshElement* tria[2];
    if ( findTriangles( aMesh->FindNode( anId1 ), aMesh->FindNode( anId2 ), tria[0],tria[1]))
    {
      myOkBtn->setEnabled( true );
      myApplyBtn->setEnabled( true );
    }
  }
  else
    myEdge->clear();
}

//=======================================================================
// name    : SMESHGUI_SingleEditDlg::onDeactivate
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_SingleEditDlg::onDeactivate()
{
  setEnabled( false );
}

//=======================================================================
// name    : SMESHGUI_SingleEditDlg::enterEvent
// Purpose : Event filter
//=======================================================================
void SMESHGUI_SingleEditDlg::enterEvent( QEvent* )
{
  if ( !isEnabled() ) {
    SMESHGUI::GetSMESHGUI()->EmitSignalDeactivateDialog();
    // set selection mode
    QAD_Application::getDesktop()->SetSelectionMode( EdgeOfCellSelection, true ); 
    setEnabled( true );
  }
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SingleEditDlg::closeEvent( QCloseEvent* e )
{
  onClose() ;
}
//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================

void SMESHGUI_SingleEditDlg::hideEvent ( QHideEvent * e )
{
  if ( !isMinimized() )
    onClose();
}

//=================================================================================
// function : onApply
// purpose  : SLOT. Called when apply button is pressed
//=================================================================================
bool SMESHGUI_SingleEditDlg::onApply()
{
  if (SMESHGUI::GetSMESHGUI()->ActiveStudyLocked())
    return false;
  // verify validity of input data
  if ( !isValid( true ) )
    return false;

  // get mesh, actor and nodes    
  SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO( mySelection->firstIObject() );

  if ( aMesh->_is_nil() )
  {
    QMessageBox::information( SMESHGUI::GetSMESHGUI()->GetDesktop(),
    tr( "SMESH_ERROR" ), tr( "SMESHG_NO_MESH" ), QMessageBox::Ok );
    return false;
  }

  SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
  int anId1= 0, anId2 = 0;
  if ( aMeshEditor->_is_nil() || !getNodeIds( myEdge->text(), anId1, anId2 ) )
    return false;

  // perform operation
  bool aResult = process( aMeshEditor.in(), anId1, anId2 );

  // update actor
  if ( aResult )
  {
    Handle(SALOME_InteractiveObject) anIO = mySelection->firstIObject();
    mySelection->ClearIObjects();
    SMESH::UpdateView();
    mySelection->AddIObject( anIO, false );
  }

  return aResult;
}

/*
  Class       : SMESHGUI_TrianglesInversionDlg
  Description : Inversion of the diagonal of a pseudo-quadrangle formed by
                2 neighboring triangles with 1 common edge
*/

SMESHGUI_TrianglesInversionDlg::SMESHGUI_TrianglesInversionDlg( QWidget*          theParent,
                                                                SALOME_Selection* theSelection,
                                                                const char*       theName )
: SMESHGUI_SingleEditDlg( theParent, theSelection, theName )
{
  setCaption( tr( "CAPTION" ) );
}

SMESHGUI_TrianglesInversionDlg::~SMESHGUI_TrianglesInversionDlg()
{
}

bool SMESHGUI_TrianglesInversionDlg::process(
  SMESH::SMESH_MeshEditor_ptr theMeshEditor, const int theId1, const int theId2 )
{
  return theMeshEditor->InverseDiag( theId1, theId2 );
}

/*
  Class       : SMESHGUI_UnionOfTwoTrianglesDlg
  Description : Construction of a quadrangle by deletion of the 
                common border of 2 neighboring triangles
*/

SMESHGUI_UnionOfTwoTrianglesDlg::SMESHGUI_UnionOfTwoTrianglesDlg( QWidget*          theParent, 
                                                                  SALOME_Selection* theSelection,
                                                                  const char*       theName )
: SMESHGUI_SingleEditDlg( theParent, theSelection, theName )
{
  setCaption( tr( "CAPTION" ) );
}

SMESHGUI_UnionOfTwoTrianglesDlg::~SMESHGUI_UnionOfTwoTrianglesDlg()
{
}

bool SMESHGUI_UnionOfTwoTrianglesDlg::process(
  SMESH::SMESH_MeshEditor_ptr theMeshEditor, const int theId1, const int theId2 )
{
  return theMeshEditor->DeleteDiag( theId1, theId2 );
}

