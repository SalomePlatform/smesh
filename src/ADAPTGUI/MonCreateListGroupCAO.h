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

#ifndef MON_CREATELISTGROUPCAO_H
#define MON_CREATELISTGROUPCAO_H

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(ADAPT_Gen)

#include "ui_CreateListGroup.h"
#include <QDialog>

class MonCreateHypothesis;
class MonCreateBoundaryCAO;
class HOMARD_EXPORT MonCreateListGroupCAO : public QDialog, public Ui_CreateListGroup
{
    Q_OBJECT

public:
    MonCreateListGroupCAO( MonCreateHypothesis* parentHyp, MonCreateBoundaryCAO* parentBound, bool modal, ADAPT::ADAPT_Gen_var myHomardGen, QString aCaseName,  QStringList listeGroupesHypo);
    MonCreateListGroupCAO( MonCreateHypothesis* parentHyp, MonCreateBoundaryCAO* parentBound, ADAPT::ADAPT_Gen_var myHomardGen, QString aCaseName, QStringList listeGroupesHypo);
    virtual ~MonCreateListGroupCAO();

protected :

    ADAPT::ADAPT_Gen_var myHomardGen;

    MonCreateHypothesis * _parentHyp;
    MonCreateBoundaryCAO * _parentBound;
    QString _aCaseName;
    QStringList _listeGroupesHypo;

    virtual void InitConnect();
    virtual void InitGroupes();

public slots:
    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();

};

#endif // MON_CREATELISTGROUPCAO_H
