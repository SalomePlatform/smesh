/********************************************************************************
** Form generated from reading UI file 'PursueIteration.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PURSUEITERATION_H
#define UI_PURSUEITERATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_PursueIteration
{
public:
    QGridLayout *gridLayout_2;
    QHBoxLayout *hboxLayout;
    QLabel *Name;
    QLineEdit *LECaseName;
    QHBoxLayout *_2;
    QLabel *Directory;
    QPushButton *PushDir;
    QLineEdit *LEDirName;
    QRadioButton *RBIteration;
    QRadioButton *RBCase;
    QHBoxLayout *hboxLayout1;
    QLabel *DirectoryStart;
    QPushButton *PushDirStart;
    QLineEdit *LEDirNameStart;
    QGroupBox *GBIterationintoCase;
    QGridLayout *gridLayout;
    QRadioButton *RBCaseLastIteration;
    QRadioButton *RBCaseNIteration;
    QSpinBox *SpinBoxNumber;
    QGroupBox *GroupButtons;
    QGridLayout *gridLayout1;
    QPushButton *buttonHelp;
    QPushButton *buttonApply;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;
    QSpacerItem *spacer_2;

    void setupUi(QDialog *PursueIteration)
    {
        if (PursueIteration->objectName().isEmpty())
            PursueIteration->setObjectName(QStringLiteral("PursueIteration"));
        PursueIteration->resize(601, 300);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(PursueIteration->sizePolicy().hasHeightForWidth());
        PursueIteration->setSizePolicy(sizePolicy);
        PursueIteration->setAutoFillBackground(true);
        gridLayout_2 = new QGridLayout(PursueIteration);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout->setContentsMargins(0, 0, 0, 0);
#endif
        hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
        Name = new QLabel(PursueIteration);
        Name->setObjectName(QStringLiteral("Name"));

        hboxLayout->addWidget(Name);

        LECaseName = new QLineEdit(PursueIteration);
        LECaseName->setObjectName(QStringLiteral("LECaseName"));
        LECaseName->setMinimumSize(QSize(382, 21));

        hboxLayout->addWidget(LECaseName);


        gridLayout_2->addLayout(hboxLayout, 0, 0, 1, 2);

        _2 = new QHBoxLayout();
#ifndef Q_OS_MAC
        _2->setSpacing(6);
#endif
        _2->setContentsMargins(0, 0, 0, 0);
        _2->setObjectName(QStringLiteral("_2"));
        Directory = new QLabel(PursueIteration);
        Directory->setObjectName(QStringLiteral("Directory"));

        _2->addWidget(Directory);

        PushDir = new QPushButton(PursueIteration);
        PushDir->setObjectName(QStringLiteral("PushDir"));
        PushDir->setAutoDefault(false);

        _2->addWidget(PushDir);

        LEDirName = new QLineEdit(PursueIteration);
        LEDirName->setObjectName(QStringLiteral("LEDirName"));
        LEDirName->setMinimumSize(QSize(382, 21));

        _2->addWidget(LEDirName);


        gridLayout_2->addLayout(_2, 1, 0, 1, 3);

        RBIteration = new QRadioButton(PursueIteration);
        RBIteration->setObjectName(QStringLiteral("RBIteration"));
        RBIteration->setChecked(true);

        gridLayout_2->addWidget(RBIteration, 2, 0, 1, 1);

        RBCase = new QRadioButton(PursueIteration);
        RBCase->setObjectName(QStringLiteral("RBCase"));

        gridLayout_2->addWidget(RBCase, 2, 1, 1, 1);

        hboxLayout1 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout1->setSpacing(6);
#endif
        hboxLayout1->setContentsMargins(0, 0, 0, 0);
        hboxLayout1->setObjectName(QStringLiteral("hboxLayout1"));
        DirectoryStart = new QLabel(PursueIteration);
        DirectoryStart->setObjectName(QStringLiteral("DirectoryStart"));

        hboxLayout1->addWidget(DirectoryStart);

        PushDirStart = new QPushButton(PursueIteration);
        PushDirStart->setObjectName(QStringLiteral("PushDirStart"));
        PushDirStart->setAutoDefault(false);

        hboxLayout1->addWidget(PushDirStart);

        LEDirNameStart = new QLineEdit(PursueIteration);
        LEDirNameStart->setObjectName(QStringLiteral("LEDirNameStart"));
        LEDirNameStart->setMinimumSize(QSize(382, 21));

        hboxLayout1->addWidget(LEDirNameStart);


        gridLayout_2->addLayout(hboxLayout1, 3, 0, 1, 3);

        GBIterationintoCase = new QGroupBox(PursueIteration);
        GBIterationintoCase->setObjectName(QStringLiteral("GBIterationintoCase"));
        gridLayout = new QGridLayout(GBIterationintoCase);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        RBCaseLastIteration = new QRadioButton(GBIterationintoCase);
        RBCaseLastIteration->setObjectName(QStringLiteral("RBCaseLastIteration"));
        RBCaseLastIteration->setChecked(true);

        gridLayout->addWidget(RBCaseLastIteration, 0, 0, 1, 1);

        RBCaseNIteration = new QRadioButton(GBIterationintoCase);
        RBCaseNIteration->setObjectName(QStringLiteral("RBCaseNIteration"));

        gridLayout->addWidget(RBCaseNIteration, 1, 0, 1, 1);

        SpinBoxNumber = new QSpinBox(GBIterationintoCase);
        SpinBoxNumber->setObjectName(QStringLiteral("SpinBoxNumber"));
        SpinBoxNumber->setMaximum(1789);

        gridLayout->addWidget(SpinBoxNumber, 1, 1, 1, 1);


        gridLayout_2->addWidget(GBIterationintoCase, 4, 0, 1, 2);

        GroupButtons = new QGroupBox(PursueIteration);
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


        gridLayout_2->addWidget(GroupButtons, 5, 0, 1, 2);

        spacer_2 = new QSpacerItem(128, 25, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_2->addItem(spacer_2, 5, 2, 1, 1);


        retranslateUi(PursueIteration);

        QMetaObject::connectSlotsByName(PursueIteration);
    } // setupUi

    void retranslateUi(QDialog *PursueIteration)
    {
        PursueIteration->setWindowTitle(QApplication::translate("PursueIteration", "Pursue an iteration", Q_NULLPTR));
        Name->setText(QApplication::translate("PursueIteration", "Name", Q_NULLPTR));
        Directory->setText(QApplication::translate("PursueIteration", "Directory", Q_NULLPTR));
        PushDir->setText(QString());
        RBIteration->setText(QApplication::translate("PursueIteration", "From an iteration", Q_NULLPTR));
        RBCase->setText(QApplication::translate("PursueIteration", "From a case", Q_NULLPTR));
        DirectoryStart->setText(QApplication::translate("PursueIteration", "Directory", Q_NULLPTR));
        PushDirStart->setText(QString());
        GBIterationintoCase->setTitle(QApplication::translate("PursueIteration", "Iteration into the case", Q_NULLPTR));
        RBCaseLastIteration->setText(QApplication::translate("PursueIteration", "Last iteration", Q_NULLPTR));
        RBCaseNIteration->setText(QApplication::translate("PursueIteration", "Iteration number", Q_NULLPTR));
        GroupButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("PursueIteration", "Help", Q_NULLPTR));
        buttonApply->setText(QApplication::translate("PursueIteration", "Apply", Q_NULLPTR));
        buttonOk->setText(QApplication::translate("PursueIteration", "OK", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("PursueIteration", "Cancel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class PursueIteration: public Ui_PursueIteration {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PURSUEITERATION_H
