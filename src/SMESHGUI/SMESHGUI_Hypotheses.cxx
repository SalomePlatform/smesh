// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMESHGUI_Hypotheses.cxx
//  Author : Julia DOROVSKIKH, Open CASCADE S.A.S.
//  SMESH includes

#include "SMESHGUI_Hypotheses.h"

#include "SMESHGUI.h"
#include "SMESHGUI_HypothesesUtils.h"
#include "SMESHGUI_SpinBox.h"
#include "SMESHGUI_Utils.h"
#include "SMESHGUI_VTKUtils.h"
#include "SMESH_Actor.h"
#include "SMESH_TypeDefs.hxx"

// SALOME KERNEL includes
#include <SALOMEDSClient_Study.hxx>
#include <utilities.h>

// SALOME GUI includes
#include <LightApp_Application.h>
#include <SUIT_MessageBox.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SalomeApp_IntSpinBox.h>
#include <SalomeApp_Tools.h>
#include <SVTK_ViewWindow.h>

// Qt includes
#include <QFrame>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>

#define SPACING 6
#define MARGIN  11

SMESHGUI_GenericHypothesisCreator::SMESHGUI_GenericHypothesisCreator( const QString& theHypType )
  : myToDeleteInitParamsHypo( false ),
    myHypType( theHypType ),
    myIsCreate( false ),
    myDlg( 0 )
{
}

SMESHGUI_GenericHypothesisCreator::~SMESHGUI_GenericHypothesisCreator()
{
  if ( myToDeleteInitParamsHypo && !myInitParamsHypo->_is_nil() )
    myInitParamsHypo->UnRegister();
}

void SMESHGUI_GenericHypothesisCreator::setInitParamsHypothesis(SMESH::SMESH_Hypothesis_ptr hyp)
{
  if ( !CORBA::is_nil( hyp ) ) {
    if ( myToDeleteInitParamsHypo && !myInitParamsHypo->_is_nil() )
      myInitParamsHypo->UnRegister();
    CORBA::String_var hypName = hyp->GetName();
    if ( hypType() == hypName.in() )
    {
      myInitParamsHypo         = SMESH::SMESH_Hypothesis::_duplicate( hyp );
      myToDeleteInitParamsHypo = !SMESH::FindSObject( myInitParamsHypo );
    }
  }
}

void SMESHGUI_GenericHypothesisCreator::create( SMESH::SMESH_Hypothesis_ptr initParamsHyp,
                                                const QString& theHypName,
                                                QWidget* parent, QObject* obj, const QString& slot )
{
  setInitParamsHypothesis( initParamsHyp );
  create( false, theHypName, parent, obj, slot );
}

void SMESHGUI_GenericHypothesisCreator::create( bool           isAlgo,
                                                const QString& theHypName,
                                                QWidget*       theParent,
                                                QObject*       obj,
                                                const QString& slot )
{
  myIsCreate = true;

  // Create hypothesis/algorithm
  if (isAlgo) {
    SMESH::SMESH_Hypothesis_var anAlgo =
      SMESH::CreateHypothesis( hypType(), theHypName, isAlgo );
    anAlgo.out(); // avoid unused variable warning
  }
  else {
    SMESH::SMESH_Hypothesis_var aHypothesis =
      SMESH::CreateHypothesis( hypType(), theHypName, false );
    editHypothesis( aHypothesis.in(), theHypName, theParent, obj, slot );
  }
}

void SMESHGUI_GenericHypothesisCreator::edit( SMESH::SMESH_Hypothesis_ptr theHypothesis,
                                              const QString&              theHypName,
                                              QWidget* theParent, QObject* obj, const QString& slot )
{
  if( CORBA::is_nil( theHypothesis ) )
    return;

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
  myHypo->Register();

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
    if ( theHypName == HypothesesSet::getCommonHypoSetHypoType() )
      Dlg->setType( tr( HypothesesSet::getCommonHypoSetHypoType()) );
    else
      Dlg->setType( type() );
    retrieveParams();
    Dlg->show(); // w/o this Dlg blocks selection
    Dlg->resize( Dlg->minimumSizeHint() );
    Dlg->exec(); // w/o this we cant wait until edition ends when applying a hypo-set
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
  for( int i = 0; anIt != aLast; anIt++, i++ )
  {
    QLabel* lab = anIt->hasName() ? new QLabel( anIt->myName, GroupC1 ) : NULL;
    if ( lab )
      GroupC1Layout->addWidget( lab, i, 0 );
    myParamLabels << lab;

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
            _PTR(Study) aStudy = SMESH::getStudy();
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

      default:;
      } // switch( (*anIt).myValue.type() )

    if ( w )
    {
      if ( lab )
        GroupC1Layout->addWidget( w, i, 1 );
      else
        GroupC1Layout->addWidget( w, i, 0, 1, 2 );
      changeWidgets().append( w );
    }
  }
  if ( QWidget* w = getHelperWidget() )
  {
    w->setParent( fr );
    w->move( QPoint( 0, 0 ) );
    lay->addWidget( w );
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
    try
    {
      SUIT_OverrideCursor wc;
      storeParams();
      // No longer needed since NoteBook appears and "Value" OB field shows names of variable
      // QString paramValues = storeParams();
      // if ( !paramValues.isEmpty() ) {
      //   if ( _PTR(SObject) SHyp = SMESH::FindSObject( myHypo ))
      //     SMESH::SetValue( SHyp, paramValues );
      // }
    }
    catch ( const SALOME::SALOME_Exception& S_ex ) {
      SalomeApp_Tools::QtCatchCorbaException( S_ex );
    }
  }

  changeWidgets().clear();

  if( myIsCreate && !res )
  {
    //remove just created hypothesis
    _PTR(SObject) aHypSObject = SMESH::FindSObject( myHypo );
    _PTR(Study) aStudy = SMESH::getStudy();
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
      for( size_t i = 0; i < listSOmesh.size(); i++ )
      {
        _PTR(SObject) submSO = listSOmesh[i];
        SMESH::SMESH_Mesh_var aMesh = SMESH::SObjectToInterface<SMESH::SMESH_Mesh>( submSO );
        SMESH::SMESH_subMesh_var aSubMesh = SMESH::SObjectToInterface<SMESH::SMESH_subMesh>( submSO );
        if( !aSubMesh->_is_nil() )
          aMesh = aSubMesh->GetFather();
        _PTR(SObject) meshSO = SMESH::FindSObject( aMesh );
        SMESH_Actor* actor = SMESH::FindActorByEntry( meshSO->GetID().c_str() );
        if( actor && actor->GetVisibility() )
          actor->Update();
      }
  }
  SMESHGUI::GetSMESHGUI()->updateObjBrowser( true, 0 );
  myHypo->UnRegister();
  myHypo = SMESH::SMESH_Hypothesis::_nil();
  myInitParamsHypo = SMESH::SMESH_Hypothesis::_nil();

  disconnect( myDlg, SIGNAL( finished( int ) ), this, SLOT( onDialogFinished( int ) ) );
  myDlg->close();
  //delete myDlg; since WA_DeleteOnClose==true
  myDlg = 0;

  SMESH::UpdateActorsAfterUpdateStudy();// remove actors of removed groups (#16522)
  if (SVTK_ViewWindow* vf = SMESH::GetCurrentVtkView()) {
    vf->Repaint();
  }
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
  int len0 = 0;
  for ( ; param != aLast; param++ )
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

//================================================================================
/*!
 * \brief Returns a QLabel of a specified parameter.
 * If isCreation(), the 1st label (supposed to be "Name") is not countered.
 */
//================================================================================

QLabel* SMESHGUI_GenericHypothesisCreator::getLabel(int i) const
{
  if ( isCreation() )
    i++;
  if ( i < myParamLabels.size() )
    return (QLabel*) myParamLabels.at(i);
  return NULL;
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
//================================================================================
/*!
 * \brief Returns a widget representing not a hypothesis parameter but some helper widget
 */
//================================================================================

QWidget* SMESHGUI_GenericHypothesisCreator::getHelperWidget() const
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
    aHelpFileName = "1d_meshing_hypo.html#average-length-anchor";
  else if ( aHypType == "MaxLength" )
    aHelpFileName = "1d_meshing_hypo.html#max-length-anchor";
  else if ( aHypType == "Arithmetic1D")
    aHelpFileName = "1d_meshing_hypo.html#arithmetic-1d-anchor";
  else if ( aHypType == "GeometricProgression")
    aHelpFileName = "1d_meshing_hypo.html#geometric-1d-anchor";
  else if ( aHypType == "FixedPoints1D")
    aHelpFileName = "1d_meshing_hypo.html#fixed-points-1d-anchor";
  else if ( aHypType == "MaxElementArea")
    aHelpFileName = "2d_meshing_hypo.html#max-element-area-anchor";
  else if ( aHypType == "MaxElementVolume")
    aHelpFileName = "3d_meshing_hypo.html#max-element-volume-hypothesis";
  else if ( aHypType == "StartEndLength")
    aHelpFileName = "1d_meshing_hypo.html#start-and-end-length-anchor";
  else if ( aHypType == "Deflection1D")
    aHelpFileName = "1d_meshing_hypo.html#deflection-1d-anchor";
  else if ( aHypType == "Adaptive1D")
    aHelpFileName = "1d_meshing_hypo.html#adaptive-1d-anchor";
  else if ( aHypType == "AutomaticLength")
    aHelpFileName = "1d_meshing_hypo.html#automatic-length-anchor";
  else if ( aHypType == "NumberOfSegments")
    aHelpFileName = "1d_meshing_hypo.html#number-of-segments-anchor";
  else if ( aHypType == "ProjectionSource1D")
    aHelpFileName = "projection_algos.html";
  else if ( aHypType == "ProjectionSource2D")
    aHelpFileName = "projection_algos.html";
  else if ( aHypType == "ProjectionSource3D")
    aHelpFileName = "projection_algos.html";
  else if ( aHypType == "NumberOfLayers")
    aHelpFileName = "radial_prism_algo.html";
  else if ( aHypType == "NumberOfLayers2D")
    aHelpFileName = "radial_quadrangle_1D2D_algo.html";
  else if ( aHypType == "LayerDistribution")
    aHelpFileName = "radial_prism_algo.html";
  else if ( aHypType == "LayerDistribution2D")
    aHelpFileName = "radial_quadrangle_1D2D_algo.html";
  else if ( aHypType == "SegmentLengthAroundVertex")
    aHelpFileName = "segments_around_vertex_algo.html";
  else if ( aHypType == "QuadrangleParams")
    aHelpFileName = "2d_meshing_hypo.html#hypo-quad-params-anchor";
  else if ( aHypType == "ViscousLayers")
    aHelpFileName = "additional_hypo.html#viscous-layers-anchor";
  else if ( aHypType == "ViscousLayers2D")
    aHelpFileName = "additional_hypo.html#viscous-layers-anchor";
  else if ( aHypType == "ImportSource1D" || aHypType == "ImportSource2D")
    aHelpFileName = "use_existing_algos.html";
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

void SMESHGUI_HypothesisDlg::showEvent(QShowEvent *event)
{
  // resize( minimumSizeHint() );
  // adjustSize();

  QtxDialog::showEvent( event );
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
  SUIT_OverrideCursor wc; // some creators temporary set params to a hyp which can be long
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
      QVariant pluginName = myCreator->property( SMESH::Plugin_Name() );
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

HypothesisData::HypothesisData( const QString&     theTypeName,
                                const QString&     thePluginName,
                                const QString&     theServerLibName,
                                const QString&     theClientLibName,
                                const QString&     theLabel,
                                const QString&     theIconId,
                                const QString&     theContext,
                                const int          theGroupID,
                                const int          thePriority,
                                const QList<int>&  theDim,
                                const bool         theIsAuxOrNeedHyp,
                                const QStringList& theBasicHypos,
                                const QStringList& theOptionalHypos,
                                const QStringList& theInputTypes,
                                const QStringList& theOutputTypes,
                                const int          theIsNeedGeometry,
                                const bool         theSupportSub)
  : TypeName( theTypeName ),
    PluginName( thePluginName ),
    ServerLibName( theServerLibName ),
    ClientLibName( theClientLibName ),
    Label( theLabel ),
    IconId( theIconId ),
    Context( theContext ),
    GroupID( theGroupID ),
    Priority( thePriority ),
    Dim( theDim ),
    IsAuxOrNeedHyp( theIsAuxOrNeedHyp ),
    IsNeedGeometry( theIsNeedGeometry ),
    IsSupportSubmeshes( theSupportSub ),
    BasicHypos( theBasicHypos ),
    OptionalHypos( theOptionalHypos ),
    InputTypes( theInputTypes ),
    OutputTypes( theOutputTypes )
{
}

// HypothesesSet::HypothesesSet( const QString& theSetName )
//   : myHypoSetName( theSetName ),
//     myIsAlgo( false ),
//     myIsCustom( false )
// {
// }

HypothesesSet::HypothesesSet( const QString& theSetName,
                              bool useCommonSize, bool isQuadDominated,
                              const QStringList& mainHypos, const QStringList& mainAlgos,
                              const QStringList& altHypos,  const QStringList& altAlgos,
                              const QStringList& intHypos,  const QStringList& intAlgos )
  : myUseCommonSize( useCommonSize ),
    myQuadDominated( isQuadDominated ),
    myHypoSetName( theSetName ),
    myHypoList { mainHypos, altHypos, intHypos },
    myAlgoList { mainAlgos, altAlgos, intAlgos },
    myIsAlgo( false ),
    myIsCustom( false ),
    myIndex( 0 )
{
  for ( myHypType = MAIN; myHypType < NB_HYP_TYPES; SMESHUtils::Increment( myHypType ))
    for ( int isAlgo = myIsAlgo = 0; isAlgo < 2; myIsAlgo = ++isAlgo )
    {
      QStringList& hyps = *list();
      for ( int i = 0; i < hyps.count(); ++i )
        hyps[ i ] = hyps[ i ].trimmed();
    }
}

QStringList* HypothesesSet::list( bool is_algo, SetType setType) const
{
  return const_cast<QStringList*>( &( is_algo ? myAlgoList[setType] : myHypoList[setType] ));
}

QStringList* HypothesesSet::list() const
{
  return list( myIsAlgo, myHypType );
}

QString HypothesesSet::name() const
{
  return myHypoSetName;
}

// void HypothesesSet::set( bool isAlgo, const QStringList& lst )
// {
//   *list(isAlgo) = lst;
// }

// int HypothesesSet::count( bool isAlgo, SetType setType ) const
// {
//   return list(isAlgo,setType)->count();
// }

// bool HypothesesSet::isAlgo() const
// {
//   return myIsAlgo;
// }

void HypothesesSet::init( bool isAlgo, SetType setType )
{
  myHypType = setType;
  myIsAlgo = isAlgo;
  myIndex = 0;
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

void HypothesesSet::setAlgoAvailable( SetType type, bool isAvailable )
{
  if ( MAIN <= type && type < NB_HYP_TYPES )
    myIsAlgoAvailable[ type ] = isAvailable;
}

bool HypothesesSet::getAlgoAvailable( SetType type )
{
  bool isAva = false;
  if ( MAIN <= type && type < NB_HYP_TYPES )
    isAva = myIsAlgoAvailable[ type ];
  return isAva;
}

HypothesesSet::SetType HypothesesSet::getPreferredHypType()
{
  SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
  int useAltHypos = !resMgr->booleanValue( "SMESH", "use-meshgems-hypo-sets", false );
  return ( HypothesesSet::SetType ) useAltHypos;
}

int HypothesesSet::maxDim() const
{
  HypothesesSet * thisSet = (HypothesesSet*) this;
  int dim = -1;
  for ( int setType = 0; setType < 2; ++setType )
    for ( int isAlgo = 0; isAlgo < 2; ++isAlgo )
    {
      for ( thisSet->init( isAlgo, SetType( setType )); thisSet->more(); thisSet->next() )
        if ( HypothesisData* hypData = SMESH::GetHypothesisData( thisSet->current() ))
          for ( int i = 0; i < hypData->Dim.count(); ++i )
            dim = qMax( dim, hypData->Dim[i] );
    }
  return dim;
}

const char* HypothesesSet::getCommonHypoSetHypoType()
{
  return "AverageLengthForHypoSet";
}
