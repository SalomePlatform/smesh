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

#ifndef MON_CREATEBOUNDARYAN_H
#define MON_CREATEBOUNDARYAN_H

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(HOMARD_Gen)
#include CORBA_CLIENT_HEADER(HOMARD_Boundary)

#include "ui_CreateBoundaryAn.h"
#include <QDialog>

class MonCreateCase;
class HOMARD_EXPORT MonCreateBoundaryAn : public QDialog, public Ui_CreateBoundaryAn
{
    Q_OBJECT

public:
    MonCreateBoundaryAn( MonCreateCase* parent, bool modal,
                   HOMARD::HOMARD_Gen_var myHomardGen,
                   QString caseName);
    virtual ~MonCreateBoundaryAn();

protected :
    MonCreateBoundaryAn( MonCreateCase* parent,
                   HOMARD::HOMARD_Gen_var myHomardGen,
                   QString caseName);

    MonCreateCase * _parent;

    QString _Name;
    QString _aCaseName;

    int _Type;
    double _BoundaryAnXcentre, _BoundaryAnYcentre, _BoundaryAnZcentre, _BoundaryAnRayon;
    double _BoundaryAnXaxis, _BoundaryAnYaxis, _BoundaryAnZaxis;
    double _Xcentre, _Ycentre, _Zcentre, _Rayon ;
    double _Xmin, _Xmax, _Xincr, _Ymin, _Ymax, _Yincr, _Zmin, _Zmax, _Zincr, _DMax ;
    double _BoundaryAnXcone1, _BoundaryAnYcone1, _BoundaryAnZcone1, _BoundaryAnRayon1;
    double _BoundaryAnXcone2, _BoundaryAnYcone2, _BoundaryAnZcone2, _BoundaryAnRayon2;
    double _BoundaryAnXaxisCone, _BoundaryAnYaxisCone, _BoundaryAnZaxisCone;
    double _BoundaryAnXorigCone, _BoundaryAnYorigCone, _BoundaryAnZorigCone;
    double _BoundaryAngle;
    double _BoundaryAnToreXcentre, _BoundaryAnToreYcentre, _BoundaryAnToreZcentre;
    double _BoundaryAnToreXaxe, _BoundaryAnToreYaxe, _BoundaryAnToreZaxe;
    double _BoundaryAnToreRRev, _BoundaryAnToreRPri;


    bool Chgt;

    HOMARD::HOMARD_Boundary_var aBoundaryAn ;
    HOMARD::HOMARD_Gen_var myHomardGen;

    virtual void InitConnect();
    virtual void InitValBoundaryAn();
    virtual void InitMinMax();
    virtual void SetNewName();
    virtual bool CreateOrUpdateBoundaryAn();
    virtual void convertRayonAngle(int option);

public slots:
    virtual void SetCylinder();
    virtual void SetSphere();
    virtual void SetCone();
    virtual void SetConeR();
    virtual void SetConeA();
    virtual void SetTore();
    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();

};

#endif // MON_CREATEBOUNDARYAN_H
