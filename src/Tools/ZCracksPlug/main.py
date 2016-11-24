import sys, pickle, tempfile, shutil
from os import path, getpid, environ, remove, system

try:
  from PyQt5.QtCore import *
  from PyQt5.QtGui import *
  from PyQt5.QtWidgets import *
except:
  from PyQt4.QtCore import *
  from PyQt4.QtGui import *

import utilityFunctions as uF
import genereCrack, Zset, output, zcracks_ui

from output import message, init
from zcracks_ui import Ui_Zui


#  ---------------------
#    FONCTIONS ANNEXES
#  ---------------------


uF.removeFromSessionPath('LD_LIBRARY_PATH', 'Meshgems-2111')

def stringToFloat(string, typ=float):
  if str(string).replace(' ','')=='':
    out=[]
  else:
    out=map(typ, str(string).split())
  return(out)

def addExtension(string, extension):
  cond=True
  strLen=len(string)
  if strLen<1:
    out=None
  else:
    start=0
    lastPt=0
    while cond:
      res=string.find('.',start)
      if res==-1:
        cond=False
      else:
        lastPt=res
        start=res+1
    if strLen<=(lastPt+1+5) and lastPt!=0:
      out=string[:(lastPt+1)]+extension.replace('.','')
    else:
      out=string+'.'+extension.replace('.','')
  return(out)


#  ----------------------------
#    DEFINITION DE LA CLASSE
#  ----------------------------
class ShipHolderApplication(QGroupBox):

  def __init__(self, parent=None):
    super (ShipHolderApplication, self).__init__(parent)

    self.salomeVers=path.normpath(environ['ROOT_SALOME'])
    self.salomeVers=path.split(self.salomeVers)[-1].split('V')[-1]

    self.createWidgets()
    self.data=dict()
    self.GroupToLoad=None
    self.ui.widget.setVisible(False)
    self.tmpdir=tempfile.mkdtemp()
    self.saneGeoName=path.join(self.tmpdir,'salome_sane.geo')
    self.crackGeoName=path.join(self.tmpdir,'salome_crack.geo')
    self.crackMedName=path.join(self.tmpdir,'salome_crack.med')
    self.crackedGeoName=path.join(self.tmpdir,'cracked.geo')

    global log
    init(self.tmpdir)

    self.verbose=1

    #self.connect(self.ui.CBQuad, SIGNAL("toggled(bool)"),self.pressQuad)
    #self.connect(self.ui.btReset, SIGNAL("clicked()"),self.pressReset)
    #self.connect(self.ui.btCancel, SIGNAL("clicked()"),self.pressCancel)
    #self.connect(self.ui.btApply, SIGNAL("clicked()"),self.pressApply)
    #self.connect(self.ui.btApplyClose, SIGNAL("clicked()"),self.pressApplyClose)
    #self.connect(self.ui.btLoad, SIGNAL("clicked()"),self.pressCharger)
    #self.connect(self.ui.btSave, SIGNAL("clicked()"),self.pressSauver)
    #self.connect(self.ui.btLoadCracked, SIGNAL("clicked()"),self.pressLoadCracked)
    #self.connect(self.ui.btLoadSane, SIGNAL("clicked()"),self.pressLoadSane)

    #self.connect(self.ui.btGrVol, SIGNAL("clicked()"),self.pressLoadGroupVOL)
    #self.connect(self.ui.btGrFace, SIGNAL("clicked()"),self.pressLoadGroupFACE)
    #self.connect(self.ui.btGrEdge, SIGNAL("clicked()"),self.pressLoadGroupEDGE)
    #self.connect(self.ui.btGrNode, SIGNAL("clicked()"),self.pressLoadGroupNODE)
    #self.connect(self.ui.btGrAll, SIGNAL("clicked()"),self.pressLoadGroupALL)
    #self.connect(self.ui.btVisu, SIGNAL("clicked()"),self.pressVisu)

    #self.connect(self.ui.CBAdvanced, SIGNAL("toggled(bool)"),self.pressAdvanced)

    self.ui.CBQuad.toggled.connect(self.pressQuad)
    self.ui.btReset.clicked.connect(self.pressReset)
    self.ui.btCancel.clicked.connect(self.pressCancel)
    self.ui.btApply.clicked.connect(self.pressApply)
    self.ui.btApplyClose.clicked.connect(self.pressApplyClose)
    self.ui.btLoad.clicked.connect(self.pressCharger)
    self.ui.btSave.clicked.connect(self.pressSauver)
    self.ui.btLoadCracked.clicked.connect(self.pressLoadCracked)
    self.ui.btLoadSane.clicked.connect(self.pressLoadSane)

    self.ui.btGrVol.clicked.connect(self.pressLoadGroupVOL)
    self.ui.btGrFace.clicked.connect(self.pressLoadGroupFACE)
    self.ui.btGrEdge.clicked.connect(self.pressLoadGroupEDGE)
    self.ui.btGrNode.clicked.connect(self.pressLoadGroupNODE)
    self.ui.btGrAll.clicked.connect(self.pressLoadGroupALL)
    self.ui.btVisu.clicked.connect(self.pressVisu)

    self.ui.CBAdvanced.toggled.connect(self.pressAdvanced)
    self.lineEditTypes=[str, str, float,
                        float, float, str,
                        str, str, str,
                        float, int, int,
                        str]

    self.lineEditNames=['crackedName','saneName','minSize',
                        'maxSize','extractLength','grVol',
                        'grFace','grEdge','grNodes',
                        'gradation','iterations','layers',
                        'surfopt']

    self.lineEditObjects=[self.ui.valCrackedName,self.ui.valSaneName,self.ui.valMinSize,
                     self.ui.valMaxSize,self.ui.valExtractLength,self.ui.valGrVol,
                     self.ui.valGrFace,self.ui.valGrEdge,self.ui.valGrNode,
                     self.ui.valGradation,self.ui.valIterations,self.ui.valLayers,
                     self.ui.valSurfopt]

  def createWidgets(self):
    self.ui = Ui_Zui()
    self.ui.setupUi(self)

#  -----------------------------------
#    FONCTIONS D'ACTIONS DES BOUTONS
#  -----------------------------------

  def pressQuad(self):
    if self.ui.CBQuad.isChecked():
      self.ui.CBBarsoum.setEnabled(True)
      self.ui.valGradation.setText(QString('2.3'))
    else:
      self.ui.valGradation.setText(QString('1.3'))
      self.ui.CBBarsoum.setChecked(False)
      self.ui.CBBarsoum.setEnabled(False)


  def pressReset(self):
    for val in self.lineEditObjects:
      val.clear()
    self.ui.CBQuad.setChecked(False)
    self.ui.CBBarsoum.setChecked(False)
    nbOnglet=self.ui.tabWidget.count()
    for iongl in range(nbOnglet):
      onglet=self.ui.tabWidget.widget(iongl)
      tab=onglet.findChildren(QTableWidget)[0]
      for irow in range(tab.rowCount()):
        if tab.item(irow,0) != None:
          tab.item(irow,0).setText(QString(''))
    self.ui.valGradation.setText(QString('1.3'))
    self.ui.valLayers.setText(QString('5'))
    self.ui.valIterations.setText(QString('2'))
    self.ui.CBIs2D.setChecked(False)
    self.ui.CBRefine.setChecked(False)


  def pressApply(self):
    message('M','\n\n -------------')
    message('M','  Nouveau cas  ')
    message('M',' -------------')
    message('M','Getting parameters and checking ...')
    self.getParameters()
    test=uF.check(self.data)
    self.cleanTmpFiles()

    if test:
      message('M','Parameters checked and ready to go')
    else:
      message('E','Parameters checking failled',goOn=True)
      return()

    message('M','\nGenerating crack ...')
    res=genereCrack.main(self.data, self.crackMedName)
    if res:
      message('M','Crack generated successfully')
    else:
      message('E','Crack generation failed',goOn=True)
      return()

    if self.ui.CBIs2D.isChecked():
      res=Zset.medToGeo(self.data['saneName'],self.saneGeoName, self.tmpdir, verbose=self.verbose, opt=['  **to_3d'])
    else:
      res=Zset.medToGeo(self.data['saneName'],self.saneGeoName, self.tmpdir, verbose=self.verbose)

    if res!=0:
      message('E','medToGeo sane failed',goOn=True)
      return()

    #opt=['**elset crack *function 1.;','**elset elset0 *function 1.;']
    res=Zset.medToGeo(self.crackMedName,self.crackGeoName, self.tmpdir, verbose=self.verbose)
    if res!=0:
      message('E','medToGeo crack failed',goOn=True)
      return()

    names={'saneGeoName':self.saneGeoName, 'crackGeoName':self.crackGeoName, 'crackedGeoName':self.crackedGeoName}
    message('M','\nInserting crack ...')
    res=Zset.insertCrack(self.data, names, self.tmpdir, verbose=self.verbose)
    if res!=0:
      message('E','Crack insertion failed',goOn=True)
      return()
    else:
      message('M','Crack inserted successfully')

    if self.ui.CBQuad.isChecked() and self.ui.CBBarsoum.isChecked():
      message('M','\nSaving cracked mesh in quadratic with Barsoum elements...')
      opt=['  **lin_to_quad','  **crack_3d_quarter_nodes','   *liset FRONT0']
      res=Zset.geoToMed(self.data['crackedName'], names['crackedGeoName'], self.tmpdir, opt=opt, verbose=self.verbose)

    elif self.ui.CBQuad.isChecked() and not self.ui.CBBarsoum.isChecked():
      message('M','\nSaving cracked mesh in quadratic...')
      opt=['  **lin_to_quad']
      res=Zset.geoToMed(self.data['crackedName'], names['crackedGeoName'], self.tmpdir, opt=opt, verbose=self.verbose)

    else:
      message('M','\nSaving cracked mesh...')
      res=Zset.geoToMed(self.data['crackedName'], names['crackedGeoName'], self.tmpdir, verbose=self.verbose)

    uF.extendElsets(self.data['crackedName'])

    if res==0:
      message('M','Cracked mesh ready at : %s' %(self.data['crackedName']))
      message('M','Maximal aspect ratio is %f' %(uF.getMaxAspectRatio(self.tmpdir)))
      #message('M','medit %s/_mesh_out_to_ghs3d.mesh' %(self.tmpdir))
      message('M','\n ----------------')
      message('M','    Fin cas OK   ')
      message('M',' ----------------')



  def pressApplyClose(self):
    self.pressApply()
    self.pressCancel()


  def pressLoadCracked(self):
    fileDiag = QFileDialog(self)
    fileDiag.setFileMode(QFileDialog.AnyFile)
    fileDiag.setNameFilters(["Parametres *.med (*.*med)","All files (*)"])
    fileDiag.setViewMode(QFileDialog.List)
    if fileDiag.exec_() :
      fileNames = fileDiag.selectedFiles()
      filedef = fileNames[0]
      filedef = addExtension(str(filedef), 'med')
      self.ui.valCrackedName.setText(QString(filedef))


  def pressLoadSane(self):
    fileDiag = QFileDialog(self)
    fileDiag.setFileMode(QFileDialog.AnyFile)
    fileDiag.setNameFilters(["Parametres *.med (*.*med)","All files (*)"])
    fileDiag.setViewMode(QFileDialog.List)
    if fileDiag.exec_() :
      fileNames = fileDiag.selectedFiles()
      filedef = fileNames[0]
      self.ui.valSaneName.setText(QString(filedef))


  def pressCharger(self):
    fileDiag = QFileDialog(self)
    fileDiag.setFileMode(QFileDialog.AnyFile)
    fileDiag.setNameFilters(["Parametres *.dic (*.dic)","All files (*)"])
    fileDiag.setViewMode(QFileDialog.List)

    if fileDiag.exec_() :
      fileNames = fileDiag.selectedFiles()
      filedef = fileNames[0]
      if not path.isfile(str(filedef)):
        message('E','Invalid dic file')
      self.data=pickle.load(open(str(filedef),'r'))
      message('M','\nLoading parameters from %s' %str(filedef))

      for cont, obj in enumerate(self.lineEditObjects):
        if self.lineEditTypes[cont] in [float, int]:
          obj.setText(QString(self.data['TXT'+self.lineEditNames[cont]]))
        else:
          obj.setText(QString(self.data[self.lineEditNames[cont]]))

      self.ui.CBQuad.setChecked(True if 'quad' in self.data.keys() and self.data['quad'] else False)
      self.ui.CBBarsoum.setChecked(True if 'barsoum' in self.data.keys() and self.data['barsoum'] else False)
      self.ui.CBIs2D.setChecked(True if 'is2D' in self.data.keys() and self.data['is2D'] else False)
      self.ui.CBRefine.setChecked(True if 'refine' in self.data.keys() and self.data['refine'] else False)





      #if self.data['quad']: self.ui.CBQuad.setChecked(True)
      #if self.data['barsoum']: self.ui.CBBarsoum.setChecked(True)
      #if self.data['is2D']: self.ui.CBIs2D.setChecked(True)
      #if self.data['refine']: self.ui.CBRefine.setChecked(True)
      self.setTableParameters()


  def pressSauver(self):
    fileDiag = QFileDialog(self)
    fileDiag.setFileMode(QFileDialog.AnyFile)
    fileDiag.setNameFilters(["Parametres *.dic (*.dic)","All files (*)"])
    fileDiag.setViewMode(QFileDialog.List)
    if fileDiag.exec_() :
      self.getParameters()
      fileNames = fileDiag.selectedFiles()
      filedef = fileNames[0]
      pickle.dump(self.data, open(addExtension(str(filedef), 'dic'),'w'))
      message('M','Saving parameters in %s' %addExtension(str(filedef), 'dic'))

  def pressLoadGroupVOL(self):
    try:
      self.GroupToLoad='VOL'
      self.loadGroups()
    except:
      message('E','Groups loading impossible',goOn=True)

  def pressLoadGroupFACE(self):
    try:
      self.GroupToLoad='FACE'
      self.loadGroups()
    except:
      message('E','Groups loading impossible',goOn=True)

  def pressLoadGroupEDGE(self):
    try:
      self.GroupToLoad='EDGE'
      self.loadGroups()
    except:
      message('E','Groups loading impossible',goOn=True)

  def pressLoadGroupNODE(self):
    try:
      self.GroupToLoad='NODE'
      self.loadGroups()
    except:
      message('E','Groups loading impossible',goOn=True)

  def pressLoadGroupALL(self):
    try:
      self.GroupToLoad='ALL'
      self.loadGroups()
    except:
      message('E','Groups loading impossible',goOn=True)

  def pressAdvanced(self):
    if self.ui.CBAdvanced.isChecked():
      self.ui.widget.setVisible(True)
    else:
      self.ui.widget.setVisible(False)

  def pressVisu(self):
    meshFile1=path.join(self.tmpdir,'_mesh_out_to_ghs3d.mesh')
    meshFile2=path.join(self.tmpdir,'_mesh_out_.mesh')
    test1=path.isfile(meshFile1)
    test2=path.isfile(meshFile2)
    medit=path.join('$Z7PATH/PUBLIC/lib-Linux_64/Zmesh/bin/medit')
    if not test1:
      if not test2:
        message('A','No mesh file to visualize')
      else:
        print medit+' %s' %meshFile2
        system(medit+' %s' %meshFile2)
    else:
      print medit+' %s' %meshFile1
      system(medit+' %s' %meshFile1)
    return()

  def pressCancel(self):
    message('M','exiting Zcracks')
    try:
      shutil.rmtree(self.tmpdir)
    except:
      message('E','Impossible to delete %s' %self.tmpdir,goOn=True)
      pass
    exit()

#  ---------------------------------
#    FONCTIONS ANNEXES A LA CLASSE
#  ---------------------------------


  def getParameters(self):
    for cont, name in enumerate(self.lineEditNames):
      value=str(self.lineEditObjects[cont].text())
      #print name
      if self.lineEditTypes[cont] == float:
        self.data['TXT'+name]=value
        self.data[name]= stringToFloat(value)
      elif self.lineEditTypes[cont] == int:
        self.data['TXT'+name]=value
        self.data[name]= stringToFloat(value, typ=int)
      else:
        self.data[name]=value
    self.data['quad']=self.ui.CBQuad.isChecked()
    self.data['barsoum']=self.ui.CBBarsoum.isChecked()
    self.data['TXTcrack']=self.getTableParameters()
    self.data['crack']=self.getTableParameters(str2float=True)
    self.data['is2D']=self.ui.CBIs2D.isChecked()
    self.data['refine']=self.ui.CBRefine.isChecked()


  def getTableParameters(self, str2float=False):
    nbOnglet=self.ui.tabWidget.count()
    out=dict()
    iOngletActif=self.ui.tabWidget.currentIndex()
    ongletActif=False
    for iongl in range(nbOnglet):
      crack=dict()
      onglet=self.ui.tabWidget.widget(iongl)
      tab=onglet.findChildren(QTableWidget)[0]
      for irow in range(tab.rowCount()):
        label=tab.verticalHeaderItem(irow).text()
        if tab.item(irow,0) is None:
          crack[str(label)]=''
        elif 'med file' in str(label):
          crack[str(label)]=str(tab.item(irow,0).text())
        else:
          value=tab.item(irow,0).text()
          if str2float:
            crack[str(label)]=stringToFloat(value)
          else:
            crack[str(label)]=str(value)
      out[str(self.ui.tabWidget.tabText(iongl))]=crack
      if iongl==iOngletActif:
        ongletActif=str(self.ui.tabWidget.tabText(iongl))

    out['actif']=ongletActif
    return(out)


  def setTableParameters(self):
    nbOnglet=self.ui.tabWidget.count()
    #iOngletActif=self.ui.tabWidget.currentIndex()
    for iongl in range(nbOnglet):
      onglet=self.ui.tabWidget.widget(iongl)
      tab=onglet.findChildren(QTableWidget)[0]
      for irow in range(tab.rowCount()):
        label=tab.verticalHeaderItem(irow).text()
        if tab.item(irow,0) == None:
          item = QTableWidgetItem()
          tab.setItem(irow, 0, item)
        tab.item(irow,0).setText(QString(self.data['TXTcrack'][str(self.ui.tabWidget.tabText(iongl))][str(label)]))
      if str(self.ui.tabWidget.tabText(iongl)) == self.data['TXTcrack']['actif']:
        self.ui.tabWidget.setCurrentWidget(onglet)


  def loadGroups(self):
    saneFile=str(self.ui.valSaneName.text())
    message('I','Loading Sane mesh...')
    if not path.isfile(saneFile):
      message('E','Sane mesh med file is not valid')
    else:
      import SMESH, salome
      #salome.salome_init()
      theStudy = salome.myStudy
      from salome.smesh import smeshBuilder
      smesh = smeshBuilder.New(theStudy)

      ([objetSain], status) = smesh.CreateMeshesFromMED(saneFile)

      groupsVOL, groupsFAC, groupsEDG, groupsNOD = '', '', '', ''
      nGr=0

      for group in objetSain.GetGroups():
        if (self.GroupToLoad in ['VOL','ALL']) and (group.GetType()==SMESH.VOLUME):
          groupsVOL+=group.GetName().replace(' ','')+" "
          nGr+=1

        if (self.GroupToLoad in ['FACE','ALL']) and (group.GetType()==SMESH.FACE):
          groupsFAC+=group.GetName().replace(' ','')+" "
          nGr+=1

        if (self.GroupToLoad in ['EDGE','ALL']) and (group.GetType()==SMESH.EDGE):
          groupsEDG+=group.GetName().replace(' ','')+" "
          nGr+=1

        if (self.GroupToLoad in ['NODE','ALL']) and (group.GetType()==SMESH.NODE):
          groupsNOD+=group.GetName().replace(' ','')+" "
          nGr+=1

      if groupsVOL!='':  self.ui.valGrVol.setText(groupsVOL)
      if groupsFAC!='': self.ui.valGrFace.setText(groupsFAC)
      if groupsEDG!='': self.ui.valGrEdge.setText(groupsEDG)
      if groupsNOD!='': self.ui.valGrNode.setText(groupsNOD)

      message('I','%d group(s) found' %nGr)

  def cleanTmpFiles(self):
    for f in [self.saneGeoName, self.crackGeoName, self.crackMedName, self.crackedGeoName]:
      try:
        remove(f)
      except:
        pass


#  ---------------------------------
#    LANCEMENT DE LA BOITE DE DIAG
#  ---------------------------------




