# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'desFenetreChoix.ui'
#
# Created: Thu Mar 27 11:25:15 2014
#      by: PyQt4 UI code generator 4.9.6
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_Choix(object):
    def setupUi(self, Choix):
        Choix.setObjectName(_fromUtf8("Choix"))
        Choix.resize(440, 708)
        self.gridLayout = QtGui.QGridLayout(Choix)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.buttonBox = QtGui.QDialogButtonBox(Choix)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.gridLayout.addWidget(self.buttonBox, 2, 0, 1, 1)
        self.groupBox = QtGui.QGroupBox(Choix)
        self.groupBox.setObjectName(_fromUtf8("groupBox"))
        self.layoutWidget = QtGui.QWidget(self.groupBox)
        self.layoutWidget.setGeometry(QtCore.QRect(40, 310, 311, 269))
        self.layoutWidget.setObjectName(_fromUtf8("layoutWidget"))
        self.gridLayout_2 = QtGui.QGridLayout(self.layoutWidget)
        self.gridLayout_2.setMargin(0)
        self.gridLayout_2.setObjectName(_fromUtf8("gridLayout_2"))
        self.PBMaille = QtGui.QPushButton(self.layoutWidget)
        self.PBMaille.setObjectName(_fromUtf8("PBMaille"))
        self.gridLayout_2.addWidget(self.PBMaille, 1, 0, 1, 1)
        self.PBRatio = QtGui.QPushButton(self.layoutWidget)
        self.PBRatio.setObjectName(_fromUtf8("PBRatio"))
        self.gridLayout_2.addWidget(self.PBRatio, 3, 0, 1, 1)
        self.PBPerf = QtGui.QPushButton(self.layoutWidget)
        self.PBPerf.setObjectName(_fromUtf8("PBPerf"))
        self.gridLayout_2.addWidget(self.PBPerf, 7, 0, 1, 1)
        self.PBGroupeRatio = QtGui.QPushButton(self.layoutWidget)
        self.PBGroupeRatio.setObjectName(_fromUtf8("PBGroupeRatio"))
        self.gridLayout_2.addWidget(self.PBGroupeRatio, 6, 0, 1, 1)
        self.PBGroupe = QtGui.QPushButton(self.layoutWidget)
        self.PBGroupe.setObjectName(_fromUtf8("PBGroupe"))
        self.gridLayout_2.addWidget(self.PBGroupe, 4, 0, 1, 1)
        self.PBTaille = QtGui.QPushButton(self.layoutWidget)
        self.PBTaille.setObjectName(_fromUtf8("PBTaille"))
        self.gridLayout_2.addWidget(self.PBTaille, 2, 0, 1, 1)
        self.PBGroupeTaille = QtGui.QPushButton(self.layoutWidget)
        self.PBGroupeTaille.setObjectName(_fromUtf8("PBGroupeTaille"))
        self.gridLayout_2.addWidget(self.PBGroupeTaille, 5, 0, 1, 1)
        self.layoutWidget1 = QtGui.QWidget(self.groupBox)
        self.layoutWidget1.setGeometry(QtCore.QRect(40, 50, 301, 211))
        self.layoutWidget1.setObjectName(_fromUtf8("layoutWidget1"))
        self.verticalLayout = QtGui.QVBoxLayout(self.layoutWidget1)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.PBMailleur = QtGui.QPushButton(self.layoutWidget1)
        self.PBMailleur.setObjectName(_fromUtf8("PBMailleur"))
        self.verticalLayout.addWidget(self.PBMailleur)
        self.PBMaillage = QtGui.QPushButton(self.layoutWidget1)
        self.PBMaillage.setObjectName(_fromUtf8("PBMaillage"))
        self.verticalLayout.addWidget(self.PBMaillage)
        self.PBMachine = QtGui.QPushButton(self.layoutWidget1)
        self.PBMachine.setObjectName(_fromUtf8("PBMachine"))
        self.verticalLayout.addWidget(self.PBMachine)
        self.PBVersion = QtGui.QPushButton(self.layoutWidget1)
        self.PBVersion.setObjectName(_fromUtf8("PBVersion"))
        self.verticalLayout.addWidget(self.PBVersion)
        self.PBGroupesRef = QtGui.QPushButton(self.layoutWidget1)
        self.PBGroupesRef.setObjectName(_fromUtf8("PBGroupesRef"))
        self.verticalLayout.addWidget(self.PBGroupesRef)
        self.gridLayout.addWidget(self.groupBox, 1, 0, 1, 1)

        self.retranslateUi(Choix)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), Choix.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), Choix.reject)
        QtCore.QMetaObject.connectSlotsByName(Choix)

    def retranslateUi(self, Choix):
        Choix.setWindowTitle(_translate("Choix", "Verification de Maillage", None))
        self.groupBox.setTitle(_translate("Choix", "Tables ", None))
        self.PBMaille.setText(_translate("Choix", "Table Mailles", None))
        self.PBRatio.setText(_translate("Choix", "Table Ratios", None))
        self.PBPerf.setText(_translate("Choix", "Table Performances", None))
        self.PBGroupeRatio.setText(_translate("Choix", "Table Groupe Ratios", None))
        self.PBGroupe.setText(_translate("Choix", "Table Groupes", None))
        self.PBTaille.setText(_translate("Choix", "Table Tailles", None))
        self.PBGroupeTaille.setText(_translate("Choix", "Table Groupe Tailles", None))
        self.PBMailleur.setText(_translate("Choix", "Table Mailleurs", None))
        self.PBMaillage.setText(_translate("Choix", "Table Maillages", None))
        self.PBMachine.setText(_translate("Choix", "Table Machines", None))
        self.PBVersion.setText(_translate("Choix", "Table Versions", None))
        self.PBGroupesRef.setText(_translate("Choix", "Table Groupes Référence", None))


if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    Choix = QtGui.QDialog()
    ui = Ui_Choix()
    ui.setupUi(Choix)
    Choix.show()
    sys.exit(app.exec_())

