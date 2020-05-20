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

#include "MonEditYACS.h"

#include "SalomeApp_Tools.h"
#include "HOMARDGUI_Utils.h"
#include "HomardQtCommun.h"
#include <utilities.h>

using namespace std;

// -------------------------------------------------------------
/* Constructs a MonEditYACS
    herite de MonCreateYACS
*/
// -------------------------------------------------------------
MonEditYACS::MonEditYACS ( bool modal,
                           HOMARD::HOMARD_Gen_var myHomardGen,
                           QString Name ):
   MonCreateYACS(myHomardGen, Name)
{
    MESSAGE("Debut de MonEditYACS" << Name.toStdString().c_str());
    setWindowTitle(QObject::tr("HOM_YACS_EDIT_WINDOW_TITLE"));
    _Name = Name;
    aYACS = myHomardGen->GetYACS(_Name.toStdString().c_str());
    InitValEdit();
}
// ------------------------------
MonEditYACS::~MonEditYACS()
// ------------------------------
{
}
// ------------------------------
void MonEditYACS::InitValEdit()
// ------------------------------
{
  MESSAGE("InitValEdit");
  LEName->setText(_Name);
  LEName->setReadOnly(true);

  _aCaseName = aYACS->GetCaseName();
  LECaseName->setText(_aCaseName);
  LECaseName->setReadOnly(true);
  PBCaseName->setVisible(0);

  _aScriptFile = aYACS->GetScriptFile();
  LEScriptFile->setText(_aScriptFile);

  _aDirName = aYACS->GetDirName();
  LEDirName->setText(_aDirName);
  LEDirName->setReadOnly(true);

  _aMeshFile = aYACS->GetMeshFile();
  LEMeshFile->setText(_aMeshFile);

  _Type=aYACS->GetType();
  if(_Type==1) { RBConstant->setChecked(true); }
  else         { RBVariable->setChecked(true); };
  RBConstant->setEnabled(false);
  RBVariable->setEnabled(false);

  _MaxIter = aYACS->GetMaxIter();
  SpinBoxMaxIter->setValue(_MaxIter) ;
  _MaxNode = aYACS->GetMaxNode();
  SpinBoxMaxNode->setValue(_MaxNode) ;
  _MaxElem = aYACS->GetMaxElem();
  SpinBoxMaxElem->setValue(_MaxElem) ;
//
}

// ---------------------------------------------------
bool MonEditYACS:: CreateOrUpdate()
//----------------------------------------------------
//  Edition/modification du schema
{
  MESSAGE("CreateOrUpdate");
  bool chgt = false ;
  int valeur_i ;
  // A. Recuperation et comparaison des valeurs
  // A.1. Le script de lancement
  QString aScriptFile=LEScriptFile->text().trimmed();
  if ( aScriptFile != _aScriptFile )
  {
//     MESSAGE("modification de ScriptFile : "<<_aScriptFile.toStdString()<<" devient "<<aScriptFile.toStdString());
    _aScriptFile = aScriptFile ;
    aYACS->SetScriptFile(CORBA::string_dup(_aScriptFile.toStdString().c_str())) ;
    chgt = true ;
  }
  // A.2. Le maillage initial
  QString aMeshFile=LEMeshFile->text().trimmed();
  if ( aMeshFile != _aMeshFile )
  {
//     MESSAGE("modification de aMeshFile : "<<_aMeshFile.toStdString()<<" devient "<<aMeshFile.toStdString());
    _aMeshFile = aMeshFile ;
    aYACS->SetMeshFile(CORBA::string_dup(_aMeshFile.toStdString().c_str())) ;
    chgt = true ;
  }
  // A.3. La convergence
  valeur_i = SpinBoxMaxIter->value() ;
  if ( valeur_i != _MaxIter )
  {
//     MESSAGE("modification de MaxIter : "<<_MaxIter<<" devient "<<valeur_i);
    _MaxIter = valeur_i ;
    aYACS->SetMaxIter(_MaxIter) ;
    chgt = true ;
  }
  valeur_i = SpinBoxMaxNode->value() ;
  if ( valeur_i != _MaxNode )
  {
//     MESSAGE("modification de MaxNode : "<<_MaxNode<<" devient "<<valeur_i);
    _MaxNode = valeur_i ;
    aYACS->SetMaxNode(_MaxNode) ;
    chgt = true ;
  }
  valeur_i = SpinBoxMaxElem->value() ;
  if ( valeur_i != _MaxElem )
  {
//     MESSAGE("modification de MaxElem : "<<_MaxElem<<" devient "<<valeur_i);
    _MaxElem = valeur_i ;
    aYACS->SetMaxElem(_MaxElem) ;
    chgt = true ;
  }
// B. Si changement
  if ( chgt )
  {
    myHomardGen->InvalideYACS(_Name.toStdString().c_str());
  }

  HOMARD_UTILS::updateObjBrowser();
  return true ;
}
