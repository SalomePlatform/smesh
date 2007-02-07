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
//  File   : SMESHGUI_CreatePatternDlg.cxx
//  Author : Sergey LITONIN
//  Module : SMESH

#include "SMESHGUI_CreatePatternDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_PatternWidget.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_PatternUtils.h"
#include "SMESHGUI_GEOMGenUtils.h"

#include "SMESH_NumberFilter.hxx"

#include "SUIT_ResourceMgr.h"
#include "SUIT_Desktop.h"
#include "SUIT_FileDlg.h"
#include "SUIT_Session.h"
#include "SUIT_MessageBox.h"

#include "SalomeApp_Study.h"
#include "LightApp_Application.h"
#include "LightApp_DataOwner.h"
#include "LightApp_SelectionMgr.h"
#include "SalomeApp_Tools.h"

#include "SALOMEDS_SObject.hxx"

#include "SALOME_ListIO.hxx"
#include "SVTK_ViewModel.h"
#include "SVTK_ViewWindow.h"
#include "SVTK_Selector.h"
#include "SVTK_Selection.h"

// OCCT Includes
#include <TColStd_MapOfInteger.hxx>

// QT Includes
#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qmessagebox.h>
#include <qapplication.h>

#define SPACING 5
#define MARGIN  10

/*!
 *  Class       : SMESHGUI_CreatePatternDlg
 *  Description : Dialog to specify filters for VTK viewer
 */

//=======================================================================
// function : SMESHGUI_CreatePatternDlg()
// purpose  : Constructor
//=======================================================================
SMESHGUI_CreatePatternDlg::SMESHGUI_CreatePatternDlg( SMESHGUI*   theModule,
                                                      const int   theType,
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

  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    mySelector = aViewWindow->GetSelector();

  myHelpFileName = "pattern_mapping.htm";

  Init(theType);
}

//=======================================================================
// function : createMainFrame()
// purpose  : Create frame containing dialog's input fields
//=======================================================================
QFrame* SMESHGUI_CreatePatternDlg::createMainFrame (QWidget* theParent)
{
  QPixmap iconSlct     (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_SELECT")));
  QPixmap icon2d       (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_PATTERN_2d")));
  QPixmap icon3d       (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_PATTERN_3d")));
  QPixmap iconSample2d (SMESH::GetResourceMgr( mySMESHGUI )->loadPixmap("SMESH", tr("ICON_PATTERN_SAMPLE_2D")));

  QGroupBox* aMainGrp = new QGroupBox(1, Qt::Horizontal, theParent);
  aMainGrp->setFrameStyle(QFrame::NoFrame);
  aMainGrp->setInsideMargin(0);

  // Pattern type group

  myTypeGrp = new QButtonGroup (1, Qt::Vertical, tr("PATTERN_TYPE"), aMainGrp);
  mySwitch2d = new QRadioButton (myTypeGrp);
  mySwitch3d = new QRadioButton (myTypeGrp);
  mySwitch2d->setPixmap(icon2d);
  mySwitch3d->setPixmap(icon3d);
  myTypeGrp->insert(mySwitch2d, Type_2d);
  myTypeGrp->insert(mySwitch3d, Type_3d);

  QGroupBox* aPatternGrp = new QGroupBox(1, Qt::Horizontal, tr("PATTERN"), aMainGrp);

  // Mesh and pattern name group

  QGroupBox* aMeshGrp = new QGroupBox(1, Qt::Vertical, aPatternGrp);
  aMeshGrp->setFrameStyle(QFrame::NoFrame);
  aMeshGrp->setInsideMargin(0);

  new QLabel(tr("MESH_OR_SUBMESH"), aMeshGrp);

  QPushButton* aSelBtn = new QPushButton(aMeshGrp);
  aSelBtn->setPixmap(iconSlct);
  myMeshEdit = new QLineEdit(aMeshGrp);
  myMeshEdit->setReadOnly(true);

  QGroupBox* aPattGrp = new QGroupBox(1, Qt::Vertical, aPatternGrp);
  aPattGrp->setFrameStyle(QFrame::NoFrame);
  aPattGrp->setInsideMargin(0);

  new QLabel(tr("PATTERN_NAME"), aPattGrp);
  myName = new QLineEdit(aPattGrp);

  // Picture 2d

  myPicture2d = new SMESHGUI_PatternWidget(aPatternGrp),
  myPicture2d->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

  // Project check box

  myProjectChk = new QCheckBox(tr("PROJECT"), aPatternGrp);

  // Connect signals and slots

  connect(myTypeGrp,    SIGNAL(clicked(int) ), SLOT(onTypeChanged(int)));
  connect(myProjectChk, SIGNAL(toggled(bool)), SLOT(onProject(bool)   ));
  connect(aSelBtn,      SIGNAL(clicked()      ), SLOT(onSelBtnClicked()));

  return aMainGrp;
}

//=======================================================================
// function : createButtonFrame()
// purpose  : Create frame containing buttons
//=======================================================================
QFrame* SMESHGUI_CreatePatternDlg::createButtonFrame (QWidget* theParent)
{
  QFrame* aFrame = new QFrame(theParent);
  aFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);

  myOkBtn    = new QPushButton(tr("SMESH_BUT_OK"    ), aFrame);
  mySaveBtn  = new QPushButton(tr("SAVE"            ), aFrame);
  myCloseBtn = new QPushButton(tr("SMESH_BUT_CANCEL"), aFrame);
  myHelpBtn = new QPushButton(tr("SMESH_BUT_HELP"), aFrame);

  QSpacerItem* aSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

  QHBoxLayout* aLay = new QHBoxLayout(aFrame, MARGIN, SPACING);

  aLay->addWidget(myOkBtn);
  aLay->addWidget(mySaveBtn);
  aLay->addItem(aSpacer);
  aLay->addWidget(myCloseBtn);
  aLay->addWidget(myHelpBtn);

  connect(myOkBtn,    SIGNAL(clicked()), SLOT(onOk()));
  connect(myCloseBtn, SIGNAL(clicked()), SLOT(onClose()));
  connect(mySaveBtn, SIGNAL(clicked()), SLOT(onSave()));
  connect(myHelpBtn, SIGNAL(clicked()), SLOT(onHelp()));

  return aFrame;
}

//=======================================================================
// function : ~SMESHGUI_CreatePatternDlg()
// purpose  : Destructor
//=======================================================================
SMESHGUI_CreatePatternDlg::~SMESHGUI_CreatePatternDlg()
{
  // no need to delete child widgets, Qt does it all for us
}

//=======================================================================
// function : onProject()
// purpose  : SLOT. Called when state of "Project nodes on ther face"
//            checkbox is changed
//=======================================================================
void SMESHGUI_CreatePatternDlg::onProject (bool)
{
  loadFromObject(false);
  displayPreview();
}

//=======================================================================
// function : Init()
// purpose  : Init dialog fields, connect signals and slots, show dialog
//=======================================================================
void SMESHGUI_CreatePatternDlg::Init( const int theType )
{
  myIsLoaded     = false;
  myType         = -1;
  myNbPoints     = -1;
  mySubMesh      = SMESH::SMESH_subMesh::_nil();
  myMesh         = SMESH::SMESH_Mesh::_nil();
  myGeomObj      = GEOM::GEOM_Object::_nil();
  myPattern      = SMESH::SMESH_Pattern::_nil();

  erasePreview();

  mySMESHGUI->SetActiveDialogBox((QDialog*)this);

  // selection and SMESHGUI
  connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
  connect(mySMESHGUI, SIGNAL(SignalDeactivateActiveDialog()), SLOT(onDeactivate()));
  connect(mySMESHGUI, SIGNAL(SignalCloseAllDialogs()), SLOT(onClose()));

  mySwitch2d->setEnabled(theType == Type_2d);
  mySwitch3d->setEnabled(theType == Type_3d);

  if (theType == Type_2d)
    myProjectChk->show();
  else
    myProjectChk->hide();

  myTypeGrp->setButton(theType);
  onTypeChanged(theType);

  myName->setText(getDefaultName());
  myMeshEdit->setText("");

  setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
  qApp->processEvents();
  updateGeometry();
  myPicture2d->updateGeometry();
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
void SMESHGUI_CreatePatternDlg::SetMesh (SMESH::SMESH_Mesh_ptr thePtr)
{
  myMesh = SMESH::SMESH_Mesh::_duplicate(thePtr);
  mySubMesh = SMESH::SMESH_subMesh::_nil();

  bool isValidMesh = false;
  if (!myMesh->_is_nil())
  {
    _PTR(SObject) aSobj = SMESH::FindSObject(myMesh.in());
    //Handle(SALOME_InteractiveObject) anIObj =
    //  new SALOME_InteractiveObject(aSobj->GetID().c_str(), "SMESH");
    SUIT_DataOwnerPtr anIObj (new LightApp_DataOwner(aSobj->GetID().c_str()));

    isValidMesh = mySelectionMgr->isOk(anIObj);
  }

  if (isValidMesh) {
    _PTR(SObject) aSO = SMESH::FindSObject(myMesh.in());
    myMeshEdit->setText(aSO->GetName().c_str());
    myGeomObj = SMESH::GetGeom(aSO);
  } else {
    myMeshEdit->setText("");
    myGeomObj = GEOM::GEOM_Object::_nil();
  }

  if (myType == Type_2d) {
    loadFromObject(false);
    displayPreview();
  }
}

//=======================================================================
// function : isValid()
// purpose  : Verify validity of entry data
//=======================================================================
bool SMESHGUI_CreatePatternDlg::isValid()
{
  if (myGeomObj->_is_nil()) {
    QMessageBox::information(SMESH::GetDesktop( mySMESHGUI ),
                             tr("SMESH_INSUFFICIENT_DATA"),
                             tr("SMESHGUI_INVALID_PARAMETERS"),
                             QMessageBox::Ok);
    return false;
  }
  else
    return true;
}

//=======================================================================
// function : getDefaultName()
// purpose  : Get default pattern name
//=======================================================================
QString SMESHGUI_CreatePatternDlg::getDefaultName() const
{
  return myType == Type_2d ? tr("DEFAULT_2D") : tr("DEFAULT_3D");
}

//=======================================================================
// function : onSave()
// purpose  : SLOT called when "Save" button pressed. Build pattern and
//           save it to disk
//=======================================================================
void SMESHGUI_CreatePatternDlg::onSave()
{
  try {
    if (!isValid())
      return;

    if (!myIsLoaded)
      loadFromObject(true);

    // Load pattern from object
    if (!myIsLoaded)
      return;

    ///////////////////////////////////////////////////////
    SUIT_FileDlg* aDlg = new SUIT_FileDlg (this, false);
    aDlg->setCaption(tr("SAVE_PATTERN"));
    aDlg->setMode(QFileDialog::AnyFile);
    aDlg->setFilters(tr("PATTERN_FILT"));
    if (myName->text() != "")
      aDlg->setSelection(myName->text());

    if (aDlg->exec() != Accepted)
      return;

    QString fName = aDlg->selectedFile();
    if (fName.isEmpty())
      return;

    if (QFileInfo(fName).extension().isEmpty())
      fName = autoExtension(fName);

    fName = QDir::convertSeparators(fName);

    QString aData (myPattern->GetString());
    long aLen = aData.length();

    QFile aFile (fName);
    aFile.open(IO_WriteOnly);
    long aWritten = aFile.writeBlock(aData, aLen);
    aFile.close();

    if (aWritten != aLen) {
      QMessageBox::information(SMESH::GetDesktop( mySMESHGUI ), tr("SMESH_ERROR"),
                               tr("ERROR_OF_SAVING"), QMessageBox::Ok);
    } else {
      //SUIT_Application::getDesktop()->setSelectionModes(ActorSelection);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode(ActorSelection);
      disconnect(mySelectionMgr, 0, this, 0);
      disconnect(mySMESHGUI, 0, this, 0);
      mySMESHGUI->ResetState();
      accept();
      emit NewPattern();
    }
  } catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  } catch (...) {
  }
}

//=======================================================================
// function : GetPatternName()
// purpose  : Get name of pattern
//=======================================================================
QString SMESHGUI_CreatePatternDlg::GetPatternName() const
{
  return myName->text();
}

//=======================================================================
// function : GetPattern()
// purpose  : Get result pattern
//=======================================================================
SMESH::SMESH_Pattern_ptr SMESHGUI_CreatePatternDlg::GetPattern()
{
  return myPattern.in();
}

//=======================================================================
// function : onOk()
// purpose  : SLOT called when "Ok" button pressed.
//=======================================================================
void SMESHGUI_CreatePatternDlg::onOk()
{
  try {
    if (!isValid())
      return;

    if (!myIsLoaded)
      loadFromObject(true);

    // Load pattern from object
    if (!myIsLoaded) {
      return;
    } else {
      //SUIT_Application::getDesktop()->setSelectionModes(ActorSelection);
      if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
	aViewWindow->SetSelectionMode(ActorSelection);
      disconnect(mySelectionMgr, 0, this, 0);
      disconnect(mySMESHGUI, 0, this, 0);
      mySMESHGUI->ResetState();
      accept();
      emit NewPattern();
    }
  } catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  } catch (...) {
  }
}

//=======================================================================
// function : onClose()
// purpose  : SLOT called when "Close" button pressed. Close dialog
//=======================================================================
void SMESHGUI_CreatePatternDlg::onClose()
{
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);
  disconnect(mySelectionMgr, 0, this, 0);
  disconnect(mySMESHGUI, 0, this, 0);
  mySMESHGUI->ResetState();
  reject();
  emit Close();
}

//=================================================================================
// function : onHelp()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePatternDlg::onHelp()
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
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", platform)).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
  }
}

//=======================================================================
// function : loadFromObject()
// purpose  : Load pattern from geom object corresponding to the mesh/submesh
//=======================================================================
bool SMESHGUI_CreatePatternDlg::loadFromObject (const bool theMess)
{
  try {
    myIsLoaded = false;

    if (myPattern->_is_nil())
      myPattern = SMESH::GetPattern();

    if (myMesh->_is_nil() && mySubMesh->_is_nil() || myGeomObj->_is_nil())
      return false;

    SMESH::SMESH_Mesh_ptr aMesh = mySubMesh->_is_nil() ? myMesh.in() : mySubMesh->GetFather();

    myIsLoaded = myType == Type_2d
      ? myPattern->LoadFromFace(aMesh, myGeomObj, myProjectChk->isChecked())
      : myPattern->LoadFrom3DBlock(aMesh, myGeomObj);

    if (!myIsLoaded && theMess) {
      QString aMess;
      SMESH::SMESH_Pattern::ErrorCode aCode = myPattern->GetErrorCode();

      if      (aCode == SMESH::SMESH_Pattern::ERR_LOAD_EMPTY_SUBMESH  ) aMess = tr("ERR_LOAD_EMPTY_SUBMESH");
      else if (aCode == SMESH::SMESH_Pattern::ERR_LOADF_NARROW_FACE   ) aMess = tr("ERR_LOADF_NARROW_FACE");
      else if (aCode == SMESH::SMESH_Pattern::ERR_LOADF_CLOSED_FACE   ) aMess = tr("ERR_LOADF_CLOSED_FACE");
      else if (aCode == SMESH::SMESH_Pattern::ERR_LOADF_CANT_PROJECT   ) aMess = tr("ERR_LOADF_CANT_PROJECT");
      else if (aCode == SMESH::SMESH_Pattern::ERR_LOADV_BAD_SHAPE     ) aMess = tr("ERR_LOADV_BAD_SHAPE");
      else if (aCode == SMESH::SMESH_Pattern::ERR_LOADV_COMPUTE_PARAMS) aMess = tr("ERR_LOADV_COMPUTE_PARAMS");
      else                                                              aMess = tr("ERROR_OF_CREATION");

      QMessageBox::information(SMESH::GetDesktop( mySMESHGUI ),
                               tr("SMESH_ERROR"), aMess, QMessageBox::Ok);
    }
  } catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  }

  return myIsLoaded;
}

//=======================================================================
// function : onSelectionDone()
// purpose  : SLOT called when selection changed
//=======================================================================
void SMESHGUI_CreatePatternDlg::onSelectionDone()
{
  try {
    SALOME_ListIO aList;
    mySelectionMgr->selectedObjects(aList, SVTK_Viewer::Type());
    if (aList.Extent() != 1)
      return;

    // Get mesh or sub-mesh from selection
    Handle(SALOME_InteractiveObject) anIO = aList.First();
    SMESH::SMESH_Mesh_var aMesh = SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(anIO);
    SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(anIO);
    if (aMesh->_is_nil() && aSubMesh->_is_nil())
      return;

    // Get geom object corresponding to the mesh
    _PTR(SObject) aSO;
    if (!aMesh->_is_nil())
      aSO = SMESH::FindSObject(aMesh.in());
    else
      aSO = SMESH::FindSObject(aSubMesh.in());

    GEOM::GEOM_Object_var aGeomObj = SMESH::GetGeom(aSO);
    if (aGeomObj->_is_nil())
      return;

    myGeomObj = aGeomObj;

    // init class fields
    if (!aMesh->_is_nil()) {
      myMesh = aMesh;
      mySubMesh = SMESH::SMESH_subMesh::_nil();
    } else {
      mySubMesh = aSubMesh;
      myMesh = SMESH::SMESH_Mesh::_nil();
    }

    QString aName;
    SMESH::GetNameOfSelectedIObjects(mySelectionMgr, aName);
    myMeshEdit->setText(aName);

    if (myType == Type_2d) {
      loadFromObject(true);
      displayPreview();
    }
  } catch (...) {
    myMesh = SMESH::SMESH_Mesh::_nil();
    mySubMesh = SMESH::SMESH_subMesh::_nil();
    myGeomObj = GEOM::GEOM_Object::_nil();
    erasePreview();
  }
}

//=======================================================================
// function : onDeactivate()
// purpose  : SLOT called when dialog must be deativated
//=======================================================================
void SMESHGUI_CreatePatternDlg::onDeactivate()
{
  disconnect(mySelectionMgr, 0, this, 0);
  setEnabled(false);
}

//=======================================================================
// function : enterEvent()
// purpose  : Event filter
//=======================================================================
void SMESHGUI_CreatePatternDlg::enterEvent (QEvent*)
{
  // there is a stange problem that enterEvent() comes after onSave()
  if ( isVisible () ) {
    mySMESHGUI->EmitSignalDeactivateDialog();
    setEnabled(true);
    activateSelection();
    connect(mySelectionMgr, SIGNAL(currentSelectionChanged()), SLOT(onSelectionDone()));
  }
}

//=================================================================================
// function : closeEvent()
// purpose  : Close dialog box
//=================================================================================
void SMESHGUI_CreatePatternDlg::closeEvent (QCloseEvent*)
{
  onClose();
}

//=======================================================================
// function : onSelBtnClicked()
// purpose  : SLOT. Called when -> button clicked.
//=======================================================================
void SMESHGUI_CreatePatternDlg::onSelBtnClicked()
{
  onSelectionDone();
}

//================================================================
// function : autoExtension()
// purpose  : Append extension to the file name
//================================================================
QString SMESHGUI_CreatePatternDlg::autoExtension (const QString& theFileName) const
{
  QString anExt = theFileName.section('.', -1);
  return anExt != "smp" && anExt != "SMP" ? theFileName + ".smp" : theFileName;
}

//=======================================================================
// function : displayPreview()
// purpose  : Display preview
//=======================================================================
void SMESHGUI_CreatePatternDlg::displayPreview()
{
  // Redisplay preview in dialog
  try {
    if (!myIsLoaded) {
      erasePreview();
    } else {
      SMESH::point_array_var pnts = myPattern->GetPoints();
      SMESH::long_array_var keyPoints = myPattern->GetKeyPoints();
      SMESH::array_of_long_array_var elemPoints = myPattern->GetElementPoints(false);

      if (pnts->length()       == 0 ||
          keyPoints->length()  == 0 ||
          elemPoints->length() == 0) {
        myIsLoaded = false;
        erasePreview();
        return;
      }

      PointVector aPoints (pnts->length());
      QValueVector<int> aKeyPoints (keyPoints->length());
      ConnectivityVector anElemPoints (elemPoints->length());

      for (int i = 0, n = pnts->length(); i < n; i++)
        aPoints[ i ] = pnts[ i ];

      for (int i2 = 0, n2 = keyPoints->length(); i2 < n2; i2++)
        aKeyPoints[ i2 ] = keyPoints[ i2 ];

      for (int i3 = 0, n3 = elemPoints->length(); i3 < n3; i3++) {
        QValueVector<int> aVec (elemPoints[ i3 ].length());
        for (int i4 = 0, n4 = elemPoints[ i3 ].length(); i4 < n4; i4++)
          aVec[ i4 ] = elemPoints[ i3 ][ i4 ];

        anElemPoints[ i3 ] = aVec;
      }

      myPicture2d->SetPoints(aPoints, aKeyPoints, anElemPoints);
    }

    return;

  } catch (const SALOME::SALOME_Exception& S_ex) {
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
  } catch (...) {
  }
  erasePreview();
}

//=======================================================================
// function : erasePreview()
// purpose  : Erase preview
//=======================================================================
void SMESHGUI_CreatePatternDlg::erasePreview()
{
  // Erase preview in 2D viewer
  myPicture2d->SetPoints(PointVector(), QValueVector<int>(), ConnectivityVector());
}

//=======================================================================
// function : activateSelection()
// purpose  : Activate selection in accordance with current pattern type
//=======================================================================
void SMESHGUI_CreatePatternDlg::activateSelection()
{
  mySelectionMgr->clearFilters();
  //SUIT_Application::getDesktop()->setSelectionModes(ActorSelection);
  if ( SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow( mySMESHGUI ))
    aViewWindow->SetSelectionMode(ActorSelection);

  if (myType == Type_2d) {
    mySelectionMgr->installFilter(new SMESH_NumberFilter
      ("SMESH", TopAbs_SHAPE, -1, TopAbs_FACE));
  } else {
    TColStd_MapOfInteger aTypes;
    aTypes.Add(TopAbs_SHELL);
    aTypes.Add(TopAbs_SOLID);
    mySelectionMgr->installFilter(new SMESH_NumberFilter
      ("SMESH", TopAbs_FACE, 6, aTypes, GEOM::GEOM_Object::_nil(), true));
  }
}

//=======================================================================
// function : onTypeChanged()
// purpose  : SLOT. Called when pattern type changed.
//            Change dialog's look and feel
//=======================================================================
void SMESHGUI_CreatePatternDlg::onTypeChanged (int theType)
{
  if (myType == theType)
    return;

  myType = theType;

  if (theType == Type_2d)
    myPicture2d->show();
  else
    myPicture2d->hide();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_CreatePatternDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Key_F1 )
    {
      e->accept();
      onHelp();
    }
}
