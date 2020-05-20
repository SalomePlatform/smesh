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

#include "MonMeshInfo.h"
#include "HOMARD.hxx"

#include <QFileDialog>
#include <QMessageBox>

#include "SalomeApp_Tools.h"
#include "HOMARDGUI_Utils.h"
#include "HomardQtCommun.h"
#include "MonEditFile.h"
#include <utilities.h>

#ifdef WIN32
#include <direct.h>
#endif

using namespace std;

// -----------------------------------------------------------------------------------------
MonMeshInfo::MonMeshInfo(QWidget* parent, bool modal, HOMARD::HOMARD_Gen_var myHomardGen0)
// -----------------------------------------------------------------------------------------
/* Constructs a MonMeshInfo
 * Inherits from CasHomard
 * Sets attributes to default values
 */
    :
    Ui_MeshInfo(),
    _aCaseName(""),_aDirName(""),
    _BlockSize(0),
    _Connection(0),
    _Diametre(0),
    _Entanglement(0),
    _Quality(0)
{
      myHomardGen=HOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
      setupUi(this);
      setModal(modal);
      InitConnect();

      SetNewCaseName() ;
      adjustSize();
}

// ------------------------------------------------------------------------
MonMeshInfo::~MonMeshInfo()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void MonMeshInfo::InitConnect()
// ------------------------------------------------------------------------
{
    connect( LECaseName,     SIGNAL(textChanged(QString)), this, SLOT(CaseNameChanged()));
    connect( PushDir,        SIGNAL(pressed()), this, SLOT(SetDirName()));
    connect( PushFichier,    SIGNAL(pressed()), this, SLOT(SetFileName()));

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
bool MonMeshInfo::PushOnApply()
// --------------------------------
{
  MESSAGE("PushOnApply");
  QString aCaseName=LECaseName->text().trimmed();
  if ( aCaseName == "" )
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_CASE_NAME") );
    return false;
  }

  QString aDirName=LEDirName->text().trimmed();
  if (aDirName == QString(""))
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_CASE_DIRECTORY_1") );
    return false;
  }
  if ( aDirName != _aDirName)
  { QString CaseNameDir = myHomardGen->VerifieDir( aDirName.toStdString().c_str()) ;
    if ( CaseNameDir != "" )
    {
      QString texte ;
      texte = QObject::tr("HOM_CASE_DIRECTORY_2") + CaseNameDir ;
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                texte );
      return false;
    }
  }
  if (CHDIR(aDirName.toStdString().c_str()) != 0)
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_CASE_DIRECTORY_3") );
    return false;
  }

  QString aFileName=LEFileName->text().trimmed();
  if (aFileName ==QString(""))
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_CASE_MESH") );
    return false;
  }

  QString aMeshName = HOMARD_QT_COMMUN::LireNomMaillage(aFileName);
  if (aMeshName == "" )
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_MED_FILE_2") );
    return false;
  }
  if ( ( _Quality == 0 ) && ( _Diametre == 0 ) && ( _Connection == 0 ) && ( _BlockSize == 0 ) && ( _Entanglement == 0 ) )
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_MESH_INFO") );
    return false;
  }

  if (aCaseName != _aCaseName )
  {
    _aCaseName = aCaseName;
    try
    {
      myHomardGen->MeshInfo( \
            CORBA::string_dup(_aCaseName.toStdString().c_str()),  \
            CORBA::string_dup(aMeshName.toStdString().c_str()),  \
            CORBA::string_dup(aFileName.toStdString().c_str()),  \
            CORBA::string_dup(aDirName.toStdString().c_str()),  \
            _Quality, _Diametre, _Connection, _BlockSize, _Entanglement );
    }
    catch( SALOME::SALOME_Exception& S_ex )
    {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr(CORBA::string_dup(S_ex.details.text)) );
      return false;
    }
  }

  // Le bilan de l'analyse a afficher
  aCase = myHomardGen->GetCase(_aCaseName.toStdString().c_str());
  string iter0 = aCase->GetIter0Name();
  HOMARD::HOMARD_Iteration_var aIter =  myHomardGen->GetIteration(iter0.c_str());
  aFileName = aIter->GetFileInfo() ;
  MonEditFile *aDlg = new MonEditFile( 0, true, HOMARD::HOMARD_Gen::_duplicate(myHomardGen), aFileName, 1 ) ;
  if ( aDlg->_codret == 0 ) { aDlg->show(); }

HOMARD_UTILS::updateObjBrowser();
  return true;
}
// ---------------------------
void MonMeshInfo::PushOnOK()
// ---------------------------
{
  bool bOK = PushOnApply();
  if ( bOK )  this->close();
}
//------------------------------
void MonMeshInfo::PushOnHelp()
//-------------------------------
{
  std::string LanguageShort = myHomardGen->GetLanguageShort();
  HOMARD_UTILS::PushOnHelp(QString("gui_create_case.html"), QString(""), QString(LanguageShort.c_str()));
}
// ---------------------------------
void MonMeshInfo::SetNewCaseName()
// ------------------------------
{
  HOMARD::listeCases_var  MyCases = myHomardGen->GetAllCasesName();
  int num = 0; QString aCaseName="";
  while (aCaseName=="" )
  {
    aCaseName.setNum(num+1) ;
    aCaseName.insert(0, QString("Case_")) ;
    for ( int i=0; i<MyCases->length(); i++)
    {
      if ( aCaseName == QString((MyCases)[i]) )
      {
        num ++ ;
        aCaseName = "" ;
        break ;
      }
   }
  }
  LECaseName->clear() ;
  LECaseName->insert(aCaseName);
}
// ------------------------------------------------------------------------
void MonMeshInfo::SetDirName()
// ------------------------------------------------------------------------
{
  QString aDirName=QFileDialog::getExistingDirectory ();
  if (!(aDirName.isEmpty()))LEDirName->setText(aDirName);
}
// ------------------------------------------------------------------------
void MonMeshInfo::SetFileName()
// ------------------------------------------------------------------------
{
  QString fileName0 = LEFileName->text().trimmed();
  QString fileName = HOMARD_QT_COMMUN::PushNomFichier( false, QString("med") ) ;
  if (fileName.isEmpty()) fileName = fileName0 ;
  LEFileName->setText(fileName);
}
// ------------------------------------------------------------------------
void MonMeshInfo::CaseNameChanged()
// ------------------------------------------------------------------------
{
  if (_aCaseName != LECaseName->text().trimmed())
  {
    LEFileName->setReadOnly(false);
    PushFichier->show();
  }
}
// ------------------------------------------------------------------------
void MonMeshInfo::SetBlockSize()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetBlockSize ");
  if ( CBBlockSize->isChecked() ) { _BlockSize = 1 ; }
  else                            { _BlockSize = 0 ; }
}
// ------------------------------------------------------------------------
void MonMeshInfo::SetConnection()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetConnection ");
  if ( CBConnection->isChecked() ) { _Connection = 1 ; }
  else                             { _Connection = 0 ; }
}
// ------------------------------------------------------------------------
void MonMeshInfo::SetDiametre()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetDiametre ");
  if ( CBDiametre->isChecked() ) { _Diametre = 1 ; }
  else                           { _Diametre = 0 ; }
}
// ------------------------------------------------------------------------
void MonMeshInfo::SetEntanglement()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetEntanglement ");
  if ( CBEntanglement->isChecked() ) { _Entanglement = 1 ; }
  else                               { _Entanglement = 0 ; }
}
// ------------------------------------------------------------------------
void MonMeshInfo::SetQuality()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetQuality ");
  if ( CBQuality->isChecked() ) { _Quality = 1 ; }
  else                          { _Quality = 0 ; }
}
