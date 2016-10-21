# -*- coding: utf-8 -*-

# Copyright (C) 2006-2016  EDF R&D
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

# if you already have plugins defined in a salome_plugins.py file, add this file at the end.
# if not, copy this file as ${HOME}/Plugins/smesh_plugins.py or ${APPLI}/Plugins/smesh_plugins.py

import sys, traceback
import math
from blocFissure import gmu

def fissureGeneraleDlg(context):
  # get context study, studyId, salomeGui
  study = context.study
  studyId = context.studyId
  sg = context.sg

  import os
  #import subprocess
  #import tempfile
  from PyQt5 import QtCore
  from PyQt5 import QtWidgets
  from PyQt5 import QtGui
  from PyQt5.QtWidgets import QFileDialog
  from PyQt5.QtWidgets import QMessageBox
  from PyQt5.QtGui import QPalette
  from PyQt5.QtGui import QColor
  from fissureGenerale_ui import Ui_Dialog

  class fissureGeneraleDialog(QtWidgets.QDialog):

    def __init__(self):
      print "__init__"
      QtWidgets.QDialog.__init__(self)
      # Set up the user interface from Designer.
      self.ui = Ui_Dialog()
      self.ui.setupUi(self)

      self.blackPalette = self.ui.dsb_influence.palette()
      self.redPalette = QPalette()
      self.redPalette.setColor(QPalette.Text, QColor(255,0,0))
      self.NOK = False

      self.initDefaut()
      self.initDialog(self.defaut)
      self.ui.lb_calcul.hide()

      # Connect up the buttons.

      self.ui.pb_exemple.clicked.connect(self.genereExemples)
      self.ui.pb_valPrec.clicked.connect(self.readValPrec)
      self.ui.pb_reset.clicked.connect(self.resetVal)
      self.ui.pb_recharger.clicked.connect(self.recharger)
      self.ui.pb_sauver.clicked.connect(self.sauver)
      self.ui.pb_maillage.clicked.connect(self.selectMaillage)
      self.ui.pb_facefiss.clicked.connect(self.selectFacefiss)
      self.ui.pb_reptrav.clicked.connect(self.selectReptrav)
      self.ui.bb_OkCancel.accepted.disconnect(self.accept)
      self.ui.bb_OkCancel.accepted.connect(self.execute)

    def initDefaut(self):
      self.defaut = dict(
        nomCas            = 'angleCube',
        maillageSain      = os.path.join(gmu.pathBloc, 'materielCasTests/CubeAngle.med'),
        brepFaceFissure   = os.path.join(gmu.pathBloc, "materielCasTests/CubeAngleFiss.brep"),
        edgeFissIds       = [4],
        lgInfluence       = 20,
        meshBrep          = (5,10),
        rayonPipe         = 5,
        lenSegPipe        = 2.5,
        nbSegRad          = 5,
        nbSegCercle       = 32,
        areteFaceFissure  = 10,
        areteVives        = 0,
        reptrav           = '.',
        nomres            = 'casStandard_fissure.med',
        verbosite         = 0)


    def initDialog(self, dico):
      self.ui.le_maillage.setText(dico['maillageSain'])
      self.ui.le_facefiss.setText(dico['brepFaceFissure'])
      self.ui.le_fondfiss.setText(str(dico['edgeFissIds']))
      self.ui.dsb_influence.setValue(dico['lgInfluence'])
      self.ui.dsb_meshBrepMin.setValue(dico['meshBrep'][0])
      self.ui.dsb_meshBrepMax.setValue(dico['meshBrep'][1])
      self.ui.dsb_rayonPipe.setValue(dico['rayonPipe'])
      self.ui.dsb_lenSegPipe.setValue(dico['lenSegPipe'])
      self.ui.sb_couronnes.setValue(dico['nbSegRad'])
      self.ui.sb_secteurs.setValue(dico['nbSegCercle'])
      self.ui.dsb_areteFaceFissure.setValue(dico['areteFaceFissure'])
      if dico.has_key('aretesVives'):
        self.ui.dsb_aretesVives.setValue(dico['aretesVives'])
      else:
        self.ui.dsb_aretesVives.setValue(0)
      self.ui.le_reptrav.setText(os.path.abspath(dico['reptrav']))
      self.ui.le_nomres.setText(os.path.split(dico['nomres'])[1])
      self.ui.cb_log.setCurrentIndex(dico['verbosite'])
      incomplet = self.testval(dico)
      pass

    def testval(self, dico):
      incomplet = False
      if not os.path.lexists(dico['maillageSain']):
        self.ui.le_maillage.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.le_maillage.setPalette(self.blackPalette)
      if not os.path.lexists(dico['brepFaceFissure']):
        self.ui.le_facefiss.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.le_facefiss.setPalette(self.blackPalette)
      edgeFissIdsOK=True
      try:
        l = dico['edgeFissIds']
        for i in l:
          if not isinstance(i, int):
            print"not isinstance(i, int)"
            incomplet = True
            edgeFissIdsOK=False
            break
      except:
        print "except eval"
        incomplet = True
        edgeFissIdsOK=False
      if edgeFissIdsOK:
        self.ui.le_fondfiss.setPalette(self.blackPalette)
      else:
        self.ui.le_fondfiss.setPalette(self.redPalette)
      if dico['meshBrep'][0] == 0:
        self.ui.dsb_meshBrepMin.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_meshBrepMin.setPalette(self.blackPalette)
      if dico['meshBrep'][1] == 0:
        self.ui.dsb_meshBrepMax.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_meshBrepMax.setPalette(self.blackPalette)
      if dico['rayonPipe'] == 0:
        self.ui.dsb_rayonPipe.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_rayonPipe.setPalette(self.blackPalette)
      if dico['lenSegPipe'] == 0:
        self.ui.dsb_lenSegPipe.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_lenSegPipe.setPalette(self.blackPalette)
      if dico['areteFaceFissure'] == 0:
        self.ui.dsb_areteFaceFissure.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_areteFaceFissure.setPalette(self.blackPalette)

      print "incomplet: ", incomplet
      return incomplet

    def fileDefault(self):
      filedef = os.path.expanduser("~/.config/salome/dialogFissureGenerale.dic")
      print filedef
      return filedef

    def writeDefault(self, dico):
      filedef = self.fileDefault()
      f = open(filedef, 'w')
      f.write(str(dico))
      f.close()

    def genereExemples(self):
      maillageSain      = os.path.join(gmu.pathBloc, 'materielCasTests/CubeAngle.med')
      brepFaceFissure   = os.path.join(gmu.pathBloc, "materielCasTests/CubeAngleFiss.brep")
      if (os.path.exists(maillageSain) and os.path.exists(brepFaceFissure)):
        self.initDialog(self.defaut)
      else:
        self.ui.lb_calcul.setText("--- Generation exemples en cours ---")
        self.ui.lb_calcul.show()
        from blocFissure.materielCasTests import genereMateriel
        self.ui.lb_calcul.hide()
        self.initDialog(self.defaut)

    def readValPrec(self):
      filedef = self.fileDefault()
      if os.path.exists(filedef):
        f = open(filedef, 'r')
        txt = f.read()
        dico = eval(txt)
        print dico
        self.initDialog(dico)

    def resetVal(self):
      #self.initDefaut()
      self.initDialog(self.defaut)

    def setLogVerbosity(self, logfile):
      from blocFissure.gmu import initLog # le mode de log s'initialise une seule fois
      print "setLogVerbosity"
      index = self.ui.cb_log.currentIndex()
      print index
      if index == 0:
        initLog.setRelease(logfile)
      elif index == 1:
        initLog.setVerbose(logfile)
      elif index == 2:
        initLog.setDebug(logfile)


    def sauver(self):
      print "sauver"
      fileDiag = QFileDialog(self)
      fileDiag.setFileMode(QFileDialog.AnyFile)
      fileDiag.setNameFilter("Parametres *.dic (*.dic)")
      fileDiag.setViewMode(QFileDialog.List)
      if fileDiag.exec_() :
        fileNames = fileDiag.selectedFiles()
        print fileNames
        filedef = fileNames[0]
        if filedef[-4:] not in ['.dic']:
          filedef += '.dic'
        dico = self.creeDico()
        f = open(filedef, 'w')
        f.write(str(dico))
        f.close()

    def recharger(self):
      print "recharger"
      fileDiag = QFileDialog(self)
      fileDiag.setFileMode(QFileDialog.ExistingFile)
      fileDiag.setNameFilter("Parametres *.dic (*.dic)")
      fileDiag.setViewMode(QFileDialog.Detail)
      if fileDiag.exec_() :
        fileNames = fileDiag.selectedFiles()
        filedef = fileNames[0]
        print filedef
        if os.path.exists(filedef):
          f = open(filedef, 'r')
          txt = f.read()
          dico = eval(txt)
          print dico
          self.initDialog(dico)

    def selectMaillage(self):
      fileDiag = QFileDialog(self)
      fileDiag.setFileMode(QFileDialog.ExistingFile)
      fileDiag.setNameFilter("Maillage *.med (*.med)")
      fileDiag.setViewMode(QFileDialog.Detail)
      if fileDiag.exec_() :
        fileNames = fileDiag.selectedFiles()
        filedef = fileNames[0]
        print filedef
        self.ui.le_maillage.setText(filedef)

    def selectFacefiss(self):
      fileDiag = QFileDialog(self)
      fileDiag.setFileMode(QFileDialog.ExistingFile)
      fileDiag.setNameFilter("Face fissure *.brep (*.brep)")
      fileDiag.setViewMode(QFileDialog.Detail)
      if fileDiag.exec_() :
        fileNames = fileDiag.selectedFiles()
        filedef = fileNames[0]
        print filedef
        self.ui.le_facefiss.setText(filedef)

    def selectReptrav(self):
      fileDiag = QFileDialog(self)
      fileDiag.setFileMode(QFileDialog.Directory)
      fileDiag.setViewMode(QFileDialog.Detail)
      fileDiag.setDirectory(self.ui.le_reptrav.text())
      if fileDiag.exec_() :
        fileNames = fileDiag.selectedFiles()
        reptrav = str(fileNames[0])
        print "reptrav ", reptrav
        self.ui.le_reptrav.setText(os.path.abspath(reptrav))


    def selectNomres(self):
      fileDiag = QFileDialog(self)
      fileDiag.setFileMode(QFileDialog.AnyFile)
      fileDiag.setViewMode(QFileDialog.Detail)
      nomres=str(os.path.split(str(self.ui.le_nomres.text()))[1])
      fileDiag.setDirectory(self.ui.le_reptrav.text())
      fileDiag.selectFile(nomres)
      fileDiag.setNameFilter("Maillage *.med (*.med)")
      self.ui.le_nomres.setText(nomres)
      if fileDiag.exec_() :
        fileNames = fileDiag.selectedFiles()
        tempnom = os.path.split(str(fileNames[0]))[1]
        print "nomres ", tempnom
        self.ui.le_nomres.setText(tempnom)
      else:
        self.ui.le_nomres.setText(nomres)

    def creeDico(self):
      dico = dict(
                  maillageSain     = str(self.ui.le_maillage.text()),
                  brepFaceFissure  = str(self.ui.le_facefiss.text()),
                  edgeFissIds      = eval(str(self.ui.le_fondfiss.text())),
                  lgInfluence      = self.ui.dsb_influence.value(),
                  meshBrep         = [self.ui.dsb_meshBrepMin.value(),self.ui.dsb_meshBrepMax.value()],
                  rayonPipe        = self.ui.dsb_rayonPipe.value(),
                  lenSegPipe       = self.ui.dsb_lenSegPipe.value(),
                  nbSegRad         = self.ui.sb_couronnes.value(),
                  nbSegCercle      = self.ui.sb_secteurs.value(),
                  areteFaceFissure = self.ui.dsb_areteFaceFissure.value(),
                  aretesVives      = self.ui.dsb_aretesVives.value(),
                  reptrav          = str(self.ui.le_reptrav.text()),
                  nomres           = str(self.ui.le_nomres.text()),
                  verbosite        = self.ui.cb_log.currentIndex()
                  )
      print dico
      return dico

    def checkValues(self):
      return self.NOK

    def execute(self):
      print "execute"
      dico = self.creeDico()
      NOK = self.testval(dico)
      if not(NOK):
        self.writeDefault(dico)
        self.ui.lb_calcul.setText("--- Calcul en cours ---")
        self.ui.lb_calcul.show()
        logfile=os.path.join(dico['reptrav'], dico['nomres']+".log")
        self.setLogVerbosity(logfile)
        from blocFissure.gmu import geomsmesh               # après intialisation log dans setLogVerbosity
        from blocFissure.gmu.casStandard import casStandard # après intialisation log dans setLogVerbosity
        from blocFissure.gmu.fissError import fissError
        try:
          execInstance = casStandard(dico)
        except fissError as erreur:
          print '-'*60
          print type(erreur)
          print '-'*60
          print erreur.msg
          print '-'*60
          for ligne in erreur.pile:
            print repr(ligne)
          print '-'*60
          texte = erreur.msg
#           texte += +"<br>" +'-'*60 +"<br>"
#           for ligne in erreur.pile:
#             texte += repr(ligne) +"<br>"
          mbox = QMessageBox(self)
          mbox.setWindowTitle("erreur blocFissure")
          mbox.setText(str(texte))
          mbox.exec_()
#        except Exception as erreur:
#          print "exception non répertoriée"
      self.NOK = NOK
      self.ui.lb_calcul.hide()
      #self.accept()

    pass

# ----------------------------------------------------------------------------

  print "main"
  window = fissureGeneraleDialog()
  retry = True
  while(retry):
    retry = False
    window.exec_()
    result = window.result()
    if result:
      # dialog accepted
      print "dialog accepted, check"
      retry = window.checkValues()
    else:
      print "dialog rejected, exit"
  pass

