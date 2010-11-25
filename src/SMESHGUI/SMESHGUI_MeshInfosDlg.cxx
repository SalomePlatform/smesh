//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// SMESH SMESHGUI : GUI for SMESH component
// File   : SMESHGUI_MeshInfosDlg.cxx
// Author : Nicolas BARBEROU
// SMESH includes
//
#include "SMESHGUI_MeshInfosDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_MeshInfosBox.h"

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>

#include <LightApp_SelectionMgr.h>
#include <LightApp_Application.h>
#include <SALOME_ListIO.hxx>

// SALOME KERNEL includes
#include <SALOMEDSClient_Study.hxx>

// Qt includes
#include <QGroupBox>
#include <QLabel>
#include <QFrame>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QKeyEvent>

// IDL includes
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)

#define COLONIZE(str)   (QString(str).contains(":") > 0 ? QString(str) : QString(str) + " :" )
#define SPACING 6
#define MARGIN  11

//=================================================================================
// function : SMESHGUI_MeshInfosDlg()
// purpose  : Constructor
//=================================================================================
SMESHGUI_MeshInfosDlg::SMESHGUI_MeshInfosDlg(SMESHGUI* theModule): 
  QDialog(SMESH::GetDesktop(theModule)),
  mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
  mySMESHGUI(theModule)
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle(tr("SMESH_MESHINFO_TITLE"));
  setSizeGripEnabled(true);

  myStartSelection = true;
  myIsActiveWindow = true;

  QVBoxLayout* aTopLayout = new QVBoxLayout(this);
  aTopLayout->setSpacing(SPACING);  aTopLayout->setMargin(MARGIN);

  // select button & label
  QPixmap image0(SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH",tr("ICON_SELECT")));
  mySelectBtn = new QPushButton(this);
  mySelectBtn->setIcon(image0);
  mySelectBtn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  mySelectLab = new QLabel(this);
  mySelectLab->setAlignment(Qt::AlignCenter);
  QFont fnt = mySelectLab->font(); fnt.setBold(true);
  mySelectLab->setFont(fnt);

  QHBoxLayout* aSelectLayout = new QHBoxLayout;
  aSelectLayout->setMargin(0); aSelectLayout->setSpacing(0);
  aSelectLayout->addWidget(mySelectBtn);
  aSelectLayout->addWidget(mySelectLab);

  // top widget stack
  myWGStack = new QStackedWidget(this);

  // no valid selection
  QWidget* myBadWidget = new QWidget(myWGStack);
  QVBoxLayout* aBadLayout = new QVBoxLayout(myBadWidget);
  QLabel* myBadLab = new QLabel(tr("SMESH_BAD_SELECTION"), myBadWidget);
  myBadLab->setAlignment(Qt::AlignCenter);
  myBadLab->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  aBadLayout->addWidget(myBadLab);
  myWGStack->addWidget(myBadWidget);

  // mesh
  myMeshWidget = new QWidget(myWGStack);
  QGridLayout* aMeshLayout = new QGridLayout(myMeshWidget);
  aMeshLayout->setSpacing(SPACING);  aMeshLayout->setMargin(0);
  myWGStack->addWidget(myMeshWidget);

  // --> name
  QLabel* myMeshNameLab = new QLabel(COLONIZE(tr("SMESH_MESHINFO_NAME")), myMeshWidget);
  myMeshName    = new QLabel(myMeshWidget);
  myMeshName->setMinimumWidth(100);
  QFrame* line1 = new QFrame(myMeshWidget);
  line1->setFrameStyle(QFrame::HLine | QFrame::Sunken);

  myMeshInfoBox = new SMESHGUI_MeshInfosBox(true, myMeshWidget);

  aMeshLayout->addWidget(myMeshNameLab,      0, 0);
  aMeshLayout->addWidget(myMeshName,         0, 1);
  aMeshLayout->addWidget(line1,              1, 0, 1, 2);
  aMeshLayout->addWidget(myMeshInfoBox,      2, 0, 1, 2);
  aMeshLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 3, 0);

  // buttons
  myButtonsGroup = new QGroupBox(this);
  QHBoxLayout* myButtonsGroupLayout = new QHBoxLayout(myButtonsGroup);
  myButtonsGroupLayout->setSpacing(SPACING); myButtonsGroupLayout->setMargin(MARGIN);

  // buttons --> OK and Help buttons
  myOkBtn = new QPushButton(tr("SMESH_BUT_OK" ), myButtonsGroup);
  myOkBtn->setAutoDefault(true);
  myOkBtn->setDefault(true);
  myOkBtn->setFocus();
  myHelpBtn = new QPushButton(tr("SMESH_BUT_HELP" ), myButtonsGroup);
  myHelpBtn->setAutoDefault(true);

  myButtonsGroupLayout->addWidget(myOkBtn);
  myButtonsGroupLayout->addSpacing(10);
  myButtonsGroupLayout->addStretch();
  myButtonsGroupLayout->addWidget(myHelpBtn);

  aTopLayout->addLayout(aSelectLayout);
  aTopLayout->addWidget(myWGStack);
  aTopLayout->addWidget(myButtonsGroup);

  mySMESHGUI->SetActiveDialogBox(this);

  // connect signals
  connect(myOkBtn,                 SIGNAL(clicked()),                      this, SLOT(close()));
  connect( myHelpBtn,              SIGNAL(clicked()),                      this, SLOT(onHelp()));
  connect(mySelectBtn,             SIGNAL(clicked()),                      this, SLOT(onStartSelection()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()),        this, SLOT(close()));
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), this, SLOT(DeactivateActiveDialog()));
  connect(mySelectionMgr,          SIGNAL(currentSelectionChanged()),      this, SLOT(onSelectionChanged()));

  // init dialog with current selection
  onSelectionChanged();

  myHelpFileName = "mesh_infos_page.html#advanced_mesh_infos_anchor";
}

//=================================================================================
// function : ~SMESHGUI_MeshInfosDlg()
// purpose  : Destructor
//=================================================================================
SMESHGUI_MeshInfosDlg::~SMESHGUI_MeshInfosDlg()
{
}

//=================================================================================
// function : DumpMeshInfos()
// purpose  : 
//=================================================================================
void SMESHGUI_MeshInfosDlg::DumpMeshInfos()
{
  SUIT_OverrideCursor wc;

  SALOME_ListIO aList;
  mySelectionMgr->selectedObjects(aList);

  int nbSel = aList.Extent();
  if (nbSel == 1) {
    myStartSelection = false;
    mySelectLab->setText("");
    Handle(SALOME_InteractiveObject) IObject = aList.First();
    _PTR(SObject) aSO = SMESH::GetActiveStudyDocument()->FindObjectID(IObject->getEntry());
    if (aSO) {
      //CORBA::Object_var anObject = aSO->GetObject();
      CORBA::Object_var anObject = SMESH::SObjectToObject(aSO);
      if (!CORBA::is_nil(anObject)) {
        SMESH::SMESH_IDSource_var anIDSource = SMESH::SMESH_IDSource::_narrow(anObject);
        if (!anIDSource->_is_nil()) {
          myWGStack->setCurrentWidget(myMeshWidget);
          setWindowTitle(tr("SMESH_MESHINFO_TITLE") + " [" + tr("SMESH_OBJECT_MESH") + "]");
          myMeshName->setText(aSO->GetName().c_str());

          SMESH::long_array_var aMeshInfo = anIDSource->GetMeshInfo();
          myMeshInfoBox->SetMeshInfo( aMeshInfo );

          return;
        }
      }
    }
  }
  myWGStack->setCurrentIndex(0);
  setWindowTitle(tr("SMESH_MESHINFO_TITLE"));
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection has changed
//=================================================================================
void SMESHGUI_MeshInfosDlg::onSelectionChanged()
{
  if (myStartSelection)
    DumpMeshInfos();
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::closeEvent(QCloseEvent* e)
{
  mySMESHGUI->ResetState();
  QDialog::closeEvent(e);
}

//=================================================================================
// function : windowActivationChange()
// purpose  : called when window is activated/deactivated
//=================================================================================
void SMESHGUI_MeshInfosDlg::windowActivationChange(bool oldActive)
{
  QDialog::windowActivationChange(oldActive);
  if (isActiveWindow() && myIsActiveWindow != isActiveWindow())
    ActivateThisDialog();
  myIsActiveWindow = isActiveWindow();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::DeactivateActiveDialog()
{
  disconnect(mySelectionMgr, 0, this, 0);
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate any active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), this, SLOT(onSelectionChanged()));
}

//=================================================================================
// function : onStartSelection()
// purpose  : starts selection
//=================================================================================
void SMESHGUI_MeshInfosDlg::onStartSelection()
{
  myStartSelection = true;
  onSelectionChanged();
  myStartSelection = true;
  mySelectLab->setText(tr("INF_SELECT_OBJECT"));
}

//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::onHelp()
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

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_MeshInfosDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    onHelp();
  }
}
