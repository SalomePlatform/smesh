# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'myMain.ui'
#
# Created: Wed Feb 19 11:02:51 2014
#      by: PyQt4 UI code generator 4.7.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_Gestion(object):
    def setupUi(self, Gestion):
        Gestion.setObjectName("Gestion")
        Gestion.resize(400, 525)
        self.gridLayout_4 = QtGui.QGridLayout(Gestion)
        self.gridLayout_4.setObjectName("gridLayout_4")
        self.gridLayout_2 = QtGui.QGridLayout()
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.gridLayout_4.addLayout(self.gridLayout_2, 2, 0, 1, 1)
        spacerItem = QtGui.QSpacerItem(20, 45, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.gridLayout_4.addItem(spacerItem, 1, 0, 1, 1)
        self.gridLayout_3 = QtGui.QGridLayout()
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.BExp = QtGui.QPushButton(Gestion)
        self.BExp.setMinimumSize(QtCore.QSize(281, 41))
        self.BExp.setObjectName("BExp")
        self.gridLayout_3.addWidget(self.BExp, 0, 0, 1, 1)
        self.BImp = QtGui.QPushButton(Gestion)
        self.BImp.setMinimumSize(QtCore.QSize(281, 41))
        self.BImp.setObjectName("BImp")
        self.gridLayout_3.addWidget(self.BImp, 1, 0, 1, 1)
        self.gridLayout_4.addLayout(self.gridLayout_3, 5, 0, 1, 1)
        self.gridLayout = QtGui.QGridLayout()
        self.gridLayout.setObjectName("gridLayout")
        self.BCree = QtGui.QPushButton(Gestion)
        self.BCree.setMinimumSize(QtCore.QSize(281, 41))
        self.BCree.setObjectName("BCree")
        self.gridLayout.addWidget(self.BCree, 0, 0, 1, 1)
        self.BVue = QtGui.QPushButton(Gestion)
        self.BVue.setMinimumSize(QtCore.QSize(281, 41))
        self.BVue.setObjectName("BVue")
        self.gridLayout.addWidget(self.BVue, 1, 0, 1, 1)
        self.gridLayout_4.addLayout(self.gridLayout, 0, 0, 1, 1)
        spacerItem1 = QtGui.QSpacerItem(20, 45, QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Expanding)
        self.gridLayout_4.addItem(spacerItem1, 4, 0, 1, 1)
        self.BStat = QtGui.QPushButton(Gestion)
        self.BStat.setMinimumSize(QtCore.QSize(281, 41))
        self.BStat.setObjectName("BStat")
        self.gridLayout_4.addWidget(self.BStat, 3, 0, 1, 1)

        self.retranslateUi(Gestion)
        QtCore.QMetaObject.connectSlotsByName(Gestion)

    def retranslateUi(self, Gestion):
        Gestion.setWindowTitle(QtGui.QApplication.translate("Gestion", "Gestion de la Database", None, QtGui.QApplication.UnicodeUTF8))
        self.BExp.setText(QtGui.QApplication.translate("Gestion", "Export de la database", None, QtGui.QApplication.UnicodeUTF8))
        self.BImp.setText(QtGui.QApplication.translate("Gestion", "Import d\'une database", None, QtGui.QApplication.UnicodeUTF8))
        self.BCree.setText(QtGui.QApplication.translate("Gestion", "Creation de la database", None, QtGui.QApplication.UnicodeUTF8))
        self.BVue.setText(QtGui.QApplication.translate("Gestion", "Visualiser  la database", None, QtGui.QApplication.UnicodeUTF8))
        self.BStat.setText(QtGui.QApplication.translate("Gestion", "Statistiques", None, QtGui.QApplication.UnicodeUTF8))


if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    Gestion = QtGui.QWidget()
    ui = Ui_Gestion()
    ui.setupUi(Gestion)
    Gestion.show()
    sys.exit(app.exec_())

