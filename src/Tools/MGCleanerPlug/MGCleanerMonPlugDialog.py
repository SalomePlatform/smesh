# -*- coding: utf-8 -*-
# Copyright (C) 2013-2016  EDF R&D
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

# Modules Python
# Modules Eficas

import os, subprocess
import tempfile
from MGCleanerPlugDialog_ui import Ui_MGCleanerPlugDialog
from MGCleanerMonViewText import MGCleanerMonViewText
from qtsalome import *

verbose = True

class MGCleanerMonPlugDialog(Ui_MGCleanerPlugDialog,QWidget):
  """
  """
  def __init__(self):
    QWidget.__init__(self)
    self.setupUi(self)
    self.connecterSignaux()
    self.fichierIn=""
    self.fichierOut=""
    self.MeshIn=""
    self.commande=""
    self.num=1
    self.__selectedMesh=None

    # complex whith QResources: not used
    # The icon are supposed to be located in the $SMESH_ROOT_DIR/share/salome/resources/smesh folder,
    # other solution could be in the same folder than this python module file:
    # iconfolder=os.path.dirname(os.path.abspath(__file__))

    self.iconfolder=os.environ["SMESH_ROOT_DIR"]+"/share/salome/resources/smesh"
    #print "MGCleanerMonPlugDialog iconfolder",iconfolder
    icon = QIcon()
    icon.addFile(os.path.join(self.iconfolder,"select1.png"))
    self.PB_LoadHyp.setIcon(icon)
    self.PB_LoadHyp.setToolTip("hypothesis from Salome Object Browser")
    self.PB_SaveHyp.setIcon(icon)
    self.PB_SaveHyp.setToolTip("hypothesis to Salome Object Browser")
    self.PB_MeshSmesh.setIcon(icon)
    self.PB_MeshSmesh.setToolTip("source mesh from Salome Object Browser")
    icon = QIcon()
    icon.addFile(os.path.join(self.iconfolder,"open.png"))
    self.PB_ParamsFileExplorer.setIcon(icon)
    self.PB_Load.setIcon(icon)
    self.PB_Load.setToolTip("hypothesis from file")
    self.PB_Save.setIcon(icon)
    self.PB_Save.setToolTip("hypothesis to file")
    self.PB_MeshFile.setIcon(icon)
    self.PB_MeshFile.setToolTip("source mesh from a file in disk")

    #Ces parametres ne sont pas remis a rien par le clean
    self.paramsFile= os.path.abspath(os.path.join(os.environ["HOME"],".MGCleaner.dat"))
    self.LE_ParamsFile.setText(self.paramsFile)
    self.LE_MeshFile.setText("")
    self.LE_MeshSmesh.setText("")

    v1=QDoubleValidator(self)
    v1.setBottom(0.)
    #v1.setTop(10000.)
    v1.setDecimals(4)
    self.SP_MinHoleSize.setValidator(v1)
    self.SP_MinHoleSize.titleForWarning="MinHoleSize"

    v2=QDoubleValidator(self)
    v2.setBottom(0.)
    #v2.setTop(10000.)
    v2.setDecimals(4)
    self.SP_ToleranceDisplacement.setValidator(v2)
    self.SP_ToleranceDisplacement.titleForWarning="ToleranceDisplacement"

    v3=QDoubleValidator(self)
    v3.setBottom(0.)
    #v3.setTop(10000.)
    v3.setDecimals(4)
    self.SP_ResolutionLength.setValidator(v3)
    self.SP_ResolutionLength.titleForWarning="ResolutionLength"
    
    v4=QDoubleValidator(self)
    v4.setBottom(0.)
    #v4.setTop(10000.)
    v4.setDecimals(4)
    self.SP_OverlapDistance.setValidator(v4)
    self.SP_OverlapDistance.titleForWarning="OverlapDistance"
    
    self.resize(800, 500)
    self.clean()

  def connecterSignaux(self) :
    self.PB_Cancel.clicked.connect(self.PBCancelPressed)
    self.PB_Default.clicked.connect(self.clean)
    self.PB_Help.clicked.connect(self.PBHelpPressed)
    self.PB_OK.clicked.connect(self.PBOKPressed)
    
    self.PB_Load.clicked.connect(self.PBLoadPressed)
    self.PB_Save.clicked.connect(self.PBSavePressed)
    self.PB_LoadHyp.clicked.connect(self.PBLoadHypPressed)
    self.PB_SaveHyp.clicked.connect(self.PBSaveHypPressed)
    
    self.PB_MeshFile.clicked.connect(self.PBMeshFilePressed)
    self.PB_MeshSmesh.clicked.connect(self.PBMeshSmeshPressed)
    self.LE_MeshSmesh.returnPressed.connect(self.meshSmeshNameChanged)
    self.PB_ParamsFileExplorer.clicked.connect(self.setParamsFileName)
    self.LE_MeshFile.returnPressed.connect(self.meshFileNameChanged)
    self.LE_ParamsFile.returnPressed.connect(self.paramsFileNameChanged)

    #QtCore.QObject.connect(self.checkBox, QtCore.SIGNAL("stateChanged(int)"), self.change) 
    self.CB_FillHoles.stateChanged[int].connect(self.SP_MinHoleSize.setEnabled)
    self.CB_ComputedToleranceDisplacement.stateChanged[int].connect(self.SP_ToleranceDisplacement.setDisabled)
    self.CB_ComputedResolutionLength.stateChanged[int].connect(self.SP_ResolutionLength.setDisabled)
    self.CB_ComputedOverlapDistance.stateChanged[int].connect(self.SP_OverlapDistance.setDisabled)

  def PBHelpPressed(self):
    import SalomePyQt
    sgPyQt = SalomePyQt.SalomePyQt()
    try:
      mydir=os.environ["SMESH_ROOT_DIR"]
    except Exception:
      QMessageBox.warning(self, "Help", "Help unavailable $SMESH_ROOT_DIR not found")
      return

    maDoc=mydir+"/share/doc/salome/gui/SMESH/MGCleaner/index.html"
    sgPyQt.helpContext(maDoc,"")
    
    #maDoc=mydir+"/share/doc/salome/gui/SMESH/MGCleaner/_downloads/mg-cleaner_user_manual.pdf"
    #command="xdg-open "+maDoc+";"
    #subprocess.call(command, shell=True)

  def PBOKPressed(self):
    if not(self.PrepareLigneCommande()):
      #warning done yet
      #QMessageBox.warning(self, "Compute", "Command not found")
      return
    maFenetre=MGCleanerMonViewText(self, self.commande)

  def enregistreResultat(self):
    import salome
    import SMESH
    from salome.kernel import studyedit
    from salome.smesh import smeshBuilder
    smesh = smeshBuilder.New(salome.myStudy)
    
    if not os.path.isfile(self.fichierOut):
      QMessageBox.warning(self, "Compute", "Result file "+self.fichierOut+" not found")

    maStudy=studyedit.getActiveStudy()
    smesh.SetCurrentStudy(maStudy)
    (outputMesh, status) = smesh.CreateMeshesFromGMF(self.fichierOut)
    name=str(self.LE_MeshSmesh.text())
    initialMeshFile=None
    initialMeshObject=None
    if name=="":
      a=str(self.fichierIn)
      name=os.path.basename(os.path.splitext(a)[0])
      initialMeshFile=a
    else:
      initialMeshObject=maStudy.FindObjectByName(name ,"SMESH")[0]

    meshname = name+"_MGC_"+str(self.num)
    smesh.SetName(outputMesh.GetMesh(), meshname)
    outputMesh.Compute() #no algorithms message for "Mesh_x" has been computed with warnings: -  global 1D algorithm is missing

    self.editor = studyedit.getStudyEditor()
    moduleEntry=self.editor.findOrCreateComponent("SMESH","SMESH")
    HypReMeshEntry = self.editor.findOrCreateItem(
        moduleEntry, name = "Plugins Hypotheses", icon="mesh_tree_hypo.png") #, comment = "HypoForRemeshing" )
    
    monStudyBuilder=maStudy.NewBuilder()
    monStudyBuilder.NewCommand()
    newStudyIter=monStudyBuilder.NewObject(HypReMeshEntry)
    self.editor.setAttributeValue(newStudyIter, "AttributeName", "MGCleaner Parameters_"+str(self.num))
    self.editor.setAttributeValue(newStudyIter, "AttributeComment", self.getResumeData(separator=" ; "))
    
    SOMesh=maStudy.FindObjectByName(meshname ,"SMESH")[0]
    
    if initialMeshFile!=None:
      newStudyFileName=monStudyBuilder.NewObject(SOMesh)
      self.editor.setAttributeValue(newStudyFileName, "AttributeName", "meshFile")
      self.editor.setAttributeValue(newStudyFileName, "AttributeExternalFileDef", initialMeshFile)
      self.editor.setAttributeValue(newStudyFileName, "AttributeComment", initialMeshFile)

    if initialMeshObject!=None:
      newLink=monStudyBuilder.NewObject(SOMesh)
      monStudyBuilder.Addreference(newLink, initialMeshObject)

    newLink=monStudyBuilder.NewObject(SOMesh)
    monStudyBuilder.Addreference(newLink, newStudyIter)

    if salome.sg.hasDesktop(): salome.sg.updateObjBrowser(False)
    self.num+=1
    return True

  def PBSavePressed(self):
    from datetime import datetime
    if not(self.PrepareLigneCommande()): return
    text = "# MGCleaner hypothesis parameters\n"
    text += "# Params for mesh : " +  self.LE_MeshSmesh.text() +"\n"
    text += datetime.now().strftime("# Date : %d/%m/%y %H:%M:%S\n")
    text += "# Command : "+self.commande+"\n"
    text += self.getResumeData(separator="\n")
    text += "\n\n"

    try:
      f=open(self.paramsFile,"a")
    except:
      QMessageBox.warning(self, "File", "Unable to open "+self.paramsFile)
      return
    try:
      f.write(text)
    except:
      QMessageBox.warning(self, "File", "Unable to write "+self.paramsFile)
      return
    f.close()

  def PBSaveHypPressed_risky(self):
    """
    save hypothesis in Object Browser outside GEOM ou MESH
    WARNING: at root of Object Browser is not politically correct
    """
    import salome
    
    if verbose: print("save hypothesis in Object Browser")
    
    name = "MGCleaner"
    #how ??? icon = "mesh_tree_hypo.png"
    namei = "MGCleaner Parameters_%i" % self.num
    datai = self.getResumeData(separator=" ; ")
    
    myStudy = salome.myStudy
    myBuilder = myStudy.NewBuilder()
    #myStudy.IsStudyLocked()
    myComponent = myStudy.FindComponent(name)
    if myComponent == None:
      print "myComponent not found, create"
      myComponent = myBuilder.NewComponent(name)
    AName = myBuilder.FindOrCreateAttribute(myComponent, "AttributeName")
    AName.SetValue(name)
    ACmt = myBuilder.FindOrCreateAttribute(myComponent, "AttributeComment")
    ACmt.SetValue(name)
    
    myObject = myBuilder.NewObject(myComponent)
    AName = myBuilder.FindOrCreateAttribute(myObject, "AttributeName")
    AName.SetValue(namei)
    ACmt = myBuilder.FindOrCreateAttribute(myObject, "AttributeComment")
    ACmt.SetValue(datai)

    if salome.sg.hasDesktop(): salome.sg.updateObjBrowser(False)
    self.num += 1
    if verbose: print("save %s in Object Browser done: %s\n%s" % (name, myObject.GetID(), datai))
    return True

  def PBSaveHypPressed(self):
    """
    save hypothesis in Object Browser
    bug: affichage ne marche pas si inclusion dans dans GEOM ou MESH depuis salome 730
    """
    import salome
    import SMESH
    from salome.kernel import studyedit
    from salome.smesh import smeshBuilder
    #[PAL issue tracker:issue1871] Les nouveaux objets ne s'affichent pas dans SMESH
    QMessageBox.warning(self, "Save", "waiting for fix: Object Browser will not display hypothesis")
    
    if verbose: print("save hypothesis in Object Browser")
    smesh = smeshBuilder.New(salome.myStudy)

    maStudy=studyedit.getActiveStudy()
    smesh.SetCurrentStudy(maStudy)

    self.editor = studyedit.getStudyEditor()
    moduleEntry=self.editor.findOrCreateComponent("SMESH","SMESH")
    HypReMeshEntry = self.editor.findOrCreateItem(
        moduleEntry, name = "Plugins Hypotheses", icon="mesh_tree_hypo.png")
    #, comment = "HypothesisForRemeshing" )

    monStudyBuilder=maStudy.NewBuilder()
    monStudyBuilder.NewCommand()
    newStudyIter=monStudyBuilder.NewObject(HypReMeshEntry)
    name = "MGCleaner Parameters_%i" % self.num
    self.editor.setAttributeValue(newStudyIter, "AttributeName", name)
    data = self.getResumeData(separator=" ; ")
    self.editor.setAttributeValue(newStudyIter, "AttributeComment", data)

    """ 
    # example storing in notebook
    import salome_notebook
    notebook = salome_notebook.notebook
    notebook.set("MGCleaner_%i" % self.num, data)
    """

    if salome.sg.hasDesktop(): salome.sg.updateObjBrowser(False)
    self.num += 1
    if verbose: print("save %s in Object Browser done:\n%s" % (name, data))
    return True

  def SP_toStr(self, widget):
    """only for a QLineEdit widget"""
    #cr, pos=widget.validator().validate(res, 0) #n.b. "1,3" is acceptable !locale!
    try:
      val=float(widget.text())
    except:
      QMessageBox.warning(self, widget.titleForWarning, "float value is incorrect: '"+widget.text()+"'")
      res=str(widget.validator().bottom())
      widget.setProperty("text", res)
      return res
    valtest=widget.validator().bottom()
    if valtest!=None:
      if val<valtest:
        QMessageBox.warning(self, widget.titleForWarning, "float value is under minimum: "+str(val)+" < "+str(valtest))
        res=str(valtest)
        widget.setProperty("text", res)
        return res
    valtest=widget.validator().top()
    if valtest!=None:
      if val>valtest:
        QMessageBox.warning(self, widget.titleForWarning, "float value is over maximum: "+str(val)+" > "+str(valtest))
        res=str(valtest)
        widget.setProperty("text", res)
        return res    
    return str(val)

  def getResumeData(self, separator="\n"):
    text=""
    if self.RB_Fix1.isChecked():
      CheckOrFix="fix1pass"
    else:
      if self.RB_Fix2.isChecked():
        CheckOrFix="fix2pass"
      else:
        CheckOrFix="check"
    text+="CheckOrFix="+CheckOrFix+separator
    text+="PreserveTopology="+str(self.CB_PreserveTopology.isChecked())+separator
    text+="FillHoles="+str(self.CB_FillHoles.isChecked())+separator
    v=self.SP_toStr(self.SP_MinHoleSize)
    text+="MinHoleSize="+v+separator
    text+="ComputedToleranceDisplacement="+str(self.CB_ComputedToleranceDisplacement.isChecked())+separator
    v=self.SP_toStr(self.SP_ToleranceDisplacement)
    text+="ToleranceDisplacement="+v+separator
    text+="ComputedResolutionLength="+str(self.CB_ComputedResolutionLength.isChecked())+separator
    v=self.SP_toStr(self.SP_ResolutionLength)
    text+="ResolutionLength="+v+separator
    text+="FoldingAngle="+str(self.SP_FoldingAngle.value())+separator
    text+="RemeshPlanes="+str(self.CB_RemeshPlanes.isChecked())+separator
    text+="ComputedOverlapDistance="+str(self.CB_ComputedOverlapDistance.isChecked())+separator
    v=self.SP_toStr(self.SP_OverlapDistance)
    text+="OverlapDistance="+v+separator
    text+="OverlapAngle="+str(self.SP_OverlapAngle.value())+separator
    text+="Verbosity="+str(self.SP_Verbosity.value())+separator
    return str(text)

  def loadResumeData(self, hypothesis, separator="\n"):
    text=str(hypothesis)
    self.clean()
    for slig in reversed(text.split(separator)):
      lig=slig.strip()
      #print "load ResumeData",lig
      if lig=="": continue #skip blanck lines
      if lig[0]=="#": break
      try:
        tit,value=lig.split("=")
        if tit=="CheckOrFix":
          self.RB_Fix1.setChecked(False)
          self.RB_Fix2.setChecked(False)
          self.RB_Check.setChecked(False)
          if value=="fix1pass": self.RB_Fix1.setChecked(True)
          if value=="fix2pass": self.RB_Fix2.setChecked(True)
          if value=="check": self.RB_Check.setChecked(True)
        if tit=="PreserveTopology": self.CB_PreserveTopology.setChecked(value=="True")
        if tit=="FillHoles": self.CB_FillHoles.setChecked(value=="True")
        if tit=="MinHoleSize": self.SP_MinHoleSize.setProperty("text", value)
        if tit=="ComputedToleranceDisplacement": self.CB_ComputedToleranceDisplacement.setChecked(value=="True")
        if tit=="ToleranceDisplacement": self.SP_ToleranceDisplacement.setProperty("text", value)
        if tit=="ComputedResolutionLength": self.CB_ComputedResolutionLength.setChecked(value=="True")
        if tit=="ResolutionLength": self.SP_ResolutionLength.setProperty("text", value)
        if tit=="FoldingAngle": self.SP_FoldingAngle.setProperty("value", float(value))
        if tit=="RemeshPlanes": self.CB_RemeshPlanes.setChecked(value=="True")
        if tit=="ComputedOverlapDistance": self.CB_ComputedOverlapDistance.setChecked(value=="True")
        if tit=="OverlapDistance": self.SP_OverlapDistance.setProperty("text", value)
        if tit=="OverlapAngle": self.SP_OverlapAngle.setProperty("value", float(value))
        if tit=="Verbosity": self.SP_Verbosity.setProperty("value", int(float(value)))
      except:
        QMessageBox.warning(self, "load MGCleaner Hypothesis", "Problem on '"+lig+"'")

  def PBLoadPressed(self):
    """load last hypothesis saved in tail of file"""
    try:
      f=open(self.paramsFile,"r")
    except:
      QMessageBox.warning(self, "File", "Unable to open "+self.paramsFile)
      return
    try:
      text=f.read()
    except:
      QMessageBox.warning(self, "File", "Unable to read "+self.paramsFile)
      return
    f.close()
    self.loadResumeData(text, separator="\n")

  def PBLoadHypPressed(self):
    """load hypothesis saved in Object Browser"""
    #QMessageBox.warning(self, "load Object Browser MGCleaner hypothesis", "TODO")
    import salome
    from salome.kernel import studyedit
    from salome.smesh.smeshstudytools import SMeshStudyTools
    from salome.gui import helper as guihelper
    from omniORB import CORBA

    mySObject, myEntry = guihelper.getSObjectSelected()
    if CORBA.is_nil(mySObject) or mySObject==None:
      QMessageBox.critical(self, "Hypothese", "select an Object Browser MGCleaner hypothesis")
      return
    
    #for i in dir(mySObject): print "dir mySObject",i
    #print "GetAllAttributes",mySObject.GetAllAttributes()
    #print "GetComment",mySObject.GetComment()
    #print "GetName",mySObject.GetName()
    
    #could be renamed...
    #if mySObject.GetFather().GetName()!="MGCleaner Hypotheses":
    #  QMessageBox.critical(self, "Hypothese", "not a child of MGCleaner Hypotheses")
    #  return
    
    text=mySObject.GetComment()
    
    #a verification
    if "CheckOrFix=" not in text:
      QMessageBox.critical(self, "Load Hypothese", "Object Browser selection not a MGCleaner Hypothesis")
      return
    self.loadResumeData(text, separator=" ; ")
    return
    
  def PBCancelPressed(self):
    self.close()

  def PBMeshFilePressed(self):
    fd = QFileDialog(self, "select an existing Mesh file", self.LE_MeshFile.text(), "Mesh-Files (*.mesh);;All Files (*)")
    if fd.exec_():
      infile = fd.selectedFiles()[0]
      self.LE_MeshFile.setText(infile)
      self.fichierIn=unicode(infile).encode("latin-1")
      self.MeshIn=""
      self.LE_MeshSmesh.setText("")

  def setParamsFileName(self):
    fd = QFileDialog(self, "select a file", self.LE_ParamsFile.text(), "dat Files (*.dat);;All Files (*)")
    if fd.exec_():
      infile = fd.selectedFiles()[0]
      self.LE_ParamsFile.setText(infile)
      self.paramsFile=unicode(infile).encode("latin-1")

  def meshFileNameChanged(self):
    self.fichierIn=str(self.LE_MeshFile.text())
    #print "meshFileNameChanged", self.fichierIn
    if os.path.exists(self.fichierIn): 
      self.__selectedMesh=None
      self.MeshIn=""
      self.LE_MeshSmesh.setText("")
      return
    QMessageBox.warning(self, "Mesh file", "File doesn't exist")

  def meshSmeshNameChanged(self):
    """only change by GUI mouse selection, otherwise clear"""
    #self.MeshIn=str(self.LE_MeshSmesh.text())
    #print "meshSmeshNameChanged", self.MeshIn
    self.__selectedMesh = None
    self.MeshIn=""
    self.LE_MeshSmesh.setText("")
    self.fichierIn=""
    return

  def paramsFileNameChanged(self):
    self.paramsFile=self.LE_ParamsFile.text()

  def PBMeshSmeshPressed(self):
    from omniORB import CORBA
    import salome
    from salome.kernel import studyedit
    from salome.smesh.smeshstudytools import SMeshStudyTools
    from salome.gui import helper as guihelper
    from salome.smesh import smeshBuilder
    smesh = smeshBuilder.New(salome.myStudy)

    mySObject, myEntry = guihelper.getSObjectSelected()
    if CORBA.is_nil(mySObject) or mySObject==None:
      QMessageBox.critical(self, "Mesh", "select an input mesh")
      #self.LE_MeshSmesh.setText("")
      #self.MeshIn=""
      #self.LE_MeshFile.setText("")
      #self.fichierIn=""
      return
    self.smeshStudyTool = SMeshStudyTools()
    try:
      self.__selectedMesh = self.smeshStudyTool.getMeshObjectFromSObject(mySObject)
    except:
      QMessageBox.critical(self, "Mesh", "select an input mesh")
      return
    if CORBA.is_nil(self.__selectedMesh):
      QMessageBox.critical(self, "Mesh", "select an input mesh")
      return
    myName = mySObject.GetName()
    #print "MeshSmeshNameChanged", myName
    self.MeshIn=myName
    self.LE_MeshSmesh.setText(myName)
    self.LE_MeshFile.setText("")
    self.fichierIn=""

  def prepareFichier(self):
    self.fichierIn=tempfile.mktemp(suffix=".mesh",prefix="ForMGCleaner_")
    if os.path.exists(self.fichierIn):
        os.remove(self.fichierIn)
    self.__selectedMesh.ExportGMF(self.__selectedMesh, self.fichierIn, True)

  def PrepareLigneCommande(self):
    """
    #use doc examples of mg-cleaner:
    ls -al /data/tmplgls/salome/prerequis/install/COMMON_64/MeshGems-1.0/bin
    source /data/tmplgls/salome/prerequis/install/LICENSE/dlim8.var.sh
    export PATH=/data/tmplgls/salome/prerequis/install/COMMON_64/MeshGems-1.0/bin/Linux_64:$PATH
    cp -r /data/tmplgls/salome/prerequis/install/COMMON_64/MeshGems-1.0/examples .
    cd examples
    mg-cleaner.exe --help
    mg-cleaner.exe --in case7.mesh --out case7-test.mesh --check
    mg-cleaner.exe case7.mesh case7-fix.mesh --fix
    mg-cleaner.exe --in Porsche.mesh --out Porsche-test.mesh --check
    mg-cleaner.exe --in Porsche.mesh --out Porschefix.mesh --fix
    mg-cleaner.exe --in Porsche.mesh --out PorscheNewfix.mesh --fix --resolution_length 0.03
    """
    
    #self.commande="mg-cleaner.exe --in " + self.fichierIn + " --out " + self.fichierOut + " --fix2pass" 
    #return True
    #print "PrepareLigneCommande '"+self.fichierIn+"' '"+self.MeshIn+"'",self.__selectedMesh
    if self.fichierIn=="" and self.MeshIn=="":
      QMessageBox.critical(self, "Mesh", "select an input mesh")
      return False
    if self.__selectedMesh!=None: self.prepareFichier()
    if not (os.path.isfile(self.fichierIn)):
      QMessageBox.critical(self, "File", "unable to read GMF Mesh in "+str(self.fichierIn))
      return False
    
    self.commande="mg-cleaner.exe"
    verbosity=str(self.SP_Verbosity.value())
    self.commande+=" --verbose " + verbosity
    self.commande+=" --in " + self.fichierIn
    #print "self.fichierIn",self.fichierIn,type(self.fichierIn)
    deb=os.path.splitext(str(self.fichierIn))
    self.fichierOut=deb[0] + "_fix.mesh"
    self.commande+=" --out "+self.fichierOut
    if self.RB_Fix1.isChecked():
      self.commande+=" --fix1pass"
    else:
      if self.RB_Fix2.isChecked():
        self.commande+=" --fix2pass"
      else:
        self.commande+=" --check"
    if self.CB_PreserveTopology.isChecked():
      self.commande+=" --topology respect"
    else:
      self.commande+=" --topology ignore"
    if self.CB_FillHoles.isChecked(): #no fill holes default
      self.commande+=" --min_hole_size " + self.SP_toStr(self.SP_MinHoleSize)
    if not self.CB_ComputedToleranceDisplacement.isChecked(): #computed default
      self.commande+=" --tolerance_displacement " + self.SP_toStr(self.SP_ToleranceDisplacement)
    if not self.CB_ComputedResolutionLength.isChecked(): #computed default
      self.commande+=" --resolution_length " + self.SP_toStr(self.SP_ResolutionLength)
    self.commande+=" --folding_angle " + str(self.SP_FoldingAngle.value())
    if self.CB_RemeshPlanes.isChecked(): #no remesh default
      self.commande+=" --remesh_planes"
    if not self.CB_ComputedOverlapDistance.isChecked(): #computed default
      self.commande+=" --overlap_distance " + self.SP_toStr(self.SP_OverlapDistance)
    self.commande+=" --overlap_angle " + str(self.SP_OverlapAngle.value())
    if verbose: print("INFO: MGCCleaner command:\n  %s" % self.commande)
    return True

  def clean(self):
    self.RB_Check.setChecked(False)
    self.RB_Fix1.setChecked(False)
    self.RB_Fix2.setChecked(True)
    self.CB_PreserveTopology.setChecked(False)
    self.CB_FillHoles.setChecked(False)
    self.CB_RemeshPlanes.setChecked(False)
    
    self.SP_MinHoleSize.setProperty("text", 0)
    self.SP_ToleranceDisplacement.setProperty("text", 0)
    self.SP_ResolutionLength.setProperty("text", 0)
    self.SP_FoldingAngle.setProperty("value", 15)
    self.SP_OverlapDistance.setProperty("text", 0)
    self.SP_OverlapAngle.setProperty("value", 15)
    self.SP_Verbosity.setProperty("value", 3)
    
    self.CB_ComputedToleranceDisplacement.setChecked(True)
    self.CB_ComputedResolutionLength.setChecked(True)
    self.CB_ComputedOverlapDistance.setChecked(True)

__dialog=None
def getDialog():
  """
  This function returns a singleton instance of the plugin dialog.
  It is mandatory in order to call show without a parent ...
  """
  global __dialog
  if __dialog is None:
    __dialog = MGCleanerMonPlugDialog()
  #else :
  #   __dialog.clean()
  return __dialog


#
# ==============================================================================
# For memory
# ==============================================================================
#
def TEST_standalone():
  """
  works only if a salome is launched yet with a study loaded
  to launch standalone python do:
  ./APPLI/runSession
  python
  or (do not works)
  python ./INSTALL/SMESH/share/salome/plugins/smesh/MGCleanerMonPlugDialog.py
  """
  import salome
  import SMESH
  from salome.kernel import studyedit
  salome.salome_init()
  maStudy=studyedit.getActiveStudy()
  #etc...a mano...

#
# ==============================================================================
# Basic use cases and unit test functions
# ==============================================================================
#
def TEST_MGCleanerMonPlugDialog():
  import sys
  from qtsalome import QApplication
  app = QApplication(sys.argv)
  app.lastWindowClosed.connect(app.quit)

  dlg=MGCleanerMonPlugDialog()
  dlg.show()
  sys.exit(app.exec_())

if __name__ == "__main__":
  TEST_MGCleanerMonPlugDialog()
  #TEST_standalone()
  pass

