// Copyright (C) 2011-2025  CEA, EDF
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

#include "SMESHGUI_HomardBoundaryDlg.h"

#include "SMESHGUI_HomardAdaptDlg.h"
#include "SMESHGUI_HomardListGroup.h"

#include "SMESHGUI_Utils.h"

#include <QFileDialog>
#include <QMessageBox>

#include "SalomeApp_Tools.h"
#include "SMESHGUI_HomardUtils.h"
#include <utilities.h>

#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_ViewManager.h>

#include "math.h"
#define PI 3.141592653589793

using namespace std;

// ----------------------------------------------------------------------------------
SMESH_CreateBoundaryAn::SMESH_CreateBoundaryAn(SMESHGUI_HomardAdaptDlg* parent, bool modal,
                                               SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                               QString caseName) :
/* Constructs a SMESH_CreateBoundaryAn
   appele pour une vraie creation
   initialise un cylindre et non une sphere
*/
    QDialog(0), SMESH_Ui_CreateBoundaryAn(),
    _parent(parent),
    _Name (""),
    _Type(1),
    _BoundaryAnXcentre(0), _BoundaryAnYcentre(0), _BoundaryAnZcentre(0), _BoundaryAnRayon(0),
    _BoundaryAnXaxis(0), _BoundaryAnYaxis(0), _BoundaryAnZaxis(0),
    _Xcentre(0), _Ycentre(0), _Zcentre(0), _Rayon(0),
    _Xmin(0), _Xmax(0), _Xincr(0), _Ymin(0), _Ymax(0), _Yincr(0), _Zmin(0), _Zmax(0), _Zincr(0), _DMax(0),
    _BoundaryAnXcone1(0), _BoundaryAnYcone1(0), _BoundaryAnZcone1(0), _BoundaryAnRayon1(0),
    _BoundaryAnXcone2(0), _BoundaryAnYcone2(0), _BoundaryAnZcone2(0), _BoundaryAnRayon2(0),
    _BoundaryAnXaxisCone(0), _BoundaryAnYaxisCone(0), _BoundaryAnZaxisCone(0),
    _BoundaryAnXorigCone(0), _BoundaryAnYorigCone(0), _BoundaryAnZorigCone(0),
    _BoundaryAngle(0),
    _BoundaryAnToreXcentre(0), _BoundaryAnToreYcentre(0), _BoundaryAnToreZcentre(0),
    _BoundaryAnToreXaxe(0), _BoundaryAnToreYaxe(0), _BoundaryAnToreZaxe(0),
    _BoundaryAnToreRRev(0), _BoundaryAnToreRPri(0),
    Chgt (false)
    {
      MESSAGE("Constructeur") ;
      myHomardGen = SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
      setupUi(this);
      setModal(modal);

      // Gestion des icones
      QPixmap pix ;
      QIcon IS ;
      SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
      pix = resMgr->loadPixmap( "HOMARD", "spherepoint.png" );
      IS=QIcon(pix);
      RBSphere->setIcon(IS);
      pix = resMgr->loadPixmap( "HOMARD", "cylinderpointvector.png" );
      IS=QIcon(pix);
      RBCylindre->setIcon(IS);
      pix = resMgr->loadPixmap( "HOMARD", "cone.png" );
      IS=QIcon(pix);
      RBCone->setIcon(IS);
      pix = resMgr->loadPixmap( "HOMARD", "conepointvector.png" );
      IS=QIcon(pix);
      RB_Def_angle->setIcon(IS);
      pix = resMgr->loadPixmap( "HOMARD", "conedxyz.png" );
      IS=QIcon(pix);
      RB_Def_radius->setIcon(IS);
      pix = resMgr->loadPixmap( "HOMARD", "toruspointvector.png" );
      IS=QIcon(pix);
      RBTore->setIcon(IS);

      InitConnect( );

      SetNewName() ;
      InitValBoundaryAn();          // Cherche les valeurs de la boite englobante le maillage
      InitMinMax();                 // Initialise les bornes des boutons
      SetCylinder();                // Propose un cylindre en premier choix
    }

// -------------------------------------------------------------------------------
SMESH_CreateBoundaryAn::SMESH_CreateBoundaryAn(SMESHGUI_HomardAdaptDlg* parent,
                                               SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                               QString caseName):
    QDialog(0), SMESH_Ui_CreateBoundaryAn(),
    _parent(parent),
    _Name (""),
    _Type(1),
    _BoundaryAnXcentre(0), _BoundaryAnYcentre(0), _BoundaryAnZcentre(0), _BoundaryAnRayon(0),
    _BoundaryAnXaxis(0), _BoundaryAnYaxis(0), _BoundaryAnZaxis(0),
    // Pour affichage lors de l edition d une BoundaryAn sans nom de Cas
    _Xcentre(0), _Ycentre(0), _Zcentre(0), _Rayon(0),
    _Xmin(1), _Xmax(1), _Xincr(1), _Ymin(1), _Ymax(1), _Yincr(1), _Zmin(1), _Zmax(1), _Zincr(1), _DMax(1),
     Chgt (false)
    {
  //  MESSAGE("Debut de  SMESH_CreateBoundaryAn")
      myHomardGen = SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen0);
      setupUi(this);
      SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
      QPixmap pix = resMgr->loadPixmap( "HOMARD", "spherepoint.png" );
      QIcon IS=QIcon(pix);
      RBSphere->setIcon(IS);
      QPixmap pix2 = resMgr->loadPixmap( "HOMARD", "cylinderpointvector.png" );
      QIcon IS2=QIcon(pix2);
      RBCylindre->setIcon(IS2);
      QPixmap pix3 = resMgr->loadPixmap( "HOMARD", "cone.png" );
      QIcon IS3=QIcon(pix3);
      RBCone->setIcon(IS3);
      QPixmap pix4 = resMgr->loadPixmap( "HOMARD", "conepointvector.png" );
      QIcon IS4=QIcon(pix4);
      RB_Def_angle->setIcon(IS4);
      QPixmap pix5 = resMgr->loadPixmap( "HOMARD", "conedxyz.png" );
      QIcon IS5=QIcon(pix5);
      RB_Def_radius->setIcon(IS5);
      setModal(true);
      InitConnect();
    }

// ------------------------------------------------------------------------
SMESH_CreateBoundaryAn::~SMESH_CreateBoundaryAn()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryAn::InitConnect()
// ------------------------------------------------------------------------
{
    connect( RBCylindre,    SIGNAL(clicked()) , this, SLOT(SetCylinder()) ) ;
    connect( RBSphere,      SIGNAL(clicked()) , this, SLOT(SetSphere()) ) ;
    connect( RBCone,        SIGNAL(clicked()) , this, SLOT(SetCone()) ) ;
    connect( RB_Def_radius, SIGNAL(clicked()) , this, SLOT(SetConeR()) );
    connect( RB_Def_angle,  SIGNAL(clicked()) , this, SLOT(SetConeA()) );
    connect( RBTore,        SIGNAL(clicked()) , this, SLOT(SetTore()) ) ;
    connect( buttonOk,     SIGNAL( pressed() ), this, SLOT( PushOnOK() ) );
    connect( buttonApply,  SIGNAL( pressed() ), this, SLOT( PushOnApply() ) );
    connect( buttonCancel, SIGNAL( pressed() ), this, SLOT( close() ) );
    connect( buttonHelp,   SIGNAL( pressed() ), this, SLOT( PushOnHelp() ) );
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryAn::InitValBoundaryAn()
// ------------------------------------------------------------------------
{
  //
  //  1. Les coordonnees extremes du maillage
  //
    SMESHHOMARD::HOMARD_Cas_var aCas = myHomardGen->GetCase();
    SMESHHOMARD::extrema_var  MesExtremes = aCas->GetBoundingBox();
    int num = MesExtremes->length() ;
    ASSERT(num == 10);
    _Xmin=MesExtremes[0]; _Xmax=MesExtremes[1]; _Xincr=MesExtremes[2];
    _Ymin=MesExtremes[3]; _Ymax=MesExtremes[4]; _Yincr=MesExtremes[5];
    _Zmin=MesExtremes[6]; _Zmax=MesExtremes[7]; _Zincr=MesExtremes[8];
    _DMax=MesExtremes[9];
     MESSAGE ("_Xmin : " << _Xmin << " _Xmax : " << _Xmax << " _Xincr : " << _Xincr ) ;
     MESSAGE ("_Ymin : " << _Ymin << " _Ymax : " << _Ymax << " _Yincr : " << _Yincr ) ;
     MESSAGE ("_Zmin : " << _Zmin << " _Zmax : " << _Zmax << " _Zincr : " << _Zincr) ;
     MESSAGE ("_DMax : " << _DMax);

//  2. Caracteristiques des frontieres
// en X
    _Xcentre=(_Xmin + _Xmax)/2.;
// en Y
    _Ycentre=(_Ymin + _Ymax)/2.;
// en Z
    _Zcentre=(_Zmin + _Zmax)/2.;
// Rayon
    _Rayon= _DMax/4.;
}

// ------------------------------------------------------------------------
void SMESH_CreateBoundaryAn::InitMinMax()
// ------------------------------------------------------------------------
{
  // Cylindre
  // . X du centre
  SpinBox_Xcent->setValue(_Xcentre);
  SpinBox_Xcent->setSingleStep(_Xincr);
  // . Y du centre
  SpinBox_Ycent->setValue(_Ycentre);
  SpinBox_Ycent->setSingleStep(_Yincr);
  // . Z du centre
  SpinBox_Zcent->setValue(_Zcentre);
  SpinBox_Zcent->setSingleStep(_Zincr);
  // . X de l'axe
  SpinBox_Xaxis->setValue(0.);
  SpinBox_Xaxis->setSingleStep(0.1);
  // . Y de l'axe
  SpinBox_Yaxis->setValue(0.);
  SpinBox_Yaxis->setSingleStep(0.1);
  // . Z de l'axe
  SpinBox_Zaxis->setValue(1.);
  SpinBox_Zaxis->setSingleStep(0.1);
  // . Rayon
  SpinBox_Radius->setValue(_Rayon);
  SpinBox_Radius->setSingleStep(_Rayon/10.);

  // Sphere
  // . X du centre
  SpinBox_Xcentre->setValue(_Xcentre);
  SpinBox_Xcentre->setSingleStep(_Xincr);
  // . Y du centre
  SpinBox_Ycentre->setValue(_Ycentre);
  SpinBox_Ycentre->setSingleStep(_Yincr);
  // . Z du centre
  SpinBox_Zcentre->setValue(_Zcentre);
  SpinBox_Zcentre->setSingleStep(_Zincr);
  // . Rayon
  SpinBox_Rayon->setValue(_Rayon);
  SpinBox_Rayon->setSingleStep(_Rayon/10.);

  // Cone en rayons
  // . X des centres
  _BoundaryAnXcone1 = _Xcentre ;
  SpinBox_Cone_X1->setSingleStep(_Xincr);
  _BoundaryAnXcone2 = _Xcentre ;
  SpinBox_Cone_X2->setSingleStep(_Xincr);
  // . Y des centres
  _BoundaryAnYcone1 = _Ycentre ;
  SpinBox_Cone_Y1->setSingleStep(_Yincr);
  _BoundaryAnYcone2 = _Ycentre ;
  SpinBox_Cone_Y2->setSingleStep(_Yincr);
  // . Z des centres
  _BoundaryAnZcone1 = _Zmin ;
  SpinBox_Cone_Z1->setSingleStep(_Zincr);
  _BoundaryAnZcone2 = _Zmax ;
  SpinBox_Cone_Z2->setSingleStep(_Zincr);
  // . Rayons/Angles
  _BoundaryAnRayon1 = 0. ;
  _BoundaryAnRayon2 = _Rayon ;
  SpinBox_Cone_V2->setSingleStep(_Rayon/10.);

  // Cone en angle
  convertRayonAngle(1) ;
  SpinBox_Cone_X1->setValue(_BoundaryAnXaxisCone);
  SpinBox_Cone_Y1->setValue(_BoundaryAnYaxisCone);
  SpinBox_Cone_Z1->setValue(_BoundaryAnZaxisCone);
  SpinBox_Cone_V1->setValue(_BoundaryAngle);
  SpinBox_Cone_X2->setValue(_BoundaryAnXorigCone);
  SpinBox_Cone_Y2->setValue(_BoundaryAnYorigCone);
  SpinBox_Cone_Z2->setValue(_BoundaryAnZorigCone);

  // Tore
  // . X du centre
  SpinBoxToreXcent->setValue(_Xcentre);
  SpinBoxToreXcent->setSingleStep(_Xincr);
  // . Y du centre
  SpinBoxToreYcent->setValue(_Ycentre);
  SpinBoxToreYcent->setSingleStep(_Yincr);
  // . Z du centre
  SpinBoxToreZcent->setValue(_Zcentre);
  SpinBoxToreZcent->setSingleStep(_Zincr);
  // . X de l'axe
  SpinBoxToreXaxe->setValue(0.);
  SpinBoxToreXaxe->setSingleStep(0.1);
  // . Y de l'axe
  SpinBoxToreYaxe->setValue(0.);
  SpinBoxToreYaxe->setSingleStep(0.1);
  // . Z de l'axe
  SpinBoxToreZaxe->setValue(1.);
  SpinBoxToreZaxe->setSingleStep(0.1);
  // . Rayon de revolution
  SpinBoxToreRRev->setValue(_Rayon);
  SpinBoxToreRRev->setSingleStep(_Rayon/10.);
  // . Rayon primaire
  SpinBoxToreRPri->setValue(_Rayon/3.);
  SpinBoxToreRPri->setSingleStep(_Rayon/20.);
}
// ------------------------------------------------------------------------
bool SMESH_CreateBoundaryAn::PushOnApply()
// ------------------------------------------------------------------------
// Appele lorsque l'un des boutons Ok ou Apply est presse
//
{
  if (LEName->text().trimmed()=="")
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_BOUN_NAME") );
    return false;
  }

  switch (_Type)
  {
      case 1 : // il s agit d un cylindre
      {
        if ((_BoundaryAnXcentre != SpinBox_Xcent->value())  ||
            (_BoundaryAnYcentre != SpinBox_Ycent->value())  ||
            (_BoundaryAnZcentre != SpinBox_Zcent->value())  ||
            (_BoundaryAnRayon   != SpinBox_Radius->value()) ||
            (_BoundaryAnXaxis   != SpinBox_Xaxis->value()) ||
            (_BoundaryAnYaxis   != SpinBox_Yaxis->value()) ||
            (_BoundaryAnZaxis   != SpinBox_Zaxis->value()) )
        {
          Chgt = true;
          _BoundaryAnXaxis= SpinBox_Xaxis->value();
          _BoundaryAnYaxis= SpinBox_Yaxis->value();
          _BoundaryAnZaxis= SpinBox_Zaxis->value();
          _BoundaryAnXcentre=SpinBox_Xcent->value();
          _BoundaryAnYcentre=SpinBox_Ycent->value();
          _BoundaryAnZcentre=SpinBox_Zcent->value();
          _BoundaryAnRayon=SpinBox_Radius->value();
        }
        break;
      }

      case 2 : // il s agit d une sphere
      {
        if ((_BoundaryAnXcentre != SpinBox_Xcentre->value()) ||
            (_BoundaryAnYcentre != SpinBox_Ycentre->value()) ||
            (_BoundaryAnZcentre != SpinBox_Zcentre->value()) ||
            (_BoundaryAnRayon   != SpinBox_Rayon->value())  )
        {
           Chgt = true;
          _BoundaryAnXcentre=SpinBox_Xcentre->value();
          _BoundaryAnYcentre=SpinBox_Ycentre->value();
          _BoundaryAnZcentre=SpinBox_Zcentre->value();
          _BoundaryAnRayon=SpinBox_Rayon->value();
        }
        break;
      }

      case 3 : // il s agit d un cone defini par un axe et un angle
      {
        if ((_BoundaryAnXaxisCone != SpinBox_Cone_X1->value())  ||
            (_BoundaryAnYaxisCone != SpinBox_Cone_Y1->value())  ||
            (_BoundaryAnZaxisCone != SpinBox_Cone_Z1->value())  ||
            (_BoundaryAnXorigCone != SpinBox_Cone_X2->value())  ||
            (_BoundaryAnYorigCone != SpinBox_Cone_Y2->value())  ||
            (_BoundaryAnZorigCone != SpinBox_Cone_Z2->value())  ||
            (_BoundaryAngle       != SpinBox_Cone_V1->value()) )
        {
           Chgt = true;
          _BoundaryAnXaxisCone = SpinBox_Cone_X1->value() ;
          _BoundaryAnYaxisCone = SpinBox_Cone_Y1->value() ;
          _BoundaryAnZaxisCone = SpinBox_Cone_Z1->value() ;
          _BoundaryAnXorigCone = SpinBox_Cone_X2->value() ;
          _BoundaryAnYorigCone = SpinBox_Cone_Y2->value() ;
          _BoundaryAnZorigCone = SpinBox_Cone_Z2->value() ;
          _BoundaryAngle       = SpinBox_Cone_V1->value() ;
        }
        break;
      }

      case 4 : // il s agit d un cone defini par les 2 rayons
      {
        if ((_BoundaryAnXcone1 != SpinBox_Cone_X1->value())  ||
            (_BoundaryAnYcone1 != SpinBox_Cone_Y1->value())  ||
            (_BoundaryAnZcone1 != SpinBox_Cone_Z1->value())  ||
            (_BoundaryAnRayon1 != SpinBox_Cone_V1->value())  ||
            (_BoundaryAnXcone2 != SpinBox_Cone_X2->value())  ||
            (_BoundaryAnYcone2 != SpinBox_Cone_Y2->value())  ||
            (_BoundaryAnZcone2 != SpinBox_Cone_Z2->value())  ||
            (_BoundaryAnRayon2 != SpinBox_Cone_V2->value()) )
        {
           Chgt = true;
          _BoundaryAnXcone1 = SpinBox_Cone_X1->value() ;
          _BoundaryAnYcone1 = SpinBox_Cone_Y1->value() ;
          _BoundaryAnZcone1 = SpinBox_Cone_Z1->value() ;
          _BoundaryAnRayon1 = SpinBox_Cone_V1->value() ;
          _BoundaryAnXcone2 = SpinBox_Cone_X2->value() ;
          _BoundaryAnYcone2 = SpinBox_Cone_Y2->value() ;
          _BoundaryAnZcone2 = SpinBox_Cone_Z2->value() ;
          _BoundaryAnRayon2 = SpinBox_Cone_V2->value() ;
        }
        break;
      }
      case 5 : // il s agit d un tore
      {
        if ((_BoundaryAnToreXcentre != SpinBoxToreXcent->value())  ||
            (_BoundaryAnToreYcentre != SpinBoxToreYcent->value())  ||
            (_BoundaryAnToreZcentre != SpinBoxToreZcent->value())  ||
            (_BoundaryAnToreRRev   != SpinBoxToreRRev->value()) ||
            (_BoundaryAnToreRPri   != SpinBoxToreRPri->value()) ||
            (_BoundaryAnToreXaxe   != SpinBoxToreXaxe->value()) ||
            (_BoundaryAnToreYaxe   != SpinBoxToreYaxe->value()) ||
            (_BoundaryAnToreZaxe   != SpinBoxToreZaxe->value()) )
        {
          Chgt = true;
          _BoundaryAnToreXcentre= SpinBoxToreXcent->value();
          _BoundaryAnToreYcentre= SpinBoxToreYcent->value();
          _BoundaryAnToreZcentre= SpinBoxToreZcent->value();
          _BoundaryAnToreRRev=SpinBoxToreRRev->value();
          _BoundaryAnToreRPri=SpinBoxToreRPri->value();
          _BoundaryAnToreXaxe=SpinBoxToreXaxe->value();
          _BoundaryAnToreYaxe=SpinBoxToreYaxe->value();
          _BoundaryAnToreZaxe=SpinBoxToreZaxe->value();
        }
        break;
      }

 }

// Controles
// L'axe pour un cylindre
  if ( _Type == 5 )
  {
    double daux = _BoundaryAnXaxis*_BoundaryAnXaxis + _BoundaryAnYaxis*_BoundaryAnYaxis + _BoundaryAnZaxis*_BoundaryAnZaxis ;
    if ( daux < 0.0000001 )
    {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr("HOM_AXE") );
      return false;
    }
  }

  //
  // Création ou mise à jour de la frontière
  //
  bool bOK = CreateOrUpdateBoundaryAn();

  //if ( bOK ) { HOMARD_UTILS::updateObjBrowser() ; }

  return bOK;

}
// ---------------------------------------------------
bool SMESH_CreateBoundaryAn::CreateOrUpdateBoundaryAn()
//----------------------------------------------------
//  Creation de l'objet boundary
{
  MESSAGE("Debut de CreateOrUpdateBoundaryAn avec _Type ="<<_Type<<", _Name ="<<_Name.toStdString().c_str()<<" et LEName ="<<LEName->text().trimmed().toStdString().c_str());
//
  if (_Name != LEName->text().trimmed())
  {
    _Name = LEName->text().trimmed() ;
    try
    {
      switch (_Type)
      {
        case 1 : // il s agit d un cylindre
        { aBoundaryAn = myHomardGen->CreateBoundaryCylinder(CORBA::string_dup(_Name.toStdString().c_str()), \
            _BoundaryAnXcentre, _BoundaryAnYcentre, _BoundaryAnZcentre, _BoundaryAnXaxis, _BoundaryAnYaxis, _BoundaryAnZaxis, _BoundaryAnRayon );
          break;
        }
        case 2 : // il s agit d une sphere
        { aBoundaryAn = myHomardGen->CreateBoundarySphere(CORBA::string_dup(_Name.toStdString().c_str()), \
            _BoundaryAnXcentre, _BoundaryAnYcentre, _BoundaryAnZcentre, _BoundaryAnRayon);
          break;
       }
        case 3 : // il s agit d un cone defini par un axe et un angle
        { aBoundaryAn = myHomardGen->CreateBoundaryConeA(CORBA::string_dup(_Name.toStdString().c_str()), \
            _BoundaryAnXaxisCone, _BoundaryAnYaxisCone, _BoundaryAnZaxisCone, _BoundaryAngle, \
            _BoundaryAnXorigCone, _BoundaryAnYorigCone, _BoundaryAnYorigCone);
          break;
        }
        case 4 : // il s agit d un cone defini par les 2 rayons
        { aBoundaryAn = myHomardGen->CreateBoundaryConeR(CORBA::string_dup(_Name.toStdString().c_str()), \
            _BoundaryAnXcone1, _BoundaryAnYcone1, _BoundaryAnZcone1, _BoundaryAnRayon1, \
            _BoundaryAnXcone2, _BoundaryAnYcone2, _BoundaryAnZcone2, _BoundaryAnRayon2);
          break;
        }
        case 5 : // il s agit d un tore
        { aBoundaryAn = myHomardGen->CreateBoundaryTorus(CORBA::string_dup(_Name.toStdString().c_str()), \
            _BoundaryAnToreXcentre, _BoundaryAnToreYcentre, _BoundaryAnToreZcentre, _BoundaryAnToreXaxe, _BoundaryAnToreYaxe, _BoundaryAnToreZaxe, _BoundaryAnToreRRev, _BoundaryAnToreRPri );
          break;
        }
      }
    }
    catch( SALOME_CMOD::SALOME_Exception& S_ex )
    {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr(CORBA::string_dup(S_ex.details.text)) );
      return false ;
    }
    _parent->AddBoundaryAn(_Name);

    return true;
  }
  else {
    QMessageBox::warning( 0, QObject::tr("HOM_WARNING"),
                             QObject::tr("HOM_SELECT_OBJECT_4") );
    return false ;
  }
  MESSAGE("Fin de CreateOrUpdateBoundaryAn");
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryAn::PushOnOK()
// ------------------------------------------------------------------------
{
     if (PushOnApply()) this->close();
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryAn::PushOnHelp()
// ------------------------------------------------------------------------
{
  SMESH::ShowHelpFile(QString("homard_create_boundary.html#analytical-boundary"));
}

// -----------------------------------
void SMESH_CreateBoundaryAn::SetNewName()
// -----------------------------------
{
// Recherche d'un nom par defaut qui n'existe pas encore

  SMESHHOMARD::listeBoundarys_var MyObjects = myHomardGen->GetAllBoundarysName();
  int num = 0; QString aName="";
  while (aName=="" )
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
void SMESH_CreateBoundaryAn::SetCylinder()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetCylinder");
  gBCylindre->setVisible(1);
  gBSphere->setVisible(0);
  gBCone->setVisible(0);
  gBTore->setVisible(0);
//
  _Type=1;
//
  adjustSize();
//   MESSAGE("Fin de SetCylinder")
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryAn::SetSphere()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetSphere");
  gBCylindre->setVisible(0);
  gBSphere->setVisible(1);
  gBCone->setVisible(0);
  gBTore->setVisible(0);
//
  _Type=2;
//
  adjustSize();
//   MESSAGE("Fin de SetSphere")
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryAn::SetConeR()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetConeR");
//
// Stockage et conversion des valeurs si elles ont change
  if ((_BoundaryAnXaxisCone != SpinBox_Cone_X1->value())  ||
      (_BoundaryAnYaxisCone != SpinBox_Cone_Y1->value())  ||
      (_BoundaryAnZaxisCone != SpinBox_Cone_Z1->value())  ||
      (_BoundaryAnXorigCone != SpinBox_Cone_X2->value())  ||
      (_BoundaryAnYorigCone != SpinBox_Cone_Y2->value())  ||
      (_BoundaryAnZorigCone != SpinBox_Cone_Z2->value())  ||
      (_BoundaryAngle       != SpinBox_Cone_V1->value()) )
  {
    MESSAGE("Stockage et conversion");
    _BoundaryAnXaxisCone = SpinBox_Cone_X1->value() ;
    _BoundaryAnYaxisCone = SpinBox_Cone_Y1->value() ;
    _BoundaryAnZaxisCone = SpinBox_Cone_Z1->value() ;
    _BoundaryAnXorigCone = SpinBox_Cone_X2->value() ;
    _BoundaryAnYorigCone = SpinBox_Cone_Y2->value() ;
    _BoundaryAnZorigCone = SpinBox_Cone_Z2->value() ;
    _BoundaryAngle       = SpinBox_Cone_V1->value() ;
    convertRayonAngle(-1) ;
  }
//
  _Type=4;
//
  TLCone_X1->setText(QApplication::translate("CreateBoundaryAn", "X centre 1", 0));
  SpinBox_Cone_X1->setValue(_BoundaryAnXcone1);
  TLCone_Y1->setText(QApplication::translate("CreateBoundaryAn", "Y centre 1", 0));
  SpinBox_Cone_Y1->setValue(_BoundaryAnYcone1);
  TLCone_Z1->setText(QApplication::translate("CreateBoundaryAn", "Z centre 1", 0));
  SpinBox_Cone_Z1->setValue(_BoundaryAnZcone1);
//
  TLCone_V1->setText(QApplication::translate("CreateBoundaryAn", "Radius 1", 0));
  SpinBox_Cone_V1->setSingleStep(_Rayon/10.);
  SpinBox_Cone_V1->setMaximum(100000.*_DMax);
  SpinBox_Cone_V1->setValue(_BoundaryAnRayon1);
//
  TLCone_X2->setText(QApplication::translate("CreateBoundaryAn", "X centre 2", 0));
  SpinBox_Cone_X2->setValue(_BoundaryAnXcone2);
  TLCone_Y2->setText(QApplication::translate("CreateBoundaryAn", "Y centre 2", 0));
  SpinBox_Cone_Y2->setValue(_BoundaryAnYcone2);
  TLCone_Z2->setText(QApplication::translate("CreateBoundaryAn", "Z centre 2", 0));
  SpinBox_Cone_Z2->setValue(_BoundaryAnZcone2);
//
  TLCone_V2->setVisible(1);
  SpinBox_Cone_V2->setVisible(1);
  TLCone_V2->setText(QApplication::translate("CreateBoundaryAn", "Radius 2", 0));
  SpinBox_Cone_V2->setValue(_BoundaryAnRayon2);
//
//   MESSAGE("Fin de SetConeR")
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryAn::SetConeA()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetConeA");
// Stockage et conversion des valeurs si elles ont change
  if ((_BoundaryAnXcone1 != SpinBox_Cone_X1->value())  ||
      (_BoundaryAnYcone1 != SpinBox_Cone_Y1->value())  ||
      (_BoundaryAnZcone1 != SpinBox_Cone_Z1->value())  ||
      (_BoundaryAnRayon1 != SpinBox_Cone_V1->value())  ||
      (_BoundaryAnXcone2 != SpinBox_Cone_X2->value())  ||
      (_BoundaryAnYcone2 != SpinBox_Cone_Y2->value())  ||
      (_BoundaryAnZcone2 != SpinBox_Cone_Z2->value())  ||
      (_BoundaryAnRayon2 != SpinBox_Cone_V2->value()) )
  {
    MESSAGE("Stockage et conversion");
    _BoundaryAnXcone1 = SpinBox_Cone_X1->value() ;
    _BoundaryAnYcone1 = SpinBox_Cone_Y1->value() ;
    _BoundaryAnZcone1 = SpinBox_Cone_Z1->value() ;
    _BoundaryAnRayon1 = SpinBox_Cone_V1->value() ;
    _BoundaryAnXcone2 = SpinBox_Cone_X2->value() ;
    _BoundaryAnYcone2 = SpinBox_Cone_Y2->value() ;
    _BoundaryAnZcone2 = SpinBox_Cone_Z2->value() ;
    _BoundaryAnRayon2 = SpinBox_Cone_V2->value() ;
    convertRayonAngle(1) ;
  }
//
  _Type=3;
//
  TLCone_X1->setText(QApplication::translate("CreateBoundaryAn", "X axis", 0));
  SpinBox_Cone_X1->setValue(_BoundaryAnXaxisCone);
  TLCone_Y1->setText(QApplication::translate("CreateBoundaryAn", "Y axis", 0));
  SpinBox_Cone_Y1->setValue(_BoundaryAnYaxisCone);
  TLCone_Z1->setText(QApplication::translate("CreateBoundaryAn", "Z axis", 0));
  SpinBox_Cone_Z1->setValue(_BoundaryAnZaxisCone);
//
  TLCone_X2->setText(QApplication::translate("CreateBoundaryAn", "X centre", 0));
  SpinBox_Cone_X2->setValue(_BoundaryAnXorigCone);
  TLCone_Y2->setText(QApplication::translate("CreateBoundaryAn", "Y centre", 0));
  SpinBox_Cone_Y2->setValue(_BoundaryAnYorigCone);
  TLCone_Z2->setText(QApplication::translate("CreateBoundaryAn", "Z centre", 0));
  SpinBox_Cone_Z2->setValue(_BoundaryAnZorigCone);
//
  TLCone_V1->setText(QApplication::translate("CreateBoundaryAn", "Angle", 0));
  SpinBox_Cone_V1->setValue(_BoundaryAngle);
  SpinBox_Cone_V1->setSingleStep(1.);
  SpinBox_Cone_V1->setMaximum(90.);
//
  TLCone_V2->setVisible(0);
  SpinBox_Cone_V2->setVisible(0);
//   MESSAGE("Fin de SetConeA")
}


// ------------------------------------------------------------------------
void SMESH_CreateBoundaryAn::SetCone()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetCone");
  gBCylindre->setVisible(0);
  gBSphere->setVisible(0);
  gBCone->setVisible(1);
  gBTore->setVisible(0);
//
  if ( RB_Def_radius->isChecked() )
  {
    SetConeR();
  }
  else
  {
    SetConeA();
  }
//
  adjustSize();
//   MESSAGE("Fin de SetCone")
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryAn::SetTore()
// ------------------------------------------------------------------------
{
  MESSAGE("Debut de SetTore");
  gBCylindre->setVisible(0);
  gBSphere->setVisible(0);
  gBCone->setVisible(0);
  gBTore->setVisible(1);
//
  _Type=5;
//
  adjustSize();
//   MESSAGE("Fin de SetTore")
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryAn::convertRayonAngle(int option)
// ------------------------------------------------------------------------
// Conversion entre les deux formulations du cone :
// par deux rayons ou avec un axe et un angle.
// Voir sfcoi1 de HOMARD
{
  MESSAGE("Debut de convertRayonAngle, option = "<<option);
//
//         o
//         !    .
//         !        .
//       RA!            .
//         !                o
//         !              RB!   .
//         !                !       .
//         A----------------B----------O
//
//   Thales : RA/RB = AO/BO  ==> BO = AB*RB/(RA-RB)
//   Angle  : tg(alpha) = RA/AO
//
  double daux ;
// De rayon vers angle :
  if ( option == 1 )
  {
    double xa, ya, za, ra ;
    double xb, yb, zb, rb ;
// Positionnement de A vers B, avec RA>RB
    if ( _BoundaryAnRayon1 > _BoundaryAnRayon2 )
    {
      xa = _BoundaryAnXcone1 ;
      ya = _BoundaryAnYcone1 ;
      za = _BoundaryAnZcone1 ;
      ra = _BoundaryAnRayon1 ;
      xb = _BoundaryAnXcone2 ;
      yb = _BoundaryAnYcone2 ;
      zb = _BoundaryAnZcone2 ;
      rb = _BoundaryAnRayon2 ;
    }
    else
    {
      xa = _BoundaryAnXcone2 ;
      ya = _BoundaryAnYcone2 ;
      za = _BoundaryAnZcone2 ;
      ra = _BoundaryAnRayon2 ;
      xb = _BoundaryAnXcone1 ;
      yb = _BoundaryAnYcone1 ;
      zb = _BoundaryAnZcone1 ;
      rb = _BoundaryAnRayon1 ;
    }
// Axe : relie les deux centres, de A vers B.  L'axe est normalise
    _BoundaryAnXaxisCone = xb - xa ;
    _BoundaryAnYaxisCone = yb - ya ;
    _BoundaryAnZaxisCone = zb - za ;
    daux = sqrt ( _BoundaryAnXaxisCone*_BoundaryAnXaxisCone + _BoundaryAnYaxisCone*_BoundaryAnYaxisCone + _BoundaryAnZaxisCone*_BoundaryAnZaxisCone ) ;
    _BoundaryAnXaxisCone = _BoundaryAnXaxisCone/daux ;
    _BoundaryAnYaxisCone = _BoundaryAnYaxisCone/daux ;
    _BoundaryAnZaxisCone = _BoundaryAnZaxisCone/daux ;
// Origine
    daux = daux * rb / (ra-rb) ;
    _BoundaryAnXorigCone = xb + daux*_BoundaryAnXaxisCone ;
    _BoundaryAnYorigCone = yb + daux*_BoundaryAnYaxisCone ;
    _BoundaryAnZorigCone = zb + daux*_BoundaryAnZaxisCone ;
// Angle en degre
    daux = ra / sqrt((_BoundaryAnXorigCone-xa)*(_BoundaryAnXorigCone-xa) + (_BoundaryAnYorigCone-ya)*(_BoundaryAnYorigCone-ya) + (_BoundaryAnZorigCone-za)*(_BoundaryAnZorigCone-za) ) ;
    _BoundaryAngle = atan(daux)*180./PI ;
  }
// D'angle vers rayon :
  else
  {
    double xax, yax, zax ;
// L'axe est normalise
    daux = sqrt ( _BoundaryAnXaxisCone*_BoundaryAnXaxisCone + _BoundaryAnYaxisCone*_BoundaryAnYaxisCone + _BoundaryAnZaxisCone*_BoundaryAnZaxisCone ) ;
    xax = _BoundaryAnXaxisCone/daux ;
    yax = _BoundaryAnYaxisCone/daux ;
    zax = _BoundaryAnZaxisCone/daux ;
// Centre 1 : l'origine
    _BoundaryAnXcone1 = _BoundaryAnXorigCone ;
    _BoundaryAnYcone1 = _BoundaryAnYorigCone ;
    _BoundaryAnZcone1 = _BoundaryAnZorigCone ;
// Rayon 1 : nul
    _BoundaryAnRayon1 = 0. ;
// Centre 2 : l'origine decalee d'une longueur arbitraire le long de l'axe
    _BoundaryAnXcone2 = _BoundaryAnXorigCone + _DMax*xax ;
    _BoundaryAnYcone2 = _BoundaryAnYorigCone + _DMax*yax ;
    _BoundaryAnZcone2 = _BoundaryAnZorigCone + _DMax*zax ;
// Rayon 2 : a calculer
    _BoundaryAnRayon2 = _DMax*tan(_BoundaryAngle*PI/180.) ;
  }
//   MESSAGE("Fin de convertRayonAngle")
}

// -------------------------------------------------------------------------------
SMESH_CreateBoundaryCAO::SMESH_CreateBoundaryCAO(SMESHGUI_HomardAdaptDlg* parent, bool modal,
                                                 SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                                 QString caseName, QString aName)
// ---------------------------------------------------------------------------------
/* Constructs a SMESH_CreateBoundaryCAO */
    :
    QDialog(0), SMESH_Ui_CreateBoundaryCAO(),
    _parent(parent), _aName(aName),
    myHomardGen(SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen0))
    {
      MESSAGE("Constructeur");
      setupUi(this);
      setModal(modal);
      InitConnect();

     if ( _aName == QString("") ) {SetNewName();};
    }

// ------------------------------------------------------------------------
SMESH_CreateBoundaryCAO::~SMESH_CreateBoundaryCAO()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryCAO::InitConnect()
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
bool SMESH_CreateBoundaryCAO::PushOnApply()
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
   }
   catch( SALOME_CMOD::SALOME_Exception& S_ex )
   {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr(CORBA::string_dup(S_ex.details.text)) );
      return false;
   }
  }

// Les groupes
  AssocieLesGroupes();

  //HOMARD_UTILS::updateObjBrowser();
  return true;
}


// ------------------------------------------------------------------------
void SMESH_CreateBoundaryCAO::PushOnOK()
// ------------------------------------------------------------------------
{
     if (PushOnApply()) this->close();
     if ( _parent ) { _parent->raise(); _parent->activateWindow(); };
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryCAO::PushOnHelp()
// ------------------------------------------------------------------------
{
  SMESH::ShowHelpFile(QString("homard_create_boundary.html#cao-boundary"));
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryCAO::AssocieLesGroupes()
// ------------------------------------------------------------------------
{
  SMESHHOMARD::ListGroupType_var aSeqGroupe = new SMESHHOMARD::ListGroupType;
  aSeqGroupe->length(_listeGroupesBoundary.size());
  QStringList::const_iterator it;
  int i=0;
  for (it = _listeGroupesBoundary.constBegin(); it != _listeGroupesBoundary.constEnd(); it++)
     aSeqGroupe[i++]=(*it).toStdString().c_str();
  aBoundary->SetGroups(aSeqGroupe);

}

// -------------------------------------------------
void SMESH_CreateBoundaryCAO::SetNewName()
// --------------------------------------------------
{

  SMESHHOMARD::listeBoundarys_var  MyObjects = myHomardGen->GetAllBoundarysName();
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
void SMESH_CreateBoundaryCAO::SetCAOFile()
// ------------------------------------------------------------------------
{
  QString aCAOFile = SMESH_HOMARD_QT_COMMUN::PushNomFichier( false, QString("xao") );
  if (!(aCAOFile.isEmpty())) LEFileName->setText(aCAOFile);
}

// ------------------------------------------------------------------------
void SMESH_CreateBoundaryCAO::setGroups (QStringList listGroup)
// ------------------------------------------------------------------------
{
    _listeGroupesBoundary = listGroup;
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryCAO::SetFiltrage()
// ------------------------------------------------------------------------
{
  if (!CBGroupe->isChecked()) return;

  SMESH_CreateListGroupCAO *aDlg = new SMESH_CreateListGroupCAO
    (this, true, SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen),
     "Case_1", _listeGroupesBoundary);
  aDlg->show();
}

// -------------------------------------------------------------------------------
SMESH_CreateBoundaryDi::SMESH_CreateBoundaryDi(SMESHGUI_HomardAdaptDlg* parent, bool modal,
                                               SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                               QString caseName, QString aName)
// ---------------------------------------------------------------------------------
/* Constructs a SMESH_CreateBoundaryDi */
    :
    QDialog(0), SMESH_Ui_CreateBoundaryDi(),
    _parent(parent), _aName(aName),
    myHomardGen(SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen0))
    {
      MESSAGE("Constructeur") ;
      setupUi(this);
      setModal(modal);
      InitConnect();

     if ( _aName == QString("") ) {SetNewName();};
    }

// ------------------------------------------------------------------------
SMESH_CreateBoundaryDi::~SMESH_CreateBoundaryDi()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryDi::InitConnect()
// ------------------------------------------------------------------------
{
    connect( PushFichier,  SIGNAL(pressed()), this, SLOT(SetMeshFile()));
    connect( buttonOk,     SIGNAL(pressed()), this, SLOT( PushOnOK()));
    connect( buttonApply,  SIGNAL(pressed()), this, SLOT( PushOnApply()));
    connect( buttonCancel, SIGNAL(pressed()), this, SLOT(close()));
    connect( buttonHelp,   SIGNAL(pressed()), this, SLOT( PushOnHelp()));
    connect( CBGroupe,     SIGNAL(stateChanged(int)), this, SLOT( SetFiltrage()));
}

// ------------------------------------------------------------------------
bool SMESH_CreateBoundaryDi::PushOnApply()
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

//  Le maillage de la frontiere discrete
  QString aMeshFile=LEFileName->text().trimmed();
  if (aMeshFile ==QString(""))
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_BOUN_MESH") );
    return false;
  }

  //  Le nom du maillage de la frontiere discrete
  QString aMeshName = SMESH_HOMARD_QT_COMMUN::LireNomMaillage(aMeshFile);
  if (aMeshName == "" )
  {
    QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                              QObject::tr("HOM_MED_FILE_2") );
    return false;
  }

// Creation de l'objet CORBA si ce n'est pas deja fait sous le meme nom
  if ( _aName != aName )
  {
   try
   {
     _aName=aName;
     aBoundary=myHomardGen->CreateBoundaryDi(CORBA::string_dup(_aName.toStdString().c_str()), aMeshName.toStdString().c_str(), aMeshFile.toStdString().c_str());
     _parent->AddBoundaryDi(_aName);
   }
   catch( SALOME_CMOD::SALOME_Exception& S_ex )
   {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr(CORBA::string_dup(S_ex.details.text)) );
      return false;
   }
  }

// Les groupes
  AssocieLesGroupes();

  //HOMARD_UTILS::updateObjBrowser();
  return true;
}


// ------------------------------------------------------------------------
void SMESH_CreateBoundaryDi::PushOnOK()
// ------------------------------------------------------------------------
{
     if (PushOnApply()) this->close();
     if ( _parent ) { _parent->raise(); _parent->activateWindow(); };
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryDi::PushOnHelp()
// ------------------------------------------------------------------------
{
  SMESH::ShowHelpFile(QString("homard_create_boundary.html#discrete-boundary"));
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryDi::AssocieLesGroupes()
// ------------------------------------------------------------------------
{
  SMESHHOMARD::ListGroupType_var aSeqGroupe = new SMESHHOMARD::ListGroupType;
  aSeqGroupe->length(_listeGroupesBoundary.size());
  QStringList::const_iterator it;
  int i=0;
  for (it = _listeGroupesBoundary.constBegin(); it != _listeGroupesBoundary.constEnd(); it++)
     aSeqGroupe[i++]=(*it).toStdString().c_str();
  aBoundary->SetGroups(aSeqGroupe);

}

// -------------------------------------------------
void SMESH_CreateBoundaryDi::SetNewName()
// --------------------------------------------------
{

  SMESHHOMARD::listeBoundarys_var  MyObjects = myHomardGen->GetAllBoundarysName();
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
void SMESH_CreateBoundaryDi::SetMeshFile()
// ------------------------------------------------------------------------
{
  QString aMeshFile = SMESH_HOMARD_QT_COMMUN::PushNomFichier( false, QString("med") );
  if (!(aMeshFile.isEmpty())) LEFileName->setText(aMeshFile);
}

// ------------------------------------------------------------------------
void SMESH_CreateBoundaryDi::setGroups (QStringList listGroup)
// ------------------------------------------------------------------------
{
    _listeGroupesBoundary = listGroup;
}
// ------------------------------------------------------------------------
void SMESH_CreateBoundaryDi::SetFiltrage()
// // ------------------------------------------------------------------------
{
  if (!CBGroupe->isChecked()) return;

  SMESH_CreateListGroup *aDlg = new SMESH_CreateListGroup
    (this, true, SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen),
     "Case_1", _listeGroupesBoundary);
  aDlg->show();
}

// ------------------------------------------------------------------------
SMESH_EditBoundaryAn::SMESH_EditBoundaryAn( SMESHGUI_HomardAdaptDlg* parent, bool modal,
                                            SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                            QString caseName, QString Name ):
// ------------------------------------------------------------------------
/* Constructs a SMESH_EditBoundaryAn
    herite de SMESH_CreateBoundaryAn
*/
    SMESH_CreateBoundaryAn(parent, myHomardGen0, caseName)
{
    MESSAGE("Debut de SMESH_EditBoundaryAn pour " << Name.toStdString().c_str());
    setWindowTitle(QObject::tr("HOM_BOUN_A_EDIT_WINDOW_TITLE"));
    _Name=Name;
    aBoundaryAn = myHomardGen->GetBoundary(_Name.toStdString().c_str());
    InitValEdit();
}
// ------------------------------------------------------------------------
SMESH_EditBoundaryAn::~SMESH_EditBoundaryAn()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryAn::InitValEdit()
// ------------------------------------------------------------------------
{
  LEName->setText(_Name);
  LEName->setReadOnly(true);
  _Type = aBoundaryAn->GetType();
  MESSAGE("_Type : "<<_Type);
  InitValBoundaryAnLimit();
  InitValBoundaryAn();
  switch (_Type)
  {
    case 1 : // il s agit d un cylindre
    {
      InitValBoundaryAnCylindre();
      SetCylinder();
      break;
    }
    case 2: // il s agit d une sphere
    {
      InitValBoundaryAnSphere();
      SetSphere();
      break;
    }
    case 3: // il s agit d un cone defini par un axe et un angle
    {
      InitValBoundaryAnConeA();
      SetConeA();
      break;
    }
    case 4: // il s agit d un cone defini par les 2 rayons
    {
      InitValBoundaryAnConeR();
      SetConeR();
      break;
    }
    case 5: // il s agit d un tore
    {
      InitValBoundaryAnTore();
      SetTore();
      break;
    }
  };
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryAn::InitValBoundaryAnLimit()
// ------------------------------------------------------------------------
{
  SMESHHOMARD::double_array_var  mesCoordLimits = aBoundaryAn->GetLimit();
  ASSERT(mesCoordLimits->length() == 3 );
  _Xincr=mesCoordLimits[0];
  _Yincr=mesCoordLimits[1];
  _Zincr=mesCoordLimits[2];
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryAn::InitValBoundaryAnCylindre()
// ------------------------------------------------------------------------
{
  SMESHHOMARD::double_array_var  mesCoordBoundary = aBoundaryAn->GetCoords();
  ASSERT(mesCoordBoundary->length() == 7 );
  _BoundaryAnXcentre=mesCoordBoundary[0];
  _BoundaryAnYcentre=mesCoordBoundary[1];
  _BoundaryAnZcentre=mesCoordBoundary[2];
  _BoundaryAnXaxis=mesCoordBoundary[3];
  _BoundaryAnYaxis=mesCoordBoundary[4];
  _BoundaryAnZaxis=mesCoordBoundary[5];
  _BoundaryAnRayon=mesCoordBoundary[6];
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryAn::InitValBoundaryAnSphere()
// ------------------------------------------------------------------------
{
  SMESHHOMARD::double_array_var  mesCoordBoundary = aBoundaryAn->GetCoords();
  ASSERT(mesCoordBoundary->length() == 4 );
  _BoundaryAnXcentre=mesCoordBoundary[0];
  _BoundaryAnYcentre=mesCoordBoundary[1];
  _BoundaryAnZcentre=mesCoordBoundary[2];
  _BoundaryAnRayon=mesCoordBoundary[3];
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryAn::InitValBoundaryAnConeA()
// ------------------------------------------------------------------------
{
  SMESHHOMARD::double_array_var  mesCoordBoundary = aBoundaryAn->GetCoords();
  ASSERT(mesCoordBoundary->length() == 7 );
  _BoundaryAnXaxisCone=mesCoordBoundary[0];
  _BoundaryAnYaxisCone=mesCoordBoundary[1];
  _BoundaryAnZaxisCone=mesCoordBoundary[2];
  _BoundaryAngle=mesCoordBoundary[3];
  _BoundaryAnXorigCone=mesCoordBoundary[4];
  _BoundaryAnYorigCone=mesCoordBoundary[5];
  _BoundaryAnZorigCone=mesCoordBoundary[6];
  convertRayonAngle(-1) ;
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryAn::InitValBoundaryAnConeR()
// ------------------------------------------------------------------------
{
  SMESHHOMARD::double_array_var  mesCoordBoundary = aBoundaryAn->GetCoords();
  ASSERT(mesCoordBoundary->length() == 8 );
  _BoundaryAnXcone1=mesCoordBoundary[0];
  _BoundaryAnYcone1=mesCoordBoundary[1];
  _BoundaryAnZcone1=mesCoordBoundary[2];
  _BoundaryAnRayon1=mesCoordBoundary[3];
  _BoundaryAnXcone2=mesCoordBoundary[4];
  _BoundaryAnYcone2=mesCoordBoundary[5];
  _BoundaryAnZcone2=mesCoordBoundary[6];
  _BoundaryAnRayon2=mesCoordBoundary[7];
  convertRayonAngle(1) ;
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryAn::InitValBoundaryAnTore()
// ------------------------------------------------------------------------
{
  SMESHHOMARD::double_array_var  mesCoordBoundary = aBoundaryAn->GetCoords();
  ASSERT(mesCoordBoundary->length() == 8 );
  _BoundaryAnXcentre=mesCoordBoundary[0];
  _BoundaryAnYcentre=mesCoordBoundary[1];
  _BoundaryAnZcentre=mesCoordBoundary[2];
  _BoundaryAnXaxis=mesCoordBoundary[3];
  _BoundaryAnYaxis=mesCoordBoundary[4];
  _BoundaryAnZaxis=mesCoordBoundary[5];
  _BoundaryAnRayon1=mesCoordBoundary[6];
  _BoundaryAnRayon2=mesCoordBoundary[7];
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryAn::SetCylinder()
// ------------------------------------------------------------------------
{
  gBCylindre->setVisible(1);
  gBSphere->setVisible(0);
  gBCone->setVisible(0);
  gBTore->setVisible(0);
  RBCylindre->setChecked(1);
  _Type=1;
  RBSphere->setDisabled(true);
  RBCone->setDisabled(true);
  RBTore->setDisabled(true);

  SpinBox_Xcent->setValue(_BoundaryAnXcentre);
  SpinBox_Ycent->setValue(_BoundaryAnYcentre);
  SpinBox_Zcent->setValue(_BoundaryAnZcentre);

  SpinBox_Xaxis->setValue(_BoundaryAnXaxis);
  SpinBox_Yaxis->setValue(_BoundaryAnYaxis);
  SpinBox_Zaxis->setValue(_BoundaryAnZaxis);


  SpinBox_Xaxis->setSingleStep(0.1);
  SpinBox_Xcentre->setSingleStep(_Xincr);
  SpinBox_Yaxis->setSingleStep(0.1);
  SpinBox_Ycentre->setSingleStep(_Yincr);
  SpinBox_Zaxis->setSingleStep(0.1);
  SpinBox_Zcentre->setSingleStep(_Zincr);
// Rayon
  SpinBox_Radius->setValue(_BoundaryAnRayon);
  SpinBox_Radius->setSingleStep(_BoundaryAnRayon/10.);
//
  adjustSize();
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryAn::SetSphere()
// ------------------------------------------------------------------------
{
  gBCylindre->setVisible(0);
  gBSphere->setVisible(1);
  RBSphere->setChecked(1);
  gBCone->setVisible(0);
  gBTore->setVisible(0);
  RBCylindre->setDisabled(true);
  RBCone->setDisabled(true);
  RBTore->setDisabled(true);
  _Type=2 ;

  SpinBox_Xcentre->setValue(_BoundaryAnXcentre);
  if ( _Xincr > 0) { SpinBox_Xcentre->setSingleStep(_Xincr); }
  else             { SpinBox_Xcentre->setSingleStep(1) ; }

  SpinBox_Ycentre->setValue(_BoundaryAnYcentre);
  if ( _Yincr > 0) { SpinBox_Ycentre->setSingleStep(_Yincr); }
  else             { SpinBox_Ycentre->setSingleStep(1) ; }

  SpinBox_Zcentre->setValue(_BoundaryAnZcentre);
  if ( _Zincr > 0) { SpinBox_Zcentre->setSingleStep(_Zincr); }
  else             { SpinBox_Zcentre->setSingleStep(1);}

  SpinBox_Rayon->setValue(_BoundaryAnRayon);
//
  adjustSize();
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryAn::SetConeA()
// ------------------------------------------------------------------------
{
  gBCylindre->setVisible(0);
  gBSphere->setVisible(0);
  gBCone->setVisible(1);
  RBCone->setChecked(1);
  gBTore->setVisible(0);
  RB_Def_angle->setChecked(1);
  RBCylindre->setDisabled(true);
  RBSphere->setDisabled(true);
  RBTore->setDisabled(true);
  _Type=3;
//
  TLCone_X1->setText(QApplication::translate("CreateBoundaryAn", "X axis", 0));
  SpinBox_Cone_X1->setValue(_BoundaryAnXaxisCone);
  TLCone_Y1->setText(QApplication::translate("CreateBoundaryAn", "Y axis", 0));
  SpinBox_Cone_Y1->setValue(_BoundaryAnYaxisCone);
  TLCone_Z1->setText(QApplication::translate("CreateBoundaryAn", "Z axis", 0));
  SpinBox_Cone_Z1->setValue(_BoundaryAnZaxisCone);
//
  TLCone_X2->setText(QApplication::translate("CreateBoundaryAn", "X centre", 0));
  SpinBox_Cone_X2->setValue(_BoundaryAnXorigCone);
  TLCone_Y2->setText(QApplication::translate("CreateBoundaryAn", "Y centre", 0));
  SpinBox_Cone_Y2->setValue(_BoundaryAnYorigCone);
  TLCone_Z2->setText(QApplication::translate("CreateBoundaryAn", "Z centre", 0));
  SpinBox_Cone_Z2->setValue(_BoundaryAnZorigCone);
//
  TLCone_V1->setText(QApplication::translate("CreateBoundaryAn", "Angle", 0));
  SpinBox_Cone_V1->setValue(_BoundaryAngle);
  SpinBox_Cone_V1->setSingleStep(1.);
  SpinBox_Cone_V1->setMaximum(90.);
//
  TLCone_V2->setVisible(0);
  SpinBox_Cone_V2->setVisible(0);
//
  adjustSize();
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryAn::SetConeR()
// ------------------------------------------------------------------------
{
  gBCylindre->setVisible(0);
  gBSphere->setVisible(0);
  gBCone->setVisible(1);
  gBTore->setVisible(0);
  RBCone->setChecked(1);
  RB_Def_radius->setChecked(1);
  RBCylindre->setDisabled(true);
  RBSphere->setDisabled(true);
  RBTore->setDisabled(true);
  _Type=4;
//
  TLCone_X1->setText(QApplication::translate("CreateBoundaryAn", "X centre 1", 0));
  SpinBox_Cone_X1->setValue(_BoundaryAnXcone1);
  TLCone_Y1->setText(QApplication::translate("CreateBoundaryAn", "Y centre 1", 0));
  SpinBox_Cone_Y1->setValue(_BoundaryAnYcone1);
  TLCone_Z1->setText(QApplication::translate("CreateBoundaryAn", "Z centre 1", 0));
  SpinBox_Cone_Z1->setValue(_BoundaryAnZcone1);
//
  TLCone_V1->setText(QApplication::translate("CreateBoundaryAn", "Radius 1", 0));
  SpinBox_Cone_V1->setSingleStep(_Rayon/10.);
  SpinBox_Cone_V1->setMaximum(100000.*_DMax);
  SpinBox_Cone_V1->setValue(_BoundaryAnRayon1);
//
  TLCone_X2->setText(QApplication::translate("CreateBoundaryAn", "X centre 2", 0));
  SpinBox_Cone_X2->setValue(_BoundaryAnXcone2);
  TLCone_Y2->setText(QApplication::translate("CreateBoundaryAn", "Y centre 2", 0));
  SpinBox_Cone_Y2->setValue(_BoundaryAnYcone2);
  TLCone_Z2->setText(QApplication::translate("CreateBoundaryAn", "Z centre 2", 0));
  SpinBox_Cone_Z2->setValue(_BoundaryAnZcone2);
//
  TLCone_V2->setVisible(1);
  SpinBox_Cone_V2->setVisible(1);
  TLCone_V2->setText(QApplication::translate("CreateBoundaryAn", "Radius 2", 0));
  SpinBox_Cone_V2->setValue(_BoundaryAnRayon2);
//
  adjustSize();
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryAn::SetTore()
// ------------------------------------------------------------------------
{
  gBCylindre->setVisible(0);
  gBSphere->setVisible(0);
  gBCone->setVisible(0);
  gBTore->setVisible(1);
  RBTore->setChecked(1);
  _Type=5;
  RBCylindre->setDisabled(true);
  RBSphere->setDisabled(true);
  RBCone->setDisabled(true);

  SpinBoxToreXcent->setValue(_BoundaryAnXcentre);
  SpinBoxToreYcent->setValue(_BoundaryAnYcentre);
  SpinBoxToreZcent->setValue(_BoundaryAnZcentre);

  SpinBoxToreXaxe->setValue(_BoundaryAnXaxis);
  SpinBoxToreYaxe->setValue(_BoundaryAnYaxis);
  SpinBoxToreZaxe->setValue(_BoundaryAnZaxis);


  SpinBoxToreXaxe->setSingleStep(0.1);
  SpinBoxToreXcent->setSingleStep(_Xincr);
  SpinBoxToreYaxe->setSingleStep(0.1);
  SpinBoxToreYcent->setSingleStep(_Yincr);
  SpinBoxToreZaxe->setSingleStep(0.1);
  SpinBoxToreZcent->setSingleStep(_Zincr);
// Rayon de revolution
  SpinBoxToreRRev->setValue(_BoundaryAnRayon1);
  SpinBoxToreRRev->setSingleStep(_BoundaryAnRayon1/10.);
// Rayon primaire
  SpinBoxToreRPri->setValue(_BoundaryAnRayon2);
  SpinBoxToreRPri->setSingleStep(_BoundaryAnRayon2/10.);
//
  adjustSize();
}
// ---------------------------------------------------
bool SMESH_EditBoundaryAn::CreateOrUpdateBoundaryAn()
//----------------------------------------------------
//  Mise a jour des attributs de la BoundaryAn
{
  switch (_Type)
  {
    case 1 : // il s agit d un cylindre
    {
      aBoundaryAn->SetCylinder(_BoundaryAnXcentre, _BoundaryAnYcentre, _BoundaryAnZcentre, _BoundaryAnXaxis, _BoundaryAnYaxis, _BoundaryAnZaxis, _BoundaryAnRayon );
      break;
    }
    case 2 : // il s agit d une sphere
    {
      aBoundaryAn->SetSphere(_BoundaryAnXcentre, _BoundaryAnYcentre, _BoundaryAnZcentre, _BoundaryAnRayon);
      break;
    }
    case 3 : // il s agit d un cone defini par un axe et un angle
    {
      aBoundaryAn = myHomardGen->CreateBoundaryConeA(CORBA::string_dup(_Name.toStdString().c_str()), \
      _BoundaryAnXaxisCone, _BoundaryAnYaxisCone, _BoundaryAnZaxisCone, _BoundaryAngle, \
      _BoundaryAnXorigCone, _BoundaryAnYorigCone, _BoundaryAnYorigCone);
      break;
    }
    case 4 : // il s agit d un cone defini par les 2 rayons
    {
      aBoundaryAn = myHomardGen->CreateBoundaryConeR(CORBA::string_dup(_Name.toStdString().c_str()), \
        _BoundaryAnXcone1, _BoundaryAnYcone1, _BoundaryAnZcone1, _BoundaryAnRayon1, \
        _BoundaryAnXcone2, _BoundaryAnYcone2, _BoundaryAnZcone2, _BoundaryAnRayon2);
      break;
    }
    case 5 : // il s agit d un tore
    {
      aBoundaryAn->SetTorus(_BoundaryAnXcentre, _BoundaryAnYcentre, _BoundaryAnZcentre, _BoundaryAnXaxis, _BoundaryAnYaxis, _BoundaryAnZaxis, _BoundaryAnRayon1, _BoundaryAnRayon2 );
      break;
    }
  }
  if (Chgt) myHomardGen->InvalideBoundary(_Name.toStdString().c_str());
  //HOMARD_UTILS::updateObjBrowser();
  return true;
}

// --------------------------------------------------------------------------------------
/* Constructs a SMESH_EditBoundaryCAO
    herite de SMESH_CreateBoundaryCAO
*/
// --------------------------------------------------------------------------------------
SMESH_EditBoundaryCAO::SMESH_EditBoundaryCAO( SMESHGUI_HomardAdaptDlg* parent, bool modal,
                                              SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                              QString caseName, QString Name):
    SMESH_CreateBoundaryCAO(parent, modal, myHomardGen0, caseName, Name)
{
    MESSAGE("Debut de Boundary pour " << Name.toStdString().c_str());
    setWindowTitle(QObject::tr("HOM_BOUN_C_EDIT_WINDOW_TITLE"));
    try {
      aBoundary = myHomardGen->GetBoundary(CORBA::string_dup(_aName.toStdString().c_str()));
      InitValEdit();
    }
    catch( SALOME_CMOD::SALOME_Exception& S_ex ) {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr(CORBA::string_dup(S_ex.details.text)) );
      return;
    }

    SMESHHOMARD::ListGroupType_var maListe = aBoundary->GetGroups();
    for ( int i = 0; i < maListe->length(); i++ )
       _listeGroupesBoundary << QString(maListe[i]);

}
// ------------------------------
SMESH_EditBoundaryCAO::~SMESH_EditBoundaryCAO()
// ------------------------------
{
}
// ------------------------------
void SMESH_EditBoundaryCAO::InitValEdit()
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
bool SMESH_EditBoundaryCAO::PushOnApply()
// ------------------------------
{
     return true;
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryCAO::SetFiltrage()
// ------------------------------------------------------------------------
{
  if (!CBGroupe->isChecked()) return;
  //SMESHHOMARD::HOMARD_Cas_var monCas = myHomardGen->GetCase();
  //SMESHHOMARD::ListGroupType_var _listeGroupesCas = monCas->GetGroups();

  SMESH_EditListGroupCAO *aDlg = new SMESH_EditListGroupCAO
    (this, true, SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen),
     "Case_1", _listeGroupesBoundary) ;
  aDlg->show();
}

// ------------------------------------------------------------------------------------
/* Constructs a SMESH_EditBoundaryDi
    herite de SMESH_CreateBoundaryDi
*/
// ------------------------------------------------------------------------------------
SMESH_EditBoundaryDi::SMESH_EditBoundaryDi( SMESHGUI_HomardAdaptDlg* parent, bool modal,
                                            SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                                            QString caseName, QString Name):
  SMESH_CreateBoundaryDi(parent, modal, myHomardGen0, caseName, Name)
{
    MESSAGE("Debut de Boundary pour " << Name.toStdString().c_str());
    setWindowTitle(QObject::tr("HOM_BOUN_D_EDIT_WINDOW_TITLE"));
    try {
      aBoundary = myHomardGen->GetBoundary(CORBA::string_dup(_aName.toStdString().c_str()));
      InitValEdit();
    }
    catch( SALOME_CMOD::SALOME_Exception& S_ex ) {
      QMessageBox::critical( 0, QObject::tr("HOM_ERROR"),
                                QObject::tr(CORBA::string_dup(S_ex.details.text)) );
      return;
    }

    SMESHHOMARD::ListGroupType_var maListe = aBoundary->GetGroups();
    for ( int i = 0; i < maListe->length(); i++ )
       _listeGroupesBoundary << QString(maListe[i]);

}
// ------------------------------
SMESH_EditBoundaryDi::~SMESH_EditBoundaryDi()
// ------------------------------
{
}
// ------------------------------
void SMESH_EditBoundaryDi::InitValEdit()
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
bool SMESH_EditBoundaryDi::PushOnApply()
// ------------------------------
{
     return true;
}
// ------------------------------------------------------------------------
void SMESH_EditBoundaryDi::SetFiltrage()
// // ------------------------------------------------------------------------
{
  if (!CBGroupe->isChecked()) return;
  SMESHHOMARD::HOMARD_Cas_var monCas = myHomardGen->GetCase();
  SMESHHOMARD::ListGroupType_var _listeGroupesCas = monCas->GetGroups();

  SMESH_EditListGroup *aDlg = new SMESH_EditListGroup
    (this, true, SMESHHOMARD::HOMARD_Gen::_duplicate(myHomardGen),
     "Case_1", _listeGroupesBoundary);
  aDlg->show();
}
