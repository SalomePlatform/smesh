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

#include "MonCreateListGroup.h"
#include "MonCreateHypothesis.h"
#include "MonCreateBoundaryDi.h"

#include <QFileDialog>

#include "SalomeApp_Tools.h"
#include "HOMARDGUI_Utils.h"
#include "HomardQtCommun.h"
#include <utilities.h>

#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_ViewManager.h>

using namespace std;

// --------------------------------------------------------------------------------------------------------------
MonCreateListGroup::MonCreateListGroup(MonCreateHypothesis* parentHyp, MonCreateBoundaryDi* parentBound, bool modal,
                                       HOMARD::HOMARD_Gen_var myHomardGen0, QString aCaseName,  QStringList listeGroupesHypo) :
// --------------------------------------------------------------------------------------------------------------
//
    QDialog(0), Ui_CreateListGroup(),
    _aCaseName (aCaseName),
    _listeGroupesHypo (listeGroupesHypo),
    _parentHyp(parentHyp),
    _parentBound(parentBound)
{
    MESSAGE("Debut de  MonCreateListGroup")
     myHomardGen=HOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
    setupUi(this);
    setModal(modal);
    InitConnect();
    InitGroupes();
}
// --------------------------------------------------------------------------------------------------------------
MonCreateListGroup::MonCreateListGroup(MonCreateHypothesis* parentHyp, MonCreateBoundaryDi* parentBound,
                                       HOMARD::HOMARD_Gen_var myHomardGen, QString aCaseName,  QStringList listeGroupesHypo) :
// --------------------------------------------------------------------------------------------------------------
//
    QDialog(0), Ui_CreateListGroup(),
    _aCaseName (aCaseName),
    _listeGroupesHypo (listeGroupesHypo),
    _parentHyp(parentHyp),
    _parentBound(parentBound)
{
    myHomardGen=HOMARD::HOMARD_Gen::_duplicate(myHomardGen);
    setupUi(this);
    InitConnect();
}

// ------------------------------------------------------------------------
MonCreateListGroup::~MonCreateListGroup()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void MonCreateListGroup::InitConnect()
// ------------------------------------------------------------------------
{
    connect( buttonOk,     SIGNAL( pressed() ), this, SLOT( PushOnOK() ) );
    connect( buttonApply,  SIGNAL( pressed() ), this, SLOT( PushOnApply() ) );
    connect( buttonCancel, SIGNAL( pressed() ), this, SLOT( close() ) );
    connect( buttonHelp,   SIGNAL( pressed() ), this, SLOT( PushOnHelp() ) );
}
// ------------------------------------------------------------------------
bool MonCreateListGroup::PushOnApply()
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
  if ( _parentHyp )   { _parentHyp->setGroups(ListeGroup);};
  if ( _parentBound ) { _parentBound->setGroups(ListeGroup);};
  return true;
}


// ------------------------------------------------------------------------
void MonCreateListGroup::PushOnOK()
// ------------------------------------------------------------------------
{
    if (PushOnApply())
    {
      this->close();
      if ( _parentHyp )   { _parentHyp->raise(); _parentHyp->activateWindow(); };
      if ( _parentBound ) { _parentBound->raise(); _parentBound->activateWindow(); };
    }
}
// ------------------------------------------------------------------------
void MonCreateListGroup::PushOnHelp()
// ------------------------------------------------------------------------
{
  std::string LanguageShort = myHomardGen->GetLanguageShort();
  HOMARD_UTILS::PushOnHelp(QString("gui_create_hypothese.html"), QString(""), QString(LanguageShort.c_str()));
}
// ------------------------------------------------------------------------
void MonCreateListGroup::InitGroupes()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de MonCreateListGroup::InitGroupes ");
  for ( int row=0; row< TWGroupe->rowCount(); row++)
      TWGroupe->removeRow(row);
  TWGroupe->setRowCount(0);
  if (_aCaseName == QString("")) { return; };
  HOMARD::HOMARD_Cas_var monCas= myHomardGen->GetCase(_aCaseName.toStdString().c_str());
  HOMARD::ListGroupType_var _listeGroupesCas = monCas->GetGroups();
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
//   MESSAGE("Fin de MonCreateListGroup::InitGroupes ");
}

