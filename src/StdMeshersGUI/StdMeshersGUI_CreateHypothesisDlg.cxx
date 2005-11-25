//  SMESH StdMeshersGUI : GUI for StdMeshers plugin
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
//  File   : StdMeshersGUI_CreateHypothesisDlg.cxx
//           Moved here from SMESHGUI_CreateHypothesisDlg.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "StdMeshersGUI_CreateHypothesisDlg.h"
#include "StdMeshersGUI_Parameters.h"
#include "SMESHGUI.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_FunctionPreview.h"

#include "SUIT_Application.h"
#include "SUIT_Desktop.h"
#include "SUIT_MessageBox.h"
#include "SUIT_OverrideCursor.h"
#include "utilities.h"

#include "SalomeApp_Tools.h"
#include "SalomeApp_Application.h"

#include "OB_Browser.h"

// QT Includes
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qspinbox.h>
#include <qtextedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>

using namespace std;

//=================================================================================
// class    : StdMeshersGUI_CreateHypothesisDlg()
// purpose  : Constructs a StdMeshersGUI_CreateHypothesisDlg which is a child of 'parent', with the 
//            name 'name' and widget flags set to 'f'.
//            The dialog will by default be modeless, unless you set 'modal' to
//            TRUE to construct a modal dialog.
//=================================================================================
StdMeshersGUI_CreateHypothesisDlg::StdMeshersGUI_CreateHypothesisDlg (const QString& hypType,
                                                                      QWidget* parent,
                                                                      const char* name,
                                                                      bool modal,
                                                                      WFlags /*fl*/)
     : QDialog (parent, name, modal, WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WDestructiveClose), myHypType( hypType )
{
}

//=======================================================================
//function : CreateDlgLayout
//purpose  : 
//=======================================================================

void StdMeshersGUI_CreateHypothesisDlg::CreateDlgLayout(const QString & theCaption,
                                                        const QPixmap & theHypIcon,
                                                        const QString & theHypTypeName)
{
  setCaption( theCaption );

  setSizeGripEnabled( TRUE );
  QVBoxLayout* StdMeshersGUI_CreateHypothesisDlgLayout = new QVBoxLayout( this ); 
  StdMeshersGUI_CreateHypothesisDlgLayout->setSpacing( 6 );
  StdMeshersGUI_CreateHypothesisDlgLayout->setMargin( 11 );

  /***************************************************************/
  QFrame* titFrame = new QFrame( this );
  QHBoxLayout* titLay = new QHBoxLayout( titFrame, 0, 0 );
  
  iconLabel = new QLabel( titFrame );
  iconLabel->setPixmap( theHypIcon );
  iconLabel->setScaledContents( false );
  iconLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  typeLabel = new QLabel( titFrame );
  typeLabel->setText( theHypTypeName );
  titLay->addWidget( iconLabel, 0 );
  titLay->addWidget( typeLabel, 0 );
  titLay->addStretch( 1 );
  
  StdMeshersGUI_CreateHypothesisDlgLayout->addWidget( titFrame, 0);
    
  /***************************************************************/
  GroupC1 = new QGroupBox( this, "GroupC1" );
  GroupC1->setTitle( tr( "SMESH_ARGUMENTS"  ) );
  GroupC1->setColumnLayout(0, Qt::Vertical );
  GroupC1->layout()->setSpacing( 0 );
  GroupC1->layout()->setMargin( 0 );
  QGridLayout* GroupC1Layout = new QGridLayout( GroupC1->layout() );
  GroupC1Layout->setAlignment( Qt::AlignTop );
  GroupC1Layout->setSpacing( 6 );
  GroupC1Layout->setMargin( 11 );

  TextLabel_NameHypothesis = new QLabel( GroupC1, "TextLabel_NameHypothesis" );
  TextLabel_NameHypothesis->setText( tr( "SMESH_NAME"  ) );
  GroupC1Layout->addWidget( TextLabel_NameHypothesis, 0, 0 );

  LineEdit_NameHypothesis = new QLineEdit( GroupC1, "LineEdit_NameHypothesis" );
  GroupC1Layout->addWidget( LineEdit_NameHypothesis, 0, 1 );

  myParamMap.clear();
  std::list<SMESHGUI_aParameterPtr> aParamList;
  GetParameters( myHypType, aParamList );
  ASSERT( !aParamList.empty() );

  /* Spin boxes with labels */
  list<SMESHGUI_aParameterPtr>::iterator paramIt = aParamList.begin();
  int row;
  for ( row = 1; paramIt != aParamList.end(); paramIt++ , row++ )
  {
    SMESHGUI_aParameterPtr param = (*paramIt);
    QLabel * label = new QLabel( GroupC1, "TextLabel" );
    GroupC1Layout->addWidget( label, row, 0 );
    label->setText( param->Label() );
    QWidget* aWidget = param->CreateWidget( GroupC1 );

    if ( aWidget ) {
      GroupC1Layout->addWidget( aWidget, row, 1 );
      aWidget->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) );
      aWidget->setMinimumSize( 150, 0 );

      QString sig = param->sigValueChanged();
      if( !sig.isEmpty() /* && param->GetType()!=SMESHGUI_aParameter::TABLE*/ )
        connect( aWidget, sig.latin1(), this, SLOT( onValueChanged() ) );
         
      param->InitializeWidget( aWidget );

      ParamInfo info;
      info.editor = aWidget;
      info.label = label;
      info.order = row-1;
      
      myParamMap.insert( param, info );
    }
  }
  myPreview = new SMESHGUI_FunctionPreview( GroupC1 );
  myPreview->hide();
  GroupC1Layout->addWidget( myPreview, row, 1 );
  
  StdMeshersGUI_CreateHypothesisDlgLayout->addWidget( GroupC1, 1 );

  /***************************************************************/
  GroupButtons = new QGroupBox( this, "GroupButtons" );
  GroupButtons->setColumnLayout(0, Qt::Vertical );
  GroupButtons->layout()->setSpacing( 0 );
  GroupButtons->layout()->setMargin( 0 );
  QGridLayout* GroupButtonsLayout = new QGridLayout( GroupButtons->layout() );
  GroupButtonsLayout->setAlignment( Qt::AlignTop );
  GroupButtonsLayout->setSpacing( 6 );
  GroupButtonsLayout->setMargin( 11 );
  buttonOk = new QPushButton( GroupButtons, "buttonOk" );
  buttonOk->setText( tr( "SMESH_BUT_OK"  ) );
  buttonOk->setAutoDefault( TRUE );
  buttonOk->setDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonOk, 0, 0 );
  buttonApply = new QPushButton( GroupButtons, "buttonApply" );
  buttonApply->setText( tr( "SMESH_BUT_APPLY"  ) );
  buttonApply->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonApply, 0, 1 );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  GroupButtonsLayout->addItem( spacer, 0, 2 );
  buttonCancel = new QPushButton( GroupButtons, "buttonCancel" );
  buttonCancel->setText( tr( "SMESH_BUT_CLOSE"  ) );
  buttonCancel->setAutoDefault( TRUE );
  GroupButtonsLayout->addWidget( buttonCancel, 0, 3 );
  StdMeshersGUI_CreateHypothesisDlgLayout->addWidget( GroupButtons, 0 );

  /***************************************************************/
  Init() ;
}


//=================================================================================
// function : ~StdMeshersGUI_CreateHypothesisDlg()
// purpose  : Destroys the object and frees any allocated resources
//=================================================================================
StdMeshersGUI_CreateHypothesisDlg::~StdMeshersGUI_CreateHypothesisDlg()
{
    // no need to delete child widgets, Qt does it all for us
}


//=================================================================================
// function : Init()
// purpose  :
//=================================================================================
void StdMeshersGUI_CreateHypothesisDlg::Init()
{
  ParameterMap::const_iterator anIt = myParamMap.begin(),
                               aLast = myParamMap.end();
  for( ; anIt!=aLast; anIt++ )
    UpdateShown( anIt.key() );
   
  mySMESHGUI = SMESHGUI::GetSMESHGUI() ;

  char* sHypType = const_cast<char*>(myHypType.latin1());
  HypothesisData* aHypData = SMESH::GetHypothesisData(sHypType);
  LineEdit_NameHypothesis->setText( aHypData ? aHypData->Label : QString("") );

  mySMESHGUI->SetActiveDialogBox( (QDialog*)this );

  /* signals and slots connections */
  connect( buttonOk, SIGNAL(clicked()),     this, SLOT(ClickOnOk()) );
  connect( buttonCancel, SIGNAL(clicked()), this, SLOT(ClickOnCancel()) ) ;
  connect( buttonApply, SIGNAL(clicked()), this, SLOT(ClickOnApply()) );

  connect( mySMESHGUI, SIGNAL ( SignalDeactivateActiveDialog() ), this, SLOT( DeactivateActiveDialog() ) ) ;  
  connect( mySMESHGUI, SIGNAL ( SignalCloseAllDialogs() ), this, SLOT( ClickOnCancel() ) ) ;

  /* Move widget on the botton right corner of main widget */
  int x, y ;
  mySMESHGUI->DefineDlgPosition( this, x, y ) ;
  this->move( x, y ) ;
  this->show() ; /* displays Dialog */
}


//=================================================================================
// function : ClickOnOk()
// purpose  :
//=================================================================================
void StdMeshersGUI_CreateHypothesisDlg::ClickOnOk()
{
  if ( ClickOnApply() )
    ClickOnCancel() ;
}

//=================================================================================
// function : ClickOnApply()
// purpose  :
//=================================================================================
bool StdMeshersGUI_CreateHypothesisDlg::ClickOnApply()
{
  if ( !mySMESHGUI || mySMESHGUI->isActiveStudyLocked() )
    return false;

  QString myHypName = LineEdit_NameHypothesis->text().stripWhiteSpace();
  if ( myHypName.isEmpty() ) {
    SUIT_MessageBox::warn1 (this, tr( "SMESH_WRN_WARNING" ),
                           tr( "SMESH_WRN_EMPTY_NAME" ), tr( "SMESH_BUT_OK" ) );
    return false;
  }

  SUIT_OverrideCursor wc;

  SMESH::SMESH_Hypothesis_var Hyp = SMESH::SMESH_Hypothesis::_narrow
      ( SMESH::CreateHypothesis( myHypType, myHypName, false ) ); // isAlgorithm
  
  try {

    list<SMESHGUI_aParameterPtr> aParamList;
    ParameterMap::const_iterator anIt = myParamMap.begin(),
                                 aLast = myParamMap.end();
    for( int i=0; i<myParamMap.count(); i++ )
      for( anIt=myParamMap.begin(); anIt!=aLast; anIt++ )
        if( (*anIt).order==i )
        {
          anIt.key()->TakeValue( anIt.data().editor );
          aParamList.push_back( anIt.key() );
	  break;
        }

    if( !SetParameters( Hyp, aParamList ) )
      return false;

    //set new Attribute Comment for hypothesis which parameters were set
    QString aParams = "";
    StdMeshersGUI_Parameters::GetParameters( Hyp.in(), aParamList, aParams );
    _PTR(SObject) SHyp = SMESH::FindSObject(Hyp.in());
    if (SHyp)
      if (!aParams.isEmpty())
      {
        SMESH::SetValue(SHyp, aParams);
        mySMESHGUI->getApp()->objectBrowser()->updateTree();
      }
  }
  catch (const SALOME::SALOME_Exception& S_ex)
  {
    wc.suspend();

    _PTR(SObject) SHyp = SMESH::FindSObject(Hyp.in());
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    if( aStudy && !aStudy->GetProperties()->IsLocked() )
    {
      _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
      aBuilder->RemoveObjectWithChildren( SHyp );
      mySMESHGUI->updateObjBrowser( true, 0 );
    }
    
    SalomeApp_Tools::QtCatchCorbaException(S_ex);
    return false;
  }
  return true;
}

//=================================================================================
// function : ClickOnCancel()
// purpose  :
//=================================================================================
void StdMeshersGUI_CreateHypothesisDlg::ClickOnCancel()
{
  close();
}

//=================================================================================
// function : DeactivateActiveDialog()
// purpose  :
//=================================================================================
void StdMeshersGUI_CreateHypothesisDlg::DeactivateActiveDialog()
{
//   iconLabel->setEnabled(false) ;
//   typeLabel->setEnabled(false) ;
//   GroupC1->setEnabled(false) ;
//   GroupButtons->setEnabled(false) ;
  setEnabled(false);
}


//=================================================================================
// function : ActivateThisDialog()
// purpose  :
//=================================================================================
void StdMeshersGUI_CreateHypothesisDlg::ActivateThisDialog()
{
  if (! isEnabled() ) {
    mySMESHGUI->EmitSignalDeactivateDialog() ;   
//     iconLabel->setEnabled(true) ;
//     typeLabel->setEnabled(true) ;
//     GroupC1->setEnabled(true) ;
//     GroupButtons->setEnabled(true) ;
    setEnabled(true) ;
  }
}


//=================================================================================
// function : enterEvent()
// purpose  :
//=================================================================================
void StdMeshersGUI_CreateHypothesisDlg::enterEvent(QEvent* e)
{
  ActivateThisDialog() ;
}


//=================================================================================
// function : closeEvent()
// purpose  :
//=================================================================================
void StdMeshersGUI_CreateHypothesisDlg::closeEvent( QCloseEvent* e )
{
  mySMESHGUI->ResetState();
  QDialog::closeEvent( e );
}

//=================================================================================
// function : onValueChanged()
// purpose  :
//=================================================================================
void StdMeshersGUI_CreateHypothesisDlg::onValueChanged()
{
  if( sender()->inherits( "QWidget" ) )
  {
    QWidget* w = ( QWidget* )sender();

    SMESHGUI_aParameterPtr param;

    ParameterMap::const_iterator anIt = myParamMap.begin(),
                                 aLast = myParamMap.end();
    for( ; anIt!=aLast; anIt++ )
      if( anIt.data().editor == w )
      {
        param = anIt.key();
	param->TakeValue( w );

	SMESHGUI_strParameter* str_param = dynamic_cast<SMESHGUI_strParameter*>( param.operator->() );
	SMESHGUI_tableParameter* tab_param = dynamic_cast<SMESHGUI_tableParameter*>( param.operator->() );
	SMESHGUI_boolParameter* bool_param = dynamic_cast<SMESHGUI_boolParameter*>( param.operator->() );

	if( str_param && str_param->needPreview() )
	{
	  QString val; str_param->GetNewText( val );
	  myPreview->setParams( val );
	}
	else if( tab_param && tab_param->needPreview() )
	{
	  SMESH::double_array d;
	  tab_param->data( d );
	  myPreview->setParams( d );
	}
	else if( bool_param && bool_param->needPreview() )
	{
	  int exp=0;
	  bool_param->GetNewInt( exp );
	  myPreview->setIsExp( exp );
	}
        UpdateShown( param );
        break;
      }
  }
}

//=================================================================================
// function : UpdateShown()
// purpose  :
//=================================================================================
void StdMeshersGUI_CreateHypothesisDlg::UpdateShown( const SMESHGUI_aParameterPtr param )
{
  SMESHGUI_dependParameter* depPar = dynamic_cast<SMESHGUI_enumParameter*>( param.get() );
  if( !depPar )
    depPar = dynamic_cast<SMESHGUI_boolParameter*>( param.get() );

  if( !depPar )
    return;

  SMESHGUI_dependParameter::ShownMap& map = depPar->shownMap();
  if( map.isEmpty() )
    return;

  int val;
  depPar->TakeValue( myParamMap[ param ].editor );
  depPar->GetNewInt( val );
  
  bool hasValue = map.contains( val );

  ParameterMap::const_iterator anIt = myParamMap.begin(),
                                aLast = myParamMap.end();
  bool preview = false;
  for( ; anIt!=aLast; anIt++ )
  {
    bool shown = hasValue && map[ val ].contains( (*anIt).order );
    (*anIt).editor->setShown( shown );
    (*anIt).label->setShown( shown );
    if( shown )
    {
      SMESHGUI_strParameter* str_param = dynamic_cast<SMESHGUI_strParameter*>( anIt.key().operator->() );
      SMESHGUI_tableParameter* tab_param = dynamic_cast<SMESHGUI_tableParameter*>( anIt.key().operator->() );
      preview = preview || ( str_param && str_param->needPreview() ) || ( tab_param && tab_param->needPreview() );
    }
  }
  myPreview->setShown( preview );
}
