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

#include "MonEditBoundaryDi.h"
#include "MonEditListGroup.h"

#include <QMessageBox>

#include "SalomeApp_Tools.h"
#include "HOMARDGUI_Utils.h"
#include "HomardQtCommun.h"
#include <utilities.h>

using namespace std;

// -------------------------------------------------------------------------------------------------------------------------------------
MonEditBoundaryDi::MonEditBoundaryDi( MonCreateCase* parent, bool modal,
                                      ADAPT::ADAPT_Gen_var myAdaptGen,
                                      QString caseName, QString Name):
// -------------------------------------------------------------------------------------------------------------------------------------
/* Constructs a MonEditBoundaryDi
    herite de MonCreateBoundaryDi
*/
    MonCreateBoundaryDi(parent, modal, myAdaptGen, caseName, Name)
{
    MESSAGE("Debut de Boundary pour " << Name.toStdString().c_str());
    setWindowTitle(QObject::tr("HOM_BOUN_D_EDIT_WINDOW_TITLE"));
    try
    {
     aBoundary=myAdaptGen->GetBoundary(CORBA::string_dup(_aName.toStdString().c_str()));
     if (caseName==QString("")) { _aCaseName=aBoundary->GetCaseCreation();}
     InitValEdit();
    }
    catch( SALOME::SALOME_Exception& S_ex )
    {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr(CORBA::string_dup(S_ex.details.text)) );
      return;
    }

    ADAPT::ListGroupType_var maListe = aBoundary->GetGroups();
    for ( int i = 0; i < maListe->length(); i++ )
       _listeGroupesBoundary << QString(maListe[i]);

}
// ------------------------------
MonEditBoundaryDi::~MonEditBoundaryDi()
// ------------------------------
{
}
// ------------------------------
void MonEditBoundaryDi::InitValEdit()
// ------------------------------
{
      LEName->setText(_aName);
      LEName->setReadOnly(true);

      QString aDataFile = aBoundary->GetDataFile();
      LEFileName->setText(aDataFile);
      LEFileName->setReadOnly(1);
      PushFichier->setVisible(0);
//
      adjustSize();
}
// ------------------------------
bool MonEditBoundaryDi::PushOnApply()
// ------------------------------
{
     return true;
}
// ------------------------------------------------------------------------
void MonEditBoundaryDi::SetFiltrage()
// // ------------------------------------------------------------------------
{
  if (!CBGroupe->isChecked()) return;
  if (_aCaseName.toStdString().c_str() == QString())
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_BOUN_CASE") );
    return;
  }
  ADAPT::HOMARD_Cas_var monCas= myAdaptGen->GetCase(_aCaseName.toStdString().c_str());
  ADAPT::ListGroupType_var _listeGroupesCas = monCas->GetGroups();

  MonEditListGroup *aDlg = new MonEditListGroup(NULL, this, true, ADAPT::ADAPT_Gen::_duplicate(myAdaptGen),
                            _aCaseName, _listeGroupesBoundary) ;
  aDlg->show();
}

