// Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
//
// This library is distributed in the hope that it will be useful
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//

#include "SMESHGUI_Hypotheses.h"
#include "SMESHGUI.h"
#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_SpinBox.h"

#include <SALOMEDSClient_Study.hxx>
#include <utilities.h>

#include <SMESHGUI.h>

#include <QtxIntSpinBox.h>

#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>

#include <LightApp_Application.h>

#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qgroupbox.h>

SMESHGUI_GenericHypothesisCreator::SMESHGUI_GenericHypothesisCreator( const QString& aHypType )
: myHypType( aHypType ),
  myIsCreate( false )
{
}

SMESHGUI_GenericHypothesisCreator::~SMESHGUI_GenericHypothesisCreator()
{
}

void SMESHGUI_GenericHypothesisCreator::create( SMESH::SMESH_Hypothesis_ptr initParamsHyp,
                                                QWidget* parent)
{
  MESSAGE( "Creation of hypothesis with initial params" );

  if ( !CORBA::is_nil( initParamsHyp ) && hypType() == initParamsHyp->GetName() )
    myInitParamsHypo = SMESH::SMESH_Hypothesis::_duplicate( initParamsHyp );
  create( false, parent );
}

void SMESHGUI_GenericHypothesisCreator::create( const bool isAlgo, QWidget* parent )
{
  MESSAGE( "Creation of hypothesis" );

  // Get default name for hypothesis/algorithm creation
  HypothesisData* aHypData = SMESH::GetHypothesisData( hypType().latin1() );
  QString aHypName = aHypData ? aHypData->Label : hypType();

  myIsCreate = true;

  // Create hypothesis/algorithm
  if (isAlgo)
    SMESH::CreateHypothesis( hypType(), aHypName, isAlgo );

  else
  {
    SMESH::SMESH_Hypothesis_var newHypo = SMESH::SMESH_Hypothesis::_narrow
      ( SMESH::CreateHypothesis( hypType(), aHypName, false ) );
  
    if( !editHypothesis( newHypo.in(), parent ) )
    { //remove just created hypothesis
      _PTR(SObject) SHyp = SMESH::FindSObject( newHypo.in() );
      _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
      if( aStudy && !aStudy->GetProperties()->IsLocked() )
      {
	_PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
	aBuilder->RemoveObjectWithChildren( SHyp );
      }
    }
  }
  SMESHGUI::GetSMESHGUI()->updateObjBrowser( true, 0 );
}

void SMESHGUI_GenericHypothesisCreator::edit( SMESH::SMESH_Hypothesis_ptr h, QWidget* parent )
{
  if( CORBA::is_nil( h ) )
    return;

  MESSAGE("Edition of hypothesis");

  myIsCreate = false;

  if( !editHypothesis( h, parent ) )
    return;

  SMESH::SObjectList listSOmesh = SMESH::GetMeshesUsingAlgoOrHypothesis( h );
  if( listSOmesh.size() > 0 )
    for( int i=0; i<listSOmesh.size(); i++ )
    {
      _PTR(SObject) submSO = listSOmesh[i];
      SMESH::SMESH_Mesh_var aMesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( submSO );
      SMESH::SMESH_subMesh_var aSubMesh = SMESH::SObjectToInterface<SMESH::SMESH_subMesh>( submSO );
      if( !aSubMesh->_is_nil() )
	aMesh = aSubMesh->GetFather();
      _PTR(SObject) meshSO = SMESH::FindSObject( aMesh );
      SMESH::ModifiedMesh( meshSO, false);
    }
  SMESHGUI::GetSMESHGUI()->updateObjBrowser( true, 0 );
}

bool SMESHGUI_GenericHypothesisCreator::editHypothesis( SMESH::SMESH_Hypothesis_ptr h, QWidget* parent )
{
  if( CORBA::is_nil( h ) )
    return false;

  bool res = true;
  myHypo = SMESH::SMESH_Hypothesis::_duplicate( h );

  QFrame* fr = buildFrame();
  if( fr )
  {
    SMESHGUI_HypothesisDlg* dlg = 
      new SMESHGUI_HypothesisDlg( const_cast<SMESHGUI_GenericHypothesisCreator*>( this ), parent );
    dlg->setCustomFrame( fr );
    dlg->setCaption( caption() );
    dlg->setHIcon( icon() );
    dlg->setType( type() );
    retrieveParams();
    res = dlg->exec()==QDialog::Accepted;
    if( res ) {
      QString paramValues = storeParams();
      if ( !paramValues.isEmpty() ) {
        if ( _PTR(SObject) SHyp = SMESH::FindSObject( myHypo ))
          SMESH::SetValue( SHyp, paramValues );
      }
    }
    delete dlg;
  }
  changeWidgets().clear();
  myHypo = SMESH::SMESH_Hypothesis::_nil();
  myInitParamsHypo = SMESH::SMESH_Hypothesis::_nil();
  return res;
}

QFrame* SMESHGUI_GenericHypothesisCreator::buildStdFrame()
{
  if( CORBA::is_nil( hypothesis() ) )
    return 0;

  ListOfStdParams params;
  if( !stdParams( params ) || params.isEmpty() )
    return 0;

  QFrame* fr = new QFrame( 0, "myframe" );
  QVBoxLayout* lay = new QVBoxLayout( fr, 5, 0 );

  QGroupBox* GroupC1 = new QGroupBox( fr, "GroupC1" );
  lay->addWidget( GroupC1 );

  GroupC1->setTitle( tr( "SMESH_ARGUMENTS"  ) );
  GroupC1->setColumnLayout(0, Qt::Vertical );
  GroupC1->layout()->setSpacing( 0 );
  GroupC1->layout()->setMargin( 0 );
  QGridLayout* GroupC1Layout = new QGridLayout( GroupC1->layout() );
  GroupC1Layout->setAlignment( Qt::AlignTop );
  GroupC1Layout->setSpacing( 6 );
  GroupC1Layout->setMargin( 11 );

  ListOfStdParams::const_iterator anIt = params.begin(), aLast = params.end();
  for( int i=0; anIt!=aLast; anIt++, i++ )
  {
    QLabel* lab = new QLabel( (*anIt).myName, GroupC1 );
    GroupC1Layout->addWidget( lab, i, 0 );

    QWidget* w = getCustomWidget( *anIt, GroupC1 );
    if ( !w ) 
      switch( (*anIt).myValue.type() )
      {
      case QVariant::Int:
        {
          QtxIntSpinBox* sb = new QtxIntSpinBox( GroupC1, (*anIt).myName.latin1() );
          attuneStdWidget( sb, i );
          sb->setValue( (*anIt).myValue.toInt() );
          connect( sb, SIGNAL( valueChanged( int ) ), this, SLOT( onValueChanged() ) );
          w = sb;
        }
        break;
      case QVariant::Double:
        {
          QtxDblSpinBox* sb = new SMESHGUI_SpinBox( GroupC1, (*anIt).myName.latin1() );
          attuneStdWidget( sb, i );
          sb->setValue( (*anIt).myValue.toDouble() );
          connect( sb, SIGNAL( valueChanged( double ) ), this, SLOT( onValueChanged() ) );
          w = sb;
        }
        break;
      case QVariant::String:
        {
          QLineEdit* le = new QLineEdit( GroupC1, (*anIt).myName.latin1() );
          attuneStdWidget( le, i );
          le->setText( (*anIt).myValue.toString() );
          connect( le, SIGNAL( textChanged( const QString& ) ), this, SLOT( onValueChanged() ) );
          w = le;
        }
        break;
      }

    if( w )
    {
      GroupC1Layout->addWidget( w, i, 1 );
      changeWidgets().append( w );
    }
  }

  return fr;
}

void SMESHGUI_GenericHypothesisCreator::onValueChanged()
{
}

bool SMESHGUI_GenericHypothesisCreator::stdParams( ListOfStdParams& ) const
{
  return false;
}

bool SMESHGUI_GenericHypothesisCreator::getStdParamFromDlg( ListOfStdParams& params ) const
{
  bool res = true;
  StdParam item;
  ListOfWidgets::const_iterator anIt = widgets().begin(), aLast = widgets().end();
  for( ; anIt!=aLast; anIt++ )
  {
    item.myName = (*anIt)->name();
    if( (*anIt)->inherits( "QtxIntSpinBox" ) )
    {
      QtxIntSpinBox* sb = ( QtxIntSpinBox* )( *anIt );
      item.myValue = sb->value();
      params.append( item );
    }
    
    else if( (*anIt)->inherits( "QtxDblSpinBox" ) )
    {
      QtxDblSpinBox* sb = ( QtxDblSpinBox* )( *anIt );
      item.myValue = sb->value();
      params.append( item );
    }

    else if( (*anIt)->inherits( "QLineEdit" ) )
    {
      QLineEdit* line = ( QLineEdit* )( *anIt );
      item.myValue = line->text();
      params.append( item );
    }

    else if ( getParamFromCustomWidget( item, *anIt ))
    {
      params.append( item );
    }

    else
      res = false;
  }
  return res;
}

QString SMESHGUI_GenericHypothesisCreator::stdParamValues( const ListOfStdParams& params)
{
  QString valueStr = "";
  ListOfStdParams::const_iterator param = params.begin(), aLast = params.end();
  for( int i=0; param!=aLast; param++, i++ )
  {
    if ( i > 0 )
      valueStr += "; ";
    switch( (*param).myValue.type() )
    {
    case QVariant::Int:
      valueStr += valueStr.number( (*param).myValue.toInt() );
      break;
    case QVariant::Double:
      valueStr += valueStr.number( (*param).myValue.toDouble() );
      break;
    case QVariant::String:
      valueStr += (*param).myValue.toString();
      break;
    default:
      QVariant valCopy = (*param).myValue;
      valueStr += valCopy.asString();
    }
  }
  return valueStr;
}

SMESH::SMESH_Hypothesis_var SMESHGUI_GenericHypothesisCreator::hypothesis() const
{
  return myHypo;
}

SMESH::SMESH_Hypothesis_var SMESHGUI_GenericHypothesisCreator::initParamsHypothesis() const
{
  if ( CORBA::is_nil( myInitParamsHypo ))
    return myHypo;
  return myInitParamsHypo;
}

QString SMESHGUI_GenericHypothesisCreator::hypType() const
{
  return myHypType;
}

const SMESHGUI_GenericHypothesisCreator::ListOfWidgets& SMESHGUI_GenericHypothesisCreator::widgets() const
{
  return myParamWidgets;
}

SMESHGUI_GenericHypothesisCreator::ListOfWidgets& SMESHGUI_GenericHypothesisCreator::changeWidgets()
{
  return myParamWidgets;
}

bool SMESHGUI_GenericHypothesisCreator::isCreation() const
{
  return myIsCreate;
}

void SMESHGUI_GenericHypothesisCreator::attuneStdWidget( QWidget*, const int ) const
{
}

QString SMESHGUI_GenericHypothesisCreator::caption() const
{
  return QString();
}

QPixmap SMESHGUI_GenericHypothesisCreator::icon() const
{
  return QPixmap();
}

QString SMESHGUI_GenericHypothesisCreator::type() const
{
  return QString();
}
QWidget* SMESHGUI_GenericHypothesisCreator::getCustomWidget( const StdParam & /*param*/,
                                                             QWidget*   /*parent*/) const
{
  return 0;
}
bool SMESHGUI_GenericHypothesisCreator::getParamFromCustomWidget( StdParam& , QWidget* ) const
{
  return false;
}




SMESHGUI_HypothesisDlg::SMESHGUI_HypothesisDlg( SMESHGUI_GenericHypothesisCreator* creator, QWidget* parent )
: QtxDialog( parent, "", true, true ),
  myCreator( creator )
{
  setMinimumSize( 300, height() );
//  setFixedSize( 300, height() );
  myLayout = new QVBoxLayout( mainFrame(), 0, 0 );

  QFrame* titFrame = new QFrame( mainFrame() );
  QHBoxLayout* titLay = new QHBoxLayout( titFrame, 0, 5 );
  
  myIconLabel = new QLabel( titFrame );
  myIconLabel->setScaledContents( false );
  myIconLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  myTypeLabel = new QLabel( titFrame );
  if( creator )
    myTypeLabel->setText( creator->hypType() );

  titLay->addWidget( myIconLabel, 0 );
  titLay->addWidget( myTypeLabel, 0 );
  titLay->addStretch( 1 );

  myLayout->addWidget( titFrame, 0 );

  QString aHypType = creator->hypType();
  if ( aHypType == "LocalLength" )
    myHelpFileName = "/files/arithmetic_1d.htm#Average_length";
  else if ( aHypType == "Arithmetic1D")
    myHelpFileName = "/files/arithmetic_1d.htm#arithmetic_1D";
  else if ( aHypType == "MaxElementArea")
    myHelpFileName = "/files/max._element_area_hypothesis.htm";
  else if ( aHypType == "MaxElementVolume")
    myHelpFileName = "/files/max._element_volume_hypothsis.htm";
  else if ( aHypType == "StartEndLength")
    myHelpFileName = "/files/arithmetic_1d.htm#start_and_end_length";
  else if ( aHypType == "Deflection1D")
    myHelpFileName = "/files/arithmetic_1d.htm#deflection_1D";
  else if ( aHypType == "AutomaticLength")
    myHelpFileName = "/files/arithmetic_1d.htm#automatic_length";
  else if ( aHypType == "NumberOfSegments")
    myHelpFileName = "/files/arithmetic_1d.htm#Number_of_elements";
  else
    myHelpFileName = "";

  connect( this, SIGNAL( dlgHelp() ), this, SLOT( onHelp() ) );

}

SMESHGUI_HypothesisDlg::~SMESHGUI_HypothesisDlg()
{
}

void SMESHGUI_HypothesisDlg::setCustomFrame( QFrame* f )
{
  if( f )
  {
    f->reparent( mainFrame(), QPoint( 0, 0 ) );
    myLayout->insertWidget( 1, f, 1 );
  }
}

void SMESHGUI_HypothesisDlg::accept()
{
  if( !myCreator || myCreator->checkParams() )
    QtxDialog::accept();
}

void SMESHGUI_HypothesisDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app) {
    SMESHGUI* aSMESHGUI = dynamic_cast<SMESHGUI*>( app->activeModule() );
    app->onHelpContextModule(aSMESHGUI ? app->moduleName(aSMESHGUI->moduleName()) : QString(""), myHelpFileName);
  }
  else {
    SUIT_MessageBox::warn1(0, QObject::tr("WRN_WARNING"),
			   QObject::tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
			   arg(app->resourceMgr()->stringValue("ExternalBrowser", "application")).arg(myHelpFileName),
			   QObject::tr("BUT_OK"));
  }
}

void SMESHGUI_HypothesisDlg::setHIcon( const QPixmap& p )
{
  myIconLabel->setPixmap( p );  
}

void SMESHGUI_HypothesisDlg::setType( const QString& t )
{
  myTypeLabel->setText( t );
}
