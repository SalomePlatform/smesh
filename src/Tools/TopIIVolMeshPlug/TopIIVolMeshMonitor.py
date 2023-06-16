# -*- coding: utf-8 -*-
# Copyright (C) 2013-2023 CEA/DES, EDF R&D
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

from TopIIVolMeshMonitor_ui import Ui_qdLogger

verbose = True

class TopIIVolMeshMonitor(Ui_qdLogger, QDialog):
  def __init__(self, parent, txt):
    QDialog.__init__(self,parent)
    self.setupUi(self)
    self.resize( QSize(1000,600).expandedTo(self.minimumSizeHint()) )
    self.qpbOK.clicked.connect( self.OnQpbOKClicked )
    # Button OK is disabled until computation is finished
    self.qpbOK.setEnabled(False)
    self.qpbSave.clicked.connect( self.OnQpbSaveClicked )
    self.qpbSave.setToolTip("Save trace in log file")
    self.qpbOK.setToolTip("Close view")
    self.myExecutable=QProcess(self)
    self.myExecutable.readyReadStandardOutput.connect( self.readFromStdOut )
    self.myExecutable.readyReadStandardError.connect( self.readFromStdErr )
    self.myExecutable.finished.connect( self.computationFinished )
    self.myExecutable.errorOccurred.connect( self.computationOnError )
    if os.path.exists(self.parent().outputMesh):
       os.remove(self.parent().outputMesh)
    self.myExecutable.start(txt)
    self.myExecutable.closeWriteChannel()
    self.show()

  def OnQpbOKClicked(self):
    self.close()

  def OnQpbSaveClicked(self):
    outputDirectory=os.path.expanduser("~")
    fn, mask = QFileDialog.getSaveFileName(None,"Save File",outputDirectory)
    if not fn:
      return
    ulfile = os.path.abspath(str(fn))
    try:
      f = open(fn, 'wb')
      f.write(self.qtbLogWindow.toPlainText().encode("utf-8"))
      f.close()
    except IOError as why:
      QMessageBox.critical(self, 'Save File',
                                 'The file <b>%s</b> could not be saved.<br>Reason: %s'%(str(fn), str(why)))

  def readFromStdErr(self):
    a=self.myExecutable.readAllStandardError()
    aa=a.data().decode(errors='ignore')
    self.qtbLogWindow.append(aa)

  def readFromStdOut(self) :
    a=self.myExecutable.readAllStandardOutput()
    aa=a.data().decode(errors='ignore')
    self.qtbLogWindow.append(aa)

  def computationFinished(self):
    self.qpbOK.setEnabled(True)
    if self.myExecutable.exitCode() == 0:
      self.parent().saveOutputMesh()
    else:
      QMessageBox.critical(self, 'Computation failed',
                                 'The computation has failed.<br>Please, check the log message.')

  def computationOnError(self):
    self.qpbOK.setEnabled(True)
    QMessageBox.critical(self, 'Computation failed',
                               'The computation has failed.<br>Please, check the log message.')
