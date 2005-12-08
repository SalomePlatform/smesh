//  SMESH SMESH : implementaion of SMESH idl descriptions
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
//  File   : StdMeshers_NumberOfSegments.cxx
//           Moved here from SMESH_NumberOfSegments.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include "StdMeshers_NumberOfSegments.hxx"
#include "StdMeshers_Distribution.hxx"
#include <Standard_ErrorHandler.hxx>
#include <TCollection_AsciiString.hxx>
#include <ExprIntrp_GenExp.hxx>
#include <Expr_NamedUnknown.hxx>
#include <CASCatch_CatchSignals.hxx>
#include <CASCatch_Failure.hxx> 
#include <CASCatch_ErrorHandler.hxx>
#include <OSD.hxx>
#include <Expr_Array1OfNamedUnknown.hxx>
#include <TColStd_Array1OfReal.hxx>


const double PRECISION = 1e-7;

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_NumberOfSegments::StdMeshers_NumberOfSegments(int hypId, int studyId,
	SMESH_Gen * gen)
  : SMESH_Hypothesis(hypId, studyId, gen),
    _numberOfSegments(1),
    _distrType(DT_Regular),
    _scaleFactor(1.),
    _convMode(1)  //cut negative by default
{
  _name = "NumberOfSegments";
  _param_algo_dim = 1;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_NumberOfSegments::~StdMeshers_NumberOfSegments()
{
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
const std::vector<double>& StdMeshers_NumberOfSegments::BuildDistributionExpr( const char* expr, int nbSeg, int conv )
throw ( SALOME_Exception )
{
  if( !buildDistribution( TCollection_AsciiString( ( Standard_CString )expr ), conv, 0.0, 1.0, nbSeg, _distr, 1E-4 ) )
    _distr.resize( 0 );
  return _distr;
}

const std::vector<double>& StdMeshers_NumberOfSegments::BuildDistributionTab( const std::vector<double>& tab,
									      int nbSeg, int conv )
throw ( SALOME_Exception )
{
  if( !buildDistribution( tab, conv, 0.0, 1.0, nbSeg, _distr, 1E-4 ) )
    _distr.resize( 0 );
  return _distr;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void StdMeshers_NumberOfSegments::SetNumberOfSegments(int segmentsNumber)
throw(SALOME_Exception)
{
	int oldNumberOfSegments = _numberOfSegments;
	if (segmentsNumber <= 0)
		throw
			SALOME_Exception(LOCALIZED("number of segments must be positive"));
	_numberOfSegments = segmentsNumber;

	if (oldNumberOfSegments != _numberOfSegments)
		NotifySubMeshesHypothesisModification();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

int StdMeshers_NumberOfSegments::GetNumberOfSegments() const
{
	return _numberOfSegments;
}

//================================================================================
/*!
 * 
 */
//================================================================================

void StdMeshers_NumberOfSegments::SetDistrType(DistrType typ)
  throw(SALOME_Exception)
{
  if (typ < DT_Regular || typ > DT_ExprFunc)
    throw SALOME_Exception(LOCALIZED("distribution type is out of range"));

  if (typ != _distrType)
  {
    _distrType = typ;
    NotifySubMeshesHypothesisModification();
  }
}

//================================================================================
/*!
 * 
 */
//================================================================================

StdMeshers_NumberOfSegments::DistrType StdMeshers_NumberOfSegments::GetDistrType() const
{
  return _distrType;
}

//================================================================================
/*!
 * 
 */
//================================================================================

void StdMeshers_NumberOfSegments::SetScaleFactor(double scaleFactor)
  throw(SALOME_Exception)
{
  if (_distrType != DT_Scale)
    throw SALOME_Exception(LOCALIZED("not a scale distribution"));
  if (scaleFactor < PRECISION)
    throw SALOME_Exception(LOCALIZED("scale factor must be positive"));
  if (fabs(scaleFactor - 1.0) < PRECISION)
    throw SALOME_Exception(LOCALIZED("scale factor must not be equal to 1"));

  if (fabs(_scaleFactor - scaleFactor) > PRECISION)
  {
    _scaleFactor = scaleFactor;
    NotifySubMeshesHypothesisModification();
  }
}

//================================================================================
/*!
 * 
 */
//================================================================================

double StdMeshers_NumberOfSegments::GetScaleFactor() const
  throw(SALOME_Exception)
{
  if (_distrType != DT_Scale)
    throw SALOME_Exception(LOCALIZED("not a scale distribution"));
  return _scaleFactor;
}

//================================================================================
/*!
 * 
 */
//================================================================================

void StdMeshers_NumberOfSegments::SetTableFunction(const std::vector<double>& table)
  throw(SALOME_Exception)
{
  if (_distrType != DT_TabFunc)
    throw SALOME_Exception(LOCALIZED("not a table function distribution"));
  if ( (table.size() % 2) != 0 )
    throw SALOME_Exception(LOCALIZED("odd size of vector of table function"));

  int i;
  double prev = -PRECISION;
  bool isSame = table.size() == _table.size();

  OSD::SetSignal( true );
  CASCatch_CatchSignals aCatchSignals;
  aCatchSignals.Activate();

  bool pos = false;
  for (i=0; i < table.size()/2; i++) {
    double par = table[i*2];
    double val = table[i*2+1];
    if( _convMode==0 )
    {
      CASCatch_TRY
      {
	val = pow( 10.0, val );
      }
      CASCatch_CATCH(CASCatch_Failure)
      {
	aCatchSignals.Deactivate();
	Handle(CASCatch_Failure) aFail = CASCatch_Failure::Caught();
	throw SALOME_Exception( LOCALIZED( "invalid value"));
	return;
      }
    }
    else if( _convMode==1 && val<0.0 )
      val = 0.0;

    if ( par<0 || par > 1)
      throw SALOME_Exception(LOCALIZED("parameter of table function is out of range [0,1]"));
    if ( fabs(par-prev)<PRECISION )
      throw SALOME_Exception(LOCALIZED("two parameters are the same"));
    if ( val < 0 )
      throw SALOME_Exception(LOCALIZED("value of table function is not positive"));
    if( val>PRECISION )
      pos = true;
    if (isSame)
    {
      double oldpar = _table[i*2];
      double oldval = _table[i*2+1];
      if (fabs(par - oldpar) > PRECISION || fabs(val - oldval) > PRECISION)
        isSame = false;
    }
    prev = par;
  }
  aCatchSignals.Deactivate();

  if( !pos )
    throw SALOME_Exception(LOCALIZED("value of table function is not positive"));

  if( pos && !isSame )
  {
    _table = table;
    NotifySubMeshesHypothesisModification();
  }
}

//================================================================================
/*!
 * 
 */
//================================================================================

const std::vector<double>& StdMeshers_NumberOfSegments::GetTableFunction() const
  throw(SALOME_Exception)
{
  if (_distrType != DT_TabFunc)
    throw SALOME_Exception(LOCALIZED("not a table function distribution"));
  return _table;
}

//================================================================================
/*! check if only 't' is unknown variable in expression
 */
//================================================================================
bool isCorrectArg( const Handle( Expr_GeneralExpression )& expr )
{
  Handle( Expr_NamedUnknown ) sub = Handle( Expr_NamedUnknown )::DownCast( expr );
  if( !sub.IsNull() )
    return sub->GetName()=="t";

  bool res = true;
  for( int i=1, n=expr->NbSubExpressions(); i<=n && res; i++ )
  {
    Handle( Expr_GeneralExpression ) sub = expr->SubExpression( i );
    Handle( Expr_NamedUnknown ) name = Handle( Expr_NamedUnknown )::DownCast( sub );
    if( !name.IsNull() )
    {
      if( name->GetName()!="t" )
	res = false;
    }
    else
      res = isCorrectArg( sub );
  }
  return res;
}

//================================================================================
/*! this function parses the expression 'str' in order to check if syntax is correct
 *  ( result in 'syntax' ) and if only 't' is unknown variable in expression ( result in 'args' )
 */
//================================================================================
bool process( const TCollection_AsciiString& str, int convMode,
	      bool& syntax, bool& args,
	      bool& non_neg, bool& non_zero,
 	      bool& singulars, double& sing_point )
{
  OSD::SetSignal( true );
  CASCatch_CatchSignals aCatchSignals;
  aCatchSignals.Activate();

  bool parsed_ok = true;
  Handle( ExprIntrp_GenExp ) myExpr;
  CASCatch_TRY
  {
    myExpr = ExprIntrp_GenExp::Create();
    myExpr->Process( str.ToCString() );
  }
  CASCatch_CATCH(CASCatch_Failure)
  {
    aCatchSignals.Deactivate();
    Handle(CASCatch_Failure) aFail = CASCatch_Failure::Caught();
    parsed_ok = false;
  }
  aCatchSignals.Deactivate();

  syntax = false;
  args = false;
  if( parsed_ok && myExpr->IsDone() )
  {
    syntax = true;
    args = isCorrectArg( myExpr->Expression() );
  }

  bool res = parsed_ok && syntax && args;
  if( !res )
    myExpr.Nullify();

  non_neg = true;
  singulars = false;
  non_zero = false;

  if( res )
  {
    FunctionExpr f( str.ToCString(), convMode );
    const int max = 500;
    for( int i=0; i<=max; i++ )
    {
      double t = double(i)/double(max), val;
      if( !f.value( t, val ) )
      {
	sing_point = t;
	singulars = true;
	break;
      }
      if( val<0 )
      {
	non_neg = false;
	break;
      }
      if( val>PRECISION )
	non_zero = true;
    }
  }
  return res && non_neg && non_zero && ( !singulars );
}

//================================================================================
/*!
 * 
 */
//================================================================================

void StdMeshers_NumberOfSegments::SetExpressionFunction(const char* expr)
  throw(SALOME_Exception)
{
  if (_distrType != DT_ExprFunc)
    throw SALOME_Exception(LOCALIZED("not an expression function distribution"));

  // remove white spaces
  TCollection_AsciiString str((Standard_CString)expr);
  str.RemoveAll(' ');
  str.RemoveAll('\t');
  str.RemoveAll('\r');
  str.RemoveAll('\n');

  bool syntax, args, non_neg, singulars, non_zero;
  double sing_point;
  bool res = process( str, _convMode, syntax, args, non_neg, non_zero, singulars, sing_point );
  if( !res )
  {
    if( !syntax )
      throw SALOME_Exception(LOCALIZED("invalid expression syntax"));
    if( !args )
      throw SALOME_Exception(LOCALIZED("only 't' may be used as function argument"));
    if( !non_neg )
      throw SALOME_Exception(LOCALIZED("only non-negative function can be used as density"));
    if( singulars )
    {
      char buf[1024];
      sprintf( buf, "Function has singular point in %.3f", sing_point );
      throw SALOME_Exception( buf );
    }
    if( !non_zero )
      throw SALOME_Exception(LOCALIZED("f(t)=0 cannot be used as density"));

    return;
  }
  
  std::string func = expr;
  if( _func != func )
  {
    _func = func;
    NotifySubMeshesHypothesisModification();
  }
}

//================================================================================
/*!
 * 
 */
//================================================================================

const char* StdMeshers_NumberOfSegments::GetExpressionFunction() const
  throw(SALOME_Exception)
{
  if (_distrType != DT_ExprFunc)
    throw SALOME_Exception(LOCALIZED("not an expression function distribution"));
  return _func.c_str();
}

//================================================================================
/*!
 * 
 */
//================================================================================

void StdMeshers_NumberOfSegments::SetConversionMode( int conv )
  throw(SALOME_Exception)
{
  if (_distrType != DT_TabFunc && _distrType != DT_ExprFunc)
    throw SALOME_Exception(LOCALIZED("not a functional distribution"));

  if( conv != _convMode )
  {
    _convMode = conv;
    NotifySubMeshesHypothesisModification();
  }
}

//================================================================================
/*!
 * 
 */
//================================================================================

int StdMeshers_NumberOfSegments::ConversionMode() const
  throw(SALOME_Exception)
{
  if (_distrType != DT_TabFunc && _distrType != DT_ExprFunc)
    throw SALOME_Exception(LOCALIZED("not a functional distribution"));
  return _convMode;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & StdMeshers_NumberOfSegments::SaveTo(ostream & save)
{
  save << _numberOfSegments << " " << (int)_distrType;
  switch (_distrType)
  {
  case DT_Scale:
    save << " " << _scaleFactor;
    break;
  case DT_TabFunc:
    int i;
    save << " " << _table.size();
    for (i=0; i < _table.size(); i++)
      save << " " << _table[i];
    break;
  case DT_ExprFunc:
    save << " " << _func;
    break;
  case DT_Regular:
  default:
    break;
  }

  if (_distrType == DT_TabFunc || _distrType == DT_ExprFunc)
    save << " " << _convMode;
  
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & StdMeshers_NumberOfSegments::LoadFrom(istream & load)
{
  bool isOK = true;
  int a;

  // read number of segments
  isOK = (load >> a);
  if (isOK)
    _numberOfSegments = a;
  else
    load.clear(ios::badbit | load.rdstate());

  // read second stored value. It can be two variants here:
  // 1. If the hypothesis is stored in old format (nb.segments and scale factor),
  //    we wait here the scale factor, which is double.
  // 2. If the hypothesis is stored in new format
  //    (nb.segments, distr.type, some other params.),
  //    we wait here the ditribution type, which is integer
  double scale_factor;
  isOK = (load >> scale_factor);
  a = (int)scale_factor;

  // try to interprete ditribution type,
  // supposing that this hypothesis was written in the new format
  if (isOK)
  {
    if (a < DT_Regular || a > DT_ExprFunc)
      _distrType = DT_Regular;
    else
      _distrType = (DistrType) a;
  }
  else
    load.clear(ios::badbit | load.rdstate());

  // parameters of distribution
  double b;
  switch (_distrType)
  {
  case DT_Scale:
    {
      isOK = (load >> b);
      if (isOK)
        _scaleFactor = b;
      else
      {
        load.clear(ios::badbit | load.rdstate());
        // this can mean, that the hypothesis is stored in old format
        _distrType = DT_Regular;
        _scaleFactor = scale_factor;
      }
    }
    break;
  case DT_TabFunc:
    {
      isOK = (load >> a);
      if (isOK)
      {
        _table.resize(a, 0.);
        int i;
        for (i=0; i < _table.size(); i++)
        {
          isOK = (load >> b);
          if (isOK)
            _table[i] = b;
          else
            load.clear(ios::badbit | load.rdstate());
        }
      }
      else
      {
        load.clear(ios::badbit | load.rdstate());
        // this can mean, that the hypothesis is stored in old format
        _distrType = DT_Regular;
        _scaleFactor = scale_factor;
      }
    }
    break;
  case DT_ExprFunc:
    {
      string str;
      isOK = (load >> str);
      if (isOK)
        _func = str;
      else
      {
        load.clear(ios::badbit | load.rdstate());
        // this can mean, that the hypothesis is stored in old format
        _distrType = DT_Regular;
        _scaleFactor = scale_factor;
      }
    }
    break;
  case DT_Regular:
  default:
    break;
  }

  if (_distrType == DT_TabFunc || _distrType == DT_ExprFunc)
  {
    isOK = (load >> a);
    if (isOK)
      _convMode = a;
    else
      load.clear(ios::badbit | load.rdstate());
  }

  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator <<(ostream & save, StdMeshers_NumberOfSegments & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >>(istream & load, StdMeshers_NumberOfSegments & hyp)
{
  return hyp.LoadFrom( load );
}
