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
// File   : SMESHGUI_ClippingDlg.cxx
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_ClippingDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_SpinBox.h"

#include <SMESH_Actor.h>
#include <SMESH_ActorUtils.h>

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>

#include <SALOME_ListIO.hxx>

#include <LightApp_Application.h>
#include <LightApp_SelectionMgr.h>

#include <SVTK_ViewWindow.h>

// Qt includes
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QKeyEvent>

// VTK includes
#include <vtkMath.h>
#include <vtkPlane.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkPlaneSource.h>
#include <vtkProperty.h>

#define SPACING 6
#define MARGIN  11

class OrientedPlane: public vtkPlane
{
  QPointer<SVTK_ViewWindow> myViewWindow;

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
    if (myViewWindow)
      myViewWindow->RemoveActor(myActor);
    myActor->Delete();
    
    myMapper->RemoveAllInputs();
    myMapper->Delete();

    // commented: porting to vtk 5.0
    //    myPlaneSource->UnRegisterAllOutputs();
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
// used in SMESHGUI::restoreVisualParameters() to avoid
// declaration of OrientedPlane outside of SMESHGUI_ClippingDlg.cxx
//=================================================================================
void SMESHGUI_ClippingDlg::AddPlane (SMESH_Actor*         theActor,
                                     SVTK_ViewWindow*     theViewWindow,
                                     SMESH::Orientation   theOrientation,
                                     double               theDistance,
                                     vtkFloatingPointType theAngle[2])
{
  OrientedPlane* aPlane = OrientedPlane::New(theViewWindow);

  aPlane->myAngle[0] = theAngle[0];
  aPlane->myAngle[1] = theAngle[1];

  aPlane->SetOrientation(theOrientation);
  aPlane->SetDistance(theDistance);

  vtkFloatingPointType aNormal[3];
  vtkFloatingPointType aDir[2][3] = {{0, 0, 0}, {0, 0, 0}};
  {
    static double aCoeff = vtkMath::Pi()/180.0;

    vtkFloatingPointType anU[2] = {cos(aCoeff * theAngle[0]), cos(aCoeff * theAngle[1])};
    vtkFloatingPointType aV[2] = {sqrt(1.0 - anU[0]*anU[0]), sqrt(1.0 - anU[1]*anU[1])};
    aV[0] = theAngle[0] > 0? aV[0]: -aV[0];
    aV[1] = theAngle[1] > 0? aV[1]: -aV[1];

    switch (theOrientation) {
    case SMESH::XY:
      aDir[0][1] = anU[0];
      aDir[0][2] = aV[0];

      aDir[1][0] = anU[1];
      aDir[1][2] = aV[1];

      break;
    case SMESH::YZ:
      aDir[0][2] = anU[0];
      aDir[0][0] = aV[0];

      aDir[1][1] = anU[1];
      aDir[1][0] = aV[1];

      break;
    case SMESH::ZX:
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

  // ???
  theActor->SetPlaneParam(aNormal, theDistance, aPlane);

  vtkDataSet* aDataSet = theActor->GetInput();
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

  theActor->AddClippingPlane(aPlane);
  aPlane->Delete();
}

//=================================================================================
// used in SMESHGUI::restoreVisualParameters() to avoid
// declaration of OrientedPlane outside of SMESHGUI_ClippingDlg.cxx
//=================================================================================
void SMESHGUI_ClippingDlg::GetPlaneParam (SMESH_Actor*          theActor,
                                          int                   thePlaneIndex,
                                          SMESH::Orientation&   theOrientation,
                                          double&               theDistance,
                                          vtkFloatingPointType* theAngle)
{
  if (vtkPlane* aPln = theActor->GetClippingPlane(thePlaneIndex)) {
    if (OrientedPlane* aPlane = OrientedPlane::SafeDownCast(aPln)) {
      theOrientation = aPlane->GetOrientation();
      theDistance = aPlane->GetDistance();
      theAngle[0] = aPlane->myAngle[0];
      theAngle[1] = aPlane->myAngle[1];
    }
  }
}

//=================================================================================
// class    : SMESHGUI_ClippingDlg()
// purpose  :
//
//=================================================================================
SMESHGUI_ClippingDlg::SMESHGUI_ClippingDlg( SMESHGUI* theModule ):
  QDialog( SMESH::GetDesktop(theModule) ),
  mySelector(SMESH::GetViewWindow(theModule)->GetSelector()),
  mySelectionMgr(SMESH::GetSelectionMgr(theModule)),
  mySMESHGUI(theModule)
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle(tr("SMESH_CLIPPING_TITLE"));
  setSizeGripEnabled(true);

  QVBoxLayout* SMESHGUI_ClippingDlgLayout = new QVBoxLayout(this);
  SMESHGUI_ClippingDlgLayout->setSpacing(SPACING);
  SMESHGUI_ClippingDlgLayout->setMargin(MARGIN);

  // Controls for selecting, creating, deleting planes
  QGroupBox* GroupPlanes = new QGroupBox(tr("CLIP_PLANES"), this);
  QHBoxLayout* GroupPlanesLayout = new QHBoxLayout(GroupPlanes);
  GroupPlanesLayout->setSpacing(SPACING);
  GroupPlanesLayout->setMargin(MARGIN);

  ComboBoxPlanes = new QComboBox(GroupPlanes);

  buttonNew = new QPushButton(tr("SMESH_BUT_NEW"), GroupPlanes);

  buttonDelete = new QPushButton(tr("SMESH_BUT_DELETE"), GroupPlanes);

  GroupPlanesLayout->addWidget(ComboBoxPlanes);
  GroupPlanesLayout->addStretch();
  GroupPlanesLayout->addWidget(buttonNew);
  GroupPlanesLayout->addWidget(buttonDelete);

  // Controls for defining plane parameters
  QGroupBox* GroupParameters = new QGroupBox(tr("SMESH_PARAMETERS"), this);
  QGridLayout* GroupParametersLayout = new QGridLayout(GroupParameters);
  GroupParametersLayout->setSpacing(SPACING);
  GroupParametersLayout->setMargin(MARGIN);

  TextLabelOrientation = new QLabel(tr("SMESH_ORIENTATION"), GroupParameters);

  ComboBoxOrientation = new QComboBox(GroupParameters);

  TextLabelDistance = new QLabel(tr("SMESH_DISTANCE"), GroupParameters);

  SpinBoxDistance = new SMESHGUI_SpinBox(GroupParameters);

  TextLabelRot1 = new QLabel(tr("ROTATION_AROUND_X_Y2Z"), GroupParameters);

  SpinBoxRot1 = new SMESHGUI_SpinBox(GroupParameters);

  TextLabelRot2 = new QLabel(tr("ROTATION_AROUND_Y_X2Z"), GroupParameters);

  SpinBoxRot2 = new SMESHGUI_SpinBox(GroupParameters);

  PreviewCheckBox = new QCheckBox(tr("SHOW_PREVIEW"), GroupParameters);
  PreviewCheckBox->setChecked(true);

  AutoApplyCheckBox = new QCheckBox(tr("AUTO_APPLY"), GroupParameters);
  AutoApplyCheckBox->setChecked(false);

  GroupParametersLayout->addWidget(TextLabelOrientation, 0, 0);
  GroupParametersLayout->addWidget(ComboBoxOrientation,  0, 1);
  GroupParametersLayout->addWidget(TextLabelDistance,    1, 0);
  GroupParametersLayout->addWidget(SpinBoxDistance,      1, 1);
  GroupParametersLayout->addWidget(TextLabelRot1,        2, 0);
  GroupParametersLayout->addWidget(SpinBoxRot1,          2, 1);
  GroupParametersLayout->addWidget(TextLabelRot2,        3, 0);
  GroupParametersLayout->addWidget(SpinBoxRot2,          3, 1);
  GroupParametersLayout->addWidget(PreviewCheckBox,      4, 0);
  GroupParametersLayout->addWidget(AutoApplyCheckBox,    4, 1);

  // Controls for "Ok", "Apply" and "Close" button
  QGroupBox* GroupButtons = new QGroupBox(this);
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout(GroupButtons);
  GroupButtonsLayout->setSpacing(SPACING);
  GroupButtonsLayout->setMargin(MARGIN);
  
  buttonOk = new QPushButton(tr("SMESH_BUT_APPLY_AND_CLOSE"), GroupButtons);
  buttonOk->setAutoDefault(true);
  buttonOk->setDefault(true);
  buttonApply = new QPushButton(tr("SMESH_BUT_APPLY"), GroupButtons);
  buttonApply->setAutoDefault(true);
  buttonCancel = new QPushButton(tr("SMESH_BUT_CLOSE"), GroupButtons);
  buttonCancel->setAutoDefault(true);
  buttonHelp = new QPushButton(tr("SMESH_BUT_HELP"), GroupButtons);
  buttonHelp->setAutoDefault(true);
  GroupButtonsLayout->addWidget(buttonOk);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addWidget(buttonApply);
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget(buttonCancel);
  GroupButtonsLayout->addWidget(buttonHelp);

  SMESHGUI_ClippingDlgLayout->addWidget(GroupPlanes);
  SMESHGUI_ClippingDlgLayout->addWidget(GroupParameters);
  SMESHGUI_ClippingDlgLayout->addWidget(GroupButtons);

  // Initial state
  SpinBoxDistance->RangeStepAndValidator(0.0, 1.0, 0.01, "length_precision" );
  SpinBoxRot1->RangeStepAndValidator(-180.0, 180.0, 1, "angle_precision" );
  SpinBoxRot2->RangeStepAndValidator(-180.0, 180.0, 1, "angle_precision" );

  ComboBoxOrientation->addItem(tr("ALONG_XY"));
  ComboBoxOrientation->addItem(tr("ALONG_YZ"));
  ComboBoxOrientation->addItem(tr("ALONG_ZX"));

  SpinBoxDistance->SetValue(0.5);

  myActor = 0;
  myIsSelectPlane = false;
  onSelectionChanged();

  myHelpFileName = "clipping_page.html";

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
  connect(mySMESHGUI, SIGNAL (SignalCloseAllDialogs()), this, SLOT(ClickOnCancel()));
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
  if (mySMESHGUI)
    if (SVTK_ViewWindow* aViewWindow = SMESH::GetViewWindow(mySMESHGUI))
      SMESH::RenderViewWindow(aViewWindow);
}

double SMESHGUI_ClippingDlg::getDistance() const
{
  return SpinBoxDistance->GetValue();
}

void SMESHGUI_ClippingDlg::setDistance( const double theDistance )
{
  SpinBoxDistance->SetValue( theDistance );
}

double SMESHGUI_ClippingDlg::getRotation1() const
{
  return SpinBoxRot1->GetValue();
}

double SMESHGUI_ClippingDlg::getRotation2() const
{
  return SpinBoxRot2->GetValue();
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
    for ( ; anIter != myPlanes.end(); anIter++) {
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
        for ( ; anId < anEnd; anId++) {
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
    ComboBoxOrientation->setCurrentIndex(0);
    onSelectOrientation(0);
    break;
  case SMESH::YZ:
    ComboBoxOrientation->setCurrentIndex(1);
    onSelectOrientation(1);
    break;
  case SMESH::ZX:
    ComboBoxOrientation->setCurrentIndex(2);
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

  int aPlaneIndex = ComboBoxPlanes->currentIndex();

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
    TextLabelRot1->setText(tr("ROTATION_AROUND_X_Y2Z"));
    TextLabelRot2->setText(tr("ROTATION_AROUND_Y_X2Z"));
  }
  else if (theItem == 1) {
    TextLabelRot1->setText(tr("ROTATION_AROUND_Y_Z2X"));
    TextLabelRot2->setText(tr("ROTATION_AROUND_Z_Y2X"));
  }
  else if (theItem == 2) {
    TextLabelRot1->setText(tr("ROTATION_AROUND_Z_X2Y"));
    TextLabelRot2->setText(tr("ROTATION_AROUND_X_Z2Y"));
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
    aName = QString(tr("PLANE_NUM")).arg(i);
    ComboBoxPlanes->addItem(aName);
  }

  int aPos = ComboBoxPlanes->count() - 1;
  ComboBoxPlanes->setCurrentIndex(aPos);

  bool anIsControlsEnable = (aPos >= 0);
  if (anIsControlsEnable) {
    onSelectPlane(aPos);
  } else {
    ComboBoxPlanes->addItem(tr("NO_PLANES"));
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

  int aCurPlaneIndex = ComboBoxPlanes->currentIndex();

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

    switch (ComboBoxOrientation->currentIndex()) {
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

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_ClippingDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}
