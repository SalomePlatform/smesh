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

#include "MonCreateZone.h"
#include "MonCreateHypothesis.h"

#include <QFileDialog>
#include <QMessageBox>

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

// ----------------------------------------------------------------------
MonCreateZone::MonCreateZone(MonCreateHypothesis* parent, bool modal,
                             HOMARD::HOMARD_Gen_var myHomardGen0,
                             QString caseName) :
// ----------------------------------------------------------------------
/* Constructs a MonCreateZone
   appele pour une vraie creation
   initialise une boite et non une sphere
*/
    QDialog(0), Ui_CreateZone(),
    _parent(parent),
    _Name (""),
    _aCaseName(caseName),
    _Orient(0),
    _Type(2),
    _Xcentre(0), _Ycentre(0), _Zcentre(0), _Rayon(0),
    _ZoneXcentre(0), _ZoneYcentre(0), _ZoneZcentre(0), _ZoneRayon(0),
    _Xmin(0), _Xmax(0), _Xincr(0), _Ymin(0), _Ymax(0), _Yincr(0), _Zmin(0), _Zmax(0), _Zincr(0),
    _ZoneXmin(0), _ZoneXmax(0), _ZoneYmin(0), _ZoneYmax(0), _ZoneZmin(0), _ZoneZmax(0),
    _Xaxis(0), _Yaxis(0), _Zaxis(0), _RayonInt(0), _Haut(0),
    _ZoneXaxis(0), _ZoneYaxis(0), _ZoneZaxis(0), _ZoneRayonInt(0), _ZoneHaut(0),
    _DMax(0),
    Chgt (false)
    {
      MESSAGE("Constructeur") ;
      myHomardGen=HOMARD::HOMARD_Gen::_duplicate(myHomardGen0) ;
      setupUi(this) ;
      setModal(modal) ;
      InitConnect( ) ;

      SetNewName() ;
      InitValZone() ;           // Cherche les valeurs de la boite englobante le maillage
      InitMinMax() ;            // Initialise les bornes des boutons
      SetBox() ;                // Propose une boite en premier choix

    }
// ----------------------------------------------------------------------
MonCreateZone::MonCreateZone(MonCreateHypothesis* parent,
                             HOMARD::HOMARD_Gen_var myHomardGen0,
                             QString caseName):
// ----------------------------------------------------------------------
// Constructeur appele par MonEditZone
//
    QDialog(0), Ui_CreateZone(),
     myHomardGen(myHomardGen0),
    _parent(parent),
    _Name (""),
    _aCaseName(caseName),
    _Orient(0),
    _Type(2),
    _Xcentre(0), _Ycentre(0), _Zcentre(0), _Rayon(0),
    _ZoneXcentre(0), _ZoneYcentre(0), _ZoneZcentre(0), _ZoneRayon(0),
    _ZoneXmin(0), _ZoneXmax(0), _ZoneYmin(0), _ZoneYmax(0), _ZoneZmin(0), _ZoneZmax(0),
    _Xaxis(0), _Yaxis(0), _Zaxis(0), _RayonInt(0), _Haut(0),
    _ZoneXaxis(0), _ZoneYaxis(0), _ZoneZaxis(0), _ZoneRayonInt(0), _ZoneHaut(0),
    // Pour affichage lors de l edition d une Zone sans nom de Cas
    _Xmin(1), _Xmax(1), _Xincr(1), _Ymin(1), _Ymax(1), _Yincr(1), _Zmin(1), _Zmax(1), _Zincr(1),
    _DMax(1),
    Chgt (false)
    {
  //  MESSAGE("Debut de  MonCreateZone")
      setupUi(this) ;

      setModal(true) ;
      InitConnect() ;
    }

// ------------------------------------------------------------------------
MonCreateZone::~MonCreateZone()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void MonCreateZone::InitConnect()
// ------------------------------------------------------------------------
{
    connect( RBBox,        SIGNAL(clicked()) , this, SLOT(SetBox()) ) ;
    connect( RBSphere,     SIGNAL(clicked()) , this, SLOT(SetSphere()) ) ;
    connect( RBCylinder,   SIGNAL(clicked()) , this, SLOT(SetCylinder()) ) ;
    connect( RBPipe,       SIGNAL(clicked()) , this, SLOT(SetPipe()) ) ;
    connect( buttonOk,     SIGNAL( pressed() ), this, SLOT( PushOnOK() ) ) ;
    connect( buttonApply,  SIGNAL( pressed() ), this, SLOT( PushOnApply() ) ) ;
    connect( buttonCancel, SIGNAL( pressed() ), this, SLOT( close() ) ) ;
    connect( buttonHelp,   SIGNAL( pressed() ), this, SLOT( PushOnHelp() ) ) ;
}
// ------------------------------------------------------------------------
void MonCreateZone::InitValZone()
// ------------------------------------------------------------------------
{
//
//  1. Les coordonnees extremes du maillage
//
  if (_aCaseName == QString("")) { return; }

  HOMARD::HOMARD_Cas_var aCas = myHomardGen->GetCase(_aCaseName.toStdString().c_str()) ;
  HOMARD::extrema_var  MesExtremes = aCas->GetBoundingBox() ;
  int num = MesExtremes->length() ;
  ASSERT(num == 10) ;
  _Xmin=MesExtremes[0]; _Xmax=MesExtremes[1]; _Xincr=MesExtremes[2];
  _Ymin=MesExtremes[3]; _Ymax=MesExtremes[4]; _Yincr=MesExtremes[5];
  _Zmin=MesExtremes[6]; _Zmax=MesExtremes[7]; _Zincr=MesExtremes[8];
  _DMax=MesExtremes[9];
  if ( _Xincr < 0 ) { _Orient = 2 ; }
  else if ( _Yincr < 0 ) { _Orient = 3 ; }
  else if ( _Zincr < 0 ) { _Orient = 1 ; }
  MESSAGE ("_Xmin : " << _Xmin << " _Xmax : " << _Xmax << " _Xincr : " << _Xincr ) ;
  MESSAGE ("_Ymin : " << _Ymin << " _Ymax : " << _Ymax << " _Yincr : " << _Yincr ) ;
  MESSAGE ("_Zmin : " << _Zmin << " _Zmax : " << _Zmax << " _Zincr : " << _Zincr) ;
  MESSAGE ("_DMax : " << _DMax) ;
  MESSAGE ("_Orient : " << _Orient) ;
//  2. Caracteristiques des zones
// en X
  if ( _Xincr < 0 )
  {
    _ZoneXmin = _Xmin;
    _ZoneXmax = _Xmax;
  }
  else
  {
    _ZoneXmin = _Xmin - _Xincr;
    _ZoneXmax = _Xmax + _Xincr;
  }
  _Xcentre=(_Xmin + _Xmax)/2.;
// en Y
  if ( _Yincr < 0 )
  {
    _ZoneYmin = _Ymin;
    _ZoneYmax = _Ymax;
  }
  else
  {
    _ZoneYmin = _Ymin - _Yincr;
    _ZoneYmax = _Ymax + _Yincr;
  }
  _Ycentre=(_Ymin + _Ymax)/2.;
// en Z
  if ( _Zincr < 0 )
  {
    _ZoneZmin = _Zmin;
    _ZoneZmax = _Zmax;
  }
  else
  {
    _ZoneZmin = _Zmin - _Zincr;
    _ZoneZmax = _Zmax + _Zincr;
  }
  _Zcentre=(_Zmin + _Zmax)/2.;
// Rayons
  _Rayon= _DMax/4.;
  _RayonInt= _DMax/8.;
// Axe et hauteur pour cylindre et tuyau
  _Haut= _DMax/2.;
// 3. Gestion des icones
  SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr() ;
  QPixmap pix = resMgr->loadPixmap( "HOMARD", "boxdxyz.png" ) ;
  if ( _Orient == 0 ) { pix = resMgr->loadPixmap( "HOMARD", "boxdxyz.png" ) ; }
  else                { pix = resMgr->loadPixmap( "HOMARD", "boxdxy.png" ) ; }
  QIcon IS=QIcon(pix) ;
  RBBox->setIcon(IS) ;

  if ( _Orient == 0 ) { pix = resMgr->loadPixmap( "HOMARD", "cylinderpointvector.png" ) ; }
  else                { pix = resMgr->loadPixmap( "HOMARD", "disk.png" ) ;
                        RBCylinder->setText(QApplication::translate("CreateZone", "Disk", 0));
                        TLXbase->setText(QApplication::translate("CreateZone", "X centre", 0));
                        TLYbase->setText(QApplication::translate("CreateZone", "Y centre", 0));
                        TLZbase->setText(QApplication::translate("CreateZone", "Z centre", 0)); }
  IS=QIcon(pix) ;
  RBCylinder->setIcon(IS) ;

  if ( _Orient == 0 ) { pix = resMgr->loadPixmap( "HOMARD", "pipe.png" ) ; }
  else                { pix = resMgr->loadPixmap( "HOMARD", "diskwithhole.png" ) ;
                        RBPipe->setText(QApplication::translate("CreateZone", "Disk with hole", 0));
                        TLXbase_p->setText(QApplication::translate("CreateZone", "X centre", 0));
                        TLYbase_p->setText(QApplication::translate("CreateZone", "Y centre", 0));
                        TLZbase_p->setText(QApplication::translate("CreateZone", "Z centre", 0)); }
  IS=QIcon(pix) ;
  RBPipe->setIcon(IS) ;

  if ( _Orient == 0 ) { pix = resMgr->loadPixmap( "HOMARD", "spherepoint.png" ) ;
                        IS=QIcon(pix) ;
                        RBSphere->setIcon(IS) ; }
  else                { RBSphere->setVisible(0) ; }
}

// ------------------------------------------------------------------------
void MonCreateZone::InitMinMax()
// ------------------------------------------------------------------------
{
// en X
    if ( _Xincr > 0) {
      SpinBox_Xmini->setRange(_ZoneXmin,_ZoneXmax) ;
      SpinBox_Xmaxi->setRange(_ZoneXmin,_ZoneXmax) ;
      SpinBox_Xmini->setSingleStep(_Xincr) ;
      SpinBox_Xmaxi->setSingleStep(_Xincr) ;
      SpinBox_Xcentre->setSingleStep(_Xincr) ;
    }
    else {
      SpinBox_Xmini->setValue(_ZoneXmin) ;
      SpinBox_Xmaxi->setValue(_ZoneXmax) ;
      SpinBox_Xcentre->setValue(_Xcentre) ;
      SpinBox_Xbase->setValue(_Xcentre) ;
      SpinBox_Xbase_p->setValue(_Xcentre) ;
      SpinBox_Xmini->setEnabled(false) ;
      SpinBox_Xmaxi->setEnabled(false) ;
      SpinBox_Xcentre->setEnabled(false) ;
      SpinBox_Xbase->setEnabled(false) ;
      SpinBox_Xbase_p->setEnabled(false) ;
    }
// en Y
    if ( _Yincr > 0) {
      SpinBox_Ymini->setRange(_ZoneYmin,_ZoneYmax) ;
      SpinBox_Ymaxi->setRange(_ZoneYmin,_ZoneYmax) ;
      SpinBox_Ymini->setSingleStep(_Yincr) ;
      SpinBox_Ymaxi->setSingleStep(_Yincr) ;
      SpinBox_Ycentre->setSingleStep(_Yincr) ;
    }
    else {
      SpinBox_Ymini->setValue(_ZoneYmin) ;
      SpinBox_Ymaxi->setValue(_ZoneYmax) ;
      SpinBox_Ycentre->setValue(_Ycentre) ;
      SpinBox_Ybase->setValue(_Ycentre) ;
      SpinBox_Ybase_p->setValue(_Ycentre) ;
      SpinBox_Ymini->setEnabled(false) ;
      SpinBox_Ymaxi->setEnabled(false) ;
      SpinBox_Ycentre->setEnabled(false) ;
      SpinBox_Ybase->setEnabled(false) ;
      SpinBox_Ybase_p->setEnabled(false) ;
    }
// en Z
    if ( _Zincr > 0) {
      SpinBox_Zmini->setRange(_ZoneZmin,_ZoneZmax) ;
      SpinBox_Zmaxi->setRange(_ZoneZmin,_ZoneZmax) ;
      SpinBox_Zmini->setSingleStep(_Zincr) ;
      SpinBox_Zmaxi->setSingleStep(_Zincr) ;
      SpinBox_Zcentre->setSingleStep(_Zincr) ;
    }
    else {
      SpinBox_Zmini->setValue(_ZoneZmin) ;
      SpinBox_Zmaxi->setValue(_ZoneZmax) ;
      SpinBox_Zcentre->setValue(_Zcentre) ;
      SpinBox_Zbase->setValue(_Zcentre) ;
      SpinBox_Zbase_p->setValue(_Zcentre) ;
      SpinBox_Zmini->setEnabled(false) ;
      SpinBox_Zmaxi->setEnabled(false) ;
      SpinBox_Zcentre->setEnabled(false) ;
      SpinBox_Zbase->setEnabled(false) ;
      SpinBox_Zbase_p->setEnabled(false) ;
    }
// Rayons
    SpinBox_Rayon->setSingleStep(_Rayon/10.) ;
    SpinBox_Radius->setSingleStep(_Rayon/10.) ;
    SpinBox_Radius_int->setSingleStep(_Rayon/20.) ;
    SpinBox_Radius_ext->setSingleStep(_Rayon/10.) ;
// Axe et hauteur
// Si une coordonnee est constante, inutile de demander l'axe et la hauteur
    if ( _Orient > 0) {
      SpinBox_Xaxis->setVisible(0) ;
      SpinBox_Yaxis->setVisible(0) ;
      SpinBox_Zaxis->setVisible(0) ;
      SpinBox_Haut->setVisible(0) ;
      TLXaxis->setVisible(0) ;
      TLYaxis->setVisible(0) ;
      TLZaxis->setVisible(0) ;
      TLHaut->setVisible(0) ;
      SpinBox_Xaxis_p->setVisible(0) ;
      SpinBox_Yaxis_p->setVisible(0) ;
      SpinBox_Zaxis_p->setVisible(0) ;
      SpinBox_Haut_p->setVisible(0) ;
      TLXaxis_p->setVisible(0) ;
      TLYaxis_p->setVisible(0) ;
      TLZaxis_p->setVisible(0) ;
      TLHaut_p->setVisible(0) ;
    }
    else {
      SpinBox_Haut->setSingleStep(_Rayon/10.) ;
      SpinBox_Haut_p->setSingleStep(_Rayon/10.) ;
    }
}
// ------------------------------------------------------------------------
bool MonCreateZone::PushOnApply()
// ------------------------------------------------------------------------
// Appele lorsque l'un des boutons Ok ou Apply est presse
//
{
  if (LEName->text().trimmed()=="")
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_ZONE_NAME") ) ;
    return false;
  }

  switch (_Type)
  {
    case 11 : // il s agit d un rectangle
    { }
    case 12 : // il s agit d un rectangle
    { }
    case 13 : // il s agit d un rectangle
    { }
    case 2 : // il s agit d un parallelipipede rectangle
    {
      if (  (_ZoneXmin != SpinBox_Xmini->value()) ||
            (_ZoneXmax != SpinBox_Xmaxi->value()) ||
            (_ZoneYmin != SpinBox_Ymini->value()) ||
            (_ZoneYmax != SpinBox_Ymaxi->value()) ||
            (_ZoneZmin != SpinBox_Zmini->value()) ||
            (_ZoneZmax   != SpinBox_Zmaxi->value()) )
      {
        Chgt = true;
        _ZoneXmin= SpinBox_Xmini->value() ; _ZoneXmax= SpinBox_Xmaxi->value() ;
        _ZoneYmin= SpinBox_Ymini->value() ; _ZoneYmax= SpinBox_Ymaxi->value() ;
        _ZoneZmin= SpinBox_Zmini->value() ; _ZoneZmax= SpinBox_Zmaxi->value() ;
      }
      break ;
    }
    case 4 : // il s agit d une sphere
    {
      if (  (_ZoneXcentre != SpinBox_Xcentre->value()) ||
            (_ZoneYcentre != SpinBox_Ycentre->value()) ||
            (_ZoneZcentre != SpinBox_Zbase->value())   ||
            (_ZoneRayon   != SpinBox_Rayon->value()) )
      {
        Chgt = true;
        _ZoneXcentre=SpinBox_Xcentre->value() ;
        _ZoneYcentre=SpinBox_Ycentre->value() ;
        _ZoneZcentre=SpinBox_Zcentre->value() ;
        _ZoneRayon=SpinBox_Rayon->value() ;
      }
      break ;
    }
    case 31 : // il s agit d un disque issu d'un cylindre
    { }
    case 32 : // il s agit d un disque issu d'un cylindre
    { }
    case 33 : // il s agit d un disque issu d'un cylindre
    { }
    case 5 : // il s agit d un cylindre
    {
      if (  (_ZoneXcentre != SpinBox_Xbase->value())  ||
            (_ZoneYcentre != SpinBox_Ybase->value())  ||
            (_ZoneZcentre != SpinBox_Zbase->value())  ||
            (_ZoneRayon   != SpinBox_Radius->value()) ||
            (_ZoneHaut    != SpinBox_Haut->value())   ||
            (_ZoneXaxis   != SpinBox_Xaxis->value())  ||
            (_ZoneYaxis   != SpinBox_Yaxis->value())  ||
            (_ZoneZaxis   != SpinBox_Zaxis->value()) )
      {
        Chgt = true;
        _ZoneXcentre=SpinBox_Xbase->value() ;
        _ZoneYcentre=SpinBox_Ybase->value() ;
        _ZoneZcentre=SpinBox_Zbase->value() ;
        _ZoneXaxis=SpinBox_Xaxis->value() ;
        _ZoneYaxis=SpinBox_Yaxis->value() ;
        _ZoneZaxis=SpinBox_Zaxis->value() ;
        _ZoneRayon=SpinBox_Radius->value() ;
        _ZoneHaut=SpinBox_Haut->value() ;
      }
      break ;
    }
    case 61 : // il s agit d un disque avec trou
    { }
    case 62 : // il s agit d un disque avec trou
    { }
    case 63 : // il s agit d un disque avec trou
    { }
    case 7 : // il s agit d un tuyau
    {
      if (  (_ZoneXcentre  != SpinBox_Xbase_p->value())  ||
            (_ZoneYcentre  != SpinBox_Ybase_p->value())  ||
            (_ZoneZcentre  != SpinBox_Zbase_p->value())  ||
            (_ZoneRayonInt != SpinBox_Radius_int->value()) ||
            (_ZoneRayon    != SpinBox_Radius_ext->value()) ||
            (_ZoneHaut     != SpinBox_Haut_p->value())   ||
            (_ZoneXaxis    != SpinBox_Xaxis_p->value())  ||
            (_ZoneYaxis    != SpinBox_Yaxis_p->value())  ||
            (_ZoneZaxis    != SpinBox_Zaxis_p->value()) )
      {
        Chgt = true;
        _ZoneXcentre=SpinBox_Xbase_p->value() ;
        _ZoneYcentre=SpinBox_Ybase_p->value() ;
        _ZoneZcentre=SpinBox_Zbase_p->value() ;
        _ZoneXaxis=SpinBox_Xaxis_p->value() ;
        _ZoneYaxis=SpinBox_Yaxis_p->value() ;
        _ZoneZaxis=SpinBox_Zaxis_p->value() ;
        _ZoneRayonInt=SpinBox_Radius_int->value() ;
        _ZoneRayon=SpinBox_Radius_ext->value() ;
        _ZoneHaut=SpinBox_Haut_p->value() ;
      }
      break ;
   }
  }

// Controles
// Pour un rectangle ou un parallelepipede :
  if ( ( _Type >= 11 && _Type <= 13 ) || _Type == 2 )
  {
    if ((_ZoneXmin >= _ZoneXmax) && (_Xincr > 0)) {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr("HOM_ZONE_LIMIT").arg("X") ) ;
      return false; }

    if  ((_ZoneYmin >= _ZoneYmax) && (_Yincr > 0)) {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr("HOM_ZONE_LIMIT").arg("Y") ) ;
      return false; }

    if ((_ZoneZmin >= _ZoneZmax) && (_Zincr > 0)) {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr("HOM_ZONE_LIMIT").arg("Z") ) ;
      return false; }
  }
// L'axe pour un cylindre ou un tuyau :
  if ( _Type == 5 || _Type == 7 )
  {
    double daux = _ZoneXaxis*_ZoneXaxis + _ZoneYaxis*_ZoneYaxis + _ZoneZaxis*_ZoneZaxis ;
    if ( daux < 0.0000001 )
    {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr("HOM_AXE") );
      return false;
    }
  }
// Rayons pour disque avec trou ou un tuyau :
  if ( ( _Type >= 61 && _Type <= 63 ) || _Type == 7 )
  {
    if ( _ZoneRayonInt >= _ZoneRayon )
    {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr("HOM_ZONE_RAYON") );
      return false;
    }
  }
//
// Création ou mise à jour de la zone
//
  bool bOK = CreateOrUpdateZone() ;

  if ( bOK ) { HOMARD_UTILS::updateObjBrowser() ; }

  return bOK;

}
// ---------------------------------------------------
bool MonCreateZone::CreateOrUpdateZone()
//----------------------------------------------------
//  Creation de la zone
{
  MESSAGE("Debut de CreateOrUpdateZone avec _Type ="<<_Type<<", _Name ="<<_Name.toStdString().c_str()<<" et LEName ="<<LEName->text().trimmed().toStdString().c_str());
//
  if (_Name != LEName->text().trimmed())
  {
    _Name = LEName->text().trimmed() ;
    try
    {
      switch (_Type)
      {
        case 11 : // il s agit d un rectangle, dans le plan (X,Y)
        { aZone = myHomardGen->CreateZoneBox2D(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneXmin, _ZoneXmax, _ZoneYmin, _ZoneYmax, _Orient );
          break;
        }
        case 12 : // il s agit d un rectangle, dans le plan (Y,Z)
        { aZone = myHomardGen->CreateZoneBox2D(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneYmin, _ZoneYmax, _ZoneZmin, _ZoneZmax, _Orient );
          break;
        }
        case 13 : // il s agit d un rectangle, dans le plan (Z,X)
        { aZone = myHomardGen->CreateZoneBox2D(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneZmin, _ZoneZmax, _ZoneXmin, _ZoneXmax, _Orient );
          break;
        }
        case 2 : // il s agit d un parallelepipede
        { aZone = myHomardGen->CreateZoneBox(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneXmin, _ZoneXmax, _ZoneYmin, _ZoneYmax, _ZoneZmin, _ZoneZmax );
          break;
        }
        case 4 : // il s agit d une sphere
        { aZone = myHomardGen->CreateZoneSphere(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneXcentre, _ZoneYcentre, _ZoneZcentre, _ZoneRayon );
          break;
        }
        case 31 : // il s agit d un disque issu d'un cylindre, dans le plan (X,Y)
        { aZone = myHomardGen->CreateZoneDisk(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneXcentre, _ZoneYcentre, _ZoneRayon, _Orient );
          break;
          }
        case 32 : // il s agit d un disque issu d'un cylindre, dans le plan (Y,Z)
        { aZone = myHomardGen->CreateZoneDisk(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneYcentre, _ZoneZcentre, _ZoneRayon, _Orient );
          break;
        }
        case 33 : // il s agit d un disque issu d'un cylindre, dans le plan (Z,X)
        { aZone = myHomardGen->CreateZoneDisk(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneZcentre, _ZoneXcentre, _ZoneRayon, _Orient );
          break;
        }
        case 5 : // il s agit d un cylindre
        { aZone = myHomardGen->CreateZoneCylinder(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneXcentre, _ZoneYcentre, _ZoneZcentre, _ZoneXaxis, _ZoneYaxis, _ZoneZaxis, _ZoneRayon, _ZoneHaut );
          break;
        }
        case 61 : // il s agit d un disque avec trou, dans le plan (X,Y)
        { aZone = myHomardGen->CreateZoneDiskWithHole(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneXcentre, _ZoneYcentre, _ZoneRayon, _ZoneRayonInt, _Orient );
          break;
        }
        case 62 : // il s agit d un disque avec trou, dans le plan (Y,Z)
        { aZone = myHomardGen->CreateZoneDiskWithHole(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneYcentre, _ZoneZcentre, _ZoneRayon, _ZoneRayonInt, _Orient );
          break;
        }
        case 63 : // il s agit d un disque avec trou, dans le plan (Z,X)
        { aZone = myHomardGen->CreateZoneDiskWithHole(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneZcentre, _ZoneXcentre, _ZoneRayon, _ZoneRayonInt, _Orient );
          break;
        }
        case 7 : // il s agit d un tuyau
        { aZone = myHomardGen->CreateZonePipe(CORBA::string_dup(_Name.toStdString().c_str()), \
          _ZoneXcentre, _ZoneYcentre, _ZoneZcentre, _ZoneXaxis, _ZoneYaxis, _ZoneZaxis, _ZoneRayon, _ZoneHaut, _ZoneRayonInt );
          break;
        }
      }
    }
    catch( SALOME::SALOME_Exception& S_ex )
    {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr(CORBA::string_dup(S_ex.details.text)) );
      return false ;
    }
    _parent->addZoneinTWZone(_Name) ;
// Mise en place des attributs
    aZone->SetLimit(_Xincr, _Yincr, _Zincr) ;

    return true;
  }
  else {
    QMessageBox::warning( 0, QObject::tr("HOM_WARNING"),
                             QObject::tr("HOM_SELECT_OBJECT_4") );
    return false ;
  }
  MESSAGE("Fin de CreateOrUpdateZone");
}
// ------------------------------------------------------------------------
void MonCreateZone::PushOnOK()
// ------------------------------------------------------------------------
{
    if (PushOnApply()) this->close() ;
}
// ------------------------------------------------------------------------
void MonCreateZone::PushOnHelp()
// ------------------------------------------------------------------------
{
  std::string LanguageShort = myHomardGen->GetLanguageShort();
  HOMARD_UTILS::PushOnHelp(QString("gui_create_zone.html"), QString(""), QString(LanguageShort.c_str()));
}

// -----------------------------------
void MonCreateZone::SetNewName()
// -----------------------------------
{
  MESSAGE("SetNewName");
// Recherche d'un nom par defaut qui n'existe pas encore

  HOMARD::listeZones_var  MyObjects = myHomardGen->GetAllZonesName() ;
  int num = 0; QString aName="";
  while (aName=="" )
  {
    aName.setNum(num+1) ;
    aName.insert(0, QString("Zone_")) ;
    for ( int i=0; i<MyObjects->length() ; i++)
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
  MESSAGE("SetNewName aName ="<<aName.toStdString().c_str());
}
// ------------------------------------------------------------------------
void MonCreateZone::SetBox()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetBox")
  gBBox->setVisible(1) ;
  gBSphere->setVisible(0) ;
  gBCylindre->setVisible(0) ;
  gBPipe->setVisible(0) ;
  adjustSize() ;
  _Type=2;
// Sachant que l'increment est le 1/100eme de l'ecart (min/max), cela revient
// a initialiser la boite sur une boite 'centrale' comprise entre 2/5 et 3/5
  if ( _Xincr > 0 ) { SpinBox_Xmini->setValue(_Xcentre-10*_Xincr) ;
                      SpinBox_Xmaxi->setValue(_Xcentre+10*_Xincr) ; }
  else { _Type=12 ; }
  if ( _Yincr > 0 ) { SpinBox_Ymini->setValue(_Ycentre-10*_Yincr) ;
                      SpinBox_Ymaxi->setValue(_Ycentre+10*_Yincr) ; }
  else { _Type=13 ; }
  if ( _Zincr > 0 ) { SpinBox_Zmini->setValue(_Zcentre-10*_Zincr) ;
                      SpinBox_Zmaxi->setValue(_Zcentre+10*_Zincr) ; }
  else { _Type=11 ; }
  MESSAGE("Fin de SetBox")
}
// ------------------------------------------------------------------------
void MonCreateZone::SetSphere()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetSphere")
  gBBox->setVisible(0) ;
  gBSphere->setVisible(1) ;
  gBCylindre->setVisible(0) ;
  gBPipe->setVisible(0) ;
  adjustSize() ;
  _Type=4;
  SpinBox_Xcentre->setValue(_Xcentre) ;
  SpinBox_Ycentre->setValue(_Ycentre) ;
  SpinBox_Zcentre->setValue(_Zcentre) ;
  SpinBox_Rayon->setValue(_Rayon) ;
  MESSAGE("Fin de SetSphere")
}
// ------------------------------------------------------------------------
void MonCreateZone::SetCylinder()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetCylinder")
  gBBox->setVisible(0) ;
  gBSphere->setVisible(0) ;
  gBCylindre->setVisible(1) ;
  gBPipe->setVisible(0) ;
  adjustSize() ;
  _Type=5;
  if ( _Xincr > 0 ) { SpinBox_Xbase->setValue(_Xcentre) ;
                      SpinBox_Xaxis->setValue(0.) ; }
  else { _Type=32 ; }
  if ( _Yincr > 0 ) { SpinBox_Ybase->setValue(_Ycentre) ;
                      SpinBox_Yaxis->setValue(0.) ; }
  else { _Type=33 ; }
  if ( _Zincr > 0 ) { SpinBox_Zbase->setValue(_Zcentre) ;
                      SpinBox_Zaxis->setValue(1.) ; }
  else { _Type=31 ; }
  SpinBox_Radius->setValue(_Rayon) ;
  SpinBox_Haut->setValue(_Haut) ;
  MESSAGE("Fin de SetCylinder")
}
// ------------------------------------------------------------------------
void MonCreateZone::SetPipe()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetPipe")
  gBBox->setVisible(0) ;
  gBSphere->setVisible(0) ;
  gBCylindre->setVisible(0) ;
  gBPipe->setVisible(1) ;
  adjustSize() ;
  _Type=7;
  if ( _Xincr > 0 ) { SpinBox_Xbase_p->setValue(_Xcentre) ;
                      SpinBox_Xaxis_p->setValue(0.) ; }
  else { _Type=62 ; }
  if ( _Yincr > 0 ) { SpinBox_Ybase_p->setValue(_Ycentre) ;
                      SpinBox_Yaxis_p->setValue(0.) ; }
  else { _Type=63 ; }
  if ( _Zincr > 0 ) { SpinBox_Zbase_p->setValue(_Zcentre) ;
                      SpinBox_Zaxis_p->setValue(1.) ; }
  else { _Type=61 ; }
  SpinBox_Radius_int->setValue(_RayonInt) ;
  SpinBox_Radius_ext->setValue(_Rayon) ;
  SpinBox_Haut_p->setValue(_Haut) ;
  MESSAGE("Fin de SetPipe")
}


