/********************************************************************************
** Form generated from reading UI file 'CreateBoundaryDi.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATEBOUNDARYDI_H
#define UI_CREATEBOUNDARYDI_H

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

class Ui_CreateBoundaryDi
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
    QLabel *Mesh;
    QLineEdit *LEName;
    QLabel *Name;

    void setupUi(QDialog *CreateBoundaryDi)
    {
        if (CreateBoundaryDi->objectName().isEmpty())
            CreateBoundaryDi->setObjectName(QStringLiteral("CreateBoundaryDi"));
        CreateBoundaryDi->resize(566, 169);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(CreateBoundaryDi->sizePolicy().hasHeightForWidth());
        CreateBoundaryDi->setSizePolicy(sizePolicy);
        CreateBoundaryDi->setAutoFillBackground(true);
        CreateBoundaryDi->setSizeGripEnabled(true);
        gridLayout = new QGridLayout(CreateBoundaryDi);
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        GBButtons = new QGroupBox(CreateBoundaryDi);
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

        CBGroupe = new QCheckBox(CreateBoundaryDi);
        CBGroupe->setObjectName(QStringLiteral("CBGroupe"));

        gridLayout->addWidget(CBGroupe, 2, 0, 1, 3);

        LEFileName = new QLineEdit(CreateBoundaryDi);
        LEFileName->setObjectName(QStringLiteral("LEFileName"));
        LEFileName->setMinimumSize(QSize(370, 21));

        gridLayout->addWidget(LEFileName, 1, 2, 1, 1);

        PushFichier = new QPushButton(CreateBoundaryDi);
        PushFichier->setObjectName(QStringLiteral("PushFichier"));

        gridLayout->addWidget(PushFichier, 1, 1, 1, 1);

        Mesh = new QLabel(CreateBoundaryDi);
        Mesh->setObjectName(QStringLiteral("Mesh"));

        gridLayout->addWidget(Mesh, 1, 0, 1, 1);

        LEName = new QLineEdit(CreateBoundaryDi);
        LEName->setObjectName(QStringLiteral("LEName"));
        LEName->setMinimumSize(QSize(382, 21));
        LEName->setMaxLength(32);

        gridLayout->addWidget(LEName, 0, 1, 1, 2);

        Name = new QLabel(CreateBoundaryDi);
        Name->setObjectName(QStringLiteral("Name"));

        gridLayout->addWidget(Name, 0, 0, 1, 1);


        retranslateUi(CreateBoundaryDi);

        QMetaObject::connectSlotsByName(CreateBoundaryDi);
    } // setupUi

    void retranslateUi(QDialog *CreateBoundaryDi)
    {
        CreateBoundaryDi->setWindowTitle(QApplication::translate("CreateBoundaryDi", "Create a discrete boundary", Q_NULLPTR));
        GBButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("CreateBoundaryDi", "Help", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("CreateBoundaryDi", "Cancel", Q_NULLPTR));
        buttonApply->setText(QApplication::translate("CreateBoundaryDi", "Apply", Q_NULLPTR));
        buttonOk->setText(QApplication::translate("CreateBoundaryDi", "OK", Q_NULLPTR));
        CBGroupe->setText(QApplication::translate("CreateBoundaryDi", "Filtering with groups", Q_NULLPTR));
        PushFichier->setText(QString());
        Mesh->setText(QApplication::translate("CreateBoundaryDi", "Mesh", Q_NULLPTR));
        Name->setText(QApplication::translate("CreateBoundaryDi", "Name", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CreateBoundaryDi: public Ui_CreateBoundaryDi {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATEBOUNDARYDI_H
