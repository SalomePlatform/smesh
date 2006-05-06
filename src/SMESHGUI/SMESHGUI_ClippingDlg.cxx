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
//  File   : SMESHGUI_ClippingDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESHGUI_ClippingDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"

#include "SMESH_Actor.h"
#include "SMESH_ActorUtils.h"

#include "SUIT_Session.h"
#include "SUIT_OverrideCursor.h"
#include "SUIT_MessageBox.h"

#include "SALOME_ListIO.hxx"
#include "SALOME_InteractiveObject.hxx"
#include "SALOME_ListIteratorOfListIO.hxx"

#include "LightApp_Application.h"
#include "LightApp_SelectionMgr.h"

#include "SVTK_Selector.h"
#include "SVTK_ViewWindow.h"

// QT Includes
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qgroupbox.h>

// VTK Includes
#include <vtkMath.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkImplicitBoolean.h>
#include <vtkImplicitFunctionCollection.h>
#include <vtkObjectFactory.h>

#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkPlaneSource.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>

// STL includes
#include <algorithm>

using namespace std;


class OrientedPlane: public vtkPlane
{
  SVTK_ViewWindow* myViewWindow;

  vtkDataSetMapper* myMapper;

public:
  static OrientedPlane *New()
  {
    return new OrientedPlane();
  }
  static OrientedPlane *New(SVTK_ViewWindow* theViewWindow)
  {
    return new OrientedPlane(theViewWindow);
  }
  vtkTypeMacro (OrientedPlane, vtkPlane);

  SMESH::Orientation myOrientation;
  float myDistance;
  double myAngle[2];

  vtkPlaneSource* myPlaneSource;
  SALOME_Actor *myActor;

  void SetOrientation (SMESH::Orientation theOrientation) { myOrientation = theOrientation; }
  SMESH::Orientation GetOrientation() { return myOrientation; }

  void SetDistance (float theDistance) { myDistance = theDistance; }
  float GetDistance() { return myDistance; }

  void ShallowCopy (OrientedPlane* theOrientedPlane)
  {
    SetNormal(theOrientedPlane->GetNormal());
    SetOrigin(theOrientedPlane->GetOrigin());

    myOrientation = theOrientedPlane->GetOrientation();
    myDistance = theOrientedPlane->GetDistance();

    myAngle[0] = theOrientedPlane->myAngle[0];
    myAngle[1] = theOrientedPlane->myAngle[1];

    myPlaneSource->SetNormal(theOrientedPlane->myPlaneSource->GetNormal());
    myPlaneSource->SetOrigin(theOrientedPlane->myPlaneSource->GetOrigin());
    myPlaneSource->SetPoint1(theOrientedPlane->myPlaneSource->GetPoint1());
    myPlaneSource->SetPoint2(theOrientedPlane->myPlaneSource->GetPoint2());
  }

protected:
  OrientedPlane(SVTK_ViewWindow* theViewWindow):
    myViewWindow(theViewWindow),
    myOrientation(SMESH::XY),
    myDistance(0.5)
  {
    Init();
    myViewWindow->AddActor(myActor);
  }

  OrientedPlane():
    myOrientation(SMESH::XY),
    myViewWindow(NULL),
    myDistance(0.5)
  {
    Init();
  }

  void Init()
  {
    myPlaneSource = vtkPlaneSource::New();

    myAngle[0] = myAngle[1] = 0.0;

    // Create and display actor
    myMapper = vtkDataSetMapper::New();
    myMapper->SetInput(myPlaneSource->GetOutput());

    myActor = SALOME_Actor::New();
    myActor->VisibilityOff();
    myActor->PickableOff();
    myActor->SetInfinitive(true);
    myActor->SetMapper(myMapper);

    vtkFloatingPointType anRGB[3];
    vtkProperty* aProp = vtkProperty::New();
    SMESH::GetColor( "SMESH", "fill_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 170, 255 ) );
    aProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
    aProp->SetOpacity(0.75);
    myActor->SetProperty(aProp);
    aProp->Delete();

    vtkProperty* aBackProp = vtkProperty::New();
    SMESH::GetColor( "SMESH", "backface_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 0, 255 ) );
    aBackProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
    aBackProp->SetOpacity(0.75);
    myActor->SetBackfaceProperty(aBackProp);
    aBackProp->Delete();
  }

  ~OrientedPlane(){
    myViewWindow->RemoveActor(myActor);
    myActor->Delete();
    
    myMapper->RemoveAllInputs();
    myMapper->Delete();

    myPlaneSource->UnRegisterAllOutputs();
    myPlaneSource->Delete();
  };

private:
  // Not implemented.
  OrientedPlane (const OrientedPlane&);
  void operator= (const OrientedPlane&);

};

struct TSetVisiblity {
  TSetVisiblity(int theIsVisible): myIsVisible(theIsVisible){}
  void operator()(SMESH::TVTKPlane& theOrientedPlane){
    theOrientedPlane->myActor->SetVisibility(myIsVisible);
  }
  int myIsVisible;
};

//=================================================================================
// class    : SMESHGUI_ClippingDlg()
// purpose  :
//
//=================================================================================
SMESHGUI_ClippingDlg::SMESHGUI_ClippingDlg (SMESHGUI* theModule,
					    const char* name,
					    bool modal,
					    WFlags fl):
  QDialog(SMESH::GetDesktop(theModule),
	  name, 
	  modal, 
	  WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WDestructiveClose),
  mySelector(SMESH::GetViewWindow(theModule)->GetSelector()),
  mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
  mySMESHGUI(theModule)
{
  if (!name)
    setName("SMESHGUI_ClippingDlg");
  setCaption(tr("SMESH_CLIPPING_TITLE"));
  setSizeGripEnabled(TRUE);
  QGridLayout* SMESHGUI_ClippingDlgLayout = new QGridLayout(this);
  SMESHGUI_ClippingDlgLayout->setSpacing(6);
  SMESHGUI_ClippingDlgLayout->setMargin(11);

  // Controls for selecting, creating, deleting planes
  QGroupBox* GroupPlanes = new QGroupBox (this, "GroupPlanes");
  GroupPlanes->setTitle(tr("Clipping planes"));
  GroupPlanes->setColumnLayout(0, Qt::Vertical);
  GroupPlanes->layout()->setSpacing(0);
  GroupPlanes->layout()->setMargin(0);
  QGridLayout* GroupPlanesLayout = new QGridLayout (GroupPlanes->layout());
  GroupPlanesLayout->setAlignment(Qt::AlignTop);
  GroupPlanesLayout->setSpacing(6);
  GroupPlanesLayout->setMargin(11);

  ComboBoxPlanes = new QComboBox(GroupPlanes, "ComboBoxPlanes");
  GroupPlanesLayout->addWidget(ComboBoxPlanes, 0, 0);

  QSpacerItem* spacerGP = new QSpacerItem (20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupPlanesLayout->addItem(spacerGP, 0, 1);

  buttonNew = new QPushButton (GroupPlanes, "buttonNew");
  buttonNew->setText(tr("SMESH_BUT_NEW"));
  GroupPlanesLayout->addWidget(buttonNew, 0, 2);

  buttonDelete = new QPushButton (GroupPlanes, "buttonDelete");
  buttonDelete->setText(tr("SMESH_BUT_DELETE"));
  GroupPlanesLayout->addWidget(buttonDelete, 0, 3);

  // Controls for defining plane parameters
  QGroupBox* GroupParameters = new QGroupBox (this, "GroupParameters");
  GroupParameters->setTitle(tr("SMESH_PARAMETERS"));
  GroupParameters->setColumnLayout(0, Qt::Vertical);
  GroupParameters->layout()->setSpacing(0);
  GroupParameters->layout()->setMargin(0);
  QGridLayout* GroupParametersLayout = new QGridLayout (GroupParameters->layout());
  GroupParametersLayout->setAlignment(Qt::AlignTop);
  GroupParametersLayout->setSpacing(6);
  GroupParametersLayout->setMargin(11);

  TextLabelOrientation = new QLabel(GroupParameters, "TextLabelOrientation");
  TextLabelOrientation->setText(tr("SMESH_ORIENTATION"));
  GroupParametersLayout->addWidget(TextLabelOrientation, 0, 0);

  ComboBoxOrientation = new QComboBox(GroupParameters, "ComboBoxOrientation");
  GroupParametersLayout->addWidget(ComboBoxOrientation, 0, 1);

  TextLabelDistance = new QLabel(GroupParameters, "TextLabelDistance");
  TextLabelDistance->setText(tr("SMESH_DISTANCE"));
  GroupParametersLayout->addWidget(TextLabelDistance, 1, 0);

  SpinBoxDistance = new SMESHGUI_SpinBox(GroupParameters, "SpinBoxDistance");
  GroupParametersLayout->addWidget(SpinBoxDistance, 1, 1);

  TextLabelRot1 = new QLabel(GroupParameters, "TextLabelRot1");
  TextLabelRot1->setText(tr("Rotation around X (Y to Z):"));
  GroupParametersLayout->addWidget(TextLabelRot1, 2, 0);

  SpinBoxRot1 = new SMESHGUI_SpinBox(GroupParameters, "SpinBoxRot1");
  GroupParametersLayout->addWidget(SpinBoxRot1, 2, 1);

  TextLabelRot2 = new QLabel(GroupParameters, "TextLabelRot2");
  TextLabelRot2->setText(tr("Rotation around Y (X to Z):"));
  GroupParametersLayout->addWidget(TextLabelRot2, 3, 0);

  SpinBoxRot2 = new SMESHGUI_SpinBox(GroupParameters, "SpinBoxRot2");
  GroupParametersLayout->addWidget(SpinBoxRot2, 3, 1);

  PreviewCheckBox = new QCheckBox(tr("Show preview"), GroupParameters);
  PreviewCheckBox->setChecked(true);
  GroupParametersLayout->addWidget(PreviewCheckBox, 4, 0);

  AutoApplyCheckBox = new QCheckBox(tr("Auto Apply"), GroupParameters);
  AutoApplyCheckBox->setChecked(false);
  GroupParametersLayout->addWidget(AutoApplyCheckBox, 4, 1);

  // Controls for "Ok", "Apply" and "Close" button
  QGroupBox* GroupButtons = new QGroupBox(this, "GroupButtons");
  GroupButtons->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, GroupButtons->sizePolicy().hasHeightForWidth()));
  GroupButtons->setGeometry(QRect(10, 10, 281, 48));
  GroupButtons->setTitle(tr("" ));
  GroupButtons->setColumnLayout(0, Qt::Vertical);
  GroupButtons->layout()->setSpacing(0);
  GroupButtons->layout()->setMargin(0);
  QGridLayout* GroupButtonsLayout = new QGridLayout(GroupButtons->layout());
  GroupButtonsLayout->setAlignment(Qt::AlignTop);
  GroupButtonsLayout->setSpacing(6);
  GroupButtonsLayout->setMargin(11);
  buttonCancel = new QPushButton(GroupButtons, "buttonCancel");
  buttonCancel->setText(tr("SMESH_BUT_CLOSE" ));
  buttonCancel->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonCancel, 0, 3);
  buttonApply = new QPushButton(GroupButtons, "buttonApply");
  buttonApply->setText(tr("SMESH_BUT_APPLY" ));
  buttonApply->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonApply, 0, 1);
  QSpacerItem* spacer_9 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  GroupButtonsLayout->addItem(spacer_9, 0, 2);
  buttonOk = new QPushButton(GroupButtons, "buttonOk");
  buttonOk->setText(tr("SMESH_BUT_OK" ));
  buttonOk->setAutoDefault(TRUE);
  buttonOk->setDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonOk, 0, 0);
  buttonHelp = new QPushButton(GroupButtons, "buttonHelp");
  buttonHelp->setText(tr("SMESH_BUT_HELP" ));
  buttonHelp->setAutoDefault(TRUE);
  GroupButtonsLayout->addWidget(buttonHelp, 0, 4);

  SMESHGUI_ClippingDlgLayout->addWidget(GroupPlanes,      0, 0);
  SMESHGUI_ClippingDlgLayout->addWidget(GroupParameters,  1, 0);
  SMESHGUI_ClippingDlgLayout->addWidget(GroupButtons,     2, 0);

  // Initial state
  SpinBoxDistance->RangeStepAndValidator(0.0, 1.0, 0.01, 3);
  SpinBoxRot1->RangeStepAndValidator(-180.0, 180.0, 1, 3);
  SpinBoxRot2->RangeStepAndValidator(-180.0, 180.0, 1, 3);

  ComboBoxOrientation->insertItem(tr("|| X-Y"));
  ComboBoxOrientation->insertItem(tr("|| Y-Z"));
  ComboBoxOrientation->insertItem(tr("|| Z-X"));

  SpinBoxDistance->SetValue(0.5);

  myActor = 0;
  myIsSelectPlane = false;
  onSelectionChanged();

  myHelpFileName = "clipping.htm";

  // signals and slots connections :
  connect(ComboBoxPlanes, SIGNAL(activated(int)), this, SLOT(onSelectPlane(int)));
  connect(buttonNew, SIGNAL(clicked()), this, SLOT(ClickOnNew()));
  connect(buttonDelete, SIGNAL(clicked()), this, SLOT(ClickOnDelete()));
  connect(ComboBoxOrientation, SIGNAL(activated(int)), this, SLOT(onSelectOrientation(int)));
  connect(SpinBoxDistance, SIGNAL(valueChanged(double)), this, SLOT(SetCurrentPlaneParam()));
  connect(SpinBoxRot1, SIGNAL(valueChanged(double)), this, SLOT(SetCurrentPlaneParam()));
  connect(SpinBoxRot2, SIGNAL(valueChanged(double)), this, SLOT(SetCurrentPlaneParam()));
  connect(PreviewCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnPreviewToggle(bool)));
  connect(AutoApplyCheckBox, SIGNAL(toggled(bool)), this, SLOT(ClickOnApply()));
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(ClickOnOk()));
  connect(buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()));
  connect(buttonApply, SIGNAL(clicked()), this, SLOT(ClickOnApply()));
  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(ClickOnHelp()));
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnOk()));
  connect(mySelectionMgr,  SIGNAL(currentSelectionChanged()), this, SLOT(onSelectionChanged()));
  /* to close dialog if study frame change */
  connect(mySMESHGUI, SIGNAL (SignalStudyFrameChanged()), this, SLOT(ClickOnCancel()));

  this->show();
}

//=================================================================================
// function : ~SMESHGUI_ClippingDlg()
// purpose  :
//=================================================================================
SMESHGUI_ClippingDlg::~SMESHGUI_ClippingDlg()
{
  // no need to delete child widgets, Qt does it all for us
  std::for_each(myPlanes.begin(),myPlanes.end(),TSetVisiblity(false));
  SMESH::RenderViewWindow(SMESH::GetViewWindow(mySMESHGUI));
}

//=======================================================================
// function : ClickOnApply()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::ClickOnApply()
{
  if (!myActor)
    return;

  if (SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow(mySMESHGUI)) {
    SUIT_OverrideCursor wc;
    
    QWidget *aCurrWid = this->focusWidget();
    aCurrWid->clearFocus();
    aCurrWid->setFocus();

    myActor->RemoveAllClippingPlanes();

    SMESH::TPlanes::iterator anIter = myPlanes.begin();
    for (; anIter != myPlanes.end(); anIter++) {
      OrientedPlane* anOrientedPlane = OrientedPlane::New(aViewWindow);
      anOrientedPlane->ShallowCopy(anIter->GetPointer());
      myActor->AddClippingPlane(anOrientedPlane);
      anOrientedPlane->Delete();
    }

    SMESH::RenderViewWindow(aViewWindow);
  }
}

//=======================================================================
// function : ClickOnOk()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::ClickOnOk()
{
  ClickOnApply();
  ClickOnCancel();
}

//=======================================================================
// function : ClickOnCancel()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::ClickOnCancel()
{
  close();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_ClippingDlg::ClickOnHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app) 
    app->onHelpContextModule(mySMESHGUI ? app->moduleName(mySMESHGUI->moduleName()) : QString(""), myHelpFileName);
  else {
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", "application")).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
  }
}

//=================================================================================
// function : onSelectionChanged()
// purpose  : Called when selection is changed
//=================================================================================
void SMESHGUI_ClippingDlg::onSelectionChanged()
{
  if (SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow(mySMESHGUI)) {
    const SALOME_ListIO& aList = mySelector->StoredIObjects();
    if (aList.Extent() > 0) {
      Handle(SALOME_InteractiveObject) IOS = aList.First();
      myActor = SMESH::FindActorByEntry(IOS->getEntry());
      if (myActor) {
	std::for_each(myPlanes.begin(),myPlanes.end(),TSetVisiblity(false));
	myPlanes.clear();

	vtkIdType anId = 0, anEnd = myActor->GetNumberOfClippingPlanes();
	for (; anId < anEnd; anId++) {
	  if (vtkImplicitFunction* aFunction = myActor->GetClippingPlane(anId)) {
	    if(OrientedPlane* aPlane = OrientedPlane::SafeDownCast(aFunction)){
	      OrientedPlane* anOrientedPlane = OrientedPlane::New(aViewWindow);
	      SMESH::TVTKPlane aTVTKPlane(anOrientedPlane);
	      anOrientedPlane->Delete();
	      aTVTKPlane->ShallowCopy(aPlane);
	      myPlanes.push_back(aTVTKPlane);
	    }
	  }
	}

	std::for_each(myPlanes.begin(),myPlanes.end(),
		      TSetVisiblity(PreviewCheckBox->isChecked()));
      }
    }
    SMESH::RenderViewWindow(aViewWindow);
  }
  Sinchronize();
}

//=======================================================================
// function : onSelectPlane()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::onSelectPlane (int theIndex)
{
  if (!myActor || myPlanes.empty())
    return;

  OrientedPlane* aPlane = myPlanes[theIndex].GetPointer();

  // Orientation
  SMESH::Orientation anOrientation = aPlane->GetOrientation();

  // Rotations
  double aRot[2] = {aPlane->myAngle[0], aPlane->myAngle[1]};

  // Set plane parameters in the dialog
  myIsSelectPlane = true;
  setDistance(aPlane->GetDistance());
  setRotation(aRot[0], aRot[1]);
  switch (anOrientation) {
  case SMESH::XY:
    ComboBoxOrientation->setCurrentItem(0);
    onSelectOrientation(0);
    break;
  case SMESH::YZ:
    ComboBoxOrientation->setCurrentItem(1);
    onSelectOrientation(1);
    break;
  case SMESH::ZX:
    ComboBoxOrientation->setCurrentItem(2);
    onSelectOrientation(2);
    break;
  }
  myIsSelectPlane = false;
}

//=======================================================================
// function : ClickOnNew()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::ClickOnNew()
{
  if (!myActor)
    return;

  if(SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow(mySMESHGUI)){
    OrientedPlane* aPlane = OrientedPlane::New(aViewWindow);
    SMESH::TVTKPlane aTVTKPlane(aPlane);
    myPlanes.push_back(aTVTKPlane);

    if (PreviewCheckBox->isChecked())
      aTVTKPlane->myActor->VisibilityOn();
    
    Sinchronize();
    SetCurrentPlaneParam();
  }
}

//=======================================================================
// function : ClickOnDelete()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::ClickOnDelete()
{
  if (!myActor || myPlanes.empty())
    return;

  int aPlaneIndex = ComboBoxPlanes->currentItem();

  SMESH::TPlanes::iterator anIter = myPlanes.begin() + aPlaneIndex;
  anIter->GetPointer()->myActor->SetVisibility(false);
  myPlanes.erase(anIter);

  if(AutoApplyCheckBox->isChecked())
    ClickOnApply();

  Sinchronize();
  SMESH::RenderViewWindow(SMESH::GetCurrentVtkView());
}

//=======================================================================
// function : onSelectOrientation()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::onSelectOrientation (int theItem)
{
  if (myPlanes.empty())
    return;

  if      (theItem == 0) {
    TextLabelRot1->setText(tr("Rotation around X (Y to Z):"));
    TextLabelRot2->setText(tr("Rotation around Y (X to Z):"));
  }
  else if (theItem == 1) {
    TextLabelRot1->setText(tr("Rotation around Y (Z to X):"));
    TextLabelRot2->setText(tr("Rotation around Z (Y to X):"));
  }
  else if (theItem == 2) {
    TextLabelRot1->setText(tr("Rotation around Z (X to Y):"));
    TextLabelRot2->setText(tr("Rotation around X (Z to Y):"));
  }

  if((QComboBox*)sender() == ComboBoxOrientation)
    SetCurrentPlaneParam();
}

//=======================================================================
// function : Sinchronize()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::Sinchronize()
{
  int aNbPlanes = myPlanes.size();
  ComboBoxPlanes->clear();

  QString aName;
  for(int i = 1; i<=aNbPlanes; i++) {
    aName = QString(tr("Plane# %1")).arg(i);
    ComboBoxPlanes->insertItem(aName);
  }

  int aPos = ComboBoxPlanes->count() - 1;
  ComboBoxPlanes->setCurrentItem(aPos);

  bool anIsControlsEnable = (aPos >= 0);
  if (anIsControlsEnable) {
    onSelectPlane(aPos);
  } else {
    ComboBoxPlanes->insertItem(tr("No planes"));
    SpinBoxRot1->SetValue(0.0);
    SpinBoxRot2->SetValue(0.0);
    SpinBoxDistance->SetValue(0.5);
  }

  buttonDelete->setEnabled(anIsControlsEnable);
  buttonApply->setEnabled(anIsControlsEnable);
  PreviewCheckBox->setEnabled(anIsControlsEnable);
  AutoApplyCheckBox->setEnabled(anIsControlsEnable);
  ComboBoxOrientation->setEnabled(anIsControlsEnable);
  SpinBoxDistance->setEnabled(anIsControlsEnable);
  SpinBoxRot1->setEnabled(anIsControlsEnable);
  SpinBoxRot2->setEnabled(anIsControlsEnable);
}

//=======================================================================
// function : setRotation()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::setRotation (const double theRot1, const double theRot2)
{
  SpinBoxRot1->SetValue(theRot1);
  SpinBoxRot2->SetValue(theRot2);
}

//=======================================================================
// function : SetCurrentPlaneParam()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::SetCurrentPlaneParam()
{
  if (myPlanes.empty() || myIsSelectPlane)
    return;

  int aCurPlaneIndex = ComboBoxPlanes->currentItem();

  OrientedPlane* aPlane = myPlanes[aCurPlaneIndex].GetPointer();

  vtkFloatingPointType aNormal[3];
  SMESH::Orientation anOrientation;
  vtkFloatingPointType aDir[3][3] = {{0, 0, 0}, {0, 0, 0}};
  {
    static double aCoeff = vtkMath::Pi()/180.0;

    vtkFloatingPointType aRot[2] = {getRotation1(), getRotation2()};
    aPlane->myAngle[0] = aRot[0];
    aPlane->myAngle[1] = aRot[1];

    vtkFloatingPointType anU[2] = {cos(aCoeff*aRot[0]), cos(aCoeff*aRot[1])};
    vtkFloatingPointType aV[2] = {sqrt(1.0-anU[0]*anU[0]), sqrt(1.0-anU[1]*anU[1])};
    aV[0] = aRot[0] > 0? aV[0]: -aV[0];
    aV[1] = aRot[1] > 0? aV[1]: -aV[1];

    switch (ComboBoxOrientation->currentItem()) {
    case 0:
      anOrientation = SMESH::XY;

      aDir[0][1] = anU[0];
      aDir[0][2] = aV[0];

      aDir[1][0] = anU[1];
      aDir[1][2] = aV[1];

      break;
    case 1:
      anOrientation = SMESH::YZ;

      aDir[0][2] = anU[0];
      aDir[0][0] = aV[0];

      aDir[1][1] = anU[1];
      aDir[1][0] = aV[1];

      break;
    case 2:
      anOrientation = SMESH::ZX;

      aDir[0][0] = anU[0];
      aDir[0][1] = aV[0];

      aDir[1][2] = anU[1];
      aDir[1][1] = aV[1];

      break;
    }

    vtkMath::Cross(aDir[1],aDir[0],aNormal);
    vtkMath::Normalize(aNormal);
    vtkMath::Cross(aNormal,aDir[1],aDir[0]);
  }

  aPlane->SetOrientation(anOrientation);
  aPlane->SetDistance(getDistance());

  myActor->SetPlaneParam(aNormal, getDistance(), aPlane);

  vtkDataSet* aDataSet = myActor->GetInput();
  vtkFloatingPointType *aPnt = aDataSet->GetCenter();

  vtkFloatingPointType* anOrigin = aPlane->GetOrigin();
  vtkFloatingPointType aDel = aDataSet->GetLength()/2.0;

  vtkFloatingPointType aDelta[2][3] = {{aDir[0][0]*aDel, aDir[0][1]*aDel, aDir[0][2]*aDel},
				       {aDir[1][0]*aDel, aDir[1][1]*aDel, aDir[1][2]*aDel}};
  vtkFloatingPointType aParam, aPnt0[3], aPnt1[3], aPnt2[3];

  vtkFloatingPointType aPnt01[3] = {aPnt[0] - aDelta[0][0] - aDelta[1][0],
				    aPnt[1] - aDelta[0][1] - aDelta[1][1],
				    aPnt[2] - aDelta[0][2] - aDelta[1][2]};
  vtkFloatingPointType aPnt02[3] = {aPnt01[0] + aNormal[0],
				    aPnt01[1] + aNormal[1],
				    aPnt01[2] + aNormal[2]};
  vtkPlane::IntersectWithLine(aPnt01,aPnt02,aNormal,anOrigin,aParam,aPnt0);

  vtkFloatingPointType aPnt11[3] = {aPnt[0] - aDelta[0][0] + aDelta[1][0],
				    aPnt[1] - aDelta[0][1] + aDelta[1][1],
				    aPnt[2] - aDelta[0][2] + aDelta[1][2]};
  vtkFloatingPointType aPnt12[3] = {aPnt11[0] + aNormal[0],
				    aPnt11[1] + aNormal[1],
				    aPnt11[2] + aNormal[2]};
  vtkPlane::IntersectWithLine(aPnt11,aPnt12,aNormal,anOrigin,aParam,aPnt1);

  vtkFloatingPointType aPnt21[3] = {aPnt[0] + aDelta[0][0] - aDelta[1][0],
				    aPnt[1] + aDelta[0][1] - aDelta[1][1],
				    aPnt[2] + aDelta[0][2] - aDelta[1][2]};
  vtkFloatingPointType aPnt22[3] = {aPnt21[0] + aNormal[0],
				    aPnt21[1] + aNormal[1],
				    aPnt21[2] + aNormal[2]};
  vtkPlane::IntersectWithLine(aPnt21,aPnt22,aNormal,anOrigin,aParam,aPnt2);

  vtkPlaneSource* aPlaneSource = aPlane->myPlaneSource;
  aPlaneSource->SetNormal(aNormal[0],aNormal[1],aNormal[2]);
  aPlaneSource->SetOrigin(aPnt0[0],aPnt0[1],aPnt0[2]);
  aPlaneSource->SetPoint1(aPnt1[0],aPnt1[1],aPnt1[2]);
  aPlaneSource->SetPoint2(aPnt2[0],aPnt2[1],aPnt2[2]);

  if(AutoApplyCheckBox->isChecked())
    ClickOnApply();

  SMESH::RenderViewWindow(SMESH::GetCurrentVtkView());
}

//=======================================================================
// function : OnPreviewToggle()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::OnPreviewToggle (bool theIsToggled)
{
  std::for_each(myPlanes.begin(),myPlanes.end(),TSetVisiblity(theIsToggled));
  SMESH::RenderViewWindow(SMESH::GetCurrentVtkView());
}
