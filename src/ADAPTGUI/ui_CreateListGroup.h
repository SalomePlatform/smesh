/********************************************************************************
** Form generated from reading UI file 'CreateListGroup.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATELISTGROUP_H
#define UI_CREATELISTGROUP_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>

QT_BEGIN_NAMESPACE

class Ui_CreateListGroup
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
            CreateListGroup->setObjectName(QStringLiteral("CreateListGroup"));
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
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        GBButtons = new QGroupBox(CreateListGroup);
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


        gridLayout->addWidget(GBButtons, 1, 0, 1, 1);

        GBOptions = new QGroupBox(CreateListGroup);
        GBOptions->setObjectName(QStringLiteral("GBOptions"));
        gridLayout2 = new QGridLayout(GBOptions);
#ifndef Q_OS_MAC
        gridLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout2->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout2->setObjectName(QStringLiteral("gridLayout2"));
        TWGroupe = new QTableWidget(GBOptions);
        if (TWGroupe->columnCount() < 2)
            TWGroupe->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        TWGroupe->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        TWGroupe->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        TWGroupe->setObjectName(QStringLiteral("TWGroupe"));
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
        CreateListGroup->setWindowTitle(QApplication::translate("CreateListGroup", "Selection of groups", Q_NULLPTR));
        GBButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("CreateListGroup", "Help", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("CreateListGroup", "Cancel", Q_NULLPTR));
        buttonApply->setText(QApplication::translate("CreateListGroup", "Apply", Q_NULLPTR));
        buttonOk->setText(QApplication::translate("CreateListGroup", "OK", Q_NULLPTR));
        GBOptions->setTitle(QApplication::translate("CreateListGroup", "Selected groups", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem = TWGroupe->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("CreateListGroup", "Selection", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem1 = TWGroupe->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("CreateListGroup", "Group", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CreateListGroup: public Ui_CreateListGroup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATELISTGROUP_H
