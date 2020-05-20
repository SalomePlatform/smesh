// Copyright (C) 2011-2020  CEA/DEN, EDF R&D
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

#include "MonIterInfo.h"

#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>

#include "SalomeApp_Tools.h"
#include "HOMARDGUI_Utils.h"
#include "HomardQtCommun.h"
#include "MonEditFile.h"
#include <utilities.h>

using namespace std;

// -----------------------------------------------------------------------------------------
MonIterInfo::MonIterInfo(QWidget* parent, bool modal, HOMARD::HOMARD_Gen_var myHomardGen0, QString IterName)
// -----------------------------------------------------------------------------------------
/* Constructs a MonIterInfo
 * Inherits from CasHomard
 * Sets attributes to default values
 */
    :
    Ui_IterInfo(),
    _IterName(IterName),
    _aCaseName(""),
    _BlockSize(0),
    _Connection(0),
    _Diametre(0),
    _Entanglement(0),
    _Quality(0),
    _Option(-1)
{
    MESSAGE("appel de _duplicate");
      myHomardGen=HOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
      setupUi(this);
      setModal(modal);
      InitConnect();

      adjustSize();
}

// ------------------------------------------------------------------------
MonIterInfo::~MonIterInfo()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void MonIterInfo::InitConnect()
// ------------------------------------------------------------------------
{
    MESSAGE("InitConnect");
    connect( CBBlockSize,    SIGNAL(stateChanged(int)), this, SLOT(SetBlockSize()));
    connect( CBConnection,   SIGNAL(stateChanged(int)), this, SLOT(SetConnection()));
    connect( CBDiametre,     SIGNAL(stateChanged(int)), this, SLOT(SetDiametre()));
    connect( CBEntanglement, SIGNAL(stateChanged(int)), this, SLOT(SetEntanglement()));
    connect( CBQuality,      SIGNAL(stateChanged(int)), this, SLOT(SetQuality()));

    connect( buttonOk,       SIGNAL(pressed()), this, SLOT(PushOnOK()));
    connect( buttonApply,    SIGNAL(pressed()), this, SLOT(PushOnApply()));
    connect( buttonCancel,   SIGNAL(pressed()), this, SLOT(close()));
    connect( buttonHelp,     SIGNAL(pressed()), this, SLOT(PushOnHelp()));
}
// -------------------------------
bool MonIterInfo::PushOnApply()
// --------------------------------
{
  MESSAGE("PushOnApply");

  // Au moins une option a ete choisie
  if ( ( _Quality == 0 ) && ( _Diametre == 0 ) && ( _Connection == 0 ) && ( _BlockSize == 0 ) && ( _Entanglement == 0 ) )
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_MESH_INFO_1") );
    return false;
  }

  // Recuperation de l'iteration
  aIter = myHomardGen->GetIteration(_IterName.toStdString().c_str()) ;

  // Lancement de l'analyse
  try
  {
    aIter->MeshInfoOption( _Quality, _Diametre, _Connection, _BlockSize, _Entanglement, _Option );
  }
  catch( SALOME::SALOME_Exception& S_ex )
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr(CORBA::string_dup(S_ex.details.text)) );
    return false;
  }

  // Le bilan de l'analyse a afficher
  QString aFileName = aIter->GetFileInfo() ;
  MonEditFile *aDlg = new MonEditFile( 0, true, HOMARD::HOMARD_Gen::_duplicate(myHomardGen), aFileName, 0 ) ;
  if ( aDlg->_codret == 0 ) { aDlg->show(); }


  HOMARD_UTILS::updateObjBrowser();
  return true;
}
// ---------------------------
void MonIterInfo::PushOnOK()
// ---------------------------
{
     bool bOK = PushOnApply();
     if ( bOK )  this->close();
}
//------------------------------
void MonIterInfo::PushOnHelp()
//-------------------------------
{
  std::string LanguageShort = myHomardGen->GetLanguageShort();
  HOMARD_UTILS::PushOnHelp(QString("gui_mesh_info.html"), QString(""), QString(LanguageShort.c_str()));
}
// ------------------------------------------------------------------------
void MonIterInfo::SetBlockSize()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetBlockSize ");
  if ( CBBlockSize->isChecked() ) { _BlockSize = 1 ; }
  else                            { _BlockSize = 0 ; }
}
// ------------------------------------------------------------------------
void MonIterInfo::SetConnection()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetConnection ");
  if ( CBConnection->isChecked() ) { _Connection = 1 ; }
  else                             { _Connection = 0 ; }
}
// ------------------------------------------------------------------------
void MonIterInfo::SetDiametre()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetDiametre ");
  if ( CBDiametre->isChecked() ) { _Diametre = 1 ; }
  else                           { _Diametre = 0 ; }
}
// ------------------------------------------------------------------------
void MonIterInfo::SetEntanglement()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetEntanglement ");
  if ( CBEntanglement->isChecked() ) { _Entanglement = 1 ; }
  else                               { _Entanglement = 0 ; }
}
// ------------------------------------------------------------------------
void MonIterInfo::SetQuality()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetQuality ");
  if ( CBQuality->isChecked() ) { _Quality = 1 ; }
  else                          { _Quality = 0 ; }
}
