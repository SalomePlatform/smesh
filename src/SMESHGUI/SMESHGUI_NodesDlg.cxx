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
//  File   : SMESHGUI_NodesDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

using namespace std;
#include "SMESHGUI_NodesDlg.h"
#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"

#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "utilities.h"
#include "QAD_MessageBox.h"

#include "SMESH_Actor.h"
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>

// VTK Includes
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkDataSetMapper.h>
#include <vtkIdList.h>
#include <vtkVertex.h>

// QT Includes
#include <qbuttongroup.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qvalidator.h>
#include <qevent.h>


//=================================================================================
// class    : SMESHGUI_NodesDlg()
// purpose  : 
//=================================================================================
SMESHGUI_NodesDlg::SMESHGUI_NodesDlg( QWidget* parent,
				      const char* name,
				      SALOME_Selection* Sel,
				      bool modal,
				      WFlags fl )
  : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu )
{
  
  QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_NODE")));
  if ( !name )
    setName( "SMESHGUI_NodesDlg" );
  resize( 303, 185 ); 
  setCaption( tr( "MESH_NODE_TITLE"  ) );
  setSizeGripEnabled( TRUE );
  SMESHGUI_NodesDlgLayout = new QGridLayout( this ); 
  SMESHGUI_NodesDlgLayout->setSpacing( 6 );
  SMESHGUI_NodesDlgLayout->setMargin( 11 );

  /***************************************************************/
  GroupButtons = new QGroupBox( this, "GroupButtons" );
  GroupButtons->setGeometry( QRect( 10, 10, 281, 48 ) ); 
  GroupButtons->setTitle( tr( ""  ) );
  GroupButtons->setColumnLayout(0, Qt::Vertical );
  GroupButtons->layout()->setSpacing( 0 );
  GroupButtons->layout()->setMargin( 0 );
  GroupButtonsLayout = new QGridLayout( GroupButtons->layout() );
  GroupButtonsLayout->setAlignment( Qt::AlignTop );
  GroupButtonsLayout->setSpacing( 6 );
  GroupButtonsLayout->setMargin( 11 );
  buttonCancel = new QPushButton( GroupButtons, "buttonCancel" );
  buttonCancel->setText( tr( "SMESH_BUT_CLOSE"  ) );
  buttonCancel->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonCancel, 0, 3 );
  buttonApply = new QPushButton( GroupButtons, "buttonApply" );
  buttonApply->setText( tr( "SMESH_BUT_APPLY"  ) );
  buttonApply->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonApply, 0, 1 );
  QSpacerItem* spacer_9 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  GroupButtonsLayout->addItem( spacer_9, 0, 2 );
  buttonOk = new QPushButton( GroupButtons, "buttonOk" );
  buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
  buttonOk->setAutoDefault( TRUE );
  buttonOk->setDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonOk, 0, 0 );
  SMESHGUI_NodesDlgLayout->addWidget( GroupButtons, 2, 0 );

  /***************************************************************/
  GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
  GroupConstructors->setTitle( tr( "MESH_NODE"  ) );
  GroupConstructors->setExclusive( TRUE );
  GroupConstructors->setColumnLayout(0, Qt::Vertical );
  GroupConstructors->layout()->setSpacing( 0 );
  GroupConstructors->layout()->setMargin( 0 );
  GroupConstructorsLayout = new QGridLayout( GroupConstructors->layout() );
  GroupConstructorsLayout->setAlignment( Qt::AlignTop );
  GroupConstructorsLayout->setSpacing( 6 );
  GroupConstructorsLayout->setMargin( 11 );
  Constructor1 = new QRadioButton( GroupConstructors, "Constructor1" );
  Constructor1->setText( tr( ""  ) );
  Constructor1->setPixmap( image0 );
  Constructor1->setChecked( TRUE );
  GroupConstructorsLayout->addWidget( Constructor1, 0, 0 );
  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  GroupConstructorsLayout->addItem( spacer_2, 0, 1 );
  SMESHGUI_NodesDlgLayout->addWidget( GroupConstructors, 0, 0 );

  /***************************************************************/
  GroupCoordinates = new QGroupBox( this, "GroupCoordinates" );
  GroupCoordinates->setTitle( tr( "SMESH_COORDINATES"  ) );
  GroupCoordinates->setColumnLayout(0, Qt::Vertical );
  GroupCoordinates->layout()->setSpacing( 0 );
  GroupCoordinates->layout()->setMargin( 0 );
  GroupCoordinatesLayout = new QGridLayout( GroupCoordinates->layout() );
  GroupCoordinatesLayout->setAlignment( Qt::AlignTop );
  GroupCoordinatesLayout->setSpacing( 6 );
  GroupCoordinatesLayout->setMargin( 11 );
  TextLabel_X = new QLabel( GroupCoordinates, "TextLabel_X" );
  TextLabel_X->setText( tr( "SMESH_X"  ) );
  GroupCoordinatesLayout->addWidget( TextLabel_X, 0, 0 );
  TextLabel_Y = new QLabel( GroupCoordinates, "TextLabel_Y" );
  TextLabel_Y->setText( tr( "SMESH_Y"  ) );
  GroupCoordinatesLayout->addWidget( TextLabel_Y, 0, 2 );
  
  TextLabel_Z = new QLabel( GroupCoordinates, "TextLabel_Z" );
  TextLabel_Z->setText( tr( "SMESH_Z"  ) );
  GroupCoordinatesLayout->addWidget( TextLabel_Z, 0, 4 );
  
  SpinBox_X = new SMESHGUI_SpinBox( GroupCoordinates, "SpinBox_X" ) ;
  GroupCoordinatesLayout->addWidget( SpinBox_X, 0, 1 );

  SpinBox_Y = new SMESHGUI_SpinBox( GroupCoordinates, "SpinBox_Y" ) ;
  GroupCoordinatesLayout->addWidget( SpinBox_Y, 0, 3 );
 
  SpinBox_Z = new SMESHGUI_SpinBox( GroupCoordinates, "SpinBox_Z" ) ;
  GroupCoordinatesLayout->addWidget( SpinBox_Z, 0, 5 );

  SMESHGUI_NodesDlgLayout->addWidget( GroupCoordinates, 1, 0 );
 
  /* Initialisation and display */
  Init(Sel) ;
}


//=======================================================================
// function : ~SMESHGUI_NodesDlg()
// purpose  : Destructor
//=======================================================================
SMESHGUI_NodesDlg::~SMESHGUI_NodesDlg()
{}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::Init(SALOME_Selection* Sel)
{
  /* Get setting of step value from file configuration */
  double step ;
  // QString St = QAD_CONFIG->getSetting( "xxxxxxxxxxxxx" ) ;  TODO
  // step = St.toDouble() ;                                    TODO
  step = 25.0 ;
  
  /* min, max, step and decimals for spin boxes */
  SpinBox_X->RangeStepAndValidator( -999.999, +999.999, step, 3 ) ;
  SpinBox_Y->RangeStepAndValidator( -999.999, +999.999, step, 3 ) ;
  SpinBox_Z->RangeStepAndValidator( -999.999, +999.999, step, 3 ) ;  
  SpinBox_X->SetValue( 0.0 ) ;
  SpinBox_Y->SetValue( 0.0 ) ;
  SpinBox_Z->SetValue( 0.0 ) ;
  
  mySelection = Sel ;
  myMeshGUI = SMESHGUI::GetSMESHGUI() ;   
  myMeshGUI->SetActiveDialogBox( (QDialog*)this ) ;
  
  /* signals and slots connections */
  connect( buttonOk, SIGNAL( clicked() ), this, SLOT( ClickOnOk() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) );
  connect( buttonApply, SIGNAL( clicked() ), this, SLOT(ClickOnApply() ) );
  
  connect( SpinBox_X, SIGNAL ( valueChanged( double) ), this, SLOT( ValueChangedInSpinBox( double) ) ) ;
  connect( SpinBox_Y, SIGNAL ( valueChanged( double) ), this, SLOT( ValueChangedInSpinBox( double) ) ) ;
  connect( SpinBox_Z, SIGNAL ( valueChanged( double) ), this, SLOT( ValueChangedInSpinBox( double) ) ) ;

  connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  connect( myMeshGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  /* to close dialog if study change */
  connect( myMeshGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;
 
  /* Move widget on the botton right corner of main widget */
  int x, y ;
  myMeshGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ;

  SelectionIntoArgument();
}

//=================================================================================
// function : ValueChangedInSpinBox()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::ValueChangedInSpinBox( double newValue )
{
  double vx = SpinBox_X->GetValue() ;
  double vy = SpinBox_Y->GetValue() ;
  double vz = SpinBox_Z->GetValue() ;
  myMeshGUI->DisplaySimulationNode( myMesh, vx, vy, vz );
  //myMeshGUI->ViewNodes();

  return ;
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::ClickOnOk()
{
  this->ClickOnApply() ;
  this->ClickOnCancel() ;

  return ;
}

//=======================================================================
// function : ClickOnApply()
// purpose  :
//=======================================================================
void SMESHGUI_NodesDlg::ClickOnApply()
{
  if ( myMeshGUI->GetActiveStudy()->getActiveStudyFrame()->getTypeView() != VIEW_VTK ) {
    return;
  }
  
  if ( Constructor1->isChecked() ) {
    /* Recup args and call method */
    double x = SpinBox_X->GetValue() ;
    double y = SpinBox_Y->GetValue() ;
    double z = SpinBox_Z->GetValue() ;
    myMeshGUI->EraseSimulationActors() ;
    myMeshGUI->AddNode( myMesh, x, y, z ) ; 
    myMeshGUI->ViewNodes();
    mySelection->ClearIObjects();
  }
}


//=======================================================================
// function : ClickOnCancel()
// purpose  :
//=======================================================================
void SMESHGUI_NodesDlg::ClickOnCancel()
{
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
  disconnect( mySelection, 0, this, 0 );
  myMeshGUI->ResetState() ;
  myMeshGUI->EraseSimulationActors() ;
  reject() ;
  return ;
}


//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_NodesDlg::SelectionIntoArgument()
{
  int nbSel = mySelection->IObjectCount();
  if(nbSel != 1) {
    SpinBox_X->SetValue(0.0) ;
    SpinBox_Y->SetValue(0.0) ;
    SpinBox_Z->SetValue(0.0) ;
    return;
  }

  Standard_Boolean res;
  myMesh = myMeshGUI->ConvertIOinMesh( mySelection->firstIObject(), res );
  if (!res) {
    SpinBox_X->SetValue(0.0) ;
    SpinBox_Y->SetValue(0.0) ;
    SpinBox_Z->SetValue(0.0) ;
    return ;
  }
  
  if ( mySelection->SelectionMode() != NodeSelection ) {
    SpinBox_X->SetValue(0.0) ;
    SpinBox_Y->SetValue(0.0) ;
    SpinBox_Z->SetValue(0.0) ;
    QAD_MessageBox::warn1 ( QAD_Application::getDesktop(), tr ("SMESH_WRN_WARNING"),
			    tr ("SMESH_WRN_SELECTIONMODE_NODES"), tr ("SMESH_BUT_OK") );
    return;
  }

  QString aString = "";
  int nbNodes = myMeshGUI->GetNameOfSelectedNodes(mySelection, aString) ;
  if(nbNodes != 1) {
    SpinBox_X->SetValue(0.0) ;
    SpinBox_Y->SetValue(0.0) ;
    SpinBox_Z->SetValue(0.0) ;
    return ;
  } 

  if ( nbNodes == 1 ) {
    TColStd_MapOfInteger myMapIndex;
    mySelection->GetIndex( mySelection->firstIObject(), myMapIndex);
    TColStd_MapIteratorOfMapOfInteger ite( myMapIndex );
    int idNodes[1];
    for ( ; ite.More(); ite.Next() ) {
      idNodes[0] = ite.Key();
    }

    Standard_Boolean result;
    SMESH_Actor* ac = myMeshGUI->FindActor( myMesh, result, true );
    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast( ac->GetUnstructuredGrid() );
    float *p0 = ugrid->GetPoint(idNodes[0]);

    SpinBox_X->SetValue( p0[0] ) ;
    SpinBox_Y->SetValue( p0[1] ) ;
    SpinBox_Z->SetValue( p0[2] ) ;
       
    myMeshGUI->DisplaySimulationNode( myMesh, p0[0], p0[1], p0[2] );
    //  myMeshGUI->ViewNodes();
  } 

  return ;
}


//=======================================================================
// function : closeEvent()
// purpose  :
//=======================================================================
void SMESHGUI_NodesDlg::closeEvent(QCloseEvent* e)
{
  this->ClickOnCancel() ; /* same than click on cancel button */
  return ;
}


//=================================================================================
// function : enterEvent()
// purpose  : to reactivate this dialog box when mouse enter onto the window
//=================================================================================
void SMESHGUI_NodesDlg::enterEvent( QEvent* e)
{
  if ( GroupConstructors->isEnabled() )
    return ;  
  ActivateThisDialog() ;
  return ;  
}


//=================================================================================
// function : DeactivateActiveDialog()
// purpose  : public slot to deactivate if active
//=================================================================================
void SMESHGUI_NodesDlg::DeactivateActiveDialog()
{
  if ( GroupConstructors->isEnabled() ) {
    GroupConstructors->setEnabled(false) ;
    GroupCoordinates->setEnabled(false) ;
    GroupButtons->setEnabled(false) ;
    myMeshGUI->EraseSimulationActors() ;
    myMeshGUI->ResetState() ;    
    myMeshGUI->SetActiveDialogBox(0) ;
  }
  return ;
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_NodesDlg::ActivateThisDialog( )
{
  myMeshGUI->EmitSignalDeactivateDialog() ;
  GroupConstructors->setEnabled(true) ;
  GroupCoordinates->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
  return ;
}
