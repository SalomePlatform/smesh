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

#include <QtxDoubleSpinSlider.h>
#include <QtxIntSpinSlider.h>

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
#include <SVTK_RenderWindowInteractor.h>

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
#include <QStackedLayout>
#include <QSlider>
#include <QMenu>

// VTK includes
#include <vtkMath.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkPlaneSource.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkCallbackCommand.h>
#include <vtkImplicitPlaneWidget.h>

#define SPACING 6
#define MARGIN  11
#define SIZEFACTOR 1.1

/*!
  Create new object of class OrientedPlane
 */
SMESH::OrientedPlane* SMESH::OrientedPlane::New()
{
  return new OrientedPlane();
}

/*!
  Create new object of class OrientedPlane
 */
SMESH::OrientedPlane* SMESH::OrientedPlane::New( SVTK_ViewWindow* theViewWindow )
{
  return new OrientedPlane( theViewWindow );
}

/*!
  Copy the object of class OrientedPlane
 */
void SMESH::OrientedPlane::ShallowCopy( SMESH::OrientedPlane* theOrientedPlane )
{
  SetNormal( theOrientedPlane->GetNormal() );
  SetOrigin( theOrientedPlane->GetOrigin() );

  myRelativeOrientation = theOrientedPlane->GetOrientation();
  myDistance = theOrientedPlane->GetDistance();

  IsOpenGLClipping = theOrientedPlane->IsOpenGLClipping;

  myAngle[0] = theOrientedPlane->myAngle[0];
  myAngle[1] = theOrientedPlane->myAngle[1];

  myAbsoluteOrientation = theOrientedPlane->myAbsoluteOrientation;
  X = theOrientedPlane->X;
  Y = theOrientedPlane->Y;
  Z = theOrientedPlane->Z;
  Dx = theOrientedPlane->Dx;
  Dy = theOrientedPlane->Dy;
  Dz = theOrientedPlane->Dz;

  PlaneMode = theOrientedPlane->PlaneMode;

  myPlaneSource->SetNormal( theOrientedPlane->myPlaneSource->GetNormal() );
  myPlaneSource->SetOrigin( theOrientedPlane->myPlaneSource->GetOrigin() );
  myPlaneSource->SetPoint1( theOrientedPlane->myPlaneSource->GetPoint1() );
  myPlaneSource->SetPoint2( theOrientedPlane->myPlaneSource->GetPoint2() );
  myPlaneSource->Update();
}

/*!
  Invert current clipping plane in contrary direction
 */
SMESH::OrientedPlane* SMESH::OrientedPlane::InvertPlane()
{
  OrientedPlane* aPlane = new OrientedPlane();
  aPlane->ShallowCopy( this );
  double* aNormal = aPlane->GetNormal();
  for( int i=0; i<3; i++ )
    aNormal[i] = -aNormal[i];
  aPlane->SetNormal( aNormal );
  return aPlane;
}

/*!
  Constructor of class OrientedPlane
 */
SMESH::OrientedPlane::OrientedPlane(SVTK_ViewWindow* theViewWindow):
  myViewWindow(theViewWindow)
{
  Init();
  myViewWindow->AddActor(myActor, false, false); // don't adjust actors
}

/*!
  Constructor of class OrientedPlane
 */
SMESH::OrientedPlane::OrientedPlane():
  myViewWindow(NULL)
{
  Init();
}

/*!
  Initialize parameters of class OrientedPlane
 */
void SMESH::OrientedPlane::Init()
{
  myPlaneSource = vtkPlaneSource::New();

  PlaneMode = SMESH::Absolute;
  X = Y = Z = 0.0;
  Dx = Dy = Dz = 1.0;
  myAbsoluteOrientation = 0; // CUSTOM
  myRelativeOrientation = SMESH::XY;
  myDistance = 0.5;
  myAngle[0] = myAngle[1] = 0.0;
  IsInvert = false;
  IsOpenGLClipping = false;

  // Create and display actor
  myMapper = vtkDataSetMapper::New();
  myMapper->SetInputConnection(myPlaneSource->GetOutputPort());

  myActor = SALOME_Actor::New();
  myActor->VisibilityOff();
  myActor->PickableOff();
  myActor->SetInfinitive(true);
  myActor->SetMapper(myMapper);

  QColor ffc, bfc;
  int delta;
  SMESH::GetColor( "SMESH", "fill_color", ffc, delta, "255, 170, 0|-100" ) ;
 
  vtkProperty* aProp = vtkProperty::New();
  SMESH::GetColor( "SMESH", "fill_color", ffc, delta, "255, 170, 0|-100" ) ;
  aProp->SetColor(ffc.red() / 255. , ffc.green() / 255. , ffc.blue() / 255.);
  aProp->SetOpacity(0.75);
  myActor->SetProperty(aProp);
  aProp->Delete();

  vtkProperty* aBackProp = vtkProperty::New();
  bfc = Qtx::mainColorToSecondary(ffc, delta);
  aBackProp->SetColor( bfc.red() / 255. , bfc.green() / 255. , bfc.blue() / 255.);
  aBackProp->SetOpacity(0.75);
  myActor->SetBackfaceProperty(aBackProp);
  aBackProp->Delete();
}

/*!
  Destructor of class OrientedPlane
 */
SMESH::OrientedPlane::~OrientedPlane()
{
  if (myViewWindow)
    myViewWindow->RemoveActor(myActor);
  myActor->Delete();
    
  myMapper->RemoveAllInputs();
  myMapper->Delete();

  // commented: porting to vtk 5.0
  // myPlaneSource->UnRegisterAllOutputs();
  myPlaneSource->Delete();
}

namespace
{
  /*!
    Definition of class ActorItem
  */
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

  /*!
    Definition of class TSetVisibility
  */
  struct TSetVisibility {
    // Set visibility of cutting plane
    TSetVisibility(int theIsVisible): myIsVisible(theIsVisible){}
    void operator()(SMESH::TPlaneData& thePlaneData){
      bool anIsEmpty = thePlaneData.ActorList.empty();
      thePlaneData.Plane.GetPointer()->myActor->SetVisibility(myIsVisible && !anIsEmpty);
    }
    int myIsVisible;
  };
}

/*********************************************************************************
 *********************      class SMESHGUI_ClippingDlg      *********************
 *********************************************************************************/

/*!
  Constructor
*/
SMESHGUI_ClippingDlg::SMESHGUI_ClippingDlg( SMESHGUI* theModule, SVTK_ViewWindow* theViewWindow ):
  QDialog( SMESH::GetDesktop(theModule) ),
  mySMESHGUI(theModule),
  myViewWindow(theViewWindow)
{
  setModal( false );
  setAttribute( Qt::WA_DeleteOnClose, true );
  setWindowTitle(tr("SMESH_CLIPPING_TITLE"));
  setSizeGripEnabled(true);

  myPreviewWidget = vtkImplicitPlaneWidget::New();
  myCallback = vtkCallbackCommand::New();
  myCallback->SetClientData( this );
  myCallback->SetCallback( SMESHGUI_ClippingDlg::ProcessEvents );
  myPreviewWidget = createPreviewWidget();

  myIsPreviewMoved = false;

  QVBoxLayout* SMESHGUI_ClippingDlgLayout = new QVBoxLayout(this);
  SMESHGUI_ClippingDlgLayout->setSpacing(SPACING);
  SMESHGUI_ClippingDlgLayout->setMargin(MARGIN);

  // Controls for selecting, creating, deleting planes
  QGroupBox* GroupPlanes = new QGroupBox(tr("CLIP_PLANES"), this);
  QGridLayout* GroupPlanesLayout = new QGridLayout(GroupPlanes);
  GroupPlanesLayout->setSpacing(SPACING);
  GroupPlanesLayout->setMargin(MARGIN);

  ComboBoxPlanes = new QComboBox(GroupPlanes);

  isOpenGLClipping = new QCheckBox( GroupPlanes );
  isOpenGLClipping->setText( tr( "IS_OPENGL_CLIPPING" ) );

  buttonNew = new QPushButton(tr("SMESH_BUT_NEW"), GroupPlanes);

  MenuMode = new QMenu( "MenuMode", buttonNew );
  MenuMode->addAction( tr( "ABSOLUTE" ), this, SLOT( onModeAbsolute() ) );
  MenuMode->addAction( tr( "RELATIVE" ), this, SLOT( onModeRelative() ) );
  buttonNew->setMenu( MenuMode );
  CurrentMode = SMESH::Absolute;

  buttonDelete = new QPushButton(tr("SMESH_BUT_DELETE"), GroupPlanes);

  QLabel* aLabel = new QLabel(tr("MESHES_SUBMESHES_GROUPS"), GroupPlanes);

  ActorList = new QListWidget(GroupPlanes);
  ActorList->setSelectionMode(QAbstractItemView::SingleSelection);
  
  SelectAllCheckBox = new QCheckBox(tr("SELECT_ALL"), GroupPlanes);

  GroupPlanesLayout->addWidget(ComboBoxPlanes,    0, 0);
  GroupPlanesLayout->addWidget(isOpenGLClipping,  0, 1);
  GroupPlanesLayout->addWidget(new QWidget(),     0, 2);
  GroupPlanesLayout->addWidget(buttonNew,         0, 3);
  GroupPlanesLayout->addWidget(buttonDelete,      0, 4);
  GroupPlanesLayout->addWidget(aLabel,            1, 0, 1, 5);
  GroupPlanesLayout->addWidget(ActorList,         2, 0, 1, 5);
  GroupPlanesLayout->addWidget(SelectAllCheckBox, 3, 0, 1, 5);
  GroupPlanesLayout->setColumnStretch( 1, 1 );

  ModeStackedLayout = new QStackedLayout();

  // Controls for defining plane parameters
  /**********************   Mode Absolute   **********************/
  /* Controls for absolute mode of clipping plane:
     X, Y, Z - coordinates of the intersection of cutting plane and the three axes
     Dx, Dy, Dz - components of normal to the cutting plane
     Orientation - direction of cutting plane
  */
  const double min = -1e+7;
  const double max =  1e+7;
  const double step = 5;
  const int precision = -7;

  // Croup Point
  QGroupBox* GroupAbsolutePoint = new QGroupBox( this );
  GroupAbsolutePoint->setObjectName( "GroupPoint" );
  GroupAbsolutePoint->setTitle( tr("BASE_POINT") );
  QGridLayout* GroupPointLayout = new QGridLayout( GroupAbsolutePoint );
  GroupPointLayout->setAlignment( Qt::AlignTop );
  GroupPointLayout->setSpacing( 6 ); GroupPointLayout->setMargin( 11 );

  TextLabelX = new QLabel( GroupAbsolutePoint );
  TextLabelX->setObjectName( "TextLabelX" );
  TextLabelX->setText( tr("X:") );
  GroupPointLayout->addWidget( TextLabelX, 0, 0 );

  SpinBox_X = new QtxDoubleSpinBox( min, max, step, GroupAbsolutePoint );
  SpinBox_X->setObjectName("SpinBox_X" );
  SpinBox_X->setPrecision( precision );
  GroupPointLayout->addWidget( SpinBox_X, 0, 1 );

  TextLabelY = new QLabel( GroupAbsolutePoint );
  TextLabelY->setObjectName( "TextLabelY" );
  TextLabelY->setText( tr("Y:") );
  GroupPointLayout->addWidget( TextLabelY, 0, 2 );

  SpinBox_Y = new QtxDoubleSpinBox( min, max, step, GroupAbsolutePoint );
  SpinBox_Y->setObjectName("SpinBox_Y" );
  SpinBox_Y->setPrecision( precision );
  GroupPointLayout->addWidget( SpinBox_Y, 0, 3 );

  TextLabelZ = new QLabel( GroupAbsolutePoint );
  TextLabelZ->setObjectName( "TextLabelZ" );
  TextLabelZ->setText( tr("Z:") );
  GroupPointLayout->addWidget( TextLabelZ, 0, 4 );

  SpinBox_Z = new QtxDoubleSpinBox( min, max, step, GroupAbsolutePoint );
  SpinBox_Z->setObjectName("SpinBox_Z" );
  SpinBox_Z->setPrecision( precision );
  GroupPointLayout->addWidget( SpinBox_Z, 0, 5 );

  resetButton  = new QPushButton( GroupAbsolutePoint );
  resetButton->setObjectName( "resetButton" );
  resetButton->setText( tr( "RESET"  ) );
  GroupPointLayout->addWidget( resetButton, 0, 6 );

  // Group Direction
  GroupAbsoluteDirection = new QGroupBox( this );
  GroupAbsoluteDirection->setObjectName( "GroupDirection" );
  GroupAbsoluteDirection->setTitle( tr("DIRECTION") );
  QGridLayout* GroupDirectionLayout = new QGridLayout( GroupAbsoluteDirection );
  GroupDirectionLayout->setAlignment( Qt::AlignTop );
  GroupDirectionLayout->setSpacing( 6 );
  GroupDirectionLayout->setMargin( 11 );

  TextLabelDx = new QLabel( GroupAbsoluteDirection );
  TextLabelDx->setObjectName( "TextLabelDx" );
  TextLabelDx->setText( tr("Dx:") );
  GroupDirectionLayout->addWidget( TextLabelDx, 0, 0 );

  SpinBox_Dx = new QtxDoubleSpinBox( min, max, step, GroupAbsoluteDirection );
  SpinBox_Dx->setObjectName("SpinBox_Dx" );
  SpinBox_Dx->setPrecision( precision );
  GroupDirectionLayout->addWidget( SpinBox_Dx, 0, 1 );

  TextLabelDy = new QLabel( GroupAbsoluteDirection );
  TextLabelDy->setObjectName( "TextLabelDy" );
  TextLabelDy->setText( tr("Dy:") );
  GroupDirectionLayout->addWidget( TextLabelDy, 0, 2 );

  SpinBox_Dy = new QtxDoubleSpinBox( min, max, step, GroupAbsoluteDirection );
  SpinBox_Dy->setObjectName("SpinBox_Dy" );
  SpinBox_Dy->setPrecision( precision );
  GroupDirectionLayout->addWidget( SpinBox_Dy, 0, 3 );

  TextLabelDz = new QLabel( GroupAbsoluteDirection );
  TextLabelDz->setObjectName( "TextLabelDz" );
  TextLabelDz->setText( tr("Dz:") );
  GroupDirectionLayout->addWidget( TextLabelDz, 0, 4 );

  SpinBox_Dz = new QtxDoubleSpinBox( min, max, step, GroupAbsoluteDirection );
  SpinBox_Dz->setObjectName("SpinBox_Dz" );
  SpinBox_Dz->setPrecision( precision );
  GroupDirectionLayout->addWidget( SpinBox_Dz, 0, 5 );

  invertButton  = new QPushButton( GroupAbsoluteDirection );
  invertButton->setObjectName( "invertButton" );
  invertButton->setText( tr( "INVERT"  ) );
  GroupDirectionLayout->addWidget( invertButton, 0, 6 );

  CBAbsoluteOrientation = new QComboBox( GroupAbsoluteDirection );
  CBAbsoluteOrientation->setObjectName( "AbsoluteOrientation" );
  CBAbsoluteOrientation->insertItem( CBAbsoluteOrientation->count(), tr( "CUSTOM" ) );
  CBAbsoluteOrientation->insertItem( CBAbsoluteOrientation->count(), tr( "||X-Y" ) );
  CBAbsoluteOrientation->insertItem( CBAbsoluteOrientation->count(), tr( "||Y-Z" ) );
  CBAbsoluteOrientation->insertItem( CBAbsoluteOrientation->count(), tr( "||Z-X" ) );
  GroupDirectionLayout->addWidget( CBAbsoluteOrientation, 1, 0, 1, 6 );

  QVBoxLayout* ModeActiveLayout = new QVBoxLayout();
  ModeActiveLayout->setMargin( 11 ); ModeActiveLayout->setSpacing( 6 );
  ModeActiveLayout->addWidget( GroupAbsolutePoint );
  ModeActiveLayout->addWidget( GroupAbsoluteDirection );

  QWidget* ModeActiveWidget = new QWidget( this );
  ModeActiveWidget->setLayout( ModeActiveLayout );

  /**********************   Mode Relative   **********************/
  /* Controls for relative mode of clipping plane:
     Distance - Value from 0 to 1.
     Specifies the distance from the minimum value in a given direction of bounding box to the current position
     Rotation1, Rotation2 - turn angles of cutting plane in given directions
     Orientation - direction of cutting plane
  */
  QGroupBox* GroupParameters = new QGroupBox( tr("SMESH_PARAMETERS"), this );
  QGridLayout* GroupParametersLayout = new QGridLayout( GroupParameters );
  GroupParametersLayout->setMargin( 11 ); GroupParametersLayout->setSpacing( 6 );

  TextLabelOrientation = new QLabel( tr("SMESH_ORIENTATION"), GroupParameters);
  TextLabelOrientation->setObjectName( "TextLabelOrientation" );
  GroupParametersLayout->addWidget( TextLabelOrientation, 0, 0 );

  CBRelativeOrientation = new QComboBox(GroupParameters);
  CBRelativeOrientation->setObjectName( "RelativeOrientation" );
  CBRelativeOrientation->addItem( tr("ALONG_XY") );
  CBRelativeOrientation->addItem( tr("ALONG_YZ") );
  CBRelativeOrientation->addItem( tr("ALONG_ZX") );
  GroupParametersLayout->addWidget( CBRelativeOrientation, 0, 1 );

  TextLabelDistance = new QLabel( tr("SMESH_DISTANCE"), GroupParameters );
  TextLabelDistance->setObjectName( "TextLabelDistance" );
  GroupParametersLayout->addWidget( TextLabelDistance, 1, 0 );

  SpinSliderDistance = new QtxDoubleSpinSlider( 0., 1., 0.01, GroupParameters );
  SpinSliderDistance->setObjectName( "SpinSliderDistance" );
  SpinSliderDistance->setPrecision( precision );
  QFont fnt = SpinSliderDistance->font(); fnt.setBold( true ); SpinSliderDistance->setFont( fnt );
  GroupParametersLayout->addWidget( SpinSliderDistance, 1, 1 );
  
  QString aUnitRot = QString(QChar(0xB0));

  TextLabelRotation1 = new QLabel( tr("ROTATION_AROUND_X_Y2Z"), GroupParameters );
  TextLabelRotation1->setObjectName( "TextLabelRotation1" );
  GroupParametersLayout->addWidget( TextLabelRotation1, 2, 0 );

  SpinSliderRotation1 = new QtxIntSpinSlider( -180, 180, 1, GroupParameters );
  SpinSliderRotation1->setObjectName( "SpinSliderRotation1" );
  SpinSliderRotation1->setUnit( aUnitRot );
  SpinSliderRotation1->setFont( fnt );
  GroupParametersLayout->addWidget( SpinSliderRotation1, 2, 1 );

  TextLabelRotation2 = new QLabel(tr("ROTATION_AROUND_Y_X2Z"), GroupParameters);
  TextLabelRotation2->setObjectName( "TextLabelRotation2" );
  TextLabelRotation2->setObjectName( "TextLabelRotation2" );
  GroupParametersLayout->addWidget( TextLabelRotation2, 3, 0 );

  SpinSliderRotation2 = new QtxIntSpinSlider( -180, 180, 1, GroupParameters );
  SpinSliderRotation2->setObjectName( "SpinSliderRotation2" );
  SpinSliderRotation2->setUnit( aUnitRot );
  SpinSliderRotation2->setFont( fnt );
  GroupParametersLayout->addWidget( SpinSliderRotation2, 3, 1 );

  /***************************************************************/
  QWidget* CheckBoxWidget = new QWidget( this );
  QHBoxLayout* CheckBoxLayout = new QHBoxLayout( CheckBoxWidget );

  PreviewCheckBox = new QCheckBox( tr("SHOW_PREVIEW"), CheckBoxWidget );
  PreviewCheckBox->setObjectName( "PreviewCheckBox" );
  PreviewCheckBox->setChecked( true );
  CheckBoxLayout->addWidget( PreviewCheckBox, 0, Qt::AlignCenter );

  AutoApplyCheckBox = new QCheckBox( tr("AUTO_APPLY"), CheckBoxWidget );
  AutoApplyCheckBox->setObjectName( "AutoApplyCheckBox" );
  CheckBoxLayout->addWidget( AutoApplyCheckBox, 0, Qt::AlignCenter );

  /***************************************************************/
  // Controls for "Ok", "Apply" and "Close" button
  QGroupBox* GroupButtons = new QGroupBox(this);
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout(GroupButtons);
  GroupButtonsLayout->setSpacing(SPACING);
  GroupButtonsLayout->setMargin(MARGIN);
  
  buttonOk = new QPushButton( tr( "SMESH_BUT_APPLY_AND_CLOSE" ), GroupButtons );
  buttonOk->setAutoDefault( true );
  buttonOk->setDefault( true );
  buttonApply = new QPushButton( tr( "SMESH_BUT_APPLY" ), GroupButtons );
  buttonApply->setAutoDefault( true );
  buttonCancel = new QPushButton( tr( "SMESH_BUT_CLOSE" ), GroupButtons );
  buttonCancel->setAutoDefault( true );
  buttonHelp = new QPushButton( tr( "SMESH_BUT_HELP" ), GroupButtons );
  buttonHelp->setAutoDefault( true );
  GroupButtonsLayout->addWidget( buttonOk );
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addWidget( buttonApply );
  GroupButtonsLayout->addSpacing(10);
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget( buttonCancel );
  GroupButtonsLayout->addWidget( buttonHelp );

  ModeStackedLayout->addWidget( ModeActiveWidget );
  ModeStackedLayout->addWidget( GroupParameters );

  SMESHGUI_ClippingDlgLayout->addWidget( GroupPlanes );
  SMESHGUI_ClippingDlgLayout->addLayout( ModeStackedLayout );
  SMESHGUI_ClippingDlgLayout->addWidget( CheckBoxWidget );
  SMESHGUI_ClippingDlgLayout->addWidget( GroupButtons );

  // Initializations
  initParam();

  myIsSelectPlane = false;

  myHelpFileName = "clipping_page.html";

  // signals and slots connections :
  connect( ComboBoxPlanes, SIGNAL( activated( int ) ), this, SLOT( onSelectPlane( int ) ) );
  connect( isOpenGLClipping, SIGNAL( toggled( bool ) ), this, SLOT( onIsOpenGLClipping( bool ) ) );
  connect( buttonNew, SIGNAL( clicked() ), buttonNew, SLOT( showMenu() ) );
  connect( buttonDelete, SIGNAL( clicked() ), this, SLOT( ClickOnDelete() ) );
  connect( ActorList, SIGNAL( itemChanged( QListWidgetItem* ) ), this, SLOT( onActorItemChanged( QListWidgetItem*) ) );
  connect( SelectAllCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( onSelectAll( int ) ) );

  connect( invertButton, SIGNAL (clicked() ), this, SLOT( onInvert() ) ) ;
  connect( resetButton,  SIGNAL (clicked() ), this, SLOT( onReset() ) );
  connect( SpinBox_X,  SIGNAL ( valueChanged( double ) ),  this, SLOT( SetCurrentPlaneParam() ) );
  connect( SpinBox_Y,  SIGNAL ( valueChanged( double ) ),  this, SLOT( SetCurrentPlaneParam() ) );
  connect( SpinBox_Z,  SIGNAL ( valueChanged( double ) ),  this, SLOT( SetCurrentPlaneParam() ) );
  connect( SpinBox_Dx, SIGNAL ( valueChanged( double ) ),  this, SLOT( SetCurrentPlaneParam() ) );
  connect( SpinBox_Dy, SIGNAL ( valueChanged( double ) ),  this, SLOT( SetCurrentPlaneParam() ) );
  connect( SpinBox_Dz, SIGNAL ( valueChanged( double ) ),  this, SLOT( SetCurrentPlaneParam() ) );
  connect( CBAbsoluteOrientation, SIGNAL ( activated ( int ) ), this, SLOT( onSelectAbsoluteOrientation( int ) ) ) ;

  connect( CBRelativeOrientation, SIGNAL( activated( int ) ), this, SLOT( onSelectRelativeOrientation( int ) ) );
  connect( SpinSliderDistance, SIGNAL( valueChanged( double ) ),  this, SLOT( SetCurrentPlaneParam() ) );
  connect( SpinSliderRotation1, SIGNAL( valueChanged( int ) ), this, SLOT( SetCurrentPlaneParam() ) );
  connect( SpinSliderRotation2, SIGNAL( valueChanged( int ) ), this, SLOT( SetCurrentPlaneParam() ) );

  connect( PreviewCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( OnPreviewToggle( bool ) ) );
  connect( AutoApplyCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( onAutoApply( bool ) ) );
  connect( buttonOk, SIGNAL( clicked() ), this, SLOT( ClickOnOk() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect( buttonApply, SIGNAL( clicked() ), this, SLOT( ClickOnApply() ) );
  connect( buttonHelp, SIGNAL( clicked() ), this, SLOT( ClickOnHelp() ) );
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( reject() ) );
  /* to close dialog if study frame change */
  connect( mySMESHGUI, SIGNAL ( SignalStudyFrameChanged() ), this, SLOT( reject() ) );

  initializePlaneData();
  synchronize();

  this->show();
}

/*!
  Destructor
  Destroys the object and frees any allocated resources
*/
SMESHGUI_ClippingDlg::~SMESHGUI_ClippingDlg()
{
  // no need to delete child widgets, Qt does it all for us
  std::for_each(myPlanes.begin(),myPlanes.end(),TSetVisibility(false));
  if (myViewWindow)
    SMESH::RenderViewWindow(myViewWindow);

  for ( size_t i = 0; i < myPlanes.size(); i++ ) {
    SMESH::TPlaneData aPlaneData = myPlanes[i];
    aPlaneData.Plane->Delete();
  }

  if ( myPreviewWidget ) {
    myPreviewWidget->Off();
    myPreviewWidget->Delete();
  }
  myPreviewWidget = 0;
  myCallback->Delete();

  myViewWindow->Repaint();
}

/*!
  Get distance for cutting plane in relative mode
*/
double SMESHGUI_ClippingDlg::getDistance() const
{
  return SpinSliderDistance->value();
}

/*!
  Set distance of cutting plane in relative mode
*/
void SMESHGUI_ClippingDlg::setDistance( const double theDistance )
{
  SpinSliderDistance->setValue( theDistance );
}

/*!
  Get rotation1 for cutting plane in relative mode
*/
double SMESHGUI_ClippingDlg::getRotation1() const
{
  return SpinSliderRotation1->value();
}

/*!
  Get rotation2 for cutting plane in relative mode
*/
double SMESHGUI_ClippingDlg::getRotation2() const
{
  return SpinSliderRotation2->value();
}

/*!
  Set angles of clipping plane in relative mode
*/
void SMESHGUI_ClippingDlg::setRotation (const double theRot1, const double theRot2)
{
  SpinSliderRotation1->setValue( int(floor(theRot1)) );
  SpinSliderRotation2->setValue( int(floor(theRot2)) );
}

/*!
  Set coordinates of origin point in dialog box
*/
void SMESHGUI_ClippingDlg::setOrigin( double theVal[3] )
{
  int anOrientation = CBAbsoluteOrientation->currentIndex();
  if( anOrientation == 0 || anOrientation == 2 )
    SpinBox_X->setValue( theVal[0] );
  if( anOrientation == 0 || anOrientation == 3 )
    SpinBox_Y->setValue( theVal[1] );
  if( anOrientation == 0 || anOrientation == 1 )
    SpinBox_Z->setValue( theVal[2] );
}

/*!
  Set coordinates of normal vector in dialog box
*/
void SMESHGUI_ClippingDlg::setDirection( double theVal[3] )
{
  int anOrientation = CBAbsoluteOrientation->currentIndex();
  if( anOrientation == 0 ) {
    SpinBox_Dx->setValue( theVal[0] );
    SpinBox_Dy->setValue( theVal[1] );
    SpinBox_Dz->setValue( theVal[2] );
  }
}

/*!
  Create a new widget for preview clipping plane
*/
vtkImplicitPlaneWidget* SMESHGUI_ClippingDlg::createPreviewWidget()
{
  vtkImplicitPlaneWidget* aPlaneWgt = vtkImplicitPlaneWidget::New();

  aPlaneWgt->SetInteractor( myViewWindow->getInteractor() );
  aPlaneWgt->SetPlaceFactor( SIZEFACTOR );
  aPlaneWgt->ScaleEnabledOff();
  aPlaneWgt->SetOrigin( 0, 0, 0 );
  aPlaneWgt->SetNormal( -1, -1, -1 );
  aPlaneWgt->Off();

  double anRGB[3];
  SMESH::GetColor( "SMESH", "fill_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 170, 255 ) );

  aPlaneWgt->GetPlaneProperty()->SetColor( anRGB[0],anRGB[1],anRGB[2] );
  aPlaneWgt->GetPlaneProperty()->SetOpacity( 0.2 );;

  aPlaneWgt->GetSelectedPlaneProperty()->SetColor( anRGB[0],anRGB[1],anRGB[2] );
  aPlaneWgt->GetSelectedPlaneProperty()->SetOpacity( 0.2 );
  aPlaneWgt->GetSelectedPlaneProperty()->SetLineWidth( 2.0 );

  aPlaneWgt->AddObserver(vtkCommand::InteractionEvent, myCallback, 0.);

  return aPlaneWgt;
}

namespace
{
  /*!
    Translate two angles of plane to normal
  */
  void rotationToNormal ( double theRotation[2],
                          int theOrientation,
                          double theNormal[3],
                          double theDir[2][3] )
  {
    static double aCoeff = M_PI/180.0;

    double anU[2] = { cos( aCoeff * theRotation[0] ), cos( aCoeff * theRotation[1] ) };
    double aV[2] = { sqrt( 1.0 - anU[0]*anU[0] ), sqrt( 1.0 - anU[1] * anU[1] ) };
    aV[0] = theRotation[0] > 0? aV[0]: -aV[0];
    aV[1] = theRotation[1] > 0? aV[1]: -aV[1];

    switch ( theOrientation ) {
    case 0:
    case 1:
      theDir[0][1] = anU[0];
      theDir[0][2] = aV[0];
      theDir[1][0] = anU[1];
      theDir[1][2] = aV[1];
      break;
    case 2:
      theDir[0][2] = anU[0];
      theDir[0][0] = aV[0];
      theDir[1][1] = anU[1];
      theDir[1][0] = aV[1];
      break;
    case 3:
      theDir[0][0] = anU[0];
      theDir[0][1] = aV[0];
      theDir[1][2] = anU[1];
      theDir[1][1] = aV[1];
      break;
    }

    vtkMath::Cross( theDir[1], theDir[0], theNormal );
    vtkMath::Normalize( theNormal );
    vtkMath::Cross( theNormal, theDir[1], theDir[0] );
  }

  /*!
   * \brief Return a name of a father mesh if any
   */
  QString getFatherName( _PTR(SObject)& theSObj )
  {
    _PTR(SComponent) objComponent = theSObj->GetFatherComponent();
    const int theMeshDepth = 1 + objComponent->Depth();
    if ( theSObj->Depth() <= theMeshDepth )
      return QString(); // theSObj is a mesh

    _PTR(SObject) sobj = theSObj->GetFather();
    while ( sobj && sobj->Depth() > theMeshDepth )
      sobj = sobj->GetFather();

    return sobj ? sobj->GetName().c_str() : "";
  }
}

/*!
  Used in SMESHGUI::restoreVisualParameters() to avoid
  Declaration of OrientedPlane outside of SMESHGUI_ClippingDlg.cxx
*/
bool SMESHGUI_ClippingDlg::AddPlane ( SMESH::TActorList       theActorList,
                                      SMESH::OrientedPlane*   thePlane )
{
  double aNormal[3];
  double aDir[2][3] = {{0, 0, 0}, {0, 0, 0}};
  static double aCoeff = vtkMath::Pi()/180.0;

  int anOrientation = 0;
  if ( thePlane->PlaneMode == SMESH::Absolute )
    anOrientation = thePlane->myAbsoluteOrientation;
  else if ( thePlane->PlaneMode == SMESH::Relative )
    anOrientation = thePlane->myRelativeOrientation + 1;

  if ( anOrientation == 0 ) {
    // compute a direction for plane in absolute mode
    double znam = sqrt( thePlane->Dx*thePlane->Dx + thePlane->Dy*thePlane->Dy + thePlane->Dz*thePlane->Dz );
    double aRotation = acos( thePlane->Dy/znam )/aCoeff;
    if ( thePlane->Dy >= 0.0 && thePlane->Dz >= 0.0 )      thePlane->myAngle[0] = 90.0 + aRotation;
    else if ( thePlane->Dy >= 0.0 && thePlane->Dz < 0.0 ) thePlane->myAngle[0] = 90.0 - aRotation;
    else if ( thePlane->Dy < 0.0 && thePlane->Dz >= 0.0 ) thePlane->myAngle[0] = aRotation - 90.0;
    else if ( thePlane->Dy < 0.0 && thePlane->Dz < 0.0 ) thePlane->myAngle[0] = 270.0 - aRotation;

    aRotation = acos( thePlane->Dx/znam )/aCoeff;
    if ( thePlane->Dx >= 0.0 && thePlane->Dz >= 0.0 )      thePlane->myAngle[1] = 90.0 + aRotation;
    else if ( thePlane->Dx >= 0.0 && thePlane->Dz < 0.0 ) thePlane->myAngle[1] = 90.0 - aRotation;
    else if ( thePlane->Dx < 0.0 && thePlane->Dz >= 0.0 ) thePlane->myAngle[1] = aRotation - 90.0;
    else if ( thePlane->Dx < 0.0 && thePlane->Dz < 0.0 ) thePlane->myAngle[1] = 270.0 - aRotation;
  }

  // compute a normal
  rotationToNormal( thePlane->myAngle, anOrientation, aNormal, aDir );

  double aBounds[6];
  double anOrigin[3];

  if ( thePlane->PlaneMode == SMESH::Absolute ) {
    aNormal[0] = thePlane->Dx;
    aNormal[1] = thePlane->Dy;
    aNormal[2] = thePlane->Dz;
  }

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
                                                    thePlane->myDistance,
                                                    aBounds,
                                                    anOrigin );
  if( !anIsOk )
    return false;

  if ( thePlane->PlaneMode == SMESH::Absolute ) {
    anOrigin[0] = thePlane->X;
    anOrigin[1] = thePlane->Y;
    anOrigin[2] = thePlane->Z;
  }
  thePlane->SetNormal( aNormal );
  thePlane->SetOrigin( anOrigin );


  double aPnt[3] = { ( aBounds[0] + aBounds[1] ) / 2.,
                     ( aBounds[2] + aBounds[3] ) / 2.,
                     ( aBounds[4] + aBounds[5] ) / 2. };

  double aDel = pow( pow( aBounds[1] - aBounds[0], 2 ) +
                     pow( aBounds[3] - aBounds[2], 2 ) +
                     pow( aBounds[5] - aBounds[4], 2 ), 0.5 );

  double aDelta[2][3] = { { aDir[0][0]*aDel, aDir[0][1]*aDel, aDir[0][2]*aDel },
                          { aDir[1][0]*aDel, aDir[1][1]*aDel, aDir[1][2]*aDel } };
  double aParam, aPnt0[3], aPnt1[3], aPnt2[3];

  double aPnt01[3] = { aPnt[0] - aDelta[0][0] - aDelta[1][0],
                       aPnt[1] - aDelta[0][1] - aDelta[1][1],
                       aPnt[2] - aDelta[0][2] - aDelta[1][2] };
  double aPnt02[3] = { aPnt01[0] + aNormal[0],
                       aPnt01[1] + aNormal[1],
                       aPnt01[2] + aNormal[2] };
  vtkPlane::IntersectWithLine( aPnt01, aPnt02, aNormal, anOrigin, aParam, aPnt0 );

  double aPnt11[3] = { aPnt[0] - aDelta[0][0] + aDelta[1][0],
                       aPnt[1] - aDelta[0][1] + aDelta[1][1],
                       aPnt[2] - aDelta[0][2] + aDelta[1][2] };
  double aPnt12[3] = { aPnt11[0] + aNormal[0],
                       aPnt11[1] + aNormal[1],
                       aPnt11[2] + aNormal[2] };
  vtkPlane::IntersectWithLine( aPnt11, aPnt12, aNormal, anOrigin, aParam, aPnt1);

  double aPnt21[3] = { aPnt[0] + aDelta[0][0] - aDelta[1][0],
                       aPnt[1] + aDelta[0][1] - aDelta[1][1],
                       aPnt[2] + aDelta[0][2] - aDelta[1][2] };
  double aPnt22[3] = { aPnt21[0] + aNormal[0],
                       aPnt21[1] + aNormal[1],
                       aPnt21[2] + aNormal[2] };
  vtkPlane::IntersectWithLine( aPnt21, aPnt22, aNormal, anOrigin, aParam, aPnt2);

  vtkPlaneSource* aPlaneSource = thePlane->myPlaneSource;
  aPlaneSource->SetNormal( aNormal[0], aNormal[1], aNormal[2] );
  aPlaneSource->SetOrigin( aPnt0[0], aPnt0[1], aPnt0[2] );
  aPlaneSource->SetPoint1( aPnt1[0], aPnt1[1], aPnt1[2] );
  aPlaneSource->SetPoint2( aPnt2[0], aPnt2[1], aPnt2[2] );
  aPlaneSource->Update();

  SMESH::TActorList::iterator anIter = theActorList.begin();
  for ( ; anIter != theActorList.end(); anIter++ )
    if( vtkActor* aVTKActor = *anIter )
      if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( aVTKActor ) ) {
        if( thePlane->IsOpenGLClipping )
          anActor->AddOpenGLClippingPlane( thePlane->InvertPlane() );
        else
          anActor->AddClippingPlane( thePlane );
      }

  return true;
}

/*!
  Custom handling of events
*/
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

/*!
  Handles the char preview widget activation event
*/
void SMESHGUI_ClippingDlg::ProcessEvents( vtkObject* theObject,
                                          unsigned long theEvent,
                                          void* theClientData,
                                          void* vtkNotUsed( theCallData ) )
{
  vtkImplicitPlaneWidget* aWidget = vtkImplicitPlaneWidget::SafeDownCast( theObject );
  if ( aWidget == NULL ) return;
  if ( theClientData == NULL ) return;

  SMESHGUI_ClippingDlg* aDlg = (SMESHGUI_ClippingDlg*) theClientData;

  double anOrigin[3];
  double aDir[3];

  switch( theEvent ){
  case vtkCommand::InteractionEvent:
    aWidget->GetOrigin( anOrigin );
    aWidget->GetNormal( aDir );

    aDlg->myIsSelectPlane = true;

    if( aDlg->CurrentMode == SMESH::Absolute ) {
      aDlg->setOrigin( anOrigin );
      aDlg->setDirection( aDir );
      aDlg->myIsPreviewMoved = true;
    }
    else if( aDlg->CurrentMode == SMESH::Relative ) {
      aDlg->absolutePlaneToRelative( anOrigin, aDir );
    }
    aDlg->myIsSelectPlane = false;

    aDlg->SetCurrentPlaneParam();
    aDlg->myIsPreviewMoved = false;
    break;
  }
}

/*!
  Initialize the planes's data when the dialog opened
*/
void SMESHGUI_ClippingDlg::initializePlaneData()
{
  const SMESHGUI_ClippingPlaneInfoMap& aClippingPlaneInfoMap = mySMESHGUI->getClippingPlaneInfoMap();
  SMESHGUI_ClippingPlaneInfoMap::const_iterator anIter1 = aClippingPlaneInfoMap.find( myViewWindow->getViewManager() );
  if( anIter1 != aClippingPlaneInfoMap.end() ) {
    const SMESHGUI_ClippingPlaneInfoList& aClippingPlaneInfoList = anIter1->second;
    SMESHGUI_ClippingPlaneInfoList::const_iterator anIter2 = aClippingPlaneInfoList.begin();
    for( ; anIter2 != aClippingPlaneInfoList.end(); anIter2++ ) {
      const SMESH::ClippingPlaneInfo& aClippingPlaneInfo = *anIter2;
      SMESH::OrientedPlane* anOrientedPlane = SMESH::OrientedPlane::New(myViewWindow);
      anOrientedPlane->ShallowCopy(aClippingPlaneInfo.Plane);
      SMESH::TPlane aTPlane( anOrientedPlane );
      SMESH::TPlaneData aPlaneData( aTPlane, aClippingPlaneInfo.ActorList );
      myPlanes.push_back( aPlaneData );
    }
  }
  std::for_each( myPlanes.begin(),myPlanes.end(), TSetVisibility( PreviewCheckBox->isChecked() ) );
  if( myPlanes.size() )
    myPreviewWidget->SetEnabled( PreviewCheckBox->isChecked() );
}

/*!
  Initialization of initial values of widgets
*/
void SMESHGUI_ClippingDlg::initParam()
{
  SpinBox_X->setValue( 0.0 );
  SpinBox_Y->setValue( 0.0 );
  SpinBox_Z->setValue( 0.0 );

  SpinBox_Dx->setValue( 1.0 );
  SpinBox_Dy->setValue( 1.0 );
  SpinBox_Dz->setValue( 1.0 );

  CBAbsoluteOrientation->setCurrentIndex(0);

  SpinSliderDistance->setValue( 0.5 );
  SpinSliderRotation1->setValue( 0 );
  SpinSliderRotation2->setValue( 0 );
  CBRelativeOrientation->setCurrentIndex( 0 );
}

/*!
  Synchronize dialog's widgets with data
*/
void SMESHGUI_ClippingDlg::synchronize()
{
  int aNbPlanes = myPlanes.size();
  ComboBoxPlanes->clear();

  QString aName;
  for( int i = 1; i<=aNbPlanes; i++ ) {
    aName = QString( tr( "PLANE_NUM" ) ).arg(i);
    ComboBoxPlanes->addItem( aName );
  }

  int aPos = ComboBoxPlanes->count() - 1;
  ComboBoxPlanes->setCurrentIndex( aPos );

  bool anIsControlsEnable = ( aPos >= 0 );
  if ( anIsControlsEnable ) {
    onSelectPlane( aPos );
    updateActorList();
    if( PreviewCheckBox->isChecked() )
      myPreviewWidget->On();
  }
  else {
    ComboBoxPlanes->addItem( tr( "NO_PLANES" ) );
    ActorList->clear();
    initParam();
    if( PreviewCheckBox->isChecked() )
      myPreviewWidget->Off();
  }

  isOpenGLClipping->setEnabled( anIsControlsEnable );
  ActorList->setEnabled( anIsControlsEnable );
  SelectAllCheckBox->setEnabled( anIsControlsEnable );
  buttonDelete->setEnabled( anIsControlsEnable );
  if ( CurrentMode == SMESH::Absolute ) {
    SpinBox_X->setEnabled( anIsControlsEnable );
    SpinBox_Y->setEnabled( anIsControlsEnable );
    SpinBox_Z->setEnabled( anIsControlsEnable );
    SpinBox_Dx->setEnabled( anIsControlsEnable );
    SpinBox_Dy->setEnabled( anIsControlsEnable );
    SpinBox_Dz->setEnabled( anIsControlsEnable );
    CBAbsoluteOrientation->setEnabled( anIsControlsEnable );
    invertButton->setEnabled( anIsControlsEnable );
    resetButton->setEnabled( anIsControlsEnable );
  }
  else if ( CurrentMode == SMESH::Relative ) {
    CBRelativeOrientation->setEnabled( anIsControlsEnable );
    SpinSliderDistance->setEnabled( anIsControlsEnable );
    SpinSliderRotation1->setEnabled( anIsControlsEnable );
    SpinSliderRotation2->setEnabled( anIsControlsEnable );
  }
}

/*!
  Update the list of actors
*/
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

  std::for_each( myPlanes.begin(),myPlanes.end(), TSetVisibility( PreviewCheckBox->isChecked() ) );
  aPlaneData.Plane.GetPointer()->myActor->SetVisibility( false );

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
          QString aFatherName = getFatherName( aSObj );
          if ( !aFatherName.isEmpty() )
            aName = aFatherName + " / " + aName;
          aName += QString(" (%1)").arg( aSObj->GetID().c_str() );
          QListWidgetItem* anItem = new ActorItem( anActor, aName, ActorList );
          anItem->setCheckState( anIsChecked ? Qt::Checked : Qt::Unchecked );
          updateActorItem( anItem, true, false );
        }
      }
    }
  }
}

/*!
  Update an actor in actor's list
*/
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

    bool anIsBlocked = SelectAllCheckBox->blockSignals( true );
    SelectAllCheckBox->setCheckState( aNbChecked == aNbItems ? Qt::Checked : Qt::Unchecked);
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

        if( SMESH::ComputeBounds( anActorList, myBounds ) ) {
          myPreviewWidget->PlaceWidget( myBounds[0], myBounds[1], myBounds[2],
                                        myBounds[3], myBounds[4], myBounds[5] );
          if( PreviewCheckBox->isChecked() )
            myPreviewWidget->On();
        }
        else
          myPreviewWidget->Off();
      }
    }
  }
}

/*!
  Get the list of current actors
*/
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

/*!
  Dump the parameters of clipping planes
*/
void SMESHGUI_ClippingDlg::dumpPlaneData() const
{
  printf( "----------- Plane Data -----------\n" );
  int anId = 1;
  SMESH::TPlaneDataVector::const_iterator anIter1 = myPlanes.begin();
  for ( ; anIter1 != myPlanes.end(); anIter1++, anId++ ) {
    SMESH::TPlaneData aPlaneData = *anIter1;
    SMESH::TPlane aPlane = aPlaneData.Plane;
    double* aNormal = aPlane->GetNormal();
    double* anOrigin = aPlane->GetOrigin();
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

/*!
  SLOT on close button click: rejects dialog
*/
void SMESHGUI_ClippingDlg::reject()
{
  //here we can insert actions to do at close.
  QDialog::reject();
}

/*!
  Set absolute mode of clipping plane
*/
void SMESHGUI_ClippingDlg::onModeAbsolute()
{
  ModeStackedLayout->setCurrentIndex(0);
  CurrentMode = SMESH::Absolute;
  ClickOnNew();
  SetCurrentPlaneParam();
}

/*!
  Set relative mode of clipping plane
*/
void SMESHGUI_ClippingDlg::onModeRelative()
{
  ModeStackedLayout->setCurrentIndex(1);
  CurrentMode = SMESH::Relative;
  ClickOnNew();
  SetCurrentPlaneParam();
}

/*!
  SLOT on new button click: create a new clipping plane
*/
void SMESHGUI_ClippingDlg::ClickOnNew()
{
  if(myViewWindow) {
    SMESH::OrientedPlane* aPlane = SMESH::OrientedPlane::New(myViewWindow);
    SMESH::TPlane aTPlane(aPlane);
    aPlane->PlaneMode = CurrentMode;
    SMESH::TActorList anActorList;
    VTK::ActorCollectionCopy aCopy( myViewWindow->getRenderer()->GetActors() );
    vtkActorCollection* anAllActors = aCopy.GetActors();
    anAllActors->InitTraversal();
    while( vtkActor* aVTKActor = anAllActors->GetNextActor() )
      if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( aVTKActor ) )
        anActorList.push_back( anActor );

    SMESH::TPlaneData aPlaneData(aTPlane, anActorList);

    myPlanes.push_back(aPlaneData);


    std::for_each( myPlanes.begin(),myPlanes.end(), TSetVisibility( PreviewCheckBox->isChecked() ) );
    aPlane->myActor->SetVisibility( false );

    bool anIsBlocked = ActorList->blockSignals( true );

    if( SMESH::ComputeBounds( anActorList, myBounds ) ) {
      myPreviewWidget->PlaceWidget( myBounds[0], myBounds[1], myBounds[2],
                                    myBounds[3], myBounds[4], myBounds[5] );
      if( PreviewCheckBox->isChecked() )
        myPreviewWidget->On();
    }
    else
      myPreviewWidget->Off();

    synchronize();
    SetCurrentPlaneParam();

    ActorList->blockSignals( anIsBlocked );
  }
}

/*!
  SLOT on delete button click: Delete selected clipping plane
*/
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

/*!
  Set current parameters of selected plane
*/
void SMESHGUI_ClippingDlg::onSelectPlane ( int theIndex )
{
  if ( myPlanes.empty() )
    return;

  SMESH::TPlaneData aPlaneData = myPlanes[theIndex];
  SMESH::OrientedPlane* aPlane = aPlaneData.Plane.GetPointer();

  myIsSelectPlane = true;
  isOpenGLClipping->setChecked( aPlane->IsOpenGLClipping );

  if ( aPlane->PlaneMode == SMESH::Absolute ) {
    ModeStackedLayout->setCurrentIndex( 0 );
    CurrentMode = SMESH::Absolute;
    int anOrientation = aPlane->myAbsoluteOrientation;
    // Set plane parameters in the dialog
    double anOrigin[3], aDir[3];
    anOrigin[0] = aPlane->X;
    anOrigin[1] = aPlane->Y;
    anOrigin[2] = aPlane->Z;
    setOrigin( anOrigin );
    aDir[0] = aPlane->Dx;
    aDir[1] = aPlane->Dy;
    aDir[2] = aPlane->Dz;
    setDirection( aDir );
    CBAbsoluteOrientation->setCurrentIndex( anOrientation );
    onSelectAbsoluteOrientation( anOrientation );
  }
  else if ( aPlane->PlaneMode == SMESH::Relative ) {
    ModeStackedLayout->setCurrentIndex( 1 );
    CurrentMode = SMESH::Relative;
    SMESH::Orientation anOrientation = aPlane->GetOrientation();
    double aRot[2] = { aPlane->myAngle[0], aPlane->myAngle[1] };
    // Set plane parameters in the dialog
    setDistance( aPlane->GetDistance() );
    setRotation( aRot[0], aRot[1] );
    switch ( anOrientation ) {
    case SMESH::XY:
      CBRelativeOrientation->setCurrentIndex(0);
      onSelectRelativeOrientation(0);
      break;
    case SMESH::YZ:
      CBRelativeOrientation->setCurrentIndex(1);
      onSelectRelativeOrientation(1);
      break;
    case SMESH::ZX:
      CBRelativeOrientation->setCurrentIndex(2);
      onSelectRelativeOrientation(2);
      break;
    }
  }
  myIsSelectPlane = false;

  if( SMESH::ComputeBounds( aPlaneData.ActorList, myBounds ) ) {
    myPreviewWidget->PlaceWidget( myBounds[0], myBounds[1], myBounds[2],
                                  myBounds[3], myBounds[4], myBounds[5] );
    if( PreviewCheckBox->isChecked() )
      myPreviewWidget->On();
  }
  else
    myPreviewWidget->Off();

  SetCurrentPlaneParam();

  // Actors
  bool anIsBlocked = ActorList->blockSignals( true );
  updateActorList();
  ActorList->blockSignals( anIsBlocked );
}

/*!
  SLOT: called on OpenGLClipping check box toggled
*/
void SMESHGUI_ClippingDlg::onIsOpenGLClipping( bool toggled )
{
  if ( myPlanes.empty() || myIsSelectPlane )
    return;

  int aCurPlaneIndex = ComboBoxPlanes->currentIndex();
  SMESH::TPlaneData aPlane = myPlanes[aCurPlaneIndex];
  SMESH::OrientedPlane* aPlaneData = aPlane.Plane.GetPointer();
  aPlaneData->IsOpenGLClipping = toggled;

  if( AutoApplyCheckBox->isChecked() )
    ClickOnApply();
}

/*!
  SLOT: called on SelectAll check box toggled
*/
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

/*!
  SLOT: called when actor item was changed
*/
void SMESHGUI_ClippingDlg::onActorItemChanged( QListWidgetItem* theItem )
{
  updateActorItem( theItem, true, true );
  SetCurrentPlaneParam();
}

/*!
  Restore parameters of selected plane
*/
void SMESHGUI_ClippingDlg::SetCurrentPlaneParam()
{
  if ( myPlanes.empty() || myIsSelectPlane )
    return;

  int aCurPlaneIndex = ComboBoxPlanes->currentIndex();

  SMESH::TPlaneData aPlaneData = myPlanes[aCurPlaneIndex];
  SMESH::OrientedPlane* aPlane = aPlaneData.Plane.GetPointer();

  if ( aPlane->PlaneMode == SMESH::Absolute ) {
    aPlane->myAbsoluteOrientation = CBAbsoluteOrientation->currentIndex();
    aPlane->X = SpinBox_X->value();
    aPlane->Y = SpinBox_Y->value();
    aPlane->Z = SpinBox_Z->value();
    aPlane->Dx = SpinBox_Dx->value();
    aPlane->Dy = SpinBox_Dy->value();
    aPlane->Dz = SpinBox_Dz->value();
  }

  double aNormal[3];
  double aDir[2][3] = { {0, 0, 0}, {0, 0, 0} };
  static double aCoeff = vtkMath::Pi()/180.0;

  double aRot[2] = { getRotation1(), getRotation2() };
  int anOrient = 0;
  if ( aPlane->PlaneMode == SMESH::Absolute )
    anOrient = CBAbsoluteOrientation->currentIndex();
  else if ( aPlane->PlaneMode == SMESH::Relative )
    anOrient = CBRelativeOrientation->currentIndex() + 1;

  if ( aPlane->PlaneMode == SMESH::Relative ) {
    aPlane->myAngle[0] = aRot[0];
    aPlane->myAngle[1] = aRot[1];
    aPlane->SetOrientation( SMESH::Orientation( CBRelativeOrientation->currentIndex() ) );
    aPlane->SetDistance( getDistance() );
  }

  if ( anOrient == 0 ) {
    // compute a direction for plane in absolute mode
    double znam = sqrt( aPlane->Dx*aPlane->Dx + aPlane->Dy*aPlane->Dy + aPlane->Dz*aPlane->Dz );
    double aRotation = acos( aPlane->Dy/znam )/aCoeff;
    if ( aPlane->Dy >= 0.0 && aPlane->Dz >= 0.0 )     aRot[0] = 90.0 + aRotation;
    else if ( aPlane->Dy >= 0.0 && aPlane->Dz < 0.0 ) aRot[0] = 90.0 - aRotation;
    else if ( aPlane->Dy < 0.0 && aPlane->Dz >= 0.0 ) aRot[0] = aRotation - 90.0;
    else if ( aPlane->Dy < 0.0 && aPlane->Dz < 0.0 )  aRot[0] = 270.0 - aRotation;

    aRotation = acos( aPlane->Dx/znam )/aCoeff;
    if ( aPlane->Dx >= 0.0 && aPlane->Dz >= 0.0 )     aRot[1] = 90.0 + aRotation;
    else if ( aPlane->Dx >= 0.0 && aPlane->Dz < 0.0 ) aRot[1] = 90.0 - aRotation;
    else if ( aPlane->Dx < 0.0 && aPlane->Dz >= 0.0 ) aRot[1] = aRotation - 90.0;
    else if ( aPlane->Dx < 0.0 && aPlane->Dz < 0.0 )  aRot[1] = 270.0 - aRotation;
  }

  // compute a normal
  rotationToNormal( aRot, anOrient, aNormal, aDir );


  SMESH::TActorList anActorList = aPlaneData.ActorList;

  double aBounds[6];
  double anOrigin[3];
  double aDistance;
  aDistance = getDistance();
  if ( aPlane->PlaneMode == SMESH::Absolute ) {
    aNormal[0] = aPlane->Dx;
    aNormal[1] = aPlane->Dy;
    aNormal[2] = aPlane->Dz;
  }

  bool anIsOk = SMESH::ComputeClippingPlaneParameters( anActorList,
                                                       aNormal,
                                                       aDistance,
                                                       aBounds,
                                                       anOrigin );

  if ( aPlane->PlaneMode == SMESH::Absolute ) {
    anOrigin[0] = aPlane->X;
    anOrigin[1] = aPlane->Y;
    anOrigin[2] = aPlane->Z;
  }

  if( anIsOk ) {
    aPlane->SetNormal( aNormal );
    aPlane->SetOrigin( anOrigin );

    double aPnt[3] = { ( aBounds[0] + aBounds[1] ) / 2.,
                       ( aBounds[2] + aBounds[3] ) / 2.,
                       ( aBounds[4] + aBounds[5] ) / 2. };

    double aDel = pow( pow( aBounds[1] - aBounds[0], 2 ) +
                       pow( aBounds[3] - aBounds[2], 2 ) +
                       pow( aBounds[5] - aBounds[4], 2 ), 0.5 );

    double aDelta[2][3] = { { aDir[0][0]*aDel, aDir[0][1]*aDel, aDir[0][2]*aDel },
                            { aDir[1][0]*aDel, aDir[1][1]*aDel, aDir[1][2]*aDel } };
    double aParam, aPnt0[3], aPnt1[3], aPnt2[3];

    double aPnt01[3] = { aPnt[0] - aDelta[0][0] - aDelta[1][0],
                         aPnt[1] - aDelta[0][1] - aDelta[1][1],
                         aPnt[2] - aDelta[0][2] - aDelta[1][2] };
    double aPnt02[3] = { aPnt01[0] + aNormal[0],
                         aPnt01[1] + aNormal[1],
                         aPnt01[2] + aNormal[2] };
    vtkPlane::IntersectWithLine(aPnt01,aPnt02,aNormal,anOrigin,aParam,aPnt0);

    double aPnt11[3] = { aPnt[0] - aDelta[0][0] + aDelta[1][0],
                         aPnt[1] - aDelta[0][1] + aDelta[1][1],
                         aPnt[2] - aDelta[0][2] + aDelta[1][2] };
    double aPnt12[3] = { aPnt11[0] + aNormal[0],
                         aPnt11[1] + aNormal[1],
                         aPnt11[2] + aNormal[2] };
    vtkPlane::IntersectWithLine(aPnt11,aPnt12,aNormal,anOrigin,aParam,aPnt1);

    double aPnt21[3] = { aPnt[0] + aDelta[0][0] - aDelta[1][0],
                         aPnt[1] + aDelta[0][1] - aDelta[1][1],
                         aPnt[2] + aDelta[0][2] - aDelta[1][2] };
    double aPnt22[3] = { aPnt21[0] + aNormal[0],
                         aPnt21[1] + aNormal[1],
                         aPnt21[2] + aNormal[2] };
    vtkPlane::IntersectWithLine(aPnt21,aPnt22,aNormal,anOrigin,aParam,aPnt2);

    vtkPlaneSource* aPlaneSource = aPlane->myPlaneSource;

    aPlaneSource->SetNormal( aNormal[0], aNormal[1], aNormal[2] );
    aPlaneSource->SetOrigin( aPnt0[0], aPnt0[1], aPnt0[2] );
    aPlaneSource->SetPoint1( aPnt1[0], aPnt1[1], aPnt1[2] );
    aPlaneSource->SetPoint2( aPnt2[0], aPnt2[1], aPnt2[2] );
    aPlaneSource->Update();
  }

  setBoundsForPreviewWidget();

  myPreviewWidget->SetOrigin( anOrigin );
  myPreviewWidget->SetNormal( aNormal );

  if(AutoApplyCheckBox->isChecked())
    ClickOnApply();

  SMESH::RenderViewWindow( myViewWindow );
}

/*!
  Set current bounds for preview widget
*/
void SMESHGUI_ClippingDlg::setBoundsForPreviewWidget()
{
  int aCurPlaneIndex = ComboBoxPlanes->currentIndex();
  SMESH::TPlaneData aPlaneData = myPlanes[aCurPlaneIndex];
  SMESH::OrientedPlane* aPlane = aPlaneData.Plane.GetPointer();
  SMESH::TActorList anActorList = aPlaneData.ActorList;

  double* anOrigin = aPlane->GetOrigin();

  double aBounds[6];
  SMESH::ComputeBounds( anActorList, aBounds );

  bool isBoundsChanged = false;

  if( myIsPreviewMoved ) {
    // if widget has moved by hand the bounds can to minimize
    if( anOrigin[0] > myBounds[0] && anOrigin[0] < aBounds[0] ) {
      myBounds[0] = anOrigin[0]; isBoundsChanged = true; }
    if( anOrigin[0] < myBounds[1] && anOrigin[0] > aBounds[1] ) {
      myBounds[1] = anOrigin[0]; isBoundsChanged = true; }
    if( anOrigin[1] > myBounds[2] && anOrigin[1] < aBounds[2] ) {
      myBounds[2] = anOrigin[1]; isBoundsChanged = true; }
    if( anOrigin[1] < myBounds[3] && anOrigin[1] > aBounds[3] ) {
      myBounds[3] = anOrigin[1]; isBoundsChanged = true; }
    if( anOrigin[2] > myBounds[4] && anOrigin[2] < aBounds[4] ) {
      myBounds[4] = anOrigin[2]; isBoundsChanged = true; }
    if( anOrigin[2] < myBounds[5] && anOrigin[2] > aBounds[5] ) {
      myBounds[5] = anOrigin[2]; isBoundsChanged = true; }
  }
  else {
    // if widget has moved by dialog data the bounds can to take necessary size
    if( anOrigin[0] < aBounds[0] ) {
      myBounds[0] = anOrigin[0]; isBoundsChanged = true; }
    if( anOrigin[0] > aBounds[1] ) {
      myBounds[1] = anOrigin[0]; isBoundsChanged = true; }
    if( anOrigin[1] < aBounds[2] ) {
      myBounds[2] = anOrigin[1]; isBoundsChanged = true; }
    if( anOrigin[1] > aBounds[3] ) {
      myBounds[3] = anOrigin[1]; isBoundsChanged = true; }
    if( anOrigin[2] < aBounds[4] ) {
      myBounds[4] = anOrigin[2]; isBoundsChanged = true; }
    if( anOrigin[2] > aBounds[5] ) {
        myBounds[5] = anOrigin[2]; isBoundsChanged = true; }
  }

  if( isBoundsChanged )
    myPreviewWidget->PlaceWidget( myBounds[0],myBounds[1],myBounds[2],
                                  myBounds[3],myBounds[4],myBounds[5] );
  else if( !myIsPreviewMoved )
    myPreviewWidget->PlaceWidget( aBounds[0],aBounds[1],aBounds[2],
                                  aBounds[3],aBounds[4],aBounds[5] );

}

/*!
  Convert absolute coordinates of plane to relative mode
*/
void SMESHGUI_ClippingDlg::absolutePlaneToRelative ( double theOrigin[3], double theDir[3] )
{
  double aRot[2];

  aRot[0] = getRotation1();
  aRot[1] = getRotation2();

  double eps = 0.0001;

  int anOrientation = CBRelativeOrientation->currentIndex();
  double aDirection[3] = { 0.,0.,0. };
  double aRotation1 = 0, aRotation2 = 0;
  switch( anOrientation ) {
  case 0:
    aDirection[0] = theDir[0] + eps;
    aDirection[1] = theDir[1] + eps;
    aDirection[2] = theDir[2] + eps;
    aRotation1 = atan2( theDir[2], theDir[1] )*180.0/M_PI;
    aRotation2 = atan2( theDir[2], theDir[0] )*180.0/M_PI;
    break;
  case 1:
    aDirection[0] = theDir[1] + eps;
    aDirection[1] = theDir[2] + eps;
    aDirection[2] = theDir[0] + eps;
    aRotation1 = atan2( theDir[0], theDir[2] )*180.0/M_PI;
    aRotation2 = atan2( theDir[0], theDir[1] )*180.0/M_PI;
    break;
  case 2:
    aDirection[0] = theDir[2] + eps;
    aDirection[1] = theDir[0] + eps;
    aDirection[2] = theDir[1] + eps;
    aRotation1 = atan2( theDir[1], theDir[0] )*180.0/M_PI;
    aRotation2 = atan2( theDir[1], theDir[2] )*180.0/M_PI;
    break;
  }

  if( aDirection[0] > 0 && aDirection[1] > 0 && aDirection[2] > 0 && aRot[0] <= 0 ) {
    aRot[0] = aRotation1 - 90.0;  aRot[1] = aRotation2 - 90.0; }
  else if( aDirection[0] > 0 && aDirection[1] > 0 && aDirection[2] > 0 && aRot[0] > 0 ) {
    aRot[0] = aRotation1 + 90.0;  aRot[1] = aRotation2 + 90.0; }
  else if( aDirection[0] > 0 && aDirection[1] > 0 && aDirection[2] < 0 && aRot[0] <= 0 ) {
    aRot[0] = aRotation1 - 90.0;  aRot[1] = aRotation2 + 90.0; }
  else if( aDirection[0] > 0 && aDirection[1] > 0 && aDirection[2] < 0 && aRot[0] > 0 ) {
    aRot[0] = aRotation1 + 90.0;  aRot[1] = aRotation2 - 90.0; }
  else if( aDirection[0] > 0 && aDirection[1] < 0 && aDirection[2] > 0 && aRot[0] <= 0 ) {
    aRot[0] = aRotation1 - 270.0; aRot[1] = aRotation2 + 90.0; }
  else if( aDirection[0] > 0 && aDirection[1] < 0 && aDirection[2] > 0 && aRot[0] > 0 ) {
    aRot[0] = aRotation1 - 90.0;  aRot[1] = aRotation2 - 90.0; }
  else if( aDirection[0] > 0 && aDirection[1] < 0 && aDirection[2] < 0 && aRot[0] <= 0 ) {
    aRot[0] = aRotation1 + 90.0;  aRot[1] = aRotation2 - 90.0; }
  else if( aDirection[0] > 0 && aDirection[1] < 0 && aDirection[2] < 0 && aRot[0] > 0 ) {
    aRot[0] = aRotation1 + 270.0; aRot[1] = aRotation2 + 90.0; }
  else if( aDirection[0] < 0 && aDirection[1] > 0 && aDirection[2] > 0 && aRot[0] <= 0 ) {
    aRot[0] = aRotation1 - 90.0;  aRot[1] = aRotation2 - 90.0; }
  else if( aDirection[0] < 0 && aDirection[1] > 0 && aDirection[2] > 0 && aRot[0] > 0 ) {
    aRot[0] = aRotation1 + 90.0;  aRot[1] = aRotation2 - 270.0; }
  else if( aDirection[0] < 0 && aDirection[1] > 0 && aDirection[2] < 0 && aRot[0] <= 0 ) {
    aRot[0] = aRotation1 - 90.0;  aRot[1] = aRotation2 + 90.0; }
  else if( aDirection[0] < 0 && aDirection[1] > 0 && aDirection[2] < 0 && aRot[0] > 0 ) {
    aRot[0] = aRotation1 + 90.0;  aRot[1] = aRotation2 + 270.0; }
  else if( aDirection[0] < 0 && aDirection[1] < 0 && aDirection[2] > 0 && aRot[0] <= 0  ) {
    aRot[0] = aRotation1 - 270.0; aRot[1] = aRotation2 - 270.0; }
  else if( aDirection[0] < 0 && aDirection[1] < 0 && aDirection[2] > 0 && aRot[0] > 0 ) {
    aRot[0] = aRotation1 - 90.0;  aRot[1] = aRotation2 - 90.0; }
  else if( aDirection[0] < 0 && aDirection[1] < 0 && aDirection[2] < 0 && aRot[0] <= 0  ) {
    aRot[0] = aRotation1 + 90.0;  aRot[1] = aRotation2 + 270.0; }
  else if( aDirection[0] < 0 && aDirection[1] < 0 && aDirection[2] < 0 && aRot[0] > 0 ) {
    aRot[0] = aRotation1 + 270.0; aRot[1] = aRotation2 + 90.0; }

  SpinSliderRotation1->setValue( qRound( aRot[0] ) );
  SpinSliderRotation2->setValue( qRound( aRot[1] ) );

  int aCurPlaneIndex = ComboBoxPlanes->currentIndex();
  const SMESH::TPlaneData& aPlaneData = myPlanes[ aCurPlaneIndex ];
  const SMESH::TActorList& anActorList = aPlaneData.ActorList;

  double aBounds[6];
  double aDist;
  SMESH::ComputeBounds( anActorList, aBounds );
  SMESH::PositionToDistance( aBounds, theDir, theOrigin, aDist );
  aDist = 1.0 - aDist;
  if( aDist>1.0 )
    aDist = 1.0;
  else if( aDist < 0.0 )
    aDist = 0.0;

  SpinSliderDistance->setValue( qRound( aDist*100 ) );
  return;
}

/*!
  SLOT: called on preview check box toggled
*/
void SMESHGUI_ClippingDlg::OnPreviewToggle (bool theIsToggled)
{
  std::for_each( myPlanes.begin(), myPlanes.end(), TSetVisibility( theIsToggled ) );
  int aCurPlaneIndex = ComboBoxPlanes->currentIndex();
  SMESH::TPlaneData aPlane = myPlanes[aCurPlaneIndex];
  SMESH::OrientedPlane* aPlaneData = aPlane.Plane.GetPointer();
  aPlaneData->myActor->SetVisibility( false );
  if( theIsToggled )
    myPreviewWidget->On();
  else
    myPreviewWidget->Off();
  SMESH::RenderViewWindow( myViewWindow );
}

/*!
  SLOT: called on Auto Apply check box toggled
*/
void SMESHGUI_ClippingDlg::onAutoApply(bool toggled)
{
  if ( toggled ) ClickOnApply();
}

/*!
  SLOT on ok button click: sets cutting plane and closes dialog
*/
void SMESHGUI_ClippingDlg::ClickOnOk()
{
  ClickOnApply();
  accept();
}


/*!
  SLOT on Apply button click: sets cutting plane and update viewer
*/
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
          if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( aVTKActor ) ) {
            if( anOrientedPlane->IsOpenGLClipping )
              anActor->AddOpenGLClippingPlane( anOrientedPlane->InvertPlane() );
            else
              anActor->AddClippingPlane( anOrientedPlane );
          }

      SMESH::ClippingPlaneInfo aClippingPlaneInfo;
      aClippingPlaneInfo.Plane = anOrientedPlane;
      aClippingPlaneInfo.ActorList = anActorList;

      aClippingPlaneInfoList.push_back( aClippingPlaneInfo );
    }

    anAllActors->InitTraversal();
    while( vtkActor* aVTKActor = anAllActors->GetNextActor() )
      if( SMESH_Actor* anActor = SMESH_Actor::SafeDownCast( aVTKActor ) ) {
        anActor->SetOpenGLClippingPlane();
      }

    SMESH::RenderViewWindow( myViewWindow );
  }
}

/*!
  SLOT on help button click: opens a help page
*/
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

void SMESHGUI_ClippingDlg::onSelectAbsoluteOrientation( int mode )
{
  bool isUserMode = (mode==0);

  TextLabelX->setEnabled( isUserMode );
  TextLabelY->setEnabled( isUserMode );
  TextLabelZ->setEnabled( isUserMode );

  SpinBox_X->setEnabled( isUserMode );
  SpinBox_Y->setEnabled( isUserMode );
  SpinBox_Z->setEnabled( isUserMode );

  TextLabelDx->setEnabled( isUserMode );
  TextLabelDy->setEnabled( isUserMode );
  TextLabelDz->setEnabled( isUserMode );

  SpinBox_Dx->setEnabled( isUserMode );
  SpinBox_Dy->setEnabled( isUserMode );
  SpinBox_Dz->setEnabled( isUserMode );

  if ( isUserMode )
    return;

  double aDx = 0, aDy = 0, aDz = 0;

  if ( mode == 1 )
  {
    aDz = 1;
    TextLabelZ->setEnabled( true );
    SpinBox_Z->setEnabled( true );
    SpinBox_Z->setFocus();
  }
  else if ( mode == 2 )
  {
    aDx = 1;
    TextLabelX->setEnabled( true );
    SpinBox_X->setEnabled( true );
    SpinBox_X->setFocus();
  }
  else if ( mode == 3 )
  {
    aDy = 1;
    TextLabelY->setEnabled( true );
    SpinBox_Y->setEnabled( true );
    SpinBox_Y->setFocus();
  }

  int aCurPlaneIndex = ComboBoxPlanes->currentIndex();
  SMESH::TPlaneData aPlane = myPlanes[aCurPlaneIndex];
  SMESH::OrientedPlane* aPlaneData = aPlane.Plane.GetPointer();

  if ( aPlaneData->IsInvert == true ) {
    aDx = -aDx; aDy = -aDy; aDz = -aDz;
  }

  myIsSelectPlane = true;
  SpinBox_Dx->setValue( aDx );
  SpinBox_Dy->setValue( aDy );
  SpinBox_Dz->setValue( aDz );
  myIsSelectPlane = false;

  SetCurrentPlaneParam();
}

/*!
  SLOT: called on orientation of clipping plane in relative mode changed
*/
void SMESHGUI_ClippingDlg::onSelectRelativeOrientation ( int theItem )
{
  if ( myPlanes.empty() )
    return;

  if ( theItem == 0 ) {
    TextLabelRotation1->setText( tr( "ROTATION_AROUND_X_Y2Z" ) );
    TextLabelRotation2->setText( tr( "ROTATION_AROUND_Y_X2Z" ) );
  }
  else if ( theItem == 1 ) {
    TextLabelRotation1->setText( tr( "ROTATION_AROUND_Y_Z2X" ) );
    TextLabelRotation2->setText( tr( "ROTATION_AROUND_Z_Y2X" ) );
  }
  else if ( theItem == 2 ) {
    TextLabelRotation1->setText( tr( "ROTATION_AROUND_Z_X2Y" ) );
    TextLabelRotation2->setText( tr( "ROTATION_AROUND_X_Z2Y" ) );
  }

  if( (QComboBox*)sender() == CBRelativeOrientation )
    SetCurrentPlaneParam();
}

/*!
  SLOT on reset button click: sets default values
*/
void SMESHGUI_ClippingDlg::onReset()
{
  myIsSelectPlane = true;
  SpinBox_X->setValue(0);
  SpinBox_Y->setValue(0);
  SpinBox_Z->setValue(0);
  myIsSelectPlane = false;

  SetCurrentPlaneParam();
}

/*!
  SLOT on invert button click: inverts normal of cutting plane
*/
void SMESHGUI_ClippingDlg::onInvert()
{
  double Dx = SpinBox_Dx->value();
  double Dy = SpinBox_Dy->value();
  double Dz = SpinBox_Dz->value();

  myIsSelectPlane = true;
  SpinBox_Dx->setValue( -Dx );
  SpinBox_Dy->setValue( -Dy );
  SpinBox_Dz->setValue( -Dz );
  myIsSelectPlane = false;

  if ( !myPlanes.empty() ) {
    int aCurPlaneIndex = ComboBoxPlanes->currentIndex();
    SMESH::TPlaneData aPlane = myPlanes[aCurPlaneIndex];
    SMESH::OrientedPlane* aPlaneData = aPlane.Plane.GetPointer();
    aPlaneData->IsInvert = !aPlaneData->IsInvert;
  }
  SetCurrentPlaneParam();
}

