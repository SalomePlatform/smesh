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

using namespace std;

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
//function : GetParameters
//purpose  : 
//=======================================================================

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
                                       0.001, 999.999, 1.0, 3));
  }
  else if (hypType.compare("NumberOfSegments") == 0)
  {
    paramList.push_back ( INT_PARAM (3,
                                     QObject::tr("SMESH_NB_SEGMENTS_PARAM"),
                                     1, 9999 ));
  }
  else if (hypType.compare("Arithmetic1D") == 0)
  {
    paramList.push_back( DOUBLE_PARAM ( 1.0,
                                      QObject::tr("SMESH_START_LENGTH_PARAM"), 
                                      1.0E-5, 1E6, 1, 6));
    paramList.push_back( DOUBLE_PARAM ( 10.0,
                                       QObject::tr("SMESH_END_LENGTH_PARAM"),
                                       1.0E-5, 1E6, 1, 6));
  }
  else if (hypType.compare("MaxElementArea") == 0)
  {
    paramList.push_back( DOUBLE_PARAM (1.0,
                                       QObject::tr("SMESH_MAX_ELEMENT_AREA_PARAM"), 
                                       0.001, 999999.999, 1.0, 3));
  }
  else if (hypType.compare("MaxElementVolume") == 0)
  {
    paramList.push_back( DOUBLE_PARAM ( 1.0,
                                       QObject::tr("SMESH_MAX_ELEMENT_VOLUME_PARAM"), 
                                       0.001, 999999.999, 1.0, 3));
  }
  else if (hypType.compare("StartEndLength") == 0)
  {
    paramList.push_back( DOUBLE_PARAM ( 1.0,
                                      QObject::tr("SMESH_START_LENGTH_PARAM"), 
                                      1.0E-5, 1E6, 1, 6));
    paramList.push_back( DOUBLE_PARAM ( 10.0,
                                       QObject::tr("SMESH_END_LENGTH_PARAM"),
                                       1.0E-5, 1E6, 1, 6));
  }
  else if (hypType.compare("Deflection1D") == 0)
  {
    paramList.push_back( DOUBLE_PARAM ( 1.0,
                                       QObject::tr("SMESH_DEFLECTION1D_PARAM"), 
                                       1.0E-5, 1E6, 1, 6));
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

  if (hypType.compare("LocalLength") == 0)
  {
    StdMeshers::StdMeshers_LocalLength_var LL =
      StdMeshers::StdMeshers_LocalLength::_narrow(theHyp);
    double length = LL->GetLength();
    paramList.push_back( DOUBLE_PARAM (length,
                                       QObject::tr("SMESH_LOCAL_LENGTH_PARAM"),
                                       0.001, 999.999, 1.0, 3));
  }
  else if (hypType.compare("NumberOfSegments") == 0)
  {
    StdMeshers::StdMeshers_NumberOfSegments_var NOS =
      StdMeshers::StdMeshers_NumberOfSegments::_narrow(theHyp);
    int NbSeg = NOS->GetNumberOfSegments() ;
    paramList.push_back ( INT_PARAM (NbSeg,
                                     QObject::tr("SMESH_NB_SEGMENTS_PARAM"),
                                     1, 9999 ));
  }
  else if (hypType.compare("Arithmetic1D") == 0)
  {
    StdMeshers::StdMeshers_Arithmetic1D_var hyp =
      StdMeshers::StdMeshers_Arithmetic1D::_narrow(theHyp);
    double begLength = hyp->GetLength( true ) ;
    double endLength = hyp->GetLength( false ) ;
    paramList.push_back( DOUBLE_PARAM ( begLength,
                                      QObject::tr("SMESH_START_LENGTH_PARAM"), 
                                      1.0E-5, 1E6, 1, 6));
    paramList.push_back( DOUBLE_PARAM ( endLength,
                                      QObject::tr("SMESH_END_LENGTH_PARAM"),
                                      1.0E-5, 1E6, 1, 6));
  }
  else if (hypType.compare("MaxElementArea") == 0)
  {
    StdMeshers::StdMeshers_MaxElementArea_var MEA =
      StdMeshers::StdMeshers_MaxElementArea::_narrow(theHyp);
    double MaxArea = MEA->GetMaxElementArea();
    paramList.push_back( DOUBLE_PARAM (MaxArea,
                                       QObject::tr("SMESH_MAX_ELEMENT_AREA_PARAM"), 
                                       0.001, 999999.999, 1.0, 3));
  }
  else if (hypType.compare("MaxElementVolume") == 0)
  {
    StdMeshers::StdMeshers_MaxElementVolume_var MEV =
      StdMeshers::StdMeshers_MaxElementVolume::_narrow(theHyp);
    double MaxVolume = MEV->GetMaxElementVolume() ;
    paramList.push_back( DOUBLE_PARAM ( MaxVolume,
                                       QObject::tr("SMESH_MAX_ELEMENT_VOLUME_PARAM"), 
                                       0.001, 999999.999, 1.0, 3));
  }
  else if (hypType.compare("StartEndLength") == 0)
  {
    StdMeshers::StdMeshers_StartEndLength_var hyp =
      StdMeshers::StdMeshers_StartEndLength::_narrow(theHyp);
    double begLength = hyp->GetLength( true ) ;
    double endLength = hyp->GetLength( false ) ;
    paramList.push_back( DOUBLE_PARAM ( begLength,
                                      QObject::tr("SMESH_START_LENGTH_PARAM"), 
                                      1.0E-5, 1E6, 1, 6));
    paramList.push_back( DOUBLE_PARAM ( endLength,
                                      QObject::tr("SMESH_END_LENGTH_PARAM"),
                                      1.0E-5, 1E6, 1, 6));
  }
  else if (hypType.compare("Deflection1D") == 0)
  {
    StdMeshers::StdMeshers_Deflection1D_var hyp =
      StdMeshers::StdMeshers_Deflection1D::_narrow(theHyp);
    double value = hyp->GetDeflection() ;
    paramList.push_back( DOUBLE_PARAM ( value,
                                      QObject::tr("SMESH_DEFLECTION1D_PARAM"), 
                                      1.0E-5, 1E6, 1, 6));
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
    int aIntValue;
    double aDoubleValue;
    if (params.compare("")) params += " ; ";
    if ((*paramIt)->GetType() == SMESHGUI_aParameter::INT) {
      (*paramIt)->GetNewInt(aIntValue);
      params += QString::number(aIntValue);;
    }
    else {
      (*paramIt)->GetNewDouble(aDoubleValue);
	params += QString::number(aDoubleValue);
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
    modified = paramList.front()->GetNewInt( NbSeg );
    NOS->SetNumberOfSegments(NbSeg);
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
  
