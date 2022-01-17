// Copyright (C) 2011-2021  CEA/DEN, EDF R&D
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

#include "SMESHGUI_HomardListGroup.h"

#include "SMESHGUI_HomardBoundaryDlg.h"
#include "SMESHGUI_HomardUtils.h"
#include "SMESHGUI_Utils.h"

#include "SalomeApp_Tools.h"

#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_ViewManager.h>

#include <utilities.h>

#include <QFileDialog>

using namespace std;

// ------------------------------------------------------------------------------------
SMESH_CreateListGroupCAO::SMESH_CreateListGroupCAO(SMESH_CreateBoundaryCAO* parentBound,
                                                   bool modal,
                                                   SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                                   QString aCaseName,
                                                   QStringList listeGroupesHypo)
  : QDialog(0), SMESH_Ui_CreateListGroup(),
    _aCaseName (aCaseName),
    _listeGroupesHypo (listeGroupesHypo),
    _parentBound(parentBound)
{
  MESSAGE("Debut de SMESH_CreateListGroupCAO");
  myHomardGen = SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
  setupUi(this);
  setModal(modal);
  InitConnect();
  InitGroupes();
}
// ------------------------------------------------------------------------------------
SMESH_CreateListGroupCAO::SMESH_CreateListGroupCAO(SMESH_CreateBoundaryCAO* parentBound,
                                                   SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                                   QString aCaseName,
                                                   QStringList listeGroupesHypo)
  : QDialog(0), SMESH_Ui_CreateListGroup(),
    _aCaseName (aCaseName),
    _listeGroupesHypo (listeGroupesHypo),
    _parentBound(parentBound)
{
    myHomardGen = SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
    setupUi(this);
    InitConnect();
}

// ------------------------------------------------------------------------
SMESH_CreateListGroupCAO::~SMESH_CreateListGroupCAO()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void SMESH_CreateListGroupCAO::InitConnect()
// ------------------------------------------------------------------------
{
    connect( buttonOk,     SIGNAL( pressed() ), this, SLOT( PushOnOK() ) );
    connect( buttonApply,  SIGNAL( pressed() ), this, SLOT( PushOnApply() ) );
    connect( buttonCancel, SIGNAL( pressed() ), this, SLOT( close() ) );
    connect( buttonHelp,   SIGNAL( pressed() ), this, SLOT( PushOnHelp() ) );
}
// ------------------------------------------------------------------------
bool SMESH_CreateListGroupCAO::PushOnApply()
// ------------------------------------------------------------------------
// Appele lorsque l'un des boutons Ok ou Apply est presse
//
{
  QStringList ListeGroup ;
  for ( int row=0; row< TWGroupe->rowCount(); row++)
  {
      if ( TWGroupe->item( row, 0 )->checkState() ==  Qt::Checked )
          ListeGroup.insert(0, QString(TWGroupe->item(row, 1)->text()) );
  }
  if ( _parentBound ) { _parentBound->setGroups(ListeGroup);};
  return true;
}


// ------------------------------------------------------------------------
void SMESH_CreateListGroupCAO::PushOnOK()
// ------------------------------------------------------------------------
{
     if (PushOnApply()) this->close();
     if ( _parentBound ) { _parentBound->raise(); _parentBound->activateWindow(); };
}
// ------------------------------------------------------------------------
void SMESH_CreateListGroupCAO::PushOnHelp()
// ------------------------------------------------------------------------
{
  SMESH::ShowHelpFile(QString("homard_create_boundary.html#filtering-by-the-groups"));
}
// ------------------------------------------------------------------------
void SMESH_CreateListGroupCAO::InitGroupes()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SMESH_CreateListGroupCAO::InitGroupes ");
  for ( int row=0; row< TWGroupe->rowCount(); row++)
      TWGroupe->removeRow(row);
  TWGroupe->setRowCount(0);
  if (_aCaseName == QString("")) { return; };
  SMESHHOMARD::HOMARD_Cas_var monCas = myHomardGen->GetCase();
  SMESHHOMARD::ListGroupType_var _listeGroupesCas = monCas->GetGroups();
  for ( int i = 0; i < _listeGroupesCas->length(); i++ ) {
    TWGroupe->insertRow(i);
    TWGroupe->setItem( i, 0, new QTableWidgetItem( QString ("") ) );
    TWGroupe->item( i, 0 )->setFlags( 0 );
    TWGroupe->item( i, 0 )->setFlags( Qt::ItemIsUserCheckable|Qt::ItemIsEnabled  );
    if (_listeGroupesHypo.contains (QString((_listeGroupesCas)[i])))
      {TWGroupe->item( i, 0 )->setCheckState( Qt::Checked );}
    else
      {TWGroupe->item( i, 0 )->setCheckState( Qt::Unchecked );}
    TWGroupe->setItem( i, 1, new QTableWidgetItem(QString((_listeGroupesCas)[i]).trimmed()));
    TWGroupe->item( i, 1 )->setFlags(Qt::ItemIsEnabled |Qt::ItemIsSelectable );
  }
  TWGroupe->resizeColumnsToContents();
  TWGroupe->resizeRowsToContents();
  TWGroupe->clearSelection();
//   MESSAGE("Fin de SMESH_CreateListGroupCAO::InitGroupes ");
}


// ------------------------------------------------------------------------
SMESH_CreateListGroup::SMESH_CreateListGroup(SMESH_CreateBoundaryDi* parentBound, bool modal,
                                             SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                             QString aCaseName,  QStringList listeGroupesHypo)
  : QDialog(0), SMESH_Ui_CreateListGroup(),
    _aCaseName (aCaseName),
    _listeGroupesHypo (listeGroupesHypo),
    _parentBound(parentBound)
{
  MESSAGE("Debut de SMESH_CreateListGroup");
  myHomardGen = SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
  setupUi(this);
  setModal(modal);
  InitConnect();
  InitGroupes();
}

// ------------------------------------------------------------------------
SMESH_CreateListGroup::SMESH_CreateListGroup(SMESH_CreateBoundaryDi* parentBound,
                                             SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                             QString aCaseName,  QStringList listeGroupesHypo)
  : QDialog(0), SMESH_Ui_CreateListGroup(),
    _aCaseName (aCaseName),
    _listeGroupesHypo (listeGroupesHypo),
    _parentBound(parentBound)
{
  myHomardGen = SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
  setupUi(this);
  InitConnect();
}

// ------------------------------------------------------------------------
SMESH_CreateListGroup::~SMESH_CreateListGroup()
{
  // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void SMESH_CreateListGroup::InitConnect()
// ------------------------------------------------------------------------
{
    connect( buttonOk,     SIGNAL( pressed() ), this, SLOT( PushOnOK() ) );
    connect( buttonApply,  SIGNAL( pressed() ), this, SLOT( PushOnApply() ) );
    connect( buttonCancel, SIGNAL( pressed() ), this, SLOT( close() ) );
    connect( buttonHelp,   SIGNAL( pressed() ), this, SLOT( PushOnHelp() ) );
}
// ------------------------------------------------------------------------
bool SMESH_CreateListGroup::PushOnApply()
// ------------------------------------------------------------------------
// Appele lorsque l'un des boutons Ok ou Apply est presse
//
{
  QStringList ListeGroup ;
  for ( int row=0; row< TWGroupe->rowCount(); row++)
  {
      if ( TWGroupe->item( row, 0 )->checkState() ==  Qt::Checked )
          ListeGroup.insert(0, QString(TWGroupe->item(row, 1)->text()) );
  }
  if ( _parentBound ) { _parentBound->setGroups(ListeGroup);};
  return true;
}


// ------------------------------------------------------------------------
void SMESH_CreateListGroup::PushOnOK()
// ------------------------------------------------------------------------
{
    if (PushOnApply())
    {
      this->close();
      if ( _parentBound ) { _parentBound->raise(); _parentBound->activateWindow(); };
    }
}
// ------------------------------------------------------------------------
void SMESH_CreateListGroup::PushOnHelp()
// ------------------------------------------------------------------------
{
  SMESH::ShowHelpFile(QString("homard_create_boundary.html#filtering-by-the-groups"));
}
// ------------------------------------------------------------------------
void SMESH_CreateListGroup::InitGroupes()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SMESH_CreateListGroup::InitGroupes ");
  for ( int row=0; row< TWGroupe->rowCount(); row++)
      TWGroupe->removeRow(row);
  TWGroupe->setRowCount(0);
  if (_aCaseName == QString("")) { return; };
  SMESHHOMARD::HOMARD_Cas_var monCas = myHomardGen->GetCase();
  SMESHHOMARD::ListGroupType_var _listeGroupesCas = monCas->GetGroups();
  for ( int i = 0; i < _listeGroupesCas->length(); i++ )
  {
    TWGroupe->insertRow(i);
    TWGroupe->setItem( i, 0, new QTableWidgetItem( QString ("") ) );
    TWGroupe->item( i, 0 )->setFlags( 0 );
    TWGroupe->item( i, 0 )->setFlags( Qt::ItemIsUserCheckable|Qt::ItemIsEnabled  );
    if (_listeGroupesHypo.contains (QString((_listeGroupesCas)[i])))
      {TWGroupe->item( i, 0 )->setCheckState( Qt::Checked );}
    else
      {TWGroupe->item( i, 0 )->setCheckState( Qt::Unchecked );}
    TWGroupe->setItem( i, 1, new QTableWidgetItem(QString((_listeGroupesCas)[i]).trimmed()));
    TWGroupe->item( i, 1 )->setFlags(Qt::ItemIsEnabled |Qt::ItemIsSelectable );
  }
  TWGroupe->resizeColumnsToContents();
  TWGroupe->resizeRowsToContents();
  TWGroupe->clearSelection();
//   MESSAGE("Fin de SMESH_CreateListGroup::InitGroupes ");
}

//---------------------------------------------------------------------
SMESH_EditListGroupCAO::SMESH_EditListGroupCAO( SMESH_CreateBoundaryCAO* parentBound,
                                                bool modal,
                                                SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                                QString aCaseName,
                                                QStringList listeGroupesHypo):
//---------------------------------------------------------------------
SMESH_CreateListGroupCAO(parentBound,myHomardGen0,aCaseName,listeGroupesHypo)
{
  MESSAGE("Debut de SMESH_EditListGroupCAO");
    setWindowTitle(QObject::tr("HOM_GROU_EDIT_WINDOW_TITLE"));
  setModal(true);
  InitGroupes();
}

//------------------------------------
SMESH_EditListGroupCAO:: ~SMESH_EditListGroupCAO()
//------------------------------------
{
}
// -------------------------------------
void SMESH_EditListGroupCAO:: InitGroupes()
// -------------------------------------
{
  for (int i = 0; i < _listeGroupesHypo.size(); i++ )
  {
     std::cerr << _listeGroupesHypo[i].toStdString().c_str() << std::endl;
     TWGroupe->insertRow(i);
     TWGroupe->setItem( i, 0, new QTableWidgetItem( QString ("") ) );
     TWGroupe->item( i, 0 )->setFlags( 0 );
     TWGroupe->item( i, 0 )->setCheckState( Qt::Checked );
     TWGroupe->setItem( i, 1, new QTableWidgetItem(_listeGroupesHypo[i]));
   }
   TWGroupe->resizeRowsToContents();
}

//---------------------------------------------------------------------
SMESH_EditListGroup::SMESH_EditListGroup( SMESH_CreateBoundaryDi* parentBound,
                                          bool modal,
                                          SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                          QString aCaseName,
                                          QStringList listeGroupesHypo):
//---------------------------------------------------------------------
SMESH_CreateListGroup(parentBound,myHomardGen0,aCaseName,listeGroupesHypo)
{
  MESSAGE("Debut de SMESH_EditListGroup");
    setWindowTitle(QObject::tr("HOM_GROU_EDIT_WINDOW_TITLE"));
  setModal(true);
  InitGroupes();
}

//------------------------------------
SMESH_EditListGroup::~SMESH_EditListGroup()
//------------------------------------
{
}
// -------------------------------------
void SMESH_EditListGroup::InitGroupes()
// -------------------------------------
{
  for (int i = 0; i < _listeGroupesHypo.size(); i++ )
  {
     std::cerr << _listeGroupesHypo[i].toStdString().c_str() << std::endl;
     TWGroupe->insertRow(i);
     TWGroupe->setItem( i, 0, new QTableWidgetItem( QString ("") ) );
     TWGroupe->item( i, 0 )->setFlags( 0 );
     TWGroupe->item( i, 0 )->setCheckState( Qt::Checked );
     TWGroupe->setItem( i, 1, new QTableWidgetItem(_listeGroupesHypo[i]));
   }
   TWGroupe->resizeRowsToContents();
}
