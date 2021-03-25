# -*- coding: utf-8 -*-
# Copyright (C) 2014-2021  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

# Form implementation generated from reading ui file 'desFenetreChoix.ui'
#
# Created by: PyQt5 UI code generator 5.6
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_Choix(object):
    def setupUi(self, Choix):
        Choix.setObjectName("Choix")
        Choix.resize(440, 708)
        self.gridLayout = QtWidgets.QGridLayout(Choix)
        self.gridLayout.setObjectName("gridLayout")
        self.buttonBox = QtWidgets.QDialogButtonBox(Choix)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtWidgets.QDialogButtonBox.Cancel|QtWidgets.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.gridLayout.addWidget(self.buttonBox, 2, 0, 1, 1)
        self.groupBox = QtWidgets.QGroupBox(Choix)
        self.groupBox.setObjectName("groupBox")
        self.layoutWidget = QtWidgets.QWidget(self.groupBox)
        self.layoutWidget.setGeometry(QtCore.QRect(40, 310, 311, 269))
        self.layoutWidget.setObjectName("layoutWidget")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.layoutWidget)
        self.gridLayout_2.setContentsMargins(0, 0, 0, 0)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.PBMaille = QtWidgets.QPushButton(self.layoutWidget)
        self.PBMaille.setObjectName("PBMaille")
        self.gridLayout_2.addWidget(self.PBMaille, 1, 0, 1, 1)
        self.PBRatio = QtWidgets.QPushButton(self.layoutWidget)
        self.PBRatio.setObjectName("PBRatio")
        self.gridLayout_2.addWidget(self.PBRatio, 3, 0, 1, 1)
        self.PBPerf = QtWidgets.QPushButton(self.layoutWidget)
        self.PBPerf.setObjectName("PBPerf")
        self.gridLayout_2.addWidget(self.PBPerf, 7, 0, 1, 1)
        self.PBGroupeRatio = QtWidgets.QPushButton(self.layoutWidget)
        self.PBGroupeRatio.setObjectName("PBGroupeRatio")
        self.gridLayout_2.addWidget(self.PBGroupeRatio, 6, 0, 1, 1)
        self.PBGroupe = QtWidgets.QPushButton(self.layoutWidget)
        self.PBGroupe.setObjectName("PBGroupe")
        self.gridLayout_2.addWidget(self.PBGroupe, 4, 0, 1, 1)
        self.PBTaille = QtWidgets.QPushButton(self.layoutWidget)
        self.PBTaille.setObjectName("PBTaille")
        self.gridLayout_2.addWidget(self.PBTaille, 2, 0, 1, 1)
        self.PBGroupeTaille = QtWidgets.QPushButton(self.layoutWidget)
        self.PBGroupeTaille.setObjectName("PBGroupeTaille")
        self.gridLayout_2.addWidget(self.PBGroupeTaille, 5, 0, 1, 1)
        self.layoutWidget1 = QtWidgets.QWidget(self.groupBox)
        self.layoutWidget1.setGeometry(QtCore.QRect(40, 50, 301, 211))
        self.layoutWidget1.setObjectName("layoutWidget1")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.layoutWidget1)
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout.setObjectName("verticalLayout")
        self.PBMailleur = QtWidgets.QPushButton(self.layoutWidget1)
        self.PBMailleur.setObjectName("PBMailleur")
        self.verticalLayout.addWidget(self.PBMailleur)
        self.PBMaillage = QtWidgets.QPushButton(self.layoutWidget1)
        self.PBMaillage.setObjectName("PBMaillage")
        self.verticalLayout.addWidget(self.PBMaillage)
        self.PBMachine = QtWidgets.QPushButton(self.layoutWidget1)
        self.PBMachine.setObjectName("PBMachine")
        self.verticalLayout.addWidget(self.PBMachine)
        self.PBVersion = QtWidgets.QPushButton(self.layoutWidget1)
        self.PBVersion.setObjectName("PBVersion")
        self.verticalLayout.addWidget(self.PBVersion)
        self.PBGroupesRef = QtWidgets.QPushButton(self.layoutWidget1)
        self.PBGroupesRef.setObjectName("PBGroupesRef")
        self.verticalLayout.addWidget(self.PBGroupesRef)
        self.gridLayout.addWidget(self.groupBox, 1, 0, 1, 1)

        self.retranslateUi(Choix)
        self.buttonBox.accepted.connect(Choix.accept)
        self.buttonBox.rejected.connect(Choix.reject)
        QtCore.QMetaObject.connectSlotsByName(Choix)

    def retranslateUi(self, Choix):
        _translate = QtCore.QCoreApplication.translate
        Choix.setWindowTitle(_translate("Choix", "Verification de Maillage"))
        self.groupBox.setTitle(_translate("Choix", "Tables "))
        self.PBMaille.setText(_translate("Choix", "Table Mailles"))
        self.PBRatio.setText(_translate("Choix", "Table Ratios"))
        self.PBPerf.setText(_translate("Choix", "Table Performances"))
        self.PBGroupeRatio.setText(_translate("Choix", "Table Groupe Ratios"))
        self.PBGroupe.setText(_translate("Choix", "Table Groupes"))
        self.PBTaille.setText(_translate("Choix", "Table Tailles"))
        self.PBGroupeTaille.setText(_translate("Choix", "Table Groupe Tailles"))
        self.PBMailleur.setText(_translate("Choix", "Table Mailleurs"))
        self.PBMaillage.setText(_translate("Choix", "Table Maillages"))
        self.PBMachine.setText(_translate("Choix", "Table Machines"))
        self.PBVersion.setText(_translate("Choix", "Table Versions"))
        self.PBGroupesRef.setText(_translate("Choix", "Table Groupes Référence"))


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    Choix = QtWidgets.QDialog()
    ui = Ui_Choix()
    ui.setupUi(Choix)
    Choix.show()
    sys.exit(app.exec_())

