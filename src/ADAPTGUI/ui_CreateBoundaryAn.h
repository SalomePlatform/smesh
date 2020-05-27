/********************************************************************************
** Form generated from reading UI file 'CreateBoundaryAn.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATEBOUNDARYAN_H
#define UI_CREATEBOUNDARYAN_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>

QT_BEGIN_NAMESPACE

class Ui_CreateBoundaryAn
{
public:
    QGridLayout *gridLayout_5;
    QLabel *Name;
    QLineEdit *LEName;
    QGroupBox *TypeBoundary;
    QGridLayout *gridLayout;
    QRadioButton *RBCylindre;
    QRadioButton *RBSphere;
    QRadioButton *RBCone;
    QRadioButton *RBTore;
    QGroupBox *gBCylindre;
    QGridLayout *gridLayout1;
    QDoubleSpinBox *SpinBox_Xcent;
    QLabel *TLXcent;
    QDoubleSpinBox *SpinBox_Radius;
    QDoubleSpinBox *SpinBox_Zcent;
    QLabel *TLradius;
    QLabel *TLZcent;
    QLabel *TLYcent;
    QDoubleSpinBox *SpinBox_Ycent;
    QLabel *TLXaxis;
    QLabel *TLYaxis;
    QLabel *TLZaxis;
    QDoubleSpinBox *SpinBox_Zaxis;
    QDoubleSpinBox *SpinBox_Yaxis;
    QDoubleSpinBox *SpinBox_Xaxis;
    QGroupBox *gBSphere;
    QGridLayout *gridLayout2;
    QDoubleSpinBox *SpinBox_Rayon;
    QDoubleSpinBox *SpinBox_Zcentre;
    QLabel *TLRayon;
    QLabel *TLZcentre;
    QLabel *TLYcentre;
    QDoubleSpinBox *SpinBox_Ycentre;
    QDoubleSpinBox *SpinBox_Xcentre;
    QLabel *TLXcentre;
    QGroupBox *gBCone;
    QGridLayout *gridLayout_3;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QRadioButton *RB_Def_radius;
    QRadioButton *RB_Def_angle;
    QLabel *TLCone_X1;
    QDoubleSpinBox *SpinBox_Cone_X1;
    QLabel *TLCone_X2;
    QDoubleSpinBox *SpinBox_Cone_X2;
    QLabel *TLCone_Y1;
    QDoubleSpinBox *SpinBox_Cone_Y1;
    QLabel *TLCone_Y2;
    QDoubleSpinBox *SpinBox_Cone_Y2;
    QLabel *TLCone_Z1;
    QDoubleSpinBox *SpinBox_Cone_Z1;
    QLabel *TLCone_Z2;
    QDoubleSpinBox *SpinBox_Cone_Z2;
    QLabel *TLCone_V1;
    QDoubleSpinBox *SpinBox_Cone_V1;
    QLabel *TLCone_V2;
    QDoubleSpinBox *SpinBox_Cone_V2;
    QGroupBox *gBTore;
    QGridLayout *gridLayout_4;
    QLabel *TLToreXcent;
    QDoubleSpinBox *SpinBoxToreXcent;
    QLabel *TLToreXaxe;
    QDoubleSpinBox *SpinBoxToreXaxe;
    QLabel *TLToreYcent;
    QDoubleSpinBox *SpinBoxToreYcent;
    QLabel *TLToreYaxe;
    QDoubleSpinBox *SpinBoxToreYaxe;
    QLabel *TLToreZcent;
    QDoubleSpinBox *SpinBoxToreZcent;
    QLabel *TLToreZaxe;
    QDoubleSpinBox *SpinBoxToreZaxe;
    QLabel *TLToreRayRev;
    QDoubleSpinBox *SpinBoxToreRRev;
    QLabel *TLToreRayPri;
    QDoubleSpinBox *SpinBoxToreRPri;
    QGroupBox *GBButtons;
    QGridLayout *gridLayout3;
    QPushButton *buttonHelp;
    QPushButton *buttonCancel;
    QPushButton *buttonApply;
    QPushButton *buttonOk;

    void setupUi(QDialog *CreateBoundaryAn)
    {
        if (CreateBoundaryAn->objectName().isEmpty())
            CreateBoundaryAn->setObjectName(QStringLiteral("CreateBoundaryAn"));
        CreateBoundaryAn->resize(522, 835);
        CreateBoundaryAn->setAutoFillBackground(true);
        CreateBoundaryAn->setSizeGripEnabled(true);
        gridLayout_5 = new QGridLayout(CreateBoundaryAn);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        Name = new QLabel(CreateBoundaryAn);
        Name->setObjectName(QStringLiteral("Name"));

        gridLayout_5->addWidget(Name, 0, 0, 1, 1);

        LEName = new QLineEdit(CreateBoundaryAn);
        LEName->setObjectName(QStringLiteral("LEName"));
        LEName->setMaxLength(32);

        gridLayout_5->addWidget(LEName, 0, 1, 1, 1);

        TypeBoundary = new QGroupBox(CreateBoundaryAn);
        TypeBoundary->setObjectName(QStringLiteral("TypeBoundary"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(TypeBoundary->sizePolicy().hasHeightForWidth());
        TypeBoundary->setSizePolicy(sizePolicy);
        TypeBoundary->setMinimumSize(QSize(340, 0));
        gridLayout = new QGridLayout(TypeBoundary);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        RBCylindre = new QRadioButton(TypeBoundary);
        RBCylindre->setObjectName(QStringLiteral("RBCylindre"));
        QIcon icon;
        icon.addFile(QStringLiteral("../../resources/cylinderpointvector.png"), QSize(), QIcon::Normal, QIcon::Off);
        RBCylindre->setIcon(icon);
        RBCylindre->setCheckable(true);
        RBCylindre->setChecked(true);

        gridLayout->addWidget(RBCylindre, 0, 0, 1, 1);

        RBSphere = new QRadioButton(TypeBoundary);
        RBSphere->setObjectName(QStringLiteral("RBSphere"));
        QIcon icon1;
        icon1.addFile(QStringLiteral("../../resources/zone_spherepoint.png"), QSize(), QIcon::Normal, QIcon::Off);
        RBSphere->setIcon(icon1);

        gridLayout->addWidget(RBSphere, 0, 1, 1, 1);

        RBCone = new QRadioButton(TypeBoundary);
        RBCone->setObjectName(QStringLiteral("RBCone"));
        QIcon icon2;
        icon2.addFile(QStringLiteral("../../resources/cone.png"), QSize(), QIcon::Normal, QIcon::Off);
        RBCone->setIcon(icon2);

        gridLayout->addWidget(RBCone, 0, 2, 1, 1);

        RBTore = new QRadioButton(TypeBoundary);
        RBTore->setObjectName(QStringLiteral("RBTore"));
        QIcon icon3;
        icon3.addFile(QStringLiteral("../../resources/toruspointvector.png"), QSize(), QIcon::Normal, QIcon::Off);
        RBTore->setIcon(icon3);
        RBTore->setCheckable(true);
        RBTore->setChecked(false);

        gridLayout->addWidget(RBTore, 0, 3, 1, 1);


        gridLayout_5->addWidget(TypeBoundary, 1, 0, 1, 2);

        gBCylindre = new QGroupBox(CreateBoundaryAn);
        gBCylindre->setObjectName(QStringLiteral("gBCylindre"));
        sizePolicy.setHeightForWidth(gBCylindre->sizePolicy().hasHeightForWidth());
        gBCylindre->setSizePolicy(sizePolicy);
        gridLayout1 = new QGridLayout(gBCylindre);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QStringLiteral("gridLayout1"));
        SpinBox_Xcent = new QDoubleSpinBox(gBCylindre);
        SpinBox_Xcent->setObjectName(QStringLiteral("SpinBox_Xcent"));
        SpinBox_Xcent->setDecimals(5);
        SpinBox_Xcent->setMinimum(-1e+9);
        SpinBox_Xcent->setMaximum(1e+9);
        SpinBox_Xcent->setValue(0);

        gridLayout1->addWidget(SpinBox_Xcent, 0, 1, 1, 1);

        TLXcent = new QLabel(gBCylindre);
        TLXcent->setObjectName(QStringLiteral("TLXcent"));
        sizePolicy.setHeightForWidth(TLXcent->sizePolicy().hasHeightForWidth());
        TLXcent->setSizePolicy(sizePolicy);
        TLXcent->setWordWrap(false);

        gridLayout1->addWidget(TLXcent, 0, 0, 1, 1);

        SpinBox_Radius = new QDoubleSpinBox(gBCylindre);
        SpinBox_Radius->setObjectName(QStringLiteral("SpinBox_Radius"));
        SpinBox_Radius->setDecimals(5);
        SpinBox_Radius->setMaximum(1e+9);

        gridLayout1->addWidget(SpinBox_Radius, 3, 1, 1, 2);

        SpinBox_Zcent = new QDoubleSpinBox(gBCylindre);
        SpinBox_Zcent->setObjectName(QStringLiteral("SpinBox_Zcent"));
        SpinBox_Zcent->setDecimals(5);
        SpinBox_Zcent->setMinimum(-1e+9);
        SpinBox_Zcent->setMaximum(1e+9);
        SpinBox_Zcent->setValue(0);

        gridLayout1->addWidget(SpinBox_Zcent, 2, 1, 1, 1);

        TLradius = new QLabel(gBCylindre);
        TLradius->setObjectName(QStringLiteral("TLradius"));
        sizePolicy.setHeightForWidth(TLradius->sizePolicy().hasHeightForWidth());
        TLradius->setSizePolicy(sizePolicy);
        TLradius->setWordWrap(false);

        gridLayout1->addWidget(TLradius, 3, 0, 1, 1);

        TLZcent = new QLabel(gBCylindre);
        TLZcent->setObjectName(QStringLiteral("TLZcent"));
        sizePolicy.setHeightForWidth(TLZcent->sizePolicy().hasHeightForWidth());
        TLZcent->setSizePolicy(sizePolicy);
        TLZcent->setWordWrap(false);

        gridLayout1->addWidget(TLZcent, 2, 0, 1, 1);

        TLYcent = new QLabel(gBCylindre);
        TLYcent->setObjectName(QStringLiteral("TLYcent"));
        sizePolicy.setHeightForWidth(TLYcent->sizePolicy().hasHeightForWidth());
        TLYcent->setSizePolicy(sizePolicy);
        TLYcent->setWordWrap(false);

        gridLayout1->addWidget(TLYcent, 1, 0, 1, 1);

        SpinBox_Ycent = new QDoubleSpinBox(gBCylindre);
        SpinBox_Ycent->setObjectName(QStringLiteral("SpinBox_Ycent"));
        SpinBox_Ycent->setDecimals(5);
        SpinBox_Ycent->setMinimum(-1e+9);
        SpinBox_Ycent->setMaximum(1e+9);
        SpinBox_Ycent->setValue(0);

        gridLayout1->addWidget(SpinBox_Ycent, 1, 1, 1, 1);

        TLXaxis = new QLabel(gBCylindre);
        TLXaxis->setObjectName(QStringLiteral("TLXaxis"));
        sizePolicy.setHeightForWidth(TLXaxis->sizePolicy().hasHeightForWidth());
        TLXaxis->setSizePolicy(sizePolicy);
        TLXaxis->setWordWrap(false);

        gridLayout1->addWidget(TLXaxis, 0, 2, 1, 1);

        TLYaxis = new QLabel(gBCylindre);
        TLYaxis->setObjectName(QStringLiteral("TLYaxis"));
        sizePolicy.setHeightForWidth(TLYaxis->sizePolicy().hasHeightForWidth());
        TLYaxis->setSizePolicy(sizePolicy);
        TLYaxis->setWordWrap(false);

        gridLayout1->addWidget(TLYaxis, 1, 2, 1, 1);

        TLZaxis = new QLabel(gBCylindre);
        TLZaxis->setObjectName(QStringLiteral("TLZaxis"));
        sizePolicy.setHeightForWidth(TLZaxis->sizePolicy().hasHeightForWidth());
        TLZaxis->setSizePolicy(sizePolicy);
        TLZaxis->setWordWrap(false);

        gridLayout1->addWidget(TLZaxis, 2, 2, 1, 1);

        SpinBox_Zaxis = new QDoubleSpinBox(gBCylindre);
        SpinBox_Zaxis->setObjectName(QStringLiteral("SpinBox_Zaxis"));
        SpinBox_Zaxis->setDecimals(5);
        SpinBox_Zaxis->setMinimum(-1e+9);
        SpinBox_Zaxis->setMaximum(1e+9);
        SpinBox_Zaxis->setValue(0);

        gridLayout1->addWidget(SpinBox_Zaxis, 2, 3, 1, 1);

        SpinBox_Yaxis = new QDoubleSpinBox(gBCylindre);
        SpinBox_Yaxis->setObjectName(QStringLiteral("SpinBox_Yaxis"));
        SpinBox_Yaxis->setDecimals(5);
        SpinBox_Yaxis->setMinimum(-1e+9);
        SpinBox_Yaxis->setMaximum(1e+9);
        SpinBox_Yaxis->setValue(0);

        gridLayout1->addWidget(SpinBox_Yaxis, 1, 3, 1, 1);

        SpinBox_Xaxis = new QDoubleSpinBox(gBCylindre);
        SpinBox_Xaxis->setObjectName(QStringLiteral("SpinBox_Xaxis"));
        SpinBox_Xaxis->setDecimals(5);
        SpinBox_Xaxis->setMinimum(-1e+9);
        SpinBox_Xaxis->setMaximum(1e+9);
        SpinBox_Xaxis->setValue(0);

        gridLayout1->addWidget(SpinBox_Xaxis, 0, 3, 1, 1);


        gridLayout_5->addWidget(gBCylindre, 2, 0, 1, 2);

        gBSphere = new QGroupBox(CreateBoundaryAn);
        gBSphere->setObjectName(QStringLiteral("gBSphere"));
        sizePolicy.setHeightForWidth(gBSphere->sizePolicy().hasHeightForWidth());
        gBSphere->setSizePolicy(sizePolicy);
        gridLayout2 = new QGridLayout(gBSphere);
#ifndef Q_OS_MAC
        gridLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout2->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout2->setObjectName(QStringLiteral("gridLayout2"));
        SpinBox_Rayon = new QDoubleSpinBox(gBSphere);
        SpinBox_Rayon->setObjectName(QStringLiteral("SpinBox_Rayon"));
        SpinBox_Rayon->setDecimals(5);
        SpinBox_Rayon->setMinimum(0);
        SpinBox_Rayon->setMaximum(1e+9);
        SpinBox_Rayon->setValue(0);

        gridLayout2->addWidget(SpinBox_Rayon, 1, 3, 1, 1);

        SpinBox_Zcentre = new QDoubleSpinBox(gBSphere);
        SpinBox_Zcentre->setObjectName(QStringLiteral("SpinBox_Zcentre"));
        SpinBox_Zcentre->setDecimals(5);
        SpinBox_Zcentre->setMinimum(-1e+9);
        SpinBox_Zcentre->setMaximum(1e+9);
        SpinBox_Zcentre->setValue(0);

        gridLayout2->addWidget(SpinBox_Zcentre, 2, 1, 1, 1);

        TLRayon = new QLabel(gBSphere);
        TLRayon->setObjectName(QStringLiteral("TLRayon"));
        sizePolicy.setHeightForWidth(TLRayon->sizePolicy().hasHeightForWidth());
        TLRayon->setSizePolicy(sizePolicy);
        TLRayon->setWordWrap(false);

        gridLayout2->addWidget(TLRayon, 1, 2, 1, 1);

        TLZcentre = new QLabel(gBSphere);
        TLZcentre->setObjectName(QStringLiteral("TLZcentre"));
        sizePolicy.setHeightForWidth(TLZcentre->sizePolicy().hasHeightForWidth());
        TLZcentre->setSizePolicy(sizePolicy);
        TLZcentre->setWordWrap(false);

        gridLayout2->addWidget(TLZcentre, 2, 0, 1, 1);

        TLYcentre = new QLabel(gBSphere);
        TLYcentre->setObjectName(QStringLiteral("TLYcentre"));
        sizePolicy.setHeightForWidth(TLYcentre->sizePolicy().hasHeightForWidth());
        TLYcentre->setSizePolicy(sizePolicy);
        TLYcentre->setWordWrap(false);

        gridLayout2->addWidget(TLYcentre, 1, 0, 1, 1);

        SpinBox_Ycentre = new QDoubleSpinBox(gBSphere);
        SpinBox_Ycentre->setObjectName(QStringLiteral("SpinBox_Ycentre"));
        SpinBox_Ycentre->setDecimals(5);
        SpinBox_Ycentre->setMinimum(-1e+9);
        SpinBox_Ycentre->setMaximum(1e+9);
        SpinBox_Ycentre->setValue(0);

        gridLayout2->addWidget(SpinBox_Ycentre, 1, 1, 1, 1);

        SpinBox_Xcentre = new QDoubleSpinBox(gBSphere);
        SpinBox_Xcentre->setObjectName(QStringLiteral("SpinBox_Xcentre"));
        SpinBox_Xcentre->setDecimals(5);
        SpinBox_Xcentre->setMinimum(-1e+9);
        SpinBox_Xcentre->setMaximum(1e+9);
        SpinBox_Xcentre->setValue(0);

        gridLayout2->addWidget(SpinBox_Xcentre, 0, 1, 1, 1);

        TLXcentre = new QLabel(gBSphere);
        TLXcentre->setObjectName(QStringLiteral("TLXcentre"));
        sizePolicy.setHeightForWidth(TLXcentre->sizePolicy().hasHeightForWidth());
        TLXcentre->setSizePolicy(sizePolicy);
        TLXcentre->setWordWrap(false);

        gridLayout2->addWidget(TLXcentre, 0, 0, 1, 1);


        gridLayout_5->addWidget(gBSphere, 3, 0, 1, 2);

        gBCone = new QGroupBox(CreateBoundaryAn);
        gBCone->setObjectName(QStringLiteral("gBCone"));
        sizePolicy.setHeightForWidth(gBCone->sizePolicy().hasHeightForWidth());
        gBCone->setSizePolicy(sizePolicy);
        gridLayout_3 = new QGridLayout(gBCone);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        groupBox = new QGroupBox(gBCone);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        RB_Def_radius = new QRadioButton(groupBox);
        RB_Def_radius->setObjectName(QStringLiteral("RB_Def_radius"));
        QIcon icon4;
        icon4.addFile(QStringLiteral("../../resources/conedxyz.png"), QSize(), QIcon::Normal, QIcon::Off);
        RB_Def_radius->setIcon(icon4);
        RB_Def_radius->setChecked(true);

        gridLayout_2->addWidget(RB_Def_radius, 0, 0, 1, 1);

        RB_Def_angle = new QRadioButton(groupBox);
        RB_Def_angle->setObjectName(QStringLiteral("RB_Def_angle"));
        QIcon icon5;
        icon5.addFile(QStringLiteral("../../resources/conepointvector.png"), QSize(), QIcon::Normal, QIcon::Off);
        RB_Def_angle->setIcon(icon5);

        gridLayout_2->addWidget(RB_Def_angle, 0, 1, 1, 1);


        gridLayout_3->addWidget(groupBox, 0, 0, 1, 2);

        TLCone_X1 = new QLabel(gBCone);
        TLCone_X1->setObjectName(QStringLiteral("TLCone_X1"));
        sizePolicy.setHeightForWidth(TLCone_X1->sizePolicy().hasHeightForWidth());
        TLCone_X1->setSizePolicy(sizePolicy);
        TLCone_X1->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_X1, 1, 0, 1, 1);

        SpinBox_Cone_X1 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_X1->setObjectName(QStringLiteral("SpinBox_Cone_X1"));
        SpinBox_Cone_X1->setDecimals(5);
        SpinBox_Cone_X1->setMinimum(-1e+9);
        SpinBox_Cone_X1->setMaximum(1e+9);
        SpinBox_Cone_X1->setValue(0);

        gridLayout_3->addWidget(SpinBox_Cone_X1, 1, 1, 1, 1);

        TLCone_X2 = new QLabel(gBCone);
        TLCone_X2->setObjectName(QStringLiteral("TLCone_X2"));
        sizePolicy.setHeightForWidth(TLCone_X2->sizePolicy().hasHeightForWidth());
        TLCone_X2->setSizePolicy(sizePolicy);
        TLCone_X2->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_X2, 1, 2, 1, 1);

        SpinBox_Cone_X2 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_X2->setObjectName(QStringLiteral("SpinBox_Cone_X2"));
        SpinBox_Cone_X2->setDecimals(5);
        SpinBox_Cone_X2->setMinimum(-1e+9);
        SpinBox_Cone_X2->setMaximum(1e+9);
        SpinBox_Cone_X2->setValue(0);

        gridLayout_3->addWidget(SpinBox_Cone_X2, 1, 3, 1, 1);

        TLCone_Y1 = new QLabel(gBCone);
        TLCone_Y1->setObjectName(QStringLiteral("TLCone_Y1"));
        sizePolicy.setHeightForWidth(TLCone_Y1->sizePolicy().hasHeightForWidth());
        TLCone_Y1->setSizePolicy(sizePolicy);
        TLCone_Y1->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_Y1, 2, 0, 1, 1);

        SpinBox_Cone_Y1 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_Y1->setObjectName(QStringLiteral("SpinBox_Cone_Y1"));
        SpinBox_Cone_Y1->setDecimals(5);
        SpinBox_Cone_Y1->setMinimum(-1e+9);
        SpinBox_Cone_Y1->setMaximum(1e+9);
        SpinBox_Cone_Y1->setValue(0);

        gridLayout_3->addWidget(SpinBox_Cone_Y1, 2, 1, 1, 1);

        TLCone_Y2 = new QLabel(gBCone);
        TLCone_Y2->setObjectName(QStringLiteral("TLCone_Y2"));
        sizePolicy.setHeightForWidth(TLCone_Y2->sizePolicy().hasHeightForWidth());
        TLCone_Y2->setSizePolicy(sizePolicy);
        TLCone_Y2->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_Y2, 2, 2, 1, 1);

        SpinBox_Cone_Y2 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_Y2->setObjectName(QStringLiteral("SpinBox_Cone_Y2"));
        SpinBox_Cone_Y2->setDecimals(5);
        SpinBox_Cone_Y2->setMinimum(-1e+9);
        SpinBox_Cone_Y2->setMaximum(1e+9);
        SpinBox_Cone_Y2->setValue(0);

        gridLayout_3->addWidget(SpinBox_Cone_Y2, 2, 3, 1, 1);

        TLCone_Z1 = new QLabel(gBCone);
        TLCone_Z1->setObjectName(QStringLiteral("TLCone_Z1"));
        sizePolicy.setHeightForWidth(TLCone_Z1->sizePolicy().hasHeightForWidth());
        TLCone_Z1->setSizePolicy(sizePolicy);
        TLCone_Z1->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_Z1, 3, 0, 1, 1);

        SpinBox_Cone_Z1 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_Z1->setObjectName(QStringLiteral("SpinBox_Cone_Z1"));
        SpinBox_Cone_Z1->setDecimals(5);
        SpinBox_Cone_Z1->setMinimum(-1e+9);
        SpinBox_Cone_Z1->setMaximum(1e+9);
        SpinBox_Cone_Z1->setValue(0);

        gridLayout_3->addWidget(SpinBox_Cone_Z1, 3, 1, 1, 1);

        TLCone_Z2 = new QLabel(gBCone);
        TLCone_Z2->setObjectName(QStringLiteral("TLCone_Z2"));
        sizePolicy.setHeightForWidth(TLCone_Z2->sizePolicy().hasHeightForWidth());
        TLCone_Z2->setSizePolicy(sizePolicy);
        TLCone_Z2->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_Z2, 3, 2, 1, 1);

        SpinBox_Cone_Z2 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_Z2->setObjectName(QStringLiteral("SpinBox_Cone_Z2"));
        SpinBox_Cone_Z2->setDecimals(5);
        SpinBox_Cone_Z2->setMinimum(-1e+9);
        SpinBox_Cone_Z2->setMaximum(1e+9);
        SpinBox_Cone_Z2->setValue(0);

        gridLayout_3->addWidget(SpinBox_Cone_Z2, 3, 3, 1, 1);

        TLCone_V1 = new QLabel(gBCone);
        TLCone_V1->setObjectName(QStringLiteral("TLCone_V1"));
        sizePolicy.setHeightForWidth(TLCone_V1->sizePolicy().hasHeightForWidth());
        TLCone_V1->setSizePolicy(sizePolicy);
        TLCone_V1->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_V1, 4, 0, 1, 1);

        SpinBox_Cone_V1 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_V1->setObjectName(QStringLiteral("SpinBox_Cone_V1"));
        SpinBox_Cone_V1->setDecimals(5);
        SpinBox_Cone_V1->setMaximum(1e+9);

        gridLayout_3->addWidget(SpinBox_Cone_V1, 4, 1, 1, 1);

        TLCone_V2 = new QLabel(gBCone);
        TLCone_V2->setObjectName(QStringLiteral("TLCone_V2"));
        sizePolicy.setHeightForWidth(TLCone_V2->sizePolicy().hasHeightForWidth());
        TLCone_V2->setSizePolicy(sizePolicy);
        TLCone_V2->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_V2, 4, 2, 1, 1);

        SpinBox_Cone_V2 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_V2->setObjectName(QStringLiteral("SpinBox_Cone_V2"));
        SpinBox_Cone_V2->setDecimals(5);
        SpinBox_Cone_V2->setMaximum(1e+9);

        gridLayout_3->addWidget(SpinBox_Cone_V2, 4, 3, 1, 1);


        gridLayout_5->addWidget(gBCone, 4, 0, 1, 2);

        gBTore = new QGroupBox(CreateBoundaryAn);
        gBTore->setObjectName(QStringLiteral("gBTore"));
        sizePolicy.setHeightForWidth(gBTore->sizePolicy().hasHeightForWidth());
        gBTore->setSizePolicy(sizePolicy);
        gridLayout_4 = new QGridLayout(gBTore);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        TLToreXcent = new QLabel(gBTore);
        TLToreXcent->setObjectName(QStringLiteral("TLToreXcent"));
        sizePolicy.setHeightForWidth(TLToreXcent->sizePolicy().hasHeightForWidth());
        TLToreXcent->setSizePolicy(sizePolicy);
        TLToreXcent->setWordWrap(false);

        gridLayout_4->addWidget(TLToreXcent, 0, 0, 1, 1);

        SpinBoxToreXcent = new QDoubleSpinBox(gBTore);
        SpinBoxToreXcent->setObjectName(QStringLiteral("SpinBoxToreXcent"));
        SpinBoxToreXcent->setDecimals(5);
        SpinBoxToreXcent->setMinimum(-1e+9);
        SpinBoxToreXcent->setMaximum(1e+9);
        SpinBoxToreXcent->setValue(0);

        gridLayout_4->addWidget(SpinBoxToreXcent, 0, 1, 1, 1);

        TLToreXaxe = new QLabel(gBTore);
        TLToreXaxe->setObjectName(QStringLiteral("TLToreXaxe"));
        sizePolicy.setHeightForWidth(TLToreXaxe->sizePolicy().hasHeightForWidth());
        TLToreXaxe->setSizePolicy(sizePolicy);
        TLToreXaxe->setWordWrap(false);

        gridLayout_4->addWidget(TLToreXaxe, 0, 2, 1, 1);

        SpinBoxToreXaxe = new QDoubleSpinBox(gBTore);
        SpinBoxToreXaxe->setObjectName(QStringLiteral("SpinBoxToreXaxe"));
        SpinBoxToreXaxe->setDecimals(5);
        SpinBoxToreXaxe->setMinimum(-1e+9);
        SpinBoxToreXaxe->setMaximum(1e+9);
        SpinBoxToreXaxe->setValue(0);

        gridLayout_4->addWidget(SpinBoxToreXaxe, 0, 3, 1, 1);

        TLToreYcent = new QLabel(gBTore);
        TLToreYcent->setObjectName(QStringLiteral("TLToreYcent"));
        sizePolicy.setHeightForWidth(TLToreYcent->sizePolicy().hasHeightForWidth());
        TLToreYcent->setSizePolicy(sizePolicy);
        TLToreYcent->setWordWrap(false);

        gridLayout_4->addWidget(TLToreYcent, 1, 0, 1, 1);

        SpinBoxToreYcent = new QDoubleSpinBox(gBTore);
        SpinBoxToreYcent->setObjectName(QStringLiteral("SpinBoxToreYcent"));
        SpinBoxToreYcent->setDecimals(5);
        SpinBoxToreYcent->setMinimum(-1e+9);
        SpinBoxToreYcent->setMaximum(1e+9);
        SpinBoxToreYcent->setValue(0);

        gridLayout_4->addWidget(SpinBoxToreYcent, 1, 1, 1, 1);

        TLToreYaxe = new QLabel(gBTore);
        TLToreYaxe->setObjectName(QStringLiteral("TLToreYaxe"));
        sizePolicy.setHeightForWidth(TLToreYaxe->sizePolicy().hasHeightForWidth());
        TLToreYaxe->setSizePolicy(sizePolicy);
        TLToreYaxe->setWordWrap(false);

        gridLayout_4->addWidget(TLToreYaxe, 1, 2, 1, 1);

        SpinBoxToreYaxe = new QDoubleSpinBox(gBTore);
        SpinBoxToreYaxe->setObjectName(QStringLiteral("SpinBoxToreYaxe"));
        SpinBoxToreYaxe->setDecimals(5);
        SpinBoxToreYaxe->setMinimum(-1e+9);
        SpinBoxToreYaxe->setMaximum(1e+9);
        SpinBoxToreYaxe->setValue(0);

        gridLayout_4->addWidget(SpinBoxToreYaxe, 1, 3, 1, 1);

        TLToreZcent = new QLabel(gBTore);
        TLToreZcent->setObjectName(QStringLiteral("TLToreZcent"));
        sizePolicy.setHeightForWidth(TLToreZcent->sizePolicy().hasHeightForWidth());
        TLToreZcent->setSizePolicy(sizePolicy);
        TLToreZcent->setWordWrap(false);

        gridLayout_4->addWidget(TLToreZcent, 2, 0, 1, 1);

        SpinBoxToreZcent = new QDoubleSpinBox(gBTore);
        SpinBoxToreZcent->setObjectName(QStringLiteral("SpinBoxToreZcent"));
        SpinBoxToreZcent->setDecimals(5);
        SpinBoxToreZcent->setMinimum(-1e+9);
        SpinBoxToreZcent->setMaximum(1e+9);
        SpinBoxToreZcent->setValue(0);

        gridLayout_4->addWidget(SpinBoxToreZcent, 2, 1, 1, 1);

        TLToreZaxe = new QLabel(gBTore);
        TLToreZaxe->setObjectName(QStringLiteral("TLToreZaxe"));
        sizePolicy.setHeightForWidth(TLToreZaxe->sizePolicy().hasHeightForWidth());
        TLToreZaxe->setSizePolicy(sizePolicy);
        TLToreZaxe->setWordWrap(false);

        gridLayout_4->addWidget(TLToreZaxe, 2, 2, 1, 1);

        SpinBoxToreZaxe = new QDoubleSpinBox(gBTore);
        SpinBoxToreZaxe->setObjectName(QStringLiteral("SpinBoxToreZaxe"));
        SpinBoxToreZaxe->setDecimals(5);
        SpinBoxToreZaxe->setMinimum(-1e+9);
        SpinBoxToreZaxe->setMaximum(1e+9);
        SpinBoxToreZaxe->setValue(0);

        gridLayout_4->addWidget(SpinBoxToreZaxe, 2, 3, 1, 1);

        TLToreRayRev = new QLabel(gBTore);
        TLToreRayRev->setObjectName(QStringLiteral("TLToreRayRev"));
        sizePolicy.setHeightForWidth(TLToreRayRev->sizePolicy().hasHeightForWidth());
        TLToreRayRev->setSizePolicy(sizePolicy);
        TLToreRayRev->setWordWrap(false);

        gridLayout_4->addWidget(TLToreRayRev, 3, 0, 1, 1);

        SpinBoxToreRRev = new QDoubleSpinBox(gBTore);
        SpinBoxToreRRev->setObjectName(QStringLiteral("SpinBoxToreRRev"));
        SpinBoxToreRRev->setDecimals(5);
        SpinBoxToreRRev->setMaximum(1e+9);

        gridLayout_4->addWidget(SpinBoxToreRRev, 3, 1, 1, 1);

        TLToreRayPri = new QLabel(gBTore);
        TLToreRayPri->setObjectName(QStringLiteral("TLToreRayPri"));
        sizePolicy.setHeightForWidth(TLToreRayPri->sizePolicy().hasHeightForWidth());
        TLToreRayPri->setSizePolicy(sizePolicy);
        TLToreRayPri->setWordWrap(false);

        gridLayout_4->addWidget(TLToreRayPri, 3, 2, 1, 1);

        SpinBoxToreRPri = new QDoubleSpinBox(gBTore);
        SpinBoxToreRPri->setObjectName(QStringLiteral("SpinBoxToreRPri"));
        SpinBoxToreRPri->setDecimals(5);
        SpinBoxToreRPri->setMaximum(1e+9);

        gridLayout_4->addWidget(SpinBoxToreRPri, 3, 3, 1, 1);


        gridLayout_5->addWidget(gBTore, 5, 0, 1, 2);

        GBButtons = new QGroupBox(CreateBoundaryAn);
        GBButtons->setObjectName(QStringLiteral("GBButtons"));
        sizePolicy.setHeightForWidth(GBButtons->sizePolicy().hasHeightForWidth());
        GBButtons->setSizePolicy(sizePolicy);
        gridLayout3 = new QGridLayout(GBButtons);
#ifndef Q_OS_MAC
        gridLayout3->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout3->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout3->setObjectName(QStringLiteral("gridLayout3"));
        buttonHelp = new QPushButton(GBButtons);
        buttonHelp->setObjectName(QStringLiteral("buttonHelp"));

        gridLayout3->addWidget(buttonHelp, 0, 3, 1, 1);

        buttonCancel = new QPushButton(GBButtons);
        buttonCancel->setObjectName(QStringLiteral("buttonCancel"));

        gridLayout3->addWidget(buttonCancel, 0, 2, 1, 1);

        buttonApply = new QPushButton(GBButtons);
        buttonApply->setObjectName(QStringLiteral("buttonApply"));

        gridLayout3->addWidget(buttonApply, 0, 1, 1, 1);

        buttonOk = new QPushButton(GBButtons);
        buttonOk->setObjectName(QStringLiteral("buttonOk"));

        gridLayout3->addWidget(buttonOk, 0, 0, 1, 1);


        gridLayout_5->addWidget(GBButtons, 6, 0, 1, 2);


        retranslateUi(CreateBoundaryAn);

        QMetaObject::connectSlotsByName(CreateBoundaryAn);
    } // setupUi

    void retranslateUi(QDialog *CreateBoundaryAn)
    {
        CreateBoundaryAn->setWindowTitle(QApplication::translate("CreateBoundaryAn", "Create an analytical boundary", Q_NULLPTR));
        Name->setText(QApplication::translate("CreateBoundaryAn", "Name", Q_NULLPTR));
        TypeBoundary->setTitle(QApplication::translate("CreateBoundaryAn", "Type of boundary", Q_NULLPTR));
        RBCylindre->setText(QApplication::translate("CreateBoundaryAn", "Cylinder", Q_NULLPTR));
        RBSphere->setText(QApplication::translate("CreateBoundaryAn", "Sphere", Q_NULLPTR));
        RBCone->setText(QApplication::translate("CreateBoundaryAn", "Cone", Q_NULLPTR));
        RBTore->setText(QApplication::translate("CreateBoundaryAn", "Torus", Q_NULLPTR));
        gBCylindre->setTitle(QApplication::translate("CreateBoundaryAn", "Coordinates", Q_NULLPTR));
        TLXcent->setText(QApplication::translate("CreateBoundaryAn", "X centre", Q_NULLPTR));
        TLradius->setText(QApplication::translate("CreateBoundaryAn", "Radius", Q_NULLPTR));
        TLZcent->setText(QApplication::translate("CreateBoundaryAn", "Z centre", Q_NULLPTR));
        TLYcent->setText(QApplication::translate("CreateBoundaryAn", "Y centre", Q_NULLPTR));
        TLXaxis->setText(QApplication::translate("CreateBoundaryAn", "X axis", Q_NULLPTR));
        TLYaxis->setText(QApplication::translate("CreateBoundaryAn", "Y axis", Q_NULLPTR));
        TLZaxis->setText(QApplication::translate("CreateBoundaryAn", "Z axis", Q_NULLPTR));
        gBSphere->setTitle(QApplication::translate("CreateBoundaryAn", "Coordinates", Q_NULLPTR));
        TLRayon->setText(QApplication::translate("CreateBoundaryAn", "Radius", Q_NULLPTR));
        TLZcentre->setText(QApplication::translate("CreateBoundaryAn", "Z centre", Q_NULLPTR));
        TLYcentre->setText(QApplication::translate("CreateBoundaryAn", "Y centre", Q_NULLPTR));
        TLXcentre->setText(QApplication::translate("CreateBoundaryAn", "X centre", Q_NULLPTR));
        gBCone->setTitle(QApplication::translate("CreateBoundaryAn", "Coordinates", Q_NULLPTR));
        groupBox->setTitle(QApplication::translate("CreateBoundaryAn", "Definition", Q_NULLPTR));
        RB_Def_radius->setText(QApplication::translate("CreateBoundaryAn", "Radius", Q_NULLPTR));
        RB_Def_angle->setText(QApplication::translate("CreateBoundaryAn", "Angle", Q_NULLPTR));
        TLCone_X1->setText(QApplication::translate("CreateBoundaryAn", "X 1", Q_NULLPTR));
        TLCone_X2->setText(QApplication::translate("CreateBoundaryAn", "X 2", Q_NULLPTR));
        TLCone_Y1->setText(QApplication::translate("CreateBoundaryAn", "Y 1", Q_NULLPTR));
        TLCone_Y2->setText(QApplication::translate("CreateBoundaryAn", "Y 2", Q_NULLPTR));
        TLCone_Z1->setText(QApplication::translate("CreateBoundaryAn", "Z 1", Q_NULLPTR));
        TLCone_Z2->setText(QApplication::translate("CreateBoundaryAn", "Z 2", Q_NULLPTR));
        TLCone_V1->setText(QApplication::translate("CreateBoundaryAn", "V 1", Q_NULLPTR));
        TLCone_V2->setText(QApplication::translate("CreateBoundaryAn", "V 2", Q_NULLPTR));
        gBTore->setTitle(QApplication::translate("CreateBoundaryAn", "Coordinates", Q_NULLPTR));
        TLToreXcent->setText(QApplication::translate("CreateBoundaryAn", "X centre", Q_NULLPTR));
        TLToreXaxe->setText(QApplication::translate("CreateBoundaryAn", "X axis", Q_NULLPTR));
        TLToreYcent->setText(QApplication::translate("CreateBoundaryAn", "Y centre", Q_NULLPTR));
        TLToreYaxe->setText(QApplication::translate("CreateBoundaryAn", "Y axis", Q_NULLPTR));
        TLToreZcent->setText(QApplication::translate("CreateBoundaryAn", "Z centre", Q_NULLPTR));
        TLToreZaxe->setText(QApplication::translate("CreateBoundaryAn", "Z axis", Q_NULLPTR));
        TLToreRayRev->setText(QApplication::translate("CreateBoundaryAn", "R revolution", Q_NULLPTR));
        TLToreRayPri->setText(QApplication::translate("CreateBoundaryAn", "Primary R", Q_NULLPTR));
        GBButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("CreateBoundaryAn", "Help", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("CreateBoundaryAn", "Cancel", Q_NULLPTR));
        buttonApply->setText(QApplication::translate("CreateBoundaryAn", "Apply", Q_NULLPTR));
        buttonOk->setText(QApplication::translate("CreateBoundaryAn", "OK", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CreateBoundaryAn: public Ui_CreateBoundaryAn {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATEBOUNDARYAN_H
