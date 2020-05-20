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

#include "MonPursueIteration.h"
#include "HOMARD.hxx"

#include <QFileDialog>
#include <QMessageBox>

#include "SalomeApp_Tools.h"
#include "HOMARDGUI_Utils.h"
#include "HomardQtCommun.h"
#include <utilities.h>

#ifdef WIN32
#include <direct.h>
#endif

using namespace std;

// -----------------------------------------------------------------------------------------
/* Constructs a MonPursueIteration
 * Sets attributes to default values
 */
// -----------------------------------------------------------------------------------------
MonPursueIteration::MonPursueIteration ( bool modal, HOMARD::HOMARD_Gen_var myHomardGen0 )
    :
    Ui_PursueIteration(),
    _aCaseName(""), _aDirName(""), _aDirNameStart("")
{
  MESSAGE("Debut du constructeur de MonPursueIteration");
  myHomardGen=HOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
  setupUi(this);
  setModal(modal);

  InitConnect();

  SetNewCaseName() ;
  _Type = 1 ;
  GBIterationintoCase->setVisible(0);
  SpinBoxNumber->setVisible(0);

  adjustSize();
  MESSAGE("Fin du constructeur de MonPursueIteration");
}

// ------------------------------------------------------------------------
MonPursueIteration::~MonPursueIteration()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void MonPursueIteration::InitConnect()
// ------------------------------------------------------------------------
{
    connect( PushDir,        SIGNAL(pressed()), this, SLOT(SetDirName()));

    connect( RBIteration,    SIGNAL(clicked()), this, SLOT(FromIteration()));
    connect( RBCase,         SIGNAL(clicked()), this, SLOT(FromCase()));
    connect( PushDirStart,   SIGNAL(pressed()), this, SLOT(SetDirNameStart()));

    connect( RBCaseLastIteration, SIGNAL(clicked()), this, SLOT(CaseLastIteration()));
    connect( RBCaseNIteration,    SIGNAL(clicked()), this, SLOT(CaseNIteration()));

    connect( buttonOk,       SIGNAL(pressed()), this, SLOT(PushOnOK()));
    connect( buttonApply,    SIGNAL(pressed()), this, SLOT(PushOnApply()));
    connect( buttonCancel,   SIGNAL(pressed()), this, SLOT(close()));
    connect( buttonHelp,     SIGNAL(pressed()), this, SLOT(PushOnHelp()));
}
// -------------------------------
bool MonPursueIteration::PushOnApply()
// --------------------------------
{
  MESSAGE("PushOnApply");
// 1. Enregistrement du repertoire du cas
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
// 2. Enregistrement du repertoire de depart
  QString aDirNameStart=LEDirNameStart->text().trimmed();
  if (aDirNameStart == QString(""))
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_START_DIRECTORY_1") );
    return false;
  }
  if (CHDIR(aDirNameStart.toStdString().c_str()) != 0)
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_START_DIRECTORY_3") );
    return false;
  }

// 3. Enregistrement du numero d'iteration
  int Number ;
  if ( _Type == 3 ) { Number = SpinBoxNumber->value() ; }

// 4. Creation du cas
  QString _aCaseName=LECaseName->text().trimmed();
  _aDirNameStart=aDirNameStart;

  MESSAGE("_aCaseName = "<<_aCaseName.toStdString().c_str());
  MESSAGE("_aDirNameStart = "<<_aDirNameStart.toStdString().c_str());
  MESSAGE("_Type = "<<_Type);
  switch (_Type)
  {
    case 1 : // Poursuite a partir d'une iteration
    {
      try
      {
        MESSAGE("Poursuite a partir d'une iteration");
        aCase = myHomardGen->CreateCaseFromIteration( \
                  CORBA::string_dup(_aCaseName.toStdString().c_str()),  \
                  CORBA::string_dup(_aDirNameStart.toStdString().c_str()) );
      }
      catch( SALOME::SALOME_Exception& S_ex )
      {
        QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                  QObject::tr(CORBA::string_dup(S_ex.details.text)) );
        return false ;
      }
      break;
    }
    case 2 : // Poursuite a partir de la derniere iteration dans un cas
    {
      try
      {
        MESSAGE("Poursuite a partir de la derniere iteration dans un cas");
        aCase = myHomardGen->CreateCaseFromCaseLastIteration( \
                  CORBA::string_dup(_aCaseName.toStdString().c_str()),  \
                  CORBA::string_dup(_aDirNameStart.toStdString().c_str()) );
      }
      catch( SALOME::SALOME_Exception& S_ex )
      {
        QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                  QObject::tr(CORBA::string_dup(S_ex.details.text)) );
        return false ;
      }
      break;
    }
    case 3 : // Poursuite a partir d'une iteration dans un cas
    {
      try
      {
        MESSAGE("Poursuite a partir d'une iteration dans un cas");
        aCase = myHomardGen->CreateCaseFromCaseIteration( \
                  CORBA::string_dup(_aCaseName.toStdString().c_str()),  \
                  CORBA::string_dup(_aDirNameStart.toStdString().c_str()),  \
                  Number );
      }
      catch( SALOME::SALOME_Exception& S_ex )
      {
        QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                  QObject::tr(CORBA::string_dup(S_ex.details.text)) );
        return false ;
      }
      break;
    }
  }

  aCase->SetDirName(aDirName.toStdString().c_str());
  _aDirName=aDirName;

  HOMARD_UTILS::updateObjBrowser();

  return true;
}
// ---------------------------
void MonPursueIteration::PushOnOK()
// ---------------------------
{
  bool bOK = PushOnApply();
  if ( bOK )  this->close();
}
//------------------------------
void MonPursueIteration::PushOnHelp()
//-------------------------------
{
  std::string LanguageShort = myHomardGen->GetLanguageShort();
  HOMARD_UTILS::PushOnHelp(QString("gui_create_case.html"), QString(""), QString(LanguageShort.c_str()));
}
// ---------------------------------
void MonPursueIteration::SetNewCaseName()
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
      if ( aCaseName == QString((MyCases)[i]))
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
void MonPursueIteration::SetDirName()
// ------------------------------------------------------------------------
{
  QString aDirName=QFileDialog::getExistingDirectory ();
  if (!(aDirName.isEmpty()))LEDirName->setText(aDirName);
}
// ------------------------------------------------------------------------
void MonPursueIteration::SetDirNameStart()
// ------------------------------------------------------------------------
{
  QString aDirName=QFileDialog::getExistingDirectory ();
  if (!(aDirName.isEmpty()))LEDirNameStart->setText(aDirName);
}
// ------------------------------------------------------------------------
void MonPursueIteration::FromIteration()
// ------------------------------------------------------------------------
{
  GBIterationintoCase->setVisible(0);
  SpinBoxNumber->setVisible(0);
  _Type = 1 ;
  adjustSize();
}
// ------------------------------------------------------------------------
void MonPursueIteration::FromCase()
// ------------------------------------------------------------------------
{
  GBIterationintoCase->setVisible(1);
  CaseLastIteration();
}
// ------------------------------------------------------------------------
void MonPursueIteration::CaseLastIteration()
// ------------------------------------------------------------------------
{
  SpinBoxNumber->setVisible(0);
  _Type = 2 ;
  adjustSize();
}
// ------------------------------------------------------------------------
void MonPursueIteration::CaseNIteration()
// ------------------------------------------------------------------------
{
  SpinBoxNumber->setVisible(1);
  _Type = 3 ;
  adjustSize();
}
