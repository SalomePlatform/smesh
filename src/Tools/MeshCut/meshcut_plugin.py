# if you already have plugins defined in a salome_plugins.py file, add this file at the end.
# if not, copy this file as ${HOME}/Plugins/salome_plugins.py or ${APPLI}/Plugins/salome_plugins.py

import salome_pluginsmanager

def MeshCut(context):
  # get context study, studyId, salomeGui
  study = context.study
  studyId = context.studyId
  sg = context.sg
  
  import os
  import subprocess
  import tempfile
  from PyQt4 import QtGui
  from PyQt4.QtGui import QFileDialog
  from PyQt4.QtGui import QMessageBox
  from MeshCutDialog import Ui_Dialog
  
  class CutDialog(QtGui.QDialog):
    
    def __init__(self):
      QtGui.QDialog.__init__(self)
      # Set up the user interface from Designer.
      self.ui = Ui_Dialog()
      self.ui.setupUi(self)
      # Connect up the buttons.
      self.connect(self.ui.pb_origMeshFile, QtCore.SIGNAL("clicked()"),
                   self.setInputFile)
      self.connect(self.ui.pb_cutMeshFile, QtCore.SIGNAL("clicked()"),
                   self.setOutputFile)
      self.connect(self.ui.pb_help, QtCore.SIGNAL("clicked()"),
                   self.helpMessage)
      pass
    
    def setInputFile(self):
      fd = QFileDialog(self, "select an existing Med file", self.ui.le_origMeshFile.text(), "MED-Files (*.med);;All Files (*)")
      if fd.exec_():
        infile = fd.selectedFiles()[0]
        self.ui.le_origMeshFile.setText(infile)
        insplit = os.path.splitext(infile.toLocal8Bit().data())
        outfile = insplit[0] + '_cut' + insplit[1]
        self.ui.le_cutMeshFile.setText(outfile)
      pass
    
    def setOutputFile(self):
      fd = QFileDialog(self, "select an output Med file", self.ui.le_cutMeshFile.text(), "MED-Files (*.med);;All Files (*)")
      if fd.exec_():
        self.ui.le_cutMeshFile.setText(fd.selectedFiles()[0])
      pass
    
    def helpMessage(self):
      QMessageBox.about(None, "About MeshCut",
      """
      Cut a tetrahedron mesh by a plane
      ---------------------------------
                 
MeshCut allows to cut a mesh constituted of linear
tetrahedrons by a plane. The tetrahedrons intersected
by the plane are cut and replaced by elements of
various types (tetrahedron, pyramid, pentahedron).

MeshCut is a standalone program, reading and
producing med files. The cutting plane is defined
by a vector normal to the plane and a vertex
belonging to the plane.

Vertices of a tetrahedron are considered as belonging to
the cut plane if their distance to the plane is inferior
to L*T where L is the mean edge size of the tetrahedron
and T the tolerance.
      """)
      pass
    pass
  
  
                     
  window = CutDialog()
  window.ui.dsb_tolerance.setValue(0.01)
  retry = True
  while(retry):
    retry = False
    window.exec_()
    result = window.result()
    if result:
      # dialog accepted
      args = ['MeshCut']
      args += [window.ui.le_origMeshFile.text().toLocal8Bit().data()]
      args += [window.ui.le_cutMeshFile.text().toLocal8Bit().data()]
      args += [window.ui.le_outMeshName.text().toLocal8Bit().data()]
      args += [window.ui.le_groupAbove.text().toLocal8Bit().data()]
      args += [window.ui.le_groupBelow.text().toLocal8Bit().data()]
      args += [str(window.ui.dsb_normX.value())]
      args += [str(window.ui.dsb_normY.value())]
      args += [str(window.ui.dsb_normZ.value())]
      args += [str(window.ui.dsb_vertX.value())]
      args += [str(window.ui.dsb_vertY.value())]
      args += [str(window.ui.dsb_vertZ.value())]
      args += [str(window.ui.dsb_tolerance.value())]
      f= tempfile.NamedTemporaryFile(delete=False)
      fname = f.name
      p = subprocess.Popen(args, stdout=f, stderr=f)
      err = p.wait()
      f.close()
      if err==0:
        os.remove(fname)
      else:
        f = open(fname, 'r')
        m = f.read()
        msgBox = QMessageBox()
        msgBox.setText("Parameters are not OK")
        msgBox.setInformativeText("Do you want to retry ?")
        msgBox.setDetailedText(m)
        msgBox.setStandardButtons(QMessageBox.Retry | QMessageBox.Cancel)
        msgBox.setDefaultButton(QMessageBox.Retry)
        ret = msgBox.exec_()
        if ret == QMessageBox.Retry:
          retry = True
        pass
      pass
    pass
  pass

# register the function in the plugin manager
salome_pluginsmanager.AddFunction('MeshCut', 'Cut a tetrahedron mesh by a plane', MeshCut)

