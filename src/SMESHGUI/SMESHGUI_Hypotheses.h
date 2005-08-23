//  SMESH SMESHGUI : GUI for SMESH component
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
//  File   : SMESHGUI_Hypotheses.h
//  Author : Julia DOROVSKIKH
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_Hypotheses_HeaderFile
#define SMESHGUI_Hypotheses_HeaderFile

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)

// QT Includes
#include <qstring.h>
#include <qwidget.h>
#include <qvaluevector.h>

/*!
 * \brief Auxiliary class for creation of hypotheses
*/
class SMESHGUI_GenericHypothesisCreator
{
  public:
  virtual void CreateHypothesis (const bool isAlgo, QWidget* parent) = 0;
  virtual void EditHypothesis (SMESH::SMESH_Hypothesis_ptr theHyp) = 0;
};

/*!
 * \brief Class containing information about hypothesis
*/
class HypothesisData
{
 public:
  HypothesisData( const QString& thePluginName,
                  const QString& theServerLibName,
                  const QString& theClientLibName,
                  const QString& theLabel,
                  const QString& theIconId,
                  const QValueList<int>& theDim,
                  const bool theIsAux ) 
: PluginName( thePluginName ),
  ServerLibName( theServerLibName ),
  ClientLibName( theClientLibName ),
  Label( theLabel ),
  IconId( theIconId ),
  Dim( theDim ),
  IsAux( theIsAux )
 {};

 QString PluginName;      //!< plugin name
 QString ServerLibName;   //!< server library name
 QString ClientLibName;   //!< client library name
 QString Label;           //!< label
 QString IconId;          //!< icon identifier
 QValueList<int> Dim;     //!< list of supported dimensions (see SMESH::Dimension enumeration)
 bool IsAux;              //!< TRUE if given hypothesis is auxiliary one, FALSE otherwise
};

#endif
