//  SMESH StdMeshersGUI : GUI for plugged-in meshers
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
//  File   : StdMeshersGUI_StdHypothesisCreator.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header: /home/server/cvs/SMESH/SMESH_SRC/src/StdMeshersGUI/StdMeshersGUI_StdHypothesisCreator.cxx

#include "StdMeshersGUI_StdHypothesisCreator.h"

#include <SMESHGUI.h>
#include <SMESHGUI_SpinBox.h>
#include <SMESHGUI_HypothesesUtils.h>
#include <SMESHGUI_Utils.h>

#include <SUIT_ResourceMgr.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_BasicHypothesis)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#include <qpixmap.h>


const double VALUE_MAX = 1.0e+15, // COORD_MAX
             VALUE_MAX_2  = VALUE_MAX * VALUE_MAX,
             VALUE_MAX_3  = VALUE_MAX_2 * VALUE_MAX,
             VALUE_SMALL = 1.0e-15,
             VALUE_SMALL_2 = VALUE_SMALL * VALUE_SMALL,
             VALUE_SMALL_3 = VALUE_SMALL_2 * VALUE_SMALL;

StdMeshersGUI_StdHypothesisCreator::StdMeshersGUI_StdHypothesisCreator( const QString& type )
: SMESHGUI_GenericHypothesisCreator( type )
{
}

StdMeshersGUI_StdHypothesisCreator::~StdMeshersGUI_StdHypothesisCreator()
{
}

QFrame* StdMeshersGUI_StdHypothesisCreator::buildFrame()
{
  return buildStdFrame();
}

bool StdMeshersGUI_StdHypothesisCreator::checkParams() const
{
  return true;
}

void StdMeshersGUI_StdHypothesisCreator::retrieveParams() const
{
  //here this method must be empty because buildStdParam sets values itself
}

void StdMeshersGUI_StdHypothesisCreator::storeParams() const
{
  ListOfStdParams params;
  bool res = getStdParamFromDlg( params );
  if( isCreation() )
  {
    SMESH::SetName( SMESH::FindSObject( hypothesis() ), params[0].myValue.toString().latin1() );
    params.remove( params.begin() );
  }

  if( res && !params.isEmpty() )
  {
    if( hypType()=="LocalLength" )
    {
      StdMeshers::StdMeshers_LocalLength_var h =
	StdMeshers::StdMeshers_LocalLength::_narrow( hypothesis() );

      h->SetLength( params[0].myValue.toDouble() );
    }
    else if( hypType()=="Arithmetic1D" )
    {
      StdMeshers::StdMeshers_Arithmetic1D_var h =
	StdMeshers::StdMeshers_Arithmetic1D::_narrow( hypothesis() );

      h->SetLength( params[0].myValue.toDouble(), true );
      h->SetLength( params[1].myValue.toDouble(), false );
    }
    else if( hypType()=="MaxElementArea" )
    {
      StdMeshers::StdMeshers_MaxElementArea_var h =
	StdMeshers::StdMeshers_MaxElementArea::_narrow( hypothesis() );

      h->SetMaxElementArea( params[0].myValue.toDouble() );
    }
    else if( hypType()=="MaxElementVolume" )
    {
      StdMeshers::StdMeshers_MaxElementVolume_var h =
	StdMeshers::StdMeshers_MaxElementVolume::_narrow( hypothesis() );

      h->SetMaxElementVolume( params[0].myValue.toDouble() );
    }
    else if( hypType()=="StartEndLength" )
    {
      StdMeshers::StdMeshers_StartEndLength_var h =
	StdMeshers::StdMeshers_StartEndLength::_narrow( hypothesis() );

      h->SetLength( params[0].myValue.toDouble(), true );
      h->SetLength( params[1].myValue.toDouble(), false );
    }
    else if( hypType()=="Deflection1D" )
    {
      StdMeshers::StdMeshers_Deflection1D_var h =
	StdMeshers::StdMeshers_Deflection1D::_narrow( hypothesis() );

      h->SetDeflection( params[0].myValue.toDouble() );
    }
  }
}

bool StdMeshersGUI_StdHypothesisCreator::stdParams( ListOfStdParams& p ) const
{
  bool res = true;
  SMESHGUI_GenericHypothesisCreator::StdParam item;

  p.clear();
  if( isCreation() )
  {
    HypothesisData* data = SMESH::GetHypothesisData( hypType() );
    item.myName = tr( "SMESH_NAME" );
    item.myValue = data ? data->Label : QString();
    p.append( item );
  }

  if( hypType()=="LocalLength" )
  {
    StdMeshers::StdMeshers_LocalLength_var h =
      StdMeshers::StdMeshers_LocalLength::_narrow( hypothesis() );

    item.myName = tr("SMESH_LOCAL_LENGTH_PARAM");
    item.myValue = isCreation() ? 1.0 : h->GetLength();
    p.append( item );
  }
  else if( hypType()=="Arithmetic1D" )
  {
    StdMeshers::StdMeshers_Arithmetic1D_var h =
      StdMeshers::StdMeshers_Arithmetic1D::_narrow( hypothesis() );

    item.myName = tr( "SMESH_START_LENGTH_PARAM" );
    item.myValue = isCreation() ? 1.0 : h->GetLength( true );
    p.append( item );
    item.myName = tr( "SMESH_END_LENGTH_PARAM" );
    item.myValue = isCreation() ? 10.0 : h->GetLength( false );
    p.append( item );
  }
  else if( hypType()=="MaxElementArea" )
  {
    StdMeshers::StdMeshers_MaxElementArea_var h =
      StdMeshers::StdMeshers_MaxElementArea::_narrow( hypothesis() );

    item.myName = tr( "SMESH_MAX_ELEMENT_AREA_PARAM" );
    item.myValue = isCreation() ? 1.0 : h->GetMaxElementArea();
    p.append( item );
  }
  else if( hypType()=="MaxElementVolume" )
  {
    StdMeshers::StdMeshers_MaxElementVolume_var h =
      StdMeshers::StdMeshers_MaxElementVolume::_narrow( hypothesis() );

    item.myName = tr( "SMESH_MAX_ELEMENT_VOLUME_PARAM" );
    item.myValue = isCreation() ? 1.0 : h->GetMaxElementVolume();
    p.append( item );
  }
  else if( hypType()=="StartEndLength" )
  {
    StdMeshers::StdMeshers_StartEndLength_var h =
      StdMeshers::StdMeshers_StartEndLength::_narrow( hypothesis() );

    item.myName = tr( "SMESH_START_LENGTH_PARAM" );
    item.myValue = isCreation() ? 1.0 : h->GetLength( true );
    p.append( item );
    item.myName = tr( "SMESH_END_LENGTH_PARAM" );
    item.myValue = isCreation() ? 10.0 : h->GetLength( false );
    p.append( item );
  }
  else if( hypType()=="Deflection1D" )
  {
    StdMeshers::StdMeshers_Deflection1D_var h =
      StdMeshers::StdMeshers_Deflection1D::_narrow( hypothesis() );

    item.myName = tr( "SMESH_DEFLECTION1D_PARAM" );
    item.myValue = isCreation() ? 1.0 : h->GetDeflection();
    p.append( item );
  }
  else
    res = false;
  return res;
}

void StdMeshersGUI_StdHypothesisCreator::attuneStdWidget( QWidget* w, const int ) const
{
  SMESHGUI_SpinBox* sb = w->inherits( "SMESHGUI_SpinBox" ) ? ( SMESHGUI_SpinBox* )w : 0;
  if( hypType()=="LocalLength" &&  sb )
  {
    sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, 6 );
  }
  else if( hypType()=="Arithmetic1D" && sb )
  {
    sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, 6 );
  }
  else if( hypType()=="MaxElementArea" && sb )
  {
    sb->RangeStepAndValidator( VALUE_SMALL_2, VALUE_MAX_2, 1.0, 6 );
  }
  else if( hypType()=="MaxElementVolume" && sb )
  {
    sb->RangeStepAndValidator( VALUE_SMALL_3, VALUE_MAX_3, 1.0, 6 );
  }
  else if( hypType()=="StartEndLength" && sb )
  {
    sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, 6 );
  }
  else if( hypType()=="Deflection1D" && sb )
  {
    sb->RangeStepAndValidator( VALUE_SMALL, VALUE_MAX, 1.0, 6 );
  }
}

QString StdMeshersGUI_StdHypothesisCreator::caption() const
{
  return tr( QString( "SMESH_%1_TITLE" ).arg( hypTypeName( hypType() ) ) );
}

QPixmap StdMeshersGUI_StdHypothesisCreator::icon() const
{
  QString hypIconName = tr( QString( "ICON_DLG_%1" ).arg( hypTypeName( hypType() ) ) );
  return SMESHGUI::resourceMgr()->loadPixmap( "SMESH", hypIconName );
}

QString StdMeshersGUI_StdHypothesisCreator::type() const
{
  return tr( QString( "SMESH_%1_HYPOTHESIS" ).arg( hypTypeName( hypType() ) ) );
}

QString StdMeshersGUI_StdHypothesisCreator::hypTypeName( const QString& t ) const
{
  static QMap<QString,QString>  types;
  if( types.isEmpty() )
  {
    types.insert( "LocalLength", "LOCAL_LENGTH" );
    types.insert( "NumberOfSegments", "NB_SEGMENTS" );
    types.insert( "MaxElementArea", "MAX_ELEMENT_AREA" );
    types.insert( "MaxElementVolume", "MAX_ELEMENT_VOLUME" );
    types.insert( "StartEndLength", "START_END_LENGTH" );
    types.insert( "Deflection1D", "DEFLECTION1D" );
    types.insert( "Arithmetic1D", "ARITHMETIC_1D" );
  }

  QString res;
  if( types.contains( t ) )
    res = types[ t ];

  return res;
}
