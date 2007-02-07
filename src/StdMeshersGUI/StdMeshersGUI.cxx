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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : StdMeshersGUI.cxx
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header$

#include "StdMeshersGUI_StdHypothesisCreator.h"
#include "StdMeshersGUI_NbSegmentsCreator.h"


//=============================================================================
/*! GetHypothesisCreator
 *
 */
//=============================================================================
extern "C"
{
 STDMESHERSGUI_EXPORT
  SMESHGUI_GenericHypothesisCreator* GetHypothesisCreator( const QString& aHypType )
  {
    if( aHypType=="NumberOfSegments" )
      return new StdMeshersGUI_NbSegmentsCreator();
    else
      return new StdMeshersGUI_StdHypothesisCreator( aHypType );
  }
}
