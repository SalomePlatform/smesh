/********************************************************************************
** Form generated from reading UI file 'CreateIteration.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATEITERATION_H
#define UI_CREATEITERATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CreateIteration
{
public:
    QWidget *scrollAreaWidgetContents;
    QGridLayout *gridLayout;
    QLabel *Iteration_Name;
    QSpacerItem *horizontalSpacer_6;
    QLineEdit *LEName;
    QSpacerItem *horizontalSpacer_3;
    QLabel *Iter_Parent;
    QPushButton *PBIterParent;
    QLineEdit *LEIterationParentName;
    QLabel *Mesh_n;
    QSpacerItem *horizontalSpacer_7;
    QLineEdit *LEMeshName_n;
    QSpacerItem *horizontalSpacer_4;
    QLabel *Mesh_np1;
    QSpacerItem *horizontalSpacer_8;
    QLineEdit *LEMeshName_np1;
    QSpacerItem *horizontalSpacer_5;
    QSpacerItem *verticalSpacer;
    QGroupBox *GBField;
    QGridLayout *gridLayout1;
    QLineEdit *LEFieldFile;
    QSpacerItem *spacerItem;
    QRadioButton *RBChosen;
    QSpacerItem *spacerItem1;
    QSpinBox *SpinBox_Rank;
    QLabel *Rank;
    QSpacerItem *spacerItem2;
    QSpinBox *SpinBox_TimeStep;
    QLabel *TimeStep;
    QSpacerItem *spacerItem3;
    QRadioButton *RBLast;
    QRadioButton *RBNo;
    QPushButton *PushFieldFile;
    QLabel *FieldFile;
    QSpacerItem *horizontalSpacer_10;
    QSpacerItem *spacerItem4;
    QGroupBox *Hypothese;
    QGridLayout *gridLayout2;
    QComboBox *CBHypothese;
    QSpacerItem *spacerItem5;
    QPushButton *PBHypoEdit;
    QPushButton *PBHypoNew;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *spacerItem6;
    QGroupBox *GroupButtons;
    QGridLayout *gridLayout3;
    QPushButton *buttonOk;
    QPushButton *buttonApply;
    QPushButton *buttonCancel;
    QPushButton *buttonHelp;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *horizontalSpacer_9;

    void setupUi(QScrollArea *CreateIteration)
    {
        if (CreateIteration->objectName().isEmpty())
            CreateIteration->setObjectName(QStringLiteral("CreateIteration"));
        CreateIteration->resize(777, 668);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(CreateIteration->sizePolicy().hasHeightForWidth());
        CreateIteration->setSizePolicy(sizePolicy);
        CreateIteration->setMinimumSize(QSize(750, 400));
        CreateIteration->setSizeIncrement(QSize(1, 1));
        CreateIteration->setBaseSize(QSize(750, 400));
        CreateIteration->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 773, 664));
        scrollAreaWidgetContents->setProperty("sizeGripEnabled", QVariant(true));
        gridLayout = new QGridLayout(scrollAreaWidgetContents);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        Iteration_Name = new QLabel(scrollAreaWidgetContents);
        Iteration_Name->setObjectName(QStringLiteral("Iteration_Name"));

        gridLayout->addWidget(Iteration_Name, 0, 0, 1, 2);

        horizontalSpacer_6 = new QSpacerItem(117, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_6, 0, 2, 1, 1);

        LEName = new QLineEdit(scrollAreaWidgetContents);
        LEName->setObjectName(QStringLiteral("LEName"));
        LEName->setMinimumSize(QSize(290, 21));
        LEName->setMaxLength(64);

        gridLayout->addWidget(LEName, 0, 3, 1, 2);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_3, 0, 5, 1, 1);

        Iter_Parent = new QLabel(scrollAreaWidgetContents);
        Iter_Parent->setObjectName(QStringLiteral("Iter_Parent"));

        gridLayout->addWidget(Iter_Parent, 1, 0, 1, 2);

        PBIterParent = new QPushButton(scrollAreaWidgetContents);
        PBIterParent->setObjectName(QStringLiteral("PBIterParent"));
        PBIterParent->setEnabled(true);
        PBIterParent->setMaximumSize(QSize(50, 27));

        gridLayout->addWidget(PBIterParent, 1, 2, 1, 1);

        LEIterationParentName = new QLineEdit(scrollAreaWidgetContents);
        LEIterationParentName->setObjectName(QStringLiteral("LEIterationParentName"));
        LEIterationParentName->setMinimumSize(QSize(0, 21));
        LEIterationParentName->setMaxLength(64);
        LEIterationParentName->setReadOnly(true);

        gridLayout->addWidget(LEIterationParentName, 1, 3, 1, 2);

        Mesh_n = new QLabel(scrollAreaWidgetContents);
        Mesh_n->setObjectName(QStringLiteral("Mesh_n"));

        gridLayout->addWidget(Mesh_n, 2, 0, 1, 1);

        horizontalSpacer_7 = new QSpacerItem(117, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_7, 2, 2, 1, 1);

        LEMeshName_n = new QLineEdit(scrollAreaWidgetContents);
        LEMeshName_n->setObjectName(QStringLiteral("LEMeshName_n"));
        LEMeshName_n->setMinimumSize(QSize(290, 21));
        LEMeshName_n->setMaxLength(64);
        LEMeshName_n->setReadOnly(true);

        gridLayout->addWidget(LEMeshName_n, 2, 3, 1, 3);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_4, 2, 6, 1, 1);

        Mesh_np1 = new QLabel(scrollAreaWidgetContents);
        Mesh_np1->setObjectName(QStringLiteral("Mesh_np1"));

        gridLayout->addWidget(Mesh_np1, 3, 0, 1, 2);

        horizontalSpacer_8 = new QSpacerItem(117, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_8, 3, 2, 1, 1);

        LEMeshName_np1 = new QLineEdit(scrollAreaWidgetContents);
        LEMeshName_np1->setObjectName(QStringLiteral("LEMeshName_np1"));
        LEMeshName_np1->setMinimumSize(QSize(290, 21));
        LEMeshName_np1->setMaxLength(64);

        gridLayout->addWidget(LEMeshName_np1, 3, 3, 1, 3);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_5, 3, 6, 1, 1);

        verticalSpacer = new QSpacerItem(20, 62, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 4, 0, 1, 1);

        GBField = new QGroupBox(scrollAreaWidgetContents);
        GBField->setObjectName(QStringLiteral("GBField"));
        gridLayout1 = new QGridLayout(GBField);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QStringLiteral("gridLayout1"));
        LEFieldFile = new QLineEdit(GBField);
        LEFieldFile->setObjectName(QStringLiteral("LEFieldFile"));
        LEFieldFile->setMinimumSize(QSize(282, 21));

        gridLayout1->addWidget(LEFieldFile, 0, 2, 1, 5);

        spacerItem = new QSpacerItem(138, 18, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout1->addItem(spacerItem, 1, 3, 1, 4);

        RBChosen = new QRadioButton(GBField);
        RBChosen->setObjectName(QStringLiteral("RBChosen"));

        gridLayout1->addWidget(RBChosen, 2, 6, 1, 1);

        spacerItem1 = new QSpacerItem(255, 13, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout1->addItem(spacerItem1, 4, 6, 1, 1);

        SpinBox_Rank = new QSpinBox(GBField);
        SpinBox_Rank->setObjectName(QStringLiteral("SpinBox_Rank"));
        SpinBox_Rank->setMinimum(-1);
        SpinBox_Rank->setMaximum(1010000);
        SpinBox_Rank->setValue(-1);

        gridLayout1->addWidget(SpinBox_Rank, 4, 5, 1, 1);

        Rank = new QLabel(GBField);
        Rank->setObjectName(QStringLiteral("Rank"));

        gridLayout1->addWidget(Rank, 4, 4, 1, 1);

        spacerItem2 = new QSpacerItem(40, 13, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout1->addItem(spacerItem2, 4, 2, 1, 2);

        SpinBox_TimeStep = new QSpinBox(GBField);
        SpinBox_TimeStep->setObjectName(QStringLiteral("SpinBox_TimeStep"));
        SpinBox_TimeStep->setMinimum(-2);
        SpinBox_TimeStep->setMaximum(100000);
        SpinBox_TimeStep->setValue(-1);

        gridLayout1->addWidget(SpinBox_TimeStep, 4, 1, 1, 1);

        TimeStep = new QLabel(GBField);
        TimeStep->setObjectName(QStringLiteral("TimeStep"));

        gridLayout1->addWidget(TimeStep, 4, 0, 1, 1);

        spacerItem3 = new QSpacerItem(138, 28, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout1->addItem(spacerItem3, 3, 0, 1, 4);

        RBLast = new QRadioButton(GBField);
        RBLast->setObjectName(QStringLiteral("RBLast"));

        gridLayout1->addWidget(RBLast, 2, 3, 1, 3);

        RBNo = new QRadioButton(GBField);
        RBNo->setObjectName(QStringLiteral("RBNo"));
        RBNo->setCheckable(true);
        RBNo->setChecked(true);

        gridLayout1->addWidget(RBNo, 2, 0, 1, 3);

        PushFieldFile = new QPushButton(GBField);
        PushFieldFile->setObjectName(QStringLiteral("PushFieldFile"));

        gridLayout1->addWidget(PushFieldFile, 0, 1, 1, 1);

        FieldFile = new QLabel(GBField);
        FieldFile->setObjectName(QStringLiteral("FieldFile"));

        gridLayout1->addWidget(FieldFile, 0, 0, 1, 1);


        gridLayout->addWidget(GBField, 5, 0, 1, 6);

        horizontalSpacer_10 = new QSpacerItem(69, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_10, 5, 6, 1, 1);

        spacerItem4 = new QSpacerItem(20, 22, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(spacerItem4, 6, 0, 1, 1);

        Hypothese = new QGroupBox(scrollAreaWidgetContents);
        Hypothese->setObjectName(QStringLiteral("Hypothese"));
        gridLayout2 = new QGridLayout(Hypothese);
#ifndef Q_OS_MAC
        gridLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout2->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout2->setObjectName(QStringLiteral("gridLayout2"));
        CBHypothese = new QComboBox(Hypothese);
        CBHypothese->setObjectName(QStringLiteral("CBHypothese"));
        CBHypothese->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        gridLayout2->addWidget(CBHypothese, 0, 0, 1, 1);

        spacerItem5 = new QSpacerItem(40, 13, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout2->addItem(spacerItem5, 0, 1, 1, 1);

        PBHypoEdit = new QPushButton(Hypothese);
        PBHypoEdit->setObjectName(QStringLiteral("PBHypoEdit"));

        gridLayout2->addWidget(PBHypoEdit, 0, 2, 1, 1);

        PBHypoNew = new QPushButton(Hypothese);
        PBHypoNew->setObjectName(QStringLiteral("PBHypoNew"));

        gridLayout2->addWidget(PBHypoNew, 0, 3, 1, 1);


        gridLayout->addWidget(Hypothese, 7, 0, 1, 4);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 7, 4, 1, 2);

        spacerItem6 = new QSpacerItem(20, 21, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(spacerItem6, 8, 1, 1, 1);

        GroupButtons = new QGroupBox(scrollAreaWidgetContents);
        GroupButtons->setObjectName(QStringLiteral("GroupButtons"));
        gridLayout3 = new QGridLayout(GroupButtons);
#ifndef Q_OS_MAC
        gridLayout3->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout3->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout3->setObjectName(QStringLiteral("gridLayout3"));
        buttonOk = new QPushButton(GroupButtons);
        buttonOk->setObjectName(QStringLiteral("buttonOk"));
        buttonOk->setAutoDefault(false);

        gridLayout3->addWidget(buttonOk, 0, 0, 1, 1);

        buttonApply = new QPushButton(GroupButtons);
        buttonApply->setObjectName(QStringLiteral("buttonApply"));

        gridLayout3->addWidget(buttonApply, 0, 1, 1, 1);

        buttonCancel = new QPushButton(GroupButtons);
        buttonCancel->setObjectName(QStringLiteral("buttonCancel"));

        gridLayout3->addWidget(buttonCancel, 0, 2, 1, 1);

        buttonHelp = new QPushButton(GroupButtons);
        buttonHelp->setObjectName(QStringLiteral("buttonHelp"));

        gridLayout3->addWidget(buttonHelp, 0, 3, 1, 1);


        gridLayout->addWidget(GroupButtons, 9, 0, 1, 4);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 9, 4, 1, 2);

        horizontalSpacer_9 = new QSpacerItem(139, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_9, 1, 5, 1, 1);

        CreateIteration->setWidget(scrollAreaWidgetContents);

        retranslateUi(CreateIteration);

        CBHypothese->setCurrentIndex(-1);
        buttonOk->setDefault(false);


        QMetaObject::connectSlotsByName(CreateIteration);
    } // setupUi

    void retranslateUi(QScrollArea *CreateIteration)
    {
        CreateIteration->setWindowTitle(QApplication::translate("CreateIteration", "Create an iteration", Q_NULLPTR));
        Iteration_Name->setText(QApplication::translate("CreateIteration", "Iteration Name", Q_NULLPTR));
        Iter_Parent->setText(QApplication::translate("CreateIteration", "Previous iteration", Q_NULLPTR));
        PBIterParent->setText(QString());
        Mesh_n->setText(QApplication::translate("CreateIteration", "Mesh n", Q_NULLPTR));
        Mesh_np1->setText(QApplication::translate("CreateIteration", "Mesh n+1", Q_NULLPTR));
        GBField->setTitle(QApplication::translate("CreateIteration", "Field information", Q_NULLPTR));
        RBChosen->setText(QApplication::translate("CreateIteration", "Chosen time step", Q_NULLPTR));
        Rank->setText(QApplication::translate("CreateIteration", "Rank", Q_NULLPTR));
        TimeStep->setText(QApplication::translate("CreateIteration", "Time step", Q_NULLPTR));
        RBLast->setText(QApplication::translate("CreateIteration", "Last time step", Q_NULLPTR));
        RBNo->setText(QApplication::translate("CreateIteration", "No time step", Q_NULLPTR));
        PushFieldFile->setText(QString());
        FieldFile->setText(QApplication::translate("CreateIteration", "Field file", Q_NULLPTR));
        Hypothese->setTitle(QApplication::translate("CreateIteration", "Hypothesis", Q_NULLPTR));
        PBHypoEdit->setText(QApplication::translate("CreateIteration", "Edit", Q_NULLPTR));
        PBHypoNew->setText(QApplication::translate("CreateIteration", "New", Q_NULLPTR));
        GroupButtons->setTitle(QString());
        buttonOk->setText(QApplication::translate("CreateIteration", "OK", Q_NULLPTR));
        buttonApply->setText(QApplication::translate("CreateIteration", "Apply", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("CreateIteration", "Cancel", Q_NULLPTR));
        buttonHelp->setText(QApplication::translate("CreateIteration", "Help", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CreateIteration: public Ui_CreateIteration {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATEITERATION_H
