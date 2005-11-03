//  SMESH SMESHGUI_Selection
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
//  File   : SMESHGUI_Selection.h
//  Author : Alexander SOLOVYOV
//  Module : SMESH
//  $Header$

#ifndef SMESHGUI_SELECTION_HeaderFile
#define SMESHGUI_SELECTION_HeaderFile

#include "LightApp_Selection.h"
#include "SALOMEDSClient_definitions.hxx"

class LightApp_SelectionMgr;
class SALOMEDSClient_Study;
class LightApp_DataOwner;
class SMESH_Actor;

class SMESHGUI_Selection : public LightApp_Selection
{
public:
  SMESHGUI_Selection();
  virtual ~SMESHGUI_Selection();

  virtual void     init( const QString&, LightApp_SelectionMgr* );
  virtual QtxValue param( const int , const QString& paramName ) const;
  virtual void     processOwner( const LightApp_DataOwner* );

  // got from object, not from actor
  virtual int numberOfNodes( int ind ) const;
  virtual QVariant isComputable( int ind ) const;
  virtual QVariant hasReference( int ind ) const;
  virtual QVariant isVisible( int ind ) const;

  // parameters got from actor return nothing if an actor is not visible
  virtual QValueList<QVariant> elemTypes( int ind ) const;
  virtual QValueList<QVariant> labeledTypes( int ind ) const;
  virtual QString displayMode( int ind ) const;
  virtual QString shrinkMode( int ind ) const;
  virtual QValueList<QVariant> entityMode( int ind ) const;
  virtual QString controlMode( int ind ) const;
  
  SMESH_Actor* getActor( int ind ) const;

  static int       type( const QString&, _PTR(Study) );
  static QString   typeName( const int type);

private:
  QStringList            myTypes;
  QPtrList<SMESH_Actor>  myActors;
};

#endif
