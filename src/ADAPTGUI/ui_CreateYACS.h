/********************************************************************************
** Form generated from reading UI file 'CreateYACS.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATEYACS_H
#define UI_CREATEYACS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
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

class Ui_CreateYACS
{
public:
    QWidget *scrollAreaWidgetContents;
    QGridLayout *gridLayout_5;
    QWidget *WName;
    QGridLayout *gridLayout_2;
    QLabel *Name;
    QLineEdit *LEName;
    QSpacerItem *horizontalSpacer;
    QGridLayout *gridLayout;
    QLabel *Case;
    QPushButton *PBCaseName;
    QLineEdit *LECaseName;
    QHBoxLayout *_2;
    QLabel *Script;
    QPushButton *PBScriptFile;
    QLineEdit *LEScriptFile;
    QHBoxLayout *hboxLayout;
    QLabel *DirectoryStart;
    QPushButton *PBDir;
    QLineEdit *LEDirName;
    QHBoxLayout *_3;
    QLabel *MeshFile;
    QPushButton *PBMeshFile;
    QLineEdit *LEMeshFile;
    QGroupBox *GBTypeSchema;
    QHBoxLayout *_4;
    QRadioButton *RBConstant;
    QRadioButton *RBVariable;
    QGroupBox *GBMax;
    QGridLayout *gridLayout_3;
    QLabel *TLMaxIteration;
    QSpinBox *SpinBoxMaxIter;
    QLabel *TLMaxNodes;
    QSpinBox *SpinBoxMaxNode;
    QLabel *TLMaxElem;
    QSpinBox *SpinBoxMaxElem;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *GBConvergence;
    QGridLayout *gridLayout_4;
    QRadioButton *RBNone;
    QRadioButton *RBVMinAbs;
    QRadioButton *RBVMaxAbs;
    QLabel *labelVref;
    QDoubleSpinBox *doubleSpinBoxConvergence;
    QSpacerItem *horizontalSpacer_3;
    QGroupBox *GroupButtons;
    QGridLayout *gridLayout1;
    QPushButton *buttonHelp;
    QPushButton *buttonApply;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;
    QSpacerItem *spacer_2;

    void setupUi(QScrollArea *CreateYACS)
    {
        if (CreateYACS->objectName().isEmpty())
            CreateYACS->setObjectName(QStringLiteral("CreateYACS"));
        CreateYACS->resize(684, 649);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(CreateYACS->sizePolicy().hasHeightForWidth());
        CreateYACS->setSizePolicy(sizePolicy);
        CreateYACS->setMinimumSize(QSize(600, 500));
        CreateYACS->setSizeIncrement(QSize(1, 1));
        CreateYACS->setBaseSize(QSize(600, 500));
        CreateYACS->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 680, 645));
        scrollAreaWidgetContents->setProperty("sizeGripEnabled", QVariant(true));
        gridLayout_5 = new QGridLayout(scrollAreaWidgetContents);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        WName = new QWidget(scrollAreaWidgetContents);
        WName->setObjectName(QStringLiteral("WName"));
        gridLayout_2 = new QGridLayout(WName);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        Name = new QLabel(WName);
        Name->setObjectName(QStringLiteral("Name"));

        gridLayout_2->addWidget(Name, 0, 0, 1, 1);

        LEName = new QLineEdit(WName);
        LEName->setObjectName(QStringLiteral("LEName"));
        LEName->setMaxLength(32);

        gridLayout_2->addWidget(LEName, 0, 1, 1, 1);


        gridLayout_5->addWidget(WName, 0, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(131, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer, 0, 1, 1, 1);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        Case = new QLabel(scrollAreaWidgetContents);
        Case->setObjectName(QStringLiteral("Case"));

        gridLayout->addWidget(Case, 0, 0, 1, 1);

        PBCaseName = new QPushButton(scrollAreaWidgetContents);
        PBCaseName->setObjectName(QStringLiteral("PBCaseName"));
        PBCaseName->setEnabled(true);
        PBCaseName->setMaximumSize(QSize(50, 27));

        gridLayout->addWidget(PBCaseName, 0, 1, 1, 1);

        LECaseName = new QLineEdit(scrollAreaWidgetContents);
        LECaseName->setObjectName(QStringLiteral("LECaseName"));
        LECaseName->setMinimumSize(QSize(382, 21));

        gridLayout->addWidget(LECaseName, 0, 2, 1, 1);


        gridLayout_5->addLayout(gridLayout, 1, 0, 1, 2);

        _2 = new QHBoxLayout();
#ifndef Q_OS_MAC
        _2->setSpacing(6);
#endif
        _2->setContentsMargins(0, 0, 0, 0);
        _2->setObjectName(QStringLiteral("_2"));
        Script = new QLabel(scrollAreaWidgetContents);
        Script->setObjectName(QStringLiteral("Script"));

        _2->addWidget(Script);

        PBScriptFile = new QPushButton(scrollAreaWidgetContents);
        PBScriptFile->setObjectName(QStringLiteral("PBScriptFile"));
        PBScriptFile->setAutoDefault(false);

        _2->addWidget(PBScriptFile);

        LEScriptFile = new QLineEdit(scrollAreaWidgetContents);
        LEScriptFile->setObjectName(QStringLiteral("LEScriptFile"));
        LEScriptFile->setMinimumSize(QSize(382, 21));

        _2->addWidget(LEScriptFile);


        gridLayout_5->addLayout(_2, 2, 0, 1, 2);

        hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
        DirectoryStart = new QLabel(scrollAreaWidgetContents);
        DirectoryStart->setObjectName(QStringLiteral("DirectoryStart"));

        hboxLayout->addWidget(DirectoryStart);

        PBDir = new QPushButton(scrollAreaWidgetContents);
        PBDir->setObjectName(QStringLiteral("PBDir"));
        PBDir->setAutoDefault(false);

        hboxLayout->addWidget(PBDir);

        LEDirName = new QLineEdit(scrollAreaWidgetContents);
        LEDirName->setObjectName(QStringLiteral("LEDirName"));
        LEDirName->setMinimumSize(QSize(382, 21));

        hboxLayout->addWidget(LEDirName);


        gridLayout_5->addLayout(hboxLayout, 3, 0, 1, 2);

        _3 = new QHBoxLayout();
#ifndef Q_OS_MAC
        _3->setSpacing(6);
#endif
        _3->setContentsMargins(0, 0, 0, 0);
        _3->setObjectName(QStringLiteral("_3"));
        MeshFile = new QLabel(scrollAreaWidgetContents);
        MeshFile->setObjectName(QStringLiteral("MeshFile"));

        _3->addWidget(MeshFile);

        PBMeshFile = new QPushButton(scrollAreaWidgetContents);
        PBMeshFile->setObjectName(QStringLiteral("PBMeshFile"));
        PBMeshFile->setAutoDefault(false);

        _3->addWidget(PBMeshFile);

        LEMeshFile = new QLineEdit(scrollAreaWidgetContents);
        LEMeshFile->setObjectName(QStringLiteral("LEMeshFile"));
        LEMeshFile->setMinimumSize(QSize(382, 21));

        _3->addWidget(LEMeshFile);


        gridLayout_5->addLayout(_3, 4, 0, 1, 2);

        GBTypeSchema = new QGroupBox(scrollAreaWidgetContents);
        GBTypeSchema->setObjectName(QStringLiteral("GBTypeSchema"));
        _4 = new QHBoxLayout(GBTypeSchema);
#ifndef Q_OS_MAC
        _4->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        _4->setContentsMargins(9, 9, 9, 9);
#endif
        _4->setObjectName(QStringLiteral("_4"));
        RBConstant = new QRadioButton(GBTypeSchema);
        RBConstant->setObjectName(QStringLiteral("RBConstant"));
        RBConstant->setChecked(true);

        _4->addWidget(RBConstant);

        RBVariable = new QRadioButton(GBTypeSchema);
        RBVariable->setObjectName(QStringLiteral("RBVariable"));

        _4->addWidget(RBVariable);


        gridLayout_5->addWidget(GBTypeSchema, 5, 0, 1, 1);

        GBMax = new QGroupBox(scrollAreaWidgetContents);
        GBMax->setObjectName(QStringLiteral("GBMax"));
        gridLayout_3 = new QGridLayout(GBMax);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        TLMaxIteration = new QLabel(GBMax);
        TLMaxIteration->setObjectName(QStringLiteral("TLMaxIteration"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(TLMaxIteration->sizePolicy().hasHeightForWidth());
        TLMaxIteration->setSizePolicy(sizePolicy1);
        TLMaxIteration->setWordWrap(false);

        gridLayout_3->addWidget(TLMaxIteration, 0, 0, 1, 1);

        SpinBoxMaxIter = new QSpinBox(GBMax);
        SpinBoxMaxIter->setObjectName(QStringLiteral("SpinBoxMaxIter"));
        SpinBoxMaxIter->setMinimum(0);
        SpinBoxMaxIter->setMaximum(999999999);
        SpinBoxMaxIter->setValue(0);

        gridLayout_3->addWidget(SpinBoxMaxIter, 0, 1, 1, 1);

        TLMaxNodes = new QLabel(GBMax);
        TLMaxNodes->setObjectName(QStringLiteral("TLMaxNodes"));
        sizePolicy1.setHeightForWidth(TLMaxNodes->sizePolicy().hasHeightForWidth());
        TLMaxNodes->setSizePolicy(sizePolicy1);
        TLMaxNodes->setWordWrap(false);

        gridLayout_3->addWidget(TLMaxNodes, 1, 0, 1, 1);

        SpinBoxMaxNode = new QSpinBox(GBMax);
        SpinBoxMaxNode->setObjectName(QStringLiteral("SpinBoxMaxNode"));
        SpinBoxMaxNode->setMinimum(0);
        SpinBoxMaxNode->setMaximum(999999999);
        SpinBoxMaxNode->setSingleStep(1000);
        SpinBoxMaxNode->setValue(0);

        gridLayout_3->addWidget(SpinBoxMaxNode, 1, 1, 1, 1);

        TLMaxElem = new QLabel(GBMax);
        TLMaxElem->setObjectName(QStringLiteral("TLMaxElem"));
        sizePolicy1.setHeightForWidth(TLMaxElem->sizePolicy().hasHeightForWidth());
        TLMaxElem->setSizePolicy(sizePolicy1);
        TLMaxElem->setWordWrap(false);

        gridLayout_3->addWidget(TLMaxElem, 1, 2, 1, 1);

        SpinBoxMaxElem = new QSpinBox(GBMax);
        SpinBoxMaxElem->setObjectName(QStringLiteral("SpinBoxMaxElem"));
        SpinBoxMaxElem->setMinimum(0);
        SpinBoxMaxElem->setMaximum(999999999);
        SpinBoxMaxElem->setSingleStep(1000);
        SpinBoxMaxElem->setValue(0);

        gridLayout_3->addWidget(SpinBoxMaxElem, 1, 3, 1, 1);


        gridLayout_5->addWidget(GBMax, 6, 0, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(269, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_2, 6, 1, 1, 1);

        GBConvergence = new QGroupBox(scrollAreaWidgetContents);
        GBConvergence->setObjectName(QStringLiteral("GBConvergence"));
        gridLayout_4 = new QGridLayout(GBConvergence);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        RBNone = new QRadioButton(GBConvergence);
        RBNone->setObjectName(QStringLiteral("RBNone"));
        RBNone->setChecked(true);

        gridLayout_4->addWidget(RBNone, 0, 0, 1, 1);

        RBVMinAbs = new QRadioButton(GBConvergence);
        RBVMinAbs->setObjectName(QStringLiteral("RBVMinAbs"));

        gridLayout_4->addWidget(RBVMinAbs, 0, 1, 1, 1);

        RBVMaxAbs = new QRadioButton(GBConvergence);
        RBVMaxAbs->setObjectName(QStringLiteral("RBVMaxAbs"));

        gridLayout_4->addWidget(RBVMaxAbs, 0, 2, 1, 1);

        labelVref = new QLabel(GBConvergence);
        labelVref->setObjectName(QStringLiteral("labelVref"));

        gridLayout_4->addWidget(labelVref, 1, 0, 1, 1);

        doubleSpinBoxConvergence = new QDoubleSpinBox(GBConvergence);
        doubleSpinBoxConvergence->setObjectName(QStringLiteral("doubleSpinBoxConvergence"));
        doubleSpinBoxConvergence->setDecimals(4);
        doubleSpinBoxConvergence->setMinimum(-1e+9);
        doubleSpinBoxConvergence->setMaximum(1e+9);

        gridLayout_4->addWidget(doubleSpinBoxConvergence, 1, 1, 1, 1);


        gridLayout_5->addWidget(GBConvergence, 7, 0, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(269, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_3, 7, 1, 1, 1);

        GroupButtons = new QGroupBox(scrollAreaWidgetContents);
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


        gridLayout_5->addWidget(GroupButtons, 8, 0, 1, 1);

        spacer_2 = new QSpacerItem(128, 25, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_5->addItem(spacer_2, 8, 1, 1, 1);

        CreateYACS->setWidget(scrollAreaWidgetContents);

        retranslateUi(CreateYACS);

        QMetaObject::connectSlotsByName(CreateYACS);
    } // setupUi

    void retranslateUi(QScrollArea *CreateYACS)
    {
        CreateYACS->setWindowTitle(QApplication::translate("CreateYACS", "Create YACS", Q_NULLPTR));
        Name->setText(QApplication::translate("CreateYACS", "Name", Q_NULLPTR));
        Case->setText(QApplication::translate("CreateYACS", "Case", Q_NULLPTR));
        PBCaseName->setText(QString());
        Script->setText(QApplication::translate("CreateYACS", "Script", Q_NULLPTR));
        PBScriptFile->setText(QString());
        DirectoryStart->setText(QApplication::translate("CreateYACS", "Directory", Q_NULLPTR));
        PBDir->setText(QString());
        MeshFile->setText(QApplication::translate("CreateYACS", "Mesh file", Q_NULLPTR));
        PBMeshFile->setText(QString());
        GBTypeSchema->setTitle(QApplication::translate("CreateYACS", "Type of schema", Q_NULLPTR));
        RBConstant->setText(QApplication::translate("CreateYACS", "Constant", Q_NULLPTR));
        RBVariable->setText(QApplication::translate("CreateYACS", "Variable", Q_NULLPTR));
        GBMax->setTitle(QApplication::translate("CreateYACS", "Maximum of ...", Q_NULLPTR));
        TLMaxIteration->setText(QApplication::translate("CreateYACS", "Iterations", Q_NULLPTR));
        TLMaxNodes->setText(QApplication::translate("CreateYACS", "Nodes", Q_NULLPTR));
        TLMaxElem->setText(QApplication::translate("CreateYACS", "Elements", Q_NULLPTR));
        GBConvergence->setTitle(QApplication::translate("CreateYACS", "Test of convergence", Q_NULLPTR));
        RBNone->setText(QApplication::translate("CreateYACS", "None", Q_NULLPTR));
        RBVMinAbs->setText(QApplication::translate("CreateYACS", "Vtest > Vref", Q_NULLPTR));
        RBVMaxAbs->setText(QApplication::translate("CreateYACS", "Vtest < Vref", Q_NULLPTR));
        labelVref->setText(QApplication::translate("CreateYACS", "Vref", Q_NULLPTR));
        GroupButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("CreateYACS", "Help", Q_NULLPTR));
        buttonApply->setText(QApplication::translate("CreateYACS", "Apply", Q_NULLPTR));
        buttonOk->setText(QApplication::translate("CreateYACS", "OK", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("CreateYACS", "Cancel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CreateYACS: public Ui_CreateYACS {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATEYACS_H
