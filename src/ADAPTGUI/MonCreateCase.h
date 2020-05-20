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

#ifndef MON_CREATECASE_H
#define MON_CREATECASE_H

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(HOMARD_Gen)
#include CORBA_CLIENT_HEADER(HOMARD_Cas)

#include "ui_CreateCase.h"
#include <QWidget>

class HOMARD_EXPORT MonCreateCase : public QDialog, public Ui_CreateCase
{
    Q_OBJECT

public:
    MonCreateCase( bool modal, HOMARD::HOMARD_Gen_var myHomardGen );
    ~MonCreateCase();

    void AddBoundaryCAO(QString newBoundary);
    void AddBoundaryAn(QString newBoundary);
    void AddBoundaryDi(QString newBoundary);

protected :
    QString _aCaseName;
    QString _aDirName;

    int _ConfType;
    int _ExtType;

    int _Pyram;

    HOMARD::HOMARD_Cas_var aCase ;
    HOMARD::HOMARD_Gen_var myHomardGen;

    virtual void InitConnect();
    virtual void InitBoundarys();
    virtual void SetNewName();

public slots:
    virtual void SetDirName();
    virtual void SetFileName();

    virtual void SetConforme();
    virtual void SetNonConforme();
    virtual void SetStandard();
    virtual void SetBox();
    virtual void SetNC1NpA();
    virtual void SetNCQuelconque();

    virtual void SetMED();
    virtual void SetSaturne();
    virtual void SetSaturne2D();

    virtual void SetBoundaryNo();
    virtual void SetBoundaryCAO();
    virtual void SetBoundaryNonCAO();
    virtual void PushBoundaryCAONew();
    virtual void PushBoundaryCAOEdit();
    virtual void PushBoundaryCAOHelp();
    virtual void SetBoundaryD();
    virtual void PushBoundaryDiNew();
    virtual void PushBoundaryDiEdit();
    virtual void PushBoundaryDiHelp();
    virtual void SetBoundaryA();
    virtual void PushBoundaryAnNew();
    virtual void PushBoundaryAnEdit();
    virtual void PushBoundaryAnHelp();

    virtual void SetAdvanced();

    virtual void PushOnOK();
    virtual bool PushOnApply(int option);
    virtual void PushOnHelp();

    virtual void CaseNameChanged();
};

#endif // MON_CREATECASE_H
