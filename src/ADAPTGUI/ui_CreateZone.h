/********************************************************************************
** Form generated from reading UI file 'CreateZone.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREATEZONE_H
#define UI_CREATEZONE_H

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
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_CreateZone
{
public:
    QGridLayout *gridLayout;
    QLabel *Name;
    QLineEdit *LEName;
    QSpacerItem *horizontalSpacer_3;
    QGroupBox *TypeZone;
    QGridLayout *gridLayout1;
    QRadioButton *RBSphere;
    QRadioButton *RBPipe;
    QRadioButton *RBCylinder;
    QRadioButton *RBBox;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *gBBox;
    QGridLayout *gridLayout2;
    QLabel *TLXmini;
    QDoubleSpinBox *SpinBox_Xmini;
    QLabel *TLYmini;
    QDoubleSpinBox *SpinBox_Ymini;
    QLabel *TLZmini;
    QDoubleSpinBox *SpinBox_Zmini;
    QLabel *TLXmaxi;
    QLabel *TLZmaxi;
    QDoubleSpinBox *SpinBox_Zmaxi;
    QLabel *TLYmaxi;
    QDoubleSpinBox *SpinBox_Xmaxi;
    QDoubleSpinBox *SpinBox_Ymaxi;
    QGroupBox *gBSphere;
    QGridLayout *gridLayout3;
    QLabel *TLXcentre;
    QDoubleSpinBox *SpinBox_Xcentre;
    QLabel *TLYcentre;
    QDoubleSpinBox *SpinBox_Ycentre;
    QLabel *TLRayon;
    QDoubleSpinBox *SpinBox_Rayon;
    QLabel *TLZcentre;
    QDoubleSpinBox *SpinBox_Zcentre;
    QGroupBox *gBCylindre;
    QGridLayout *gridLayout4;
    QDoubleSpinBox *SpinBox_Haut;
    QLabel *TLHaut;
    QLabel *TLRadius;
    QDoubleSpinBox *SpinBox_Xaxis;
    QDoubleSpinBox *SpinBox_Yaxis;
    QDoubleSpinBox *SpinBox_Zaxis;
    QLabel *TLZaxis;
    QLabel *TLYaxis;
    QLabel *TLXaxis;
    QDoubleSpinBox *SpinBox_Ybase;
    QLabel *TLYbase;
    QLabel *TLZbase;
    QDoubleSpinBox *SpinBox_Zbase;
    QDoubleSpinBox *SpinBox_Radius;
    QLabel *TLXbase;
    QDoubleSpinBox *SpinBox_Xbase;
    QGroupBox *gBPipe;
    QGridLayout *gridLayout5;
    QLabel *TLHaut_p;
    QDoubleSpinBox *SpinBox_Xbase_p;
    QLabel *TLXbase_p;
    QDoubleSpinBox *SpinBox_Radius_int;
    QDoubleSpinBox *SpinBox_Zbase_p;
    QLabel *TLRadius_int;
    QLabel *TLZbase_p;
    QLabel *TLYbase_p;
    QDoubleSpinBox *SpinBox_Ybase_p;
    QLabel *TLXaxis_p;
    QLabel *TLYaxis_p;
    QLabel *TLZaxis_p;
    QDoubleSpinBox *SpinBox_Zaxis_p;
    QDoubleSpinBox *SpinBox_Yaxis_p;
    QDoubleSpinBox *SpinBox_Xaxis_p;
    QDoubleSpinBox *SpinBox_Radius_ext;
    QLabel *TLRadius_ext;
    QDoubleSpinBox *SpinBox_Haut_p;
    QGroupBox *GBButtons;
    QGridLayout *gridLayout6;
    QPushButton *buttonHelp;
    QPushButton *buttonCancel;
    QPushButton *buttonApply;
    QPushButton *buttonOk;
    QSpacerItem *horizontalSpacer;

    void setupUi(QDialog *CreateZone)
    {
        if (CreateZone->objectName().isEmpty())
            CreateZone->setObjectName(QStringLiteral("CreateZone"));
        CreateZone->resize(545, 778);
        CreateZone->setSizeIncrement(QSize(1, 1));
        CreateZone->setBaseSize(QSize(550, 400));
        CreateZone->setAutoFillBackground(true);
        CreateZone->setSizeGripEnabled(true);
        gridLayout = new QGridLayout(CreateZone);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        Name = new QLabel(CreateZone);
        Name->setObjectName(QStringLiteral("Name"));

        gridLayout->addWidget(Name, 0, 0, 1, 1);

        LEName = new QLineEdit(CreateZone);
        LEName->setObjectName(QStringLiteral("LEName"));
        LEName->setMaxLength(32);

        gridLayout->addWidget(LEName, 0, 1, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(142, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_3, 0, 2, 1, 1);

        TypeZone = new QGroupBox(CreateZone);
        TypeZone->setObjectName(QStringLiteral("TypeZone"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(TypeZone->sizePolicy().hasHeightForWidth());
        TypeZone->setSizePolicy(sizePolicy);
        TypeZone->setMinimumSize(QSize(340, 0));
        gridLayout1 = new QGridLayout(TypeZone);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QStringLiteral("gridLayout1"));
        RBSphere = new QRadioButton(TypeZone);
        RBSphere->setObjectName(QStringLiteral("RBSphere"));
        QIcon icon;
        icon.addFile(QStringLiteral("../../resources/zone_spherepoint.png"), QSize(), QIcon::Normal, QIcon::Off);
        RBSphere->setIcon(icon);

        gridLayout1->addWidget(RBSphere, 0, 3, 1, 1);

        RBPipe = new QRadioButton(TypeZone);
        RBPipe->setObjectName(QStringLiteral("RBPipe"));
        QIcon icon1;
        icon1.addFile(QStringLiteral("../../resources/pipe.png"), QSize(), QIcon::Normal, QIcon::Off);
        RBPipe->setIcon(icon1);

        gridLayout1->addWidget(RBPipe, 0, 2, 1, 1);

        RBCylinder = new QRadioButton(TypeZone);
        RBCylinder->setObjectName(QStringLiteral("RBCylinder"));
        QIcon icon2;
        icon2.addFile(QStringLiteral("../../resources/cylinderpointvector.png"), QSize(), QIcon::Normal, QIcon::Off);
        RBCylinder->setIcon(icon2);

        gridLayout1->addWidget(RBCylinder, 0, 1, 1, 1);

        RBBox = new QRadioButton(TypeZone);
        RBBox->setObjectName(QStringLiteral("RBBox"));
        QIcon icon3;
        icon3.addFile(QStringLiteral("../../resources/zone_boxdxyz.png"), QSize(), QIcon::Normal, QIcon::Off);
        RBBox->setIcon(icon3);
        RBBox->setCheckable(true);
        RBBox->setChecked(true);

        gridLayout1->addWidget(RBBox, 0, 0, 1, 1);


        gridLayout->addWidget(TypeZone, 1, 0, 1, 4);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 1, 4, 1, 1);

        gBBox = new QGroupBox(CreateZone);
        gBBox->setObjectName(QStringLiteral("gBBox"));
        sizePolicy.setHeightForWidth(gBBox->sizePolicy().hasHeightForWidth());
        gBBox->setSizePolicy(sizePolicy);
        gridLayout2 = new QGridLayout(gBBox);
#ifndef Q_OS_MAC
        gridLayout2->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout2->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout2->setObjectName(QStringLiteral("gridLayout2"));
        TLXmini = new QLabel(gBBox);
        TLXmini->setObjectName(QStringLiteral("TLXmini"));
        sizePolicy.setHeightForWidth(TLXmini->sizePolicy().hasHeightForWidth());
        TLXmini->setSizePolicy(sizePolicy);
        TLXmini->setWordWrap(false);

        gridLayout2->addWidget(TLXmini, 0, 0, 1, 1);

        SpinBox_Xmini = new QDoubleSpinBox(gBBox);
        SpinBox_Xmini->setObjectName(QStringLiteral("SpinBox_Xmini"));
        SpinBox_Xmini->setDecimals(5);
        SpinBox_Xmini->setMinimum(-1e+9);
        SpinBox_Xmini->setMaximum(1e+9);
        SpinBox_Xmini->setValue(0);

        gridLayout2->addWidget(SpinBox_Xmini, 0, 1, 1, 1);

        TLYmini = new QLabel(gBBox);
        TLYmini->setObjectName(QStringLiteral("TLYmini"));
        sizePolicy.setHeightForWidth(TLYmini->sizePolicy().hasHeightForWidth());
        TLYmini->setSizePolicy(sizePolicy);
        TLYmini->setWordWrap(false);

        gridLayout2->addWidget(TLYmini, 1, 0, 1, 1);

        SpinBox_Ymini = new QDoubleSpinBox(gBBox);
        SpinBox_Ymini->setObjectName(QStringLiteral("SpinBox_Ymini"));
        SpinBox_Ymini->setDecimals(5);
        SpinBox_Ymini->setMinimum(-1e+9);
        SpinBox_Ymini->setMaximum(1e+9);
        SpinBox_Ymini->setValue(0);

        gridLayout2->addWidget(SpinBox_Ymini, 1, 1, 1, 1);

        TLZmini = new QLabel(gBBox);
        TLZmini->setObjectName(QStringLiteral("TLZmini"));
        sizePolicy.setHeightForWidth(TLZmini->sizePolicy().hasHeightForWidth());
        TLZmini->setSizePolicy(sizePolicy);
        TLZmini->setWordWrap(false);

        gridLayout2->addWidget(TLZmini, 2, 0, 1, 1);

        SpinBox_Zmini = new QDoubleSpinBox(gBBox);
        SpinBox_Zmini->setObjectName(QStringLiteral("SpinBox_Zmini"));
        SpinBox_Zmini->setDecimals(5);
        SpinBox_Zmini->setMinimum(-1e+9);
        SpinBox_Zmini->setMaximum(1e+9);
        SpinBox_Zmini->setValue(0);

        gridLayout2->addWidget(SpinBox_Zmini, 2, 1, 1, 1);

        TLXmaxi = new QLabel(gBBox);
        TLXmaxi->setObjectName(QStringLiteral("TLXmaxi"));
        sizePolicy.setHeightForWidth(TLXmaxi->sizePolicy().hasHeightForWidth());
        TLXmaxi->setSizePolicy(sizePolicy);
        TLXmaxi->setWordWrap(false);

        gridLayout2->addWidget(TLXmaxi, 0, 2, 1, 1);

        TLZmaxi = new QLabel(gBBox);
        TLZmaxi->setObjectName(QStringLiteral("TLZmaxi"));
        sizePolicy.setHeightForWidth(TLZmaxi->sizePolicy().hasHeightForWidth());
        TLZmaxi->setSizePolicy(sizePolicy);
        TLZmaxi->setWordWrap(false);

        gridLayout2->addWidget(TLZmaxi, 2, 2, 1, 1);

        SpinBox_Zmaxi = new QDoubleSpinBox(gBBox);
        SpinBox_Zmaxi->setObjectName(QStringLiteral("SpinBox_Zmaxi"));
        SpinBox_Zmaxi->setDecimals(5);
        SpinBox_Zmaxi->setMinimum(-1e+9);
        SpinBox_Zmaxi->setMaximum(1e+9);
        SpinBox_Zmaxi->setValue(0);

        gridLayout2->addWidget(SpinBox_Zmaxi, 2, 3, 1, 1);

        TLYmaxi = new QLabel(gBBox);
        TLYmaxi->setObjectName(QStringLiteral("TLYmaxi"));
        sizePolicy.setHeightForWidth(TLYmaxi->sizePolicy().hasHeightForWidth());
        TLYmaxi->setSizePolicy(sizePolicy);
        TLYmaxi->setWordWrap(false);

        gridLayout2->addWidget(TLYmaxi, 1, 2, 1, 1);

        SpinBox_Xmaxi = new QDoubleSpinBox(gBBox);
        SpinBox_Xmaxi->setObjectName(QStringLiteral("SpinBox_Xmaxi"));
        SpinBox_Xmaxi->setDecimals(5);
        SpinBox_Xmaxi->setMinimum(-1e+9);
        SpinBox_Xmaxi->setMaximum(1e+9);
        SpinBox_Xmaxi->setValue(0);

        gridLayout2->addWidget(SpinBox_Xmaxi, 0, 3, 1, 1);

        SpinBox_Ymaxi = new QDoubleSpinBox(gBBox);
        SpinBox_Ymaxi->setObjectName(QStringLiteral("SpinBox_Ymaxi"));
        SpinBox_Ymaxi->setDecimals(5);
        SpinBox_Ymaxi->setMinimum(-1e+9);
        SpinBox_Ymaxi->setMaximum(1e+9);
        SpinBox_Ymaxi->setValue(0);

        gridLayout2->addWidget(SpinBox_Ymaxi, 1, 3, 1, 1);


        gridLayout->addWidget(gBBox, 2, 0, 1, 4);

        gBSphere = new QGroupBox(CreateZone);
        gBSphere->setObjectName(QStringLiteral("gBSphere"));
        sizePolicy.setHeightForWidth(gBSphere->sizePolicy().hasHeightForWidth());
        gBSphere->setSizePolicy(sizePolicy);
        gridLayout3 = new QGridLayout(gBSphere);
#ifndef Q_OS_MAC
        gridLayout3->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout3->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout3->setObjectName(QStringLiteral("gridLayout3"));
        TLXcentre = new QLabel(gBSphere);
        TLXcentre->setObjectName(QStringLiteral("TLXcentre"));
        sizePolicy.setHeightForWidth(TLXcentre->sizePolicy().hasHeightForWidth());
        TLXcentre->setSizePolicy(sizePolicy);
        TLXcentre->setWordWrap(false);

        gridLayout3->addWidget(TLXcentre, 0, 0, 1, 1);

        SpinBox_Xcentre = new QDoubleSpinBox(gBSphere);
        SpinBox_Xcentre->setObjectName(QStringLiteral("SpinBox_Xcentre"));
        SpinBox_Xcentre->setDecimals(5);
        SpinBox_Xcentre->setMinimum(-1e+9);
        SpinBox_Xcentre->setMaximum(1e+9);
        SpinBox_Xcentre->setValue(0);

        gridLayout3->addWidget(SpinBox_Xcentre, 0, 1, 1, 1);

        TLYcentre = new QLabel(gBSphere);
        TLYcentre->setObjectName(QStringLiteral("TLYcentre"));
        sizePolicy.setHeightForWidth(TLYcentre->sizePolicy().hasHeightForWidth());
        TLYcentre->setSizePolicy(sizePolicy);
        TLYcentre->setWordWrap(false);

        gridLayout3->addWidget(TLYcentre, 1, 0, 1, 1);

        SpinBox_Ycentre = new QDoubleSpinBox(gBSphere);
        SpinBox_Ycentre->setObjectName(QStringLiteral("SpinBox_Ycentre"));
        SpinBox_Ycentre->setDecimals(5);
        SpinBox_Ycentre->setMinimum(-1e+9);
        SpinBox_Ycentre->setMaximum(1e+9);
        SpinBox_Ycentre->setValue(0);

        gridLayout3->addWidget(SpinBox_Ycentre, 1, 1, 1, 1);

        TLRayon = new QLabel(gBSphere);
        TLRayon->setObjectName(QStringLiteral("TLRayon"));
        sizePolicy.setHeightForWidth(TLRayon->sizePolicy().hasHeightForWidth());
        TLRayon->setSizePolicy(sizePolicy);
        TLRayon->setWordWrap(false);

        gridLayout3->addWidget(TLRayon, 1, 2, 1, 1);

        SpinBox_Rayon = new QDoubleSpinBox(gBSphere);
        SpinBox_Rayon->setObjectName(QStringLiteral("SpinBox_Rayon"));
        SpinBox_Rayon->setDecimals(5);
        SpinBox_Rayon->setMinimum(0);
        SpinBox_Rayon->setMaximum(1e+9);
        SpinBox_Rayon->setValue(0);

        gridLayout3->addWidget(SpinBox_Rayon, 1, 3, 1, 1);

        TLZcentre = new QLabel(gBSphere);
        TLZcentre->setObjectName(QStringLiteral("TLZcentre"));
        sizePolicy.setHeightForWidth(TLZcentre->sizePolicy().hasHeightForWidth());
        TLZcentre->setSizePolicy(sizePolicy);
        TLZcentre->setWordWrap(false);

        gridLayout3->addWidget(TLZcentre, 2, 0, 1, 1);

        SpinBox_Zcentre = new QDoubleSpinBox(gBSphere);
        SpinBox_Zcentre->setObjectName(QStringLiteral("SpinBox_Zcentre"));
        SpinBox_Zcentre->setDecimals(5);
        SpinBox_Zcentre->setMinimum(-1e+9);
        SpinBox_Zcentre->setMaximum(1e+9);
        SpinBox_Zcentre->setValue(0);

        gridLayout3->addWidget(SpinBox_Zcentre, 2, 1, 1, 1);


        gridLayout->addWidget(gBSphere, 3, 0, 1, 4);

        gBCylindre = new QGroupBox(CreateZone);
        gBCylindre->setObjectName(QStringLiteral("gBCylindre"));
        sizePolicy.setHeightForWidth(gBCylindre->sizePolicy().hasHeightForWidth());
        gBCylindre->setSizePolicy(sizePolicy);
        gBCylindre->setFocusPolicy(Qt::TabFocus);
        gridLayout4 = new QGridLayout(gBCylindre);
#ifndef Q_OS_MAC
        gridLayout4->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout4->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout4->setObjectName(QStringLiteral("gridLayout4"));
        SpinBox_Haut = new QDoubleSpinBox(gBCylindre);
        SpinBox_Haut->setObjectName(QStringLiteral("SpinBox_Haut"));
        SpinBox_Haut->setDecimals(5);
        SpinBox_Haut->setMaximum(1e+9);

        gridLayout4->addWidget(SpinBox_Haut, 4, 1, 1, 1);

        TLHaut = new QLabel(gBCylindre);
        TLHaut->setObjectName(QStringLiteral("TLHaut"));
        sizePolicy.setHeightForWidth(TLHaut->sizePolicy().hasHeightForWidth());
        TLHaut->setSizePolicy(sizePolicy);
        TLHaut->setWordWrap(false);

        gridLayout4->addWidget(TLHaut, 4, 0, 1, 1);

        TLRadius = new QLabel(gBCylindre);
        TLRadius->setObjectName(QStringLiteral("TLRadius"));
        sizePolicy.setHeightForWidth(TLRadius->sizePolicy().hasHeightForWidth());
        TLRadius->setSizePolicy(sizePolicy);
        TLRadius->setWordWrap(false);

        gridLayout4->addWidget(TLRadius, 3, 0, 1, 1);

        SpinBox_Xaxis = new QDoubleSpinBox(gBCylindre);
        SpinBox_Xaxis->setObjectName(QStringLiteral("SpinBox_Xaxis"));
        SpinBox_Xaxis->setDecimals(5);
        SpinBox_Xaxis->setMinimum(-1e+9);
        SpinBox_Xaxis->setMaximum(1e+9);
        SpinBox_Xaxis->setValue(0);

        gridLayout4->addWidget(SpinBox_Xaxis, 0, 3, 1, 1);

        SpinBox_Yaxis = new QDoubleSpinBox(gBCylindre);
        SpinBox_Yaxis->setObjectName(QStringLiteral("SpinBox_Yaxis"));
        SpinBox_Yaxis->setDecimals(5);
        SpinBox_Yaxis->setMinimum(-1e+9);
        SpinBox_Yaxis->setMaximum(1e+9);
        SpinBox_Yaxis->setValue(0);

        gridLayout4->addWidget(SpinBox_Yaxis, 1, 3, 1, 1);

        SpinBox_Zaxis = new QDoubleSpinBox(gBCylindre);
        SpinBox_Zaxis->setObjectName(QStringLiteral("SpinBox_Zaxis"));
        SpinBox_Zaxis->setDecimals(5);
        SpinBox_Zaxis->setMinimum(-1e+9);
        SpinBox_Zaxis->setMaximum(1e+9);
        SpinBox_Zaxis->setValue(0);

        gridLayout4->addWidget(SpinBox_Zaxis, 2, 3, 1, 1);

        TLZaxis = new QLabel(gBCylindre);
        TLZaxis->setObjectName(QStringLiteral("TLZaxis"));
        sizePolicy.setHeightForWidth(TLZaxis->sizePolicy().hasHeightForWidth());
        TLZaxis->setSizePolicy(sizePolicy);
        TLZaxis->setWordWrap(false);

        gridLayout4->addWidget(TLZaxis, 2, 2, 1, 1);

        TLYaxis = new QLabel(gBCylindre);
        TLYaxis->setObjectName(QStringLiteral("TLYaxis"));
        sizePolicy.setHeightForWidth(TLYaxis->sizePolicy().hasHeightForWidth());
        TLYaxis->setSizePolicy(sizePolicy);
        TLYaxis->setWordWrap(false);

        gridLayout4->addWidget(TLYaxis, 1, 2, 1, 1);

        TLXaxis = new QLabel(gBCylindre);
        TLXaxis->setObjectName(QStringLiteral("TLXaxis"));
        sizePolicy.setHeightForWidth(TLXaxis->sizePolicy().hasHeightForWidth());
        TLXaxis->setSizePolicy(sizePolicy);
        TLXaxis->setWordWrap(false);

        gridLayout4->addWidget(TLXaxis, 0, 2, 1, 1);

        SpinBox_Ybase = new QDoubleSpinBox(gBCylindre);
        SpinBox_Ybase->setObjectName(QStringLiteral("SpinBox_Ybase"));
        SpinBox_Ybase->setDecimals(5);
        SpinBox_Ybase->setMinimum(-1e+9);
        SpinBox_Ybase->setMaximum(1e+9);
        SpinBox_Ybase->setValue(0);

        gridLayout4->addWidget(SpinBox_Ybase, 1, 1, 1, 1);

        TLYbase = new QLabel(gBCylindre);
        TLYbase->setObjectName(QStringLiteral("TLYbase"));
        sizePolicy.setHeightForWidth(TLYbase->sizePolicy().hasHeightForWidth());
        TLYbase->setSizePolicy(sizePolicy);
        TLYbase->setWordWrap(false);

        gridLayout4->addWidget(TLYbase, 1, 0, 1, 1);

        TLZbase = new QLabel(gBCylindre);
        TLZbase->setObjectName(QStringLiteral("TLZbase"));
        sizePolicy.setHeightForWidth(TLZbase->sizePolicy().hasHeightForWidth());
        TLZbase->setSizePolicy(sizePolicy);
        TLZbase->setWordWrap(false);

        gridLayout4->addWidget(TLZbase, 2, 0, 1, 1);

        SpinBox_Zbase = new QDoubleSpinBox(gBCylindre);
        SpinBox_Zbase->setObjectName(QStringLiteral("SpinBox_Zbase"));
        SpinBox_Zbase->setDecimals(5);
        SpinBox_Zbase->setMinimum(-1e+9);
        SpinBox_Zbase->setMaximum(1e+9);
        SpinBox_Zbase->setValue(0);

        gridLayout4->addWidget(SpinBox_Zbase, 2, 1, 1, 1);

        SpinBox_Radius = new QDoubleSpinBox(gBCylindre);
        SpinBox_Radius->setObjectName(QStringLiteral("SpinBox_Radius"));
        SpinBox_Radius->setDecimals(5);
        SpinBox_Radius->setMaximum(1e+9);

        gridLayout4->addWidget(SpinBox_Radius, 3, 1, 1, 1);

        TLXbase = new QLabel(gBCylindre);
        TLXbase->setObjectName(QStringLiteral("TLXbase"));
        sizePolicy.setHeightForWidth(TLXbase->sizePolicy().hasHeightForWidth());
        TLXbase->setSizePolicy(sizePolicy);
        TLXbase->setWordWrap(false);

        gridLayout4->addWidget(TLXbase, 0, 0, 1, 1);

        SpinBox_Xbase = new QDoubleSpinBox(gBCylindre);
        SpinBox_Xbase->setObjectName(QStringLiteral("SpinBox_Xbase"));
        SpinBox_Xbase->setDecimals(5);
        SpinBox_Xbase->setMinimum(-1e+9);
        SpinBox_Xbase->setMaximum(1e+9);
        SpinBox_Xbase->setValue(0);

        gridLayout4->addWidget(SpinBox_Xbase, 0, 1, 1, 1);


        gridLayout->addWidget(gBCylindre, 4, 0, 1, 4);

        gBPipe = new QGroupBox(CreateZone);
        gBPipe->setObjectName(QStringLiteral("gBPipe"));
        sizePolicy.setHeightForWidth(gBPipe->sizePolicy().hasHeightForWidth());
        gBPipe->setSizePolicy(sizePolicy);
        gridLayout5 = new QGridLayout(gBPipe);
#ifndef Q_OS_MAC
        gridLayout5->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout5->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout5->setObjectName(QStringLiteral("gridLayout5"));
        TLHaut_p = new QLabel(gBPipe);
        TLHaut_p->setObjectName(QStringLiteral("TLHaut_p"));
        sizePolicy.setHeightForWidth(TLHaut_p->sizePolicy().hasHeightForWidth());
        TLHaut_p->setSizePolicy(sizePolicy);
        TLHaut_p->setWordWrap(false);

        gridLayout5->addWidget(TLHaut_p, 4, 0, 1, 1);

        SpinBox_Xbase_p = new QDoubleSpinBox(gBPipe);
        SpinBox_Xbase_p->setObjectName(QStringLiteral("SpinBox_Xbase_p"));
        SpinBox_Xbase_p->setDecimals(5);
        SpinBox_Xbase_p->setMinimum(-1e+9);
        SpinBox_Xbase_p->setMaximum(1e+9);
        SpinBox_Xbase_p->setValue(0);

        gridLayout5->addWidget(SpinBox_Xbase_p, 0, 1, 1, 1);

        TLXbase_p = new QLabel(gBPipe);
        TLXbase_p->setObjectName(QStringLiteral("TLXbase_p"));
        sizePolicy.setHeightForWidth(TLXbase_p->sizePolicy().hasHeightForWidth());
        TLXbase_p->setSizePolicy(sizePolicy);
        TLXbase_p->setWordWrap(false);

        gridLayout5->addWidget(TLXbase_p, 0, 0, 1, 1);

        SpinBox_Radius_int = new QDoubleSpinBox(gBPipe);
        SpinBox_Radius_int->setObjectName(QStringLiteral("SpinBox_Radius_int"));
        SpinBox_Radius_int->setDecimals(5);
        SpinBox_Radius_int->setMaximum(1e+9);

        gridLayout5->addWidget(SpinBox_Radius_int, 3, 1, 1, 1);

        SpinBox_Zbase_p = new QDoubleSpinBox(gBPipe);
        SpinBox_Zbase_p->setObjectName(QStringLiteral("SpinBox_Zbase_p"));
        SpinBox_Zbase_p->setDecimals(5);
        SpinBox_Zbase_p->setMinimum(-1e+9);
        SpinBox_Zbase_p->setMaximum(1e+9);
        SpinBox_Zbase_p->setValue(0);

        gridLayout5->addWidget(SpinBox_Zbase_p, 2, 1, 1, 1);

        TLRadius_int = new QLabel(gBPipe);
        TLRadius_int->setObjectName(QStringLiteral("TLRadius_int"));
        sizePolicy.setHeightForWidth(TLRadius_int->sizePolicy().hasHeightForWidth());
        TLRadius_int->setSizePolicy(sizePolicy);
        TLRadius_int->setWordWrap(false);

        gridLayout5->addWidget(TLRadius_int, 3, 0, 1, 1);

        TLZbase_p = new QLabel(gBPipe);
        TLZbase_p->setObjectName(QStringLiteral("TLZbase_p"));
        sizePolicy.setHeightForWidth(TLZbase_p->sizePolicy().hasHeightForWidth());
        TLZbase_p->setSizePolicy(sizePolicy);
        TLZbase_p->setWordWrap(false);

        gridLayout5->addWidget(TLZbase_p, 2, 0, 1, 1);

        TLYbase_p = new QLabel(gBPipe);
        TLYbase_p->setObjectName(QStringLiteral("TLYbase_p"));
        sizePolicy.setHeightForWidth(TLYbase_p->sizePolicy().hasHeightForWidth());
        TLYbase_p->setSizePolicy(sizePolicy);
        TLYbase_p->setWordWrap(false);

        gridLayout5->addWidget(TLYbase_p, 1, 0, 1, 1);

        SpinBox_Ybase_p = new QDoubleSpinBox(gBPipe);
        SpinBox_Ybase_p->setObjectName(QStringLiteral("SpinBox_Ybase_p"));
        SpinBox_Ybase_p->setDecimals(5);
        SpinBox_Ybase_p->setMinimum(-1e+9);
        SpinBox_Ybase_p->setMaximum(1e+9);
        SpinBox_Ybase_p->setValue(0);

        gridLayout5->addWidget(SpinBox_Ybase_p, 1, 1, 1, 1);

        TLXaxis_p = new QLabel(gBPipe);
        TLXaxis_p->setObjectName(QStringLiteral("TLXaxis_p"));
        sizePolicy.setHeightForWidth(TLXaxis_p->sizePolicy().hasHeightForWidth());
        TLXaxis_p->setSizePolicy(sizePolicy);
        TLXaxis_p->setWordWrap(false);

        gridLayout5->addWidget(TLXaxis_p, 0, 2, 1, 1);

        TLYaxis_p = new QLabel(gBPipe);
        TLYaxis_p->setObjectName(QStringLiteral("TLYaxis_p"));
        sizePolicy.setHeightForWidth(TLYaxis_p->sizePolicy().hasHeightForWidth());
        TLYaxis_p->setSizePolicy(sizePolicy);
        TLYaxis_p->setWordWrap(false);

        gridLayout5->addWidget(TLYaxis_p, 1, 2, 1, 1);

        TLZaxis_p = new QLabel(gBPipe);
        TLZaxis_p->setObjectName(QStringLiteral("TLZaxis_p"));
        sizePolicy.setHeightForWidth(TLZaxis_p->sizePolicy().hasHeightForWidth());
        TLZaxis_p->setSizePolicy(sizePolicy);
        TLZaxis_p->setWordWrap(false);

        gridLayout5->addWidget(TLZaxis_p, 2, 2, 1, 1);

        SpinBox_Zaxis_p = new QDoubleSpinBox(gBPipe);
        SpinBox_Zaxis_p->setObjectName(QStringLiteral("SpinBox_Zaxis_p"));
        SpinBox_Zaxis_p->setDecimals(5);
        SpinBox_Zaxis_p->setMinimum(-1e+9);
        SpinBox_Zaxis_p->setMaximum(1e+9);
        SpinBox_Zaxis_p->setValue(0);

        gridLayout5->addWidget(SpinBox_Zaxis_p, 2, 3, 1, 1);

        SpinBox_Yaxis_p = new QDoubleSpinBox(gBPipe);
        SpinBox_Yaxis_p->setObjectName(QStringLiteral("SpinBox_Yaxis_p"));
        SpinBox_Yaxis_p->setDecimals(5);
        SpinBox_Yaxis_p->setMinimum(-1e+9);
        SpinBox_Yaxis_p->setMaximum(1e+9);
        SpinBox_Yaxis_p->setValue(0);

        gridLayout5->addWidget(SpinBox_Yaxis_p, 1, 3, 1, 1);

        SpinBox_Xaxis_p = new QDoubleSpinBox(gBPipe);
        SpinBox_Xaxis_p->setObjectName(QStringLiteral("SpinBox_Xaxis_p"));
        SpinBox_Xaxis_p->setDecimals(5);
        SpinBox_Xaxis_p->setMinimum(-1e+9);
        SpinBox_Xaxis_p->setMaximum(1e+9);
        SpinBox_Xaxis_p->setValue(0);

        gridLayout5->addWidget(SpinBox_Xaxis_p, 0, 3, 1, 1);

        SpinBox_Radius_ext = new QDoubleSpinBox(gBPipe);
        SpinBox_Radius_ext->setObjectName(QStringLiteral("SpinBox_Radius_ext"));
        SpinBox_Radius_ext->setDecimals(5);
        SpinBox_Radius_ext->setMaximum(1e+9);

        gridLayout5->addWidget(SpinBox_Radius_ext, 3, 3, 1, 1);

        TLRadius_ext = new QLabel(gBPipe);
        TLRadius_ext->setObjectName(QStringLiteral("TLRadius_ext"));
        sizePolicy.setHeightForWidth(TLRadius_ext->sizePolicy().hasHeightForWidth());
        TLRadius_ext->setSizePolicy(sizePolicy);
        TLRadius_ext->setWordWrap(false);

        gridLayout5->addWidget(TLRadius_ext, 3, 2, 1, 1);

        SpinBox_Haut_p = new QDoubleSpinBox(gBPipe);
        SpinBox_Haut_p->setObjectName(QStringLiteral("SpinBox_Haut_p"));
        SpinBox_Haut_p->setDecimals(5);
        SpinBox_Haut_p->setMaximum(1e+9);

        gridLayout5->addWidget(SpinBox_Haut_p, 4, 1, 1, 1);


        gridLayout->addWidget(gBPipe, 5, 0, 1, 5);

        GBButtons = new QGroupBox(CreateZone);
        GBButtons->setObjectName(QStringLiteral("GBButtons"));
        sizePolicy.setHeightForWidth(GBButtons->sizePolicy().hasHeightForWidth());
        GBButtons->setSizePolicy(sizePolicy);
        gridLayout6 = new QGridLayout(GBButtons);
#ifndef Q_OS_MAC
        gridLayout6->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout6->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout6->setObjectName(QStringLiteral("gridLayout6"));
        buttonHelp = new QPushButton(GBButtons);
        buttonHelp->setObjectName(QStringLiteral("buttonHelp"));

        gridLayout6->addWidget(buttonHelp, 0, 3, 1, 1);

        buttonCancel = new QPushButton(GBButtons);
        buttonCancel->setObjectName(QStringLiteral("buttonCancel"));

        gridLayout6->addWidget(buttonCancel, 0, 2, 1, 1);

        buttonApply = new QPushButton(GBButtons);
        buttonApply->setObjectName(QStringLiteral("buttonApply"));

        gridLayout6->addWidget(buttonApply, 0, 1, 1, 1);

        buttonOk = new QPushButton(GBButtons);
        buttonOk->setObjectName(QStringLiteral("buttonOk"));

        gridLayout6->addWidget(buttonOk, 0, 0, 1, 1);


        gridLayout->addWidget(GBButtons, 6, 0, 1, 3);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 6, 3, 1, 2);


        retranslateUi(CreateZone);

        QMetaObject::connectSlotsByName(CreateZone);
    } // setupUi

    void retranslateUi(QDialog *CreateZone)
    {
        CreateZone->setWindowTitle(QApplication::translate("CreateZone", "Create a zone", Q_NULLPTR));
        Name->setText(QApplication::translate("CreateZone", "Name", Q_NULLPTR));
        TypeZone->setTitle(QApplication::translate("CreateZone", "Type of zone", Q_NULLPTR));
        RBSphere->setText(QApplication::translate("CreateZone", "Sphere", Q_NULLPTR));
        RBPipe->setText(QApplication::translate("CreateZone", "Pipe", Q_NULLPTR));
        RBCylinder->setText(QApplication::translate("CreateZone", "Cylinder", Q_NULLPTR));
        RBBox->setText(QApplication::translate("CreateZone", "Box", Q_NULLPTR));
        gBBox->setTitle(QApplication::translate("CreateZone", "Coordinates", Q_NULLPTR));
        TLXmini->setText(QApplication::translate("CreateZone", "X mini", Q_NULLPTR));
        TLYmini->setText(QApplication::translate("CreateZone", "Y mini", Q_NULLPTR));
        TLZmini->setText(QApplication::translate("CreateZone", "Z mini", Q_NULLPTR));
        TLXmaxi->setText(QApplication::translate("CreateZone", "X maxi", Q_NULLPTR));
        TLZmaxi->setText(QApplication::translate("CreateZone", "Z maxi", Q_NULLPTR));
        TLYmaxi->setText(QApplication::translate("CreateZone", "Y maxi", Q_NULLPTR));
        gBSphere->setTitle(QApplication::translate("CreateZone", "Coordinates", Q_NULLPTR));
        TLXcentre->setText(QApplication::translate("CreateZone", "X centre", Q_NULLPTR));
        TLYcentre->setText(QApplication::translate("CreateZone", "Y centre", Q_NULLPTR));
        TLRayon->setText(QApplication::translate("CreateZone", "Radius", Q_NULLPTR));
        TLZcentre->setText(QApplication::translate("CreateZone", "Z centre", Q_NULLPTR));
        gBCylindre->setTitle(QApplication::translate("CreateZone", "Coordinates", Q_NULLPTR));
        TLHaut->setText(QApplication::translate("CreateZone", "Height", Q_NULLPTR));
        TLRadius->setText(QApplication::translate("CreateZone", "Radius", Q_NULLPTR));
        TLZaxis->setText(QApplication::translate("CreateZone", "Z axis", Q_NULLPTR));
        TLYaxis->setText(QApplication::translate("CreateZone", "Y axis", Q_NULLPTR));
        TLXaxis->setText(QApplication::translate("CreateZone", "X axis", Q_NULLPTR));
        TLYbase->setText(QApplication::translate("CreateZone", "Y base", Q_NULLPTR));
        TLZbase->setText(QApplication::translate("CreateZone", "Z base", Q_NULLPTR));
        TLXbase->setText(QApplication::translate("CreateZone", "X base", Q_NULLPTR));
        gBPipe->setTitle(QApplication::translate("CreateZone", "Coordinates", Q_NULLPTR));
        TLHaut_p->setText(QApplication::translate("CreateZone", "Height", Q_NULLPTR));
        TLXbase_p->setText(QApplication::translate("CreateZone", "X base", Q_NULLPTR));
        TLRadius_int->setText(QApplication::translate("CreateZone", "Internal radius", Q_NULLPTR));
        TLZbase_p->setText(QApplication::translate("CreateZone", "Z base", Q_NULLPTR));
        TLYbase_p->setText(QApplication::translate("CreateZone", "Y base", Q_NULLPTR));
        TLXaxis_p->setText(QApplication::translate("CreateZone", "X axis", Q_NULLPTR));
        TLYaxis_p->setText(QApplication::translate("CreateZone", "Y axis", Q_NULLPTR));
        TLZaxis_p->setText(QApplication::translate("CreateZone", "Z axis", Q_NULLPTR));
        TLRadius_ext->setText(QApplication::translate("CreateZone", "External radius", Q_NULLPTR));
        GBButtons->setTitle(QString());
        buttonHelp->setText(QApplication::translate("CreateZone", "Help", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("CreateZone", "Cancel", Q_NULLPTR));
        buttonApply->setText(QApplication::translate("CreateZone", "Apply", Q_NULLPTR));
        buttonOk->setText(QApplication::translate("CreateZone", "OK", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CreateZone: public Ui_CreateZone {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREATEZONE_H
