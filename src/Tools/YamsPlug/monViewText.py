# -*- coding: utf-8 -*-
# Copyright (C) 2007-2021  EDF R&D
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

import os
import sys
import string
import types
import tempfile
import traceback
import pprint as PP #pretty print

from qtsalome import *

# Import des panels

from ViewText_ui import Ui_ViewExe

verbose = True

force = os.getenv("FORCE_DISTENE_LICENSE_FILE")
if force != None:
    os.environ["DISTENE_LICENSE_FILE"] = force
    os.environ["DLIM8VAR"] = "NOTHING"

class MonViewText(Ui_ViewExe, QDialog):
    """
    Classe permettant la visualisation de texte
    """
    def __init__(self, parent, txt):
        QDialog.__init__(self,parent)
        self.setupUi(self)
        self.resize( QSize(1000,600).expandedTo(self.minimumSizeHint()) )
        self.PB_Ok.clicked.connect( self.theClose )
        # Button OK is disabled until computation is finished
        self.PB_Ok.setEnabled(False)
        # Button cancel allows to kill the computation
        # It is disabled when the computation is finished
        self.PB_Cancel.clicked.connect( self.cancelComputation )
        self.PB_Cancel.setToolTip("Cancel computation")
        self.PB_Save.clicked.connect( self.saveFile )
        self.PB_Save.setToolTip("Save trace in log file")
        self.PB_Ok.setToolTip("Close view")
        self.monExe=QProcess(self)

        self.monExe.readyReadStandardOutput.connect( self.readFromStdOut )
        self.monExe.readyReadStandardError.connect( self.readFromStdErr )
        self.monExe.finished.connect( self.finished )
        self.monExe.errorOccurred.connect( self.errorOccured )

        if os.path.exists(self.parent().fichierOut):
            os.remove(self.parent().fichierOut)

        self.monExe.start(txt)
        self.monExe.closeWriteChannel()
        self.hasBeenCanceled = False
        self.anErrorOccured = False
        self.show()

    def make_executable(self, path):
        mode = os.stat(path).st_mode
        mode |= (mode & 0o444) >> 2    # copy R bits to X
        os.chmod(path, mode)

    def saveFile(self):
        #recuperation du nom du fichier
        savedir=os.path.expanduser("~")
        fn, mask = QFileDialog.getSaveFileName(None,"Save File",savedir)
        if not fn: return
        ulfile = os.path.abspath(str(fn))
        try:
            f = open(fn, 'wb')
            f.write(self.TB_Exe.toPlainText().encode("utf-8"))
            f.close()
        except IOError as why:
            QMessageBox.critical(self, 'Save File',
                 'The file <b>%s</b> could not be saved.<br>Reason: %s'%(str(fn), str(why)))

    def readFromStdErr(self):
        a=self.monExe.readAllStandardError()
        aa=a.data().decode(errors='ignore')
        self.TB_Exe.append(aa)

    def readFromStdOut(self) :
        a=self.monExe.readAllStandardOutput()
        aa=a.data().decode(errors='ignore')
        self.TB_Exe.append(aa)

    def finished(self):
        self.PB_Ok.setEnabled(True)
        self.PB_Cancel.setEnabled(False)
        exit_code = self.monExe.exitCode()
        if exit_code == 0 and not self.anErrorOccured:
            self.parent().enregistreResultat()
        elif not self.hasBeenCanceled:
            if os.path.exists(self.parent().fichierOut):
                self.parent().enregistreResultat()
                QMessageBox.critical(self, 'Computation ended in error',
                  'A new mesh has been generated but with some errors.'+
                  '<br>Please, check the log message.')
            else:
                QMessageBox.critical(self, 'Computation failed',
                  'The computation has failed.<br>Please, check the log message.')
        pass

    def errorOccured(self):
        # for instance if the executable is not found
        self.anErrorOccured = True
        self.finished()

    def cancelComputation(self):
        self.hasBeenCanceled = True
        self.monExe.kill()

    def theClose(self):
        self.close()
