//  SALOME SMESHGUI
//
//  Copyright (C) 2005  CEA/DEN, EDF R&D
//
//
//
//  File   : SMESHGUI_Operation.h
//  Author : Sergey LITONIN
//  Module : SALOME


#ifndef SMESHGUI_Operation_H
#define SMESHGUI_Operation_H

#include <SalomeApp_Operation.h>
#include <SALOMEDSClient.hxx>

class SMESHGUI;

/*
  Class       : SMESHGUI_Operation
  Description : Base class for all SMESH operations
*/

class SMESHGUI_Operation : public SalomeApp_Operation
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
  
  //! Set according dialog active or inactive
  virtual void      setDialogActive( const bool );

  SMESHGUI*         getSMESHGUI() const;
  bool              isStudyLocked( const bool = true ) const;

  _PTR(Study)       studyDS() const;
  
  virtual bool      isValid( SUIT_Operation* ) const;

protected slots:
  virtual void onOk();
  virtual bool onApply();
  virtual void onCancel();
};

#endif






