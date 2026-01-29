# -*- coding: utf-8 -*-
# Copyright (C) 2014-2026  CEA, EDF
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

# Form implementation generated from reading ui file 'myMain.ui'
#
# Created: Wed Feb 19 11:02:51 2014
#      by: PyQt4 UI code generator 4.7.3
#
# WARNING! All changes made in this file will be lost!

import os
if 'SALOME_USE_PYSIDE' in os.environ:
  from PySide2.QtWidgets import QGridLayout, QSpacerItem, QSizePolicy, QPushButton, QApplication, QWidget
  from PySide2.QtCore import QSize, QMetaObject
else:
  from PyQt5.QtGui import QGridLayout, QSpacerItem, QSizePolicy, QPushButton, QApplication, QWidget
  from PyQt5.QtCore import QSize, QMetaObject

class Ui_Gestion(object):
    def setupUi(self, Gestion):
        Gestion.setObjectName("Gestion")
        Gestion.resize(400, 525)
        self.gridLayout_4 = QGridLayout(Gestion)
        self.gridLayout_4.setObjectName("gridLayout_4")
        self.gridLayout_2 = QGridLayout()
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.gridLayout_4.addLayout(self.gridLayout_2, 2, 0, 1, 1)
        spacerItem = QSpacerItem(20, 45, QSizePolicy.Minimum, QSizePolicy.Expanding)
        self.gridLayout_4.addItem(spacerItem, 1, 0, 1, 1)
        self.gridLayout_3 = QGridLayout()
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.BExp = QPushButton(Gestion)
        self.BExp.setMinimumSize(QtCore.QSize(281, 41))
        self.BExp.setObjectName("BExp")
        self.gridLayout_3.addWidget(self.BExp, 0, 0, 1, 1)
        self.BImp = QPushButton(Gestion)
        self.BImp.setMinimumSize(QSize(281, 41))
        self.BImp.setObjectName("BImp")
        self.gridLayout_3.addWidget(self.BImp, 1, 0, 1, 1)
        self.gridLayout_4.addLayout(self.gridLayout_3, 5, 0, 1, 1)
        self.gridLayout = QGridLayout()
        self.gridLayout.setObjectName("gridLayout")
        self.BCree = QPushButton(Gestion)
        self.BCree.setMinimumSize(QSize(281, 41))
        self.BCree.setObjectName("BCree")
        self.gridLayout.addWidget(self.BCree, 0, 0, 1, 1)
        self.BVue = QPushButton(Gestion)
        self.BVue.setMinimumSize(QSize(281, 41))
        self.BVue.setObjectName("BVue")
        self.gridLayout.addWidget(self.BVue, 1, 0, 1, 1)
        self.gridLayout_4.addLayout(self.gridLayout, 0, 0, 1, 1)
        spacerItem1 = QSpacerItem(20, 45, QSizePolicy.Minimum, QSizePolicy.Expanding)
        self.gridLayout_4.addItem(spacerItem1, 4, 0, 1, 1)
        self.BStat = QPushButton(Gestion)
        self.BStat.setMinimumSize(QSize(281, 41))
        self.BStat.setObjectName("BStat")
        self.gridLayout_4.addWidget(self.BStat, 3, 0, 1, 1)

        self.retranslateUi(Gestion)
        QMetaObject.connectSlotsByName(Gestion)

    def retranslateUi(self, Gestion):
        Gestion.setWindowTitle(QApplication.translate("Gestion", "Gestion de la Database", None, QApplication.UnicodeUTF8))
        self.BExp.setText(QApplication.translate("Gestion", "Export de la database", None, QApplication.UnicodeUTF8))
        self.BImp.setText(QApplication.translate("Gestion", "Import d\'une database", None, QApplication.UnicodeUTF8))
        self.BCree.setText(QApplication.translate("Gestion", "Creation de la database", None, QApplication.UnicodeUTF8))
        self.BVue.setText(QApplication.translate("Gestion", "Visualiser  la database", None, QApplication.UnicodeUTF8))
        self.BStat.setText(QApplication.translate("Gestion", "Statistiques", None, QApplication.UnicodeUTF8))


if __name__ == "__main__":
    import sys
    app = QApplication(sys.argv)
    Gestion = QWidget()
    ui = Ui_Gestion()
    ui.setupUi(Gestion)
    Gestion.show()
    sys.exit(app.exec_())

