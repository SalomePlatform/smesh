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

#include "MonEditListGroupCAO.h"
#include <utilities.h>

using namespace std;
//---------------------------------------------------------------------
MonEditListGroupCAO::MonEditListGroupCAO( MonCreateHypothesis* parentHyp,
                                    MonCreateBoundaryCAO* parentBound,
                                    bool modal,
                                    HOMARD::HOMARD_Gen_var myHomardGen,
                                    QString aCaseName,
                                    QStringList listeGroupesHypo):
//---------------------------------------------------------------------
MonCreateListGroupCAO(parentHyp,parentBound,myHomardGen,aCaseName,listeGroupesHypo)
{
  MESSAGE("Debut de MonEditListGroupCAO");
    setWindowTitle(QObject::tr("HOM_GROU_EDIT_WINDOW_TITLE"));
  setModal(true);
  InitGroupes();
}

//------------------------------------
MonEditListGroupCAO:: ~MonEditListGroupCAO()
//------------------------------------
{
}
// -------------------------------------
void MonEditListGroupCAO:: InitGroupes()
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


