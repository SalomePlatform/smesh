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

#include "MonEditHypothesis.h"
#include "MonEditListGroup.h"

#include "SalomeApp_Tools.h"
#include "HOMARDGUI_Utils.h"
#include "HomardQtCommun.h"
#include "HOMARD.hxx"
#include <utilities.h>

using namespace std;

// ----------------------------------------------------------------------------
MonEditHypothesis::MonEditHypothesis( MonCreateIteration* parent, bool modal,
                                      HOMARD::HOMARD_Gen_var myHomardGen,
                                      QString Name,
                                      QString caseName,  QString aFieldFile ):
// ----------------------------------------------------------------------------
/* Constructs a MonEditHypothesis
    herite de MonCreateHypothesis
*/
    MonCreateHypothesis(parent, modal,myHomardGen, Name, caseName, aFieldFile)
{
    MESSAGE("Hypothese " << Name.toStdString().c_str());
    setWindowTitle(QObject::tr("HOM_HYPO_EDIT_WINDOW_TITLE"));
    aHypothesis    = myHomardGen->GetHypothesis(_Name.toStdString().c_str());
    if (caseName == QString("") ){ _aCaseName = aHypothesis->GetCaseCreation();}
    InitValEdit();
    InitGroupes();
}
// ------------------------------
MonEditHypothesis::~MonEditHypothesis()
// ------------------------------
{
}
// ------------------------------
void MonEditHypothesis::InitGroupes()
// ------------------------------
{
    HOMARD::ListGroupType_var maListe = aHypothesis->GetGroups();
    for ( int i = 0; i < maListe->length(); i++ )
       _aListeGroupes << QString(maListe[i]);

    if (maListe->length() == 0) { CBGroupe->hide();}
}

// ------------------------------
void MonEditHypothesis::InitValEdit()
// ------------------------------
{
  MESSAGE("Debut de InitValEdit");
  LEName->setText(_Name);
  LEName->setReadOnly(true);
  HOMARD::listeTypes_var ListTypes (aHypothesis->GetAdapRefinUnRef());
  ASSERT( ListTypes->length()==3) ;
  _aTypeAdap = ListTypes[0];
  _aTypeRaff = ListTypes[1];
  _aTypeDera = ListTypes[2];

  if (_aTypeAdap == -1) InitAdaptUniforme();
  if (_aTypeAdap ==  0) InitAdaptZone();
  if (_aTypeAdap ==  1) InitAdaptChamps();

  RBUniDera->setDisabled(true);
  RBUniRaff->setDisabled(true);

  InitFieldInterp();

  if (_aTypeAdap == 1 || _TypeFieldInterp >= 1 )
  {
    if (_aFieldFile == QString("")) { GBFieldFile->setVisible(0); }
    else
    {
      GBFieldFile->setVisible(1);
      LEFieldFile->setText(_aFieldFile);
      LEFieldFile->setReadOnly(1);
    }
  }
  else
  {
    GBFieldFile->setVisible(0);
  }
// Les options avancees (non modifiables)
  CBAdvanced->setVisible(0) ;
  int NivMax = aHypothesis->GetNivMax();
  double DiamMin = aHypothesis->GetDiamMin();
  int AdapInit = aHypothesis->GetAdapInit();
  int ExtraOutput = aHypothesis->GetExtraOutput();
  if ( NivMax > 0 || DiamMin > 0 || AdapInit != 0 || ExtraOutput != 1 )
  { GBAdvancedOptions->setVisible(1);
    if ( NivMax > 0 )
    { SpinBoxNivMax->setValue(NivMax);
      SpinBoxNivMax->setDisabled(true); }
    else
    { TLMaximalLevel->setVisible(0);
      SpinBoxNivMax->setVisible(0); }
    if ( DiamMin > 0 )
    { doubleSpinBoxDiamMin->setValue(DiamMin);
      doubleSpinBoxDiamMin->setDisabled(true); }
    else
    { TLMinimalDiameter->setVisible(0);
      doubleSpinBoxDiamMin->setVisible(0); }
    if ( AdapInit != 0 )
    {
      if ( AdapInit > 0 )
      { RBAIR->setChecked(true); }
      else
      { RBAID->setChecked(true); }
      RBAIN->setEnabled(false);
      RBAIR->setEnabled(false);
      RBAID->setEnabled(false);
    }
    else
    { GBAdapInit->setVisible(0) ;
    }
    if ( ExtraOutput % 2 == 0 )
    {
      CBOutputLevel->setChecked(true);
      CBOutputLevel->setEnabled(false);
    }
    else { CBOutputLevel->setVisible(0) ; }
    if ( ExtraOutput % 3 == 0 )
    {
      CBOutputQuality->setChecked(true);
      CBOutputQuality->setEnabled(false);
    }
    else { CBOutputQuality->setVisible(0) ; }
    if ( ExtraOutput % 5 == 0 )
    {
      CBOutputDiameter->setChecked(true);
      CBOutputDiameter->setEnabled(false);
    }
    else { CBOutputDiameter->setVisible(0) ; }
    if ( ExtraOutput % 7 == 0 )
    {
      CBOutputParent->setChecked(true);
      CBOutputParent->setEnabled(false);
    }
    else { CBOutputParent->setVisible(0) ; }
    if ( ExtraOutput % 11 == 0 )
    {
      CBOutputVoisins->setChecked(true);
      CBOutputVoisins->setEnabled(false);
    }
    else { CBOutputVoisins->setVisible(0) ; }
  }
  else
  { GBAdvancedOptions->setVisible(0); }
//
}
// ----------------------------------------
void MonEditHypothesis::InitAdaptUniforme()
// ----------------------------------------
// Affichage des informations pour une adaptation uniforme
{
//  Choix des options generales
  GBFieldManagement->setVisible(0);
  GBAreaManagement->setVisible(0);
  GBUniform->setVisible(1);
  RBUniforme->setChecked(true);
  RBUniforme->setEnabled(false);
  RBChamp->setEnabled(false);
  RBZone->setEnabled(false);
//
// Raffinement ou deraffinement ?
  if (_aTypeDera == 0)
  {
      VERIFICATION(_aTypeRaff==1);
      RBUniDera->setChecked(false);
      RBUniRaff->setChecked(true);
  }
  if (_aTypeDera == 1)
  {
      VERIFICATION(_aTypeRaff==0);
      RBUniDera->setChecked(true);
      RBUniRaff->setChecked(false);
  }
//
  adjustSize();
//
}
// -------------------------------------
void MonEditHypothesis::InitAdaptZone()
// -------------------------------------
// Affichage des informations pour une adaptation selon des zones :
{
  MESSAGE ("Debut de InitAdaptZone");
//  Choix des options generales
  GBUniform->setVisible(0);
  GBFieldManagement->setVisible(0);
  GBAreaManagement->setVisible(1);
  RBZone->setChecked(true);
  RBChamp->setEnabled(false);
  RBUniforme->setEnabled(false);
  RBZone->setEnabled(false);

//  Recuperation de toutes les zones decrites et notation de celles retenues
  GetAllZones();
  HOMARD::listeZonesHypo_var mesZonesAvant = aHypothesis->GetZones();
  for (int i=0; i<mesZonesAvant->length(); i++)
  {
    MESSAGE ("i"<<i<<", zone :"<<string(mesZonesAvant[i])<<", type :"<<string(mesZonesAvant[i+1]));
    for ( int j =0 ; j < TWZone->rowCount(); j++)
    {
      MESSAGE (". j"<<j<<", zone :"<<TWZone->item(j,2)->text().toStdString());
      if ( TWZone->item(j,2)->text().toStdString() == string(mesZonesAvant[i]) )
      {
        MESSAGE ("OK avec "<<string(mesZonesAvant[i]));
        if ( string(mesZonesAvant[i+1]) == "1" )
        {
          MESSAGE ("... RAFF");
          TWZone->item( j,0 )->setCheckState( Qt::Checked );
          TWZone->item( j,1 )->setCheckState( Qt::Unchecked ); }
        else
        {
          MESSAGE ("... DERA");
          TWZone->item( j,0 )->setCheckState( Qt::Unchecked );
          TWZone->item( j,1 )->setCheckState( Qt::Checked ); }
        break;
      }
    }
    i += 1 ;
  }
//
//  Inactivation des choix
  for ( int j =0 ; j < TWZone->rowCount(); j++)
  {
    TWZone->item( j, 0 )->setFlags(0);
    TWZone->item( j, 1 )->setFlags(0);
  }
  PBZoneNew->setVisible(0);
//
  adjustSize();
//
}
// -------------------------------------
void MonEditHypothesis::InitAdaptChamps()
// -------------------------------------
// Affichage des informations pour une adaptation selon un champ :
// . Nom du champ
// . Composantes
// . Seuils
// . Absolu/relatif
{
    MESSAGE ("Debut de InitAdaptChamps");
//  Choix des options generales
    GBUniform->setVisible(0);
    GBAreaManagement->setVisible(0);
    GBFieldManagement->setVisible(1);
    RBChamp->setChecked(true);
    RBUniforme->setEnabled(false);
    RBChamp->setEnabled(false);
    RBZone->setEnabled(false);

    HOMARD::InfosHypo_var  aInfosHypo = aHypothesis->GetField();
    _aFieldName =  aInfosHypo->FieldName;
    _TypeThR = aInfosHypo->TypeThR;
    _ThreshR = aInfosHypo->ThreshR;
    _TypeThC = aInfosHypo->TypeThC;
    _ThreshC = aInfosHypo->ThreshC;
    _UsField = aInfosHypo->UsField;
    _UsCmpI  = aInfosHypo->UsCmpI;

    CBFieldName->insertItem(0,_aFieldName);
    CBFieldName->setCurrentIndex(0);
    CBFieldName->setEnabled(false);
    //SetFieldName(Qt::Unchecked);

    HOMARD::listeComposantsHypo_var mesComposantsAvant = aHypothesis->GetComps();
    TWCMP->clear();
    TWCMP->setRowCount(0);
    TWCMP->resizeRowsToContents();
    for (int i=0; i<mesComposantsAvant->length(); i++)
    {
       TWCMP->insertRow(0);
       TWCMP->setItem( 0, 0, new QTableWidgetItem( QString ("") ) );
       TWCMP->item( 0, 0 )->setFlags( Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
       TWCMP->item( 0, 0 )->setCheckState(Qt::Checked );
       TWCMP->item( 0, 0 )->setFlags( 0 );
       TWCMP->setItem( 0, 1, new QTableWidgetItem(QString(mesComposantsAvant[i]).trimmed()));
       TWCMP->item( 0, 1 )->setFlags( Qt::ItemIsEnabled |Qt::ItemIsSelectable );
    }
    TWCMP->resizeColumnsToContents();
    TWCMP->resizeRowsToContents();
    TWCMP->clearSelection();

  // Les seuils de raffinement
  RBRAbs->setEnabled(false) ;
  RBRRel->setEnabled(false) ;
  RBRPE->setEnabled(false) ;
  RBRMuSigma->setEnabled(false) ;
  RBRNo->setEnabled(false) ;
  if (_aTypeRaff== 0 )
  {
    SetRNo();
    RBRNo->setEnabled(true) ;
  }
  else
  {
    if ( _TypeThR == 1 ) { SpinBox_RAbs->setValue(_ThreshR); SetRAbs(); RBRAbs->setEnabled(true);}
    if ( _TypeThR == 2 ) { SpinBox_RRel->setValue(_ThreshR); SetRRel(); RBRRel->setEnabled(true);}
    if ( _TypeThR == 3 ) { SpinBox_RPE->setValue(_ThreshR);  SetRPE();  RBRPE->setEnabled(true);}
    if ( _TypeThR == 4 ) { SpinBox_RMuSigma->setValue(_ThreshR);  SetRMS();  RBRMuSigma->setEnabled(true);}
  }

  // Les seuils de deraffinement
  RBCAbs->setEnabled(false) ;
  RBCRel->setEnabled(false) ;
  RBCPE->setEnabled(false) ;
  RBCMuSigma->setEnabled(false) ;
  RBCNo->setEnabled(false) ;
  if (_aTypeDera== 0 )
  {
    SetCNo();
    RBCNo->setEnabled(true) ;
  }
  else
  {
    if ( _TypeThC == 1 ) { SpinBox_CAbs->setValue(_ThreshC); SetCAbs(); RBCAbs->setEnabled(true);}
    if ( _TypeThC == 2 ) { SpinBox_CRel->setValue(_ThreshC); SetCRel(); RBCRel->setEnabled(true);}
    if ( _TypeThC == 3 ) { SpinBox_CPE->setValue(_ThreshC);  SetCPE(); RBCPE->setEnabled(true);}
    if ( _TypeThC == 4 ) { SpinBox_CMuSigma->setValue(_ThreshC);  SetCMS();  RBCMuSigma->setEnabled(true);}
  }
  // Le choix de la prise en compte des composantes
  if ( TWCMP->rowCount() == 1 )
  { RBL2->setText(QObject::tr("HOM_HYPO_NORM_ABS"));
    RBInf->setText(QObject::tr("HOM_HYPO_NORM_REL"));
  }
  else
  { RBL2->setText(QObject::tr("HOM_HYPO_NORM_L2"));
    RBInf->setText(QObject::tr("HOM_HYPO_NORM_INF"));
  }
  if ( _UsField == 0 ) { CBJump->hide(); }
  else
  {
    CBJump->setChecked(true);
    CBJump->setEnabled(false);
  }
  if ( _UsCmpI == 0 )
  {
    RBL2->setChecked(true);
    RBL2->setEnabled(true);
    RBInf->setEnabled(false) ;
  }
  else
  {
    RBL2->setEnabled(false) ;
    RBInf->setChecked(true);
    RBInf->setEnabled(true);
  }
//
  adjustSize();
}
// -------------------------------------
void MonEditHypothesis::InitFieldInterp()
// -------------------------------------
// Affichage des informations pour les interpolations
{
    MESSAGE ("Debut de InitFieldInterp");
//  Choix des options generales
    _TypeFieldInterp = aHypothesis->GetTypeFieldInterp();
    MESSAGE ("_TypeFieldInterp = " << _TypeFieldInterp);
//
//  Aucune interpolation
    if ( _TypeFieldInterp == 0 )
    {
      RBFieldNo->setChecked(true);
      TWField->setVisible(0);
    }
//  Interpolation de tous les champs
    if ( _TypeFieldInterp == 1 )
    {
      RBFieldAll->setChecked(true);
      TWField->setVisible(0);
    }
//  Interpolation de champs choisis
    if ( _TypeFieldInterp == 2 )
    {
      RBFieldChosen->setChecked(true);
//
      TWField->setVisible(1);
      HOMARD::listeFieldInterpsHypo_var mesChampsAvant = aHypothesis->GetFieldInterps();
      TWField->clear();
      TWField->setRowCount(0);
      TWField->resizeRowsToContents();
      for (int iaux=0; iaux<mesChampsAvant->length(); iaux++)
      {
        TWField->insertRow(0);
        TWField->setItem( 0, 0, new QTableWidgetItem( QString ("") ) );
        TWField->item( 0, 0 )->setFlags( Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
        TWField->item( 0, 0 )->setCheckState(Qt::Checked );
        TWField->item( 0, 0 )->setFlags( 0 );
        TWField->setItem( 0, 1, new QTableWidgetItem(QString(mesChampsAvant[iaux]).trimmed()));
        TWField->item( 0, 1 )->setFlags( Qt::ItemIsEnabled |Qt::ItemIsSelectable );
        iaux++;
      }
      TWField->resizeColumnsToContents();
      TWField->resizeRowsToContents();
      TWField->clearSelection();
    }
    RBFieldNo->setEnabled(false) ;
    RBFieldAll->setEnabled(false) ;
    RBFieldChosen->setEnabled(false) ;
}

// -------------------------------------
bool MonEditHypothesis::PushOnApply()
// -------------------------------------
{
// Pour du raffinement selon un champ, les seuils ont-ils change ?
  if ( _aTypeAdap ==  1 )
  {
    if (_aTypeRaff!= 0 )
    {
      if (_TypeThR == 1) { _ThreshR = SpinBox_RAbs->value(); }
      if (_TypeThR == 2) { _ThreshR = SpinBox_RRel->value(); }
      if (_TypeThR == 3) { _ThreshR = SpinBox_RPE->value();  }
      if (_TypeThR == 4) { _ThreshR = SpinBox_RMuSigma->value();  }
      aHypothesis->SetRefinThr(_TypeThR, _ThreshR) ;
    }
    if (_aTypeDera!= 0 )
    {
      if (_TypeThC == 1) { _ThreshC = SpinBox_CAbs->value() ; }
      if (_TypeThC == 2) { _ThreshC = SpinBox_CRel->value() ; }
      if (_TypeThC == 3) { _ThreshC = SpinBox_CPE->value() ; }
      if (_TypeThC == 4) { _ThreshC = SpinBox_CMuSigma->value() ; }
      aHypothesis->SetUnRefThr(_TypeThC, _ThreshC) ;
    }

    myHomardGen->InvalideHypo(_Name.toStdString().c_str());
    HOMARD_UTILS::updateObjBrowser();
  }
  return true;
};
// ------------------------------------------------------------------------
void MonEditHypothesis::SetFiltrage()
// ------------------------------------------------------------------------
{
  if (CBGroupe->isChecked())
  {
    MonEditListGroup *aDlg = new MonEditListGroup(this, NULL, true, HOMARD::HOMARD_Gen::_duplicate(myHomardGen),_aCaseName, _aListeGroupes) ;
    aDlg->show();
  }
}

