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

#ifndef MON_CREATEHYPOTHESIS_H
#define MON_CREATEHYPOTHESIS_H

#include "HOMARDGUI_Exports.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(HOMARD_Gen)
#include CORBA_CLIENT_HEADER(HOMARD_Hypothesis)

#include "ui_CreateHypothesis.h"
#include <QScrollArea>

class MonCreateIteration;
class HOMARD_EXPORT MonCreateHypothesis : public QScrollArea, public Ui_CreateHypothesis
{
    Q_OBJECT

public:
    MonCreateHypothesis( MonCreateIteration* parent, bool modal, HOMARD::HOMARD_Gen_var myHomardGen,
                         QString aHypotheseName, QString caseName, QString aFieldFile);
    ~MonCreateHypothesis();

    virtual void addZoneinTWZone(QString newZone);
    virtual void setGroups(QStringList listGroup);

protected :
   // MonCreateHypothesis( MonCreateIteration* parent, HOMARD::HOMARD_Gen_var myHomardGen,
   //                      QString caseName, QString aFieldFile);

    MonCreateIteration *_parent;

    QString _Name;
    QString _aFieldFile ;
    QString _aFieldName;
    QString _aCaseName;

    int _aTypeAdap;
    int _aTypeRaff;
    int _aTypeDera;

    int _TypeThR;
    double _ThreshR;
    int _TypeThC;
    double _ThreshC;

    int _UsField;
    int _UsCmpI;
    int _TypeFieldInterp;

    int _NivMax;
    double _DiamMin;
    int _AdapInit;
    int _ExtraOutput;

    HOMARD::HOMARD_Hypothesis_var aHypothesis;
    HOMARD::HOMARD_Gen_var myHomardGen;

    QStringList _aListeZone;
    QStringList _aListeComposant;
    QStringList _aListeGroupes;

    virtual void InitConnect();
    virtual void InitFields();
    virtual void GetAllZones();
    virtual void SetNewName();

    virtual void AssocieFieldInterp();
    virtual void AssocieLesZones();
    virtual void AssocieComposants();
    virtual void AssocieLesGroupes();
    virtual bool VerifieZone();
    virtual bool VerifieComposant();

    virtual QStringList GetZonesChecked();
    virtual QStringList GetListCompChecked();

public slots:

    virtual void SetUniforme();
    virtual void SetChamp();
    virtual void SetZone();
    virtual void SetUniRaff();
    virtual void SetUniDera();
    virtual void SetFieldName();
    virtual void SetRPE();
    virtual void SetRAbs();
    virtual void SetRRel();
    virtual void SetRMS();
    virtual void SetRNo();
    virtual void SetCPE();
    virtual void SetCAbs();
    virtual void SetCRel();
    virtual void SetCMS();
    virtual void SetCNo();
    virtual void SetUCL2();
    virtual void SetUCInf();
    virtual void SetUseField();
    virtual void PushZoneNew();
    virtual void PushZoneEdit();
    virtual void PushZoneDelete();
    virtual void SetFiltrage();
    virtual void SetFieldNo();
    virtual void SetFieldAll();
    virtual void SetFieldChosen();

    virtual void SetAdvanced();
    virtual void SetAIN();
    virtual void SetAIR();
    virtual void SetAID();

    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();
};

#endif // MON_CREATEHYPOTHESIS_H
