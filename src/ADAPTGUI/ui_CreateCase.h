/********************************************************************************
** Form generated from reading UI file 'CreateCase.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATECASE_H
#define UI_CREATECASE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CreateCase
{
public:
    QGridLayout *gridLayout_5;
    QWidget *WName;
    QGridLayout *gridLayout_2;
    QLabel *Name;
    QLineEdit *LEName;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *hboxLayout;
    QLabel *Directory;
    QPushButton *PushDir;
    QLineEdit *LEDirName;
    QSpacerItem *spacerItem;
    QHBoxLayout *hboxLayout1;
    QLabel *Mesh;
    QPushButton *PushFichier;
    QLineEdit *LEFileName;
    QSpacerItem *spacerItem1;
    QGroupBox *GBTypeConf;
    QHBoxLayout *hboxLayout2;
    QRadioButton *RBConforme;
    QRadioButton *RBNonConforme;
    QGroupBox *GBTypeBoun;
    QHBoxLayout *_3;
    QRadioButton *RBBoundaryNo;
    QRadioButton *RBBoundaryCAO;
    QRadioButton *RBBoundaryNonCAO;
    QSpacerItem *spacerItem2;
    QHBoxLayout *hboxLayout3;
    QCheckBox *CBBoundaryD;
    QCheckBox *CBBoundaryA;
    QGroupBox *GBBoundaryC;
    QGridLayout *_2;
    QPushButton *PBBoundaryCAOEdit;
    QPushButton *PBBoundaryCAOHelp;
    QComboBox *CBBoundaryCAO;
    QSpacerItem *spacerItem3;
    QPushButton *PBBoundaryCAONew;
    QSpacerItem *spacerItem4;
    QGroupBox *GBBoundaryD;
    QGridLayout *gridLayout;
    QComboBox *CBBoundaryDi;
    QSpacerItem *spacerItem5;
    QPushButton *PBBoundaryDiEdit;
    QPushButton *PBBoundaryDiHelp;
    QPushButton *PBBoundaryDiNew;
    QGroupBox *GBBoundaryA;
    QFormLayout *formLayout;
    QTableWidget *TWBoundary;
    QGridLayout *gridLayout1;
    QPushButton *PBBoundaryAnEdit;
    QPushButton *PBBoundaryAnNew;
    QPushButton *PBBoundaryAnHelp;
    QSpacerItem *spacer;
    QCheckBox *CBAdvanced;
    QGroupBox *GBAdvancedOptions;
    QGridLayout *gridLayout_4;
    QCheckBox *CBPyramid;
    QGroupBox *GBConforme;
    QGridLayout *gridLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QRadioButton *RBStandard;
    QRadioButton *RBBox;
    QRadioButton *RBNC1NpA;
    QRadioButton *RBNCQuelconque;
    QGroupBox *GBFormat;
    QGridLayout *gridLayout2;
    QRadioButton *RBMED;
    QRadioButton *RBSaturne;
    QRadioButton *RBSaturne2D;
    QSpacerItem *spacer_3;
    QSpacerItem *spacer_4;
    QLabel *Comment;
    QSpacerItem *verticalSpacer;
    QGroupBox *GroupButtons;
    QGridLayout *gridLayout3;
    QPushButton *buttonHelp;
    QPushButton *buttonApply;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;
    QSpacerItem *spacer_2;

    void setupUi(QDialog *CreateCase)
    {
        if (CreateCase->objectName().isEmpty())
            CreateCase->setObjectName(QStringLiteral("CreateCase"));
        CreateCase->resize(600, 1150);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(CreateCase->sizePolicy().hasHeightForWidth());
        CreateCase->setSizePolicy(sizePolicy);
        CreateCase->setMinimumSize(QSize(600, 320));
        CreateCase->setSizeIncrement(QSize(1, 1));
        CreateCase->setBaseSize(QSize(600, 320));
        CreateCase->setAutoFillBackground(true);
        gridLayout_5 = new QGridLayout(CreateCase);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        WName = new QWidget(CreateCase);
        WName->setObjectName(QStringLiteral("WName"));
        gridLayout_2 = new QGridLayout(WName);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        Name = new QLabel(WName);
        Name->setObjectName(QStringLiteral("Name"));

        gridLayout_2->addWidget(Name, 0, 0, 1, 1);

        LEName = new QLineEdit(WName);
        LEName->setObjectName(QStringLiteral("LEName"));

        gridLayout_2->addWidget(LEName, 0, 1, 1, 1);


        gridLayout_5->addWidget(WName, 0, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(199, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer, 0, 2, 1, 2);

        hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
        Directory = new QLabel(CreateCase);
        Directory->setObjectName(QStringLiteral("Directory"));

        hboxLayout->addWidget(Directory);

        PushDir = new QPushButton(CreateCase);
        PushDir->setObjectName(QStringLiteral("PushDir"));
        PushDir->setAutoDefault(false);

        hboxLayout->addWidget(PushDir);

        LEDirName = new QLineEdit(CreateCase);
        LEDirName->setObjectName(QStringLiteral("LEDirName"));
        LEDirName->setMinimumSize(QSize(382, 21));

        hboxLayout->addWidget(LEDirName);


        gridLayout_5->addLayout(hboxLayout, 1, 0, 1, 5);

        spacerItem = new QSpacerItem(20, 18, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_5->addItem(spacerItem, 2, 4, 1, 1);

        hboxLayout1 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout1->setSpacing(6);
#endif
        hboxLayout1->setContentsMargins(0, 0, 0, 0);
        hboxLayout1->setObjectName(QStringLiteral("hboxLayout1"));
        Mesh = new QLabel(CreateCase);
        Mesh->setObjectName(QStringLiteral("Mesh"));

        hboxLayout1->addWidget(Mesh);

        PushFichier = new QPushButton(CreateCase);
        PushFichier->setObjectName(QStringLiteral("PushFichier"));
        PushFichier->setAutoDefault(false);

        hboxLayout1->addWidget(PushFichier);

        LEFileName = new QLineEdit(CreateCase);
        LEFileName->setObjectName(QStringLiteral("LEFileName"));
        LEFileName->setMinimumSize(QSize(382, 21));

        hboxLayout1->addWidget(LEFileName);


        gridLayout_5->addLayout(hboxLayout1, 3, 0, 1, 5);

        spacerItem1 = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_5->addItem(spacerItem1, 3, 5, 1, 1);

        GBTypeConf = new QGroupBox(CreateCase);
        GBTypeConf->setObjectName(QStringLiteral("GBTypeConf"));
        hboxLayout2 = new QHBoxLayout(GBTypeConf);
#ifndef Q_OS_MAC
        hboxLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout2->setContentsMargins(9, 9, 9, 9);
#endif
        hboxLayout2->setObjectName(QStringLiteral("hboxLayout2"));
        RBConforme = new QRadioButton(GBTypeConf);
        RBConforme->setObjectName(QStringLiteral("RBConforme"));
        RBConforme->setChecked(true);

        hboxLayout2->addWidget(RBConforme);

        RBNonConforme = new QRadioButton(GBTypeConf);
        RBNonConforme->setObjectName(QStringLiteral("RBNonConforme"));

        hboxLayout2->addWidget(RBNonConforme);


        gridLayout_5->addWidget(GBTypeConf, 4, 0, 1, 1);

        GBTypeBoun = new QGroupBox(CreateCase);
        GBTypeBoun->setObjectName(QStringLiteral("GBTypeBoun"));
        _3 = new QHBoxLayout(GBTypeBoun);
#ifndef Q_OS_MAC
        _3->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        _3->setContentsMargins(9, 9, 9, 9);
#endif
        _3->setObjectName(QStringLiteral("_3"));
        RBBoundaryNo = new QRadioButton(GBTypeBoun);
        RBBoundaryNo->setObjectName(QStringLiteral("RBBoundaryNo"));
        RBBoundaryNo->setChecked(true);

        _3->addWidget(RBBoundaryNo);

        RBBoundaryCAO = new QRadioButton(GBTypeBoun);
        RBBoundaryCAO->setObjectName(QStringLiteral("RBBoundaryCAO"));
        RBBoundaryCAO->setChecked(false);

        _3->addWidget(RBBoundaryCAO);

        RBBoundaryNonCAO = new QRadioButton(GBTypeBoun);
        RBBoundaryNonCAO->setObjectName(QStringLiteral("RBBoundaryNonCAO"));

        _3->addWidget(RBBoundaryNonCAO);


        gridLayout_5->addWidget(GBTypeBoun, 5, 0, 1, 4);

        spacerItem2 = new QSpacerItem(20, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_5->addItem(spacerItem2, 5, 6, 1, 1);

        hboxLayout3 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout3->setSpacing(6);
#endif
        hboxLayout3->setContentsMargins(0, 0, 0, 0);
        hboxLayout3->setObjectName(QStringLiteral("hboxLayout3"));
        CBBoundaryD = new QCheckBox(CreateCase);
        CBBoundaryD->setObjectName(QStringLiteral("CBBoundaryD"));

        hboxLayout3->addWidget(CBBoundaryD);

        CBBoundaryA = new QCheckBox(CreateCase);
        CBBoundaryA->setObjectName(QStringLiteral("CBBoundaryA"));

        hboxLayout3->addWidget(CBBoundaryA);


        gridLayout_5->addLayout(hboxLayout3, 6, 0, 1, 3);

        GBBoundaryC = new QGroupBox(CreateCase);
        GBBoundaryC->setObjectName(QStringLiteral("GBBoundaryC"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(GBBoundaryC->sizePolicy().hasHeightForWidth());
        GBBoundaryC->setSizePolicy(sizePolicy1);
        _2 = new QGridLayout(GBBoundaryC);
#ifndef Q_OS_MAC
        _2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        _2->setContentsMargins(9, 9, 9, 9);
#endif
        _2->setObjectName(QStringLiteral("_2"));
        PBBoundaryCAOEdit = new QPushButton(GBBoundaryC);
        PBBoundaryCAOEdit->setObjectName(QStringLiteral("PBBoundaryCAOEdit"));
        PBBoundaryCAOEdit->setAutoDefault(false);

        _2->addWidget(PBBoundaryCAOEdit, 0, 3, 1, 1);

        PBBoundaryCAOHelp = new QPushButton(GBBoundaryC);
        PBBoundaryCAOHelp->setObjectName(QStringLiteral("PBBoundaryCAOHelp"));
        PBBoundaryCAOHelp->setAutoDefault(false);

        _2->addWidget(PBBoundaryCAOHelp, 0, 4, 1, 1);

        CBBoundaryCAO = new QComboBox(GBBoundaryC);
        CBBoundaryCAO->setObjectName(QStringLiteral("CBBoundaryCAO"));
        CBBoundaryCAO->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        _2->addWidget(CBBoundaryCAO, 0, 0, 1, 1);

        spacerItem3 = new QSpacerItem(40, 13, QSizePolicy::Fixed, QSizePolicy::Minimum);

        _2->addItem(spacerItem3, 0, 1, 1, 1);

        PBBoundaryCAONew = new QPushButton(GBBoundaryC);
        PBBoundaryCAONew->setObjectName(QStringLiteral("PBBoundaryCAONew"));
        PBBoundaryCAONew->setAutoDefault(false);

        _2->addWidget(PBBoundaryCAONew, 0, 2, 1, 1);


        gridLayout_5->addWidget(GBBoundaryC, 7, 0, 1, 2);

        spacerItem4 = new QSpacerItem(20, 2, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_5->addItem(spacerItem4, 7, 2, 1, 1);

        GBBoundaryD = new QGroupBox(CreateCase);
        GBBoundaryD->setObjectName(QStringLiteral("GBBoundaryD"));
        sizePolicy1.setHeightForWidth(GBBoundaryD->sizePolicy().hasHeightForWidth());
        GBBoundaryD->setSizePolicy(sizePolicy1);
        gridLayout = new QGridLayout(GBBoundaryD);
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        CBBoundaryDi = new QComboBox(GBBoundaryD);
        CBBoundaryDi->setObjectName(QStringLiteral("CBBoundaryDi"));
        CBBoundaryDi->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        gridLayout->addWidget(CBBoundaryDi, 0, 0, 1, 1);

        spacerItem5 = new QSpacerItem(40, 13, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(spacerItem5, 0, 1, 1, 1);

        PBBoundaryDiEdit = new QPushButton(GBBoundaryD);
        PBBoundaryDiEdit->setObjectName(QStringLiteral("PBBoundaryDiEdit"));
        PBBoundaryDiEdit->setAutoDefault(false);

        gridLayout->addWidget(PBBoundaryDiEdit, 0, 3, 1, 1);

        PBBoundaryDiHelp = new QPushButton(GBBoundaryD);
        PBBoundaryDiHelp->setObjectName(QStringLiteral("PBBoundaryDiHelp"));
        PBBoundaryDiHelp->setAutoDefault(false);

        gridLayout->addWidget(PBBoundaryDiHelp, 0, 4, 1, 1);

        PBBoundaryDiNew = new QPushButton(GBBoundaryD);
        PBBoundaryDiNew->setObjectName(QStringLiteral("PBBoundaryDiNew"));
        PBBoundaryDiNew->setAutoDefault(false);

        gridLayout->addWidget(PBBoundaryDiNew, 0, 2, 1, 1);


        gridLayout_5->addWidget(GBBoundaryD, 8, 0, 1, 4);

        GBBoundaryA = new QGroupBox(CreateCase);
        GBBoundaryA->setObjectName(QStringLiteral("GBBoundaryA"));
        GBBoundaryA->setMinimumSize(QSize(548, 200));
        formLayout = new QFormLayout(GBBoundaryA);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        TWBoundary = new QTableWidget(GBBoundaryA);
        if (TWBoundary->columnCount() < 1)
            TWBoundary->setColumnCount(1);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        TWBoundary->setHorizontalHeaderItem(0, __qtablewidgetitem);
        TWBoundary->setObjectName(QStringLiteral("TWBoundary"));
        TWBoundary->setEditTriggers(QAbstractItemView::AnyKeyPressed|QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed|QAbstractItemView::SelectedClicked);
        TWBoundary->setShowGrid(true);
        TWBoundary->setRowCount(0);
        TWBoundary->setColumnCount(1);

        formLayout->setWidget(0, QFormLayout::LabelRole, TWBoundary);

        gridLayout1 = new QGridLayout();
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
        gridLayout1->setContentsMargins(0, 0, 0, 0);
        gridLayout1->setObjectName(QStringLiteral("gridLayout1"));
        PBBoundaryAnEdit = new QPushButton(GBBoundaryA);
        PBBoundaryAnEdit->setObjectName(QStringLiteral("PBBoundaryAnEdit"));
        PBBoundaryAnEdit->setAutoDefault(false);

        gridLayout1->addWidget(PBBoundaryAnEdit, 1, 0, 1, 1);

        PBBoundaryAnNew = new QPushButton(GBBoundaryA);
        PBBoundaryAnNew->setObjectName(QStringLiteral("PBBoundaryAnNew"));
        PBBoundaryAnNew->setAutoDefault(false);

        gridLayout1->addWidget(PBBoundaryAnNew, 0, 0, 1, 1);

        PBBoundaryAnHelp = new QPushButton(GBBoundaryA);
        PBBoundaryAnHelp->setObjectName(QStringLiteral("PBBoundaryAnHelp"));
        PBBoundaryAnHelp->setAutoDefault(false);

        gridLayout1->addWidget(PBBoundaryAnHelp, 2, 0, 1, 1);


        formLayout->setLayout(0, QFormLayout::FieldRole, gridLayout1);


        gridLayout_5->addWidget(GBBoundaryA, 9, 0, 1, 7);

        spacer = new QSpacerItem(239, 41, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_5->addItem(spacer, 10, 0, 1, 1);

        CBAdvanced = new QCheckBox(CreateCase);
        CBAdvanced->setObjectName(QStringLiteral("CBAdvanced"));

        gridLayout_5->addWidget(CBAdvanced, 11, 0, 1, 1);

        GBAdvancedOptions = new QGroupBox(CreateCase);
        GBAdvancedOptions->setObjectName(QStringLiteral("GBAdvancedOptions"));
        gridLayout_4 = new QGridLayout(GBAdvancedOptions);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        CBPyramid = new QCheckBox(GBAdvancedOptions);
        CBPyramid->setObjectName(QStringLiteral("CBPyramid"));

        gridLayout_4->addWidget(CBPyramid, 0, 0, 1, 1);

        GBConforme = new QGroupBox(GBAdvancedOptions);
        GBConforme->setObjectName(QStringLiteral("GBConforme"));
        gridLayout_3 = new QGridLayout(GBConforme);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        RBStandard = new QRadioButton(GBConforme);
        RBStandard->setObjectName(QStringLiteral("RBStandard"));
        RBStandard->setChecked(true);

        horizontalLayout_2->addWidget(RBStandard);

        RBBox = new QRadioButton(GBConforme);
        RBBox->setObjectName(QStringLiteral("RBBox"));
        RBBox->setChecked(false);

        horizontalLayout_2->addWidget(RBBox);

        RBNC1NpA = new QRadioButton(GBConforme);
        RBNC1NpA->setObjectName(QStringLiteral("RBNC1NpA"));

        horizontalLayout_2->addWidget(RBNC1NpA);

        RBNCQuelconque = new QRadioButton(GBConforme);
        RBNCQuelconque->setObjectName(QStringLiteral("RBNCQuelconque"));

        horizontalLayout_2->addWidget(RBNCQuelconque);


        gridLayout_3->addLayout(horizontalLayout_2, 0, 0, 1, 1);

        GBFormat = new QGroupBox(GBConforme);
        GBFormat->setObjectName(QStringLiteral("GBFormat"));
        gridLayout2 = new QGridLayout(GBFormat);
        gridLayout2->setObjectName(QStringLiteral("gridLayout2"));
        RBMED = new QRadioButton(GBFormat);
        RBMED->setObjectName(QStringLiteral("RBMED"));
        RBMED->setChecked(true);

        gridLayout2->addWidget(RBMED, 0, 0, 1, 1);

        RBSaturne = new QRadioButton(GBFormat);
        RBSaturne->setObjectName(QStringLiteral("RBSaturne"));

        gridLayout2->addWidget(RBSaturne, 0, 1, 1, 1);

        RBSaturne2D = new QRadioButton(GBFormat);
        RBSaturne2D->setObjectName(QStringLiteral("RBSaturne2D"));

        gridLayout2->addWidget(RBSaturne2D, 0, 2, 1, 1);


        gridLayout_3->addWidget(GBFormat, 1, 0, 1, 1);


        gridLayout_4->addWidget(GBConforme, 1, 0, 1, 1);


        gridLayout_5->addWidget(GBAdvancedOptions, 12, 0, 1, 3);

        spacer_3 = new QSpacerItem(128, 13, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_5->addItem(spacer_3, 12, 4, 1, 3);

        spacer_4 = new QSpacerItem(239, 41, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_5->addItem(spacer_4, 13, 0, 1, 1);

        Comment = new QLabel(CreateCase);
        Comment->setObjectName(QStringLiteral("Comment"));

        gridLayout_5->addWidget(Comment, 14, 0, 1, 1);

        verticalSpacer = new QSpacerItem(20, 35, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_5->addItem(verticalSpacer, 14, 1, 1, 1);

        GroupButtons = new QGroupBox(CreateCase);
        GroupButtons->setObjectName(QStringLiteral("GroupButtons"));
        gridLayout3 = new QGridLayout(GroupButtons);
#ifndef Q_OS_MAC
        gridLayout3->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout3->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout3->setObjectName(QStringLiteral("gridLayout3"));
        buttonHelp = new QPushButton(GroupButtons);
        buttonHelp->setObjectName(QStringLiteral("buttonHelp"));
        buttonHelp->setAutoDefault(false);

        gridLayout3->addWidget(buttonHelp, 0, 4, 1, 1);

        buttonApply = new QPushButton(GroupButtons);
        buttonApply->setObjectName(QStringLiteral("buttonApply"));
        buttonApply->setAutoDefault(false);

        gridLayout3->addWidget(buttonApply, 0, 1, 1, 1);

        buttonOk = new QPushButton(GroupButtons);
        buttonOk->setObjectName(QStringLiteral("buttonOk"));
        buttonOk->setAutoDefault(false);

        gridLayout3->addWidget(buttonOk, 0, 0, 1, 1);

        buttonCancel = new QPushButton(GroupButtons);
        buttonCancel->setObjectName(QStringLiteral("buttonCancel"));
        buttonCancel->setAutoDefault(false);

        gridLayout3->addWidget(buttonCancel, 0, 2, 1, 1);


        gridLayout_5->addWidget(GroupButtons, 15, 0, 1, 2);

        spacer_2 = new QSpacerItem(128, 25, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_5->addItem(spacer_2, 15, 3, 1, 2);

        WName->raise();
        GBTypeConf->raise();
        GBBoundaryD->raise();
        GBBoundaryA->raise();
        CBAdvanced->raise();
        GBAdvancedOptions->raise();
        Comment->raise();
        GroupButtons->raise();
        GBBoundaryC->raise();
        GBTypeBoun->raise();

        retranslateUi(CreateCase);

        CBBoundaryCAO->setCurrentIndex(-1);
        CBBoundaryDi->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(CreateCase);
    } // setupUi

    void retranslateUi(QDialog *CreateCase)
    {
        CreateCase->setWindowTitle(QApplication::translate("CreateCase", "Create a case", Q_NULLPTR));
        Name->setText(QApplication::translate("CreateCase", "Name", Q_NULLPTR));
        Directory->setText(QApplication::translate("CreateCase", "Directory", Q_NULLPTR));
        PushDir->setText(QString());
        Mesh->setText(QApplication::translate("CreateCase", "Mesh", Q_NULLPTR));
        PushFichier->setText(QString());
        GBTypeConf->setTitle(QApplication::translate("CreateCase", "Conformity type", Q_NULLPTR));
        RBConforme->setText(QApplication::translate("CreateCase", "Conformal", Q_NULLPTR));
        RBNonConforme->setText(QApplication::translate("CreateCase", "Non conformal", Q_NULLPTR));
        GBTypeBoun->setTitle(QApplication::translate("CreateCase", "Boundary type", Q_NULLPTR));
        RBBoundaryNo->setText(QApplication::translate("CreateCase", "No boundary", Q_NULLPTR));
        RBBoundaryCAO->setText(QApplication::translate("CreateCase", "CAO", Q_NULLPTR));
        RBBoundaryNonCAO->setText(QApplication::translate("CreateCase", "Non CAO", Q_NULLPTR));
        CBBoundaryD->setText(QApplication::translate("CreateCase", "Discrete boundary", Q_NULLPTR));
        CBBoundaryA->setText(QApplication::translate("CreateCase", "Analytical boundary", Q_NULLPTR));
        GBBoundaryC->setTitle(QApplication::translate("CreateCase", "CAO", Q_NULLPTR));
        PBBoundaryCAOEdit->setText(QApplication::translate("CreateCase", "Edit", Q_NULLPTR));
        PBBoundaryCAOHelp->setText(QApplication::translate("CreateCase", "Help", Q_NULLPTR));
        PBBoundaryCAONew->setText(QApplication::translate("CreateCase", "New", Q_NULLPTR));
        GBBoundaryD->setTitle(QApplication::translate("CreateCase", "Discrete boundary", Q_NULLPTR));
        PBBoundaryDiEdit->setText(QApplication::translate("CreateCase", "Edit", Q_NULLPTR));
        PBBoundaryDiHelp->setText(QApplication::translate("CreateCase", "Help", Q_NULLPTR));
        PBBoundaryDiNew->setText(QApplication::translate("CreateCase", "New", Q_NULLPTR));
        GBBoundaryA->setTitle(QApplication::translate("CreateCase", "Analytical boundary", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem = TWBoundary->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("CreateCase", "a_virer", Q_NULLPTR));
        PBBoundaryAnEdit->setText(QApplication::translate("CreateCase", "Edit", Q_NULLPTR));
        PBBoundaryAnNew->setText(QApplication::translate("CreateCase", "New", Q_NULLPTR));
        PBBoundaryAnHelp->setText(QApplication::translate("CreateCase", "Help", Q_NULLPTR));
        CBAdvanced->setText(QApplication::translate("CreateCase", "Advanced options", Q_NULLPTR));
        GBAdvancedOptions->setTitle(QApplication::translate("CreateCase", "Advanced options", Q_NULLPTR));
        CBPyramid->setText(QApplication::translate("CreateCase", "Authorized pyramids", Q_NULLPTR));
        GBConforme->setTitle(QApplication::translate("CreateCase", "Conformity +", Q_NULLPTR));
        RBStandard->setText(QApplication::translate("CreateCase", "Standard", Q_NULLPTR));
        RBBox->setText(QApplication::translate("CreateCase", "Box", Q_NULLPTR));
        RBNC1NpA->setText(QApplication::translate("CreateCase", "1 node per edge", Q_NULLPTR));
        RBNCQuelconque->setText(QApplication::translate("CreateCase", "Free", Q_NULLPTR));
        GBFormat->setTitle(QApplication::translate("CreateCase", "Format", Q_NULLPTR));
        RBMED->setText(QApplication::translate("CreateCase", "MED", Q_NULLPTR));
        RBSaturne->setText(QApplication::translate("CreateCase", "Saturne", Q_NULLPTR));
        RBSaturne2D->setText(QApplication::translate("CreateCase", "Saturne 2D", Q_NULLPTR));
        Comment->setText(QApplication::translate("CreateCase", " No comment.", Q_NULLPTR));
        GroupButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("CreateCase", "Help", Q_NULLPTR));
        buttonApply->setText(QApplication::translate("CreateCase", "Apply", Q_NULLPTR));
        buttonOk->setText(QApplication::translate("CreateCase", "OK", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("CreateCase", "Cancel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CreateCase: public Ui_CreateCase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATECASE_H
