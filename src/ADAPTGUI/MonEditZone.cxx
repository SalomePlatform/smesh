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

#include "MonEditZone.h"

#include "SalomeApp_Tools.h"
#include "HOMARDGUI_Utils.h"
#include <utilities.h>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_ViewManager.h>

using namespace std;

// ------------------------------------------------------------------------
MonEditZone::MonEditZone( MonCreateHypothesis* parent, bool modal,
                          HOMARD::HOMARD_Gen_var myHomardGen,
                          QString caseName, QString Name ):
// ------------------------------------------------------------------------
/* Constructs a MonEditZone
    herite de MonCreateZone
*/
    MonCreateZone(parent, myHomardGen, caseName)
{
    MESSAGE("Debut de MonEditZone pour " << Name.toStdString().c_str());
    setWindowTitle(QObject::tr("HOM_ZONE_EDIT_WINDOW_TITLE"));
    _Name=Name;
    aZone = myHomardGen->GetZone(_Name.toStdString().c_str());
    InitValEdit();
}
// ------------------------------------------------------------------------
MonEditZone::~MonEditZone()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void MonEditZone::InitValEdit()
// ------------------------------------------------------------------------
{
  MESSAGE("InitValEdit ");
  LEName->setText(_Name);
  LEName->setReadOnly(true);
  _Type = aZone->GetType();
  MESSAGE("InitValEdit _Type ="<<_Type);
  InitValZoneLimit();
  if (_aCaseName != QString("")) InitValZone();
  switch (_Type)
  {
    case 11 : // il s agit d un rectangle
    { }
    case 12 : // il s agit d un rectangle
    { }
    case 13 : // il s agit d un rectangle
    { }
    case 2 : // il s agit d une boite
    {
      InitValZoneBox();
      SetBox();
      break;
    }
    case 4 : // il s agit d une sphere
    {
      InitValZoneSphere();
      SetSphere();
      break;
    }
    case 31 : // il s agit d un cercle issu d'un cylindre
    { }
    case 32 : // il s agit d un cercle issu d'un cylindre
    { }
    case 33 : // il s agit d un cercle issu d'un cylindre
    { }
    case 5 : // il s agit d un cylindre
    {
      InitValZoneCylinder();
      SetCylinder();
      break;
    }
    case 61 : // il s agit d un disque avec trou issu d'un tuyau
    { }
    case 62 : // il s agit d un disque avec trou issu d'un tuyau
    { }
    case 63 : // il s agit d un disque avec trou issu d'un tuyau
    { }
    case 7 : // il s agit d un tuyau
    {
      InitValZonePipe();
      SetPipe();
      break;
    }
  };
}
// ------------------------------------------------------------------------
void MonEditZone::InitValZoneLimit()
// ------------------------------------------------------------------------
{
  HOMARD::double_array_var  mesCoordLimits = aZone->GetLimit();
  ASSERT(mesCoordLimits->length() == 3 );
  _Xincr=mesCoordLimits[0];
  _Yincr=mesCoordLimits[1];
  _Zincr=mesCoordLimits[2];
}
// ------------------------------------------------------------------------
void MonEditZone::InitValZoneBox()
// ------------------------------------------------------------------------
{
  HOMARD::double_array_var  mesCoordZones = aZone->GetCoords();
  ASSERT(mesCoordZones->length() == 6 );
  _ZoneXmin=mesCoordZones[0];
  _ZoneXmax=mesCoordZones[1];
  _ZoneYmin=mesCoordZones[2];
  _ZoneYmax=mesCoordZones[3];
  _ZoneZmin=mesCoordZones[4];
  _ZoneZmax=mesCoordZones[5];
}
// ------------------------------------------------------------------------
void MonEditZone::InitValZoneSphere()
// ------------------------------------------------------------------------
{
  HOMARD::double_array_var  mesCoordZones = aZone->GetCoords();
  ASSERT(mesCoordZones->length() == 4 );
  _ZoneXcentre=mesCoordZones[0];
  _ZoneYcentre=mesCoordZones[1];
  _ZoneZcentre=mesCoordZones[2];
  _ZoneRayon=mesCoordZones[3];

}
// ------------------------------------------------------------------------
void MonEditZone::InitValZoneCylinder()
// ------------------------------------------------------------------------
{
  HOMARD::double_array_var  mesCoordZones = aZone->GetCoords();
  ASSERT(mesCoordZones->length() == 8 );
  _ZoneXcentre=mesCoordZones[0];
  _ZoneYcentre=mesCoordZones[1];
  _ZoneZcentre=mesCoordZones[2];
  _ZoneXaxis=mesCoordZones[3];
  _ZoneYaxis=mesCoordZones[4];
  _ZoneZaxis=mesCoordZones[5];
  _ZoneRayon=mesCoordZones[6];
  _ZoneHaut=mesCoordZones[7];
}
// ------------------------------------------------------------------------
void MonEditZone::InitValZonePipe()
// ------------------------------------------------------------------------
{
  HOMARD::double_array_var  mesCoordZones = aZone->GetCoords();
  ASSERT(mesCoordZones->length() == 9 );
  _ZoneXcentre=mesCoordZones[0];
  _ZoneYcentre=mesCoordZones[1];
  _ZoneZcentre=mesCoordZones[2];
  _ZoneXaxis=mesCoordZones[3];
  _ZoneYaxis=mesCoordZones[4];
  _ZoneZaxis=mesCoordZones[5];
  _ZoneRayon=mesCoordZones[6];
  _ZoneHaut=mesCoordZones[7];
  _ZoneRayonInt=mesCoordZones[8];
}
// ------------------------------------------------------------------------
void MonEditZone::SetBox()
// ------------------------------------------------------------------------
{
  MESSAGE("SetBox ");
  gBBox->setVisible(1);
  gBSphere->setVisible(0);
  gBCylindre->setVisible(0) ;
  gBPipe->setVisible(0) ;
  RBBox->setChecked(1);
  adjustSize();
  RBCylinder->setDisabled(true);
  RBPipe->setDisabled(true);
  if ( _Type == 2 ) { RBSphere->setDisabled(true); }
  else                  { RBSphere->setVisible(0);
                          RBPipe->setText(QApplication::translate("CreateZone", "Disk with hole", 0));
                          RBCylinder->setText(QApplication::translate("CreateZone", "Disk", 0));
                          SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr() ;
                          QPixmap pix = resMgr->loadPixmap( "HOMARD", "boxdxy.png" ) ;
                          QIcon IS=QIcon(pix) ;
                          RBBox->setIcon(IS) ; }
  adjustSize();

  SpinBox_Xmini->setValue(_ZoneXmin);
  SpinBox_Xmaxi->setValue(_ZoneXmax);

  double incr ;
  if ( _Xincr > 0 ) {incr=_Xincr;} else{incr=1.;}
  SpinBox_Xmini->setSingleStep(incr);
  SpinBox_Xmaxi->setSingleStep(incr);

  SpinBox_Ymini->setValue(_ZoneYmin);
  SpinBox_Ymaxi->setValue(_ZoneYmax);
  if ( _Yincr > 0 ) {incr=_Yincr;} else{incr=1.;}
  SpinBox_Ymini->setSingleStep(incr);
  SpinBox_Ymaxi->setSingleStep(incr);

  SpinBox_Zmini->setValue(_ZoneZmin);
  SpinBox_Zmaxi->setValue(_ZoneZmax);
  if ( _Zincr > 0 ) {incr=_Zincr;} else{incr=1.;}
  SpinBox_Zmini->setSingleStep(incr);
  SpinBox_Zmaxi->setSingleStep(incr);

  if ( _Type == 12 ) { SpinBox_Xmini->setDisabled(true) ;
                           SpinBox_Xmaxi->setDisabled(true) ; }
  else if ( _Type == 13 ) { SpinBox_Ymini->setDisabled(true) ;
                                SpinBox_Ymaxi->setDisabled(true) ; }
  else if ( _Type == 11 ) { SpinBox_Zmini->setDisabled(true) ;
                                SpinBox_Zmaxi->setDisabled(true) ; }

}
// ------------------------------------------------------------------------
void MonEditZone::SetSphere()
// ------------------------------------------------------------------------
{
  gBBox->setVisible(0);
  gBSphere->setVisible(1);
  gBCylindre->setVisible(0) ;
  gBPipe->setVisible(0) ;
  RBSphere->setChecked(1);
  RBBox->setDisabled(true);
  RBCylinder->setDisabled(true);
  RBPipe->setDisabled(true);
  adjustSize();

  SpinBox_Xcentre->setValue(_ZoneXcentre);
  if ( _Xincr > 0) { SpinBox_Xcentre->setSingleStep(_Xincr); }
  else             { SpinBox_Xcentre->setSingleStep(1) ; }

  SpinBox_Ycentre->setValue(_ZoneYcentre);
  if ( _Yincr > 0) { SpinBox_Ycentre->setSingleStep(_Yincr); }
  else             { SpinBox_Ycentre->setSingleStep(1) ; }

  SpinBox_Zcentre->setValue(_ZoneZcentre);
  if ( _Zincr > 0) { SpinBox_Zcentre->setSingleStep(_Zincr); }
  else             { SpinBox_Zcentre->setSingleStep(1);}

  SpinBox_Rayon->setValue(_ZoneRayon);
}
// ------------------------------------------------------------------------
void MonEditZone::SetCylinder()
// ------------------------------------------------------------------------
{
  MESSAGE("SetCylinder _Xincr ="<<_Xincr<< " _Yincr ="<<_Yincr<< " _Zincr ="<<_Zincr);
  gBBox->setVisible(0);
  gBSphere->setVisible(0);
  gBCylindre->setVisible(1) ;
  gBPipe->setVisible(0) ;
  RBCylinder->setChecked(1);
  RBBox->setDisabled(true);
  RBPipe->setDisabled(true);
  if ( _Type == 5 ) { RBSphere->setDisabled(true); }
  else                  { RBSphere->setVisible(0);
                          RBPipe->setText(QApplication::translate("CreateZone", "Disk with hole", 0));
                          RBCylinder->setText(QApplication::translate("CreateZone", "Disk", 0));
                          TLXbase->setText(QApplication::translate("CreateZone", "X centre", 0));
                          TLYbase->setText(QApplication::translate("CreateZone", "Y centre", 0));
                          TLZbase->setText(QApplication::translate("CreateZone", "Z centre", 0));
                          SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr() ;
                          QPixmap pix = resMgr->loadPixmap( "HOMARD", "disk.png" ) ;
                          QIcon IS=QIcon(pix) ;
                          RBCylinder->setIcon(IS) ; }
  adjustSize();

  SpinBox_Xbase->setValue(_ZoneXcentre);
  if ( _Xincr > 0) { SpinBox_Xbase->setSingleStep(_Xincr); }
  else             { SpinBox_Xbase->setSingleStep(1) ; }

  SpinBox_Ybase->setValue(_ZoneYcentre);
  if ( _Yincr > 0) { SpinBox_Ybase->setSingleStep(_Yincr); }
  else             { SpinBox_Ybase->setSingleStep(1) ; }

  SpinBox_Zbase->setValue(_ZoneZcentre);
  if ( _Zincr > 0) { SpinBox_Zbase->setSingleStep(_Zincr); }
  else             { SpinBox_Zbase->setSingleStep(1) ;}

  SpinBox_Radius->setValue(_ZoneRayon);

  if ( _Type == 5 )
  { SpinBox_Xaxis->setValue(_ZoneXaxis) ;
    SpinBox_Yaxis->setValue(_ZoneYaxis) ;
    SpinBox_Zaxis->setValue(_ZoneZaxis) ;
    SpinBox_Haut->setValue(_ZoneHaut) ;
  }
  else
  { SpinBox_Xaxis->setVisible(0) ;
    SpinBox_Yaxis->setVisible(0) ;
    SpinBox_Zaxis->setVisible(0) ;
    SpinBox_Haut->setVisible(0) ;
    TLXaxis->setVisible(0) ;
    TLYaxis->setVisible(0) ;
    TLZaxis->setVisible(0) ;
    TLHaut->setVisible(0) ;
    if ( _Type == 32 ) { SpinBox_Xbase->setDisabled(true) ; }
    else if ( _Type == 33 ) { SpinBox_Ybase->setDisabled(true) ; }
    else if ( _Type == 31 ) { SpinBox_Zbase->setDisabled(true) ; }
  }
}
// ------------------------------------------------------------------------
void MonEditZone::SetPipe()
// ------------------------------------------------------------------------
{
  MESSAGE("SetPipe _Xincr ="<<_Xincr<< " _Yincr ="<<_Yincr<< " _Zincr ="<<_Zincr);
  gBBox->setVisible(0);
  gBSphere->setVisible(0);
  gBCylindre->setVisible(0) ;
  gBPipe->setVisible(1) ;
  RBPipe->setChecked(1);
  RBBox->setDisabled(true);
  RBCylinder->setDisabled(true);
  if ( _Type == 7 ) { RBSphere->setDisabled(true); }
  else                  { RBSphere->setVisible(0);
                          RBPipe->setText(QApplication::translate("CreateZone", "Disk with hole", 0));
                          RBCylinder->setText(QApplication::translate("CreateZone", "Disk", 0));
                          TLXbase_p->setText(QApplication::translate("CreateZone", "X centre", 0));
                          TLYbase_p->setText(QApplication::translate("CreateZone", "Y centre", 0));
                          TLZbase_p->setText(QApplication::translate("CreateZone", "Z centre", 0));
                          SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr() ;
                          QPixmap pix = resMgr->loadPixmap( "HOMARD", "diskwithhole.png" ) ;
                          QIcon IS=QIcon(pix) ;
                          RBPipe->setIcon(IS) ; }
  adjustSize();

  SpinBox_Xbase_p->setValue(_ZoneXcentre);
  if ( _Xincr > 0) { SpinBox_Xbase_p->setSingleStep(_Xincr); }
  else             { SpinBox_Xbase_p->setSingleStep(1) ; }

  SpinBox_Ybase_p->setValue(_ZoneYcentre);
  if ( _Yincr > 0) { SpinBox_Ybase_p->setSingleStep(_Yincr); }
  else             { SpinBox_Ybase_p->setSingleStep(1) ; }

  SpinBox_Zbase_p->setValue(_ZoneZcentre);
  if ( _Zincr > 0) { SpinBox_Zbase_p->setSingleStep(_Zincr); }
  else             { SpinBox_Zbase_p->setSingleStep(1) ;}

  SpinBox_Radius_int->setValue(_ZoneRayonInt);
  SpinBox_Radius_ext->setValue(_ZoneRayon);

  if ( _Type == 7 )
  { SpinBox_Xaxis_p->setValue(_ZoneXaxis) ;
    SpinBox_Yaxis_p->setValue(_ZoneYaxis) ;
    SpinBox_Zaxis_p->setValue(_ZoneZaxis) ;
    SpinBox_Haut_p->setValue(_ZoneHaut) ;
  }
  else
  { SpinBox_Xaxis_p->setVisible(0) ;
    SpinBox_Yaxis_p->setVisible(0) ;
    SpinBox_Zaxis_p->setVisible(0) ;
    SpinBox_Haut_p->setVisible(0) ;
    TLXaxis_p->setVisible(0) ;
    TLYaxis_p->setVisible(0) ;
    TLZaxis_p->setVisible(0) ;
    TLHaut_p->setVisible(0) ;
    if ( _Type == 62 ) { SpinBox_Xbase_p->setDisabled(true) ; }
    else if ( _Type == 63 ) { SpinBox_Ybase_p->setDisabled(true) ; }
    else if ( _Type == 61 ) { SpinBox_Zbase_p->setDisabled(true) ; }
  }
}


// ---------------------------------------------------
bool MonEditZone::CreateOrUpdateZone()
//----------------------------------------------------
//  Pas de Creation de la zone
//  Mise a jour des attributs de la Zone
{
  try
  {
    switch (_Type)
    {
      case 11 : // il s agit d un rectangle
      { }
      case 12 : // il s agit d un rectangle
      { }
      case 13 : // il s agit d un rectangle
      { }
      case 2 : // il s agit d un parallelepipede
      { aZone->SetBox( _ZoneXmin, _ZoneXmax, _ZoneYmin, _ZoneYmax, _ZoneZmin, _ZoneZmax );
        break;
      }
      case 4 : // il s agit d une sphere
      { aZone->SetSphere( _ZoneXcentre, _ZoneYcentre, _ZoneZcentre, _ZoneRayon );
        break;
      }
      case 31 : // il s agit d un disque issu d'un cylindre
      { }
      case 32 : // il s agit d un disque issu d'un cylindre
      { }
      case 33 : // il s agit d un disque issu d'un cylindre
      { }
      case 5 : // il s agit d un cylindre
      { aZone->SetCylinder( _ZoneXcentre, _ZoneYcentre, _ZoneZcentre, _ZoneXaxis, _ZoneYaxis, _ZoneZaxis, _ZoneRayon, _ZoneHaut );
        break;
      }
      case 61 : // il s agit d un disque issu d'un cylindre
      { }
      case 62 : // il s agit d un disque issu d'un cylindre
      { }
      case 63 : // il s agit d un disque issu d'un cylindre
      { }
      case 7 : // il s agit d un tuyau
      { aZone->SetPipe( _ZoneXcentre, _ZoneYcentre, _ZoneZcentre, _ZoneXaxis, _ZoneYaxis, _ZoneZaxis, _ZoneRayon, _ZoneHaut, _ZoneRayonInt );
        break;
      }
    }
    if (Chgt) myHomardGen->InvalideZone(_Name.toStdString().c_str());
    HOMARD_UTILS::updateObjBrowser();
  }
  catch( const SALOME::SALOME_Exception& S_ex ) {
       SalomeApp_Tools::QtCatchCorbaException( S_ex );
       return false;
  }
  return true;
}

