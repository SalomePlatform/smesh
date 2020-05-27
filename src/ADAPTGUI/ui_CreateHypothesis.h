/********************************************************************************
** Form generated from reading UI file 'CreateHypothesis.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATEHYPOTHESIS_H
#define UI_CREATEHYPOTHESIS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
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
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CreateHypothesis
{
public:
    QWidget *scrollAreaWidgetContents;
    QGridLayout *gridLayout_5;
    QWidget *WName;
    QGridLayout *gridLayout;
    QLabel *Name;
    QLineEdit *LEName;
    QSpacerItem *horizontalSpacer_9;
    QGroupBox *GBTypeAdaptation;
    QHBoxLayout *hboxLayout;
    QRadioButton *RBUniforme;
    QRadioButton *RBChamp;
    QRadioButton *RBZone;
    QSpacerItem *horizontalSpacer_6;
    QCheckBox *CBGroupe;
    QSpacerItem *horizontalSpacer_8;
    QGroupBox *GBUniform;
    QGridLayout *gridLayout1;
    QRadioButton *RBUniDera;
    QRadioButton *RBUniRaff;
    QSpacerItem *horizontalSpacer_7;
    QGroupBox *GBFieldFile;
    QGridLayout *gridLayout2;
    QLabel *FieldFile;
    QLineEdit *LEFieldFile;
    QGroupBox *GBFieldManagement;
    QGridLayout *gridLayout3;
    QHBoxLayout *hboxLayout1;
    QLabel *FieldName;
    QComboBox *CBFieldName;
    QSpacerItem *spacerItem;
    QHBoxLayout *hboxLayout2;
    QTableWidget *TWCMP;
    QSpacerItem *spacerItem1;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout3;
    QRadioButton *RBL2;
    QRadioButton *RBInf;
    QCheckBox *CBJump;
    QHBoxLayout *hboxLayout4;
    QGroupBox *GBRefinementThresholds;
    QGridLayout *gridLayout4;
    QRadioButton *RBRPE;
    QDoubleSpinBox *SpinBox_RPE;
    QRadioButton *RBRRel;
    QDoubleSpinBox *SpinBox_RRel;
    QRadioButton *RBRAbs;
    QDoubleSpinBox *SpinBox_RAbs;
    QRadioButton *RBRMuSigma;
    QDoubleSpinBox *SpinBox_RMuSigma;
    QRadioButton *RBRNo;
    QGroupBox *GBCoarseningThresholds;
    QGridLayout *gridLayout5;
    QRadioButton *RBCPE;
    QDoubleSpinBox *SpinBox_CPE;
    QRadioButton *RBCRel;
    QDoubleSpinBox *SpinBox_CRel;
    QRadioButton *RBCAbs;
    QDoubleSpinBox *SpinBox_CAbs;
    QRadioButton *RBCMuSigma;
    QDoubleSpinBox *SpinBox_CMuSigma;
    QRadioButton *RBCNo;
    QGroupBox *GBAreaManagement;
    QGridLayout *gridLayout_4;
    QTableWidget *TWZone;
    QSpacerItem *horizontalSpacer;
    QVBoxLayout *vboxLayout1;
    QVBoxLayout *vboxLayout2;
    QPushButton *PBZoneNew;
    QPushButton *PBZoneEdit;
    QPushButton *PBZoneDelete;
    QSpacerItem *spacerItem2;
    QGroupBox *GBField;
    QGridLayout *gridLayout6;
    QGridLayout *gridLayout7;
    QRadioButton *RBFieldNo;
    QRadioButton *RBFieldAll;
    QRadioButton *RBFieldChosen;
    QTableWidget *TWField;
    QSpacerItem *horizontalSpacer_5;
    QCheckBox *CBAdvanced;
    QSpacerItem *horizontalSpacer_4;
    QGroupBox *GBAdvancedOptions;
    QGridLayout *gridLayout_3;
    QLabel *TLMinimalDiameter;
    QDoubleSpinBox *doubleSpinBoxDiamMin;
    QLabel *TLMaximalLevel;
    QSpinBox *SpinBoxNivMax;
    QGroupBox *GBAdapInit;
    QGridLayout *gridLayout_2;
    QRadioButton *RBAIN;
    QRadioButton *RBAIR;
    QRadioButton *RBAID;
    QCheckBox *CBOutputLevel;
    QCheckBox *CBOutputQuality;
    QCheckBox *CBOutputDiameter;
    QCheckBox *CBOutputParent;
    QCheckBox *CBOutputVoisins;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *GBButtons;
    QGridLayout *gridLayout8;
    QPushButton *buttonOk;
    QPushButton *buttonApply;
    QPushButton *buttonCancel;
    QPushButton *buttonHelp;

    void setupUi(QScrollArea *CreateHypothesis)
    {
        if (CreateHypothesis->objectName().isEmpty())
            CreateHypothesis->setObjectName(QStringLiteral("CreateHypothesis"));
        CreateHypothesis->resize(848, 1650);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(CreateHypothesis->sizePolicy().hasHeightForWidth());
        CreateHypothesis->setSizePolicy(sizePolicy);
        CreateHypothesis->setMinimumSize(QSize(800, 600));
        CreateHypothesis->setSizeIncrement(QSize(1, 1));
        CreateHypothesis->setBaseSize(QSize(800, 1200));
        CreateHypothesis->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 844, 1646));
        scrollAreaWidgetContents->setProperty("sizeGripEnabled", QVariant(true));
        gridLayout_5 = new QGridLayout(scrollAreaWidgetContents);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        WName = new QWidget(scrollAreaWidgetContents);
        WName->setObjectName(QStringLiteral("WName"));
        gridLayout = new QGridLayout(WName);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        Name = new QLabel(WName);
        Name->setObjectName(QStringLiteral("Name"));

        gridLayout->addWidget(Name, 0, 0, 1, 1);

        LEName = new QLineEdit(WName);
        LEName->setObjectName(QStringLiteral("LEName"));
        LEName->setMinimumSize(QSize(282, 31));

        gridLayout->addWidget(LEName, 0, 1, 1, 1);


        gridLayout_5->addWidget(WName, 0, 0, 1, 1);

        horizontalSpacer_9 = new QSpacerItem(224, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_9, 0, 1, 1, 1);

        GBTypeAdaptation = new QGroupBox(scrollAreaWidgetContents);
        GBTypeAdaptation->setObjectName(QStringLiteral("GBTypeAdaptation"));
        hboxLayout = new QHBoxLayout(GBTypeAdaptation);
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout->setContentsMargins(9, 9, 9, 9);
#endif
        hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
        RBUniforme = new QRadioButton(GBTypeAdaptation);
        RBUniforme->setObjectName(QStringLiteral("RBUniforme"));
        RBUniforme->setCheckable(true);

        hboxLayout->addWidget(RBUniforme);

        RBChamp = new QRadioButton(GBTypeAdaptation);
        RBChamp->setObjectName(QStringLiteral("RBChamp"));

        hboxLayout->addWidget(RBChamp);

        RBZone = new QRadioButton(GBTypeAdaptation);
        RBZone->setObjectName(QStringLiteral("RBZone"));

        hboxLayout->addWidget(RBZone);


        gridLayout_5->addWidget(GBTypeAdaptation, 1, 0, 1, 1);

        horizontalSpacer_6 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_6, 1, 2, 1, 1);

        CBGroupe = new QCheckBox(scrollAreaWidgetContents);
        CBGroupe->setObjectName(QStringLiteral("CBGroupe"));

        gridLayout_5->addWidget(CBGroupe, 2, 0, 1, 1);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_8, 2, 2, 1, 1);

        GBUniform = new QGroupBox(scrollAreaWidgetContents);
        GBUniform->setObjectName(QStringLiteral("GBUniform"));
        gridLayout1 = new QGridLayout(GBUniform);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QStringLiteral("gridLayout1"));
        RBUniDera = new QRadioButton(GBUniform);
        RBUniDera->setObjectName(QStringLiteral("RBUniDera"));

        gridLayout1->addWidget(RBUniDera, 0, 1, 1, 1);

        RBUniRaff = new QRadioButton(GBUniform);
        RBUniRaff->setObjectName(QStringLiteral("RBUniRaff"));
        RBUniRaff->setChecked(true);

        gridLayout1->addWidget(RBUniRaff, 0, 0, 1, 1);


        gridLayout_5->addWidget(GBUniform, 3, 0, 1, 1);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_7, 3, 2, 1, 1);

        GBFieldFile = new QGroupBox(scrollAreaWidgetContents);
        GBFieldFile->setObjectName(QStringLiteral("GBFieldFile"));
        gridLayout2 = new QGridLayout(GBFieldFile);
#ifndef Q_OS_MAC
        gridLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout2->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout2->setObjectName(QStringLiteral("gridLayout2"));
        FieldFile = new QLabel(GBFieldFile);
        FieldFile->setObjectName(QStringLiteral("FieldFile"));

        gridLayout2->addWidget(FieldFile, 0, 0, 1, 1);

        LEFieldFile = new QLineEdit(GBFieldFile);
        LEFieldFile->setObjectName(QStringLiteral("LEFieldFile"));
        LEFieldFile->setMinimumSize(QSize(282, 31));

        gridLayout2->addWidget(LEFieldFile, 0, 1, 1, 1);


        gridLayout_5->addWidget(GBFieldFile, 4, 0, 1, 1);

        GBFieldManagement = new QGroupBox(scrollAreaWidgetContents);
        GBFieldManagement->setObjectName(QStringLiteral("GBFieldManagement"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(GBFieldManagement->sizePolicy().hasHeightForWidth());
        GBFieldManagement->setSizePolicy(sizePolicy1);
        gridLayout3 = new QGridLayout(GBFieldManagement);
#ifndef Q_OS_MAC
        gridLayout3->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout3->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout3->setObjectName(QStringLiteral("gridLayout3"));
        hboxLayout1 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout1->setContentsMargins(0, 0, 0, 0);
#endif
        hboxLayout1->setObjectName(QStringLiteral("hboxLayout1"));
        FieldName = new QLabel(GBFieldManagement);
        FieldName->setObjectName(QStringLiteral("FieldName"));

        hboxLayout1->addWidget(FieldName);

        CBFieldName = new QComboBox(GBFieldManagement);
        CBFieldName->setObjectName(QStringLiteral("CBFieldName"));
        sizePolicy1.setHeightForWidth(CBFieldName->sizePolicy().hasHeightForWidth());
        CBFieldName->setSizePolicy(sizePolicy1);
        CBFieldName->setEditable(false);
        CBFieldName->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        hboxLayout1->addWidget(CBFieldName);

        spacerItem = new QSpacerItem(48, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout1->addItem(spacerItem);


        gridLayout3->addLayout(hboxLayout1, 0, 0, 1, 1);

        hboxLayout2 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout2->setSpacing(6);
#endif
        hboxLayout2->setContentsMargins(0, 0, 0, 0);
        hboxLayout2->setObjectName(QStringLiteral("hboxLayout2"));
        TWCMP = new QTableWidget(GBFieldManagement);
        if (TWCMP->columnCount() < 2)
            TWCMP->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        TWCMP->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        TWCMP->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        TWCMP->setObjectName(QStringLiteral("TWCMP"));
        TWCMP->setEditTriggers(QAbstractItemView::AnyKeyPressed|QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed|QAbstractItemView::SelectedClicked);
        TWCMP->setShowGrid(true);
        TWCMP->setRowCount(0);
        TWCMP->setColumnCount(2);

        hboxLayout2->addWidget(TWCMP);

        spacerItem1 = new QSpacerItem(60, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout2->addItem(spacerItem1);

        vboxLayout = new QVBoxLayout();
#ifndef Q_OS_MAC
        vboxLayout->setSpacing(6);
#endif
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        vboxLayout->setObjectName(QStringLiteral("vboxLayout"));
        hboxLayout3 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout3->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        hboxLayout3->setContentsMargins(0, 0, 0, 0);
#endif
        hboxLayout3->setObjectName(QStringLiteral("hboxLayout3"));
        RBL2 = new QRadioButton(GBFieldManagement);
        RBL2->setObjectName(QStringLiteral("RBL2"));
        RBL2->setChecked(true);

        hboxLayout3->addWidget(RBL2);

        RBInf = new QRadioButton(GBFieldManagement);
        RBInf->setObjectName(QStringLiteral("RBInf"));
        RBInf->setChecked(false);

        hboxLayout3->addWidget(RBInf);


        vboxLayout->addLayout(hboxLayout3);

        CBJump = new QCheckBox(GBFieldManagement);
        CBJump->setObjectName(QStringLiteral("CBJump"));

        vboxLayout->addWidget(CBJump);


        hboxLayout2->addLayout(vboxLayout);


        gridLayout3->addLayout(hboxLayout2, 1, 0, 1, 1);

        hboxLayout4 = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout4->setSpacing(6);
#endif
        hboxLayout4->setContentsMargins(0, 0, 0, 0);
        hboxLayout4->setObjectName(QStringLiteral("hboxLayout4"));
        GBRefinementThresholds = new QGroupBox(GBFieldManagement);
        GBRefinementThresholds->setObjectName(QStringLiteral("GBRefinementThresholds"));
        sizePolicy1.setHeightForWidth(GBRefinementThresholds->sizePolicy().hasHeightForWidth());
        GBRefinementThresholds->setSizePolicy(sizePolicy1);
        gridLayout4 = new QGridLayout(GBRefinementThresholds);
#ifndef Q_OS_MAC
        gridLayout4->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout4->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout4->setObjectName(QStringLiteral("gridLayout4"));
        RBRPE = new QRadioButton(GBRefinementThresholds);
        RBRPE->setObjectName(QStringLiteral("RBRPE"));
        RBRPE->setCheckable(true);
        RBRPE->setChecked(true);

        gridLayout4->addWidget(RBRPE, 0, 0, 1, 1);

        SpinBox_RPE = new QDoubleSpinBox(GBRefinementThresholds);
        SpinBox_RPE->setObjectName(QStringLiteral("SpinBox_RPE"));
        SpinBox_RPE->setDecimals(3);
        SpinBox_RPE->setMaximum(100);
        SpinBox_RPE->setSingleStep(0.1);
        SpinBox_RPE->setValue(2);

        gridLayout4->addWidget(SpinBox_RPE, 0, 1, 1, 1);

        RBRRel = new QRadioButton(GBRefinementThresholds);
        RBRRel->setObjectName(QStringLiteral("RBRRel"));

        gridLayout4->addWidget(RBRRel, 1, 0, 1, 1);

        SpinBox_RRel = new QDoubleSpinBox(GBRefinementThresholds);
        SpinBox_RRel->setObjectName(QStringLiteral("SpinBox_RRel"));
        SpinBox_RRel->setEnabled(false);
        SpinBox_RRel->setDecimals(3);
        SpinBox_RRel->setMaximum(100);
        SpinBox_RRel->setSingleStep(0.1);

        gridLayout4->addWidget(SpinBox_RRel, 1, 1, 1, 1);

        RBRAbs = new QRadioButton(GBRefinementThresholds);
        RBRAbs->setObjectName(QStringLiteral("RBRAbs"));

        gridLayout4->addWidget(RBRAbs, 2, 0, 1, 1);

        SpinBox_RAbs = new QDoubleSpinBox(GBRefinementThresholds);
        SpinBox_RAbs->setObjectName(QStringLiteral("SpinBox_RAbs"));
        SpinBox_RAbs->setEnabled(false);
        SpinBox_RAbs->setDecimals(8);
        SpinBox_RAbs->setMinimum(-1e+12);
        SpinBox_RAbs->setMaximum(1e+12);
        SpinBox_RAbs->setSingleStep(0.1);

        gridLayout4->addWidget(SpinBox_RAbs, 2, 1, 1, 1);

        RBRMuSigma = new QRadioButton(GBRefinementThresholds);
        RBRMuSigma->setObjectName(QStringLiteral("RBRMuSigma"));

        gridLayout4->addWidget(RBRMuSigma, 3, 0, 1, 1);

        SpinBox_RMuSigma = new QDoubleSpinBox(GBRefinementThresholds);
        SpinBox_RMuSigma->setObjectName(QStringLiteral("SpinBox_RMuSigma"));
        SpinBox_RMuSigma->setEnabled(false);
        SpinBox_RMuSigma->setDecimals(8);
        SpinBox_RMuSigma->setMinimum(-1e+12);
        SpinBox_RMuSigma->setMaximum(1e+12);
        SpinBox_RMuSigma->setSingleStep(0.1);
        SpinBox_RMuSigma->setValue(3);

        gridLayout4->addWidget(SpinBox_RMuSigma, 3, 1, 1, 1);

        RBRNo = new QRadioButton(GBRefinementThresholds);
        RBRNo->setObjectName(QStringLiteral("RBRNo"));

        gridLayout4->addWidget(RBRNo, 4, 0, 1, 1);


        hboxLayout4->addWidget(GBRefinementThresholds);

        GBCoarseningThresholds = new QGroupBox(GBFieldManagement);
        GBCoarseningThresholds->setObjectName(QStringLiteral("GBCoarseningThresholds"));
        gridLayout5 = new QGridLayout(GBCoarseningThresholds);
#ifndef Q_OS_MAC
        gridLayout5->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout5->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout5->setObjectName(QStringLiteral("gridLayout5"));
        RBCPE = new QRadioButton(GBCoarseningThresholds);
        RBCPE->setObjectName(QStringLiteral("RBCPE"));
        RBCPE->setCheckable(true);
        RBCPE->setChecked(false);

        gridLayout5->addWidget(RBCPE, 0, 0, 1, 1);

        SpinBox_CPE = new QDoubleSpinBox(GBCoarseningThresholds);
        SpinBox_CPE->setObjectName(QStringLiteral("SpinBox_CPE"));
        SpinBox_CPE->setEnabled(false);
        SpinBox_CPE->setDecimals(3);
        SpinBox_CPE->setMaximum(100);
        SpinBox_CPE->setSingleStep(0.1);

        gridLayout5->addWidget(SpinBox_CPE, 0, 1, 1, 1);

        RBCRel = new QRadioButton(GBCoarseningThresholds);
        RBCRel->setObjectName(QStringLiteral("RBCRel"));

        gridLayout5->addWidget(RBCRel, 1, 0, 1, 1);

        SpinBox_CRel = new QDoubleSpinBox(GBCoarseningThresholds);
        SpinBox_CRel->setObjectName(QStringLiteral("SpinBox_CRel"));
        SpinBox_CRel->setEnabled(false);
        SpinBox_CRel->setDecimals(3);
        SpinBox_CRel->setMaximum(100);
        SpinBox_CRel->setSingleStep(0.1);

        gridLayout5->addWidget(SpinBox_CRel, 1, 1, 1, 1);

        RBCAbs = new QRadioButton(GBCoarseningThresholds);
        RBCAbs->setObjectName(QStringLiteral("RBCAbs"));

        gridLayout5->addWidget(RBCAbs, 2, 0, 1, 1);

        SpinBox_CAbs = new QDoubleSpinBox(GBCoarseningThresholds);
        SpinBox_CAbs->setObjectName(QStringLiteral("SpinBox_CAbs"));
        SpinBox_CAbs->setEnabled(false);
        SpinBox_CAbs->setDecimals(8);
        SpinBox_CAbs->setMinimum(-1e+12);
        SpinBox_CAbs->setMaximum(1e+12);
        SpinBox_CAbs->setSingleStep(0.1);

        gridLayout5->addWidget(SpinBox_CAbs, 2, 1, 1, 1);

        RBCMuSigma = new QRadioButton(GBCoarseningThresholds);
        RBCMuSigma->setObjectName(QStringLiteral("RBCMuSigma"));

        gridLayout5->addWidget(RBCMuSigma, 3, 0, 1, 1);

        SpinBox_CMuSigma = new QDoubleSpinBox(GBCoarseningThresholds);
        SpinBox_CMuSigma->setObjectName(QStringLiteral("SpinBox_CMuSigma"));
        SpinBox_CMuSigma->setEnabled(false);
        SpinBox_CMuSigma->setDecimals(8);
        SpinBox_CMuSigma->setMinimum(-1e+12);
        SpinBox_CMuSigma->setMaximum(1e+12);
        SpinBox_CMuSigma->setSingleStep(0.1);
        SpinBox_CMuSigma->setValue(4);

        gridLayout5->addWidget(SpinBox_CMuSigma, 3, 1, 1, 1);

        RBCNo = new QRadioButton(GBCoarseningThresholds);
        RBCNo->setObjectName(QStringLiteral("RBCNo"));
        RBCNo->setChecked(true);

        gridLayout5->addWidget(RBCNo, 4, 0, 1, 1);


        hboxLayout4->addWidget(GBCoarseningThresholds);


        gridLayout3->addLayout(hboxLayout4, 2, 0, 1, 1);


        gridLayout_5->addWidget(GBFieldManagement, 5, 0, 1, 3);

        GBAreaManagement = new QGroupBox(scrollAreaWidgetContents);
        GBAreaManagement->setObjectName(QStringLiteral("GBAreaManagement"));
        sizePolicy1.setHeightForWidth(GBAreaManagement->sizePolicy().hasHeightForWidth());
        GBAreaManagement->setSizePolicy(sizePolicy1);
        gridLayout_4 = new QGridLayout(GBAreaManagement);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        TWZone = new QTableWidget(GBAreaManagement);
        if (TWZone->columnCount() < 3)
            TWZone->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        TWZone->setHorizontalHeaderItem(0, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        TWZone->setHorizontalHeaderItem(1, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        TWZone->setHorizontalHeaderItem(2, __qtablewidgetitem4);
        TWZone->setObjectName(QStringLiteral("TWZone"));
        TWZone->setMinimumSize(QSize(400, 0));
        TWZone->setEditTriggers(QAbstractItemView::AnyKeyPressed|QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed|QAbstractItemView::SelectedClicked);
        TWZone->setShowGrid(true);
        TWZone->setRowCount(0);
        TWZone->setColumnCount(3);

        gridLayout_4->addWidget(TWZone, 0, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer, 0, 1, 1, 1);

        vboxLayout1 = new QVBoxLayout();
#ifndef Q_OS_MAC
        vboxLayout1->setSpacing(6);
#endif
        vboxLayout1->setContentsMargins(0, 0, 0, 0);
        vboxLayout1->setObjectName(QStringLiteral("vboxLayout1"));
        vboxLayout2 = new QVBoxLayout();
#ifndef Q_OS_MAC
        vboxLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout2->setContentsMargins(0, 0, 0, 0);
#endif
        vboxLayout2->setObjectName(QStringLiteral("vboxLayout2"));
        PBZoneNew = new QPushButton(GBAreaManagement);
        PBZoneNew->setObjectName(QStringLiteral("PBZoneNew"));

        vboxLayout2->addWidget(PBZoneNew);

        PBZoneEdit = new QPushButton(GBAreaManagement);
        PBZoneEdit->setObjectName(QStringLiteral("PBZoneEdit"));

        vboxLayout2->addWidget(PBZoneEdit);

        PBZoneDelete = new QPushButton(GBAreaManagement);
        PBZoneDelete->setObjectName(QStringLiteral("PBZoneDelete"));

        vboxLayout2->addWidget(PBZoneDelete);


        vboxLayout1->addLayout(vboxLayout2);

        spacerItem2 = new QSpacerItem(20, 48, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout1->addItem(spacerItem2);


        gridLayout_4->addLayout(vboxLayout1, 0, 2, 1, 1);


        gridLayout_5->addWidget(GBAreaManagement, 6, 0, 1, 2);

        GBField = new QGroupBox(scrollAreaWidgetContents);
        GBField->setObjectName(QStringLiteral("GBField"));
        gridLayout6 = new QGridLayout(GBField);
#ifndef Q_OS_MAC
        gridLayout6->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout6->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout6->setObjectName(QStringLiteral("gridLayout6"));
        gridLayout7 = new QGridLayout();
#ifndef Q_OS_MAC
        gridLayout7->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout7->setContentsMargins(0, 0, 0, 0);
#endif
        gridLayout7->setObjectName(QStringLiteral("gridLayout7"));
        RBFieldNo = new QRadioButton(GBField);
        RBFieldNo->setObjectName(QStringLiteral("RBFieldNo"));
        RBFieldNo->setChecked(true);

        gridLayout7->addWidget(RBFieldNo, 0, 0, 1, 1);

        RBFieldAll = new QRadioButton(GBField);
        RBFieldAll->setObjectName(QStringLiteral("RBFieldAll"));
        RBFieldAll->setChecked(false);

        gridLayout7->addWidget(RBFieldAll, 0, 1, 1, 1);

        RBFieldChosen = new QRadioButton(GBField);
        RBFieldChosen->setObjectName(QStringLiteral("RBFieldChosen"));
        RBFieldChosen->setChecked(false);

        gridLayout7->addWidget(RBFieldChosen, 0, 2, 1, 1);


        gridLayout6->addLayout(gridLayout7, 0, 0, 1, 1);

        TWField = new QTableWidget(GBField);
        if (TWField->columnCount() < 2)
            TWField->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        TWField->setHorizontalHeaderItem(0, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        TWField->setHorizontalHeaderItem(1, __qtablewidgetitem6);
        TWField->setObjectName(QStringLiteral("TWField"));

        gridLayout6->addWidget(TWField, 1, 0, 1, 1);


        gridLayout_5->addWidget(GBField, 7, 0, 1, 1);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_5, 7, 2, 1, 1);

        CBAdvanced = new QCheckBox(scrollAreaWidgetContents);
        CBAdvanced->setObjectName(QStringLiteral("CBAdvanced"));

        gridLayout_5->addWidget(CBAdvanced, 8, 0, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_4, 8, 2, 1, 1);

        GBAdvancedOptions = new QGroupBox(scrollAreaWidgetContents);
        GBAdvancedOptions->setObjectName(QStringLiteral("GBAdvancedOptions"));
        gridLayout_3 = new QGridLayout(GBAdvancedOptions);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        TLMinimalDiameter = new QLabel(GBAdvancedOptions);
        TLMinimalDiameter->setObjectName(QStringLiteral("TLMinimalDiameter"));
        sizePolicy1.setHeightForWidth(TLMinimalDiameter->sizePolicy().hasHeightForWidth());
        TLMinimalDiameter->setSizePolicy(sizePolicy1);
        TLMinimalDiameter->setWordWrap(false);

        gridLayout_3->addWidget(TLMinimalDiameter, 0, 0, 1, 1);

        doubleSpinBoxDiamMin = new QDoubleSpinBox(GBAdvancedOptions);
        doubleSpinBoxDiamMin->setObjectName(QStringLiteral("doubleSpinBoxDiamMin"));
        doubleSpinBoxDiamMin->setDecimals(7);

        gridLayout_3->addWidget(doubleSpinBoxDiamMin, 0, 1, 1, 1);

        TLMaximalLevel = new QLabel(GBAdvancedOptions);
        TLMaximalLevel->setObjectName(QStringLiteral("TLMaximalLevel"));
        sizePolicy1.setHeightForWidth(TLMaximalLevel->sizePolicy().hasHeightForWidth());
        TLMaximalLevel->setSizePolicy(sizePolicy1);
        TLMaximalLevel->setWordWrap(false);

        gridLayout_3->addWidget(TLMaximalLevel, 1, 0, 1, 1);

        SpinBoxNivMax = new QSpinBox(GBAdvancedOptions);
        SpinBoxNivMax->setObjectName(QStringLiteral("SpinBoxNivMax"));
        SpinBoxNivMax->setValue(99);

        gridLayout_3->addWidget(SpinBoxNivMax, 1, 1, 1, 1);

        GBAdapInit = new QGroupBox(GBAdvancedOptions);
        GBAdapInit->setObjectName(QStringLiteral("GBAdapInit"));
        gridLayout_2 = new QGridLayout(GBAdapInit);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        RBAIN = new QRadioButton(GBAdapInit);
        RBAIN->setObjectName(QStringLiteral("RBAIN"));
        RBAIN->setChecked(true);

        gridLayout_2->addWidget(RBAIN, 0, 0, 1, 1);

        RBAIR = new QRadioButton(GBAdapInit);
        RBAIR->setObjectName(QStringLiteral("RBAIR"));

        gridLayout_2->addWidget(RBAIR, 0, 1, 1, 1);

        RBAID = new QRadioButton(GBAdapInit);
        RBAID->setObjectName(QStringLiteral("RBAID"));

        gridLayout_2->addWidget(RBAID, 0, 2, 1, 1);


        gridLayout_3->addWidget(GBAdapInit, 2, 0, 1, 2);

        CBOutputLevel = new QCheckBox(GBAdvancedOptions);
        CBOutputLevel->setObjectName(QStringLiteral("CBOutputLevel"));

        gridLayout_3->addWidget(CBOutputLevel, 3, 0, 1, 1);

        CBOutputQuality = new QCheckBox(GBAdvancedOptions);
        CBOutputQuality->setObjectName(QStringLiteral("CBOutputQuality"));

        gridLayout_3->addWidget(CBOutputQuality, 4, 0, 1, 1);

        CBOutputDiameter = new QCheckBox(GBAdvancedOptions);
        CBOutputDiameter->setObjectName(QStringLiteral("CBOutputDiameter"));

        gridLayout_3->addWidget(CBOutputDiameter, 5, 0, 1, 1);

        CBOutputParent = new QCheckBox(GBAdvancedOptions);
        CBOutputParent->setObjectName(QStringLiteral("CBOutputParent"));

        gridLayout_3->addWidget(CBOutputParent, 6, 0, 1, 1);

        CBOutputVoisins = new QCheckBox(GBAdvancedOptions);
        CBOutputVoisins->setObjectName(QStringLiteral("CBOutputVoisins"));

        gridLayout_3->addWidget(CBOutputVoisins, 7, 0, 1, 1);


        gridLayout_5->addWidget(GBAdvancedOptions, 9, 0, 2, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_3, 9, 2, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_2, 10, 1, 1, 1);

        GBButtons = new QGroupBox(scrollAreaWidgetContents);
        GBButtons->setObjectName(QStringLiteral("GBButtons"));
        gridLayout8 = new QGridLayout(GBButtons);
#ifndef Q_OS_MAC
        gridLayout8->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout8->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout8->setObjectName(QStringLiteral("gridLayout8"));
        buttonOk = new QPushButton(GBButtons);
        buttonOk->setObjectName(QStringLiteral("buttonOk"));

        gridLayout8->addWidget(buttonOk, 0, 0, 1, 1);

        buttonApply = new QPushButton(GBButtons);
        buttonApply->setObjectName(QStringLiteral("buttonApply"));

        gridLayout8->addWidget(buttonApply, 0, 1, 1, 1);

        buttonCancel = new QPushButton(GBButtons);
        buttonCancel->setObjectName(QStringLiteral("buttonCancel"));

        gridLayout8->addWidget(buttonCancel, 0, 2, 1, 1);

        buttonHelp = new QPushButton(GBButtons);
        buttonHelp->setObjectName(QStringLiteral("buttonHelp"));

        gridLayout8->addWidget(buttonHelp, 0, 3, 1, 1);


        gridLayout_5->addWidget(GBButtons, 11, 0, 1, 1);

        CreateHypothesis->setWidget(scrollAreaWidgetContents);

        retranslateUi(CreateHypothesis);

        QMetaObject::connectSlotsByName(CreateHypothesis);
    } // setupUi

    void retranslateUi(QScrollArea *CreateHypothesis)
    {
        CreateHypothesis->setWindowTitle(QApplication::translate("CreateHypothesis", "Create a hypothesis", Q_NULLPTR));
        Name->setText(QApplication::translate("CreateHypothesis", "Name", Q_NULLPTR));
        GBTypeAdaptation->setTitle(QApplication::translate("CreateHypothesis", "Type of adaptation", Q_NULLPTR));
        RBUniforme->setText(QApplication::translate("CreateHypothesis", "Uniform", Q_NULLPTR));
        RBChamp->setText(QApplication::translate("CreateHypothesis", "Driven by a field", Q_NULLPTR));
        RBZone->setText(QApplication::translate("CreateHypothesis", "With geometrical zones", Q_NULLPTR));
        CBGroupe->setText(QApplication::translate("CreateHypothesis", "Filtering with groups", Q_NULLPTR));
        GBUniform->setTitle(QApplication::translate("CreateHypothesis", "Uniform adaptation", Q_NULLPTR));
        RBUniDera->setText(QApplication::translate("CreateHypothesis", "Coarsening", Q_NULLPTR));
        RBUniRaff->setText(QApplication::translate("CreateHypothesis", "Refinement", Q_NULLPTR));
        GBFieldFile->setTitle(QString());
        FieldFile->setText(QApplication::translate("CreateHypothesis", "File of the fields", Q_NULLPTR));
        GBFieldManagement->setTitle(QApplication::translate("CreateHypothesis", "Governing field for the adaptation", Q_NULLPTR));
        FieldName->setText(QApplication::translate("CreateHypothesis", "Field name", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem = TWCMP->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("CreateHypothesis", "Selection", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem1 = TWCMP->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("CreateHypothesis", "Component", Q_NULLPTR));
        RBL2->setText(QApplication::translate("CreateHypothesis", "L2 norm", Q_NULLPTR));
        RBInf->setText(QApplication::translate("CreateHypothesis", "Infinite norm", Q_NULLPTR));
        CBJump->setText(QApplication::translate("CreateHypothesis", "Jump between elements", Q_NULLPTR));
        GBRefinementThresholds->setTitle(QApplication::translate("CreateHypothesis", "Refinement threshold", Q_NULLPTR));
        RBRPE->setText(QApplication::translate("CreateHypothesis", "Percentage of meshes", Q_NULLPTR));
        SpinBox_RPE->setSuffix(QApplication::translate("CreateHypothesis", " %", Q_NULLPTR));
        RBRRel->setText(QApplication::translate("CreateHypothesis", "Relative", Q_NULLPTR));
        SpinBox_RRel->setSuffix(QApplication::translate("CreateHypothesis", " %", Q_NULLPTR));
        RBRAbs->setText(QApplication::translate("CreateHypothesis", "Absolute", Q_NULLPTR));
        RBRMuSigma->setText(QApplication::translate("CreateHypothesis", "Mean + n*(std deviation)", Q_NULLPTR));
        RBRNo->setText(QApplication::translate("CreateHypothesis", "No refinement", Q_NULLPTR));
        GBCoarseningThresholds->setTitle(QApplication::translate("CreateHypothesis", "Coarsening threshold", Q_NULLPTR));
        RBCPE->setText(QApplication::translate("CreateHypothesis", "Percentage of meshes", Q_NULLPTR));
        SpinBox_CPE->setSuffix(QApplication::translate("CreateHypothesis", " %", Q_NULLPTR));
        RBCRel->setText(QApplication::translate("CreateHypothesis", "Relative", Q_NULLPTR));
        SpinBox_CRel->setSuffix(QApplication::translate("CreateHypothesis", " %", Q_NULLPTR));
        RBCAbs->setText(QApplication::translate("CreateHypothesis", "Absolute", Q_NULLPTR));
        RBCMuSigma->setText(QApplication::translate("CreateHypothesis", "Mean - n*(std deviation)", Q_NULLPTR));
        RBCNo->setText(QApplication::translate("CreateHypothesis", "No coarsening", Q_NULLPTR));
        GBAreaManagement->setTitle(QApplication::translate("CreateHypothesis", "Zone management", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem2 = TWZone->horizontalHeaderItem(0);
        ___qtablewidgetitem2->setText(QApplication::translate("CreateHypothesis", "Refinement", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem3 = TWZone->horizontalHeaderItem(1);
        ___qtablewidgetitem3->setText(QApplication::translate("CreateHypothesis", "Coarsening", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem4 = TWZone->horizontalHeaderItem(2);
        ___qtablewidgetitem4->setText(QApplication::translate("CreateHypothesis", "Zone name", Q_NULLPTR));
        PBZoneNew->setText(QApplication::translate("CreateHypothesis", "New", Q_NULLPTR));
        PBZoneEdit->setText(QApplication::translate("CreateHypothesis", "Edit", Q_NULLPTR));
        PBZoneDelete->setText(QString());
        GBField->setTitle(QApplication::translate("CreateHypothesis", "Field Interpolation", Q_NULLPTR));
        RBFieldNo->setText(QApplication::translate("CreateHypothesis", "None", Q_NULLPTR));
        RBFieldAll->setText(QApplication::translate("CreateHypothesis", "All", Q_NULLPTR));
        RBFieldChosen->setText(QApplication::translate("CreateHypothesis", "Chosen", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem5 = TWField->horizontalHeaderItem(0);
        ___qtablewidgetitem5->setText(QApplication::translate("CreateHypothesis", "Selection", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem6 = TWField->horizontalHeaderItem(1);
        ___qtablewidgetitem6->setText(QApplication::translate("CreateHypothesis", "Field Name", Q_NULLPTR));
        CBAdvanced->setText(QApplication::translate("CreateHypothesis", "Advanced options", Q_NULLPTR));
        GBAdvancedOptions->setTitle(QApplication::translate("CreateHypothesis", "Advanced options", Q_NULLPTR));
        TLMinimalDiameter->setText(QApplication::translate("CreateHypothesis", "Minimal diameter", Q_NULLPTR));
        TLMaximalLevel->setText(QApplication::translate("CreateHypothesis", "Maximal level", Q_NULLPTR));
        GBAdapInit->setTitle(QApplication::translate("CreateHypothesis", "Initialization of adaptation", Q_NULLPTR));
        RBAIN->setText(QApplication::translate("CreateHypothesis", "Nothing", Q_NULLPTR));
        RBAIR->setText(QApplication::translate("CreateHypothesis", "Refinement", Q_NULLPTR));
        RBAID->setText(QApplication::translate("CreateHypothesis", "Coarsening", Q_NULLPTR));
        CBOutputLevel->setText(QApplication::translate("CreateHypothesis", "Output of the level of refinement", Q_NULLPTR));
        CBOutputQuality->setText(QApplication::translate("CreateHypothesis", "Output of the qualities", Q_NULLPTR));
        CBOutputDiameter->setText(QApplication::translate("CreateHypothesis", "Output of the diameters", Q_NULLPTR));
        CBOutputParent->setText(QApplication::translate("CreateHypothesis", "Output of the parents", Q_NULLPTR));
        CBOutputVoisins->setText(QApplication::translate("CreateHypothesis", "Output of the neighbours", Q_NULLPTR));
        GBButtons->setTitle(QString());
        buttonOk->setText(QApplication::translate("CreateHypothesis", "OK", Q_NULLPTR));
        buttonApply->setText(QApplication::translate("CreateHypothesis", "Apply", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("CreateHypothesis", "Cancel", Q_NULLPTR));
        buttonHelp->setText(QApplication::translate("CreateHypothesis", "Help", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CreateHypothesis: public Ui_CreateHypothesis {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATEHYPOTHESIS_H
