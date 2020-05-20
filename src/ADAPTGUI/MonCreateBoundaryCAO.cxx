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

#include "MonCreateBoundaryCAO.h"
#include "MonCreateListGroupCAO.h"
#include "MonCreateCase.h"

#include <QFileDialog>
#include <QMessageBox>

#include "SalomeApp_Tools.h"
#include "HOMARDGUI_Utils.h"
#include "HomardQtCommun.h"
#include <utilities.h>

using namespace std;

// -------------------------------------------------------------------------------
MonCreateBoundaryCAO::MonCreateBoundaryCAO(MonCreateCase* parent, bool modal,
                                         HOMARD::HOMARD_Gen_var myHomardGen0,
                                         QString caseName, QString aName)
// ---------------------------------------------------------------------------------
/* Constructs a MonCreateBoundaryCAO */
    :
    QDialog(0), Ui_CreateBoundaryCAO(),
    _parent(parent), _aName(aName),
    myHomardGen(HOMARD::HOMARD_Gen::_duplicate(myHomardGen0)),
    _aCaseName(caseName)
    {
      MESSAGE("Constructeur") ;
      setupUi(this);
      setModal(modal);
      InitConnect();

     if ( _aName == QString("") ) {SetNewName();};
    }

// ------------------------------------------------------------------------
MonCreateBoundaryCAO::~MonCreateBoundaryCAO()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void MonCreateBoundaryCAO::InitConnect()
// ------------------------------------------------------------------------
{
    connect( PushFichier,  SIGNAL(pressed()), this, SLOT(SetCAOFile()));
    connect( buttonOk,     SIGNAL(pressed()), this, SLOT( PushOnOK()));
    connect( buttonApply,  SIGNAL(pressed()), this, SLOT( PushOnApply()));
    connect( buttonCancel, SIGNAL(pressed()), this, SLOT(close()));
    connect( buttonHelp,   SIGNAL(pressed()), this, SLOT( PushOnHelp()));
    connect( CBGroupe,     SIGNAL(stateChanged(int)), this, SLOT( SetFiltrage()));
}

// ------------------------------------------------------------------------
bool MonCreateBoundaryCAO::PushOnApply()
// ------------------------------------------------------------------------
// Appele lorsque l'un des boutons Ok ou Apply est presse
//
{
// Verifications

  QString aName=LEName->text().trimmed();
  if (aName=="") {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_BOUN_NAME") );
    return false;
  }

//  La CAO
  QString aCAOFile=LEFileName->text().trimmed();
  if (aCAOFile ==QString(""))
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_BOUN_CAO") );
    return false;
  }

// Creation de l'objet CORBA si ce n'est pas deja fait sous le meme nom
  if ( _aName != aName )
  {
   try
   {
     _aName=aName;
     aBoundary=myHomardGen->CreateBoundaryCAO(CORBA::string_dup(_aName.toStdString().c_str()), aCAOFile.toStdString().c_str());
     _parent->AddBoundaryCAO(_aName);
     aBoundary->SetCaseCreation(_aCaseName.toStdString().c_str());
   }
   catch( SALOME::SALOME_Exception& S_ex )
   {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr(CORBA::string_dup(S_ex.details.text)) );
      return false;
   }
  }

// Les groupes
  AssocieLesGroupes();

  HOMARD_UTILS::updateObjBrowser();
  return true;
}


// ------------------------------------------------------------------------
void MonCreateBoundaryCAO::PushOnOK()
// ------------------------------------------------------------------------
{
     if (PushOnApply()) this->close();
     if ( _parent ) { _parent->raise(); _parent->activateWindow(); };
}
// ------------------------------------------------------------------------
void MonCreateBoundaryCAO::PushOnHelp()
// ------------------------------------------------------------------------
{
  std::string LanguageShort = myHomardGen->GetLanguageShort();
  HOMARD_UTILS::PushOnHelp(QString("gui_create_boundary.html"), QString("CAO"), QString(LanguageShort.c_str()));
}
// ------------------------------------------------------------------------
void MonCreateBoundaryCAO::AssocieLesGroupes()
// ------------------------------------------------------------------------
{
  HOMARD::ListGroupType_var aSeqGroupe = new HOMARD::ListGroupType;
  aSeqGroupe->length(_listeGroupesBoundary.size());
  QStringList::const_iterator it;
  int i=0;
  for (it = _listeGroupesBoundary.constBegin(); it != _listeGroupesBoundary.constEnd(); it++)
     aSeqGroupe[i++]=(*it).toStdString().c_str();
  aBoundary->SetGroups(aSeqGroupe);

}

// -------------------------------------------------
void MonCreateBoundaryCAO::SetNewName()
// --------------------------------------------------
{

  HOMARD::listeBoundarys_var  MyObjects = myHomardGen->GetAllBoundarysName();
  int num = 0; QString aName="";
  while (aName == QString("") )
  {
    aName.setNum(num+1) ;
    aName.insert(0, QString("Boun_")) ;
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
void MonCreateBoundaryCAO::SetCAOFile()
// ------------------------------------------------------------------------
{
  QString aCAOFile = HOMARD_QT_COMMUN::PushNomFichier( false, QString("xao") );
  if (!(aCAOFile.isEmpty())) LEFileName->setText(aCAOFile);
}

// ------------------------------------------------------------------------
void MonCreateBoundaryCAO::setGroups (QStringList listGroup)
// ------------------------------------------------------------------------
{
    _listeGroupesBoundary = listGroup;
}
// ------------------------------------------------------------------------
void MonCreateBoundaryCAO::SetFiltrage()
// // ------------------------------------------------------------------------
{
  if (!CBGroupe->isChecked()) return;
  if (_aCaseName.toStdString().c_str() == QString()) {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_BOUN_CASE") );
    return;
  }

  MonCreateListGroupCAO *aDlg = new MonCreateListGroupCAO(NULL, this, true, HOMARD::HOMARD_Gen::_duplicate(myHomardGen),
                            _aCaseName, _listeGroupesBoundary) ;
  aDlg->show();
}

