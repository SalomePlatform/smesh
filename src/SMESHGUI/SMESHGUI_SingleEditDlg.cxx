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

// File   : SMESHGUI_SingleEditDlg.cxx
// Author : Sergey LITONIN, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_SingleEditDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"

#include <SMESH_Actor.h>
#include <SMDS_Mesh.hxx>

// SALOME GUI includes
#include <LightApp_SelectionMgr.h>
#include <LightApp_Application.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>

#include <SVTK_Selector.h>
#include <SVTK_ViewWindow.h>
#include <SALOME_ListIO.hxx>

// OCCT includes
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QValidator>
#include <QKeyEvent>

#define SPACING 6
#define MARGIN  11

/*!
  \class BusyLocker
  \brief Simple 'busy state' flag locker.
  \internal
*/

class BusyLocker
{
public:
  //! Constructor. Sets passed boolean flag to \c true.
  BusyLocker( bool& busy ) : myBusy( busy ) { myBusy = true; }
  //! Destructor. Clear external boolean flag passed as parameter to the constructor to \c false.
  ~BusyLocker() { myBusy = false; }
private:
  bool& myBusy; //! External 'busy state' boolean flag
};

/*!
 *  Class       : SMESHGUI_SingleEditDlg
 *  Description : Inversion of the diagonal of a pseudo-quadrangle formed by
 *                2 neighboring triangles with 1 common edge
 */

//=======================================================================
// name    : SMESHGUI_SingleEditDlg()
// Purpose : Constructor
//=======================================================================
SMESHGUI_SingleEditDlg
::SMESHGUI_SingleEditDlg(SMESHGUI* theModule)
  : QDialog(SMESH::GetDesktop(theModule)),
    mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
    mySelector(SMESH::GetViewWindow(theModule)->GetSelector()),
    mySMESHGUI(theModule)
{
  setModal(false);

  QVBoxLayout* aDlgLay = new QVBoxLayout(this);
  aDlgLay->setMargin(MARGIN);
  aDlgLay->setSpacing(SPACING);

  QWidget* aMainFrame = createMainFrame  (this);
  QWidget* aBtnFrame  = createButtonFrame(this);

  aDlgLay->addWidget(aMainFrame);
  aDlgLay->addWidget(aBtnFrame);

  Init();
}

//=======================================================================
// name    : createMainFrame()
// Purpose : Create frame containing dialog's input fields
//=======================================================================
QWidget* SMESHGUI_SingleEditDlg::createMainFrame (QWidget* theParent)
{
  QGroupBox* aMainGrp = new QGroupBox(tr("EDGE_BETWEEN"), theParent);
  QHBoxLayout* aLay = new QHBoxLayout(aMainGrp);
  aLay->setMargin(MARGIN);
  aLay->setSpacing(SPACING);

  QPixmap aPix (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));

  QLabel* aLab = new QLabel(tr("SMESH_EDGE"), aMainGrp);
  QPushButton* aBtn = new QPushButton(aMainGrp);
  aBtn->setIcon(aPix);
  myEdge = new QLineEdit(aMainGrp);
  myEdge->setValidator(new QRegExpValidator(QRegExp("[\\d]*-[\\d]*"), this));

  aLay->addWidget(aLab);
  aLay->addWidget(aBtn);
  aLay->addWidget(myEdge);

  return aMainGrp;
}

//=======================================================================
// name    : createButtonFrame()
// Purpose : Create frame containing buttons
//=======================================================================
QWidget* SMESHGUI_SingleEditDlg::createButtonFrame (QWidget* theParent)
{
  QGroupBox* aFrame = new QGroupBox(theParent);

  myOkBtn     = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), aFrame);
  myApplyBtn  = new QPushButton(tr("SMESH_BUT_APPLY"), aFrame);
  myCloseBtn  = new QPushButton(tr("SMESH_BUT_CLOSE"), aFrame);
  myHelpBtn   = new QPushButton(tr("SMESH_BUT_HELP"),  aFrame);

  QHBoxLayout* aLay = new QHBoxLayout(aFrame);
  aLay->setMargin(MARGIN);
  aLay->setSpacing(SPACING);

  aLay->addWidget(myOkBtn);
  aLay->addSpacing(10);
  aLay->addWidget(myApplyBtn);
  aLay->addSpacing(10);
  aLay->addStretch();
  aLay->addWidget(myCloseBtn);
  aLay->addWidget(myHelpBtn);

  return aFrame;
}

//=======================================================================
// name    : isValid()
// Purpose : Verify validity of input data
//=======================================================================
bool SMESHGUI_SingleEditDlg::isValid (const bool theMess) const
{
  int id1, id2;
  return getNodeIds(myEdge->text(), id1, id2);
}

//=======================================================================
// name    : getNodeIds()
// Purpose : Retrieve node ids from string
//=======================================================================
bool SMESHGUI_SingleEditDlg::getNodeIds (const QString& theStr,
                                         int& theId1, int&  theId2) const
{
  if (!theStr.contains('-'))
    return false;

  bool ok1, ok2;
  QString str1 = theStr.section('-', 0, 0, QString::SectionSkipEmpty);
  QString str2 = theStr.section('-', 1, 1, QString::SectionSkipEmpty);
  theId1 = str1.toInt(&ok1);
  theId2 = str2.toInt(&ok2);

  return ok1 & ok2;
}

//=======================================================================
// name    : ~SMESHGUI_SingleEditDlg()
// Purpose : Destructor
//=======================================================================
SMESHGUI_SingleEditDlg::~SMESHGUI_SingleEditDlg()
{
}

//=======================================================================
// name    : Init()
// Purpose : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_SingleEditDlg::Init()
{
  mySMESHGUI->SetActiveDialogBox((QDialog*)this);
  myBusy = false;
  myActor = 0;

  // main buttons
  connect(myOkBtn,    SIGNAL(clicked()), SLOT(onOk()));
  connect(myCloseBtn, SIGNAL(clicked()), SLOT(reject()));
  connect(myApplyBtn, SIGNAL(clicked()), SLOT(onApply()));
  connect(myHelpBtn,  SIGNAL(clicked()), SLOT(onHelp()));

  // selection and SMESHGUI
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(reject()));
  connect(mySMESHGUI, SIGNAL(SignalActivatedViewManager()), SLOT(onOpenView()));
  connect(mySMESHGUI, SIGNAL(SignalCloseView()),            SLOT(onCloseView()));
  connect(myEdge, SIGNAL(textChanged(const QString&)), SLOT(onTextChange(const QString&)));

  myOkBtn->setEnabled(false);
  myApplyBtn->setEnabled(false);
  setEnabled(true);

  // set selection mode
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(EdgeOfCellSelection);

  onSelectionDone();
}

//=======================================================================
// name    : onOk()
// Purpose : SLOT called when "Ok" button pressed.
//           Assign filters VTK viewer and close dialog
//=======================================================================
void SMESHGUI_SingleEditDlg::onOk()
{
  if (onApply())
    reject();
}

//=======================================================================
// name    : reject()
// Purpose : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_SingleEditDlg::reject()
{
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  //mySelectionMgr->clearSelected();
  disconnect(mySelectionMgr, 0, this, 0);
  disconnect(mySMESHGUI, 0, this, 0);
  mySMESHGUI->ResetState();
  QDialog::reject();
}

//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_SingleEditDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app) 
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
    QString platform;
#ifdef WIN32
    platform = "winapplication";
#else
    platform = "application";
#endif
    SUIT_MessageBox::warning(this, tr("WRN_WARNING"),
                             tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                             arg(app->resourceMgr()->stringValue("ExternalBrowser", 
                                                                 platform)).
                             arg(myHelpFileName));
  }
}

//=======================================================================
//function : findTriangles()
//purpose  : find triangles sharing theNode1-theNode2 link
//           THIS IS A PIECE OF SMESH_MeshEditor.cxx
//           TO DO: make it available in SMDS for ex.
//=======================================================================
static bool findTriangles (const SMDS_MeshNode *    theNode1,
                           const SMDS_MeshNode *    theNode2,
                           const SMDS_MeshElement*& theTria1,
                           const SMDS_MeshElement*& theTria2)
{
  if (!theNode1 || !theNode2) return false;

  theTria1 = theTria2 = 0;

  std::set< const SMDS_MeshElement* > emap;
  SMDS_ElemIteratorPtr it = theNode1->GetInverseElementIterator();
  while (it->more()) {
    const SMDS_MeshElement* elem = it->next();
    if (elem->GetType() == SMDSAbs_Face && elem->NbCornerNodes() == 3)
      emap.insert(elem);
  }
  it = theNode2->GetInverseElementIterator();
  while (it->more()) {
    const SMDS_MeshElement* elem = it->next();
    if (elem->GetType() == SMDSAbs_Face &&
         emap.find(elem) != emap.end())
    {
      if (theTria1) {
        theTria2 = elem;
        break;
      } else {
        theTria1 = elem;
      }
    }
  }
  return (theTria1 && theTria2);
}

//=======================================================================
//function : onTextChange()
//purpose  :
//=======================================================================
void SMESHGUI_SingleEditDlg::onTextChange (const QString& theNewText)
{
  if (myBusy) return;
  BusyLocker lock(myBusy);

  myOkBtn->setEnabled(false);
  myApplyBtn->setEnabled(false);

  // hilight entered edge
  if(myActor){
    if(SMDS_Mesh* aMesh = myActor->GetObject()->GetMesh()){
      Handle(SALOME_InteractiveObject) anIO = myActor->getIO();
      SALOME_ListIO aList;
      aList.Append(anIO);
      mySelectionMgr->setSelectedObjects(aList,false);
      
      TColStd_IndexedMapOfInteger selectedIndices;
      TColStd_MapOfInteger newIndices;
      mySelector->GetIndex(anIO,selectedIndices);

      int id1, id2;
      if ( !getNodeIds(myEdge->text(), id1, id2) )
        return;

      const SMDS_MeshNode* aNode1 = aMesh->FindNode( id1 );
      const SMDS_MeshNode* aNode2 = aMesh->FindNode( id2 );

      if ( !aNode1 || !aNode2 || aNode1 == aNode2 )
        return;

      // find a triangle and an edge index
      const SMDS_MeshElement* tria1;
      const SMDS_MeshElement* tria2;

      if ( findTriangles(aNode1,aNode2,tria1,tria2) )
      {
        newIndices.Add(tria1->GetID());

        const SMDS_MeshNode* a3Nodes[3];
        SMDS_ElemIteratorPtr it;
        int edgeInd = 2, i;
        for (i = 0, it = tria1->nodesIterator(); it->more(); i++) {
          a3Nodes[ i ] = static_cast<const SMDS_MeshNode*>(it->next());
          if (i > 0 && ( (a3Nodes[ i ] == aNode1 && a3Nodes[ i - 1] == aNode2) ||
                         (a3Nodes[ i ] == aNode2 && a3Nodes[ i - 1] == aNode1) ) ) {
            edgeInd = i - 1;
            break;
          }
        }
        newIndices.Add(-edgeInd-1);
        
        myOkBtn->setEnabled(true);
        myApplyBtn->setEnabled(true);
      }
      mySelector->AddOrRemoveIndex(anIO,newIndices, false);
      SMESH::GetViewWindow(mySMESHGUI)->highlight( anIO, true, true );
    }
  }
}

//=======================================================================
// name    : onSelectionDone()
// Purpose : SLOT called when selection changed
//=======================================================================
void SMESHGUI_SingleEditDlg::onSelectionDone()
{
  if (myBusy) return;
  BusyLocker lock(myBusy);

  int anId1 = 0, anId2 = 0;

  myOkBtn->setEnabled(false);
  myApplyBtn->setEnabled(false);

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);

  if (aList.Extent() != 1) {
    myEdge->clear();
    return;
  }

  Handle(SALOME_InteractiveObject) anIO = aList.First();
  myActor = SMESH::FindActorByEntry(anIO->getEntry());
  if(myActor){
    TVisualObjPtr aVisualObj = myActor->GetObject();
    if(SMDS_Mesh* aMesh = aVisualObj->GetMesh())
    {
      const SMDS_MeshElement* tria[2];
      if( SMESH::GetEdgeNodes( mySelector, aVisualObj, anId1, anId2 ) >= 1 &&
          findTriangles( aMesh->FindNode( anId1 ), aMesh->FindNode( anId2 ), tria[0],tria[1] ) )
      {
        QString aText = QString("%1-%2").arg(anId1).arg(anId2);
        myEdge->setText(aText);
        
        myOkBtn->setEnabled(true);
        myApplyBtn->setEnabled(true);
      }
      else
      {
        myEdge->clear();
      }
    }
  }
}

//=======================================================================
// name    : onDeactivate()
// Purpose : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_SingleEditDlg::onDeactivate()
{
  setEnabled(false);
}

//=================================================================================
// function : onOpenView()
// purpose  :
//=================================================================================
void SMESHGUI_SingleEditDlg::onOpenView()
{
  if ( !mySelector ) {
    mySelector = SMESH::GetViewWindow( mySMESHGUI )->GetSelector();
    mySMESHGUI->EmitSignalDeactivateDialog();
    setEnabled(true);
  }
}

//=================================================================================
// function : onCloseView()
// purpose  :
//=================================================================================
void SMESHGUI_SingleEditDlg::onCloseView()
{
  onDeactivate();
  mySelector = 0;
}

//=======================================================================
// name    : enterEvent()
// Purpose : Event filter
//=======================================================================
void SMESHGUI_SingleEditDlg::enterEvent (QEvent*)
{
  if (!isEnabled()) {
    mySMESHGUI->EmitSignalDeactivateDialog();
    SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI );
    if ( aViewWindow) {
      aViewWindow->SetSelectionMode(EdgeOfCellSelection);
      if (!mySelector)
        mySelector = aViewWindow->GetSelector();
    }
    setEnabled(true);
  }
}

//=================================================================================
// function : onApply()
// purpose  : SLOT. Called when apply button is pressed
//=================================================================================
bool SMESHGUI_SingleEditDlg::onApply()
{
  if (mySMESHGUI->isActiveStudyLocked())
    return false;
  // verify validity of input data
  if (!isValid(true))
    return false;

  // get mesh, actor and nodes
  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);

  SMESH::SMESH_Mesh_var aMesh = SMESH::GetMeshByIO(aList.First());

  if (aMesh->_is_nil()) {
    SUIT_MessageBox::information(SMESH::GetDesktop(mySMESHGUI), 
                                 tr("SMESH_ERROR"),
                                 tr("SMESHG_NO_MESH"));
    return false;
  }

  SMESH::SMESH_MeshEditor_var aMeshEditor = aMesh->GetMeshEditor();
  int anId1= 0, anId2 = 0;
  if (aMeshEditor->_is_nil() || !getNodeIds(myEdge->text(), anId1, anId2))
    return false;

  // perform operation
  bool aResult = process(aMeshEditor.in(), anId1, anId2);

  // update actor
  if (aResult) {
    mySelector->ClearIndex();
    mySelectionMgr->setSelectedObjects(aList, false);
    onSelectionDone();
    SMESH::UpdateView();
    SMESHGUI::Modified();
  }

  return aResult;
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SingleEditDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    onHelp();
  }
}

/*!
 *  Class       : SMESHGUI_TrianglesInversionDlg
 *  Description : Inversion of the diagonal of a pseudo-quadrangle formed by
 *                2 neighboring triangles with 1 common edge
 */

SMESHGUI_TrianglesInversionDlg
::SMESHGUI_TrianglesInversionDlg(SMESHGUI* theModule)
: SMESHGUI_SingleEditDlg(theModule)
{
  setWindowTitle(tr("CAPTION"));
  myHelpFileName = "diagonal_inversion_of_elements_page.html";
}

SMESHGUI_TrianglesInversionDlg::~SMESHGUI_TrianglesInversionDlg()
{
}

bool SMESHGUI_TrianglesInversionDlg::process (SMESH::SMESH_MeshEditor_ptr theMeshEditor,
                                              const int theId1, const int theId2)
{
  return theMeshEditor->InverseDiag(theId1, theId2);
}

/*!
 *  Class       : SMESHGUI_UnionOfTwoTrianglesDlg
 *  Description : Construction of a quadrangle by deletion of the
 *                common border of 2 neighboring triangles
 */

SMESHGUI_UnionOfTwoTrianglesDlg
::SMESHGUI_UnionOfTwoTrianglesDlg(SMESHGUI* theModule)
: SMESHGUI_SingleEditDlg(theModule)
{
  setWindowTitle(tr("CAPTION"));
  myHelpFileName = "uniting_two_triangles_page.html";
}

SMESHGUI_UnionOfTwoTrianglesDlg::~SMESHGUI_UnionOfTwoTrianglesDlg()
{
}

bool SMESHGUI_UnionOfTwoTrianglesDlg::process (SMESH::SMESH_MeshEditor_ptr theMeshEditor,
                                               const int theId1, const int theId2)
{
  return theMeshEditor->DeleteDiag(theId1, theId2);
}
