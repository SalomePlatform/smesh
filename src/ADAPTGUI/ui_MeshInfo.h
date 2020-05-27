/********************************************************************************
** Form generated from reading UI file 'MeshInfo.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MESHINFO_H
#define UI_MESHINFO_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_MeshInfo
{
public:
    QGridLayout *gridLayout_2;
    QHBoxLayout *hboxLayout;
    QLabel *Name;
    QLineEdit *LECaseName;
    QHBoxLayout *hboxLayout1;
    QLabel *Directory;
    QPushButton *PushDir;
    QLineEdit *LEDirName;
    QSpacerItem *spacerItem;
    QHBoxLayout *hboxLayout2;
    QLabel *Mesh_2;
    QPushButton *PushFichier;
    QLineEdit *LEFileName;
    QSpacerItem *spacerItem1;
    QGroupBox *GBOptions;
    QGridLayout *gridLayout;
    QCheckBox *CBQuality;
    QCheckBox *CBConnection;
    QCheckBox *CBDiametre;
    QCheckBox *CBBlockSize;
    QCheckBox *CBEntanglement;
    QSpacerItem *verticalSpacer;
    QGroupBox *GroupButtons;
    QGridLayout *gridLayout1;
    QPushButton *buttonHelp;
    QPushButton *buttonApply;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;
    QSpacerItem *spacer_2;

    void setupUi(QDialog *MeshInfo)
    {
        if (MeshInfo->objectName().isEmpty())
            MeshInfo->setObjectName(QStringLiteral("MeshInfo"));
        MeshInfo->resize(536, 372);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MeshInfo->sizePolicy().hasHeightForWidth());
        MeshInfo->setSizePolicy(sizePolicy);
        MeshInfo->setAutoFillBackground(true);
        gridLayout_2 = new QGridLayout(MeshInfo);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout->setContentsMargins(0, 0, 0, 0);
#endif
        hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
        Name = new QLabel(MeshInfo);
        Name->setObjectName(QStringLiteral("Name"));

        hboxLayout->addWidget(Name);

        LECaseName = new QLineEdit(MeshInfo);
        LECaseName->setObjectName(QStringLiteral("LECaseName"));
        LECaseName->setMinimumSize(QSize(382, 21));

        hboxLayout->addWidget(LECaseName);


        gridLayout_2->addLayout(hboxLayout, 0, 0, 1, 2);

        hboxLayout1 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout1->setSpacing(6);
#endif
        hboxLayout1->setContentsMargins(0, 0, 0, 0);
        hboxLayout1->setObjectName(QStringLiteral("hboxLayout1"));
        Directory = new QLabel(MeshInfo);
        Directory->setObjectName(QStringLiteral("Directory"));

        hboxLayout1->addWidget(Directory);

        PushDir = new QPushButton(MeshInfo);
        PushDir->setObjectName(QStringLiteral("PushDir"));
        PushDir->setAutoDefault(false);

        hboxLayout1->addWidget(PushDir);

        LEDirName = new QLineEdit(MeshInfo);
        LEDirName->setObjectName(QStringLiteral("LEDirName"));
        LEDirName->setMinimumSize(QSize(382, 21));

        hboxLayout1->addWidget(LEDirName);


        gridLayout_2->addLayout(hboxLayout1, 1, 0, 1, 2);

        spacerItem = new QSpacerItem(20, 18, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(spacerItem, 2, 0, 1, 1);

        hboxLayout2 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout2->setSpacing(6);
#endif
        hboxLayout2->setContentsMargins(0, 0, 0, 0);
        hboxLayout2->setObjectName(QStringLiteral("hboxLayout2"));
        Mesh_2 = new QLabel(MeshInfo);
        Mesh_2->setObjectName(QStringLiteral("Mesh_2"));

        hboxLayout2->addWidget(Mesh_2);

        PushFichier = new QPushButton(MeshInfo);
        PushFichier->setObjectName(QStringLiteral("PushFichier"));
        PushFichier->setAutoDefault(false);

        hboxLayout2->addWidget(PushFichier);

        LEFileName = new QLineEdit(MeshInfo);
        LEFileName->setObjectName(QStringLiteral("LEFileName"));
        LEFileName->setMinimumSize(QSize(382, 21));

        hboxLayout2->addWidget(LEFileName);


        gridLayout_2->addLayout(hboxLayout2, 3, 0, 1, 2);

        spacerItem1 = new QSpacerItem(20, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(spacerItem1, 4, 0, 1, 1);

        GBOptions = new QGroupBox(MeshInfo);
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


        gridLayout_2->addWidget(GBOptions, 5, 0, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer, 6, 0, 1, 1);

        GroupButtons = new QGroupBox(MeshInfo);
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


        gridLayout_2->addWidget(GroupButtons, 7, 0, 1, 1);

        spacer_2 = new QSpacerItem(128, 25, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_2->addItem(spacer_2, 7, 1, 1, 1);


        retranslateUi(MeshInfo);

        QMetaObject::connectSlotsByName(MeshInfo);
    } // setupUi

    void retranslateUi(QDialog *MeshInfo)
    {
        MeshInfo->setWindowTitle(QApplication::translate("MeshInfo", "Information on a mesh", Q_NULLPTR));
        Name->setText(QApplication::translate("MeshInfo", "Name", Q_NULLPTR));
        Directory->setText(QApplication::translate("MeshInfo", "Directory", Q_NULLPTR));
        PushDir->setText(QString());
        Mesh_2->setText(QApplication::translate("MeshInfo", "Mesh", Q_NULLPTR));
        PushFichier->setText(QString());
        GBOptions->setTitle(QApplication::translate("MeshInfo", "Options", Q_NULLPTR));
        CBQuality->setText(QApplication::translate("MeshInfo", "Quality", Q_NULLPTR));
        CBConnection->setText(QApplication::translate("MeshInfo", "Connection", Q_NULLPTR));
        CBDiametre->setText(QApplication::translate("MeshInfo", "Diametre", Q_NULLPTR));
        CBBlockSize->setText(QApplication::translate("MeshInfo", "Group size", Q_NULLPTR));
        CBEntanglement->setText(QApplication::translate("MeshInfo", "Entanglement", Q_NULLPTR));
        GroupButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("MeshInfo", "Help", Q_NULLPTR));
        buttonApply->setText(QApplication::translate("MeshInfo", "Apply", Q_NULLPTR));
        buttonOk->setText(QApplication::translate("MeshInfo", "OK", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("MeshInfo", "Cancel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MeshInfo: public Ui_MeshInfo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MESHINFO_H
