/********************************************************************************
** Form generated from reading UI file 'EditFile.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDITFILE_H
#define UI_EDITFILE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_EditFile
{
public:
    QGridLayout *gridLayout;
    QSpacerItem *spacerItem;
    QGroupBox *GroupButtons;
    QGridLayout *gridLayout1;
    QPushButton *buttonPrint;
    QPushButton *buttonQuit;
    QSpacerItem *spacerItem1;
    QTextBrowser *QTBEditFile;

    void setupUi(QWidget *EditFile)
    {
        if (EditFile->objectName().isEmpty())
            EditFile->setObjectName(QStringLiteral("EditFile"));
        EditFile->resize(675, 901);
        gridLayout = new QGridLayout(EditFile);
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        spacerItem = new QSpacerItem(331, 49, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(spacerItem, 2, 1, 1, 1);

        GroupButtons = new QGroupBox(EditFile);
        GroupButtons->setObjectName(QStringLiteral("GroupButtons"));
        gridLayout1 = new QGridLayout(GroupButtons);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QStringLiteral("gridLayout1"));
        buttonPrint = new QPushButton(GroupButtons);
        buttonPrint->setObjectName(QStringLiteral("buttonPrint"));
        buttonPrint->setAutoDefault(false);
        buttonPrint->setFlat(false);

        gridLayout1->addWidget(buttonPrint, 0, 1, 1, 1);

        buttonQuit = new QPushButton(GroupButtons);
        buttonQuit->setObjectName(QStringLiteral("buttonQuit"));
        buttonQuit->setAutoDefault(false);
        buttonQuit->setFlat(false);

        gridLayout1->addWidget(buttonQuit, 0, 0, 1, 1);


        gridLayout->addWidget(GroupButtons, 2, 0, 1, 1);

        spacerItem1 = new QSpacerItem(20, 14, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(spacerItem1, 1, 0, 1, 2);

        QTBEditFile = new QTextBrowser(EditFile);
        QTBEditFile->setObjectName(QStringLiteral("QTBEditFile"));
        QTBEditFile->setMinimumSize(QSize(530, 800));
        QFont font;
        font.setFamily(QStringLiteral("Courier New"));
        QTBEditFile->setFont(font);

        gridLayout->addWidget(QTBEditFile, 0, 0, 1, 2);


        retranslateUi(EditFile);

        buttonPrint->setDefault(false);
        buttonQuit->setDefault(true);


        QMetaObject::connectSlotsByName(EditFile);
    } // setupUi

    void retranslateUi(QWidget *EditFile)
    {
        EditFile->setWindowTitle(QApplication::translate("EditFile", "Edit a file", Q_NULLPTR));
        GroupButtons->setTitle(QString());
        buttonPrint->setText(QApplication::translate("EditFile", "Print", Q_NULLPTR));
        buttonQuit->setText(QApplication::translate("EditFile", "Quit", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class EditFile: public Ui_EditFile {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDITFILE_H
