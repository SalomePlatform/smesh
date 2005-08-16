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
#include <Standard_ErrorHandler.hxx>
#include <TCollection_AsciiString.hxx>
#include <ExprIntrp_GenExp.hxx>
#include <Expr_NamedUnknown.hxx>

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
    _expMode(false)
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

  for (i=0; i < table.size()/2; i++) {
    double par = table[i*2];
    double val = table[i*2+1];
    if ( par<0 || par > 1)
      throw SALOME_Exception(LOCALIZED("parameter of table function is out of range [0,1]"));
    if ( fabs(par-prev)<PRECISION )
      throw SALOME_Exception(LOCALIZED("two parameters are the same"));
    if (val < PRECISION)
      throw SALOME_Exception(LOCALIZED("value of table function is not positive"));
    if (isSame)
    {
      double oldpar = _table[i*2];
      double oldval = _table[i*2+1];
      if (fabs(par - oldpar) > PRECISION || fabs(val - oldval) > PRECISION)
        isSame = false;
    }
    prev = par;
  }

  if (!isSame)
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
bool isCorrect( const Handle( Expr_GeneralExpression )& expr )
{
  if( expr.IsNull() )
    return true;
    
  bool res = true;
  for( int i=1, n=expr->NbSubExpressions(); i<=n && res; i++ )
  {
    Handle( Expr_GeneralExpression ) subexpr = expr->SubExpression( i );
    Handle( Expr_NamedUnknown ) name = Handle( Expr_NamedUnknown )::DownCast( subexpr );
    if( !name.IsNull() )
    {
      if( name->GetName()!="t" )
        res = false;
    }
    else
      res = isCorrect( subexpr );
  }
  return res;
}

//================================================================================
/*! this function parses the expression 'str' in order to check if syntax is correct
 *  ( result in 'syntax' ) and if only 't' is unknown variable in expression ( result in 'args' )
 */
//================================================================================
void casProcess( const TCollection_AsciiString& str, bool& syntax, bool& args )
{
  // check validity of expression
  syntax = false;
  args = false;
  try
  {
    Handle( ExprIntrp_GenExp ) gen = ExprIntrp_GenExp::Create();
    gen->Process( str );
        
    if( gen->IsDone() ) 
    {
      syntax = true;
      args = isCorrect( gen->Expression() );
    }
  }
  catch (Standard_Failure)
  {
  }
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

  bool syntax, args;
  casProcess( str, syntax, args );
  if( !syntax )
    throw SALOME_Exception(LOCALIZED("invalid expression syntax"));
  if( !args )
    throw SALOME_Exception(LOCALIZED("only 't' may be used as function argument"));

  string func(str.ToCString());
  if (_func != func)
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

void StdMeshers_NumberOfSegments::SetExponentMode(bool isExp)
  throw(SALOME_Exception)
{
  if (_distrType != DT_TabFunc && _distrType != DT_ExprFunc)
    throw SALOME_Exception(LOCALIZED("not a functional distribution"));

  if (isExp != _expMode)
  {
    _expMode = isExp;
    NotifySubMeshesHypothesisModification();
  }
}

//================================================================================
/*!
 * 
 */
//================================================================================

bool StdMeshers_NumberOfSegments::IsExponentMode() const
  throw(SALOME_Exception)
{
  if (_distrType != DT_TabFunc && _distrType != DT_ExprFunc)
    throw SALOME_Exception(LOCALIZED("not a functional distribution"));
  return _expMode;
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
    save << " " << (int)_expMode;
  
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

  // read ditribution type
  isOK = (load >> a);
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
        load.clear(ios::badbit | load.rdstate());
    }
    break;
  case DT_TabFunc:
    {
      isOK = (load >> a);
      if (isOK)
        _table.resize(a, 0.);
      else
        load.clear(ios::badbit | load.rdstate());
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
    break;
  case DT_ExprFunc:
    {
      string str;
      isOK = (load >> str);
      if (isOK)
        _func = str;
      else
        load.clear(ios::badbit | load.rdstate());
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
      _expMode = (bool) a;
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
