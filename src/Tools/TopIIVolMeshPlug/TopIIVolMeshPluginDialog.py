# -*- coding: utf-8 -*-
# Copyright (C) 2013-2026 CEA/DES, EDF
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


import os, subprocess
import random
import getpass
import time
import pathlib

# set seed
from datetime import datetime
# see https://docs.python.org/3.13/library/random.html#random.seed
random.seed(datetime.now().timestamp())

import platform
import tempfile
from TopIIVolMeshPluginDialog_ui import Ui_TopIIVolMeshMainFrame
from TopIIVolMeshMonitor import TopIIVolMeshMonitor
from qtsalome import *

verbose = True

class TopIIVolMeshPluginDialog(Ui_TopIIVolMeshMainFrame,QWidget):
  """
  """
  def __init__(self):
    QWidget.__init__(self)
    self.setupUi(self)
    self.qpbHelp.clicked.connect(self.OnQpbHelpClicked)
    self.qpbCompute.clicked.connect(self.OnQpbComputeClicked)
    self.qpbMeshFile.clicked.connect(self.OnQpbMeshFileClicked)
    self.qpbMeshFile.setToolTip("Select input DEM file")
    self.qpbClose.clicked.connect(self.OnQpbCloseClicked)
    self.qrbDist.clicked.connect(self.OnqrbDistClicked)
    self.qrbPar.clicked.connect(self.OnqrbParClicked)
    self.qrbSeq.clicked.connect(self.OnqrbSeqClicked)
    self.SALOME_TMP_DIR = None
    try:
      self.qleTmpDir.setText(os.path.join('/tmp',getpass.getuser(),'top-ii-vol'))
    except:
      self.qleTmpDir.setText('/tmp')
    self.outputMesh = ''

  def OnQpbHelpClicked(self):
    import SalomePyQt
    sgPyQt = SalomePyQt.SalomePyQt()
    try:
      mydir=os.environ["SMESH_ROOT_DIR"]
    except Exception:
      QMessageBox.warning(self, "Help", "Help unavailable $SMESH_ROOT_DIR not found")
      return

    myDoc=mydir + "/share/doc/salome/gui/SMESH/TopIIVolMesh/index.html"
    sgPyQt.helpContext(myDoc,"")

  def OnQpbMeshFileClicked(self):
    fd = QFileDialog(self, "select an existing Mesh file", self.qleMeshFile.text(), "Mesh-Files (*.xyz);;All Files (*)")
    if fd.exec_():
      infile = fd.selectedFiles()[0]
      self.qleMeshFile.setText(infile)

  def OnQpbComputeClicked(self):
    if self.qleMeshFile.text() == '':
      QMessageBox.critical(self, "Mesh", "select an input mesh")
      return
    inputMesh = self.qleMeshFile.text()
    # retrieve x,y,z and depth parameters
    xPoints = self.qsbXPoints.value()
    yPoints = self.qsbYPoints.value()
    zPoints = self.qsbZPoints.value()
    depth   = self.qsbDepth.value()
    nProcs  = self.qsbNBprocs.value()
    if not self.qrbDist.isChecked():
      if nProcs == 1:
        shellCmd = "topIIvol_Mesher"
      else:
        shellCmd = "mpirun -np {} topIIvol_ParMesher".format(nProcs)
      shellCmd+= " --xpoints " + str(xPoints)
      shellCmd+= " --ypoints " + str(yPoints)
      shellCmd+= " --zpoints " + str(zPoints)
      shellCmd+= " --depth   " + str(depth)
      shellCmd+= " --in " + inputMesh
    else:
      xParts = self.qsbXParts.value()
      yParts = self.qsbYParts.value()
      zParts = self.qsbZParts.value()
      shellCmd = "mpirun -np {} topIIvol_DistMesher".format(nProcs)
      shellCmd+= " --xpoints " + str(xPoints)
      shellCmd+= " --ypoints " + str(yPoints)
      shellCmd+= " --zpoints " + str(zPoints)
      shellCmd+= " --depth   " + str(depth)
      shellCmd+= " --partition_x " + str(xParts)
      shellCmd+= " --partition_y " + str(yParts)
      shellCmd+= " --partition_z " + str(zParts)
      shellCmd+= " --in " + inputMesh
    if platform.system()=="Windows" :
      self.SALOME_TMP_DIR = os.getenv("SALOME_TMP_DIR")
    else:
      self.SALOME_TMP_DIR = os.path.join(self.qleTmpDir.text(), time.strftime("%Y-%m-%d-%H-%M-%S"))
      pathlib.Path(self.SALOME_TMP_DIR).mkdir(parents=True, exist_ok=True)
    self.outputMesh= os.path.join(self.SALOME_TMP_DIR, inputMesh.split('/').pop().replace('.xyz','.mesh'))
    shellCmd+= " --out " + self.outputMesh
    os.chdir(self.SALOME_TMP_DIR)
    print("INFO: ", shellCmd)
    myMonitorView=TopIIVolMeshMonitor(self, shellCmd)

  def OnqrbDistClicked(self):
    state = self.qrbDist.isChecked()
    self.qgbDist.setEnabled(state)
    self.qsbNBprocs.setEnabled(state)
    self.qlbNBprocs.setEnabled(state)

  def OnqrbParClicked(self):
    state = self.qrbPar.isChecked()
    self.qgbDist.setEnabled(not state)
    self.qsbNBprocs.setEnabled(state)
    self.qlbNBprocs.setEnabled(state)

  def OnqrbSeqClicked(self):
    state = self.qrbSeq.isChecked()
    if state:
      self.qsbNBprocs.setValue(1)
    self.qgbDist.setEnabled(not state)
    self.qsbNBprocs.setEnabled(not state)
    self.qlbNBprocs.setEnabled(not state)
    
  def OnQpbCloseClicked(self):
    self.close()

  def saveOutputMesh(self):
    if not self.qcbDisplayMesh.isChecked():
      return True
    from salome.kernel import salome
    from salome.kernel import SMESH
    from salome.kernel import SALOMEDS
    from salome.smesh import smeshBuilder
    smesh = smeshBuilder.New()
    self.outputMesh.split('/')
    for mesh in pathlib.Path(self.SALOME_TMP_DIR).glob('*.mesh'):
      (outputMesh, status) = smesh.CreateMeshesFromGMF(os.path.join(self.SALOME_TMP_DIR, mesh))
    if salome.sg.hasDesktop():
        salome.sg.updateObjBrowser()
    return True

__instance = None

def getInstance():
  """
  This function returns a singleton instance of the plugin dialog.
  It is mandatory in order to call show without a parent ...
  """
  global __instance
  if __instance is None:
    __instance = TopIIVolMeshPluginDialog()
  return __instance
