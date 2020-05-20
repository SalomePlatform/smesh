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

#include "MonEditBoundaryAn.h"

#include "SalomeApp_Tools.h"
#include "HOMARDGUI_Utils.h"
#include <utilities.h>

using namespace std;

// ------------------------------------------------------------------------
MonEditBoundaryAn::MonEditBoundaryAn( MonCreateCase* parent, bool modal,
                          HOMARD::HOMARD_Gen_var myHomardGen,
                          QString caseName, QString Name ):
// ------------------------------------------------------------------------
/* Constructs a MonEditBoundaryAn
    herite de MonCreateBoundaryAn
*/
    MonCreateBoundaryAn(parent, myHomardGen, caseName)
{
    MESSAGE("Debut de MonEditBoundaryAn pour " << Name.toStdString().c_str());
    setWindowTitle(QObject::tr("HOM_BOUN_A_EDIT_WINDOW_TITLE"));
    _Name=Name;
    aBoundaryAn = myHomardGen->GetBoundary(_Name.toStdString().c_str());
    InitValEdit();
}
// ------------------------------------------------------------------------
MonEditBoundaryAn::~MonEditBoundaryAn()
// ------------------------------------------------------------------------
{
    // no need to delete child widgets, Qt does it all for us
}
// ------------------------------------------------------------------------
void MonEditBoundaryAn::InitValEdit()
// ------------------------------------------------------------------------
{
  LEName->setText(_Name);
  LEName->setReadOnly(true);
  _Type = aBoundaryAn->GetType();
  MESSAGE("_Type : "<<_Type);
  InitValBoundaryAnLimit();
  if (_aCaseName != QString("")) InitValBoundaryAn();
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
void MonEditBoundaryAn::InitValBoundaryAnLimit()
// ------------------------------------------------------------------------
{
  HOMARD::double_array_var  mesCoordLimits = aBoundaryAn->GetLimit();
  ASSERT(mesCoordLimits->length() == 3 );
  _Xincr=mesCoordLimits[0];
  _Yincr=mesCoordLimits[1];
  _Zincr=mesCoordLimits[2];
}
// ------------------------------------------------------------------------
void MonEditBoundaryAn::InitValBoundaryAnCylindre()
// ------------------------------------------------------------------------
{
  HOMARD::double_array_var  mesCoordBoundary = aBoundaryAn->GetCoords();
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
void MonEditBoundaryAn::InitValBoundaryAnSphere()
// ------------------------------------------------------------------------
{
  HOMARD::double_array_var  mesCoordBoundary = aBoundaryAn->GetCoords();
  ASSERT(mesCoordBoundary->length() == 4 );
  _BoundaryAnXcentre=mesCoordBoundary[0];
  _BoundaryAnYcentre=mesCoordBoundary[1];
  _BoundaryAnZcentre=mesCoordBoundary[2];
  _BoundaryAnRayon=mesCoordBoundary[3];
}
// ------------------------------------------------------------------------
void MonEditBoundaryAn::InitValBoundaryAnConeA()
// ------------------------------------------------------------------------
{
  HOMARD::double_array_var  mesCoordBoundary = aBoundaryAn->GetCoords();
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
void MonEditBoundaryAn::InitValBoundaryAnConeR()
// ------------------------------------------------------------------------
{
  HOMARD::double_array_var  mesCoordBoundary = aBoundaryAn->GetCoords();
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
void MonEditBoundaryAn::InitValBoundaryAnTore()
// ------------------------------------------------------------------------
{
  HOMARD::double_array_var  mesCoordBoundary = aBoundaryAn->GetCoords();
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
void MonEditBoundaryAn::SetCylinder()
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
void MonEditBoundaryAn::SetSphere()
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
void MonEditBoundaryAn::SetConeA()
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
void MonEditBoundaryAn::SetConeR()
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
void MonEditBoundaryAn::SetTore()
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
bool MonEditBoundaryAn::CreateOrUpdateBoundaryAn()
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
  HOMARD_UTILS::updateObjBrowser();
  return true;
}

