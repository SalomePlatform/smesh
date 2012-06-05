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
// File   : SMESHGUI_Preferences_ColorDlg.cxx
// Author : Nicolas REJNERI, Open CASCADE S.A.S.
// SMESH includes
//
#include "SMESHGUI_Preferences_ColorDlg.h"

#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"

// SALOME GUI includes
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <QtxColorButton.h>
#include <VTKViewer_MarkerWidget.h>
#include <LightApp_Application.h>
#include <SalomeApp_IntSpinBox.h>

// Qt includes
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QKeyEvent>

#define SPACING 6
#define MARGIN  11

//=================================================================================
// function : SMESHGUI_Preferences_ColorDlg()
// purpose  : Constructs a SMESHGUI_Preferences_ColorDlg which is a child
//            of 'parent', with the name 'name' and widget flags set to 'f'
//            The dialog will by default be modeless, unless you
//            set'modal' to true to construct a modal dialog.
//=================================================================================
SMESHGUI_Preferences_ColorDlg::SMESHGUI_Preferences_ColorDlg( SMESHGUI* theModule )
  : QDialog( SMESH::GetDesktop( theModule ) ),
    mySMESHGUI( theModule )
{
  setModal( true );
  setWindowTitle( tr( "DIALOG_TITLE" ) );
  setSizeGripEnabled( true );

  // -------------------------------
  QVBoxLayout* topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( SPACING );
  topLayout->setMargin( MARGIN );

  // -------------------------------
  QGroupBox* ButtonGroup1 = new QGroupBox( tr( "GRP_ELEMENTS" ), this );
  QGridLayout* ButtonGroup1Layout = new QGridLayout( ButtonGroup1 );
  ButtonGroup1Layout->setSpacing( SPACING );
  ButtonGroup1Layout->setMargin( MARGIN );

  QLabel* TextLabel_Fill = new QLabel( tr( "SURFACE_COLOR_LBL" ), ButtonGroup1 );

  toolSurfColor = new QtxBiColorTool(ButtonGroup1);
  toolSurfColor->setText( tr( "BACKSURFACE_COLOR_LBL" ));

  QLabel* TextLabel_Outline = new QLabel( tr( "OUTLINE_COLOR_LBL" ), ButtonGroup1 );
  btnOutlineColor = new QtxColorButton( ButtonGroup1 );

  QLabel* TextLabel_Wireframe = new QLabel( tr( "WIREFRAME_COLOR_LBL" ), ButtonGroup1 );
  btnWireframeColor = new QtxColorButton( ButtonGroup1 );

  QLabel* TextLabel_0DElements_Color = new QLabel( tr( "0D_ELEMENTS_COLOR_LBL" ), ButtonGroup1 );
  btn0DElementsColor = new QtxColorButton( ButtonGroup1 );

  QLabel* TextLabel_0DElements_Size = new QLabel( tr( "0D_ELEMENTS_SIZE_LBL" ), ButtonGroup1 );
  SpinBox_0DElements_Size = new SalomeApp_IntSpinBox( ButtonGroup1 );
  SpinBox_0DElements_Size->setAcceptNames( false ); // No Notebook variables allowed
  SpinBox_0DElements_Size->setRange( 1, 10 );
  SpinBox_0DElements_Size->setSingleStep( 1 );
  SpinBox_0DElements_Size->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  SpinBox_0DElements_Size->setButtonSymbols( QSpinBox::PlusMinus );

  QLabel* TextLabel_Width = new QLabel( tr( "LINE_WIDTH_LBL" ), ButtonGroup1 );
  SpinBox_Width = new SalomeApp_IntSpinBox( ButtonGroup1 );
  SpinBox_Width->setAcceptNames( false ); // No Notebook variables allowed
  SpinBox_Width->setRange( 1, 5 );
  SpinBox_Width->setSingleStep( 1 );
  SpinBox_Width->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  SpinBox_Width->setButtonSymbols( QSpinBox::PlusMinus );

  QLabel* TextLabel_ShrinkCoeff = new QLabel( tr( "SHRINK_COEF_LBL" ), ButtonGroup1 );
  SpinBox_Shrink = new SalomeApp_IntSpinBox( ButtonGroup1 );
  SpinBox_Shrink->setAcceptNames( false ); // No Notebook variables allowed
  SpinBox_Shrink->setRange( 20, 100 );
  SpinBox_Shrink->setSingleStep( 1 );
  SpinBox_Shrink->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  SpinBox_Shrink->setButtonSymbols( QSpinBox::PlusMinus );

  ButtonGroup1Layout->addWidget( TextLabel_Fill,             0, 0 );
  ButtonGroup1Layout->addWidget( toolSurfColor,              0, 1, 1, 3 );

  ButtonGroup1Layout->addWidget( TextLabel_Outline,          1, 0 );
  ButtonGroup1Layout->addWidget( btnOutlineColor,            1, 1 );
  ButtonGroup1Layout->addWidget( TextLabel_Wireframe,        1, 2 );
  ButtonGroup1Layout->addWidget( btnWireframeColor,         1, 3 );

  ButtonGroup1Layout->addWidget( TextLabel_0DElements_Color, 2, 0 );
  ButtonGroup1Layout->addWidget( btn0DElementsColor,         2, 1 );

  ButtonGroup1Layout->addWidget( TextLabel_0DElements_Size,  2, 2 );
  ButtonGroup1Layout->addWidget( SpinBox_0DElements_Size,    2, 3 );

  ButtonGroup1Layout->addWidget( TextLabel_Width,            3, 0 );
  ButtonGroup1Layout->addWidget( SpinBox_Width,              3, 1 );
  ButtonGroup1Layout->addWidget( TextLabel_ShrinkCoeff,      3, 2 );
  ButtonGroup1Layout->addWidget( SpinBox_Shrink,             3, 3 );

  // -------------------------------
  QGroupBox* ButtonGroup2 = new QGroupBox( tr( "GRP_NODES" ), this );
  QGridLayout* ButtonGroup2Layout = new QGridLayout( ButtonGroup2 );
  ButtonGroup2Layout->setSpacing( SPACING );
  ButtonGroup2Layout->setMargin( MARGIN );

  QLabel* TextLabel_Nodes_Color = new QLabel( tr( "NODES_COLOR_LBL" ), ButtonGroup2 );
  btnNodeColor = new QtxColorButton( ButtonGroup2 );

  QGroupBox* MarkerGroup = new QGroupBox( tr( "NODES_MARKER_LBL" ), ButtonGroup2 );
  QVBoxLayout* MarkerGroupLayout = new QVBoxLayout( MarkerGroup );
  MarkerGroupLayout->setSpacing( 0 );
  MarkerGroupLayout->setMargin( 0 );

  MarkerWidget = new VTKViewer_MarkerWidget( MarkerGroup );

  MarkerGroupLayout->addWidget( MarkerWidget );

  ButtonGroup2Layout->addWidget( TextLabel_Nodes_Color, 0, 0 );
  ButtonGroup2Layout->addWidget( btnNodeColor,          0, 1 );
  ButtonGroup2Layout->addWidget( MarkerGroup,           1, 0, 1, 3 );
  ButtonGroup2Layout->setColumnStretch( 2, 1 );

  // -------------------------------
  QGroupBox* ButtonGroup3 = new QGroupBox( tr( "GRP_ORIENTATION" ), this );
  QGridLayout* ButtonGroup3Layout = new QGridLayout( ButtonGroup3 );
  ButtonGroup3Layout->setSpacing( SPACING );
  ButtonGroup3Layout->setMargin( MARGIN );

  QLabel* TextLabel_Orientation_Color = new QLabel( tr( "ORIENTATION_COLOR_LBL" ), ButtonGroup3 );
  btnOrientationColor = new QtxColorButton( ButtonGroup3 );

  QLabel* TextLabel_Orientation_Scale = new QLabel( tr( "ORIENTATION_SCALE_LBL" ), ButtonGroup3 );
  SpinBox_Orientation_Scale = new SMESHGUI_SpinBox( ButtonGroup3 );
  SpinBox_Orientation_Scale->setAcceptNames( false ); // No Notebook variables allowed
  SpinBox_Orientation_Scale->RangeStepAndValidator( .05, .5, .05, "parametric_precision" );
  SpinBox_Orientation_Scale->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
  SpinBox_Orientation_Scale->setButtonSymbols( QSpinBox::PlusMinus );

  CheckBox_Orientation_3DVectors = new QCheckBox( tr( "3D_VECTORS_LBL" ), ButtonGroup3 );

  ButtonGroup3Layout->addWidget( TextLabel_Orientation_Color,    0, 0 );
  ButtonGroup3Layout->addWidget( btnOrientationColor,            0, 1 );
  ButtonGroup3Layout->addWidget( TextLabel_Orientation_Scale,    0, 2 );
  ButtonGroup3Layout->addWidget( SpinBox_Orientation_Scale,      0, 3 );
  ButtonGroup3Layout->addWidget( CheckBox_Orientation_3DVectors, 1, 0, 1, 4 );

  // -------------------------------
  QGroupBox* ButtonGroup4 = new QGroupBox( tr( "GRP_SELECTION" ), this );
  QGridLayout* ButtonGroup4Layout = new QGridLayout( ButtonGroup4 );
  ButtonGroup3Layout->setSpacing( SPACING );
  ButtonGroup3Layout->setMargin( MARGIN );
  
  QLabel* TextLabel_Selection_Color = new QLabel( tr( "SELECTION_COLOR_LBL" ), ButtonGroup4 );
  btnSelectionColor = new QtxColorButton( ButtonGroup4 );
  
  QLabel* TextLabel_Preselection_Color = new QLabel( tr( "PRESELECTION_COLOR_LBL" ), ButtonGroup4 );
  btnPreselectionColor = new QtxColorButton( ButtonGroup4 );
  
  ButtonGroup4Layout->addWidget( TextLabel_Selection_Color,      0, 0 );
  ButtonGroup4Layout->addWidget( btnSelectionColor,              0, 1 );
  ButtonGroup4Layout->addWidget( TextLabel_Preselection_Color,   0, 2 );
  ButtonGroup4Layout->addWidget( btnPreselectionColor,           0, 3 );

  // -------------------------------
  QGroupBox* GroupButtons = new QGroupBox( this );
  QHBoxLayout* GroupButtonsLayout = new QHBoxLayout( GroupButtons );
  GroupButtonsLayout->setSpacing( SPACING );
  GroupButtonsLayout->setMargin( MARGIN );

  QPushButton* buttonOk = new QPushButton( tr( "SMESH_BUT_OK" ), GroupButtons );
  buttonOk->setAutoDefault( true );
  buttonOk->setDefault( true );

  QPushButton* buttonCancel = new QPushButton( tr( "SMESH_BUT_CANCEL" ), GroupButtons );
  buttonCancel->setAutoDefault( true );

  QPushButton* buttonHelp = new QPushButton( tr( "SMESH_BUT_HELP" ), GroupButtons );
  buttonHelp->setAutoDefault( true );

  GroupButtonsLayout->addWidget( buttonOk );
  GroupButtonsLayout->addSpacing( 10 );
  GroupButtonsLayout->addStretch();
  GroupButtonsLayout->addWidget( buttonCancel );
  GroupButtonsLayout->addWidget( buttonHelp );

  // -------------------------------
  topLayout->addWidget( ButtonGroup1 );
  topLayout->addWidget( ButtonGroup2 );
  topLayout->addWidget( ButtonGroup3 );
  //  rnv: Selection and preselection colors are defined only in the Preferences 
  //  topLayout->addWidget( ButtonGroup4 );
  ButtonGroup4->hide();
  topLayout->addWidget( GroupButtons );

  // -------------------------------
  mySMESHGUI->SetActiveDialogBox( this );

  myHelpFileName = "colors_size_page.html";

  /* signals and slots connections */
  connect( buttonOk,     SIGNAL( clicked() ), this, SLOT( ClickOnOk() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) );
  connect( buttonHelp,   SIGNAL( clicked() ), this, SLOT( ClickOnHelp() ) );

  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ),
           this,       SLOT( DeactivateActiveDialog() ) );
  /* to close dialog if study change */
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ),
           this,       SLOT( ClickOnCancel() ) );
}

//=================================================================================
// function : ~SMESHGUI_Preferences_ColorDlg()
// purpose  : Destructor
//=================================================================================
SMESHGUI_Preferences_ColorDlg::~SMESHGUI_Preferences_ColorDlg()
{
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::ClickOnOk()
{
  mySMESHGUI->ResetState();
  accept();
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::ClickOnCancel()
{
  mySMESHGUI->ResetState();
  reject();
}

//=================================================================================
// function : ClickOnHelp()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::ClickOnHelp()
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
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::DeactivateActiveDialog()
{
}

//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::closeEvent( QCloseEvent* )
{
  ClickOnCancel(); /* same than click on cancel button */
}

//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate any active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog();
}

//=================================================================================
// function : SetColor()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SetColor( int type, const QColor& color )
{
  switch ( type ) {
  case 1 : toolSurfColor->setMainColor( color );     break; // fill
  case 2 : btnWireframeColor->setColor( color );     break; // wireframe
  case 3 : btnNodeColor->setColor( color );          break; // node
  case 4 : btnOutlineColor->setColor( color );       break; // outline
  case 5 : btn0DElementsColor->setColor( color );    break; // 0d elements
  case 6 : btnOrientationColor->setColor( color );   break; // orientation of faces
  case 7 : btnSelectionColor->setColor( color );     break; // selection color
  case 8 : btnPreselectionColor->setColor( color );  break; // pre-selection color
  default: break;
  }
}

//=================================================================================
// function : GetColor()
// purpose  :
//=================================================================================
QColor SMESHGUI_Preferences_ColorDlg::GetColor( int type )
{
  QColor color;
  switch ( type ) {
  case 1 : color = toolSurfColor->mainColor();    break; // fill
  case 2 : color = btnWireframeColor->color();    break; // outline
  case 3 : color = btnNodeColor->color();         break; // node
  case 4 : color = btnOutlineColor->color();      break; // node
  case 5 : color = btn0DElementsColor->color();   break; // 0d elements
  case 6 : color = btnOrientationColor->color();  break; // orientation of faces
  case 7 : color = btnSelectionColor->color();    break; // selection color
  case 8 : color = btnPreselectionColor->color(); break; // pre-selection color

  default: break;
  }
  return color;
}

//=================================================================================
// function : SetIntValue()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SetIntValue( int type, int value )
{
  switch ( type ) {
  case 1 : SpinBox_Width->setValue( value );           break; // width
  case 2 : SpinBox_Shrink->setValue( value );          break; // shrink coeff
  case 3 : SpinBox_0DElements_Size->setValue( value ); break; // 0d elements
  default: break;
  }
}

//=================================================================================
// function : GetIntValue()
// purpose  :
//=================================================================================
int SMESHGUI_Preferences_ColorDlg::GetIntValue( int type )
{
  int res = 0;
  switch ( type ) {
  case 1 : res = SpinBox_Width->value();           break; // width
  case 2 : res = SpinBox_Shrink->value();          break; // shrink coeff
  case 3 : res = SpinBox_0DElements_Size->value(); break; // 0d elements
  default: break;
  }
  return res;
}

//=================================================================================
// function : SetDoubleValue()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SetDoubleValue( int type, double value )
{
  switch ( type ) {
  case 1 : SpinBox_Orientation_Scale->setValue( value ); break; // orientation scale
  default: break;
  }
}

//=================================================================================
// function : GetDoubleValue()
// purpose  :
//=================================================================================
double SMESHGUI_Preferences_ColorDlg::GetDoubleValue( int type )
{
  double res = 0;
  switch ( type ) {
  case 1 : res = SpinBox_Orientation_Scale->value(); break; // orientation scale
  default: break;
  }
  return res;
}

//=================================================================================
// function : SetBooleanValue()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SetBooleanValue( int type, bool value )
{
  switch ( type ) {
  case 1 : CheckBox_Orientation_3DVectors->setChecked( value ); break; // 3D vectors
  default: break;
  }
}

//=================================================================================
// function : GetBooleanValue()
// purpose  :
//=================================================================================
bool SMESHGUI_Preferences_ColorDlg::GetBooleanValue( int type )
{
  bool res = false;
  switch ( type ) {
  case 1 : res = CheckBox_Orientation_3DVectors->isChecked(); break; // 3D vectors
  default: break;
  }
  return res;
}

//=================================================================================
// function : setCustomMarkerMap()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::setCustomMarkerMap( VTK::MarkerMap theMarkerMap )
{
  MarkerWidget->setCustomMarkerMap( theMarkerMap );
}

//=================================================================================
// function : getCustomMarkerMap()
// purpose  :
//=================================================================================
VTK::MarkerMap SMESHGUI_Preferences_ColorDlg::getCustomMarkerMap()
{
  return MarkerWidget->getCustomMarkerMap();
}

//=================================================================================
// function : setStandardMarker()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::setStandardMarker( VTK::MarkerType theMarkerType,
                                                       VTK::MarkerScale theMarkerScale )
{
  MarkerWidget->setStandardMarker( theMarkerType, theMarkerScale );
}

//=================================================================================
// function : setCustomMarker()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::setCustomMarker( int theId )
{
  MarkerWidget->setCustomMarker( theId );
}

//=================================================================================
// function : getMarkerType()
// purpose  :
//=================================================================================
VTK::MarkerType SMESHGUI_Preferences_ColorDlg::getMarkerType() const
{
  return MarkerWidget->getMarkerType();
}

//=================================================================================
// function : getStandardMarkerScale()
// purpose  :
//=================================================================================
VTK::MarkerScale SMESHGUI_Preferences_ColorDlg::getStandardMarkerScale() const
{
  return MarkerWidget->getStandardMarkerScale();
}

//=================================================================================
// function : getCustomMarkerID()
// purpose  :
//=================================================================================
int SMESHGUI_Preferences_ColorDlg::getCustomMarkerID() const
{
  return MarkerWidget->getCustomMarkerID();
}

//=================================================================================
// function : SetDeltaBrightness(int)
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::SetDeltaBrightness(int delta) 
{
  toolSurfColor->setDelta(delta);
}
//=================================================================================
// function : GetDeltaBrightness()
// purpose  :
//=================================================================================
int SMESHGUI_Preferences_ColorDlg::GetDeltaBrightness() 
{
  return toolSurfColor->delta();
}

//=================================================================================
// function : keyPressEvent()
// purpose  :
//=================================================================================
void SMESHGUI_Preferences_ColorDlg::keyPressEvent( QKeyEvent* e )
{
  QDialog::keyPressEvent( e );
  if ( e->isAccepted() )
    return;

  if ( e->key() == Qt::Key_F1 ) {
    e->accept();
    ClickOnHelp();
  }
}
