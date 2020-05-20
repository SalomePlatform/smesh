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

#include "MonEditCase.h"

#include "SalomeApp_Tools.h"
#include "HOMARDGUI_Utils.h"
#include "HomardQtCommun.h"
#include <utilities.h>

using namespace std;

// -------------------------------------------------------------
/* Constructs a MonEditCase
    herite de MonCreateCase
*/
// -------------------------------------------------------------
MonEditCase::MonEditCase ( bool modal,
                           HOMARD::HOMARD_Gen_var myHomardGen,
                           QString CaseName ):
   MonCreateCase(modal, myHomardGen)
{
    MESSAGE("Debut de MonEditCase" << CaseName.toStdString().c_str());
    setWindowTitle(QObject::tr("HOM_CASE_EDIT_WINDOW_TITLE"));
    _aCaseName = CaseName;
    aCase = myHomardGen->GetCase(_aCaseName.toStdString().c_str());
    InitValEdit();
}
// ------------------------------
MonEditCase::~MonEditCase()
// ------------------------------
{
}
// ------------------------------
void MonEditCase::InitValEdit()
// ------------------------------
{
  MESSAGE("InitValEdit");
  LEName->setText(_aCaseName);
  LEName->setReadOnly(true);

  QString aDirName = aCase->GetDirName();
  LEDirName->setText(aDirName);
  LEDirName->setReadOnly(true);
  PushDir->setVisible(0);

  QString _aitername=aCase->GetIter0Name();
  HOMARD::HOMARD_Iteration_var aIter = myHomardGen->GetIteration(_aitername.toStdString().c_str());
  QString aFileName = aIter->GetMeshFile();
  LEFileName->setText(aFileName);
  LEFileName->setReadOnly(true);
  PushFichier->setVisible(0);

  int ConfType=aCase->GetConfType();
  if ( ( ConfType == 0 ) || ( ConfType == -1 ) ) { RBConforme->setChecked(true); }
  else                                           { RBNonConforme->setChecked(true); };
  RBConforme->setEnabled(false);
  RBNonConforme->setEnabled(false);
  int ExtType=aCase->GetExtType();


// Suivi de frontiere
// A priori, aucun affichage
  GBTypeBoun->setVisible(0);
  CBBoundaryD->setVisible(0);
  CBBoundaryA->setVisible(0);
  GBBoundaryC->setVisible(0);
  GBBoundaryD->setVisible(0);
  GBBoundaryA->setVisible(0);
//    On passe en revue tous les couples (frontiere,groupe) du cas
  HOMARD::ListBoundaryGroupType_var mesBoundarys = aCase->GetBoundaryGroup();
  if (mesBoundarys->length()>0)
  {
    QStringList ListeFron ;
    QString NomFron ;
    bool BounCAO = false ;
    bool BounDi = false ;
    bool BounAn = false ;
    for (int i=0; i<mesBoundarys->length(); i++)
    {
//    Nom de la frontiere
      NomFron = mesBoundarys[i++];
      MESSAGE("NomFron "<<NomFron.toStdString().c_str());
//    L'objet associe pour en deduire le type
      HOMARD::HOMARD_Boundary_var myBoundary = myHomardGen->GetBoundary(NomFron.toStdString().c_str());
      int type_obj = myBoundary->GetType() ;
      MESSAGE("type_obj "<<type_obj);
//        C'est une frontiere CAO
//        Remarque : on ne gere pas les groupes
      if ( type_obj==-1 )
      {
        BounCAO = true ;
        CBBoundaryCAO->addItem(NomFron);
      }
//        C'est une frontiere discrete
//        Rermarque : on ne gere pas les groupes
      else if ( type_obj==0 )
      {
        BounDi = true ;
        CBBoundaryDi->addItem(NomFron);
      }
//        C'est une frontiere analytique
      else
      {
        BounAn = true ;
        int nbcol = TWBoundary->columnCount();
//          On ajoute une ligne pour le groupe
        TWBoundary->insertRow(0);
//          La colonne 0 comporte le nom du groupe
        TWBoundary->setItem( 0, 0, new QTableWidgetItem(QString(mesBoundarys[i]).trimmed()));
//             TWBoundary->item( 0, 0 )->setFlags(Qt::ItemIsEnabled |Qt::ItemIsSelectable );
//          Chacune des colonnes suivantes est associé a une frontiere deja presente : on y met une
//          case non cochee
        for ( int j = 1; j < nbcol; j++ )
        {
          TWBoundary->setItem( 0, j, new QTableWidgetItem( QString ("") ) );
          TWBoundary->item( 0, j )->setFlags( 0 );
          TWBoundary->item( 0, j )->setFlags( Qt::ItemIsUserCheckable  );
          TWBoundary->item( 0, j )->setCheckState( Qt::Unchecked );
        }
//          On cherche si la frontiere en cours d'examen a deja ete rencontree :
//          si oui, on stocke son numero de colonne
        int ok = -1 ;
        for ( int nufr = 0 ; nufr<ListeFron.size(); nufr++)
        { if ( ListeFron[nufr] == NomFron ) ok = nufr+1 ; }
//            si non, on ajoute une colonne
        if ( ok < 0 )
        {
          ListeFron.append(NomFron);
          ok = ListeFron.size() ;
          AddBoundaryAn(NomFron);
        }
//          on coche la case correspondant au couple (frontiere,groupe) en cours d'examen
        TWBoundary->item( 0, ok )->setCheckState( Qt::Checked );
      }
    }
    MESSAGE("BounCAO "<<BounCAO<<",BounDi "<<BounDi<<", BounAn "<<BounAn);
    GBTypeBoun->setVisible(1);
    if ( BounCAO )
    { RBBoundaryCAO->setChecked(true);
      GBBoundaryC->setVisible(1);
      CBBoundaryCAO->setDisabled(true);
      PBBoundaryCAONew->setVisible(0);
      PBBoundaryCAOHelp->setVisible(0); }
    if ( BounDi )
    { RBBoundaryNonCAO->setChecked(true);
      GBBoundaryD->setVisible(1);
      CBBoundaryDi->setDisabled(true);
      PBBoundaryDiNew->setVisible(0);
      PBBoundaryDiHelp->setVisible(0); }
    if ( BounAn )
    { RBBoundaryNonCAO->setChecked(true);
      GBBoundaryA->setVisible(1);
//    On rend les cases non modifiables.
//    On ne peut pas le faire pour tout le tableau sinon on perd l'ascenseur !
      int nbcol = TWBoundary->columnCount();
      int nbrow = TWBoundary->rowCount();
      for ( int i = 0; i < nbrow; i++ )
      { for ( int j = 0; j < nbcol; j++ ) TWBoundary->item( i, j )->setFlags( Qt::ItemIsSelectable ); }
//    on met un nom blanc au coin
      QTableWidgetItem *__colItem = new QTableWidgetItem();
      __colItem->setText(QApplication::translate("CreateCase", "", 0));
      TWBoundary->setHorizontalHeaderItem(0, __colItem);
//    on cache les boutons inutiles
      PBBoundaryAnNew->setVisible(0);
      PBBoundaryAnHelp->setVisible(0);
    }
    RBBoundaryNo->setEnabled(false);
    RBBoundaryCAO->setEnabled(false);
    RBBoundaryNonCAO->setEnabled(false);
  }
//
// Les options avancees (non modifiables)
  CBAdvanced->setVisible(0) ;
  CBAdvanced->setEnabled(false) ;
  int Pyram = aCase->GetPyram();
  MESSAGE("Pyram "<<Pyram);
  if ( ( Pyram > 0 ) || ( ConfType < 0 ) || ( ConfType > 1 ) || ( ExtType > 0 ) )
  { GBAdvancedOptions->setVisible(1);
//
    if ( Pyram > 0 )
    { CBPyramid->setChecked(true);
      CBPyramid->setVisible(1);
    }
    else
    { CBPyramid->setChecked(false);
      CBPyramid->setVisible(0);
    }
    CBPyramid->setEnabled(false);
//
    if ( ( ConfType == 0 ) || ( ConfType == -1 ) )
    { if ( ConfType == 0 ) { RBStandard->setChecked(true); }
      else                 { RBBox->setChecked(true); }
      RBStandard->setVisible(1);
      RBBox->setVisible(1);
      RBNC1NpA->setVisible(0);
      RBNCQuelconque->setVisible(0);
    }
    else
    { if (ConfType==-2) { RBBox->setChecked(true);};
      if (ConfType==1) { RBStandard->setChecked(true);};
      if (ConfType==2) { RBNC1NpA->setChecked(true);};
      if (ConfType==3) { RBNCQuelconque->setChecked(true);};
      RBStandard->setVisible(1);
      RBBox->setVisible(1);
      RBNC1NpA->setVisible(1);
      RBNCQuelconque->setVisible(1);
    }
    RBStandard->setEnabled(false);
    RBBox->setEnabled(false);
    RBNC1NpA->setEnabled(false);
    RBNCQuelconque->setEnabled(false);
//
    if ( ExtType == 0 )
    { GBFormat->setVisible(0);
      RBMED->setChecked(true);
    }
    else
    { GBFormat->setVisible(1);
      RBMED->setVisible(1);
      RBSaturne->setVisible(1);
      RBSaturne2D->setVisible(1);
      if ( ExtType == 1 ) { RBSaturne->setChecked(true); }
      else                { RBSaturne2D->setChecked(true); }
    }
    RBMED->setEnabled(false);
    RBSaturne->setEnabled(false);
    RBSaturne2D->setEnabled(false);
  }
  else
  { GBAdvancedOptions->setVisible(0);
    CBPyramid->setChecked(false);
    RBStandard->setChecked(true);
    RBMED->setChecked(true);
  }
//
// L'etat
  int etat = aCase->GetState();
  MESSAGE("etat "<<etat);
  if ( etat == 0 ) { Comment->setText(QApplication::translate("CreateCase", "HOM_CASE_EDIT_STATE_0", 0)); }
  else             { Comment->setText(QApplication::translate("CreateCase", "HOM_CASE_EDIT_STATE", 0)); }

  Comment->setVisible(1);
//
  adjustSize();
}

// -------------------------------------
bool MonEditCase::PushOnApply()
// -------------------------------------
{
  return true ;
};
