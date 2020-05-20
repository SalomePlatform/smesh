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

#include "MonCreateCase.h"
#include "MonCreateBoundaryCAO.h"
#include "MonEditBoundaryCAO.h"
#include "MonCreateBoundaryAn.h"
#include "MonEditBoundaryAn.h"
#include "MonCreateBoundaryDi.h"
#include "MonEditBoundaryDi.h"
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
/* Constructs a MonCreateCase
 * Inherits from CasHomard
 * Sets attributes to default values
 */
// -----------------------------------------------------------------------------------------
MonCreateCase::MonCreateCase( bool modal, HOMARD::HOMARD_Gen_var myHomardGen0 )
    :
    Ui_CreateCase(),
    _aCaseName(""),_aDirName(""),
    _ConfType(0),
    _ExtType(0),
    _Pyram(0)
{
  MESSAGE("Debut du constructeur de MonCreateCase");
  myHomardGen=HOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
  setupUi(this);
  setModal(modal);
  InitConnect();

  SetNewName() ;

  GBBoundaryC->setVisible(0);
  GBBoundaryA->setVisible(0);
  GBBoundaryD->setVisible(0);

  CBBoundaryA->setVisible(0);
  CBBoundaryD->setVisible(0);

  GBAdvancedOptions->setVisible(0);
  Comment->setVisible(0);
  CBPyramid->setChecked(false);
//
  adjustSize();

//   MESSAGE("Fin du constructeur de MonCreateCase");
}
// ------------------------------------------------------------------------
MonCreateCase::~MonCreateCase()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void MonCreateCase::InitConnect()
// ------------------------------------------------------------------------
{
    connect( LEName,      SIGNAL(textChanged(QString)), this, SLOT(CaseNameChanged()));
    connect( PushDir,     SIGNAL(pressed()), this, SLOT(SetDirName()));
    connect( PushFichier, SIGNAL(pressed()), this, SLOT(SetFileName()));

    connect( RBConforme,     SIGNAL(clicked()), this, SLOT(SetConforme()));
    connect( RBNonConforme,  SIGNAL(clicked()), this, SLOT(SetNonConforme()));

    connect( RBBoundaryNo,      SIGNAL(clicked()), this, SLOT(SetBoundaryNo()));
    connect( RBBoundaryCAO,     SIGNAL(clicked()), this, SLOT(SetBoundaryCAO()));
    connect( RBBoundaryNonCAO,  SIGNAL(clicked()), this, SLOT(SetBoundaryNonCAO()));

    connect( PBBoundaryCAONew,  SIGNAL(pressed()), this, SLOT(PushBoundaryCAONew()));
    connect( PBBoundaryCAOEdit, SIGNAL(pressed()), this, SLOT(PushBoundaryCAOEdit()) );
    connect( PBBoundaryCAOHelp, SIGNAL(pressed()), this, SLOT(PushBoundaryCAOHelp()) );
    connect( CBBoundaryD,       SIGNAL(stateChanged(int)), this, SLOT(SetBoundaryD()));
    connect( PBBoundaryDiNew,   SIGNAL(pressed()), this, SLOT(PushBoundaryDiNew()));
    connect( PBBoundaryDiEdit,  SIGNAL(pressed()), this, SLOT(PushBoundaryDiEdit()) );
    connect( PBBoundaryDiHelp,  SIGNAL(pressed()), this, SLOT(PushBoundaryDiHelp()) );
    connect( CBBoundaryA,       SIGNAL(stateChanged(int)), this, SLOT(SetBoundaryA()));
    connect( PBBoundaryAnNew,   SIGNAL(pressed()), this, SLOT(PushBoundaryAnNew()));
    connect( PBBoundaryAnEdit,  SIGNAL(pressed()), this, SLOT(PushBoundaryAnEdit()) );
    connect( PBBoundaryAnHelp,  SIGNAL(pressed()), this, SLOT(PushBoundaryAnHelp()) );

    connect( CBAdvanced,     SIGNAL(stateChanged(int)), this, SLOT(SetAdvanced()));
    connect( RBStandard,     SIGNAL(clicked()), this, SLOT(SetStandard()));
    connect( RBBox,          SIGNAL(clicked()), this, SLOT(SetBox()));
    connect( RBNC1NpA,       SIGNAL(clicked()), this, SLOT(SetNC1NpA()));
    connect( RBNCQuelconque, SIGNAL(clicked()), this, SLOT(SetNCQuelconque()));

    connect( RBMED,          SIGNAL(clicked()), this, SLOT(SetMED()));
    connect( RBSaturne,      SIGNAL(clicked()), this, SLOT(SetSaturne()));
    connect( RBSaturne2D,    SIGNAL(clicked()), this, SLOT(SetSaturne2D()));

    connect( buttonOk,       SIGNAL(pressed()), this, SLOT(PushOnOK()));
    connect( buttonApply,    SIGNAL(pressed()), this, SLOT(PushOnApply(0)));
    connect( buttonCancel,   SIGNAL(pressed()), this, SLOT(close()));
    connect( buttonHelp,     SIGNAL(pressed()), this, SLOT(PushOnHelp()));
}
// ------------------------------------------------------------------------
void MonCreateCase::InitBoundarys()
// ------------------------------------------------------------------------
// Initialisation des menus avec les frontieres deja enregistrees
{
  MESSAGE("InitBoundarys");
// Pour les frontieres analytiques : la colonne des groupes
  HOMARD::ListGroupType_var _listeGroupesCas = aCase->GetGroups();
  QTableWidgetItem *__colItem = new QTableWidgetItem();
  __colItem->setText(QApplication::translate("CreateCase", "", 0));
  TWBoundary->setHorizontalHeaderItem(0, __colItem);
  for ( int i = 0; i < _listeGroupesCas->length(); i++ )
  {
    TWBoundary->insertRow(i);
    TWBoundary->setItem( i, 0, new QTableWidgetItem(QString((_listeGroupesCas)[i]).trimmed()));
    TWBoundary->item( i, 0 )->setFlags(Qt::ItemIsEnabled |Qt::ItemIsSelectable );
  }
// Pour les frontieres CAO : la liste a saisir
// Pour les frontieres discretes : la liste a saisir
// Pour les frontieres analytiques : les colonnes de chaque frontiere
  HOMARD::HOMARD_Boundary_var myBoundary ;
  HOMARD::listeBoundarys_var  mesBoundarys = myHomardGen->GetAllBoundarysName();
//   MESSAGE("Nombre de frontieres enregistrees : "<<mesBoundarys->length());
  for (int i=0; i<mesBoundarys->length(); i++)
  {
    myBoundary = myHomardGen->GetBoundary(mesBoundarys[i]);
    int type_obj = myBoundary->GetType() ;
    if ( type_obj==-1 )     { CBBoundaryCAO->addItem(QString(mesBoundarys[i])); }
    else if ( type_obj==0 ) { CBBoundaryDi->addItem(QString(mesBoundarys[i])); }
    else                    { AddBoundaryAn(QString(mesBoundarys[i])); }
  }
// Ajustement
  TWBoundary->resizeColumnsToContents();
  TWBoundary->resizeRowsToContents();
  TWBoundary->clearSelection();
}
// -------------------------------
bool MonCreateCase::PushOnApply(int option)
// --------------------------------
{
  MESSAGE("PushOnApply");
  QString aCaseName=LEName->text().trimmed();
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
    if ( ( CaseNameDir != "" ) & ( CaseNameDir != aCaseName ) )
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

// On verifie qu'un groupe n'est pas associe a deux frontieres differentes
  if (CBBoundaryA->isChecked())
  {
    QStringList ListeGroup ;
    QString NomGroup ;
    int nbcol = TWBoundary->columnCount();
    int nbrow = TWBoundary->rowCount();
    for ( int col=1; col< nbcol; col++)
    {
      for ( int row=0; row< nbrow; row++)
      {
        if ( TWBoundary->item( row, col )->checkState() ==  Qt::Checked )
        {
//        Nom du groupe
          NomGroup = QString(TWBoundary->item(row, 0)->text()) ;
//           MESSAGE("NomGroup "<<NomGroup.toStdString().c_str());
          for ( int nugr = 0 ; nugr<ListeGroup.size(); nugr++)
          {
//             MESSAGE("....... "<<ListeGroup[nugr].toStdString().c_str());
            if ( NomGroup == ListeGroup[nugr] )
            {
              QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                        QObject::tr("HOM_CASE_GROUP").arg(NomGroup) );
              return false;
            }
          }
          ListeGroup.insert(0, NomGroup );
        }
      }
    }
  }

// Creation du cas
  if (aCaseName != _aCaseName )
  {
    _aCaseName = aCaseName;
    try
    {
    aCase = myHomardGen->CreateCase( \
            CORBA::string_dup(_aCaseName.toStdString().c_str()),  \
            CORBA::string_dup(aMeshName.toStdString().c_str()),  \
            CORBA::string_dup(aFileName.toStdString().c_str()) );
    }
    catch( SALOME::SALOME_Exception& S_ex )
    {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr(CORBA::string_dup(S_ex.details.text)) );
      return false;
    }
    LEFileName->setReadOnly(true);
    PushFichier->hide();
    InitBoundarys();
  }

// Repertoire et type
  aCase->SetDirName(aDirName.toStdString().c_str());
  _aDirName=aDirName;
  aCase->SetConfType(_ConfType);
  aCase->SetExtType(_ExtType);

//   Menage des eventuelles frontieres deja enregistrees
  aCase->SupprBoundaryGroup() ;

  // Enregistrement et publication dans l'arbre d'etudes a la sortie definitive
  if ( option > 0 )
  {
    if (RBBoundaryCAO->isChecked())
    {
      QString monBoundaryCAOName=CBBoundaryCAO->currentText();
      if (monBoundaryCAOName != "" )
      {
        aCase->AddBoundary(monBoundaryCAOName.toStdString().c_str());
      }
    }
    if (CBBoundaryD->isChecked())
    {
      QString monBoundaryDiName=CBBoundaryDi->currentText();
      if (monBoundaryDiName != "" )
      {
        aCase->AddBoundary(monBoundaryDiName.toStdString().c_str());
      }
    }
    if (CBBoundaryA->isChecked())
    {
      QString NomGroup ;
      int nbcol = TWBoundary->columnCount();
      int nbrow = TWBoundary->rowCount();
      for ( int col=1; col< nbcol; col++)
      {
        for ( int row=0; row< nbrow; row++)
        {
          if ( TWBoundary->item( row, col )->checkState() ==  Qt::Checked )
          {
  //        Nom du groupe
            NomGroup = QString(TWBoundary->item(row, 0)->text()) ;
  //        Nom de la frontiere
            QTableWidgetItem *__colItem = new QTableWidgetItem();
            __colItem = TWBoundary->horizontalHeaderItem(col);
            aCase->AddBoundaryGroup(QString(__colItem->text()).toStdString().c_str(), NomGroup.toStdString().c_str());
          }
        }
      }
    }
  }


// Options avancees
  if (CBAdvanced->isChecked())
  {
// Autorisation des pyramides
    if (CBPyramid->isChecked()) { _Pyram = 1 ; }
  }
  aCase->SetPyram(_Pyram);

  HOMARD_UTILS::updateObjBrowser();

  return true;
}
// ---------------------------
void MonCreateCase::PushOnOK()
// ---------------------------
{
  bool bOK = PushOnApply(1);
  if ( bOK ) this->close();
}
//------------------------------
void MonCreateCase::PushOnHelp()
//-------------------------------
{
  std::string LanguageShort = myHomardGen->GetLanguageShort();
  HOMARD_UTILS::PushOnHelp(QString("gui_create_case.html"), QString(""), QString(LanguageShort.c_str()));
}
// ---------------------------------
void MonCreateCase::SetNewName()
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
      if ( aCaseName ==  QString((MyCases)[i]))
      {
        num ++ ;
        aCaseName = "" ;
        break ;
      }
   }
  }
  LEName->clear() ;
  LEName->insert(aCaseName);
}

// ------------------------------------------------------------------------
void MonCreateCase::SetDirName()
// ------------------------------------------------------------------------
{
  QString aDirName=QFileDialog::getExistingDirectory ();
  if (!(aDirName.isEmpty()))LEDirName->setText(aDirName);
}
// ------------------------------------------------------------------------
void MonCreateCase::SetFileName()
// ------------------------------------------------------------------------
{
  QString fileName0 = LEFileName->text().trimmed();
  QString fileName = HOMARD_QT_COMMUN::PushNomFichier( false, QString("med") ) ;
  if (fileName.isEmpty()) fileName = fileName0 ;
  LEFileName->setText(fileName);
}
// ------------------------------------------------------------------------
void MonCreateCase::SetConforme()
// ------------------------------------------------------------------------
{
//
  _ConfType=0;
  RBNC1NpA->setVisible(0);
  RBNCQuelconque->setVisible(0);
//
  adjustSize();
}
// ------------------------------------------------------------------------
void MonCreateCase::SetNonConforme()
// ------------------------------------------------------------------------
{
//
  _ConfType=1;
  RBNC1NpA->setVisible(1);
  RBNCQuelconque->setVisible(1);
//
  adjustSize();
}
// ------------------------------------------------------------------------
void MonCreateCase::SetStandard()
// ------------------------------------------------------------------------
{
  if ( ( _ConfType == 0 ) || ( _ConfType == -1 ) ) { _ConfType = 0 ; }
  else { _ConfType = 1 ; }
  RBStandard->setChecked(true);
}
// ------------------------------------------------------------------------
void MonCreateCase::SetBox()
// ------------------------------------------------------------------------
{
  if ( ( _ConfType == 0 ) || ( _ConfType == -1 ) ) { _ConfType = -1 ; }
  else { _ConfType = -2 ; }
}
// ------------------------------------------------------------------------
void MonCreateCase::SetNC1NpA()
// ------------------------------------------------------------------------
{
  _ConfType = 2;
}
// ------------------------------------------------------------------------
void MonCreateCase::SetNCQuelconque()
// ------------------------------------------------------------------------
{
  _ConfType = 3;
}
// ------------------------------------------------------------------------
void MonCreateCase::SetMED()
// ------------------------------------------------------------------------
{
  _ExtType = 0 ;
  RBMED->setChecked(true);
}
// ------------------------------------------------------------------------
void MonCreateCase::SetSaturne()
// ------------------------------------------------------------------------
{
  _ExtType = 1 ;
  RBSaturne->setChecked(true);
}
// ------------------------------------------------------------------------
void MonCreateCase::SetSaturne2D()
// ------------------------------------------------------------------------
{
  _ExtType = 2 ;
  RBSaturne2D->setChecked(true);
}
// ------------------------------------------------------------------------
void MonCreateCase::SetBoundaryNo()
// ------------------------------------------------------------------------
{
//
  GBBoundaryC->setVisible(0);
  GBBoundaryA->setVisible(0);
  GBBoundaryD->setVisible(0);
  CBBoundaryD->setVisible(0);
  CBBoundaryA->setVisible(0);
//
  adjustSize();
}
// ------------------------------------------------------------------------
void MonCreateCase::SetBoundaryCAO()
// ------------------------------------------------------------------------
{
//
  GBBoundaryC->setVisible(1);
  GBBoundaryA->setVisible(0);
  GBBoundaryD->setVisible(0);
  CBBoundaryD->setVisible(0);
  CBBoundaryA->setVisible(0);
//
  adjustSize();
}
// ------------------------------------------------------------------------
void MonCreateCase::SetBoundaryNonCAO()
// ------------------------------------------------------------------------
{
//
  GBBoundaryC->setVisible(0);
  CBBoundaryD->setVisible(1);
  CBBoundaryA->setVisible(1);
//
  adjustSize();
}
// ------------------------------------------------------------------------
void MonCreateCase::AddBoundaryCAO(QString newBoundary)
// ------------------------------------------------------------------------
{
  CBBoundaryCAO->insertItem(0,newBoundary);
  CBBoundaryCAO->setCurrentIndex(0);
}
// ------------------------------------------------------------------------
void MonCreateCase::PushBoundaryCAONew()
// ------------------------------------------------------------------------
{
   MonCreateBoundaryCAO *BoundaryDlg = new MonCreateBoundaryCAO(this, true,
                HOMARD::HOMARD_Gen::_duplicate(myHomardGen), _aCaseName, "") ;
   BoundaryDlg->show();
}
// ------------------------------------------------------------------------
void MonCreateCase::PushBoundaryCAOEdit()
// ------------------------------------------------------------------------
{
  if (CBBoundaryCAO->currentText() == QString(""))  return;
  MonEditBoundaryCAO *BoundaryDlg = new MonEditBoundaryCAO(this, true,
       HOMARD::HOMARD_Gen::_duplicate(myHomardGen), _aCaseName, CBBoundaryCAO->currentText() ) ;
  BoundaryDlg->show();
}
// ------------------------------------------------------------------------
void MonCreateCase::PushBoundaryCAOHelp()
// ------------------------------------------------------------------------
{
  std::string LanguageShort = myHomardGen->GetLanguageShort();
  HOMARD_UTILS::PushOnHelp(QString("gui_create_boundary.html"), QString("CAO"), QString(LanguageShort.c_str()));
}
// ------------------------------------------------------------------------
void MonCreateCase::SetBoundaryD()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetBoundaryD ");
  if (CBBoundaryD->isChecked())
  {
    bool bOK = PushOnApply(0);
    if (bOK) { GBBoundaryD->setVisible(1); }
    else     { GBBoundaryD->setVisible(0);
               CBBoundaryD->setChecked(0);
               CBBoundaryD->setCheckState(Qt::Unchecked); }
  }
  else { GBBoundaryD->setVisible(0); }
//
  adjustSize();
}
// ------------------------------------------------------------------------
void MonCreateCase::AddBoundaryDi(QString newBoundary)
// ------------------------------------------------------------------------
{
  CBBoundaryDi->insertItem(0,newBoundary);
  CBBoundaryDi->setCurrentIndex(0);
}
// ------------------------------------------------------------------------
void MonCreateCase::PushBoundaryDiNew()
// ------------------------------------------------------------------------
{
   MonCreateBoundaryDi *BoundaryDlg = new MonCreateBoundaryDi(this, true,
                HOMARD::HOMARD_Gen::_duplicate(myHomardGen), _aCaseName, "") ;
   BoundaryDlg->show();
}
// ------------------------------------------------------------------------
void MonCreateCase::PushBoundaryDiEdit()
// ------------------------------------------------------------------------
{
  if (CBBoundaryDi->currentText() == QString(""))  return;
  MonEditBoundaryDi *BoundaryDlg = new MonEditBoundaryDi(this, true,
       HOMARD::HOMARD_Gen::_duplicate(myHomardGen), _aCaseName, CBBoundaryDi->currentText() ) ;
  BoundaryDlg->show();
}
// ------------------------------------------------------------------------
void MonCreateCase::PushBoundaryDiHelp()
// ------------------------------------------------------------------------
{
  std::string LanguageShort = myHomardGen->GetLanguageShort();
  HOMARD_UTILS::PushOnHelp(QString("gui_create_boundary.html"), QString("frontiere-discrete"), QString(LanguageShort.c_str()));
}
// ------------------------------------------------------------------------
void MonCreateCase::SetBoundaryA()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetBoundaryA ");
  if (CBBoundaryA->isChecked())
  {
    bool bOK = PushOnApply(0);
    if (bOK) { GBBoundaryA->setVisible(1); }
    else     { GBBoundaryA->setVisible(0);
               CBBoundaryA->setChecked(0);
               CBBoundaryA->setCheckState(Qt::Unchecked); }
  }
  else { GBBoundaryA->setVisible(0); }
//
  adjustSize();
}
// ------------------------------------------------------------------------
void MonCreateCase::AddBoundaryAn(QString newBoundary)
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de AddBoundaryAn ");
// Ajout d'une nouvelle colonne
  int nbcol = TWBoundary->columnCount();
//   MESSAGE("nbcol " <<  nbcol);
  nbcol += 1 ;
  TWBoundary->setColumnCount ( nbcol ) ;
  QTableWidgetItem *__colItem = new QTableWidgetItem();
  __colItem->setText(QApplication::translate("CreateCase", newBoundary.toStdString().c_str(), 0));
  TWBoundary->setHorizontalHeaderItem(nbcol-1, __colItem);
/*  TWBoundary->horizontalHeaderItem(nbcol-1)->setFlags( Qt::ItemIsSelectable|Qt::ItemIsEnabled );*/
// Chaque case est a cocher
  int nbrow = TWBoundary->rowCount();
//   MESSAGE("nbrow " <<  nbrow);
  for ( int i = 0; i < nbrow; i++ )
  {
    TWBoundary->setItem( i, nbcol-1, new QTableWidgetItem( QString ("") ) );
    TWBoundary->item( i, nbcol-1 )->setFlags( 0 );
    TWBoundary->item( i, nbcol-1 )->setFlags( Qt::ItemIsUserCheckable|Qt::ItemIsEnabled  );
    TWBoundary->item( i, nbcol-1 )->setCheckState( Qt::Unchecked );
  }
  TWBoundary->resizeColumnToContents(nbcol-1);
//   TWBoundary->resizeRowsToContents();
//   MESSAGE("Fin de AddBoundaryAn ");
}
// ------------------------------------------------------------------------
void MonCreateCase::PushBoundaryAnNew()
// ------------------------------------------------------------------------
{
   MonCreateBoundaryAn *BoundaryDlg = new MonCreateBoundaryAn(this, true,
                HOMARD::HOMARD_Gen::_duplicate(myHomardGen), _aCaseName) ;
   BoundaryDlg->show();
}
// ------------------------------------------------------------------------
void MonCreateCase::PushBoundaryAnEdit()
// ------------------------------------------------------------------------
{
  QString nom="";
  int nbcol = TWBoundary->columnCount();
  for ( int i = 1; i < nbcol; i++ )
  {
    QTableWidgetItem *__colItem = new QTableWidgetItem();
    __colItem = TWBoundary->horizontalHeaderItem(i);
    nom = QString(__colItem->text()) ;
    MESSAGE("nom "<<nom.toStdString().c_str());
    if (nom != QString(""))
    { MonEditBoundaryAn *BoundaryDlg = new MonEditBoundaryAn(this, true,
        HOMARD::HOMARD_Gen::_duplicate(myHomardGen), _aCaseName, nom ) ;
      BoundaryDlg->show(); }
  }
}
// ------------------------------------------------------------------------
void MonCreateCase::PushBoundaryAnHelp()
// ------------------------------------------------------------------------
{
  std::string LanguageShort = myHomardGen->GetLanguageShort();
  HOMARD_UTILS::PushOnHelp(QString("gui_create_boundary.html"), QString("frontiere-analytique"), QString(LanguageShort.c_str()));
}
// ------------------------------------------------------------------------
void MonCreateCase::CaseNameChanged()
// ------------------------------------------------------------------------
{
    if (_aCaseName != LEName->text().trimmed())
    {
       LEFileName->setReadOnly(false);
       PushFichier->show();
    }
}
// ------------------------------------------------------------------------
void MonCreateCase::SetAdvanced()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetAdvanced ");
  if (CBAdvanced->isChecked())
  { GBAdvancedOptions->setVisible(1);
    GBConforme->setVisible(1);
    RBStandard->setVisible(1);
    RBBox->setVisible(1);
    if ( ( _ConfType == 0 ) || ( _ConfType == -1 ) )
    { RBNC1NpA->setVisible(0);
      RBNCQuelconque->setVisible(0);}
    else
    { RBNC1NpA->setVisible(1);
      RBNCQuelconque->setVisible(1);}
    GBFormat->setVisible(1);
    RBMED->setVisible(1);
    RBSaturne2D->setVisible(1);
  }
  else
  { GBAdvancedOptions->setVisible(0);
    CBPyramid->setChecked(false);
    _Pyram = 0 ;
    SetStandard() ;
    SetMED() ;
 }
//
  adjustSize();
}
