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
//  File   : SMESHGUI_RevolutionDlg.cxx
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#include "SMESHGUI_RevolutionDlg.h"
#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESHGUI_MeshUtils.h"
#include "SMESHGUI_IdValidator.h"
#include "SMESH_Actor.h"
#include "SMESH_TypeFilter.hxx"
#include "SMDS_Mesh.hxx"

#include "QAD_Application.h"
#include "QAD_Desktop.h"
#include "QAD_MessageBox.h"
#include "utilities.h"

// QT Includes
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qspinbox.h> 
#include <qvalidator.h>
#include <qpixmap.h>

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Group)

using namespace std;

//=================================================================================
// class    : SMESHGUI_RevolutionDlg()
// purpose  : 
//=================================================================================
SMESHGUI_RevolutionDlg::SMESHGUI_RevolutionDlg( QWidget* parent, const char* name, SALOME_Selection* Sel,
					      bool modal, WFlags fl )
  : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu |
	     Qt::WDestructiveClose)
{
  QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_EDGE")));
  QPixmap image1(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_TRIANGLE")));
  QPixmap image2(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_SELECT")));

  if ( !name )
    setName( "SMESHGUI_RevolutionDlg" );
  resize( 303, 185 ); 
  setCaption( tr( "REVOLUTION_AROUND_AXIS" ) );
  setSizeGripEnabled( TRUE );
  SMESHGUI_RevolutionDlgLayout = new QGridLayout( this ); 
  SMESHGUI_RevolutionDlgLayout->setSpacing( 6 );
  SMESHGUI_RevolutionDlgLayout->setMargin( 11 );

  /***************************************************************/
  GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
  GroupConstructors->setTitle( tr( "SMESH_REVOLUTION"  ) );
  GroupConstructors->setExclusive( TRUE );
  GroupConstructors->setColumnLayout(0, Qt::Vertical );
  GroupConstructors->layout()->setSpacing( 0 );
  GroupConstructors->layout()->setMargin( 0 );
  GroupConstructorsLayout = new QGridLayout( GroupConstructors->layout() );
  GroupConstructorsLayout->setAlignment( Qt::AlignTop );
  GroupConstructorsLayout->setSpacing( 6 );
  GroupConstructorsLayout->setMargin( 11 );
  RadioButton1= new QRadioButton( GroupConstructors, "RadioButton1" );
  RadioButton1->setText( tr( ""  ) );
  RadioButton1->setPixmap( image0 );
  GroupConstructorsLayout->addWidget( RadioButton1, 0, 0 );
  RadioButton2= new QRadioButton( GroupConstructors, "RadioButton2" );
  RadioButton2->setText( tr( ""  ) );
  RadioButton2->setPixmap( image1 );
  GroupConstructorsLayout->addWidget( RadioButton2, 0, 2 );
  SMESHGUI_RevolutionDlgLayout->addWidget( GroupConstructors, 0, 0 );
  
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
  SMESHGUI_RevolutionDlgLayout->addWidget( GroupButtons, 2, 0 );

  /***************************************************************/
  GroupArguments = new QGroupBox( this, "GroupArguments" );
  GroupArguments->setTitle( tr( "REVOLUTION_1D"  ) );
  GroupArguments->setColumnLayout(0, Qt::Vertical );
  GroupArguments->layout()->setSpacing( 0 );
  GroupArguments->layout()->setMargin( 0 );
  GroupArgumentsLayout = new QGridLayout( GroupArguments->layout() );
  GroupArgumentsLayout->setAlignment( Qt::AlignTop );
  GroupArgumentsLayout->setSpacing( 6 );
  GroupArgumentsLayout->setMargin( 11 );

  // Controls for elements selection
  TextLabelElements  = new QLabel( GroupArguments, "TextLabelElements" );
  TextLabelElements->setText( tr( "SMESH_ID_ELEMENTS"  ) );
  TextLabelElements->setFixedWidth(74);
  GroupArgumentsLayout->addWidget( TextLabelElements, 0, 0 );

  SelectElementsButton  = new QPushButton( GroupArguments, "SelectElementsButton" );
  SelectElementsButton->setText( tr( ""  ) );
  SelectElementsButton->setPixmap( image2 );
  SelectElementsButton->setToggleButton( FALSE );
  GroupArgumentsLayout->addWidget( SelectElementsButton, 0, 1 );

  LineEditElements  = new QLineEdit( GroupArguments, "LineEditElements" );
  LineEditElements->setValidator( new SMESHGUI_IdValidator( this, "validator" ));
  GroupArgumentsLayout->addWidget( LineEditElements, 0, 2 );

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox( GroupArguments, "CheckBoxMesh" );
  CheckBoxMesh->setText( tr( "SMESH_SELECT_WHOLE_MESH"  ) );
  GroupArgumentsLayout->addMultiCellWidget( CheckBoxMesh, 1, 1, 0, 3 );

  // Controls for axis defining
  GroupAxis = new QGroupBox( GroupArguments, "GroupAxis" );
  GroupAxis->setTitle( tr( "SMESH_AXIS" ) );
  GroupAxis->setColumnLayout(0, Qt::Vertical );
  GroupAxis->layout()->setSpacing( 0 );
  GroupAxis->layout()->setMargin( 0 );
  QGridLayout* GroupAxisLayout = new QGridLayout( GroupAxis->layout() );
  GroupAxisLayout->setAlignment( Qt::AlignTop );
  GroupAxisLayout->setSpacing( 6 );
  GroupAxisLayout->setMargin( 11 );
  
  TextLabelPoint = new QLabel( GroupAxis, "TextLabelPoint" );
  TextLabelPoint->setText( tr( "SMESH_POINT" ) );
  GroupAxisLayout->addWidget( TextLabelPoint, 0, 0 );

  SelectPointButton  = new QPushButton( GroupAxis, "SelectPointButton" );
  SelectPointButton->setPixmap( image2 );
  GroupAxisLayout->addWidget( SelectPointButton, 0, 1 );
  
  TextLabelX = new QLabel( GroupAxis, "TextLabelX" );
  TextLabelX->setText( tr( "SMESH_X" ) );
  GroupAxisLayout->addWidget( TextLabelX, 0, 2 );

  SpinBox_X = new SMESHGUI_SpinBox( GroupAxis, "SpinBox_X");
  GroupAxisLayout->addWidget( SpinBox_X, 0, 3 );
  
  TextLabelY = new QLabel( GroupAxis, "TextLabelY" );
  TextLabelY->setText( tr( "SMESH_Y" ) );
  GroupAxisLayout->addWidget( TextLabelY, 0, 4 );

  SpinBox_Y = new SMESHGUI_SpinBox( GroupAxis, "SpinBox_Y");
  GroupAxisLayout->addWidget( SpinBox_Y, 0, 5 );

  TextLabelZ = new QLabel( GroupAxis, "TextLabelZ" );
  TextLabelZ->setText( tr( "SMESH_Z" ) );
  GroupAxisLayout->addWidget( TextLabelZ, 0, 6 );
  
  SpinBox_Z = new SMESHGUI_SpinBox( GroupAxis, "SpinBox_Z");
  GroupAxisLayout->addWidget( SpinBox_Z, 0, 7 );

  TextLabelVector = new QLabel( GroupAxis, "TextLabelVector" );
  TextLabelVector->setText( tr( "SMESH_VECTOR" ) );
  GroupAxisLayout->addWidget( TextLabelVector, 1, 0 );
  
  SelectVectorButton = new QPushButton( GroupAxis, "SelectVectorButton" );
  SelectVectorButton->setPixmap( image2 );
  GroupAxisLayout->addWidget( SelectVectorButton, 1, 1 );
  
  TextLabelDX = new QLabel( GroupAxis, "TextLabelDX" );
  TextLabelDX->setText( tr( "SMESH_DX" ) );
  GroupAxisLayout->addWidget( TextLabelDX, 1, 2 );

  SpinBox_DX = new SMESHGUI_SpinBox( GroupAxis, "SpinBox_DX");
  GroupAxisLayout->addWidget( SpinBox_DX, 1, 3 );
  
  TextLabelDY = new QLabel( GroupAxis, "TextLabelDY" );
  TextLabelDY->setText( tr( "SMESH_DY" ) );
  GroupAxisLayout->addWidget( TextLabelDY, 1, 4 );

  SpinBox_DY = new SMESHGUI_SpinBox( GroupAxis, "SpinBox_DY");
  GroupAxisLayout->addWidget( SpinBox_DY, 1, 5 );

  TextLabelDZ = new QLabel( GroupAxis, "TextLabelDZ" );
  TextLabelDZ->setText( tr( "SMESH_DZ" ) );
  GroupAxisLayout->addWidget( TextLabelDZ, 1, 6 );
  
  SpinBox_DZ = new SMESHGUI_SpinBox( GroupAxis, "SpinBox_DZ");
  GroupAxisLayout->addWidget( SpinBox_DZ, 1, 7 );

  GroupArgumentsLayout->addMultiCellWidget( GroupAxis, 2, 2, 0, 3 );
    
  // Controls for angle defining
  TextLabelAngle = new QLabel( GroupArguments, "TextLabelAngle" );
  TextLabelAngle->setText( tr( "SMESH_ANGLE"  ) );
  GroupArgumentsLayout->addMultiCellWidget( TextLabelAngle, 3, 3, 0, 1 );

  SpinBox_Angle = new SMESHGUI_SpinBox(GroupArguments, "SpinBox_Angle");
  GroupArgumentsLayout->addWidget( SpinBox_Angle, 3, 2 );
  
  // Controls for nb. steps defining
  TextLabelNbSteps = new QLabel( GroupArguments, "TextLabelNbSteps" );
  TextLabelNbSteps->setText( tr( "SMESH_NUMBEROFSTEPS"  ) );
  GroupArgumentsLayout->addMultiCellWidget( TextLabelNbSteps, 4, 4, 0, 1 );

  SpinBox_NbSteps = new QSpinBox(GroupArguments, "SpinBox_NbSteps");
  GroupArgumentsLayout->addWidget( SpinBox_NbSteps, 4, 2 );
  
  // Controls for tolerance defining
  TextLabelTolerance = new QLabel( GroupArguments, "TextLabelTolerance" );
  TextLabelTolerance->setText( tr( "SMESH_TOLERANCE" ) );
  GroupArgumentsLayout->addMultiCellWidget( TextLabelTolerance, 5, 5, 0, 1 );

  SpinBox_Tolerance = new SMESHGUI_SpinBox(GroupArguments, "SpinBox_Tolerance");
  GroupArgumentsLayout->addWidget( SpinBox_Tolerance, 5, 2 );

  SMESHGUI_RevolutionDlgLayout->addWidget( GroupArguments, 1, 0 );
  
  /* Initialisations */
  SpinBox_X->RangeStepAndValidator( -999999.999, +999999.999, 10.0, 3 );
  SpinBox_Y->RangeStepAndValidator( -999999.999, +999999.999, 10.0, 3 );
  SpinBox_Z->RangeStepAndValidator( -999999.999, +999999.999, 10.0, 3 );
  SpinBox_DX->RangeStepAndValidator( -999999.999, +999999.999, 10.0, 3 );
  SpinBox_DY->RangeStepAndValidator( -999999.999, +999999.999, 10.0, 3 );
  SpinBox_DZ->RangeStepAndValidator( -999999.999, +999999.999, 10.0, 3 );

  SpinBox_Angle->RangeStepAndValidator( -999999.999, +999999.999, 5.0, 3 );
  
  QIntValidator* anIntValidator = new QIntValidator(SpinBox_NbSteps);
  SpinBox_NbSteps->setValidator(anIntValidator);
  SpinBox_NbSteps->setRange( 1, 999999 );
  
  SpinBox_Tolerance->RangeStepAndValidator( 0.0, +999999.999, 0.1, 6 );
 
  GroupArguments->show();
  RadioButton1->setChecked( TRUE );
  mySelection = Sel;  

  mySMESHGUI  = SMESHGUI::GetSMESHGUI() ;
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;
  
  // Costruction of the logical filter
  SMESH_ListOfFilter aListOfFilters;
  Handle(SMESH_TypeFilter) aMeshOrSubMeshFilter = new SMESH_TypeFilter( MESHorSUBMESH );
  if ( !aMeshOrSubMeshFilter.IsNull() )
    aListOfFilters.Append( aMeshOrSubMeshFilter );
  Handle(SMESH_TypeFilter) aSmeshGroupFilter = new SMESH_TypeFilter( GROUP );
  if ( !aSmeshGroupFilter.IsNull() )
    aListOfFilters.Append( aSmeshGroupFilter );
  
  myMeshOrSubMeshOrGroupFilter = new SMESH_LogicalFilter( aListOfFilters, SMESH_LogicalFilter::LO_OR );
  
  Init();
  /* signals and slots connections */
  connect( buttonOk, SIGNAL( clicked() ),     this, SLOT( ClickOnOk() ) );
  connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( ClickOnCancel() ) ) ;
  connect( buttonApply, SIGNAL( clicked() ), this, SLOT(ClickOnApply() ) );
  connect( GroupConstructors, SIGNAL(clicked(int) ), SLOT( ConstructorsClicked(int) ) );
  
  connect( SelectElementsButton, SIGNAL (clicked() ),   this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectPointButton, SIGNAL (clicked() ),   this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( SelectVectorButton, SIGNAL (clicked() ),   this, SLOT( SetEditCurrentArgument() ) ) ;

  connect( SpinBox_DX, SIGNAL( valueChanged( double )), this, SLOT(onVectorChanged() ) );
  connect( SpinBox_DY, SIGNAL( valueChanged( double )), this, SLOT(onVectorChanged() ) );
  connect( SpinBox_DZ, SIGNAL( valueChanged( double )), this, SLOT(onVectorChanged() ) );

  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  /* to close dialog if study change */
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;
  connect( LineEditElements, SIGNAL( textChanged( const QString& )),
           SLOT( onTextChange( const QString& )));
  connect( CheckBoxMesh, SIGNAL( toggled( bool )),
           SLOT( onSelectMesh( bool )));
  
  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; /* displays Dialog */

  ConstructorsClicked(0);
}

//=================================================================================
// function : ~SMESHGUI_RevolutionDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_RevolutionDlg::~SMESHGUI_RevolutionDlg()
{
  // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::Init( bool ResetControls )
{
  myBusy = false;
  
  myEditCurrentArgument = 0;
  LineEditElements->clear();
  myElementsId = "";
  myNbOkElements = 0 ;
  
  myActor     = 0;
  myMesh = SMESH::SMESH_Mesh::_nil();
  
  if( ResetControls )
    {
      SpinBox_X->SetValue(0.0);
      SpinBox_Y->SetValue(0.0);
      SpinBox_Z->SetValue(0.0);
      SpinBox_DX->SetValue(0.0);
      SpinBox_DY->SetValue(0.0);
      SpinBox_DZ->SetValue(0.0);
      
      SpinBox_Angle->SetValue(45);
      SpinBox_NbSteps->setValue(1);
      SpinBox_Tolerance->SetValue(1e-05);
      
      CheckBoxMesh->setChecked(false);
      onSelectMesh( false );
    }
}


//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_RevolutionDlg::ConstructorsClicked(int constructorId)
{
  disconnect(mySelection, 0, this, 0);
  
  Selection_Mode aSelMode = ActorSelection;

  switch(constructorId)
    { 
    case 0 :
      {
	GroupArguments->setTitle( tr( "REVOLUTION_1D" ) );
	aSelMode = EdgeSelection;
	break;
      }
    case 1 :
      { 
	GroupArguments->setTitle( tr( "REVOLUTION_2D" ) );
	aSelMode = FaceSelection;
	break;
      }
    }
  
  if (myEditCurrentArgument != (QWidget*)LineEditElements)
    SMESH::SetPointRepresentation(false);
    
  if (!CheckBoxMesh->isChecked())
    QAD_Application::getDesktop()->SetSelectionMode( aSelMode, true );
  
  myEditCurrentArgument = (QWidget*)LineEditElements;
  LineEditElements->setFocus();
  
  if (CheckBoxMesh->isChecked())
    onSelectMesh(true);
  
  connect(mySelection, SIGNAL(currentSelectionChanged()), this, SLOT(SelectionIntoArgument()));
}


//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::ClickOnApply()
{
  if (mySMESHGUI->ActiveStudyLocked())
    return;
  
  if ( myNbOkElements  && IsAxisOk())
    {
      QStringList aListElementsId = QStringList::split( " ", myElementsId, false);
      
      SMESH::long_array_var anElementsId = new SMESH::long_array;
      
      anElementsId->length( aListElementsId.count() );
      for ( int i = 0; i < aListElementsId.count(); i++ )
	anElementsId[i] = aListElementsId[i].toInt();
      
      SMESH::AxisStruct anAxis;
      
      anAxis.x =  SpinBox_X->GetValue();
      anAxis.y =  SpinBox_Y->GetValue();
      anAxis.z =  SpinBox_Z->GetValue();;
      anAxis.vx = SpinBox_DX->GetValue();
      anAxis.vy = SpinBox_DY->GetValue();
      anAxis.vz = SpinBox_DZ->GetValue();

      double anAngle = (SpinBox_Angle->GetValue())*PI/180; 
      long aNbSteps = (long)SpinBox_NbSteps->value();
      double aTolerance = SpinBox_Tolerance->GetValue();

      try
	{
	  SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
	  QApplication::setOverrideCursor(Qt::waitCursor);
	  aMeshEditor->RotationSweep(anElementsId.inout(), anAxis, anAngle, aNbSteps, aTolerance);
	  QApplication::restoreOverrideCursor();
	}
      catch( ... )
	{
	}
      
      mySelection->ClearIObjects();
      SMESH::UpdateView();
      Init( false);
      ConstructorsClicked( GetConstructorId() );
    }
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::ClickOnOk()
{
  ClickOnApply() ;
  ClickOnCancel() ;
}

	
//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::ClickOnCancel()
{
  mySelection->ClearFilters();
  mySelection->ClearIObjects();
  SMESH::SetPointRepresentation(false);
  QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
  disconnect( mySelection, 0, this, 0 );
  mySMESHGUI->ResetState() ;
  reject() ;
}

//=======================================================================
//function : onTextChange
//purpose  : 
//=======================================================================

void SMESHGUI_RevolutionDlg::onTextChange(const QString& theNewText)
{
  QLineEdit* send = (QLineEdit*)sender();

  if ( myBusy ) return;
  myBusy = true;
  
  if (send == LineEditElements)
    myNbOkElements = 0;
  
  buttonOk->setEnabled( false );
  buttonApply->setEnabled( false );
  
  // hilight entered elements
  SMDS_Mesh* aMesh = 0;
  if ( myActor )
    aMesh = myActor->GetObject()->GetMesh();
  
  if ( aMesh ) {
    mySelection->ClearIObjects();
    mySelection->AddIObject( myActor->getIO() );
    
    QStringList aListId = QStringList::split( " ", theNewText, false);
    
    if (send == LineEditElements) {
      for ( int i = 0; i < aListId.count(); i++ ) {
	const SMDS_MeshElement * e = aMesh->FindElement( aListId[ i ].toInt() );
	if ( e ) {
	  if ( !mySelection->IsIndexSelected( myActor->getIO(), e->GetID() ))
	    mySelection->AddOrRemoveIndex (myActor->getIO(), e->GetID(), true);
	  myNbOkElements++;
	}
      }
      myElementsId = theNewText;
    }
  }
  
  if ( myNbOkElements &&  IsAxisOk() ) {
    buttonOk->setEnabled( true );
    buttonApply->setEnabled( true );
  }
  
  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_RevolutionDlg::SelectionIntoArgument()
{
  if ( myBusy ) return;
  
  // clear
  myActor = 0;
  QString aString = "";

  myBusy = true;
  if (myEditCurrentArgument == (QWidget*)LineEditElements)
    {
      LineEditElements->setText( aString );
      myNbOkElements = 0;
      buttonOk->setEnabled( false );
      buttonApply->setEnabled( false );
    }
  myBusy = false;
  
  if ( !GroupButtons->isEnabled() ) // inactive
    return;
  
  // get selected mesh
  int nbSel = mySelection->IObjectCount();
  if(nbSel != 1)
    return;
  
  Handle(SALOME_InteractiveObject) IO = mySelection->firstIObject();
  myMesh = SMESH::GetMeshByIO(IO);
  if(myMesh->_is_nil())
    return;
  myActor = SMESH::FindActorByObject(myMesh);
  if (!myActor)
    return;
  
  int aNbUnits = 0;

  if (myEditCurrentArgument == (QWidget*)LineEditElements)
    {
      myElementsId = "";
      
      if (CheckBoxMesh->isChecked())
	{
	  SMESH::GetNameOfSelectedIObjects(mySelection, aString);
	
	  int aConstructorId = GetConstructorId();

	  if(!SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO)->_is_nil()) //MESH
	    {
	      // get IDs from mesh
	      SMDS_Mesh* aSMDSMesh = myActor->GetObject()->GetMesh();
	      if (!aSMDSMesh)
		return;
	      
	      if (aConstructorId == 0)
		{
		  SMDS_EdgeIteratorPtr anIter = aSMDSMesh->edgesIterator();
		  
		  while ( anIter->more() )
		    {
		      const SMDS_MeshEdge * edge = anIter->next();
		      if ( edge ) {
			myElementsId += QString(" %1").arg(edge->GetID());
			aNbUnits++;
		      }
		    }
		}
	      else if (aConstructorId == 1)
		{
		  SMDS_FaceIteratorPtr anIter = aSMDSMesh->facesIterator();		  
		  while ( anIter->more() )
		    {
		      const SMDS_MeshFace * face = anIter->next();
		      if ( face ) {
			myElementsId += QString(" %1").arg(face->GetID());
			aNbUnits++;
		      }
		    }
		}
	    }
	  else if (!SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO)->_is_nil()) //SUBMESH
	    {
	      // get submesh
	      SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO) ;
	      
	      // get IDs from submesh
	      SMESH::long_array_var anElementsIds = new SMESH::long_array;
	      if (aConstructorId == 0)
		anElementsIds = aSubMesh->GetElementsByType(SMESH::EDGE);
	      else if (aConstructorId == 1)
		anElementsIds = aSubMesh->GetElementsByType(SMESH::FACE);
	      
	      for ( int i = 0; i < anElementsIds->length(); i++ )
		myElementsId += QString(" %1").arg(anElementsIds[i]);
	      aNbUnits = anElementsIds->length();
	    }
	  else // GROUP
	    {
	      // get smesh group
	      SMESH::SMESH_GroupBase_var aGroup =
                SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IO);
	      if (aGroup->_is_nil())
		return;
	      
	      if ( (aConstructorId == 0 && aGroup->GetType()!= SMESH::EDGE) ||
		   (aConstructorId == 1 && aGroup->GetType()!= SMESH::FACE) )
		return;
	      
	      // get IDs from smesh group
	      SMESH::long_array_var anElementsIds = new SMESH::long_array;
	      anElementsIds = aGroup->GetListOfID();
	      for ( int i = 0; i < anElementsIds->length(); i++ )
		myElementsId += QString(" %1").arg(anElementsIds[i]);
	      aNbUnits = anElementsIds->length();
	    }
	}
      else
	{
	  aNbUnits = SMESH::GetNameOfSelectedElements(mySelection, aString) ;
	  myElementsId = aString;
	}
      
      if(aNbUnits < 1)
	return ;
      
      myNbOkElements = true;
    }
  else 
    {
      aNbUnits = SMESH::GetNameOfSelectedNodes(mySelection, aString);
      if(aNbUnits != 1)
	return;
      
      SMDS_Mesh* aMesh =  myActor->GetObject()->GetMesh();
      if (!aMesh)
	return;

      const SMDS_MeshNode * n = aMesh->FindNode( aString.toInt() );
      if ( !n )
	return;
      
      double x = n->X();
      double y = n->Y();
      double z = n->Z();

      if (myEditCurrentArgument == (QWidget*)SpinBox_X)
	{
	  SpinBox_X->SetValue(x);
	  SpinBox_Y->SetValue(y);
	  SpinBox_Z->SetValue(z);
	}
      else if (myEditCurrentArgument == (QWidget*)SpinBox_DX)
	{
	  SpinBox_DX->SetValue(x - SpinBox_X->GetValue());
	  SpinBox_DY->SetValue(y - SpinBox_Y->GetValue());
	  SpinBox_DZ->SetValue(z - SpinBox_Z->GetValue());
	}
    }
  
  myBusy = true;
  if (myEditCurrentArgument == (QWidget*)LineEditElements)
    LineEditElements->setText( aString );
  myBusy = false;
  
  // OK
  if(myNbOkElements && IsAxisOk())
    {
      buttonOk->setEnabled( true );
      buttonApply->setEnabled( true );
    }
}


//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  
  disconnect( mySelection, 0, this, 0 );
  mySelection->ClearIObjects();
  mySelection->ClearFilters();
  
  if(send == SelectElementsButton) {
    myEditCurrentArgument = (QWidget*)LineEditElements;
    SMESH::SetPointRepresentation(false);
    if (CheckBoxMesh->isChecked())
      {
	QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
	mySelection->AddFilter(myMeshOrSubMeshOrGroupFilter);
      }
    else
      {
	int aConstructorId = GetConstructorId();
	if (aConstructorId == 0)
	  QAD_Application::getDesktop()->SetSelectionMode( EdgeSelection, true );
	else if (aConstructorId == 1)
	  QAD_Application::getDesktop()->SetSelectionMode( FaceSelection, true );
      }
  }
  else if (send == SelectPointButton)
    {
      myEditCurrentArgument = (QWidget*)SpinBox_X;
      SMESH::SetPointRepresentation(true);
      QAD_Application::getDesktop()->SetSelectionMode( NodeSelection, true );
    }
  else if (send == SelectVectorButton)
    {
      myEditCurrentArgument = (QWidget*)SpinBox_DX;
      SMESH::SetPointRepresentation(true);
      QAD_Application::getDesktop()->SetSelectionMode( NodeSelection, true );
    }
  
  myEditCurrentArgument->setFocus();
  connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  SelectionIntoArgument() ;
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::DeactivateActiveDialog()
{
  if ( GroupConstructors->isEnabled() ) {
    GroupConstructors->setEnabled(false) ;
    GroupArguments->setEnabled(false) ;
    GroupButtons->setEnabled(false) ;
    mySMESHGUI->ResetState() ;    
    mySMESHGUI->SetActiveDialogBox(0) ;
  }
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog() ;   
  GroupConstructors->setEnabled(true) ;
  GroupArguments->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
  
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  ConstructorsClicked(GetConstructorId());
  SelectionIntoArgument();
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::enterEvent(QEvent* e)
{
  if ( GroupConstructors->isEnabled() )
    return ;  
  ActivateThisDialog() ;
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_RevolutionDlg::closeEvent( QCloseEvent* e )
{
  /* same than click on cancel button */
  this->ClickOnCancel() ;
  return ;
}


//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================

void SMESHGUI_RevolutionDlg::hideEvent ( QHideEvent * e )
{
  if ( !isMinimized() )
    ClickOnCancel();
}


//=================================================================================
// function : GetConstructorId()
// purpose  : 
//=================================================================================
int SMESHGUI_RevolutionDlg::GetConstructorId()
{ 
  if ( GroupConstructors != NULL && GroupConstructors->selected() != NULL )
    return GroupConstructors->id( GroupConstructors->selected() );
  return -1;
}


//=================================================================================
// function : IsAxisOk()
// purpose  : 
//=================================================================================
bool SMESHGUI_RevolutionDlg::IsAxisOk()
{
  return (SpinBox_DX->GetValue() != 0 || 
	  SpinBox_DY->GetValue() != 0 ||
	  SpinBox_DZ->GetValue() != 0); 
}


//=================================================================================
// function : onVectorChanged()
// purpose  : 
//=================================================================================
void SMESHGUI_RevolutionDlg::onVectorChanged()
{
  if (IsAxisOk())
    {
      buttonOk->setEnabled( true );
      buttonApply->setEnabled( true );
    }
  else
    {
      buttonOk->setEnabled( false );
      buttonApply->setEnabled( false );
    }
}


//=======================================================================
//function : onSelectMesh
//purpose  :
//=======================================================================
void SMESHGUI_RevolutionDlg::onSelectMesh ( bool toSelectMesh )
{
  if (toSelectMesh)
    TextLabelElements->setText( tr( "SMESH_NAME" ) );
  else
    TextLabelElements->setText( tr( "SMESH_ID_ELEMENTS" ) );
  
  if (myEditCurrentArgument != LineEditElements)
    {
      LineEditElements->clear();
      return;
    }
  
  mySelection->ClearFilters() ;
  SMESH::SetPointRepresentation(false);
  
  if (toSelectMesh)
    {
      QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
      mySelection->AddFilter(myMeshOrSubMeshOrGroupFilter);
      LineEditElements->setReadOnly(true);
    }
  else
    {
      int aConstructorId = GetConstructorId();
      if (aConstructorId == 0)
	QAD_Application::getDesktop()->SetSelectionMode( EdgeSelection, true );
      else if (aConstructorId == 0)
	QAD_Application::getDesktop()->SetSelectionMode( FaceSelection, true );
      
      LineEditElements->setReadOnly(false);
      onTextChange(LineEditElements->text());
    }
  
  SelectionIntoArgument();
}
