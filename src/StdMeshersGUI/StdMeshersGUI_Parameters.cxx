//  SMESH StdMeshersGUI : GUI for standard meshers
//
//  Copyright (C) 2003  CEA
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org
//
//
//
//  File   : StdMeshersGUI_Parameters.cxx
//  Module : SMESH
//  $Header$

#include "StdMeshersGUI_Parameters.h"

#include <qobject.h>
#include <qhbox.h>
#include <qslider.h>
#include <qlabel.h>

#include <math.h>
//#include <float.h>

using namespace std;

#define VALUE_MAX   1.0e+15 // COORD_MAX
#define VALUE_MAX_2 (VALUE_MAX*VALUE_MAX)
#define VALUE_MAX_3 (VALUE_MAX*VALUE_MAX*VALUE_MAX)

#define VALUE_SMALL   1.0e-15
#define VALUE_SMALL_2 (VALUE_SMALL*VALUE_SMALL)
#define VALUE_SMALL_3 (VALUE_SMALL*VALUE_SMALL*VALUE_SMALL)

//=======================================================================
//function : HasParameters
//purpose  : 
//=======================================================================

bool StdMeshersGUI_Parameters::HasParameters (const QString& hypType)
{
  return ((hypType.compare("LocalLength") == 0) ||
          (hypType.compare("NumberOfSegments") == 0) ||
          (hypType.compare("MaxElementArea") == 0) ||
          (hypType.compare("MaxElementVolume") == 0) ||
          (hypType.compare("StartEndLength") == 0) ||
          (hypType.compare("Deflection1D") == 0) ||
          (hypType.compare("AutomaticLength") == 0) ||
	  (hypType.compare("Arithmetic1D") == 0));
}

//=======================================================================
//function : SetInitValue
//purpose  : 
//=======================================================================

void StdMeshersGUI_Parameters::SetInitValue(SMESHGUI_aParameterPtr param,
                                            int                    initValue)
{
  SMESHGUI_intParameter* p = dynamic_cast<SMESHGUI_intParameter*>(param.get());
  if ( p )
  {
    p->InitValue() = initValue;
    return;
  }
  
  SMESHGUI_enumParameter* q = dynamic_cast<SMESHGUI_enumParameter*>(param.get());
  if( q )
  {
    q->InitValue() = initValue;
    return;
  }

  SMESHGUI_boolParameter* b = dynamic_cast<SMESHGUI_boolParameter*>(param.get());
  if( b )
  {
    b->InitValue() = (bool)initValue;
    return;
  }
}

//=======================================================================
//function : SetInitValue
//purpose  : 
//=======================================================================

void StdMeshersGUI_Parameters::SetInitValue(SMESHGUI_aParameterPtr param,
                                            double                 initValue)
{
  SMESHGUI_doubleParameter* p = dynamic_cast<SMESHGUI_doubleParameter*>(param.get());
  if ( p ) p->InitValue() = initValue;
}

//=======================================================================
//function : SetInitValue
//purpose  : 
//=======================================================================

void StdMeshersGUI_Parameters::SetInitValue(SMESHGUI_aParameterPtr param,
                                            const char*            initValue)
{
  SMESHGUI_strParameter* p = dynamic_cast<SMESHGUI_strParameter*>(param.get());
  if ( p ) p->InitValue() = initValue;
}

//=======================================================================
//function : SetInitValue
//purpose  :
//=======================================================================
void StdMeshersGUI_Parameters::SetInitValue( SMESHGUI_aParameterPtr param,
                                             SMESH::double_array&   initValue)
{
  SMESHGUI_tableParameter* p = dynamic_cast<SMESHGUI_tableParameter*>(param.get());
  if( p )
  {
    p->setRowCount( initValue.length()/2 );
    p->setData( initValue );
  }
}

//================================================================================
/*!
 * \brief Macros to comfortably create SMESHGUI_aParameterPtr of different types
 */
//================================================================================

// SMESHGUI_doubleParameter( initValue, label, bottom, top, step, decimals )
#define DOUBLE_PARAM(v,l,b,t,s,d) SMESHGUI_aParameterPtr(new SMESHGUI_doubleParameter(v,l,b,t,s,d))
#define INT_PARAM(v,l,b,t) SMESHGUI_aParameterPtr(new SMESHGUI_intParameter(v,l,b,t))
#define ENUM_PARAM(v,i,l) SMESHGUI_aParameterPtr(new SMESHGUI_enumParameter(v,i,l))
#define STR_PARAM(i,l,preview) SMESHGUI_aParameterPtr(new SMESHGUI_strParameter(i,l,preview))
#define BOOL_PARAM(i,l,preview) SMESHGUI_aParameterPtr(new SMESHGUI_boolParameter(i,l,preview))

//================================================================================
/*!
 * \brief Fill parameter list with default values
  * \param hypType - The name of hypothesis type
  * \param paramList - The list to fill
 */
//================================================================================

void StdMeshersGUI_Parameters::GetParameters (const QString&                 hypType,
                                              list<SMESHGUI_aParameterPtr> & paramList )
{
  paramList.clear();

  if (hypType.compare("LocalLength") == 0)
  {
    paramList.push_back( DOUBLE_PARAM (1.0,
                                       QObject::tr("SMESH_LOCAL_LENGTH_PARAM"),
                                       VALUE_SMALL, VALUE_MAX, 1.0, 6));
  }
  else if (hypType.compare("NumberOfSegments") == 0)
  {
    //0-th parameter in list
    paramList.push_back ( INT_PARAM (3,
                                     QObject::tr("SMESH_NB_SEGMENTS_PARAM"),
                                     1, 9999 ));
    QStringList types;
    types.append( QObject::tr( "SMESH_DISTR_REGULAR" ) );
    types.append( QObject::tr( "SMESH_DISTR_SCALE"   ) );
    types.append( QObject::tr( "SMESH_DISTR_TAB"     ) );
    types.append( QObject::tr( "SMESH_DISTR_EXPR"    ) );
    //string description of distribution types

    SMESHGUI_enumParameter* type = new SMESHGUI_enumParameter( types, 0, QObject::tr( "SMESH_DISTR_TYPE" ) );
    SMESHGUI_dependParameter::ShownMap& aMap = type->shownMap();
    aMap[0].append( 0 ); // if DistrType=0 (regular), then number of segments and types are shown (0-th and 1-th)
    aMap[0].append( 1 );
    aMap[1].append( 0 ); // if DistrType=1 (scale), then number of segments, types and scale are shown
    aMap[1].append( 1 );
    aMap[1].append( 2 );
    aMap[2].append( 0 ); // if DistrType=2 (table), then number of segments, types, table and exponent are shown
    aMap[2].append( 1 );
    aMap[2].append( 3 );
    aMap[2].append( 5 );
    aMap[3].append( 0 ); // if DistrType=3 (expression), then number of segments, types, expression and exponent are shown
    aMap[3].append( 1 );
    aMap[3].append( 4 );
    aMap[3].append( 5 );
    //1-th parameter in list
    paramList.push_back ( SMESHGUI_aParameterPtr( type ) );

    //2-th parameter in list
    paramList.push_back ( DOUBLE_PARAM (1.0,
                                     QObject::tr("SMESH_NB_SEGMENTS_SCALE_PARAM"),
                                     VALUE_SMALL, VALUE_MAX, 0.1, 6 ));
    SMESHGUI_tableParameter* tab = new SMESHGUI_tableParameter( 0.0, QObject::tr( "SMESH_TAB_FUNC" ), true );
    tab->setRowCount( 5 );
    tab->setColCount( 2 );
    //default size of table: 5x2
    
    tab->setColName( 0, "t" );
    tab->setColName( 1, "f(t)" );    
    tab->setValidator( 0, 0.0, 1.0, 3 );
    tab->setValidator( 1, 1E-7, 1E+300, 3 );
    tab->setEditRows( true );

    //3-th parameter in list
    paramList.push_back ( SMESHGUI_aParameterPtr( tab ) );

    //4-th parameter in list
    paramList.push_back ( STR_PARAM ( "", QObject::tr( "SMESH_EXPR_FUNC" ), true ) );

    //5-th parameter in list
    paramList.push_back ( BOOL_PARAM ( false, QObject::tr( "SMESH_EXP_MODE" ), true ) );
  }
  else if (hypType.compare("Arithmetic1D") == 0)
  {
    paramList.push_back( DOUBLE_PARAM ( 1.0,
                                      QObject::tr("SMESH_START_LENGTH_PARAM"), 
                                      VALUE_SMALL, VALUE_MAX, 1, 6));
    paramList.push_back( DOUBLE_PARAM ( 10.0,
                                       QObject::tr("SMESH_END_LENGTH_PARAM"),
                                       VALUE_SMALL, VALUE_MAX, 1, 6));
  }
  else if (hypType.compare("MaxElementArea") == 0)
  {
    paramList.push_back( DOUBLE_PARAM (1.0,
                                       QObject::tr("SMESH_MAX_ELEMENT_AREA_PARAM"), 
                                       VALUE_SMALL_2, VALUE_MAX_2, 1.0, 6));
  }
  else if (hypType.compare("MaxElementVolume") == 0)
  {
    paramList.push_back( DOUBLE_PARAM ( 1.0,
                                       QObject::tr("SMESH_MAX_ELEMENT_VOLUME_PARAM"), 
                                       VALUE_SMALL_3, VALUE_MAX_3, 1.0, 6));
  }
  else if (hypType.compare("StartEndLength") == 0)
  {
    paramList.push_back( DOUBLE_PARAM ( 1.0,
                                      QObject::tr("SMESH_START_LENGTH_PARAM"), 
                                      VALUE_SMALL, VALUE_MAX, 1, 6));
    paramList.push_back( DOUBLE_PARAM ( 10.0,
                                       QObject::tr("SMESH_END_LENGTH_PARAM"),
                                       VALUE_SMALL, VALUE_MAX, 1, 6));
  }
  else if (hypType.compare("Deflection1D") == 0)
  {
    paramList.push_back( DOUBLE_PARAM ( 1.0,
                                       QObject::tr("SMESH_DEFLECTION1D_PARAM"), 
                                       VALUE_SMALL, VALUE_MAX, 1, 6));
  }
  else if (hypType.compare("AutomaticLength") == 0)
  {
    SMESHGUI_aParameter * param =
      new StdMeshersGUI_doubleSliderParameter ( QObject::tr("SMESH_FINENESS_PARAM"),
                                                "0 ", " 1",
                                                0.0, 0.0, 1.0, 0.05);
    paramList.push_back( SMESHGUI_aParameterPtr( param ));
  }
}

//================================================================================
/*!
 * \brief  Fill parameter list with real values the hypothesis has
  * \param theHyp - The hypothesis to retrieve parameter values from
  * \param paramList - The list to fill
 */
//================================================================================

void StdMeshersGUI_Parameters::GetParameters (SMESH::SMESH_Hypothesis_ptr    theHyp,
                                              list<SMESHGUI_aParameterPtr> & paramList )
{
  paramList.clear();

  if (theHyp->_is_nil()) return;

  QString hypType = theHyp->GetName();
  GetParameters( hypType, paramList ); // get default parameters
  if ( paramList.empty() )
    return;

  // set current values
  if (hypType.compare("LocalLength") == 0)
  {
    StdMeshers::StdMeshers_LocalLength_var LL =
      StdMeshers::StdMeshers_LocalLength::_narrow(theHyp);
    SetInitValue( paramList.front(), LL->GetLength() );
  }
  else if (hypType.compare("NumberOfSegments") == 0)
  {
    StdMeshers::StdMeshers_NumberOfSegments_var NOS =
      StdMeshers::StdMeshers_NumberOfSegments::_narrow(theHyp);
      
    list<SMESHGUI_aParameterPtr>::iterator anIt = paramList.begin();
    SetInitValue( *anIt, (int) NOS->GetNumberOfSegments()); anIt++;
    int DType = (int) NOS->GetDistrType();
    SetInitValue( *anIt, DType ); anIt++;
    
    if( DType==1 )
      SetInitValue( *anIt, NOS->GetScaleFactor());
    anIt++;

    if( DType==2 )
    {
      SMESH::double_array* tab_func = NOS->GetTableFunction();
      SetInitValue( *anIt, *tab_func );
      delete tab_func;
    }
    anIt++;

    if( DType==3 )
    {
      char* expr_func = NOS->GetExpressionFunction();
      SetInitValue( *anIt, expr_func );
      //delete expr_func;
    }
    anIt++;

    if( DType==2 || DType==3 )
      SetInitValue( *anIt, (bool)NOS->IsExponentMode());
  }
  else if (hypType.compare("Arithmetic1D") == 0)
  {
    StdMeshers::StdMeshers_Arithmetic1D_var hyp =
      StdMeshers::StdMeshers_Arithmetic1D::_narrow(theHyp);
    SetInitValue( paramList.front(), hyp->GetLength( true )) ;
    SetInitValue( paramList.back(), hyp->GetLength( false )) ;
  }
  else if (hypType.compare("MaxElementArea") == 0)
  {
    StdMeshers::StdMeshers_MaxElementArea_var MEA =
      StdMeshers::StdMeshers_MaxElementArea::_narrow(theHyp);
    SetInitValue( paramList.front(), MEA->GetMaxElementArea() );
  }
  else if (hypType.compare("MaxElementVolume") == 0)
  {
    StdMeshers::StdMeshers_MaxElementVolume_var MEV =
      StdMeshers::StdMeshers_MaxElementVolume::_narrow(theHyp);
    SetInitValue( paramList.front(), MEV->GetMaxElementVolume() );
  }
  else if (hypType.compare("StartEndLength") == 0)
  {
    StdMeshers::StdMeshers_StartEndLength_var hyp =
      StdMeshers::StdMeshers_StartEndLength::_narrow(theHyp);
    SetInitValue( paramList.front(), hyp->GetLength( true ));
    SetInitValue( paramList.back(),  hyp->GetLength( false ));
  }
  else if (hypType.compare("Deflection1D") == 0)
  {
    StdMeshers::StdMeshers_Deflection1D_var hyp =
      StdMeshers::StdMeshers_Deflection1D::_narrow(theHyp);
    SetInitValue( paramList.back(),  hyp->GetDeflection()) ;
  }
  else if (hypType.compare("AutomaticLength") == 0)
  {
    StdMeshers::StdMeshers_AutomaticLength_var hyp =
      StdMeshers::StdMeshers_AutomaticLength::_narrow(theHyp);
    SetInitValue( paramList.back(),  hyp->GetFineness());
  }
}

//================================================================================
/*!
 * \brief Return parameter values as a string
  * \param hyp - not used
  * \param paramList - list of parameter values
  * \param params - output string
 */
//================================================================================

void StdMeshersGUI_Parameters::GetParameters (SMESH::SMESH_Hypothesis_ptr         ,
					      const list<SMESHGUI_aParameterPtr>& paramList,
					      QString&                            params)
{
  params = "";
  list<SMESHGUI_aParameterPtr>::const_iterator paramIt = paramList.begin();
  for ( ; paramIt != paramList.end(); paramIt++) {
    if (params.compare("")) params += " ; ";

    SMESHGUI_aParameter::Type t = (*paramIt)->GetType();
    if( t==SMESHGUI_aParameter::DOUBLE )
    {
      double aDoubleValue = 0.;
      (*paramIt)->GetNewDouble(aDoubleValue);
      params += QString::number(aDoubleValue);
    }
    else if( t==SMESHGUI_aParameter::STRING || t==SMESHGUI_aParameter::ENUM )
    {
      QString aStrValue( "" );
      (*paramIt)->GetNewText(aStrValue);
      params += aStrValue.simplifyWhiteSpace();
    }
    else if( t==SMESHGUI_aParameter::TABLE )
    {
      params += "TABLE";
    }
    else
    {
      int aIntValue = 0;
      (*paramIt)->GetNewInt(aIntValue);
      params += QString::number(aIntValue);
    }
  }
}

//=======================================================================
//function : SetParameters
//purpose  : 
//=======================================================================

//================================================================================
/*!
 * \brief Set parameter values from the list into a hypothesis
  * \param theHyp - The hypothesis to modify
  * \param paramList - list of parameter values
  * \retval bool - true if any parameter value changed
 */
//================================================================================

bool StdMeshersGUI_Parameters::SetParameters(SMESH::SMESH_Hypothesis_ptr          theHyp,
                                             const list<SMESHGUI_aParameterPtr> & paramList )
{
  if (theHyp->_is_nil() || paramList.empty()) return false;

  bool modified = false;

  QString hypType = theHyp->GetName();

  if (hypType.compare("LocalLength") == 0)
  {
    StdMeshers::StdMeshers_LocalLength_var LL =
      StdMeshers::StdMeshers_LocalLength::_narrow(theHyp);
    double length = LL->GetLength();
    modified = paramList.front()->GetNewDouble( length );
    LL->SetLength(length);
  }
  else if (hypType.compare("NumberOfSegments") == 0)
  {
    StdMeshers::StdMeshers_NumberOfSegments_var NOS =
      StdMeshers::StdMeshers_NumberOfSegments::_narrow(theHyp);

    list<SMESHGUI_aParameterPtr>::const_iterator anIt = paramList.begin();
    int NbSeg, DType;
    double Scale;
    SMESH::double_array TabF;
    QString exprF;
    int expType;

    modified = (*anIt)->GetNewInt( NbSeg ); anIt++;
    modified = (*anIt)->GetNewInt( DType ) || modified; anIt++;
    modified = (*anIt)->GetNewDouble( Scale ) || modified; anIt++;
    SMESHGUI_aParameterPtr p = *anIt;
    ((SMESHGUI_tableParameter*)p.get())->data( TabF ); anIt++; modified = true;
    modified = (*anIt)->GetNewText( exprF ) || modified; anIt++;
    modified = (*anIt)->GetNewInt( expType ) || modified;
    
    NOS->SetNumberOfSegments( NbSeg );
    NOS->SetDistrType( DType );
    if( DType==1 )
      NOS->SetScaleFactor( Scale );
    if( DType==2 )
      NOS->SetTableFunction( TabF );
    if( DType==3 )
      NOS->SetExpressionFunction( CORBA::string_dup( exprF.latin1() ) );
    if( DType==2 || DType==3 )
      NOS->SetExponentMode( (bool)expType );
  }
  else if (hypType.compare("Arithmetic1D") == 0)
  {
    if ( paramList.size() != 2 )
      return false;
    StdMeshers::StdMeshers_Arithmetic1D_var hyp =
      StdMeshers::StdMeshers_Arithmetic1D::_narrow(theHyp);
    double begLength = hyp->GetLength( true ) ;
    double endLength = hyp->GetLength( false ) ;
    modified = paramList.front()->GetNewDouble( begLength );
    modified = paramList.back()->GetNewDouble( endLength ) || modified;
    hyp->SetLength( begLength, true );
    hyp->SetLength( endLength, false );
  }
  else if (hypType.compare("MaxElementArea") == 0)
  {
    StdMeshers::StdMeshers_MaxElementArea_var MEA =
      StdMeshers::StdMeshers_MaxElementArea::_narrow(theHyp);
    double MaxArea = MEA->GetMaxElementArea();
    modified = paramList.front()->GetNewDouble( MaxArea );
    MEA->SetMaxElementArea(MaxArea);
  }
  else if (hypType.compare("MaxElementVolume") == 0)
  {
    StdMeshers::StdMeshers_MaxElementVolume_var MEV =
      StdMeshers::StdMeshers_MaxElementVolume::_narrow(theHyp);
    double MaxVolume = MEV->GetMaxElementVolume() ;
    modified = paramList.front()->GetNewDouble( MaxVolume );
    MEV->SetMaxElementVolume(MaxVolume);
  }
  else if (hypType.compare("StartEndLength") == 0)
  {
    if ( paramList.size() != 2 )
      return false;
    StdMeshers::StdMeshers_StartEndLength_var hyp =
      StdMeshers::StdMeshers_StartEndLength::_narrow(theHyp);
    double begLength = hyp->GetLength( true ) ;
    double endLength = hyp->GetLength( false ) ;
    modified = paramList.front()->GetNewDouble( begLength );
    modified = paramList.back()->GetNewDouble( endLength ) || modified;
    hyp->SetLength( begLength, true );
    hyp->SetLength( endLength, false );
  }
  else if (hypType.compare("Deflection1D") == 0)
  {
    StdMeshers::StdMeshers_Deflection1D_var hyp =
      StdMeshers::StdMeshers_Deflection1D::_narrow(theHyp);
    double value = hyp->GetDeflection() ;
    modified = paramList.front()->GetNewDouble( value );
    hyp->SetDeflection( value );
  }
  else if (hypType.compare("AutomaticLength") == 0)
  {
    StdMeshers::StdMeshers_AutomaticLength_var hyp =
      StdMeshers::StdMeshers_AutomaticLength::_narrow(theHyp);
    double value = hyp->GetFineness() ;
    modified = paramList.front()->GetNewDouble( value );
    hyp->SetFineness( value );
  }
  return modified ;
}

//================================================================================
/*!
 * \brief Widget: slider with left and right labels
 */
//================================================================================

class StdMeshersGUI_SliderWith2Lables: public QHBox
{
public:
  StdMeshersGUI_SliderWith2Lables( const QString& leftLabel,
                                   const QString& rightLabel,
                                   QWidget * parent =0,
                                   const char * name=0 );
  QSlider * getSlider() const { return _slider; }
private:
  QSlider * _slider;
};

StdMeshersGUI_SliderWith2Lables::StdMeshersGUI_SliderWith2Lables( const QString& leftLabel,
                                                                  const QString& rightLabel,
                                                                  QWidget *      parent,
                                                                  const char *   name )
  :QHBox(parent,name)
{
  if ( !leftLabel.isEmpty() )
    (new QLabel( this ))->setText( leftLabel );

  _slider = new QSlider( Horizontal, this );

  if ( !rightLabel.isEmpty() )
    (new QLabel( this ))->setText( rightLabel );
}

//================================================================================
/*!
 * \brief Constructor
  * \param label - main label
  * \param leftLabel - label to the left of slider
  * \param rightLabel - label to the right of slider
  * \param initValue - initial slider value
  * \param bottom - least slider value
  * \param top - maximal slider value
  * \param precision - slider value precision
 */
//================================================================================

StdMeshersGUI_doubleSliderParameter::
StdMeshersGUI_doubleSliderParameter (const QString& label,
                                     const QString& leftLabel,
                                     const QString& rightLabel,
                                     const double   initValue,
                                     const double   bottom,
                                     const double   top   ,
                                     const double   precision)
  :SMESHGUI_doubleParameter(initValue,label,bottom,top,precision),
   _leftLabel(leftLabel), _rightLabel(rightLabel)
{
}

QWidget* StdMeshersGUI_doubleSliderParameter::CreateWidget( QWidget* parent ) const
{
  return new StdMeshersGUI_SliderWith2Lables( _leftLabel, _rightLabel, parent );
}

void StdMeshersGUI_doubleSliderParameter::InitializeWidget( QWidget* aWidget) const
{
  StdMeshersGUI_SliderWith2Lables * paramWidget =
    dynamic_cast<StdMeshersGUI_SliderWith2Lables*> (aWidget);
  if ( paramWidget && paramWidget->getSlider() )
  {
    QSlider * slider = paramWidget->getSlider();
    slider->setRange( 0, toInt( _top ));
    slider->setValue( toInt( _initValue ));
  }
}

void StdMeshersGUI_doubleSliderParameter::TakeValue( QWidget* aWidget)
{
  StdMeshersGUI_SliderWith2Lables * paramWidget =
    dynamic_cast<StdMeshersGUI_SliderWith2Lables*> (aWidget);
  if ( paramWidget && paramWidget->getSlider() )
  {
    int val = paramWidget->getSlider()->value();
    _newValue = Bottom() + val * Step();
  }
}
int StdMeshersGUI_doubleSliderParameter::toInt( double val ) const
{
  return (int) ceil(( val - _bottom ) / _step );
}
