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

#include <QtxPopupMgr.h>

#include "SALOMEDSClient_definitions.hxx"
#include "SUIT_DataOwner.h"

class SalomeApp_SelectionMgr;
class SALOMEDSClient_Study;
class SalomeApp_DataOwner;
class SMESH_Actor;

class SMESHGUI_Selection : public QtxPopupMgr::Selection
{
public:
  SMESHGUI_Selection( const QString&, SalomeApp_SelectionMgr* );
  virtual ~SMESHGUI_Selection();

  virtual QtxValue param( const int , const QString& paramName ) const;

  virtual int count() const;

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

  static int       type( SalomeApp_DataOwner* owner, _PTR(Study) study);
  static QString   typeName( const int type);

private:
  QString               myPopupClient;
  QStringList           myTypes;
  SUIT_DataOwnerPtrList myDataOwners;
};

#endif
