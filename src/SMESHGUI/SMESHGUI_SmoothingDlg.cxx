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
//  File   : SMESHGUI_SmoothingDlg.cxx
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#include "SMESHGUI_SmoothingDlg.h"

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
#include <qcombobox.h>
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
// class    : SMESHGUI_SmoothingDlg()
// purpose  : 
//=================================================================================
SMESHGUI_SmoothingDlg::SMESHGUI_SmoothingDlg( QWidget* parent, const char* name, SALOME_Selection* Sel,
					      bool modal, WFlags fl )
  : QDialog( parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu |
	     Qt::WDestructiveClose)
{
  QPixmap image0(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_DLG_SMOOTHING")));
  QPixmap image1(QAD_Desktop::getResourceManager()->loadPixmap( "SMESH",tr("ICON_SELECT")));

  if ( !name )
    setName( "SMESHGUI_SmoothingDlg" );
  resize( 303, 185 ); 
  setCaption( tr( "SMESH_SMOOTHING" ) );
  setSizeGripEnabled( TRUE );
  SMESHGUI_SmoothingDlgLayout = new QGridLayout( this ); 
  SMESHGUI_SmoothingDlgLayout->setSpacing( 6 );
  SMESHGUI_SmoothingDlgLayout->setMargin( 11 );

  /***************************************************************/
  GroupConstructors = new QButtonGroup( this, "GroupConstructors" );
  GroupConstructors->setTitle( tr( "SMESH_SMOOTHING"  ) );
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
  Constructor1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, Constructor1->sizePolicy().hasHeightForWidth() ) );
  Constructor1->setMinimumSize( QSize( 50, 0 ) );
  GroupConstructorsLayout->addWidget( Constructor1, 0, 0 );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  GroupConstructorsLayout->addItem( spacer, 0, 1 );
  SMESHGUI_SmoothingDlgLayout->addWidget( GroupConstructors, 0, 0 );
  
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
  SMESHGUI_SmoothingDlgLayout->addWidget( GroupButtons, 2, 0 );

  /***************************************************************/
  GroupArguments = new QGroupBox( this, "GroupArguments" );
  GroupArguments->setTitle( tr( "SMESH_ARGUMENTS"  ) );
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
  GroupArgumentsLayout->addWidget( TextLabelElements, 0, 0 );

  SelectElementsButton  = new QPushButton( GroupArguments, "SelectElementsButton" );
  SelectElementsButton->setText( tr( ""  ) );
  SelectElementsButton->setPixmap( image1 );
  SelectElementsButton->setToggleButton( FALSE );
  GroupArgumentsLayout->addWidget( SelectElementsButton, 0, 1 );

  LineEditElements  = new QLineEdit( GroupArguments, "LineEditElements" );
  LineEditElements->setValidator( new SMESHGUI_IdValidator( this, "validator" ));
  GroupArgumentsLayout->addWidget( LineEditElements, 0, 2 );

  // Control for the whole mesh selection
  CheckBoxMesh = new QCheckBox( GroupArguments, "CheckBoxMesh" );
  CheckBoxMesh->setText( tr( "SMESH_SELECT_WHOLE_MESH" ) );
  GroupArgumentsLayout->addMultiCellWidget( CheckBoxMesh, 1, 1, 0, 2 );

  // Controls for nodes selection
  TextLabelNodes = new QLabel( GroupArguments, "TextLabelNodes" );
  TextLabelNodes->setText( tr( "FIXED_NODES_IDS" ) );
  GroupArgumentsLayout->addWidget( TextLabelNodes, 2, 0 );

  SelectNodesButton  = new QPushButton( GroupArguments, "SelectNodesButton" );
  SelectNodesButton->setText( tr( ""  ) );
  SelectNodesButton->setPixmap( image1 );
  SelectNodesButton->setToggleButton( FALSE );
  GroupArgumentsLayout->addWidget( SelectNodesButton, 2, 1 );

  LineEditNodes  = new QLineEdit( GroupArguments, "LineEditNodes" );
  LineEditNodes->setValidator( new SMESHGUI_IdValidator( this, "validator" ));
  GroupArgumentsLayout->addWidget( LineEditNodes, 2, 2 );

  // Controls for method selection
  TextLabelMethod = new QLabel( GroupArguments, "TextLabelMethod" );
  TextLabelMethod->setText( tr( "METHOD"  ) );
  GroupArgumentsLayout->addMultiCellWidget( TextLabelMethod, 3, 3, 0, 1 );

  ComboBoxMethod = new QComboBox( GroupArguments, "ComboBoxMethod" );
  GroupArgumentsLayout->addWidget( ComboBoxMethod, 3, 2 );

  // Controls for iteration limit defining
  TextLabelLimit = new QLabel( GroupArguments, "TextLabelLimit" );
  TextLabelLimit->setText( tr( "ITERATION_LIMIT"  ) );
  GroupArgumentsLayout->addMultiCellWidget( TextLabelLimit, 4, 4, 0, 1 );

  SpinBox_IterationLimit = new QSpinBox(GroupArguments, "SpinBox_IterationLimit");
  GroupArgumentsLayout->addWidget( SpinBox_IterationLimit, 4, 2 );

  // Controls for max. aspect ratio defining
  TextLabelAspectRatio = new QLabel( GroupArguments, "TextLabelAspectRatio" );
  TextLabelAspectRatio->setText( tr( "MAX_ASPECT_RATIO" ) );
  GroupArgumentsLayout->addMultiCellWidget( TextLabelAspectRatio, 5, 5, 0, 1 );

  SpinBox_AspectRatio = new SMESHGUI_SpinBox(GroupArguments, "SpinBox_AspectRatio");
  GroupArgumentsLayout->addWidget( SpinBox_AspectRatio, 5, 2 );


  SMESHGUI_SmoothingDlgLayout->addWidget( GroupArguments, 1, 0 );
  
  /* Initialisations */
  ComboBoxMethod->insertItem(tr("LAPLACIAN"));
  ComboBoxMethod->insertItem(tr("CENTROIDAL"));

  QIntValidator* anIntValidator = new QIntValidator(SpinBox_IterationLimit);
  SpinBox_IterationLimit->setValidator(anIntValidator);
  SpinBox_IterationLimit->setRange( 1, 999999 );
  SpinBox_AspectRatio->RangeStepAndValidator( 0.0, +999999.999, 0.1, 3 );

  GroupArguments->show();
  myConstructorId = 0 ;
  Constructor1->setChecked( TRUE );
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
  connect( SelectNodesButton, SIGNAL (clicked() ),   this, SLOT( SetEditCurrentArgument() ) ) ;
  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;
  connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
  /* to close dialog if study change */
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;
  connect( LineEditElements, SIGNAL( textChanged( const QString& )),
           SLOT( onTextChange( const QString& )));
  connect( LineEditNodes, SIGNAL( textChanged( const QString& )),
           SLOT( onTextChange( const QString& )));
  connect( CheckBoxMesh, SIGNAL( toggled( bool )),
           SLOT( onSelectMesh( bool )));
  
  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; /* displays Dialog */
}

//=================================================================================
// function : ~SMESHGUI_SmoothingDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
SMESHGUI_SmoothingDlg::~SMESHGUI_SmoothingDlg()
{
  // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::Init()
{
  myBusy = false;  
          
  ComboBoxMethod->setCurrentItem(0);
  
  SpinBox_IterationLimit->setValue(20);
  SpinBox_AspectRatio->SetValue(1.1);
  
  myEditCurrentArgument = LineEditElements;
  LineEditElements->setFocus();
  myElementsId = "";
  myNbOkElements = 0 ;
  myNbOkNodes = 0 ;
  myActor     = 0;
  myMesh = SMESH::SMESH_Mesh::_nil();
  
  CheckBoxMesh->setChecked(false);
  onSelectMesh( false );
}


//=================================================================================
// function : ConstructorsClicked()
// purpose  : Radio button management
//=================================================================================
void SMESHGUI_SmoothingDlg::ConstructorsClicked(int constructorId)
{
  return ;
}


//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::ClickOnApply()
{
  if (mySMESHGUI->ActiveStudyLocked())
    return;
  
  if ( myNbOkElements  && (myNbOkNodes || LineEditNodes->text().stripWhiteSpace().isEmpty()))
    {
      QStringList aListElementsId = QStringList::split( " ", myElementsId, false);
      QStringList aListNodesId = QStringList::split( " ", LineEditNodes->text(), false);

      SMESH::long_array_var anElementsId = new SMESH::long_array;
      SMESH::long_array_var aNodesId = new SMESH::long_array;

      anElementsId->length( aListElementsId.count() );
      for ( int i = 0; i < aListElementsId.count(); i++ )
	anElementsId[i] = aListElementsId[i].toInt();
      
      if (myNbOkNodes)
	{
	  aNodesId->length( aListNodesId.count() );
	  for ( int i = 0; i < aListNodesId.count(); i++ )
	    aNodesId[i] = aListNodesId[i].toInt();
	}
      else
	anElementsId->length(0);
      
      long anIterationLimit = (long)SpinBox_IterationLimit->value();
      double aMaxAspectRatio = SpinBox_AspectRatio->GetValue();

      SMESH::SMESH_MeshEditor::Smooth_Method aMethod = SMESH::SMESH_MeshEditor::LAPLACIAN_SMOOTH;
      if (ComboBoxMethod->currentItem() > 0)
	aMethod =  SMESH::SMESH_MeshEditor::CENTROIDAL_SMOOTH;
      
      bool aResult = false;
      try
	{
	  SMESH::SMESH_MeshEditor_var aMeshEditor = myMesh->GetMeshEditor();
	  QApplication::setOverrideCursor(Qt::waitCursor);
	  aResult = aMeshEditor->Smooth(anElementsId.inout(), aNodesId.inout(), anIterationLimit, aMaxAspectRatio, aMethod);
	  QApplication::restoreOverrideCursor();
	}
      catch( ... )
	{
	}
      
      if ( aResult )
	{
	  Handle(SALOME_InteractiveObject) anIO = myActor->getIO();
	  mySelection->ClearIObjects();
	  SMESH::UpdateView();
	  mySelection->AddIObject( anIO, false );
	  Init();
	}
    }
  
}

//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::ClickOnOk()
{
  ClickOnApply() ;
  ClickOnCancel() ;
}

	
//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::ClickOnCancel()
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

void SMESHGUI_SmoothingDlg::onTextChange(const QString& theNewText)
{
  QLineEdit* send = (QLineEdit*)sender();

  if ( myBusy ) return;
  myBusy = true;
  
  if (send == LineEditElements)
    myNbOkElements = 0;
  else if (send == LineEditNodes)
    myNbOkNodes = 0;
  
  buttonOk->setEnabled( false );
  buttonApply->setEnabled( false );
  
  // hilight entered elements/nodes
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
    else if (send == LineEditNodes) {   
      for ( int i = 0; i < aListId.count(); i++ ) {
	const SMDS_MeshNode * n = aMesh->FindNode( aListId[ i ].toInt() );
	if ( n ) {
	  if ( !mySelection->IsIndexSelected( myActor->getIO(), n->GetID() ))
	    mySelection->AddOrRemoveIndex (myActor->getIO(), n->GetID(), true);
	  myNbOkNodes++;
	}
      }
    }
  }
  
  if ( myNbOkElements && (myNbOkNodes || LineEditNodes->text().stripWhiteSpace().isEmpty()) ) {
    buttonOk->setEnabled( true );
    buttonApply->setEnabled( true );
  }
  
  myBusy = false;
}

//=================================================================================
// function : SelectionIntoArgument()
// purpose  : Called when selection as changed or other case
//=================================================================================
void SMESHGUI_SmoothingDlg::SelectionIntoArgument()
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
 
  if (myEditCurrentArgument == LineEditElements)
    {
      myElementsId = "";
      
      if (CheckBoxMesh->isChecked())
	{
	  SMESH::GetNameOfSelectedIObjects(mySelection, aString);
	  
	  if(!SMESH::IObjectToInterface<SMESH::SMESH_Mesh>(IO)->_is_nil()) //MESH
	    {
	      // get IDs from mesh
	      SMDS_Mesh* aSMDSMesh = myActor->GetObject()->GetMesh();
	      if (!aSMDSMesh)
		return;
	      
	      for (int i = aSMDSMesh->MinElementID(); i <= aSMDSMesh->MaxElementID(); i++  )
		{
		  const SMDS_MeshElement * e = aSMDSMesh->FindElement( i );
		  if ( e ) {
		    myElementsId += QString(" %1").arg(i);
		    aNbUnits++;
		  }
		}
	    }
	  else if (!SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO)->_is_nil()) //SUBMESH
	    {
	      // get submesh
	      SMESH::SMESH_subMesh_var aSubMesh = SMESH::IObjectToInterface<SMESH::SMESH_subMesh>(IO) ;
	      
	      // get IDs from submesh
	      SMESH::long_array_var anElementsIds = new SMESH::long_array;
	      anElementsIds = aSubMesh->GetElementsId();
	      for ( int i = 0; i < anElementsIds->length(); i++ )
		myElementsId += QString(" %1").arg(anElementsIds[i]);
	      aNbUnits = anElementsIds->length();
	    }
	  else // GROUP
	    {
	      // get smesh group
	      SMESH::SMESH_GroupBase_var aGroup
                = SMESH::IObjectToInterface<SMESH::SMESH_GroupBase>(IO);
	      if (aGroup->_is_nil())
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
    }
  else if (myEditCurrentArgument == LineEditNodes && !myMesh->_is_nil() && myActor)
    {
      myNbOkNodes = 0;
      aNbUnits = SMESH::GetNameOfSelectedNodes(mySelection, aString) ;
    }
  
  if(aNbUnits < 1)
    return ;

  myBusy = true;
  myEditCurrentArgument->setText( aString );
  myBusy = false;
  
  // OK
  if (myEditCurrentArgument == LineEditElements)
    myNbOkElements = true;
  else if (myEditCurrentArgument == LineEditNodes)
    myNbOkNodes = true;
  
  if (myNbOkElements)
    {
      buttonOk->setEnabled( true );
      buttonApply->setEnabled( true );
    }
}


//=================================================================================
// function : SetEditCurrentArgument()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::SetEditCurrentArgument()
{
  QPushButton* send = (QPushButton*)sender();
  
  switch (myConstructorId)
    {
    case 0: /* default constructor */
      {	
	disconnect( mySelection, 0, this, 0 );
	mySelection->ClearIObjects();
	mySelection->ClearFilters();
	
	if(send == SelectElementsButton) {
	  myEditCurrentArgument = LineEditElements;
	  SMESH::SetPointRepresentation(false);
	  if (CheckBoxMesh->isChecked())
	    {
	      QAD_Application::getDesktop()->SetSelectionMode( ActorSelection );
	      mySelection->AddFilter(myMeshOrSubMeshOrGroupFilter);
	    }
	  else
	    QAD_Application::getDesktop()->SetSelectionMode( CellSelection, true );
	}
	else if (send == SelectNodesButton)
	  {
	    myEditCurrentArgument = LineEditNodes;
	    SMESH::SetPointRepresentation(true);
	    QAD_Application::getDesktop()->SetSelectionMode( NodeSelection, true );
	  }
	
	myEditCurrentArgument->setFocus();
	connect( mySelection, SIGNAL( currentSelectionChanged() ), this, SLOT( SelectionIntoArgument() ) );
	SelectionIntoArgument() ;
	break;
      }
    }
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::DeactivateActiveDialog()
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
void SMESHGUI_SmoothingDlg::ActivateThisDialog()
{
  /* Emit a signal to deactivate the active dialog */
  mySMESHGUI->EmitSignalDeactivateDialog() ;   
  GroupConstructors->setEnabled(true) ;
  GroupArguments->setEnabled(true) ;
  GroupButtons->setEnabled(true) ;
  
  mySMESHGUI->SetActiveDialogBox( (QDialog*)this ) ;

  QAD_Application::getDesktop()->SetSelectionMode( CellSelection, true );
  SelectionIntoArgument();
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::enterEvent(QEvent* e)
{
  if ( GroupConstructors->isEnabled() )
    return ;  
  ActivateThisDialog() ;
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void SMESHGUI_SmoothingDlg::closeEvent( QCloseEvent* e )
{
  /* same than click on cancel button */
  this->ClickOnCancel() ;
}


//=======================================================================
//function : hideEvent
//purpose  : caused by ESC key
//=======================================================================

void SMESHGUI_SmoothingDlg::hideEvent ( QHideEvent * e )
{
  if ( !isMinimized() )
    ClickOnCancel();
}


//=======================================================================
//function : onSelectMesh
//purpose  :
//=======================================================================
void SMESHGUI_SmoothingDlg::onSelectMesh ( bool toSelectMesh )
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
      QAD_Application::getDesktop()->SetSelectionMode( CellSelection, true );
      LineEditElements->setReadOnly(false);
      onTextChange(LineEditElements->text());
    }
  
  SelectionIntoArgument();
}
