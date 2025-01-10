// Copyright (C) 2011-2025  CEA, EDF
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

#ifndef MON_CREATEBOUNDARY_H
#define MON_CREATEBOUNDARY_H

#include "SMESH_SMESHGUI.hxx"

#include <SALOMEconfig.h>
#include <SalomeApp_Module.h>

#include CORBA_CLIENT_HEADER(SMESH_Homard)

#include <QDialog>

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>

class SMESHGUI_HomardAdaptDlg;

class SMESH_Ui_CreateBoundaryAn
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
            CreateBoundaryAn->setObjectName(QString::fromUtf8("CreateBoundaryAn"));
        CreateBoundaryAn->resize(522, 835);
        CreateBoundaryAn->setAutoFillBackground(true);
        CreateBoundaryAn->setSizeGripEnabled(true);
        gridLayout_5 = new QGridLayout(CreateBoundaryAn);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        Name = new QLabel(CreateBoundaryAn);
        Name->setObjectName(QString::fromUtf8("Name"));

        gridLayout_5->addWidget(Name, 0, 0, 1, 1);

        LEName = new QLineEdit(CreateBoundaryAn);
        LEName->setObjectName(QString::fromUtf8("LEName"));
        LEName->setMaxLength(32);

        gridLayout_5->addWidget(LEName, 0, 1, 1, 1);

        TypeBoundary = new QGroupBox(CreateBoundaryAn);
        TypeBoundary->setObjectName(QString::fromUtf8("TypeBoundary"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(TypeBoundary->sizePolicy().hasHeightForWidth());
        TypeBoundary->setSizePolicy(sizePolicy);
        TypeBoundary->setMinimumSize(QSize(340, 0));
        gridLayout = new QGridLayout(TypeBoundary);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        RBCylindre = new QRadioButton(TypeBoundary);
        RBCylindre->setObjectName(QString::fromUtf8("RBCylindre"));
        QIcon icon;
        icon.addFile(QString::fromUtf8("../../resources/cylinderpointvector.png"), QSize(), QIcon::Normal, QIcon::Off);
        RBCylindre->setIcon(icon);
        RBCylindre->setCheckable(true);
        RBCylindre->setChecked(true);

        gridLayout->addWidget(RBCylindre, 0, 0, 1, 1);

        RBSphere = new QRadioButton(TypeBoundary);
        RBSphere->setObjectName(QString::fromUtf8("RBSphere"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8("../../resources/zone_spherepoint.png"), QSize(), QIcon::Normal, QIcon::Off);
        RBSphere->setIcon(icon1);

        gridLayout->addWidget(RBSphere, 0, 1, 1, 1);

        RBCone = new QRadioButton(TypeBoundary);
        RBCone->setObjectName(QString::fromUtf8("RBCone"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8("../../resources/cone.png"), QSize(), QIcon::Normal, QIcon::Off);
        RBCone->setIcon(icon2);

        gridLayout->addWidget(RBCone, 0, 2, 1, 1);

        RBTore = new QRadioButton(TypeBoundary);
        RBTore->setObjectName(QString::fromUtf8("RBTore"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8("../../resources/toruspointvector.png"), QSize(), QIcon::Normal, QIcon::Off);
        RBTore->setIcon(icon3);
        RBTore->setCheckable(true);
        RBTore->setChecked(false);

        gridLayout->addWidget(RBTore, 0, 3, 1, 1);


        gridLayout_5->addWidget(TypeBoundary, 1, 0, 1, 2);

        gBCylindre = new QGroupBox(CreateBoundaryAn);
        gBCylindre->setObjectName(QString::fromUtf8("gBCylindre"));
        sizePolicy.setHeightForWidth(gBCylindre->sizePolicy().hasHeightForWidth());
        gBCylindre->setSizePolicy(sizePolicy);
        gridLayout1 = new QGridLayout(gBCylindre);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        SpinBox_Xcent = new QDoubleSpinBox(gBCylindre);
        SpinBox_Xcent->setObjectName(QString::fromUtf8("SpinBox_Xcent"));
        SpinBox_Xcent->setDecimals(5);
        SpinBox_Xcent->setMinimum(-999999999.000000000000000);
        SpinBox_Xcent->setMaximum(999999999.000000000000000);
        SpinBox_Xcent->setValue(0.000000000000000);

        gridLayout1->addWidget(SpinBox_Xcent, 0, 1, 1, 1);

        TLXcent = new QLabel(gBCylindre);
        TLXcent->setObjectName(QString::fromUtf8("TLXcent"));
        sizePolicy.setHeightForWidth(TLXcent->sizePolicy().hasHeightForWidth());
        TLXcent->setSizePolicy(sizePolicy);
        TLXcent->setWordWrap(false);

        gridLayout1->addWidget(TLXcent, 0, 0, 1, 1);

        SpinBox_Radius = new QDoubleSpinBox(gBCylindre);
        SpinBox_Radius->setObjectName(QString::fromUtf8("SpinBox_Radius"));
        SpinBox_Radius->setDecimals(5);
        SpinBox_Radius->setMaximum(1000000000.000000000000000);

        gridLayout1->addWidget(SpinBox_Radius, 3, 1, 1, 2);

        SpinBox_Zcent = new QDoubleSpinBox(gBCylindre);
        SpinBox_Zcent->setObjectName(QString::fromUtf8("SpinBox_Zcent"));
        SpinBox_Zcent->setDecimals(5);
        SpinBox_Zcent->setMinimum(-999999999.000000000000000);
        SpinBox_Zcent->setMaximum(999999999.000000000000000);
        SpinBox_Zcent->setValue(0.000000000000000);

        gridLayout1->addWidget(SpinBox_Zcent, 2, 1, 1, 1);

        TLradius = new QLabel(gBCylindre);
        TLradius->setObjectName(QString::fromUtf8("TLradius"));
        sizePolicy.setHeightForWidth(TLradius->sizePolicy().hasHeightForWidth());
        TLradius->setSizePolicy(sizePolicy);
        TLradius->setWordWrap(false);

        gridLayout1->addWidget(TLradius, 3, 0, 1, 1);

        TLZcent = new QLabel(gBCylindre);
        TLZcent->setObjectName(QString::fromUtf8("TLZcent"));
        sizePolicy.setHeightForWidth(TLZcent->sizePolicy().hasHeightForWidth());
        TLZcent->setSizePolicy(sizePolicy);
        TLZcent->setWordWrap(false);

        gridLayout1->addWidget(TLZcent, 2, 0, 1, 1);

        TLYcent = new QLabel(gBCylindre);
        TLYcent->setObjectName(QString::fromUtf8("TLYcent"));
        sizePolicy.setHeightForWidth(TLYcent->sizePolicy().hasHeightForWidth());
        TLYcent->setSizePolicy(sizePolicy);
        TLYcent->setWordWrap(false);

        gridLayout1->addWidget(TLYcent, 1, 0, 1, 1);

        SpinBox_Ycent = new QDoubleSpinBox(gBCylindre);
        SpinBox_Ycent->setObjectName(QString::fromUtf8("SpinBox_Ycent"));
        SpinBox_Ycent->setDecimals(5);
        SpinBox_Ycent->setMinimum(-999999999.000000000000000);
        SpinBox_Ycent->setMaximum(999999999.000000000000000);
        SpinBox_Ycent->setValue(0.000000000000000);

        gridLayout1->addWidget(SpinBox_Ycent, 1, 1, 1, 1);

        TLXaxis = new QLabel(gBCylindre);
        TLXaxis->setObjectName(QString::fromUtf8("TLXaxis"));
        sizePolicy.setHeightForWidth(TLXaxis->sizePolicy().hasHeightForWidth());
        TLXaxis->setSizePolicy(sizePolicy);
        TLXaxis->setWordWrap(false);

        gridLayout1->addWidget(TLXaxis, 0, 2, 1, 1);

        TLYaxis = new QLabel(gBCylindre);
        TLYaxis->setObjectName(QString::fromUtf8("TLYaxis"));
        sizePolicy.setHeightForWidth(TLYaxis->sizePolicy().hasHeightForWidth());
        TLYaxis->setSizePolicy(sizePolicy);
        TLYaxis->setWordWrap(false);

        gridLayout1->addWidget(TLYaxis, 1, 2, 1, 1);

        TLZaxis = new QLabel(gBCylindre);
        TLZaxis->setObjectName(QString::fromUtf8("TLZaxis"));
        sizePolicy.setHeightForWidth(TLZaxis->sizePolicy().hasHeightForWidth());
        TLZaxis->setSizePolicy(sizePolicy);
        TLZaxis->setWordWrap(false);

        gridLayout1->addWidget(TLZaxis, 2, 2, 1, 1);

        SpinBox_Zaxis = new QDoubleSpinBox(gBCylindre);
        SpinBox_Zaxis->setObjectName(QString::fromUtf8("SpinBox_Zaxis"));
        SpinBox_Zaxis->setDecimals(5);
        SpinBox_Zaxis->setMinimum(-999999999.000000000000000);
        SpinBox_Zaxis->setMaximum(999999999.000000000000000);
        SpinBox_Zaxis->setValue(0.000000000000000);

        gridLayout1->addWidget(SpinBox_Zaxis, 2, 3, 1, 1);

        SpinBox_Yaxis = new QDoubleSpinBox(gBCylindre);
        SpinBox_Yaxis->setObjectName(QString::fromUtf8("SpinBox_Yaxis"));
        SpinBox_Yaxis->setDecimals(5);
        SpinBox_Yaxis->setMinimum(-999999999.000000000000000);
        SpinBox_Yaxis->setMaximum(999999999.000000000000000);
        SpinBox_Yaxis->setValue(0.000000000000000);

        gridLayout1->addWidget(SpinBox_Yaxis, 1, 3, 1, 1);

        SpinBox_Xaxis = new QDoubleSpinBox(gBCylindre);
        SpinBox_Xaxis->setObjectName(QString::fromUtf8("SpinBox_Xaxis"));
        SpinBox_Xaxis->setDecimals(5);
        SpinBox_Xaxis->setMinimum(-999999999.000000000000000);
        SpinBox_Xaxis->setMaximum(999999999.000000000000000);
        SpinBox_Xaxis->setValue(0.000000000000000);

        gridLayout1->addWidget(SpinBox_Xaxis, 0, 3, 1, 1);


        gridLayout_5->addWidget(gBCylindre, 2, 0, 1, 2);

        gBSphere = new QGroupBox(CreateBoundaryAn);
        gBSphere->setObjectName(QString::fromUtf8("gBSphere"));
        sizePolicy.setHeightForWidth(gBSphere->sizePolicy().hasHeightForWidth());
        gBSphere->setSizePolicy(sizePolicy);
        gridLayout2 = new QGridLayout(gBSphere);
#ifndef Q_OS_MAC
        gridLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout2->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
        SpinBox_Rayon = new QDoubleSpinBox(gBSphere);
        SpinBox_Rayon->setObjectName(QString::fromUtf8("SpinBox_Rayon"));
        SpinBox_Rayon->setDecimals(5);
        SpinBox_Rayon->setMinimum(0.000000000000000);
        SpinBox_Rayon->setMaximum(999999999.000000000000000);
        SpinBox_Rayon->setValue(0.000000000000000);

        gridLayout2->addWidget(SpinBox_Rayon, 1, 3, 1, 1);

        SpinBox_Zcentre = new QDoubleSpinBox(gBSphere);
        SpinBox_Zcentre->setObjectName(QString::fromUtf8("SpinBox_Zcentre"));
        SpinBox_Zcentre->setDecimals(5);
        SpinBox_Zcentre->setMinimum(-999999999.000000000000000);
        SpinBox_Zcentre->setMaximum(999999999.000000000000000);
        SpinBox_Zcentre->setValue(0.000000000000000);

        gridLayout2->addWidget(SpinBox_Zcentre, 2, 1, 1, 1);

        TLRayon = new QLabel(gBSphere);
        TLRayon->setObjectName(QString::fromUtf8("TLRayon"));
        sizePolicy.setHeightForWidth(TLRayon->sizePolicy().hasHeightForWidth());
        TLRayon->setSizePolicy(sizePolicy);
        TLRayon->setWordWrap(false);

        gridLayout2->addWidget(TLRayon, 1, 2, 1, 1);

        TLZcentre = new QLabel(gBSphere);
        TLZcentre->setObjectName(QString::fromUtf8("TLZcentre"));
        sizePolicy.setHeightForWidth(TLZcentre->sizePolicy().hasHeightForWidth());
        TLZcentre->setSizePolicy(sizePolicy);
        TLZcentre->setWordWrap(false);

        gridLayout2->addWidget(TLZcentre, 2, 0, 1, 1);

        TLYcentre = new QLabel(gBSphere);
        TLYcentre->setObjectName(QString::fromUtf8("TLYcentre"));
        sizePolicy.setHeightForWidth(TLYcentre->sizePolicy().hasHeightForWidth());
        TLYcentre->setSizePolicy(sizePolicy);
        TLYcentre->setWordWrap(false);

        gridLayout2->addWidget(TLYcentre, 1, 0, 1, 1);

        SpinBox_Ycentre = new QDoubleSpinBox(gBSphere);
        SpinBox_Ycentre->setObjectName(QString::fromUtf8("SpinBox_Ycentre"));
        SpinBox_Ycentre->setDecimals(5);
        SpinBox_Ycentre->setMinimum(-999999999.000000000000000);
        SpinBox_Ycentre->setMaximum(999999999.000000000000000);
        SpinBox_Ycentre->setValue(0.000000000000000);

        gridLayout2->addWidget(SpinBox_Ycentre, 1, 1, 1, 1);

        SpinBox_Xcentre = new QDoubleSpinBox(gBSphere);
        SpinBox_Xcentre->setObjectName(QString::fromUtf8("SpinBox_Xcentre"));
        SpinBox_Xcentre->setDecimals(5);
        SpinBox_Xcentre->setMinimum(-999999999.000000000000000);
        SpinBox_Xcentre->setMaximum(999999999.000000000000000);
        SpinBox_Xcentre->setValue(0.000000000000000);

        gridLayout2->addWidget(SpinBox_Xcentre, 0, 1, 1, 1);

        TLXcentre = new QLabel(gBSphere);
        TLXcentre->setObjectName(QString::fromUtf8("TLXcentre"));
        sizePolicy.setHeightForWidth(TLXcentre->sizePolicy().hasHeightForWidth());
        TLXcentre->setSizePolicy(sizePolicy);
        TLXcentre->setWordWrap(false);

        gridLayout2->addWidget(TLXcentre, 0, 0, 1, 1);


        gridLayout_5->addWidget(gBSphere, 3, 0, 1, 2);

        gBCone = new QGroupBox(CreateBoundaryAn);
        gBCone->setObjectName(QString::fromUtf8("gBCone"));
        sizePolicy.setHeightForWidth(gBCone->sizePolicy().hasHeightForWidth());
        gBCone->setSizePolicy(sizePolicy);
        gridLayout_3 = new QGridLayout(gBCone);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        groupBox = new QGroupBox(gBCone);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        RB_Def_radius = new QRadioButton(groupBox);
        RB_Def_radius->setObjectName(QString::fromUtf8("RB_Def_radius"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8("../../resources/conedxyz.png"), QSize(), QIcon::Normal, QIcon::Off);
        RB_Def_radius->setIcon(icon4);
        RB_Def_radius->setChecked(true);

        gridLayout_2->addWidget(RB_Def_radius, 0, 0, 1, 1);

        RB_Def_angle = new QRadioButton(groupBox);
        RB_Def_angle->setObjectName(QString::fromUtf8("RB_Def_angle"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8("../../resources/conepointvector.png"), QSize(), QIcon::Normal, QIcon::Off);
        RB_Def_angle->setIcon(icon5);

        gridLayout_2->addWidget(RB_Def_angle, 0, 1, 1, 1);


        gridLayout_3->addWidget(groupBox, 0, 0, 1, 2);

        TLCone_X1 = new QLabel(gBCone);
        TLCone_X1->setObjectName(QString::fromUtf8("TLCone_X1"));
        sizePolicy.setHeightForWidth(TLCone_X1->sizePolicy().hasHeightForWidth());
        TLCone_X1->setSizePolicy(sizePolicy);
        TLCone_X1->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_X1, 1, 0, 1, 1);

        SpinBox_Cone_X1 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_X1->setObjectName(QString::fromUtf8("SpinBox_Cone_X1"));
        SpinBox_Cone_X1->setDecimals(5);
        SpinBox_Cone_X1->setMinimum(-999999999.000000000000000);
        SpinBox_Cone_X1->setMaximum(999999999.000000000000000);
        SpinBox_Cone_X1->setValue(0.000000000000000);

        gridLayout_3->addWidget(SpinBox_Cone_X1, 1, 1, 1, 1);

        TLCone_X2 = new QLabel(gBCone);
        TLCone_X2->setObjectName(QString::fromUtf8("TLCone_X2"));
        sizePolicy.setHeightForWidth(TLCone_X2->sizePolicy().hasHeightForWidth());
        TLCone_X2->setSizePolicy(sizePolicy);
        TLCone_X2->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_X2, 1, 2, 1, 1);

        SpinBox_Cone_X2 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_X2->setObjectName(QString::fromUtf8("SpinBox_Cone_X2"));
        SpinBox_Cone_X2->setDecimals(5);
        SpinBox_Cone_X2->setMinimum(-999999999.000000000000000);
        SpinBox_Cone_X2->setMaximum(999999999.000000000000000);
        SpinBox_Cone_X2->setValue(0.000000000000000);

        gridLayout_3->addWidget(SpinBox_Cone_X2, 1, 3, 1, 1);

        TLCone_Y1 = new QLabel(gBCone);
        TLCone_Y1->setObjectName(QString::fromUtf8("TLCone_Y1"));
        sizePolicy.setHeightForWidth(TLCone_Y1->sizePolicy().hasHeightForWidth());
        TLCone_Y1->setSizePolicy(sizePolicy);
        TLCone_Y1->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_Y1, 2, 0, 1, 1);

        SpinBox_Cone_Y1 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_Y1->setObjectName(QString::fromUtf8("SpinBox_Cone_Y1"));
        SpinBox_Cone_Y1->setDecimals(5);
        SpinBox_Cone_Y1->setMinimum(-999999999.000000000000000);
        SpinBox_Cone_Y1->setMaximum(999999999.000000000000000);
        SpinBox_Cone_Y1->setValue(0.000000000000000);

        gridLayout_3->addWidget(SpinBox_Cone_Y1, 2, 1, 1, 1);

        TLCone_Y2 = new QLabel(gBCone);
        TLCone_Y2->setObjectName(QString::fromUtf8("TLCone_Y2"));
        sizePolicy.setHeightForWidth(TLCone_Y2->sizePolicy().hasHeightForWidth());
        TLCone_Y2->setSizePolicy(sizePolicy);
        TLCone_Y2->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_Y2, 2, 2, 1, 1);

        SpinBox_Cone_Y2 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_Y2->setObjectName(QString::fromUtf8("SpinBox_Cone_Y2"));
        SpinBox_Cone_Y2->setDecimals(5);
        SpinBox_Cone_Y2->setMinimum(-999999999.000000000000000);
        SpinBox_Cone_Y2->setMaximum(999999999.000000000000000);
        SpinBox_Cone_Y2->setValue(0.000000000000000);

        gridLayout_3->addWidget(SpinBox_Cone_Y2, 2, 3, 1, 1);

        TLCone_Z1 = new QLabel(gBCone);
        TLCone_Z1->setObjectName(QString::fromUtf8("TLCone_Z1"));
        sizePolicy.setHeightForWidth(TLCone_Z1->sizePolicy().hasHeightForWidth());
        TLCone_Z1->setSizePolicy(sizePolicy);
        TLCone_Z1->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_Z1, 3, 0, 1, 1);

        SpinBox_Cone_Z1 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_Z1->setObjectName(QString::fromUtf8("SpinBox_Cone_Z1"));
        SpinBox_Cone_Z1->setDecimals(5);
        SpinBox_Cone_Z1->setMinimum(-999999999.000000000000000);
        SpinBox_Cone_Z1->setMaximum(999999999.000000000000000);
        SpinBox_Cone_Z1->setValue(0.000000000000000);

        gridLayout_3->addWidget(SpinBox_Cone_Z1, 3, 1, 1, 1);

        TLCone_Z2 = new QLabel(gBCone);
        TLCone_Z2->setObjectName(QString::fromUtf8("TLCone_Z2"));
        sizePolicy.setHeightForWidth(TLCone_Z2->sizePolicy().hasHeightForWidth());
        TLCone_Z2->setSizePolicy(sizePolicy);
        TLCone_Z2->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_Z2, 3, 2, 1, 1);

        SpinBox_Cone_Z2 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_Z2->setObjectName(QString::fromUtf8("SpinBox_Cone_Z2"));
        SpinBox_Cone_Z2->setDecimals(5);
        SpinBox_Cone_Z2->setMinimum(-999999999.000000000000000);
        SpinBox_Cone_Z2->setMaximum(999999999.000000000000000);
        SpinBox_Cone_Z2->setValue(0.000000000000000);

        gridLayout_3->addWidget(SpinBox_Cone_Z2, 3, 3, 1, 1);

        TLCone_V1 = new QLabel(gBCone);
        TLCone_V1->setObjectName(QString::fromUtf8("TLCone_V1"));
        sizePolicy.setHeightForWidth(TLCone_V1->sizePolicy().hasHeightForWidth());
        TLCone_V1->setSizePolicy(sizePolicy);
        TLCone_V1->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_V1, 4, 0, 1, 1);

        SpinBox_Cone_V1 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_V1->setObjectName(QString::fromUtf8("SpinBox_Cone_V1"));
        SpinBox_Cone_V1->setDecimals(5);
        SpinBox_Cone_V1->setMaximum(1000000000.000000000000000);

        gridLayout_3->addWidget(SpinBox_Cone_V1, 4, 1, 1, 1);

        TLCone_V2 = new QLabel(gBCone);
        TLCone_V2->setObjectName(QString::fromUtf8("TLCone_V2"));
        sizePolicy.setHeightForWidth(TLCone_V2->sizePolicy().hasHeightForWidth());
        TLCone_V2->setSizePolicy(sizePolicy);
        TLCone_V2->setWordWrap(false);

        gridLayout_3->addWidget(TLCone_V2, 4, 2, 1, 1);

        SpinBox_Cone_V2 = new QDoubleSpinBox(gBCone);
        SpinBox_Cone_V2->setObjectName(QString::fromUtf8("SpinBox_Cone_V2"));
        SpinBox_Cone_V2->setDecimals(5);
        SpinBox_Cone_V2->setMaximum(1000000000.000000000000000);

        gridLayout_3->addWidget(SpinBox_Cone_V2, 4, 3, 1, 1);


        gridLayout_5->addWidget(gBCone, 4, 0, 1, 2);

        gBTore = new QGroupBox(CreateBoundaryAn);
        gBTore->setObjectName(QString::fromUtf8("gBTore"));
        sizePolicy.setHeightForWidth(gBTore->sizePolicy().hasHeightForWidth());
        gBTore->setSizePolicy(sizePolicy);
        gridLayout_4 = new QGridLayout(gBTore);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        TLToreXcent = new QLabel(gBTore);
        TLToreXcent->setObjectName(QString::fromUtf8("TLToreXcent"));
        sizePolicy.setHeightForWidth(TLToreXcent->sizePolicy().hasHeightForWidth());
        TLToreXcent->setSizePolicy(sizePolicy);
        TLToreXcent->setWordWrap(false);

        gridLayout_4->addWidget(TLToreXcent, 0, 0, 1, 1);

        SpinBoxToreXcent = new QDoubleSpinBox(gBTore);
        SpinBoxToreXcent->setObjectName(QString::fromUtf8("SpinBoxToreXcent"));
        SpinBoxToreXcent->setDecimals(5);
        SpinBoxToreXcent->setMinimum(-999999999.000000000000000);
        SpinBoxToreXcent->setMaximum(999999999.000000000000000);
        SpinBoxToreXcent->setValue(0.000000000000000);

        gridLayout_4->addWidget(SpinBoxToreXcent, 0, 1, 1, 1);

        TLToreXaxe = new QLabel(gBTore);
        TLToreXaxe->setObjectName(QString::fromUtf8("TLToreXaxe"));
        sizePolicy.setHeightForWidth(TLToreXaxe->sizePolicy().hasHeightForWidth());
        TLToreXaxe->setSizePolicy(sizePolicy);
        TLToreXaxe->setWordWrap(false);

        gridLayout_4->addWidget(TLToreXaxe, 0, 2, 1, 1);

        SpinBoxToreXaxe = new QDoubleSpinBox(gBTore);
        SpinBoxToreXaxe->setObjectName(QString::fromUtf8("SpinBoxToreXaxe"));
        SpinBoxToreXaxe->setDecimals(5);
        SpinBoxToreXaxe->setMinimum(-999999999.000000000000000);
        SpinBoxToreXaxe->setMaximum(999999999.000000000000000);
        SpinBoxToreXaxe->setValue(0.000000000000000);

        gridLayout_4->addWidget(SpinBoxToreXaxe, 0, 3, 1, 1);

        TLToreYcent = new QLabel(gBTore);
        TLToreYcent->setObjectName(QString::fromUtf8("TLToreYcent"));
        sizePolicy.setHeightForWidth(TLToreYcent->sizePolicy().hasHeightForWidth());
        TLToreYcent->setSizePolicy(sizePolicy);
        TLToreYcent->setWordWrap(false);

        gridLayout_4->addWidget(TLToreYcent, 1, 0, 1, 1);

        SpinBoxToreYcent = new QDoubleSpinBox(gBTore);
        SpinBoxToreYcent->setObjectName(QString::fromUtf8("SpinBoxToreYcent"));
        SpinBoxToreYcent->setDecimals(5);
        SpinBoxToreYcent->setMinimum(-999999999.000000000000000);
        SpinBoxToreYcent->setMaximum(999999999.000000000000000);
        SpinBoxToreYcent->setValue(0.000000000000000);

        gridLayout_4->addWidget(SpinBoxToreYcent, 1, 1, 1, 1);

        TLToreYaxe = new QLabel(gBTore);
        TLToreYaxe->setObjectName(QString::fromUtf8("TLToreYaxe"));
        sizePolicy.setHeightForWidth(TLToreYaxe->sizePolicy().hasHeightForWidth());
        TLToreYaxe->setSizePolicy(sizePolicy);
        TLToreYaxe->setWordWrap(false);

        gridLayout_4->addWidget(TLToreYaxe, 1, 2, 1, 1);

        SpinBoxToreYaxe = new QDoubleSpinBox(gBTore);
        SpinBoxToreYaxe->setObjectName(QString::fromUtf8("SpinBoxToreYaxe"));
        SpinBoxToreYaxe->setDecimals(5);
        SpinBoxToreYaxe->setMinimum(-999999999.000000000000000);
        SpinBoxToreYaxe->setMaximum(999999999.000000000000000);
        SpinBoxToreYaxe->setValue(0.000000000000000);

        gridLayout_4->addWidget(SpinBoxToreYaxe, 1, 3, 1, 1);

        TLToreZcent = new QLabel(gBTore);
        TLToreZcent->setObjectName(QString::fromUtf8("TLToreZcent"));
        sizePolicy.setHeightForWidth(TLToreZcent->sizePolicy().hasHeightForWidth());
        TLToreZcent->setSizePolicy(sizePolicy);
        TLToreZcent->setWordWrap(false);

        gridLayout_4->addWidget(TLToreZcent, 2, 0, 1, 1);

        SpinBoxToreZcent = new QDoubleSpinBox(gBTore);
        SpinBoxToreZcent->setObjectName(QString::fromUtf8("SpinBoxToreZcent"));
        SpinBoxToreZcent->setDecimals(5);
        SpinBoxToreZcent->setMinimum(-999999999.000000000000000);
        SpinBoxToreZcent->setMaximum(999999999.000000000000000);
        SpinBoxToreZcent->setValue(0.000000000000000);

        gridLayout_4->addWidget(SpinBoxToreZcent, 2, 1, 1, 1);

        TLToreZaxe = new QLabel(gBTore);
        TLToreZaxe->setObjectName(QString::fromUtf8("TLToreZaxe"));
        sizePolicy.setHeightForWidth(TLToreZaxe->sizePolicy().hasHeightForWidth());
        TLToreZaxe->setSizePolicy(sizePolicy);
        TLToreZaxe->setWordWrap(false);

        gridLayout_4->addWidget(TLToreZaxe, 2, 2, 1, 1);

        SpinBoxToreZaxe = new QDoubleSpinBox(gBTore);
        SpinBoxToreZaxe->setObjectName(QString::fromUtf8("SpinBoxToreZaxe"));
        SpinBoxToreZaxe->setDecimals(5);
        SpinBoxToreZaxe->setMinimum(-999999999.000000000000000);
        SpinBoxToreZaxe->setMaximum(999999999.000000000000000);
        SpinBoxToreZaxe->setValue(0.000000000000000);

        gridLayout_4->addWidget(SpinBoxToreZaxe, 2, 3, 1, 1);

        TLToreRayRev = new QLabel(gBTore);
        TLToreRayRev->setObjectName(QString::fromUtf8("TLToreRayRev"));
        sizePolicy.setHeightForWidth(TLToreRayRev->sizePolicy().hasHeightForWidth());
        TLToreRayRev->setSizePolicy(sizePolicy);
        TLToreRayRev->setWordWrap(false);

        gridLayout_4->addWidget(TLToreRayRev, 3, 0, 1, 1);

        SpinBoxToreRRev = new QDoubleSpinBox(gBTore);
        SpinBoxToreRRev->setObjectName(QString::fromUtf8("SpinBoxToreRRev"));
        SpinBoxToreRRev->setDecimals(5);
        SpinBoxToreRRev->setMaximum(1000000000.000000000000000);

        gridLayout_4->addWidget(SpinBoxToreRRev, 3, 1, 1, 1);

        TLToreRayPri = new QLabel(gBTore);
        TLToreRayPri->setObjectName(QString::fromUtf8("TLToreRayPri"));
        sizePolicy.setHeightForWidth(TLToreRayPri->sizePolicy().hasHeightForWidth());
        TLToreRayPri->setSizePolicy(sizePolicy);
        TLToreRayPri->setWordWrap(false);

        gridLayout_4->addWidget(TLToreRayPri, 3, 2, 1, 1);

        SpinBoxToreRPri = new QDoubleSpinBox(gBTore);
        SpinBoxToreRPri->setObjectName(QString::fromUtf8("SpinBoxToreRPri"));
        SpinBoxToreRPri->setDecimals(5);
        SpinBoxToreRPri->setMaximum(1000000000.000000000000000);

        gridLayout_4->addWidget(SpinBoxToreRPri, 3, 3, 1, 1);


        gridLayout_5->addWidget(gBTore, 5, 0, 1, 2);

        GBButtons = new QGroupBox(CreateBoundaryAn);
        GBButtons->setObjectName(QString::fromUtf8("GBButtons"));
        sizePolicy.setHeightForWidth(GBButtons->sizePolicy().hasHeightForWidth());
        GBButtons->setSizePolicy(sizePolicy);
        gridLayout3 = new QGridLayout(GBButtons);
#ifndef Q_OS_MAC
        gridLayout3->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout3->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
        buttonHelp = new QPushButton(GBButtons);
        buttonHelp->setObjectName(QString::fromUtf8("buttonHelp"));

        gridLayout3->addWidget(buttonHelp, 0, 3, 1, 1);

        buttonCancel = new QPushButton(GBButtons);
        buttonCancel->setObjectName(QString::fromUtf8("buttonCancel"));

        gridLayout3->addWidget(buttonCancel, 0, 2, 1, 1);

        buttonApply = new QPushButton(GBButtons);
        buttonApply->setObjectName(QString::fromUtf8("buttonApply"));

        gridLayout3->addWidget(buttonApply, 0, 1, 1, 1);

        buttonOk = new QPushButton(GBButtons);
        buttonOk->setObjectName(QString::fromUtf8("buttonOk"));

        gridLayout3->addWidget(buttonOk, 0, 0, 1, 1);


        gridLayout_5->addWidget(GBButtons, 6, 0, 1, 2);


        retranslateUi(CreateBoundaryAn);

        QMetaObject::connectSlotsByName(CreateBoundaryAn);
    } // setupUi

    void retranslateUi(QDialog *CreateBoundaryAn)
    {
        CreateBoundaryAn->setWindowTitle(QApplication::translate("CreateBoundaryAn", "Create an analytical boundary", nullptr));
        Name->setText(QApplication::translate("CreateBoundaryAn", "Name", nullptr));
        TypeBoundary->setTitle(QApplication::translate("CreateBoundaryAn", "Type of boundary", nullptr));
        RBCylindre->setText(QApplication::translate("CreateBoundaryAn", "Cylinder", nullptr));
        RBSphere->setText(QApplication::translate("CreateBoundaryAn", "Sphere", nullptr));
        RBCone->setText(QApplication::translate("CreateBoundaryAn", "Cone", nullptr));
        RBTore->setText(QApplication::translate("CreateBoundaryAn", "Torus", nullptr));
        gBCylindre->setTitle(QApplication::translate("CreateBoundaryAn", "Coordinates", nullptr));
        TLXcent->setText(QApplication::translate("CreateBoundaryAn", "X centre", nullptr));
        TLradius->setText(QApplication::translate("CreateBoundaryAn", "Radius", nullptr));
        TLZcent->setText(QApplication::translate("CreateBoundaryAn", "Z centre", nullptr));
        TLYcent->setText(QApplication::translate("CreateBoundaryAn", "Y centre", nullptr));
        TLXaxis->setText(QApplication::translate("CreateBoundaryAn", "X axis", nullptr));
        TLYaxis->setText(QApplication::translate("CreateBoundaryAn", "Y axis", nullptr));
        TLZaxis->setText(QApplication::translate("CreateBoundaryAn", "Z axis", nullptr));
        gBSphere->setTitle(QApplication::translate("CreateBoundaryAn", "Coordinates", nullptr));
        TLRayon->setText(QApplication::translate("CreateBoundaryAn", "Radius", nullptr));
        TLZcentre->setText(QApplication::translate("CreateBoundaryAn", "Z centre", nullptr));
        TLYcentre->setText(QApplication::translate("CreateBoundaryAn", "Y centre", nullptr));
        TLXcentre->setText(QApplication::translate("CreateBoundaryAn", "X centre", nullptr));
        gBCone->setTitle(QApplication::translate("CreateBoundaryAn", "Coordinates", nullptr));
        groupBox->setTitle(QApplication::translate("CreateBoundaryAn", "Definition", nullptr));
        RB_Def_radius->setText(QApplication::translate("CreateBoundaryAn", "Radius", nullptr));
        RB_Def_angle->setText(QApplication::translate("CreateBoundaryAn", "Angle", nullptr));
        TLCone_X1->setText(QApplication::translate("CreateBoundaryAn", "X 1", nullptr));
        TLCone_X2->setText(QApplication::translate("CreateBoundaryAn", "X 2", nullptr));
        TLCone_Y1->setText(QApplication::translate("CreateBoundaryAn", "Y 1", nullptr));
        TLCone_Y2->setText(QApplication::translate("CreateBoundaryAn", "Y 2", nullptr));
        TLCone_Z1->setText(QApplication::translate("CreateBoundaryAn", "Z 1", nullptr));
        TLCone_Z2->setText(QApplication::translate("CreateBoundaryAn", "Z 2", nullptr));
        TLCone_V1->setText(QApplication::translate("CreateBoundaryAn", "V 1", nullptr));
        TLCone_V2->setText(QApplication::translate("CreateBoundaryAn", "V 2", nullptr));
        gBTore->setTitle(QApplication::translate("CreateBoundaryAn", "Coordinates", nullptr));
        TLToreXcent->setText(QApplication::translate("CreateBoundaryAn", "X centre", nullptr));
        TLToreXaxe->setText(QApplication::translate("CreateBoundaryAn", "X axis", nullptr));
        TLToreYcent->setText(QApplication::translate("CreateBoundaryAn", "Y centre", nullptr));
        TLToreYaxe->setText(QApplication::translate("CreateBoundaryAn", "Y axis", nullptr));
        TLToreZcent->setText(QApplication::translate("CreateBoundaryAn", "Z centre", nullptr));
        TLToreZaxe->setText(QApplication::translate("CreateBoundaryAn", "Z axis", nullptr));
        TLToreRayRev->setText(QApplication::translate("CreateBoundaryAn", "R revolution", nullptr));
        TLToreRayPri->setText(QApplication::translate("CreateBoundaryAn", "Primary R", nullptr));
        GBButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("CreateBoundaryAn", "Help", nullptr));
        buttonCancel->setText(QApplication::translate("CreateBoundaryAn", "Cancel", nullptr));
        buttonApply->setText(QApplication::translate("CreateBoundaryAn", "Apply", nullptr));
        buttonOk->setText(QApplication::translate("CreateBoundaryAn", "OK", nullptr));
    } // retranslateUi
};

class SMESH_Ui_CreateBoundaryCAO
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
            CreateBoundaryCAO->setObjectName(QString::fromUtf8("CreateBoundaryCAO"));
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
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        GBButtons = new QGroupBox(CreateBoundaryCAO);
        GBButtons->setObjectName(QString::fromUtf8("GBButtons"));
        gridLayout1 = new QGridLayout(GBButtons);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        buttonHelp = new QPushButton(GBButtons);
        buttonHelp->setObjectName(QString::fromUtf8("buttonHelp"));

        gridLayout1->addWidget(buttonHelp, 0, 3, 1, 1);

        buttonCancel = new QPushButton(GBButtons);
        buttonCancel->setObjectName(QString::fromUtf8("buttonCancel"));

        gridLayout1->addWidget(buttonCancel, 0, 2, 1, 1);

        buttonApply = new QPushButton(GBButtons);
        buttonApply->setObjectName(QString::fromUtf8("buttonApply"));

        gridLayout1->addWidget(buttonApply, 0, 1, 1, 1);

        buttonOk = new QPushButton(GBButtons);
        buttonOk->setObjectName(QString::fromUtf8("buttonOk"));

        gridLayout1->addWidget(buttonOk, 0, 0, 1, 1);


        gridLayout->addWidget(GBButtons, 3, 0, 1, 3);

        CBGroupe = new QCheckBox(CreateBoundaryCAO);
        CBGroupe->setObjectName(QString::fromUtf8("CBGroupe"));

        gridLayout->addWidget(CBGroupe, 2, 0, 1, 3);

        LEFileName = new QLineEdit(CreateBoundaryCAO);
        LEFileName->setObjectName(QString::fromUtf8("LEFileName"));
        LEFileName->setMinimumSize(QSize(370, 21));

        gridLayout->addWidget(LEFileName, 1, 2, 1, 1);

        PushFichier = new QPushButton(CreateBoundaryCAO);
        PushFichier->setObjectName(QString::fromUtf8("PushFichier"));

        gridLayout->addWidget(PushFichier, 1, 1, 1, 1);

        XAO = new QLabel(CreateBoundaryCAO);
        XAO->setObjectName(QString::fromUtf8("XAO"));

        gridLayout->addWidget(XAO, 1, 0, 1, 1);

        LEName = new QLineEdit(CreateBoundaryCAO);
        LEName->setObjectName(QString::fromUtf8("LEName"));
        LEName->setMinimumSize(QSize(382, 21));
        LEName->setMaxLength(32);

        gridLayout->addWidget(LEName, 0, 1, 1, 2);

        Name = new QLabel(CreateBoundaryCAO);
        Name->setObjectName(QString::fromUtf8("Name"));

        gridLayout->addWidget(Name, 0, 0, 1, 1);


        retranslateUi(CreateBoundaryCAO);

        QMetaObject::connectSlotsByName(CreateBoundaryCAO);
    } // setupUi

    void retranslateUi(QDialog *CreateBoundaryCAO)
    {
        CreateBoundaryCAO->setWindowTitle(QApplication::translate("CreateBoundaryCAO", "Get CAO", nullptr));
        GBButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("CreateBoundaryCAO", "Help", nullptr));
        buttonCancel->setText(QApplication::translate("CreateBoundaryCAO", "Cancel", nullptr));
        buttonApply->setText(QApplication::translate("CreateBoundaryCAO", "Apply", nullptr));
        buttonOk->setText(QApplication::translate("CreateBoundaryCAO", "OK", nullptr));
        CBGroupe->setText(QApplication::translate("CreateBoundaryCAO", "Filtering with groups", nullptr));
        PushFichier->setText(QString());
        XAO->setText(QApplication::translate("CreateBoundaryCAO", "XAO", nullptr));
        Name->setText(QApplication::translate("CreateBoundaryCAO", "Name", nullptr));
    } // retranslateUi
};

class SMESH_Ui_CreateBoundaryDi
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
            CreateBoundaryDi->setObjectName(QString::fromUtf8("CreateBoundaryDi"));
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
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        GBButtons = new QGroupBox(CreateBoundaryDi);
        GBButtons->setObjectName(QString::fromUtf8("GBButtons"));
        gridLayout1 = new QGridLayout(GBButtons);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        buttonHelp = new QPushButton(GBButtons);
        buttonHelp->setObjectName(QString::fromUtf8("buttonHelp"));

        gridLayout1->addWidget(buttonHelp, 0, 3, 1, 1);

        buttonCancel = new QPushButton(GBButtons);
        buttonCancel->setObjectName(QString::fromUtf8("buttonCancel"));

        gridLayout1->addWidget(buttonCancel, 0, 2, 1, 1);

        buttonApply = new QPushButton(GBButtons);
        buttonApply->setObjectName(QString::fromUtf8("buttonApply"));

        gridLayout1->addWidget(buttonApply, 0, 1, 1, 1);

        buttonOk = new QPushButton(GBButtons);
        buttonOk->setObjectName(QString::fromUtf8("buttonOk"));

        gridLayout1->addWidget(buttonOk, 0, 0, 1, 1);


        gridLayout->addWidget(GBButtons, 3, 0, 1, 3);

        CBGroupe = new QCheckBox(CreateBoundaryDi);
        CBGroupe->setObjectName(QString::fromUtf8("CBGroupe"));

        gridLayout->addWidget(CBGroupe, 2, 0, 1, 3);

        LEFileName = new QLineEdit(CreateBoundaryDi);
        LEFileName->setObjectName(QString::fromUtf8("LEFileName"));
        LEFileName->setMinimumSize(QSize(370, 21));

        gridLayout->addWidget(LEFileName, 1, 2, 1, 1);

        PushFichier = new QPushButton(CreateBoundaryDi);
        PushFichier->setObjectName(QString::fromUtf8("PushFichier"));

        gridLayout->addWidget(PushFichier, 1, 1, 1, 1);

        Mesh = new QLabel(CreateBoundaryDi);
        Mesh->setObjectName(QString::fromUtf8("Mesh"));

        gridLayout->addWidget(Mesh, 1, 0, 1, 1);

        LEName = new QLineEdit(CreateBoundaryDi);
        LEName->setObjectName(QString::fromUtf8("LEName"));
        LEName->setMinimumSize(QSize(382, 21));
        LEName->setMaxLength(32);

        gridLayout->addWidget(LEName, 0, 1, 1, 2);

        Name = new QLabel(CreateBoundaryDi);
        Name->setObjectName(QString::fromUtf8("Name"));

        gridLayout->addWidget(Name, 0, 0, 1, 1);


        retranslateUi(CreateBoundaryDi);

        QMetaObject::connectSlotsByName(CreateBoundaryDi);
    } // setupUi

    void retranslateUi(QDialog *CreateBoundaryDi)
    {
        CreateBoundaryDi->setWindowTitle(QApplication::translate("CreateBoundaryDi", "Create a discrete boundary", nullptr));
        GBButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("CreateBoundaryDi", "Help", nullptr));
        buttonCancel->setText(QApplication::translate("CreateBoundaryDi", "Cancel", nullptr));
        buttonApply->setText(QApplication::translate("CreateBoundaryDi", "Apply", nullptr));
        buttonOk->setText(QApplication::translate("CreateBoundaryDi", "OK", nullptr));
        CBGroupe->setText(QApplication::translate("CreateBoundaryDi", "Filtering with groups", nullptr));
        PushFichier->setText(QString());
        Mesh->setText(QApplication::translate("CreateBoundaryDi", "Mesh", nullptr));
        Name->setText(QApplication::translate("CreateBoundaryDi", "Name", nullptr));
    } // retranslateUi
};

class SMESHGUI_EXPORT SMESH_CreateBoundaryAn : public QDialog, public SMESH_Ui_CreateBoundaryAn
{
  Q_OBJECT

public:
  SMESH_CreateBoundaryAn (SMESHGUI_HomardAdaptDlg* parent, bool modal,
                          SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                          QString caseName);
  virtual ~SMESH_CreateBoundaryAn();

protected :
  SMESH_CreateBoundaryAn (SMESHGUI_HomardAdaptDlg* parent,
                          SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                          QString caseName);

  SMESHGUI_HomardAdaptDlg * _parent;

  QString _Name;

  int _Type;
  double _BoundaryAnXcentre, _BoundaryAnYcentre, _BoundaryAnZcentre, _BoundaryAnRayon;
  double _BoundaryAnXaxis, _BoundaryAnYaxis, _BoundaryAnZaxis;
  double _Xcentre, _Ycentre, _Zcentre, _Rayon ;
  double _Xmin, _Xmax, _Xincr, _Ymin, _Ymax, _Yincr, _Zmin, _Zmax, _Zincr, _DMax ;
  double _BoundaryAnXcone1, _BoundaryAnYcone1, _BoundaryAnZcone1, _BoundaryAnRayon1;
  double _BoundaryAnXcone2, _BoundaryAnYcone2, _BoundaryAnZcone2, _BoundaryAnRayon2;
  double _BoundaryAnXaxisCone, _BoundaryAnYaxisCone, _BoundaryAnZaxisCone;
  double _BoundaryAnXorigCone, _BoundaryAnYorigCone, _BoundaryAnZorigCone;
  double _BoundaryAngle;
  double _BoundaryAnToreXcentre, _BoundaryAnToreYcentre, _BoundaryAnToreZcentre;
  double _BoundaryAnToreXaxe, _BoundaryAnToreYaxe, _BoundaryAnToreZaxe;
  double _BoundaryAnToreRRev, _BoundaryAnToreRPri;

  bool Chgt;

  SMESHHOMARD::HOMARD_Boundary_var aBoundaryAn;
  SMESHHOMARD::HOMARD_Gen_var myHomardGen;

  virtual void InitConnect();
  virtual void InitValBoundaryAn();
  virtual void InitMinMax();
  virtual void SetNewName();
  virtual bool CreateOrUpdateBoundaryAn();
  virtual void convertRayonAngle(int option);

public slots:
    virtual void SetCylinder();
    virtual void SetSphere();
    virtual void SetCone();
    virtual void SetConeR();
    virtual void SetConeA();
    virtual void SetTore();
    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();
};

class SMESHGUI_EXPORT SMESH_EditBoundaryAn : public SMESH_CreateBoundaryAn
{
    Q_OBJECT
public:
    SMESH_EditBoundaryAn( SMESHGUI_HomardAdaptDlg* parent, bool modal,
                          SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                          QString caseName, QString Name);
    virtual ~SMESH_EditBoundaryAn();

protected :
    bool CreateOrUpdateBoundaryAn();
    void InitValEdit();
    void InitValBoundaryAnLimit();
    void SetCylinder();
    void SetSphere();
    void SetConeR();
    void SetConeA();
    void SetTore();
    void InitValBoundaryAnCylindre();
    void InitValBoundaryAnSphere();
    void InitValBoundaryAnConeR();
    void InitValBoundaryAnConeA();
    void InitValBoundaryAnTore();
};

class SMESHGUI_EXPORT SMESH_CreateBoundaryCAO : public QDialog, public SMESH_Ui_CreateBoundaryCAO
{
    Q_OBJECT

public:
    SMESH_CreateBoundaryCAO( SMESHGUI_HomardAdaptDlg* parent, bool modal,
                             SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                             QString caseName, QString BoundaryName );
    ~SMESH_CreateBoundaryCAO();
    virtual void setGroups (QStringList listGroup);

protected :

    SMESHGUI_HomardAdaptDlg *_parent;

    QString _aName;

    SMESHHOMARD::HOMARD_Boundary_var aBoundary;
    SMESHHOMARD::HOMARD_Gen_var myHomardGen;

    QStringList  _listeGroupesBoundary;

    virtual void AssocieLesGroupes();
    virtual void InitConnect();
    virtual void SetNewName();

public slots:

    virtual void SetCAOFile();
    virtual void SetFiltrage();
    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();
};

class SMESHGUI_EXPORT SMESH_EditBoundaryCAO : public SMESH_CreateBoundaryCAO
{
    Q_OBJECT
public:
    SMESH_EditBoundaryCAO( SMESHGUI_HomardAdaptDlg* parent, bool modal,
                           SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                           QString caseName, QString Name );
    virtual ~SMESH_EditBoundaryCAO();

protected :
    virtual void InitValEdit();
    virtual bool PushOnApply();
    virtual void SetFiltrage();
};

class SMESHGUI_EXPORT SMESH_CreateBoundaryDi : public QDialog, public SMESH_Ui_CreateBoundaryDi
{
    Q_OBJECT

public:
    SMESH_CreateBoundaryDi( SMESHGUI_HomardAdaptDlg* parent, bool modal,
                            SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                            QString caseName, QString BoundaryName );
    ~SMESH_CreateBoundaryDi();
    virtual void setGroups (QStringList listGroup);

protected :

    SMESHGUI_HomardAdaptDlg *_parent;

    QString _aName;

    SMESHHOMARD::HOMARD_Boundary_var aBoundary;
    SMESHHOMARD::HOMARD_Gen_var myHomardGen;

    QStringList  _listeGroupesBoundary;

    virtual void AssocieLesGroupes();
    virtual void InitConnect();
    virtual void SetNewName();

public slots:

    virtual void SetMeshFile();
    virtual void SetFiltrage();
    virtual void PushOnOK();
    virtual bool PushOnApply();
    virtual void PushOnHelp();
};

class SMESHGUI_EXPORT SMESH_EditBoundaryDi : public SMESH_CreateBoundaryDi
{
    Q_OBJECT
public:
    SMESH_EditBoundaryDi( SMESHGUI_HomardAdaptDlg* parent, bool modal,
                          SMESHHOMARD::HOMARD_Gen_var myHomardGen0,
                          QString caseName, QString Name );
    virtual ~SMESH_EditBoundaryDi();

protected :
    virtual void InitValEdit();
    virtual bool PushOnApply();
    virtual void SetFiltrage();
};

#endif // MON_CREATEBOUNDARY_H
