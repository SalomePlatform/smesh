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

//=================================================================================
// class    : SMESHGUI_GenericHypothesisCreator
// purpose  :
//=================================================================================
class SMESHGUI_GenericHypothesisCreator
{
  public:
  virtual void CreateHypothesis (const bool isAlgo, QWidget* parent) = 0;
  virtual void EditHypothesis (SMESH::SMESH_Hypothesis_ptr theHyp) = 0;
};

//=================================================================================
// class    : HypothesisData
// purpose  :
//=================================================================================
class HypothesisData
{
 public:
  HypothesisData (const QString& aPluginName,
		  const QString& aServerLibName,
		  const QString& aClientLibName,
		  const QString& aLabel,
		  const QString& anIconId) :
 PluginName(aPluginName),
 ServerLibName(aServerLibName),
 ClientLibName(aClientLibName),
 Label(aLabel),
 IconId(anIconId)
 {};

 QString PluginName;
 QString ServerLibName;
 QString ClientLibName;
 QString Label;
 QString IconId;
};

#endif
