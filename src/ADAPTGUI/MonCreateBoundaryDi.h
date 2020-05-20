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

#ifndef MON_CREATEBOUNDARYDI_H
#define MON_CREATEBOUNDARYDI_H

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(HOMARD_Gen)
#include CORBA_CLIENT_HEADER(HOMARD_Boundary)

#include "ui_CreateBoundaryDi.h"
#include <QDialog>

class MonCreateCase;
class HOMARD_EXPORT MonCreateBoundaryDi : public QDialog, public Ui_CreateBoundaryDi
{
    Q_OBJECT

public:
    MonCreateBoundaryDi( MonCreateCase* parent, bool modal,
                         HOMARD::HOMARD_Gen_var myHomardGen,
                         QString caseName, QString BoundaryName );
    ~MonCreateBoundaryDi();
    virtual void setGroups (QStringList listGroup);

protected :

    MonCreateCase *_parent;

    QString _aName;
    QString _aCaseName;


    HOMARD::HOMARD_Boundary_var aBoundary;
    HOMARD::HOMARD_Gen_var myHomardGen;

    QStringList  _listeGroupesBoundary;

    virtual void AssocieLesGroupes();
    virtual void InitConnect();
    virtual void SetNewName();

public slots:

    virtual void SetMeshFile();
    virtual void SetFiltrage();
    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();
};

#endif // MON_CREATEBOUNDARYDI_H
