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

#ifndef MON_CREATEZONE_H
#define MON_CREATEZONE_H

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(HOMARD_Gen)

#include "ui_CreateZone.h"
#include <QDialog>

class MonCreateHypothesis;
class HOMARD_EXPORT MonCreateZone : public QDialog, public Ui_CreateZone
{
    Q_OBJECT

public:
    MonCreateZone( MonCreateHypothesis* parent, bool modal,
                   HOMARD::HOMARD_Gen_var myHomardGen,
                   QString caseName);
    virtual ~MonCreateZone();

protected :
    MonCreateZone( MonCreateHypothesis* parent,
                   HOMARD::HOMARD_Gen_var myHomardGen,
                   QString caseName);

    MonCreateHypothesis * _parent;

    QString _Name;
    QString _aCaseName;

    int _Orient;
    int _Type;
    double _Xcentre, _Ycentre, _Zcentre, _Rayon ;
    double _Xmin, _Xmax, _Xincr, _Ymin, _Ymax, _Yincr, _Zmin, _Zmax, _Zincr ;
    double _ZoneXcentre, _ZoneYcentre, _ZoneZcentre, _ZoneRayon ;
    double _ZoneXmin, _ZoneXmax,  _ZoneYmin, _ZoneYmax,  _ZoneZmin, _ZoneZmax ;
    double _Xaxis, _Yaxis, _Zaxis, _RayonInt, _Haut ;
    double _ZoneXaxis, _ZoneYaxis, _ZoneZaxis, _ZoneRayonInt, _ZoneHaut ;
    double _DMax ;

    bool Chgt;

    HOMARD::HOMARD_Zone_var aZone ;
    HOMARD::HOMARD_Gen_var myHomardGen;

    virtual void InitConnect();
    virtual void InitValZone();
    virtual void InitMinMax();
    virtual void SetNewName();
    virtual bool CreateOrUpdateZone();

public slots:
    virtual void SetBox();
    virtual void SetSphere();
    virtual void SetCylinder();
    virtual void SetPipe();
    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();

};

#endif // MON_CREATEZONE_H
