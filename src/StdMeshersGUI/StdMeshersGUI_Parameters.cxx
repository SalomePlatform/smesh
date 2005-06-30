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

//#include "SMESHGUI_SpinBox.h" // for the sake of COORD_MAX, COORD_MIN definition

#include <qobject.h>

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
  if ( p ) p->InitValue() = initValue;
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
//function : GetParameters
//purpose  : 
//=======================================================================

// SMESHGUI_doubleParameter( initValue, label, bottom, top, step, decimals )
#define DOUBLE_PARAM(v,l,b,t,s,d) SMESHGUI_aParameterPtr(new SMESHGUI_doubleParameter(v,l,b,t,s,d))
#define INT_PARAM(v,l,b,t) SMESHGUI_aParameterPtr(new SMESHGUI_intParameter(v,l,b,t))

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
    paramList.push_back ( INT_PARAM (3,
                                     QObject::tr("SMESH_NB_SEGMENTS_PARAM"),
                                     1, 9999 ));
    paramList.push_back ( DOUBLE_PARAM (1.0,
                                     QObject::tr("SMESH_NB_SEGMENTS_SCALE_PARAM"),
                                     VALUE_SMALL, VALUE_MAX, 0.1, 6 ));
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
}
  
//=======================================================================
//function : GetParameters
//purpose  : 
//=======================================================================

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
    SetInitValue( paramList.front(), (int) NOS->GetNumberOfSegments());
    SetInitValue( paramList.back(), NOS->GetScaleFactor());
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
}

//=======================================================================
//function : GetParameters
//purpose  : 
//=======================================================================  
void StdMeshersGUI_Parameters::GetParameters (SMESH::SMESH_Hypothesis_ptr         hyp,
					      list<SMESHGUI_aParameterPtr> &      paramList,
					      QString&                            params)
{
  params = "";
  list<SMESHGUI_aParameterPtr>::iterator paramIt = paramList.begin();
  for ( ; paramIt != paramList.end(); paramIt++) {
    if (params.compare("")) params += " ; ";

    if ((*paramIt)->GetType() == SMESHGUI_aParameter::DOUBLE ) {
      double aDoubleValue = 0.;
      (*paramIt)->GetNewDouble(aDoubleValue);
      params += QString::number(aDoubleValue);
    }
    else if ((*paramIt)->GetType() == SMESHGUI_aParameter::TEXT ) {
      QString aStrValue( "" );
      (*paramIt)->GetNewText(aStrValue);
      params += aStrValue.simplifyWhiteSpace();
    }
    else {
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
    int NbSeg = NOS->GetNumberOfSegments();
    double Scale = NOS->GetScaleFactor() ;
    modified = paramList.front()->GetNewInt( NbSeg );
    modified = paramList.back()->GetNewDouble( Scale ) || modified;
    NOS->SetNumberOfSegments(NbSeg);
    NOS->SetScaleFactor( Scale );
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
  return modified ;
}
  
