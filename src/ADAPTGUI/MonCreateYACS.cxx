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

#include "MonCreateYACS.h"
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

// ----------------------------------------------------------------------
MonCreateYACS::MonCreateYACS (bool modal, HOMARD::HOMARD_Gen_var myHomardGen0, QString CaseName ):
// ----------------------------------------------------------------------
/* Constructs a MonCreateYACS
 * Sets attributes to default values
 */
// ----------------------------------------------------------------------
  Ui_CreateYACS(),
  _aCaseName(CaseName),
  _aScriptFile(""),
  _aDirName(""),
  _aMeshFile(""),
  _Type(1)
  // Les valeurs de _Type, _MaxIter, _MaxNode, _MaxElem doivent etre les memes que celles dans HOMARD_Gen_i::CreateYACSSchema
  // et doivent correspondre aux defauts des boutons
  {
//     MESSAGE("Debut du constructeur de MonCreateYACS");
    myHomardGen=HOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
    setupUi(this);
    if ( modal ) { setWindowModality(Qt::WindowModal); }
    else         { setWindowModality(Qt::NonModal); }

    InitConnect();

    SetNewName() ;

    if (_aCaseName != QString("")) { SetCaseName(); }
    else                           { setWindowModality(Qt::NonModal); /* permet selection du cas dans l arbre d etude */}
//
//  Les valeurs definies dans les preferences
    _MaxIter = myHomardGen->GetYACSMaxIter();
    _MaxNode = myHomardGen->GetYACSMaxNode();
    _MaxElem = myHomardGen->GetYACSMaxElem();
    MESSAGE ("Valeur par defaut de MaxIter = " << _MaxIter<<", MaxNode = "<< _MaxNode<<", MaxElem = "<< _MaxElem);
    SpinBoxMaxIter->setValue(_MaxIter) ;
    SpinBoxMaxNode->setValue(_MaxNode) ;
    SpinBoxMaxElem->setValue(_MaxElem) ;
//
    adjustSize();
  }

// ----------------------------------------------------------------------
MonCreateYACS::MonCreateYACS(HOMARD::HOMARD_Gen_var myHomardGen0,
                             QString caseName):
// ----------------------------------------------------------------------
// Constructeur appele par MonEditYACS
//
myHomardGen(myHomardGen0),
_Name (""),
Chgt (false)
{
//       MESSAGE("Debut du constructeur de MonCreateYACS appele par MonEditYACS");
  setupUi(this) ;

  setWindowModality(Qt::WindowModal);
  InitConnect() ;
//
  adjustSize();
}
// ------------------------------------------------------------------------
MonCreateYACS::~MonCreateYACS()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void MonCreateYACS::InitConnect()
// ------------------------------------------------------------------------
{
    connect( PBCaseName,     SIGNAL(pressed()), this, SLOT(SetCaseName()));
    connect( PBScriptFile,   SIGNAL(pressed()), this, SLOT(SetScriptFile()));
    connect( PBDir,          SIGNAL(pressed()), this, SLOT(SetDirName()));
    connect( PBMeshFile,     SIGNAL(pressed()), this, SLOT(SetMeshFile()));

    connect( RBConstant,     SIGNAL(clicked()), this, SLOT(SetConstant()));
    connect( RBVariable,     SIGNAL(clicked()), this, SLOT(SetVariable()));

    connect( buttonOk,       SIGNAL(pressed()), this, SLOT(PushOnOK()));
    connect( buttonApply,    SIGNAL(pressed()), this, SLOT(PushOnApply()));
    connect( buttonCancel,   SIGNAL(pressed()), this, SLOT(close()));
    connect( buttonHelp,     SIGNAL(pressed()), this, SLOT(PushOnHelp()));
}
// -------------------------------
bool MonCreateYACS::PushOnApply()
// --------------------------------
{
  MESSAGE("PushOnApply");

// Le fichier du script
  QString aFileName=LEScriptFile->text().trimmed();
  if (aFileName ==QString(""))
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_SCRIPT_FILE") );
    return false;
  }

// Le repertoire de calcul
  QString aDirName=LEDirName->text().trimmed();
  if (aDirName == QString(""))
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_CASE_DIRECTORY_4") );
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

// Le fichier du tout premier maillage
  aFileName=LEMeshFile->text().trimmed();
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

  bool bOK = CreateOrUpdate() ;

  if ( bOK ) { HOMARD_UTILS::updateObjBrowser() ; }

  return bOK;
}
// ---------------------------------------------------
bool MonCreateYACS:: CreateOrUpdate()
//----------------------------------------------------
//  Creation ou modification du schema
{
  MESSAGE("CreateOrUpdate");
  bool bOK = true ;

  // 1. Verification des donnees
  // 1.1. Le cas
  if ( _aCaseName == QString (""))
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_CASE_NAME") );
    return false;
  }
  // 1.2. Les donnees
  QString aScriptFile=LEScriptFile->text().trimmed();
  if ( aScriptFile != _aScriptFile )
  {
    _aScriptFile = aScriptFile ;
    Chgt = true ;
  }
  QString aDirName=LEDirName->text().trimmed();
  if ( aDirName != _aDirName )
  {
    _aDirName = aDirName ;
    Chgt = true ;
  }
  QString aMeshFile=LEMeshFile->text().trimmed();
  if ( aMeshFile != _aMeshFile )
  {
    _aMeshFile = aMeshFile ;
    Chgt = true ;
  }

  // 2. Creation de l'objet CORBA
  try
  {
    _Name=LEName->text().trimmed();
    aYACS=myHomardGen->CreateYACSSchema(CORBA::string_dup(_Name.toStdString().c_str()), CORBA::string_dup(_aCaseName.toStdString().c_str()), CORBA::string_dup(_aScriptFile.toStdString().c_str()), CORBA::string_dup(_aDirName.toStdString().c_str()), CORBA::string_dup(_aMeshFile.toStdString().c_str()));
  }
  catch( SALOME::SALOME_Exception& S_ex )
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr(CORBA::string_dup(S_ex.details.text)) );
    bOK = false;
  }

  // 3. Options
  if ( bOK )
  {
  // 3.1. Le type du schema
    aYACS->SetType(_Type) ;

  // 3.2. Les maximums
    _MaxIter = SpinBoxMaxIter->value() ;
    aYACS->SetMaxIter(_MaxIter) ;

    _MaxNode = SpinBoxMaxNode->value() ;
    aYACS->SetMaxNode(_MaxNode) ;

    _MaxElem = SpinBoxMaxElem->value() ;
    aYACS->SetMaxElem(_MaxElem) ;

  }

  // 4. Ecriture du fichier
  if ( bOK )
  {
    int codret = aYACS->Write() ;
    if ( codret != 0 ) { bOK = false ; }
  }

  return bOK;
}
// ---------------------------
void MonCreateYACS::PushOnOK()
// ---------------------------
{
  bool bOK = PushOnApply();
  if ( bOK ) this->close();
}
//------------------------------
void MonCreateYACS::PushOnHelp()
//-------------------------------
{
  std::string LanguageShort = myHomardGen->GetLanguageShort();
  HOMARD_UTILS::PushOnHelp(QString("yacs.html"), QString(""), QString(LanguageShort.c_str()));
}
// -------------------------------------------------
void MonCreateYACS::SetNewName()
// --------------------------------------------------
{

  HOMARD::listeYACSs_var MyObjects = myHomardGen->GetAllYACSsName();
  int num = 0; QString aName="";
  while (aName == QString("") )
  {
    aName.setNum(num+1) ;
    aName.insert(0, QString("YACS_")) ;
    for ( int i=0; i<MyObjects->length(); i++)
    {
      if ( aName ==  QString(MyObjects[i]))
      {
        num ++ ;
        aName = "" ;
        break ;
      }
   }
  }
  LEName->setText(aName);
}
// ------------------------------------------------------------------------
void MonCreateYACS::SetCaseName()
// ------------------------------------------------------------------------
{
  MESSAGE ("SetCaseName avec _aCaseName = " << _aCaseName.toStdString() );
  if (_aCaseName == QString(""))
  {
    _aCaseName=HOMARD_QT_COMMUN::SelectionArbreEtude(QString("CasHomard"), 1);
    if (_aCaseName == QString("")) { raise();return;};
  }
  LECaseName->setText(_aCaseName);
}
// ------------------------------------------------------------------------
void MonCreateYACS::SetDirName()
// ------------------------------------------------------------------------
{
  QString aDirName=QFileDialog::getExistingDirectory ();
  if (!(aDirName.isEmpty()))LEDirName->setText(aDirName);
}
// ------------------------------------------------------------------------
void MonCreateYACS::SetScriptFile()
// ------------------------------------------------------------------------
{
  QString fileName0 = LEScriptFile->text().trimmed();
  QString fileName = HOMARD_QT_COMMUN::PushNomFichier( false, QString("py") ) ;
  if (fileName.isEmpty()) fileName = fileName0 ;
  LEScriptFile->setText(fileName);
}
// ------------------------------------------------------------------------
void MonCreateYACS::SetMeshFile()
// ------------------------------------------------------------------------
{
  QString fileName0 = LEMeshFile->text().trimmed();
  QString fileName = HOMARD_QT_COMMUN::PushNomFichier( false, QString("med") ) ;
  if (fileName.isEmpty()) fileName = fileName0 ;
  LEMeshFile->setText(fileName);
}
// ------------------------------------------------------------------------
void MonCreateYACS::SetConstant()
// ------------------------------------------------------------------------
{
  _Type = 1 ;
}
// ------------------------------------------------------------------------
void MonCreateYACS::SetVariable()
// ------------------------------------------------------------------------
{
  _Type = 2 ;
}
