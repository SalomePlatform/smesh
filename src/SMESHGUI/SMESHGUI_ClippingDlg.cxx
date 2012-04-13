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
#include <SUIT_ViewManager.h>

#include <SALOME_ListIO.hxx>

#include <SalomeApp_Study.h>

#include <LightApp_Application.h>

#include <VTKViewer_Algorithm.h>

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
#include <QListWidget>

// VTK includes
#include <vtkMath.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkPlaneSource.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

#define SPACING 6
#define MARGIN  11

//=================================================================================
// class    : OrientedPlane
// purpose  :
//=================================================================================
SMESH::OrientedPlane* SMESH::OrientedPlane::New()
{
  return new OrientedPlane();
}

SMESH::OrientedPlane* SMESH::OrientedPlane::New(SVTK_ViewWindow* theViewWindow)
{
  return new OrientedPlane(theViewWindow);
}

void SMESH::OrientedPlane::ShallowCopy(SMESH::OrientedPlane* theOrientedPlane)
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

SMESH::OrientedPlane::OrientedPlane(SVTK_ViewWindow* theViewWindow):
  myViewWindow(theViewWindow),
  myOrientation(SMESH::XY),
  myDistance(0.5)
{
  Init();
  myViewWindow->AddActor(myActor, false, false); // don't adjust actors
}

SMESH::OrientedPlane::OrientedPlane():
  myOrientation(SMESH::XY),
  myViewWindow(NULL),
  myDistance(0.5)
{
  Init();
}

void SMESH::OrientedPlane::Init()
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

SMESH::OrientedPlane::~OrientedPlane()
{
  if (myViewWindow)
    myViewWindow->RemoveActor(myActor);
  myActor->Delete();
    
  myMapper->RemoveAllInputs();
  myMapper->Delete();

  // commented: porting to vtk 5.0
  //    myPlaneSource->UnRegisterAllOutputs();
  myPlaneSource->Delete();
}

//=================================================================================
// class    : ActorItem
// purpose  :
//=================================================================================
class ActorItem : public QListWidgetItem
{
public:
  ActorItem( SMESH_Actor* theActor, const QString& theName, QListWidget* theListWidget ) :
    QListWidgetItem( theName, theListWidget ),
    myActor( theActor ) {}

  SMESH_Actor* getActor() const { return myActor; }

private:
  SMESH_Actor* myActor;
};

//=================================================================================
// class    : TSetVisibility
// purpose  :
//=================================================================================
struct TSetVisibility {
  TSetVisibility(int theIsVisible): myIsVisible(theIsVisible){}
  void operator()(SMESH::TPlaneData& thePlaneData){
    bool anIsEmpty = thePlaneData.ActorList.empty();
    thePlaneData.Plane.GetPointer()->myActor->SetVisibility(myIsVisible && !anIsEmpty);
  }
  int myIsVisible;
};

//=================================================================================
// used in SMESHGUI::restoreVisualParameters() to avoid
// declaration of OrientedPlane outside of SMESHGUI_ClippingDlg.cxx
//=================================================================================
SMESH::OrientedPlane* SMESHGUI_ClippingDlg::AddPlane (SMESH::TActorList          theActorList,
                                                      SVTK_ViewWindow*           theViewWindow,
                                                      SMESH::Orientation         theOrientation,
                                                      double                     theDistance,
                                                      const vtkFloatingPointType theAngle[2])
{
  SMESH::OrientedPlane* aPlane = SMESH::OrientedPlane::New(theViewWindow);

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

  vtkFloatingPointType aBounds[6];
  vtkFloatingPointType anOrigin[3];

  bool anIsOk = false;
  if( theActorList.empty() ) {
    // to support planes with empty actor list we should create
    // a nullified plane that will be initialized later 
    anOrigin[0] = anOrigin[1] = anOrigin[2] = 0;
    aBounds[0] = aBounds[2] = aBounds[4] = 0;
    aBounds[1] = aBounds[3] = aBounds[5] = 0;
    anIsOk = true;
  }
  else
    anIsOk = SMESH::ComputeClippingPlaneParameters( theActorList,
                                                    aNormal,
                                                    theDistance,
                                                    aBounds,
                                                    anOrigin );
  if( !anIsOk )
    return NULL;

  aPlane->SetNormal( aNormal );
  aPlane->SetOrigin( anOrigin );

  vtkFloatingPointType aPnt[3] = { ( aBounds[0] + aBounds[1] ) / 2.,
                                   ( aBounds[2] + aBounds[3] ) / 2.,
                                   ( aBounds[4] + aBounds[5] ) / 2. };

  vtkFloatingPointType aDel = pow( pow( aBounds[1] - aBounds[0], 2 ) +
                                   pow( aBounds[3] - aBounds[2], 2 ) +
                                   pow( aBounds[5] - aBounds[4], 2 ), 0.5 );

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

  SMESH::TActorList::iterator anIter = theActorList.begin();
  for ( ; anIter != theActorList.end(); anIter++ )
    if( vtkActor* aVTKActor = *anIter )
      if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( aVTKActor ) )
        anActor->AddClippingPlane( aPlane );

  return aPlane;
}

//=================================================================================
// class    : SMESHGUI_ClippingDlg()
// purpose  :
//
//=================================================================================
SMESHGUI_ClippingDlg::SMESHGUI_ClippingDlg( SMESHGUI* theModule, SVTK_ViewWindow* theViewWindow ):
  QDialog( SMESH::GetDesktop(theModule) ),
  mySMESHGUI(theModule),
  myViewWindow(theViewWindow)
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
  QGridLayout* GroupPlanesLayout = new QGridLayout(GroupPlanes);
  GroupPlanesLayout->setSpacing(SPACING);
  GroupPlanesLayout->setMargin(MARGIN);

  ComboBoxPlanes = new QComboBox(GroupPlanes);

  buttonNew = new QPushButton(tr("SMESH_BUT_NEW"), GroupPlanes);

  buttonDelete = new QPushButton(tr("SMESH_BUT_DELETE"), GroupPlanes);

  QLabel* aLabel = new QLabel(tr("MESHES_SUBMESHES_GROUPS"), GroupPlanes);

  ActorList = new QListWidget(GroupPlanes);
  ActorList->setSelectionMode(QAbstractItemView::SingleSelection);

  SelectAllCheckBox = new QCheckBox(tr("SELECT_ALL"), GroupPlanes);

  GroupPlanesLayout->addWidget(ComboBoxPlanes,    0, 0);
  GroupPlanesLayout->addWidget(new QWidget(),     0, 1);
  GroupPlanesLayout->addWidget(buttonNew,         0, 2);
  GroupPlanesLayout->addWidget(buttonDelete,      0, 3);
  GroupPlanesLayout->addWidget(aLabel,            1, 0, 1, 4);
  GroupPlanesLayout->addWidget(ActorList,         2, 0, 1, 4);
  GroupPlanesLayout->addWidget(SelectAllCheckBox, 3, 0, 1, 4);
  GroupPlanesLayout->setColumnStretch( 1, 1 );

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

  myIsSelectPlane = false;

  initializePlaneData();
  synchronize();

  myHelpFileName = "clipping_page.html";

  // signals and slots connections :
  connect(ComboBoxPlanes, SIGNAL(activated(int)), this, SLOT(onSelectPlane(int)));
  connect(buttonNew, SIGNAL(clicked()), this, SLOT(ClickOnNew()));
  connect(buttonDelete, SIGNAL(clicked()), this, SLOT(ClickOnDelete()));
  connect(ActorList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onActorItemChanged(QListWidgetItem*)));
  connect(SelectAllCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onSelectAll(int)));
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
  std::for_each(myPlanes.begin(),myPlanes.end(),TSetVisibility(false));
  if (myViewWindow)
    SMESH::RenderViewWindow(myViewWindow);
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
  if (myViewWindow) {
    SUIT_OverrideCursor wc;
    
    QWidget *aCurrWid = this->focusWidget();
    aCurrWid->clearFocus();
    aCurrWid->setFocus();

    SMESHGUI_ClippingPlaneInfoMap& aClippingPlaneInfoMap = mySMESHGUI->getClippingPlaneInfoMap();
    SMESHGUI_ClippingPlaneInfoList& aClippingPlaneInfoList = aClippingPlaneInfoMap[ myViewWindow->getViewManager() ];

    // clean memory allocated for planes
    SMESHGUI_ClippingPlaneInfoList::iterator anIter1 = aClippingPlaneInfoList.begin();
    for( ; anIter1 != aClippingPlaneInfoList.end(); anIter1++ )
      if( SMESH::OrientedPlane* aPlane = (*anIter1).Plane )
        aPlane->Delete();

    aClippingPlaneInfoList.clear();

    VTK::ActorCollectionCopy aCopy( myViewWindow->getRenderer()->GetActors() );
    vtkActorCollection* anAllActors = aCopy.GetActors();
    anAllActors->InitTraversal();
    while( vtkActor* aVTKActor = anAllActors->GetNextActor() )
      if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( aVTKActor ) )
        anActor->RemoveAllClippingPlanes();

    SMESH::TPlaneDataVector::iterator anIter2 = myPlanes.begin();
    for( ; anIter2 != myPlanes.end(); anIter2++ ) {
      SMESH::TPlaneData aPlaneData = *anIter2;
      SMESH::TPlane aPlane = aPlaneData.Plane;
      SMESH::TActorList anActorList = aPlaneData.ActorList;

      // the check is disabled to support planes with empty actor list
      //if( anActorList.empty() )
      //  continue;

      SMESH::OrientedPlane* anOrientedPlane = SMESH::OrientedPlane::New(myViewWindow);
      anOrientedPlane->ShallowCopy(aPlane.GetPointer());

      SMESH::TActorList::iterator anIter3 = anActorList.begin();
      for( ; anIter3 != anActorList.end(); anIter3++ )
        if( vtkActor* aVTKActor = *anIter3 )
          if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( aVTKActor ) )
            anActor->AddClippingPlane(anOrientedPlane);

      SMESH::ClippingPlaneInfo aClippingPlaneInfo;
      aClippingPlaneInfo.Plane = anOrientedPlane;
      aClippingPlaneInfo.ActorList = anActorList;

      aClippingPlaneInfoList.push_back( aClippingPlaneInfo );
    }

    SMESH::RenderViewWindow( myViewWindow );
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

//=======================================================================
// function : onSelectPlane()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::onSelectPlane (int theIndex)
{
  if (myPlanes.empty())
    return;

  SMESH::TPlaneData aPlaneData = myPlanes[theIndex];
  SMESH::OrientedPlane* aPlane = aPlaneData.Plane.GetPointer();

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

  // Actors
  bool anIsBlocked = ActorList->blockSignals( true );
  updateActorList();
  ActorList->blockSignals( anIsBlocked );
}

//=======================================================================
// function : ClickOnNew()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::ClickOnNew()
{
  if(myViewWindow){
    SMESH::OrientedPlane* aPlane = SMESH::OrientedPlane::New(myViewWindow);
    SMESH::TPlane aTPlane(aPlane);

    SMESH::TActorList anActorList;
    VTK::ActorCollectionCopy aCopy( myViewWindow->getRenderer()->GetActors() );
    vtkActorCollection* anAllActors = aCopy.GetActors();
    anAllActors->InitTraversal();
    while( vtkActor* aVTKActor = anAllActors->GetNextActor() )
      if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( aVTKActor ) )
        anActorList.push_back( anActor );

    SMESH::TPlaneData aPlaneData(aTPlane, anActorList);

    myPlanes.push_back(aPlaneData);

    if (PreviewCheckBox->isChecked())
      aTPlane->myActor->VisibilityOn();

    bool anIsBlocked = ActorList->blockSignals( true );

    synchronize();
    SetCurrentPlaneParam();

    ActorList->blockSignals( anIsBlocked );
  }
}

//=======================================================================
// function : ClickOnDelete()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::ClickOnDelete()
{
  if (myPlanes.empty())
    return;

  int aPlaneIndex = ComboBoxPlanes->currentIndex();

  SMESH::TPlaneDataVector::iterator anIter = myPlanes.begin() + aPlaneIndex;
  SMESH::TPlaneData aPlaneData = *anIter;
  aPlaneData.Plane.GetPointer()->myActor->SetVisibility(false);
  myPlanes.erase(anIter);

  if(AutoApplyCheckBox->isChecked())
    ClickOnApply();

  synchronize();
  SMESH::RenderViewWindow( myViewWindow );
}

//=======================================================================
// function : updateActorItem()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::updateActorItem( QListWidgetItem* theItem,
                                            bool theUpdateSelectAll,
                                            bool theUpdateClippingPlaneMap )
{
  // update Select All check box
  if( theUpdateSelectAll ) {
    int aNbItems = ActorList->count(), aNbChecked = 0;
    for( int i = 0; i < aNbItems; i++ )
      if( QListWidgetItem* anItem = ActorList->item( i ) )
        if( anItem->checkState() == Qt::Checked )
          aNbChecked++;

    Qt::CheckState aCheckState = Qt::Unchecked;
    if( aNbChecked == aNbItems )
      aCheckState = Qt::Checked;
    else if( aNbChecked > 0 )
      aCheckState = Qt::PartiallyChecked;

    bool anIsBlocked = SelectAllCheckBox->blockSignals( true );
    SelectAllCheckBox->setCheckState( aCheckState );
    SelectAllCheckBox->blockSignals( anIsBlocked );
  }

  // update clipping plane map
  if( theUpdateClippingPlaneMap ) {
    int aCurPlaneIndex = ComboBoxPlanes->currentIndex();
    if( ActorItem* anItem = dynamic_cast<ActorItem*>( theItem ) ) {
      if( SMESH_Actor* anActor = anItem->getActor() ) {
        SMESH::TPlaneData& aPlaneData = myPlanes[ aCurPlaneIndex ];
        SMESH::TActorList& anActorList = aPlaneData.ActorList;
        bool anIsPushed = false;
        SMESH::TActorList::iterator anIter = anActorList.begin();
        for ( ; anIter != anActorList.end(); anIter++ ) {
          if( anActor == *anIter ) {
            anIsPushed = true;
            break;
          }
        }
        if( theItem->checkState() == Qt::Checked && !anIsPushed )
          anActorList.push_back( anActor );
        else if( theItem->checkState() == Qt::Unchecked && anIsPushed )
          anActorList.remove( anActor );
      }
    }
  }
}

//=======================================================================
// function : onActorItemChanged()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::onActorItemChanged( QListWidgetItem* theItem )
{
  updateActorItem( theItem, true, true );
  SetCurrentPlaneParam();
}

//=======================================================================
// function : onSelectAll()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::onSelectAll( int theState )
{
  if( theState == Qt::PartiallyChecked ) {
    SelectAllCheckBox->setCheckState( Qt::Checked );
    return;
  }

  bool anIsBlocked = ActorList->blockSignals( true );
  for( int i = 0, n = ActorList->count(); i < n; i++ ) {
    if( QListWidgetItem* anItem = ActorList->item( i ) ) {
      anItem->setCheckState( theState == Qt::Checked ? Qt::Checked : Qt::Unchecked );
      updateActorItem( anItem, false, true );
    }
  }
  SelectAllCheckBox->setTristate( false );
  ActorList->blockSignals( anIsBlocked );
  SetCurrentPlaneParam();
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
// function : synchronize()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::synchronize()
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
    updateActorList();
  } else {
    ComboBoxPlanes->addItem(tr("NO_PLANES"));
    ActorList->clear();
    SpinBoxRot1->SetValue(0.0);
    SpinBoxRot2->SetValue(0.0);
    SpinBoxDistance->SetValue(0.5);
  }

  ActorList->setEnabled(anIsControlsEnable);
  SelectAllCheckBox->setEnabled(anIsControlsEnable);
  buttonDelete->setEnabled(anIsControlsEnable);
  // the following 3 controls should be enabled
  //buttonApply->setEnabled(anIsControlsEnable);
  //PreviewCheckBox->setEnabled(anIsControlsEnable);
  //AutoApplyCheckBox->setEnabled(anIsControlsEnable);
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

  SMESH::TPlaneData aPlaneData = myPlanes[aCurPlaneIndex];
  SMESH::OrientedPlane* aPlane = aPlaneData.Plane.GetPointer();

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

  SMESH::TActorList anActorList = aPlaneData.ActorList;

  vtkFloatingPointType aBounds[6];
  vtkFloatingPointType anOrigin[3];
  bool anIsOk = SMESH::ComputeClippingPlaneParameters( anActorList,
                                                       aNormal,
                                                       getDistance(),
                                                       aBounds,
                                                       anOrigin );

  aPlane->myActor->SetVisibility( anIsOk && PreviewCheckBox->isChecked() );

  if( anIsOk ) {
    aPlane->SetNormal( aNormal );
    aPlane->SetOrigin( anOrigin );

    vtkFloatingPointType aPnt[3] = { ( aBounds[0] + aBounds[1] ) / 2.,
                                     ( aBounds[2] + aBounds[3] ) / 2.,
                                     ( aBounds[4] + aBounds[5] ) / 2. };

    vtkFloatingPointType aDel = pow( pow( aBounds[1] - aBounds[0], 2 ) +
                                     pow( aBounds[3] - aBounds[2], 2 ) +
                                     pow( aBounds[5] - aBounds[4], 2 ), 0.5 );

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
  }

  if(AutoApplyCheckBox->isChecked())
    ClickOnApply();

  SMESH::RenderViewWindow( myViewWindow );
}

//=======================================================================
// function : OnPreviewToggle()
// purpose  :
//=======================================================================
void SMESHGUI_ClippingDlg::OnPreviewToggle (bool theIsToggled)
{
  std::for_each(myPlanes.begin(),myPlanes.end(),TSetVisibility(theIsToggled));
  SMESH::RenderViewWindow( myViewWindow );
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

//=================================================================================
// function : initializePlaneData()
// purpose  :
//=================================================================================
void SMESHGUI_ClippingDlg::initializePlaneData()
{
  const SMESHGUI_ClippingPlaneInfoMap& aClippingPlaneInfoMap = mySMESHGUI->getClippingPlaneInfoMap();
  SMESHGUI_ClippingPlaneInfoMap::const_iterator anIter1 = aClippingPlaneInfoMap.find( myViewWindow->getViewManager() );
  if( anIter1 != aClippingPlaneInfoMap.end() ) {
    const SMESHGUI_ClippingPlaneInfoList& aClippingPlaneInfoList = anIter1->second;
    SMESHGUI_ClippingPlaneInfoList::const_iterator anIter2 = aClippingPlaneInfoList.begin();
    for( ; anIter2 != aClippingPlaneInfoList.end(); anIter2++ ) {
      const SMESH::ClippingPlaneInfo& aClippingPlaneInfo = *anIter2;
      SMESH::TPlane aTPlane( aClippingPlaneInfo.Plane );
      SMESH::TPlaneData aPlaneData( aTPlane, aClippingPlaneInfo.ActorList );
      myPlanes.push_back( aPlaneData );
    }
  }
  std::for_each( myPlanes.begin(),myPlanes.end(), TSetVisibility( PreviewCheckBox->isChecked() ) );
}

//=================================================================================
// function : updateActorList()
// purpose  :
//=================================================================================
void SMESHGUI_ClippingDlg::updateActorList()
{
  ActorList->clear();

  SalomeApp_Study* anAppStudy = SMESHGUI::activeStudy();
  if( !anAppStudy )
    return;

  _PTR(Study) aStudy = anAppStudy->studyDS();
  if( !aStudy )
    return;

  if( !myViewWindow )
    return;

  int aCurPlaneIndex = ComboBoxPlanes->currentIndex();
  const SMESH::TPlaneData& aPlaneData = myPlanes[ aCurPlaneIndex ];
  const SMESH::TActorList& anActorList = aPlaneData.ActorList;

  VTK::ActorCollectionCopy aCopy( myViewWindow->getRenderer()->GetActors() );
  vtkActorCollection* anAllActors = aCopy.GetActors();
  anAllActors->InitTraversal();
  while( vtkActor* aVTKActor = anAllActors->GetNextActor() ) {
    if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( aVTKActor ) ) {
      if( anActor->hasIO() ) {
        Handle(SALOME_InteractiveObject) anIO = anActor->getIO();
        if( _PTR(SObject) aSObj = aStudy->FindObjectID( anIO->getEntry() ) ) {
          bool anIsChecked = false;
          SMESH::TActorList::const_iterator anIter = anActorList.begin();
          for ( ; anIter != anActorList.end(); anIter++ ) {
            if( vtkActor* aVTKActorRef = *anIter ) {
              if( SMESH_Actor* anActorRef = SMESH_Actor::SafeDownCast( aVTKActorRef ) ) {
                if( anActorRef == anActor ) {
                  anIsChecked = true;
                  break;
                }
              }
            }
          }
          QString aName = QString( aSObj->GetName().c_str() );
          QListWidgetItem* anItem = new ActorItem( anActor, aName, ActorList );
          anItem->setCheckState( anIsChecked ? Qt::Checked : Qt::Unchecked );
          updateActorItem( anItem, true, false );
        }
      }
    }
  }
}

//=================================================================================
// function : getCurrentActors()
// purpose  :
//=================================================================================
SMESH::TActorList SMESHGUI_ClippingDlg::getCurrentActors()
{
  SMESH::TActorList anActorList;
  for( int i = 0, n = ActorList->count(); i < n; i++ )
    if( ActorItem* anItem = dynamic_cast<ActorItem*>( ActorList->item( i ) ) )
      if( anItem->checkState() == Qt::Checked )
        if( SMESH_Actor* anActor = anItem->getActor() )
          anActorList.push_back( anActor );
  return anActorList;
}

//=================================================================================
// function : dumpPlaneData()
// purpose  :
//=================================================================================
void SMESHGUI_ClippingDlg::dumpPlaneData() const
{
  printf( "----------- Plane Data -----------\n" );
  int anId = 1;
  SMESH::TPlaneDataVector::const_iterator anIter1 = myPlanes.begin();
  for ( ; anIter1 != myPlanes.end(); anIter1++, anId++ ) {
    SMESH::TPlaneData aPlaneData = *anIter1;
    SMESH::TPlane aPlane = aPlaneData.Plane;
    vtkFloatingPointType* aNormal = aPlane->GetNormal();
    vtkFloatingPointType* anOrigin = aPlane->GetOrigin();
    printf( "Plane N%d:\n", anId );
    printf( "  Normal = ( %f, %f, %f )\n", aNormal[0], aNormal[1], aNormal[2] );
    printf( "  Origin = ( %f, %f, %f )\n", anOrigin[0], anOrigin[1], anOrigin[2] );

    SMESH::TActorList anActorList = aPlaneData.ActorList;
    SMESH::TActorList::const_iterator anIter2 = anActorList.begin();
    for ( ; anIter2 != anActorList.end(); anIter2++ ) {
      if( vtkActor* aVTKActor = *anIter2 ) {
        if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( aVTKActor ) )
          printf( "  - Actor: '%s'\n", anActor->getName() );
      }
      else
        printf( "  - Actor: NULL\n");
    }
  }
  printf( "----------------------------------\n" );
}
