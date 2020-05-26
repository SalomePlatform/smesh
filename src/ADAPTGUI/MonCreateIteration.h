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

#ifndef MON_CREATEITERATION_H
#define MON_CREATEITERATION_H

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(ADAPT_Gen)
#include CORBA_CLIENT_HEADER(HOMARD_Cas)

#include "ui_CreateIteration.h"
#include <QScrollArea>

class HOMARD_EXPORT MonCreateIteration : public QScrollArea, public Ui_CreateIteration
{
    Q_OBJECT

public:
    MonCreateIteration( QWidget* parent, bool modal, ADAPT::ADAPT_Gen_var myHomardGen, QString IterParentName );
    ~MonCreateIteration();

    void addHypothese(QString newHypothese);


protected :
    MonCreateIteration( QWidget* parent, ADAPT::ADAPT_Gen_var myHomardGen, QString IterParentName );

    QString _Name;
    QString _IterParentName;
    QString _CaseName;


    ADAPT::HOMARD_Iteration_var aIter ;
    ADAPT::HOMARD_Iteration_var aIterParent ;
    ADAPT::HOMARD_Cas_var aCas ;
    ADAPT::ADAPT_Gen_var myHomardGen;


    virtual void InitConnect();
    virtual void GetHypotheses();
    virtual void SetNewName();

public slots:
    virtual void SetIterParentName();
    virtual void PushHypoEdit();
    virtual void PushHypoNew();
    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();
    virtual void SetFieldFile();
    virtual void SetTSNo();
    virtual void SetTSLast();
    virtual void SetTSChosen();

};

#endif // MON_CREATEITERATION_H
