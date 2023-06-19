// Copyright (C) 2011-2023  CEA, EDF
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

#include "SMESH_SMESHGUI.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

//#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)
#include CORBA_CLIENT_HEADER(SMESH_Homard)

#include <QDialog>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>

class SMESH_CreateBoundaryCAO;
class SMESH_CreateBoundaryDi;


QT_BEGIN_NAMESPACE

class SMESH_Ui_CreateListGroup
{
public:
    QGridLayout *gridLayout;
    QGroupBox *GBButtons;
    QGridLayout *gridLayout1;
    QPushButton *buttonHelp;
    QPushButton *buttonCancel;
    QPushButton *buttonApply;
    QPushButton *buttonOk;
    QGroupBox *GBOptions;
    QGridLayout *gridLayout2;
    QTableWidget *TWGroupe;

    void setupUi(QDialog *CreateListGroup)
    {
        if (CreateListGroup->objectName().isEmpty())
            CreateListGroup->setObjectName(QString::fromUtf8("CreateListGroup"));
        CreateListGroup->resize(717, 600);
        QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(CreateListGroup->sizePolicy().hasHeightForWidth());
        CreateListGroup->setSizePolicy(sizePolicy);
        CreateListGroup->setAutoFillBackground(true);
        CreateListGroup->setSizeGripEnabled(true);
        gridLayout = new QGridLayout(CreateListGroup);
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        GBButtons = new QGroupBox(CreateListGroup);
        GBButtons->setObjectName(QString::fromUtf8("GBButtons"));
        gridLayout1 = new QGridLayout(GBButtons);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        buttonHelp = new QPushButton(GBButtons);
        buttonHelp->setObjectName(QString::fromUtf8("buttonHelp"));

        gridLayout1->addWidget(buttonHelp, 0, 3, 1, 1);

        buttonCancel = new QPushButton(GBButtons);
        buttonCancel->setObjectName(QString::fromUtf8("buttonCancel"));

        gridLayout1->addWidget(buttonCancel, 0, 2, 1, 1);

        buttonApply = new QPushButton(GBButtons);
        buttonApply->setObjectName(QString::fromUtf8("buttonApply"));

        gridLayout1->addWidget(buttonApply, 0, 1, 1, 1);

        buttonOk = new QPushButton(GBButtons);
        buttonOk->setObjectName(QString::fromUtf8("buttonOk"));

        gridLayout1->addWidget(buttonOk, 0, 0, 1, 1);


        gridLayout->addWidget(GBButtons, 1, 0, 1, 1);

        GBOptions = new QGroupBox(CreateListGroup);
        GBOptions->setObjectName(QString::fromUtf8("GBOptions"));
        gridLayout2 = new QGridLayout(GBOptions);
#ifndef Q_OS_MAC
        gridLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout2->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
        TWGroupe = new QTableWidget(GBOptions);
        if (TWGroupe->columnCount() < 2)
            TWGroupe->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        TWGroupe->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        TWGroupe->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        TWGroupe->setObjectName(QString::fromUtf8("TWGroupe"));
        TWGroupe->setShowGrid(true);
        TWGroupe->setRowCount(0);
        TWGroupe->setColumnCount(2);

        gridLayout2->addWidget(TWGroupe, 0, 0, 1, 1);


        gridLayout->addWidget(GBOptions, 0, 0, 1, 1);


        retranslateUi(CreateListGroup);

        QMetaObject::connectSlotsByName(CreateListGroup);
    } // setupUi

    void retranslateUi(QDialog *CreateListGroup)
    {
        CreateListGroup->setWindowTitle(QApplication::translate("CreateListGroup", "Selection of groups", nullptr));
        GBButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("CreateListGroup", "Help", nullptr));
        buttonCancel->setText(QApplication::translate("CreateListGroup", "Cancel", nullptr));
        buttonApply->setText(QApplication::translate("CreateListGroup", "Apply", nullptr));
        buttonOk->setText(QApplication::translate("CreateListGroup", "OK", nullptr));
        GBOptions->setTitle(QApplication::translate("CreateListGroup", "Selected groups", nullptr));
        QTableWidgetItem *___qtablewidgetitem = TWGroupe->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("CreateListGroup", "Selection", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = TWGroupe->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("CreateListGroup", "Group", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CreateListGroup: public SMESH_Ui_CreateListGroup {};
} // namespace Ui

QT_END_NAMESPACE

class SMESHGUI_EXPORT SMESH_CreateListGroupCAO : public QDialog, public SMESH_Ui_CreateListGroup
{
    Q_OBJECT

public:
    SMESH_CreateListGroupCAO( SMESH_CreateBoundaryCAO* parentBound, bool modal, SMESHHOMARD::HOMARD_Gen_var myHomardGen, QString aCaseName,  QStringList listeGroupesHypo);
    SMESH_CreateListGroupCAO( SMESH_CreateBoundaryCAO* parentBound, SMESHHOMARD::HOMARD_Gen_var myHomardGen, QString aCaseName, QStringList listeGroupesHypo);
    virtual ~SMESH_CreateListGroupCAO();

protected :

    SMESHHOMARD::HOMARD_Gen_var myHomardGen;

    SMESH_CreateBoundaryCAO * _parentBound;
    QString _aCaseName;
    QStringList _listeGroupesHypo;

    virtual void InitConnect();
    virtual void InitGroupes();

public slots:
    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();

};

class SMESHGUI_EXPORT SMESH_CreateListGroup : public QDialog, public SMESH_Ui_CreateListGroup
{
    Q_OBJECT

public:
    SMESH_CreateListGroup( SMESH_CreateBoundaryDi* parentBound, bool modal, SMESHHOMARD::HOMARD_Gen_var myHomardGen, QString aCaseName,  QStringList listeGroupesHypo);
    SMESH_CreateListGroup( SMESH_CreateBoundaryDi* parentBound, SMESHHOMARD::HOMARD_Gen_var myHomardGen, QString aCaseName, QStringList listeGroupesHypo);
    virtual ~SMESH_CreateListGroup();

protected :

    SMESHHOMARD::HOMARD_Gen_var myHomardGen;

    SMESH_CreateBoundaryDi * _parentBound;
    QString _aCaseName;
    QStringList _listeGroupesHypo;

    virtual void InitConnect();
    virtual void InitGroupes();

public slots:
    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();

};

class SMESHGUI_EXPORT SMESH_EditListGroupCAO : public SMESH_CreateListGroupCAO
{
    Q_OBJECT

public:
    SMESH_EditListGroupCAO( SMESH_CreateBoundaryCAO* parentBound, bool modal, SMESHHOMARD::HOMARD_Gen_var myHomardGen, QString aCaseName,  QStringList listeGroupesHypo);
    virtual ~SMESH_EditListGroupCAO();

protected :

    virtual void InitGroupes();

};

class SMESHGUI_EXPORT SMESH_EditListGroup : public SMESH_CreateListGroup
{
    Q_OBJECT

public:
    SMESH_EditListGroup( SMESH_CreateBoundaryDi* parentBound, bool modal,
                      SMESHHOMARD::HOMARD_Gen_var myHomardGen,
                      QString aCaseName, QStringList listeGroupesHypo);
    virtual ~SMESH_EditListGroup();

protected :

    virtual void InitGroupes();

};

#endif // MON_CREATELISTGROUPCAO_H
