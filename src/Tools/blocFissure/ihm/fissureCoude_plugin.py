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

def fissureCoudeDlg(context):
  # get context study, studyId, salomeGui
  study = context.study
  studyId = context.studyId
  sg = context.sg
  
  import os
  #import subprocess
  #import tempfile
  from qtsalome import QFileDialog, QMessageBox, QPalette, QColor, QDialog
  from fissureCoude_ui import Ui_Dialog
  
  class fissureCoudeDialog(QDialog):
    
    def __init__(self):
      QDialog.__init__(self)
      # Set up the user interface from Designer.
      self.ui = Ui_Dialog()
      self.ui.setupUi(self)
      
      self.blackPalette = self.ui.dsb_angle.palette()
      self.redPalette = QPalette()
      self.redPalette.setColor(QPalette.Text, QColor(255,0,0))
      self.NOK = False
      
      self.initDefaut()
      self.initDialog(self.defaut)
      self.ui.dsb_angle.setSpecialValueText("saisie_obligatoire")
      self.ui.dsb_rCintr.setSpecialValueText("saisie_obligatoire")
      self.ui.dsb_lTubeP1.setSpecialValueText("saisie_obligatoire")
      self.ui.dsb_lTubeP2.setSpecialValueText("saisie_obligatoire")
      self.ui.dsb_epais.setSpecialValueText("saisie_obligatoire")
      self.ui.dsb_dext.setSpecialValueText("saisie_obligatoire")
      self.ui.dsb_profondeur.setSpecialValueText("saisie_obligatoire")
      self.ui.dsb_longueur.setSpecialValueText("saisie_obligatoire")
      self.ui.dsb_azimut.setSpecialValueText("saisie_obligatoire")
      self.ui.dsb_orientation.setSpecialValueText("saisie_obligatoire")
      self.ui.dsb_posiAngul.setSpecialValueText("saisie_obligatoire")
      self.ui.dsb_absCurv.setSpecialValueText("saisie_obligatoire")
      self.ui.sb_nbTranches.setSpecialValueText("saisie_obligatoire")
      self.ui.sb_nbCouronne.setSpecialValueText("saisie_obligatoire")
      self.ui.sb_nbSecteur.setSpecialValueText("saisie_obligatoire")
      self.ui.dsb_aretesFaceFissure.setSpecialValueText("automatique")
      self.ui.dsb_influence.setSpecialValueText("automatique")
      self.ui.lb_calcul.hide()
      
      # Connect up the buttons.
      self.ui.pb_valPrec.clicked.connect(self.readValPrec)
      self.ui.pb_reset.clicked.connect(self.resetVal)
      self.ui.pb_recharger.clicked.connect(self.recharger)
      self.ui.pb_sauver.clicked.connect(self.sauver)
      self.ui.buttonBox.accepted.disconnect(self.accept)
      self.ui.buttonBox.accepted.connect(self.execute)
    
    def initDefaut(self):
      self.defaut = dict(
        angle             = -181.0,
        rCintr            = 0.0,
        lTubeP1           = 0.0,
        lTubeP2           = 0.0,
        epais             = 0.0,
        dext              = 0.0,
        profondeur        = 0.0,
        longueur          = 0.0,
        azimut            = -181.0,
        orientation       = -1.0,
        posiAngul         = -181.0,
        absCurv           = 0.0,
        nbTranches        = 7,
        nbCouronnes       = 1,
        nbSecteurs        = 3,
        cbOptDiscrSain    = False,
        cbOptDiscrFiss    = False,
        rbPosiAngul       = True,
        rbFissExt         = True,
        cbForceEllipse    = False,
        nbAxeTubeP1       = 15,
        nbAxeTubeP2       = 15,
        nbAxeCoude        = 10,
        nbCirconf         = 20,
        nbEpaisseur       = 3,
        rayonTore         = 2.0,
        aretesFaceFissure = 0.0,
        influence         = 0.0,
        )
      
    def initDialog(self, dico):
      self.ui.dsb_angle.setValue(dico['angle'])
      self.ui.dsb_rCintr.setValue(dico['rCintr'])
      self.ui.dsb_lTubeP1.setValue(dico['lTubeP1'])
      self.ui.dsb_lTubeP2.setValue(dico['lTubeP2'])
      self.ui.dsb_epais.setValue(dico['epais'])
      self.ui.dsb_dext.setValue(dico['dext'])
      self.ui.dsb_profondeur.setValue(dico['profondeur'])
      self.ui.dsb_longueur.setValue(dico['longueur'])
      self.ui.dsb_azimut.setValue(dico['azimut'])
      self.ui.dsb_orientation.setValue(dico['orientation'])
      self.ui.dsb_posiAngul.setValue(dico['posiAngul'])
      self.ui.dsb_absCurv.setValue(dico['absCurv'])
      self.ui.sb_nbTranches.setValue(dico['nbTranches'])
      self.ui.sb_nbCouronne.setValue(dico['nbCouronnes'])
      self.ui.sb_nbSecteur.setValue(dico['nbSecteurs'])
      self.ui.dsb_aretesFaceFissure.setValue(dico['aretesFaceFissure'])
      self.ui.dsb_influence.setValue(dico['influence'])
      self.ui.sb_nbAxeTubeP1.setValue(dico['nbAxeTubeP1'])
      self.ui.sb_nbAxeTubeP2.setValue(dico['nbAxeTubeP2'])
      self.ui.sb_nbAxeCoude.setValue(dico['nbAxeCoude'])
      self.ui.sb_nbCirconf.setValue(dico['nbCirconf'])
      self.ui.sb_nbEpaisseur.setValue(dico['nbEpaisseur'])
      self.ui.dsb_rayonTore.setValue(dico['rayonTore'])
      #self.ui.cb_optDiscrSain.setChecked(False)
      #self.ui.gb_discrSain.setShown(False)
      self.ui.cb_optDiscrSain.setChecked(not(dico['cbOptDiscrSain']))
      self.ui.cb_optDiscrSain.click()
      self.ui.cb_optDiscrFiss.setChecked(not(dico['cbOptDiscrFiss']))
      self.ui.cb_optDiscrFiss.click()
      if dico['rbPosiAngul']:
        self.ui.dsb_absCurv.setEnabled(False)
        self.ui.dsb_posiAngul.setEnabled(True)
        self.ui.rb_posiAngul.setChecked(True)
        #self.ui.rb_posiAngul.click()
      else:
        self.ui.dsb_absCurv.setEnabled(True)
        self.ui.dsb_posiAngul.setEnabled(False)
        self.ui.rb_absCurv.setChecked(True)
        #self.ui.rb_absCurv.click()
      self.ui.rb_fissExt.setChecked(dico['rbFissExt'])
      self.ui.cb_forceEllipse.setChecked(dico['cbForceEllipse'])
      incomplet = self.testval(dico)
      pass
    
    def testval(self, dico):
      incomplet = False
      if dico['angle'] < -180.0:
        self.ui.dsb_angle.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_angle.setPalette(self.blackPalette)
        
      if dico['rCintr'] == 0.0:
        self.ui.dsb_rCintr.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_rCintr.setPalette(self.blackPalette)
        
      if dico['lTubeP1'] == 0.0:  
        self.ui.dsb_lTubeP1.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_lTubeP1.setPalette(self.blackPalette)
        
      if dico['lTubeP2'] == 0.0:  
        self.ui.dsb_lTubeP2.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_lTubeP2.setPalette(self.blackPalette)
        
      if dico['epais'] == 0.0:  
        self.ui.dsb_epais.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_epais.setPalette(self.blackPalette)
        
      if dico['dext'] == 0.0:  
        self.ui.dsb_dext.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_dext.setPalette(self.blackPalette)
        
      if dico['profondeur'] == 0.0:  
        self.ui.dsb_profondeur.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_profondeur.setPalette(self.blackPalette)
        
      if dico['longueur'] == 0.0:  
        self.ui.dsb_longueur.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_longueur.setPalette(self.blackPalette)
        
      if dico['azimut'] < -180.0:  
        self.ui.dsb_azimut.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_azimut.setPalette(self.blackPalette)
        
      if dico['orientation'] < 0.0:  
        self.ui.dsb_orientation.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_orientation.setPalette(self.blackPalette)
        
      if dico['posiAngul'] < -180.0 and dico['rbPosiAngul'] == True:  
        self.ui.dsb_posiAngul.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_posiAngul.setPalette(self.blackPalette)
        
      if dico['absCurv'] == 0.0 and dico['rbPosiAngul'] == False:  
        self.ui.dsb_absCurv.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.dsb_absCurv.setPalette(self.blackPalette)
        
      if dico['nbTranches'] == 7:  
        self.ui.sb_nbTranches.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.sb_nbTranches.setPalette(self.blackPalette)
        
      if dico['nbCouronnes'] == 1:  
        self.ui.sb_nbCouronne.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.sb_nbCouronne.setPalette(self.blackPalette)
        
      if dico['nbSecteurs'] == 3:  
        self.ui.sb_nbSecteur.setPalette(self.redPalette)
        incomplet = True
      else:
        self.ui.sb_nbSecteur.setPalette(self.blackPalette)
        
      print "incomplet: ", incomplet
      return incomplet
    
    def fileDefault(self):
      filedef = os.path.expanduser("~/.config/salome/dialogFissureCoude.dic")
      print filedef
      return filedef
    
    def writeDefault(self, dico):
      filedef = self.fileDefault()
      f = open(filedef, 'w')
      f.write(str(dico))
      f.close()
    
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
      
    def sauver(self):
      print "sauver"
      fileDiag = QFileDialog(self)
      fileDiag.setFileMode(QFileDialog.AnyFile)
      fileDiag.setNameFilter("Parametres *.dic (*.dic)")
      fileDiag.setViewMode(QFileDialog.List)
      if fileDiag.exec_() :
        fileNames = fileDiag.selectedFiles()
        filedef = fileNames[0]
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
         
    def creeDico(self):
      dico = dict(
        angle             = self.ui.dsb_angle.value(),
        rCintr            = self.ui.dsb_rCintr.value(),
        lTubeP1           = self.ui.dsb_lTubeP1.value(),
        lTubeP2           = self.ui.dsb_lTubeP2.value(),
        epais             = self.ui.dsb_epais.value(),
        dext              = self.ui.dsb_dext.value(),
        profondeur        = self.ui.dsb_profondeur.value(),
        longueur          = self.ui.dsb_longueur.value(),
        azimut            = self.ui.dsb_azimut.value(),
        orientation       = self.ui.dsb_orientation.value(),
        posiAngul         = self.ui.dsb_posiAngul.value(),
        absCurv           = self.ui.dsb_absCurv.value(),
        nbTranches        = self.ui.sb_nbTranches.value(),
        nbCouronnes       = self.ui.sb_nbCouronne.value(),
        nbSecteurs        = self.ui.sb_nbSecteur.value(),
        cbOptDiscrSain    = self.ui.cb_optDiscrSain.isChecked(),
        cbOptDiscrFiss    = self.ui.cb_optDiscrFiss.isChecked(),
        rbPosiAngul       = self.ui.rb_posiAngul.isChecked(),
        rbFissExt         = self.ui.rb_fissExt.isChecked(),
        cbForceEllipse    = self.ui.cb_forceEllipse.isChecked(),
        nbAxeTubeP1       = self.ui.sb_nbAxeTubeP1.value(),
        nbAxeTubeP2       = self.ui.sb_nbAxeTubeP2.value(),
        nbAxeCoude        = self.ui.sb_nbAxeCoude.value(),
        nbCirconf         = self.ui.sb_nbCirconf.value(),
        nbEpaisseur       = self.ui.sb_nbEpaisseur.value(),
        rayonTore         = self.ui.dsb_rayonTore.value(),
        aretesFaceFissure = self.ui.dsb_aretesFaceFissure.value(),
        influence         = self.ui.dsb_influence.value(),
        )
      print dico
      return dico
      
    def checkValues(self):
      return self.NOK

    def execute(self):
      from blocFissure.gmu import initLog
      #initLog.setDebug()
      initLog.setVerbose() # don't set the level too early, to be able to modify it
      from blocFissure.gmu import geomsmesh
      from blocFissure.gmu.casStandard import casStandard
      from blocFissure.ihm.fissureCoude_ihm import fissureCoude_ihm
      
      dico = self.creeDico()
      NOK = self.testval(dico)
      if not(NOK):
        dico['lenSegPipe'] = (dico['longueur'] + math.pi*dico['profondeur'])/dico['nbTranches']
        print 'lenSegPipe', dico['lenSegPipe']
        areteMinAngle = (dico['rCintr'] -dico['dext']/2.0)*(dico['angle']*math.pi/180.0)/dico['nbAxeCoude']
        print'areteMinAngle', areteMinAngle
        areteMinCirco = dico['dext']*math.pi/(2*dico['nbCirconf'])
        print'areteMinCirco', areteMinCirco
        areteMinEpais = dico['epais']/dico['nbEpaisseur']
        print'areteMinEpais', areteMinEpais
        if dico['influence'] == 0:
          dico['influence'] = max(areteMinAngle, areteMinCirco, areteMinEpais)
          print 'influence', dico['influence']
        if dico['aretesFaceFissure'] == 0:
          dico['aretesFaceFissure'] = (areteMinAngle + areteMinCirco)/2.0
          print 'aretesFaceFissure', dico['aretesFaceFissure']
        if dico['rbPosiAngul'] == False:
          rmoy = (dico['dext'] - dico['epais'])/2.0
          eta = 1
          if dico['rbFissExt'] == False:
            eta = -1
          dico['posiAngul'] = (180.0/math.pi)*dico['absCurv']/(dico['rCintr']+(rmoy+eta*dico['epais']/2.0)*math.cos(math.pi*dico['azimut']/180.))
          print 'posiAngul' , dico['posiAngul']
        
        self.writeDefault(dico)
        self.ui.lb_calcul.show()
        probleme = fissureCoude_ihm(0)
        probleme.setDicoParams(dico)
        probleme.executeProbleme()
      self.NOK = NOK
      self.accept()
    
    pass 

# ----------------------------------------------------------------------------
                     
  window = fissureCoudeDialog()
#  window.ui.dsb_tolerance.setValue(0.01)
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
  
