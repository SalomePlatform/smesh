//  SALOME SMESHGUI
//
//  Copyright (C) 2005  CEA/DEN, EDF R&D
//
//
//
//  File   : SMESHGUI_Operation.h
//  Author : Sergey LITONIN
//  Module : SALOME

#include "SMESHGUI_Operation.h"
#include <SMESHGUI.h>
#include <SMESHGUI_Dialog.h>

#include <SalomeApp_Study.h>

#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>

#include <qstringlist.h>

/*
  Class       : SMESHGUI_Operation
  Description : Base class for all SMESH operations
*/

//=======================================================================
// name    : SMESHGUI_Operation
// Purpose : Constructor
//=======================================================================
SMESHGUI_Operation::SMESHGUI_Operation()
: SalomeApp_Operation()
{
}

//=======================================================================
// name    : ~SMESHGUI_Operation
// Purpose : Destructor
//=======================================================================
SMESHGUI_Operation::~SMESHGUI_Operation()
{
}

//=======================================================================
// name    : getSMESHGUI
// Purpose : Get SMESH module
//=======================================================================
SMESHGUI* SMESHGUI_Operation::getSMESHGUI() const
{
  return dynamic_cast<SMESHGUI*>( module() );
}

//=======================================================================
// name    : startOperation
// Purpose : Start opeartion
//=======================================================================
void SMESHGUI_Operation::startOperation()
{
  if( dlg() )
  {
    disconnect( dlg(), SIGNAL( dlgOk() ), this, SLOT( onOk() ) );
    disconnect( dlg(), SIGNAL( dlgApply() ), this, SLOT( onApply() ) );
    disconnect( dlg(), SIGNAL( dlgCancel() ), this, SLOT( onCancel() ) );
    disconnect( dlg(), SIGNAL( dlgClose() ), this, SLOT( onCancel() ) );
    
    if( dlg()->testButtonFlags( QtxDialog::OK ) )
      connect( dlg(), SIGNAL( dlgOk() ), this, SLOT( onOk() ) );
      
    if( dlg()->testButtonFlags( QtxDialog::Apply ) )
      connect( dlg(), SIGNAL( dlgApply() ), this, SLOT( onApply() ) );
      
    if( dlg()->testButtonFlags( QtxDialog::Cancel ) )
      connect( dlg(), SIGNAL( dlgCancel() ), this, SLOT( onCancel() ) );
      
    //if( dlg()->testButtonFlags( QtxDialog::Close ) )
    //if dialog hasn't close, cancel, no and etc buttons, dlgClose will be emitted when dialog is closed not by OK
    connect( dlg(), SIGNAL( dlgClose() ), this, SLOT( onCancel() ) );

    initDialog();
  }

  SalomeApp_Operation::startOperation();
}

//=======================================================================
// name    : isReadyToStart
// Purpose : Verify whether operation is ready to start
//=======================================================================
bool SMESHGUI_Operation::isReadyToStart() const
{
  if ( !SalomeApp_Operation::isReadyToStart() )
    return false;
  else if ( getSMESHGUI() == 0 )
  {
    SUIT_MessageBox::warn1( desktop(), tr( "SMESH_WRN_WARNING" ),
      tr( "NO_MODULE" ), tr( "SMESH_BUT_OK" ) );
    return false;
  }
  else if ( isStudyLocked() )
    return false;
  
  return true;
}

//=======================================================================
// name    : setDialogActive
// Purpose : 
//=======================================================================
void SMESHGUI_Operation::setDialogActive( const bool active )
{
  SalomeApp_Operation::setDialogActive( active );

  SMESHGUI_Dialog* d = dynamic_cast<SMESHGUI_Dialog*>( dlg() );
  if( d )
    d->setContentActive( active );

}

//=======================================================================
// name    : studyDS
// Purpose :
//=======================================================================
_PTR(Study) SMESHGUI_Operation::studyDS() const
{
  SalomeApp_Study* s = dynamic_cast<SalomeApp_Study*>( study() );
  return s->studyDS();
}

//=======================================================================
// name    : onOk
// Purpose :
//=======================================================================
void SMESHGUI_Operation::onOk()
{
  if( onApply() )
    commit();
  else
    abort();
}

//=======================================================================
// name    : onApply
// Purpose :
//=======================================================================
bool SMESHGUI_Operation::onApply()
{
  return false;
}

//=======================================================================
// name    : onClose
// Purpose :
//=======================================================================
void SMESHGUI_Operation::onCancel()
{
  abort();
}

//=======================================================================
// name    : initDialog
// Purpose :
//=======================================================================
void SMESHGUI_Operation::initDialog()
{
}

/*!
 * \brief Verifies whether study of operation is locked
  * \param theMess - specifies whether message box must be shown if study is locked
  * \return State of study.
*
* Verifies whether study of operation is locked. If second parameter is TRUE and study
* is locked when corresponding message box appears
*/
bool SMESHGUI_Operation::isStudyLocked( const bool theMess ) const
{
  if ( studyDS() )
  {
    if ( studyDS()->GetProperties()->IsLocked() )
    {
      if ( theMess )
        SUIT_MessageBox::warn1 ( SMESHGUI::desktop(), QObject::tr( "WRN_WARNING" ),
          QObject::tr( "WRN_STUDY_LOCKED" ), QObject::tr( "BUT_OK" ) );
      return true;
    }
  }
  
  return false;
}

/*!
 * \brief Verifies whether given operator is valid for this one
  * \param theOtherOp - other operation
  * \return Returns TRUE if the given operator is valid for this one, FALSE otherwise
*
* Virtual method redefined from base class verifies whether given operator is valid for
* this one (i.e. can be started "above" this operator). In current implementation method
* retuns false if theOtherOp operation is not intended for deleting objects or mesh
* elements.
*/
bool SMESHGUI_Operation::isValid( SUIT_Operation* theOtherOp ) const
{
  static QStringList anOps;
  if ( anOps.count() == 0 )
  {
    anOps.append( "SMESHGUI_DeleteOp" );
    // to do add other operations here
  }

  return theOtherOp && theOtherOp->inherits( "SMESHGUI_Operation" ) &&
         ( !anOps.contains( theOtherOp->className() ) || anOps.contains( className() ) );

  return true;
}












