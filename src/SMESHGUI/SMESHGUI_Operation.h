// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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

// SMESH SMDS : implementaion of Salome mesh data structure
// File   : SMESHGUI_Operation.h
// Author : Sergey LITONIN, Open CASCADE S.A.S.
//
#ifndef SMESHGUI_OPERATION_H
#define SMESHGUI_OPERATION_H

// SMESH includes
#include "SMESH_SMESHGUI.hxx"

// SALOME GUI includes
#include <LightApp_Operation.h>

// SALOME KERNEL includes
#include <SALOMEDSClient.hxx>

class SMESHGUI;

/*
  Class       : SMESHGUI_Operation
  Description : Base class for all SMESH operations
*/

class SMESHGUI_EXPORT SMESHGUI_Operation : public LightApp_Operation
{
  Q_OBJECT

public:
  SMESHGUI_Operation();
  virtual ~SMESHGUI_Operation();

protected:
  //! sets the dialog widgets to state just after operation start
  virtual void      initDialog();

  virtual void      startOperation();
  virtual bool      isReadyToStart() const;
  
  virtual void      setIsApplyAndClose( const bool theFlag );
  virtual bool      isApplyAndClose() const;

  //! Set according dialog active or inactive
  virtual void      setDialogActive( const bool );

  SMESHGUI*         getSMESHGUI() const;
  bool              isStudyLocked( const bool = true ) const;

  _PTR(Study)       studyDS() const;
  
  virtual bool      isValid( SUIT_Operation* ) const;

  QString           myHelpFileName;
  bool              myIsApplyAndClose;

protected slots:
  virtual void      onOk();
  virtual bool      onApply();
  virtual void      onCancel();
  void              onHelp();
};

#endif // SMESHGUI_OPERATION_H
