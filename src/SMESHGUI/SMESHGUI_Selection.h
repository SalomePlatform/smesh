// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// SMESH SMESHGUI_Selection
// File   : SMESHGUI_Selection.h
// Author : Alexander SOLOVYOV, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_SELECTION_H
#define SMESHGUI_SELECTION_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// SALOME GUI includes
#include <LightApp_Selection.h>

// SALOME KERNEL includes
#include <SALOMEDSClient_definitions.hxx>

class LightApp_SelectionMgr;
class LightApp_DataOwner;
class SALOMEDSClient_Study;
class SMESH_Actor;

class SMESHGUI_EXPORT SMESHGUI_Selection : public LightApp_Selection
{
public:
  SMESHGUI_Selection();
  virtual ~SMESHGUI_Selection();

  virtual void            init( const QString&, LightApp_SelectionMgr* );
  virtual QVariant        parameter( const int, const QString& ) const;
  virtual bool            processOwner( const LightApp_DataOwner* );

  // got from object, not from actor
  virtual bool            isAutoColor( int ) const;
  virtual int             numberOfNodes( int ) const;
  virtual int             dim( int ) const;
  virtual QVariant        isComputable( int ) const;
  virtual QVariant        isPreComputable( int ) const;
  virtual QVariant        hasReference( int ) const;
  virtual QVariant        isVisible( int ) const;

  virtual QString         quadratic2DMode(int ) const;

  virtual bool            isDistributionVisible(int ) const;

  // parameters got from actor return nothing if an actor is not visible
  virtual QList<QVariant> elemTypes( int ) const;
  virtual QList<QVariant> labeledTypes( int ) const;
  virtual QString         displayMode( int ) const;
  virtual QString         shrinkMode( int ) const;
  virtual QList<QVariant> entityMode( int ) const;
  virtual QString         controlMode( int ) const;
  virtual bool            isNumFunctor( int ) const;
  virtual QString         facesOrientationMode( int ) const;
  virtual QString         groupType( int ) const;
  
  SMESH_Actor*            getActor( int ) const;

  static int              type( const QString&, _PTR(Study) );
  static QString          typeName( const int );

  bool                    isImported( const int ) const;

private:
  QStringList             myTypes;
  QList<SMESH_Actor*>     myActors;
};

#endif // SMESHGUI_SELECTION_H
