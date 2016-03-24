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
//  File   : SMESHGUI_Operation.cxx
//  Author : Sergey LITONIN, Open CASCADE S.A.S.

#include "SMESHGUI_Operation.h"

#include "SMESHGUI.h"
#include "SMESHGUI_Dialog.h"

// SALOME GUI includes
#include <SalomeApp_Study.h>
#include <LightApp_Application.h>

#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>

// Qt includes
#include <QStringList>

/*
  Class       : SMESHGUI_Operation
  Description : Base class for all SMESH operations
*/

//=======================================================================
// name    : SMESHGUI_Operation
// Purpose : Constructor
//=======================================================================
SMESHGUI_Operation::SMESHGUI_Operation()
: LightApp_Operation(),
  myIsApplyAndClose( false )
{
  myHelpFileName = "";
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
    disconnect( dlg(), SIGNAL( dlgHelp() ), this, SLOT( onHelp() ) );

    if( dlg()->testButtonFlags( QtxDialog::OK ) )
      connect( dlg(), SIGNAL( dlgOk() ), this, SLOT( onOk() ) );

    if( dlg()->testButtonFlags( QtxDialog::Apply ) )
      connect( dlg(), SIGNAL( dlgApply() ), this, SLOT( onApply() ) );

    if( dlg()->testButtonFlags( QtxDialog::Cancel ) )
      connect( dlg(), SIGNAL( dlgCancel() ), this, SLOT( onCancel() ) );

    if( dlg()->testButtonFlags( QtxDialog::Help ) )
      connect( dlg(), SIGNAL( dlgHelp() ), this, SLOT( onHelp() ) );

    //if( dlg()->testButtonFlags( QtxDialog::Close ) )
    //if dialog hasn't close, cancel, no and etc buttons, dlgClose will be emitted when dialog is closed not by OK
    connect( dlg(), SIGNAL( dlgClose() ), this, SLOT( onCancel() ) );

    initDialog();
  }

  LightApp_Operation::startOperation();
}

//=======================================================================
// name    : isReadyToStart
// Purpose : Verify whether operation is ready to start
//=======================================================================
bool SMESHGUI_Operation::isReadyToStart() const
{
  if ( !LightApp_Operation::isReadyToStart() )
    return false;
  else if ( getSMESHGUI() == 0 )
  {
    SUIT_MessageBox::warning( desktop(), tr( "SMESH_WRN_WARNING" ),
                              tr( "NO_MODULE" ) );
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
  LightApp_Operation::setDialogActive( active );

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
  setIsApplyAndClose( true );
  if( onApply() )
    commit();
  setIsApplyAndClose( false );
  //else
  //  abort();
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
// name    : onHelp
// Purpose :
//=======================================================================
void SMESHGUI_Operation::onHelp()
{
  LightApp_Application* app = (LightApp_Application*)(SUIT_Session::session()->activeApplication());
  if (app)
    app->onHelpContextModule(getSMESHGUI() ? app->moduleName(getSMESHGUI()->moduleName()) : QString(""), myHelpFileName);
  else {
    QString platform;
#ifdef WIN32
    platform = "winapplication";
#else
    platform = "application";
#endif
    SUIT_MessageBox::warning( desktop(), tr("WRN_WARNING"),
                              tr("EXTERNAL_BROWSER_CANNOT_SHOW_PAGE").
                              arg(app->resourceMgr()->stringValue("ExternalBrowser",
                                                                  platform)).
                              arg(myHelpFileName) );
  }
}

//=======================================================================
// name    : initDialog
// Purpose :
//=======================================================================
void SMESHGUI_Operation::initDialog()
{
}

//================================================================
// name    : setIsApplyAndClose
// Purpose : Set value of the flag indicating that the dialog is
//           accepted by Apply & Close button
//================================================================
void SMESHGUI_Operation::setIsApplyAndClose( const bool theFlag )
{
  myIsApplyAndClose = theFlag;
}

//================================================================
// name    : isApplyAndClose
// Purpose : Get value of the flag indicating that the dialog is
//           accepted by Apply & Close button
//================================================================
bool SMESHGUI_Operation::isApplyAndClose() const
{
  return myIsApplyAndClose;
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
        SUIT_MessageBox::warning( SMESHGUI::desktop(), tr( "WRN_WARNING" ),
                                  tr( "WRN_STUDY_LOCKED" ) );
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

  return ( theOtherOp &&
         ( ( theOtherOp->inherits("SMESHGUI_Operation") && ( !anOps.contains(theOtherOp->metaObject()->className() ) || anOps.contains(metaObject()->className()) ) ) ||
           ( theOtherOp->inherits("LightApp_ShowHideOp") ) ) );

  return true;
}
