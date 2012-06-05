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
//  File   : SMESHGUI_Hypotheses.cxx
//  Author : Julia DOROVSKIKH, Open CASCADE S.A.S.
//  SMESH includes

#include "SMESHGUI_Hypotheses.h"

#include "SMESHGUI.h"
#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_SpinBox.h"

// SALOME KERNEL includes
#include <SALOMEDSClient_Study.hxx>
#include <utilities.h>

// SALOME GUI includes
#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <LightApp_Application.h>
#include <SalomeApp_IntSpinBox.h>

// Qt includes
#include <QFrame>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>

#define SPACING 6
#define MARGIN  11

//To disable automatic genericobj management, the following line should be commented.
//Otherwise, it should be uncommented. Refer to KERNEL_SRC/src/SALOMEDSImpl/SALOMEDSImpl_AttributeIOR.cxx
#define WITHGENERICOBJ

SMESHGUI_GenericHypothesisCreator::SMESHGUI_GenericHypothesisCreator( const QString& theHypType )
  : myHypType( theHypType ), myIsCreate( false ), myDlg( 0 )
{
}

SMESHGUI_GenericHypothesisCreator::~SMESHGUI_GenericHypothesisCreator()
{
}

void SMESHGUI_GenericHypothesisCreator::setInitParamsHypothesis(SMESH::SMESH_Hypothesis_ptr hyp)
{
  if ( !CORBA::is_nil( hyp ) && hypType() == hyp->GetName() )
    myInitParamsHypo = SMESH::SMESH_Hypothesis::_duplicate( hyp );
}

void SMESHGUI_GenericHypothesisCreator::create( SMESH::SMESH_Hypothesis_ptr initParamsHyp,
                                                const QString& theHypName,
                                                QWidget* parent, QObject* obj, const QString& slot )
{
  MESSAGE( "Creation of hypothesis with initial params" );
  setInitParamsHypothesis( initParamsHyp );
  create( false, theHypName, parent, obj, slot );
}

void SMESHGUI_GenericHypothesisCreator::create( bool isAlgo,
                                                const QString& theHypName,
                                                QWidget* theParent, QObject* obj, const QString& slot )
{
  MESSAGE( "Creation of hypothesis" );

  myIsCreate = true;

  // Create hypothesis/algorithm
  if (isAlgo) {
    SMESH::SMESH_Hypothesis_var anAlgo =
      SMESH::CreateHypothesis( hypType(), theHypName, isAlgo );
#ifdef WITHGENERICOBJ
    if (!CORBA::is_nil(anAlgo))
      anAlgo->UnRegister();
#endif
  }
  else {
    SMESH::SMESH_Hypothesis_var aHypothesis =
      SMESH::CreateHypothesis( hypType(), theHypName, false );
    editHypothesis( aHypothesis.in(), theHypName, theParent, obj, slot );
#ifdef WITHGENERICOBJ
    if (!CORBA::is_nil(aHypothesis))
      aHypothesis->UnRegister();
#endif
  }
}

void SMESHGUI_GenericHypothesisCreator::edit( SMESH::SMESH_Hypothesis_ptr theHypothesis,
                                              const QString& theHypName,
                                              QWidget* theParent, QObject* obj, const QString& slot )
{
  if( CORBA::is_nil( theHypothesis ) )
    return;

  MESSAGE("Edition of hypothesis");

  myIsCreate = false;

  editHypothesis( theHypothesis, theHypName, theParent, obj, slot );
}

void SMESHGUI_GenericHypothesisCreator::editHypothesis( SMESH::SMESH_Hypothesis_ptr h,
                                                        const QString& theHypName,
                                                        QWidget* theParent,
                                                        QObject* obj, const QString& slot )
{
  myHypName = theHypName;
  myHypo = SMESH::SMESH_Hypothesis::_duplicate( h );
#ifdef WITHGENERICOBJ
  myHypo->Register();
#endif

  SMESHGUI_HypothesisDlg* Dlg = new SMESHGUI_HypothesisDlg( this, theParent );
  connect( Dlg, SIGNAL( finished( int ) ), this, SLOT( onDialogFinished( int ) ) );
  connect( this, SIGNAL( finished( int ) ), obj, slot.toLatin1().constData() );
  connect( SMESHGUI::GetSMESHGUI(), SIGNAL( SignalCloseAllDialogs() ), Dlg, SLOT( reject() ));

  myDlg = Dlg;
  QFrame* fr = buildFrame();
  if( fr )
  {
    Dlg->setCustomFrame( fr );
    Dlg->setWindowTitle( caption() );
    Dlg->setObjectName( theHypName );
    Dlg->setHIcon( icon() );
    Dlg->setType( type() );
    retrieveParams();
    Dlg->show();
    Dlg->resize( Dlg->minimumSizeHint() );
  }
  else {
    emit finished( QDialog::Accepted );
        delete myDlg;
  }
}

QFrame* SMESHGUI_GenericHypothesisCreator::buildStdFrame()
{
  if( CORBA::is_nil( hypothesis() ) )
    return 0;

  ListOfStdParams params;
  if( !stdParams( params ) || params.isEmpty() )
    return 0;

  QFrame* fr = new QFrame( 0 );
  QVBoxLayout* lay = new QVBoxLayout( fr );
  lay->setMargin( 5 );
  lay->setSpacing( 0 );

  QGroupBox* GroupC1 = new QGroupBox( tr( "SMESH_ARGUMENTS" ), fr );
  lay->addWidget( GroupC1 );

  QGridLayout* GroupC1Layout = new QGridLayout( GroupC1 );
  GroupC1Layout->setSpacing( SPACING );
  GroupC1Layout->setMargin( MARGIN );

  ListOfStdParams::const_iterator anIt = params.begin(), aLast = params.end();
  for( int i=0; anIt!=aLast; anIt++, i++ )
  {
    QLabel* lab = new QLabel( (*anIt).myName, GroupC1 );
    GroupC1Layout->addWidget( lab, i, 0 );

    QWidget* w = getCustomWidget( *anIt, GroupC1, i );
    if ( !w )
      switch( (*anIt).myValue.type() )
      {
      case QVariant::Int:
        {
          SalomeApp_IntSpinBox* sb = new SalomeApp_IntSpinBox( GroupC1 );
          sb->setObjectName( (*anIt).myName );
          attuneStdWidget( sb, i );
          sb->setValue( (*anIt).myValue.toInt() );
          connect( sb, SIGNAL( valueChanged( int ) ), this, SLOT( onValueChanged() ) );
          w = sb;
        }
        break;
      case QVariant::Double:
        {
          SalomeApp_DoubleSpinBox* sb = new SMESHGUI_SpinBox( GroupC1 );
          sb->setObjectName( (*anIt).myName );
          attuneStdWidget( sb, i );
          sb->setValue( (*anIt).myValue.toDouble() );
          connect( sb, SIGNAL( valueChanged( double ) ), this, SLOT( onValueChanged() ) );
          w = sb;
        }
        break;
      case QVariant::String:
        {
          if((*anIt).isVariable) {
            _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
            QString aVar = (*anIt).myValue.toString();
            if(aStudy->IsInteger(aVar.toLatin1().constData())){
              SalomeApp_IntSpinBox* sb = new SalomeApp_IntSpinBox( GroupC1 );
              sb->setObjectName( (*anIt).myName );
              attuneStdWidget( sb, i );
              sb->setText( aVar );
              connect( sb, SIGNAL( valueChanged( int ) ), this, SLOT( onValueChanged() ) );
              w = sb;
            }
            else if(aStudy->IsReal(aVar.toLatin1().constData())){
              SalomeApp_DoubleSpinBox* sb = new SMESHGUI_SpinBox( GroupC1 );
              sb->setObjectName( (*anIt).myName );
              attuneStdWidget( sb, i );
              sb->setText( aVar );
              connect( sb, SIGNAL( valueChanged( double ) ), this, SLOT( onValueChanged() ) );
              w = sb;
            }
          }
          else {
            QLineEdit* le = new QLineEdit( GroupC1 );
            le->setObjectName( (*anIt).myName );
            attuneStdWidget( le, i );
            le->setText( (*anIt).myValue.toString() );
            connect( le, SIGNAL( textChanged( const QString& ) ), this, SLOT( onValueChanged() ) );
            w = le;
          }
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
  valueChanged( (QWidget*) sender() );
}

void SMESHGUI_GenericHypothesisCreator::valueChanged( QWidget* )
{
}

void SMESHGUI_GenericHypothesisCreator::onDialogFinished( int result )
{
  bool res = result==QDialog::Accepted;
  if( res )
  {
      /*QString paramValues = */storeParams();
      // No longer needed since NoteBook appears and "Value" OB field shows names of variable
//       if ( !paramValues.isEmpty() ) {
//         if ( _PTR(SObject) SHyp = SMESH::FindSObject( myHypo ))
//           SMESH::SetValue( SHyp, paramValues );
//       }
  }

  changeWidgets().clear();

  if( myIsCreate && !res )
  {
    //remove just created hypothesis
    _PTR(SObject) aHypSObject = SMESH::FindSObject( myHypo );
    _PTR(Study) aStudy = SMESH::GetActiveStudyDocument();
    if( aStudy && !aStudy->GetProperties()->IsLocked() )
    {
      _PTR(StudyBuilder) aBuilder = aStudy->NewBuilder();
      aBuilder->RemoveObjectWithChildren( aHypSObject );
    }
  }
  else if( !myIsCreate && res )
  {
    SMESH::SObjectList listSOmesh = SMESH::GetMeshesUsingAlgoOrHypothesis( myHypo );
    if( listSOmesh.size() > 0 )
      for( int i = 0; i < listSOmesh.size(); i++ )
      {
        _PTR(SObject) submSO = listSOmesh[i];
        SMESH::SMESH_Mesh_var aMesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( submSO );
        SMESH::SMESH_subMesh_var aSubMesh = SMESH::SObjectToInterface<SMESH::SMESH_subMesh>( submSO );
        if( !aSubMesh->_is_nil() )
          aMesh = aSubMesh->GetFather();
        _PTR(SObject) meshSO = SMESH::FindSObject( aMesh );
        SMESH::ModifiedMesh( meshSO, false, aMesh->NbNodes()==0);
      }
  }
  SMESHGUI::GetSMESHGUI()->updateObjBrowser( true, 0 );
#ifdef WITHGENERICOBJ
  myHypo->UnRegister();
#endif
  myHypo = SMESH::SMESH_Hypothesis::_nil();
  myInitParamsHypo = SMESH::SMESH_Hypothesis::_nil();

  disconnect( myDlg, SIGNAL( finished( int ) ), this, SLOT( onDialogFinished( int ) ) );
  myDlg->close();
  //delete myDlg; since WA_DeleteOnClose==true
  myDlg = 0;
  emit finished( result );
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
    item.myName = (*anIt)->objectName();
    if( (*anIt)->inherits( "SalomeApp_IntSpinBox" ) )
    {
      SalomeApp_IntSpinBox* sb = ( SalomeApp_IntSpinBox* )( *anIt );
      item.myValue = sb->value();
      item.myText = sb->text();
      params.append( item );
    }
    else if( (*anIt)->inherits( "SalomeApp_DoubleSpinBox" ) )
    {
      SalomeApp_DoubleSpinBox* sb = ( SalomeApp_DoubleSpinBox* )( *anIt );
      item.myValue = sb->value();
      item.myText = sb->text();
      params.append( item );
    }
    else if( (*anIt)->inherits( "QLineEdit" ) )
    {
      QLineEdit* line = ( QLineEdit* )( *anIt );
      item.myValue = item.myText = line->text();
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

QString SMESHGUI_GenericHypothesisCreator::getVariableName(const char* methodName) const
{
  SMESH::SMESH_Hypothesis_var h = hypothesis();
  if ( !h->_is_nil() )
  {
    CORBA::String_var aVaribaleName = h->GetVarParameter( methodName );
    return QString( aVaribaleName.in() );
  }
  return QString();
}

QStringList SMESHGUI_GenericHypothesisCreator::getVariablesFromDlg() const
{
  QStringList aResult;
  ListOfWidgets::const_iterator anIt = widgets().begin(), aLast = widgets().end();
  for( ; anIt!=aLast; anIt++ ) {
    if( (*anIt)->inherits( "QAbstractSpinBox" ) ) {
      QAbstractSpinBox* sb = ( QAbstractSpinBox* )( *anIt );
      aResult.append(sb->text());
    }
  }
  return aResult;
}

QString SMESHGUI_GenericHypothesisCreator::stdParamValues( const ListOfStdParams& params)
{
  QString valueStr = "";
  ListOfStdParams::const_iterator param = params.begin(), aLast = params.end();
  uint len0 = 0;
  for( int i=0; param!=aLast; param++, i++ )
  {
    if ( valueStr.length() > len0 ) {
      valueStr += "; ";
      len0 = valueStr.length();
    }
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
      valueStr += valCopy.toString();
    }
  }
  return valueStr;
}

SMESH::SMESH_Hypothesis_var SMESHGUI_GenericHypothesisCreator::hypothesis() const
{
  return myHypo;
}

void SMESHGUI_GenericHypothesisCreator::setShapeEntry( const QString& theEntry )
{
  myShapeEntry = theEntry;
}

//================================================================================
/*!
 * \brief Return hypothesis containing initial parameters
 *  \param strictly - if true, always return myInitParamsHypo,
 *                    else, return myInitParamsHypo only in creation mode and if it
 *                    is non-nil
 */
//================================================================================

SMESH::SMESH_Hypothesis_var SMESHGUI_GenericHypothesisCreator::initParamsHypothesis(const bool strictly) const
{
  if ( strictly )
    return myInitParamsHypo;
  if ( !isCreation() || CORBA::is_nil( myInitParamsHypo ))
    return myHypo;
  return myInitParamsHypo;
}

bool SMESHGUI_GenericHypothesisCreator::hasInitParamsHypothesis() const
{
  return !CORBA::is_nil( myInitParamsHypo );
}

QString SMESHGUI_GenericHypothesisCreator::hypType() const
{
  return myHypType;
}

QString SMESHGUI_GenericHypothesisCreator::hypName() const
{
  return myHypName;
}

const SMESHGUI_GenericHypothesisCreator::ListOfWidgets& SMESHGUI_GenericHypothesisCreator::widgets() const
{
  return myParamWidgets;
}

SMESHGUI_GenericHypothesisCreator::ListOfWidgets& SMESHGUI_GenericHypothesisCreator::changeWidgets()
{
  return myParamWidgets;
}

QtxDialog* SMESHGUI_GenericHypothesisCreator:: dlg() const
{
  return myDlg;
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
                                                             QWidget*   /*parent*/,
                                                             const int  /*index*/) const
{
  return 0;
}
bool SMESHGUI_GenericHypothesisCreator::getParamFromCustomWidget( StdParam&, QWidget* ) const
{
  return false;
}

bool SMESHGUI_GenericHypothesisCreator::checkParams( QString& msg ) const
{
  bool ok = true;
  ListOfWidgets::const_iterator anIt = widgets().begin(), aLast = widgets().end();
  for( ; anIt!=aLast; anIt++ )
  {
    if( (*anIt)->inherits( "SalomeApp_IntSpinBox" ) )
    {
      SalomeApp_IntSpinBox* sb = ( SalomeApp_IntSpinBox* )( *anIt );
      ok = sb->isValid( msg, true ) && ok;
    }
    else if( (*anIt)->inherits( "SalomeApp_DoubleSpinBox" ) )
    {
      SalomeApp_DoubleSpinBox* sb = ( SalomeApp_DoubleSpinBox* )( *anIt );
      ok = sb->isValid( msg, true ) && ok;
    }
  }
  return ok;
}

void SMESHGUI_GenericHypothesisCreator::onReject()
{
}

QString SMESHGUI_GenericHypothesisCreator::helpPage() const
{
  QString aHypType = hypType();
  QString aHelpFileName = "";
  if ( aHypType == "LocalLength" )
    aHelpFileName = "a1d_meshing_hypo_page.html#average_length_anchor";
  else if ( aHypType == "MaxLength" )
    aHelpFileName = "a1d_meshing_hypo_page.html#max_length_anchor";
  else if ( aHypType == "Arithmetic1D")
    aHelpFileName = "a1d_meshing_hypo_page.html#arithmetic_1d_anchor";
  else if ( aHypType == "FixedPoints1D")
    aHelpFileName = "a1d_meshing_hypo_page.html#fixed_points_1d_anchor";
  else if ( aHypType == "MaxElementArea")
    aHelpFileName = "a2d_meshing_hypo_page.html#max_element_area_anchor";
  else if ( aHypType == "MaxElementVolume")
    aHelpFileName = "max_element_volume_hypo_page.html";
  else if ( aHypType == "StartEndLength")
    aHelpFileName = "a1d_meshing_hypo_page.html#start_and_end_length_anchor";
  else if ( aHypType == "Deflection1D")
    aHelpFileName = "a1d_meshing_hypo_page.html#deflection_1d_anchor";
  else if ( aHypType == "AutomaticLength")
    aHelpFileName = "a1d_meshing_hypo_page.html#automatic_length_anchor";
  else if ( aHypType == "NumberOfSegments")
    aHelpFileName = "a1d_meshing_hypo_page.html#number_of_segments_anchor";
  else if ( aHypType == "ProjectionSource1D")
    aHelpFileName = "projection_algos_page.html";
  else if ( aHypType == "ProjectionSource2D")
    aHelpFileName = "projection_algos_page.html";
  else if ( aHypType == "ProjectionSource3D")
    aHelpFileName = "projection_algos_page.html";
  else if ( aHypType == "NumberOfLayers")
    aHelpFileName = "radial_prism_algo_page.html";
  else if ( aHypType == "NumberOfLayers2D")
    aHelpFileName = "radial_quadrangle_1D2D_algo_page.html";
  else if ( aHypType == "LayerDistribution")
    aHelpFileName = "radial_prism_algo_page.html";
  else if ( aHypType == "LayerDistribution2D")
    aHelpFileName = "radial_quadrangle_1D2D_algo_page.html";
  else if ( aHypType == "SegmentLengthAroundVertex")
    aHelpFileName = "segments_around_vertex_algo_page.html";
  else if ( aHypType == "QuadrangleParams")
    aHelpFileName = "a2d_meshing_hypo_page.html#hypo_quad_params_anchor";
  else if ( aHypType == "ViscousLayers")
    aHelpFileName = "additional_hypo_page.html#viscous_layers_anchor";
  else if ( aHypType == "ImportSource1D" || aHypType == "ImportSource2D")
    aHelpFileName = "import_algos_page.html";
  return aHelpFileName;
}

SMESHGUI_HypothesisDlg::SMESHGUI_HypothesisDlg( SMESHGUI_GenericHypothesisCreator* creator, QWidget* parent )
: QtxDialog( parent, false, true ),
  myCreator( creator )
{
  setAttribute(Qt::WA_DeleteOnClose, true);
 // setMinimumSize( 300, height() );
//  setFixedSize( 300, height() );
  QVBoxLayout* topLayout = new QVBoxLayout( mainFrame() );
  topLayout->setMargin( 0 );
  topLayout->setSpacing( 0 );

  QFrame* titFrame = new QFrame( mainFrame() );
  QHBoxLayout* titLay = new QHBoxLayout( titFrame );
  titLay->setMargin( 0 );
  titLay->setSpacing( SPACING );

  myIconLabel = new QLabel( titFrame );
  myIconLabel->setScaledContents( false );
  myIconLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  myTypeLabel = new QLabel( titFrame );
  if( creator )
    myTypeLabel->setText( creator->hypType() );

  titLay->addWidget( myIconLabel, 0 );
  titLay->addWidget( myTypeLabel, 0 );
  titLay->addStretch( 1 );

  topLayout->addWidget( titFrame, 0 );

  myHelpFileName = creator->helpPage();

  connect( this, SIGNAL( dlgHelp() ), this, SLOT( onHelp() ) );
}

SMESHGUI_HypothesisDlg::~SMESHGUI_HypothesisDlg()
{
  delete myCreator;
}

void SMESHGUI_HypothesisDlg::setCustomFrame( QFrame* f )
{
  if( f )
  {
    f->setParent( mainFrame() );
    qobject_cast<QVBoxLayout*>( mainFrame()->layout() )->insertWidget( 1, f, 1 );
  }
}

void SMESHGUI_HypothesisDlg::accept()
{
  QString msg;
  if ( myCreator && !myCreator->checkParams( msg ) )
  {
    QString str( tr( "SMESH_INCORRECT_INPUT" ) );
    if ( !msg.isEmpty() )
      str += "\n" + msg;
    SUIT_MessageBox::critical( this, tr( "SMESH_ERROR" ), str );
    return;
  }
  QtxDialog::accept();
}

void SMESHGUI_HypothesisDlg::reject()
{
  if ( myCreator ) myCreator->onReject();
  QtxDialog::reject();
}

void SMESHGUI_HypothesisDlg::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app) {
    QString name = "SMESH";
    if(myCreator) {
      QVariant pluginName = myCreator->property( PLUGIN_NAME );
      if( pluginName.isValid() ) {
	QString rootDir = pluginName.toString() + "PLUGIN_ROOT_DIR";
	QString varValue = QString( getenv(rootDir.toLatin1().constData()));
	if(!varValue.isEmpty())
	  name = pluginName.toString() + "PLUGIN";
      }
    }    
    app->onHelpContextModule(name, myHelpFileName);
  }
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

void SMESHGUI_HypothesisDlg::setHIcon( const QPixmap& p )
{
  myIconLabel->setPixmap( p );
}

void SMESHGUI_HypothesisDlg::setType( const QString& t )
{
  myTypeLabel->setText( t );
}

HypothesisData::HypothesisData( const QString& theTypeName,
                                const QString& thePluginName,
                                const QString& theServerLibName,
                                const QString& theClientLibName,
                                const QString& theLabel,
                                const QString& theIconId,
                                const QList<int>& theDim,
                                const bool theIsAux,
                                const QStringList& theNeededHypos,
                                const QStringList& theOptionalHypos,
                                const QStringList& theInputTypes,
                                const QStringList& theOutputTypes,
                                const bool theIsNeedGeometry,
                                const bool supportSub)
  : TypeName( theTypeName ),
    PluginName( thePluginName ),
    ServerLibName( theServerLibName ),
    ClientLibName( theClientLibName ),
    Label( theLabel ),
    IconId( theIconId ),
    Dim( theDim ),
    IsAux( theIsAux ),
    NeededHypos( theNeededHypos ),
    OptionalHypos( theOptionalHypos ),
    InputTypes( theInputTypes ),
    OutputTypes( theOutputTypes ),
    IsNeedGeometry( theIsNeedGeometry ),
    IsSupportSubmeshes( supportSub )
{
}

HypothesesSet::HypothesesSet( const QString& theSetName )
  : myHypoSetName( theSetName ),
    myIsAlgo( false ),
    myIsCustom( false )
{
}

HypothesesSet::HypothesesSet( const QString&     theSetName,
                              const QStringList& theHypoList,
                              const QStringList& theAlgoList )
  : myHypoSetName( theSetName ),
    myHypoList( theHypoList ),
    myAlgoList( theAlgoList ),
    myIsAlgo( false ),
    myIsCustom( false )
{
}

QStringList* HypothesesSet::list(bool is_algo) const
{
  return const_cast<QStringList*>( &( is_algo ? myAlgoList : myHypoList ) );
}

QStringList* HypothesesSet::list() const
{
  return list( myIsAlgo );
}

QString HypothesesSet::name() const
{
  return myHypoSetName;
}

void HypothesesSet::set( bool isAlgo, const QStringList& lst )
{
  *list(isAlgo) = lst;
}

int HypothesesSet::count( bool isAlgo ) const
{
  return list(isAlgo)->count();
}

bool HypothesesSet::isAlgo() const
{
  return myIsAlgo;
}

void HypothesesSet::init( bool isAlgo )
{
  myIsAlgo = isAlgo;
  myIndex = -1;
}

bool HypothesesSet::more() const
{
  return myIndex < list()->count();
}

void HypothesesSet::next()
{
  myIndex++;
}

QString HypothesesSet::current() const
{
  return list()->at(myIndex);
}

void HypothesesSet::setIsCustom( bool isCustom )
{
  myIsCustom = isCustom;
}

bool HypothesesSet::getIsCustom() const
{
  return myIsCustom;
}

int HypothesesSet::maxDim() const
{
  HypothesesSet * thisSet = (HypothesesSet*) this;
  int dim = -1;
  for ( int isAlgo = 0; isAlgo < 2; ++isAlgo )
  {
    thisSet->init( isAlgo );
    while ( thisSet->next(), thisSet->more() )
      if ( HypothesisData* hypData = SMESH::GetHypothesisData( thisSet->current() ))
        for ( int i = 0; i < hypData->Dim.count(); ++i )
          dim = qMax( dim, hypData->Dim[i] );
  }
  return dim;
}
