/********************************************************************************
** Form generated from reading UI file 'IterInfo.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ITERINFO_H
#define UI_ITERINFO_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_IterInfo
{
public:
    QGridLayout *gridLayout_2;
    QGroupBox *GBOptions;
    QGridLayout *gridLayout;
    QCheckBox *CBQuality;
    QCheckBox *CBConnection;
    QCheckBox *CBDiametre;
    QCheckBox *CBBlockSize;
    QCheckBox *CBEntanglement;
    QGroupBox *GroupButtons;
    QGridLayout *gridLayout1;
    QPushButton *buttonHelp;
    QPushButton *buttonApply;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;

    void setupUi(QDialog *IterInfo)
    {
        if (IterInfo->objectName().isEmpty())
            IterInfo->setObjectName(QStringLiteral("IterInfo"));
        IterInfo->resize(420, 220);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(IterInfo->sizePolicy().hasHeightForWidth());
        IterInfo->setSizePolicy(sizePolicy);
        IterInfo->setAutoFillBackground(true);
        gridLayout_2 = new QGridLayout(IterInfo);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        GBOptions = new QGroupBox(IterInfo);
        GBOptions->setObjectName(QStringLiteral("GBOptions"));
        gridLayout = new QGridLayout(GBOptions);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        CBQuality = new QCheckBox(GBOptions);
        CBQuality->setObjectName(QStringLiteral("CBQuality"));

        gridLayout->addWidget(CBQuality, 0, 0, 1, 1);

        CBConnection = new QCheckBox(GBOptions);
        CBConnection->setObjectName(QStringLiteral("CBConnection"));

        gridLayout->addWidget(CBConnection, 0, 1, 1, 1);

        CBDiametre = new QCheckBox(GBOptions);
        CBDiametre->setObjectName(QStringLiteral("CBDiametre"));

        gridLayout->addWidget(CBDiametre, 1, 0, 1, 1);

        CBBlockSize = new QCheckBox(GBOptions);
        CBBlockSize->setObjectName(QStringLiteral("CBBlockSize"));

        gridLayout->addWidget(CBBlockSize, 1, 1, 1, 1);

        CBEntanglement = new QCheckBox(GBOptions);
        CBEntanglement->setObjectName(QStringLiteral("CBEntanglement"));

        gridLayout->addWidget(CBEntanglement, 2, 0, 1, 2);


        gridLayout_2->addWidget(GBOptions, 0, 0, 2, 2);

        GroupButtons = new QGroupBox(IterInfo);
        GroupButtons->setObjectName(QStringLiteral("GroupButtons"));
        gridLayout1 = new QGridLayout(GroupButtons);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QStringLiteral("gridLayout1"));
        buttonHelp = new QPushButton(GroupButtons);
        buttonHelp->setObjectName(QStringLiteral("buttonHelp"));
        buttonHelp->setAutoDefault(false);

        gridLayout1->addWidget(buttonHelp, 0, 4, 1, 1);

        buttonApply = new QPushButton(GroupButtons);
        buttonApply->setObjectName(QStringLiteral("buttonApply"));
        buttonApply->setAutoDefault(false);

        gridLayout1->addWidget(buttonApply, 0, 1, 1, 1);

        buttonOk = new QPushButton(GroupButtons);
        buttonOk->setObjectName(QStringLiteral("buttonOk"));
        buttonOk->setAutoDefault(false);

        gridLayout1->addWidget(buttonOk, 0, 0, 1, 1);

        buttonCancel = new QPushButton(GroupButtons);
        buttonCancel->setObjectName(QStringLiteral("buttonCancel"));
        buttonCancel->setAutoDefault(false);

        gridLayout1->addWidget(buttonCancel, 0, 2, 1, 1);


        gridLayout_2->addWidget(GroupButtons, 2, 0, 1, 2);


        retranslateUi(IterInfo);

        QMetaObject::connectSlotsByName(IterInfo);
    } // setupUi

    void retranslateUi(QDialog *IterInfo)
    {
        IterInfo->setWindowTitle(QApplication::translate("IterInfo", "Information on a mesh", Q_NULLPTR));
        GBOptions->setTitle(QApplication::translate("IterInfo", "Options", Q_NULLPTR));
        CBQuality->setText(QApplication::translate("IterInfo", "Quality", Q_NULLPTR));
        CBConnection->setText(QApplication::translate("IterInfo", "Connection", Q_NULLPTR));
        CBDiametre->setText(QApplication::translate("IterInfo", "Diametre", Q_NULLPTR));
        CBBlockSize->setText(QApplication::translate("IterInfo", "Group size", Q_NULLPTR));
        CBEntanglement->setText(QApplication::translate("IterInfo", "Entanglement", Q_NULLPTR));
        GroupButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("IterInfo", "Help", Q_NULLPTR));
        buttonApply->setText(QApplication::translate("IterInfo", "Apply", Q_NULLPTR));
        buttonOk->setText(QApplication::translate("IterInfo", "OK", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("IterInfo", "Cancel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class IterInfo: public Ui_IterInfo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ITERINFO_H
