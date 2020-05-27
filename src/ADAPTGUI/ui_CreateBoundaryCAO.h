/********************************************************************************
** Form generated from reading UI file 'CreateBoundaryCAO.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATEBOUNDARYCAO_H
#define UI_CREATEBOUNDARYCAO_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_CreateBoundaryCAO
{
public:
    QGridLayout *gridLayout;
    QGroupBox *GBButtons;
    QGridLayout *gridLayout1;
    QPushButton *buttonHelp;
    QPushButton *buttonCancel;
    QPushButton *buttonApply;
    QPushButton *buttonOk;
    QCheckBox *CBGroupe;
    QLineEdit *LEFileName;
    QPushButton *PushFichier;
    QLabel *XAO;
    QLineEdit *LEName;
    QLabel *Name;

    void setupUi(QDialog *CreateBoundaryCAO)
    {
        if (CreateBoundaryCAO->objectName().isEmpty())
            CreateBoundaryCAO->setObjectName(QStringLiteral("CreateBoundaryCAO"));
        CreateBoundaryCAO->resize(566, 195);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(CreateBoundaryCAO->sizePolicy().hasHeightForWidth());
        CreateBoundaryCAO->setSizePolicy(sizePolicy);
        CreateBoundaryCAO->setAutoFillBackground(true);
        CreateBoundaryCAO->setSizeGripEnabled(true);
        gridLayout = new QGridLayout(CreateBoundaryCAO);
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        GBButtons = new QGroupBox(CreateBoundaryCAO);
        GBButtons->setObjectName(QStringLiteral("GBButtons"));
        gridLayout1 = new QGridLayout(GBButtons);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QStringLiteral("gridLayout1"));
        buttonHelp = new QPushButton(GBButtons);
        buttonHelp->setObjectName(QStringLiteral("buttonHelp"));

        gridLayout1->addWidget(buttonHelp, 0, 3, 1, 1);

        buttonCancel = new QPushButton(GBButtons);
        buttonCancel->setObjectName(QStringLiteral("buttonCancel"));

        gridLayout1->addWidget(buttonCancel, 0, 2, 1, 1);

        buttonApply = new QPushButton(GBButtons);
        buttonApply->setObjectName(QStringLiteral("buttonApply"));

        gridLayout1->addWidget(buttonApply, 0, 1, 1, 1);

        buttonOk = new QPushButton(GBButtons);
        buttonOk->setObjectName(QStringLiteral("buttonOk"));

        gridLayout1->addWidget(buttonOk, 0, 0, 1, 1);


        gridLayout->addWidget(GBButtons, 3, 0, 1, 3);

        CBGroupe = new QCheckBox(CreateBoundaryCAO);
        CBGroupe->setObjectName(QStringLiteral("CBGroupe"));

        gridLayout->addWidget(CBGroupe, 2, 0, 1, 3);

        LEFileName = new QLineEdit(CreateBoundaryCAO);
        LEFileName->setObjectName(QStringLiteral("LEFileName"));
        LEFileName->setMinimumSize(QSize(370, 21));

        gridLayout->addWidget(LEFileName, 1, 2, 1, 1);

        PushFichier = new QPushButton(CreateBoundaryCAO);
        PushFichier->setObjectName(QStringLiteral("PushFichier"));

        gridLayout->addWidget(PushFichier, 1, 1, 1, 1);

        XAO = new QLabel(CreateBoundaryCAO);
        XAO->setObjectName(QStringLiteral("XAO"));

        gridLayout->addWidget(XAO, 1, 0, 1, 1);

        LEName = new QLineEdit(CreateBoundaryCAO);
        LEName->setObjectName(QStringLiteral("LEName"));
        LEName->setMinimumSize(QSize(382, 21));
        LEName->setMaxLength(32);

        gridLayout->addWidget(LEName, 0, 1, 1, 2);

        Name = new QLabel(CreateBoundaryCAO);
        Name->setObjectName(QStringLiteral("Name"));

        gridLayout->addWidget(Name, 0, 0, 1, 1);


        retranslateUi(CreateBoundaryCAO);

        QMetaObject::connectSlotsByName(CreateBoundaryCAO);
    } // setupUi

    void retranslateUi(QDialog *CreateBoundaryCAO)
    {
        CreateBoundaryCAO->setWindowTitle(QApplication::translate("CreateBoundaryCAO", "Get CAO", Q_NULLPTR));
        GBButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("CreateBoundaryCAO", "Help", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("CreateBoundaryCAO", "Cancel", Q_NULLPTR));
        buttonApply->setText(QApplication::translate("CreateBoundaryCAO", "Apply", Q_NULLPTR));
        buttonOk->setText(QApplication::translate("CreateBoundaryCAO", "OK", Q_NULLPTR));
        CBGroupe->setText(QApplication::translate("CreateBoundaryCAO", "Filtering with groups", Q_NULLPTR));
        PushFichier->setText(QString());
        XAO->setText(QApplication::translate("CreateBoundaryCAO", "XAO", Q_NULLPTR));
        Name->setText(QApplication::translate("CreateBoundaryCAO", "Name", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CreateBoundaryCAO: public Ui_CreateBoundaryCAO {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATEBOUNDARYCAO_H
