# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'nomBase.ui'
#
# Created: Wed Feb 19 11:02:51 2014
#      by: PyQt4 UI code generator 4.7.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_LEDataBaseName(object):
    def setupUi(self, LEDataBaseName):
        LEDataBaseName.setObjectName("LEDataBaseName")
        LEDataBaseName.resize(400, 61)
        self.gridLayout = QtGui.QGridLayout(LEDataBaseName)
        self.gridLayout.setObjectName("gridLayout")
        self.LEBaseName = QtGui.QLineEdit(LEDataBaseName)
        self.LEBaseName.setMinimumSize(QtCore.QSize(341, 41))
        self.LEBaseName.setObjectName("LEBaseName")
        self.gridLayout.addWidget(self.LEBaseName, 0, 0, 1, 1)

        self.retranslateUi(LEDataBaseName)
        QtCore.QMetaObject.connectSlotsByName(LEDataBaseName)

    def retranslateUi(self, LEDataBaseName):
        LEDataBaseName.setWindowTitle(QtGui.QApplication.translate("LEDataBaseName", "Enter DataBase File", None, QtGui.QApplication.UnicodeUTF8))
        self.LEBaseName.setText(QtGui.QApplication.translate("LEDataBaseName", "myMesh.db", None, QtGui.QApplication.UnicodeUTF8))


if __name__ == "__main__":
    import sys
    app = QtGui.QApplication(sys.argv)
    LEDataBaseName = QtGui.QDialog()
    ui = Ui_LEDataBaseName()
    ui.setupUi(LEDataBaseName)
    LEDataBaseName.show()
    sys.exit(app.exec_())

