# -*- coding: utf-8 -*-
# Copyright (C) 2007-2015  EDF R&D
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
from YamsPlugDialog_ui import Ui_YamsPlugDialog
from monViewText import MonViewText
from PyQt4.QtGui import *
from PyQt4.QtCore import *


class MonYamsPlugDialog(Ui_YamsPlugDialog,QWidget):
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
    #print "monYamsPlugDialog iconfolder",iconfolder
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

    #Ces parametres ne sont pas remis à rien par le clean
    self.paramsFile= os.path.abspath(os.path.join(os.environ["HOME"],".yams.dat"))
    self.LE_ParamsFile.setText(self.paramsFile)
    self.LE_MeshFile.setText("")
    self.LE_MeshSmesh.setText("")
    
    v1=QDoubleValidator(self)
    v1.setBottom(0.)
    #v1.setTop(1000.) #per thousand... only if relative
    v1.setDecimals(3)
    self.SP_Tolerance.setValidator(v1)
    self.SP_Tolerance.titleForWarning="Chordal Tolerance"
    
    self.resize(800, 600)
    self.clean()

  def connecterSignaux(self) :
    self.connect(self.PB_Cancel,SIGNAL("clicked()"),self.PBCancelPressed)
    self.connect(self.PB_Default,SIGNAL("clicked()"),self.clean)
    self.connect(self.PB_Help,SIGNAL("clicked()"),self.PBHelpPressed)
    self.connect(self.PB_OK,SIGNAL("clicked()"),self.PBOKPressed)
    
    self.connect(self.PB_Load,SIGNAL("clicked()"),self.PBLoadPressed)
    self.connect(self.PB_Save,SIGNAL("clicked()"),self.PBSavePressed)
    self.connect(self.PB_LoadHyp,SIGNAL("clicked()"),self.PBLoadHypPressed)
    self.connect(self.PB_SaveHyp,SIGNAL("clicked()"),self.PBSaveHypPressed)
    
    self.connect(self.PB_MeshFile,SIGNAL("clicked()"),self.PBMeshFilePressed)
    self.connect(self.PB_MeshSmesh,SIGNAL("clicked()"),self.PBMeshSmeshPressed)
    self.connect(self.LE_MeshSmesh,SIGNAL("returnPressed()"),self.meshSmeshNameChanged)
    self.connect(self.PB_ParamsFileExplorer,SIGNAL("clicked()"),self.setParamsFileName)
    self.connect(self.LE_MeshFile,SIGNAL("returnPressed()"),self.meshFileNameChanged)
    self.connect(self.LE_ParamsFile,SIGNAL("returnPressed()"),self.paramsFileNameChanged)

  def PBHelpPressed(self):
    import SalomePyQt
    sgPyQt = SalomePyQt.SalomePyQt()
    try :
      mydir=os.environ["SMESH_ROOT_DIR"]
    except Exception:
      QMessageBox.warning(self, "Help", "Help unavailable $SMESH_ROOT_DIR not found")
      return

    maDoc=mydir+"/share/doc/salome/gui/SMESH/yams/index.html"
    sgPyQt.helpContext(maDoc,"")
    
    #maDoc=mydir+"/share/doc/salome/gui/SMESH/yams/_downloads/mg-surfopt_user_manual.pdf"
    #command="xdg-open "+maDoc+";"
    #subprocess.call(command, shell=True)

  def PBOKPressed(self):
    if not(self.PrepareLigneCommande()):
      #warning done yet
      #QMessageBox.warning(self, "Compute", "Command not found")
      return
    maFenetre=MonViewText(self,self.commande)

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

    meshname = name+"_YAMS_"+str(self.num)
    smesh.SetName(outputMesh.GetMesh(), meshname)
    outputMesh.Compute() #no algorithms message for "Mesh_x" has been computed with warnings: -  global 1D algorithm is missing

    self.editor = studyedit.getStudyEditor()
    moduleEntry=self.editor.findOrCreateComponent("SMESH","SMESH")
    HypReMeshEntry = self.editor.findOrCreateItem(
        moduleEntry, name = "Plugins Hypotheses", icon="mesh_tree_hypo.png") #, comment = "HypoForRemeshing" )
    
    monStudyBuilder=maStudy.NewBuilder()
    monStudyBuilder.NewCommand()
    newStudyIter=monStudyBuilder.NewObject(HypReMeshEntry)
    self.editor.setAttributeValue(newStudyIter, "AttributeName", "YAMS Parameters_"+str(self.num))
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

    if salome.sg.hasDesktop(): salome.sg.updateObjBrowser(0)
    self.num+=1
    return True

  def PBSavePressed(self):
    from datetime import datetime
    if not(self.PrepareLigneCommande()): return
    text = "# YAMS hypothesis parameters\n" 
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

  def PBSaveHypPressed(self):
    """save hypothesis in Object Browser"""
    import salome
    import SMESH
    from salome.kernel import studyedit
    from salome.smesh import smeshBuilder
    smesh = smeshBuilder.New(salome.myStudy)

    maStudy=studyedit.getActiveStudy()
    smesh.SetCurrentStudy(maStudy)
    
    self.editor = studyedit.getStudyEditor()
    moduleEntry=self.editor.findOrCreateComponent("SMESH","SMESH")
    HypReMeshEntry = self.editor.findOrCreateItem(
        moduleEntry, name = "Plugins Hypotheses", icon="mesh_tree_hypo.png") #, comment = "HypoForRemeshing" )
    
    monStudyBuilder=maStudy.NewBuilder()
    monStudyBuilder.NewCommand()
    newStudyIter=monStudyBuilder.NewObject(HypReMeshEntry)
    self.editor.setAttributeValue(newStudyIter, "AttributeName", "YAMS Parameters_"+str(self.num))
    self.editor.setAttributeValue(newStudyIter, "AttributeComment", self.getResumeData(separator=" ; "))
    
    if salome.sg.hasDesktop(): salome.sg.updateObjBrowser(0)
    self.num+=1
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
    for RB in self.GBOptim.findChildren(QRadioButton,):
      if RB.isChecked()==True:
        text+="Optimisation="+RB.text()+separator
        break
    if self.RB_Absolute.isChecked():
      text+="Units=absolute"+separator
    else:
      text+="Units=relative"+separator
    v=self.SP_toStr(self.SP_Tolerance)
    text+="ChordalToleranceDeviation="+v+separator
    text+="RidgeDetection="+str(self.CB_Ridge.isChecked())+separator
    text+="SplitEdge="+str(self.CB_SplitEdge.isChecked())+separator
    text+="PointSmoothing="+str(self.CB_Point.isChecked())+separator
    text+="GeometricalApproximation="+str(self.SP_Geomapp.value())+separator
    text+="RidgeAngle="+str(self.SP_Ridge.value())+separator
    text+="MaximumSize="+str(self.SP_MaxSize.value())+separator
    text+="MinimumSize="+str(self.SP_MinSize.value())+separator
    text+="MeshGradation="+str(self.SP_Gradation.value())+separator
    text+="Verbosity="+str(self.SP_Verbosity.value())+separator
    text+="Memory="+str(self.SP_Memory.value())+separator
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
        if tit=="Optimisation":
          #no need: exlusives QRadioButton
          #for RB in self.GBOptim.findChildren(QRadioButton,):
          #  RB.setChecked(False)
          for RB in self.GBOptim.findChildren(QRadioButton,):
            if RB.text()==value :
              RB.setChecked(True)
              break
        if tit=="Units":
          if value=="absolute":
            self.RB_Absolute.setChecked(True)
            self.RB_Relative.setChecked(False)
          else:
            self.RB_Absolute.setChecked(False)
            self.RB_Relative.setChecked(True)
        if tit=="ChordalToleranceDeviation": self.SP_Tolerance.setProperty("text", float(value))
        if tit=="RidgeDetection": self.CB_Ridge.setChecked(value=="True")
        if tit=="SplitEdge": self.CB_SplitEdge.setChecked(value=="True")
        if tit=="PointSmoothing": self.CB_Point.setChecked(value=="True")
        if tit=="GeometricalApproximation": self.SP_Geomapp.setProperty("value", float(value))
        if tit=="RidgeAngle": self.SP_Ridge.setProperty("value", float(value))
        if tit=="MaximumSize": self.SP_MaxSize.setProperty("value", float(value))
        if tit=="MinimumSize": self.SP_MinSize.setProperty("value", float(value))
        if tit=="MeshGradation": self.SP_Gradation.setProperty("value", float(value))
        if tit=="Verbosity": self.SP_Verbosity.setProperty("value", int(float(value)))
        if tit=="Memory": self.SP_Memory.setProperty("value", float(value))
      except:
        QMessageBox.warning(self, "load YAMS Hypothesis", "Problem on '"+lig+"'")

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
    #QMessageBox.warning(self, "load Object Browser YAMS hypothesis", "TODO")
    import salome
    from salome.kernel import studyedit
    from salome.smesh.smeshstudytools import SMeshStudyTools
    from salome.gui import helper as guihelper
    from omniORB import CORBA

    mySObject, myEntry = guihelper.getSObjectSelected()
    if CORBA.is_nil(mySObject) or mySObject==None:
      QMessageBox.critical(self, "Hypothese", "select an Object Browser YAMS hypothesis")
      return
    
    text=mySObject.GetComment()
    
    #a verification
    if "Optimisation=" not in text:
      QMessageBox.critical(self, "Load Hypothese", "Object Browser selection not a YAMS Hypothesis")
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
      self.fichierIn=infile.toLatin1()
      self.MeshIn=""
      self.LE_MeshSmesh.setText("")

  def setParamsFileName(self):
    fd = QFileDialog(self, "select a file", self.LE_ParamsFile.text(), "dat Files (*.dat);;All Files (*)")
    if fd.exec_():
      infile = fd.selectedFiles()[0]
      self.LE_ParamsFile.setText(infile)
      self.paramsFile=infile.toLatin1()

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
      #QMessageBox.critical(self, "Mesh", "select an input mesh")
      self.LE_MeshSmesh.setText("")
      self.MeshIn=""
      self.LE_MeshFile.setText("")
      self.fichierIn=""
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
    self.fichierIn="/tmp/ForSurfOpt_"+str(self.num)+".meshb"
    self.__selectedMesh.ExportGMF(self.__selectedMesh, self.fichierIn, True)

  def PrepareLigneCommande(self):
    if self.fichierIn=="" and self.MeshIn=="":
      QMessageBox.critical(self, "Mesh", "select an input mesh")
      return False
    if self.__selectedMesh!=None: self.prepareFichier()
    if not (os.path.isfile(self.fichierIn)):
      QMessageBox.critical(self, "File", "unable to read GMF Mesh in "+str(self.fichierIn))
      return False
    
    self.commande="mg-surfopt.exe"
    
    for obj in self.GBOptim.findChildren(QRadioButton,):
      try:
        if obj.isChecked():
          self.style=obj.objectName().remove(0,3)
          self.style.replace("_","-")
          break
      except:
        pass
      
    style = self.style.toLatin1()
    # Translation of old Yams options to new MG-SurfOpt options
    if   style == "0" :
      self.commande+= " --optimisation only"
    elif style == "2" :
      self.commande+= " --Hausdorff_like yes"
    elif style == "-1":
      self.commande+= " --enrich no"
    elif style == "-2":
      self.commande+= " --Hausdorff_like yes --enrich no"
    elif style == "U" :
      self.commande+= " --uniform_flat_subdivision yes"
    elif style == "S" :
      self.commande+= " --sand_paper yes"
    elif style == "G" :
      self.commande+= " -O G"  # This option has not been updated to the new option style yet

    deb=os.path.splitext(self.fichierIn)
    self.fichierOut=deb[0] + "_output.meshb"
    
    tolerance=self.SP_toStr(self.SP_Tolerance)
    if not self.RB_Absolute.isChecked():
      tolerance+="r"  
    self.commande+=" --chordal_error %s"%tolerance
    
    if self.CB_Ridge.isChecked()    == False : self.commande+=" --compute_ridges no"
    if self.CB_Point.isChecked()    == False : self.commande+=" --optimisation no"
    if self.CB_SplitEdge.isChecked()== True  : self.commande+=" --element_order quadratic"
    if self.SP_Geomapp.value()      != 15.0  : self.commande+=" --geometric_approximation_angle %f"%self.SP_Geomapp.value()
    if self.SP_Ridge.value()        != 45.0  : self.commande+=" --ridge_angle %f"%self.SP_Ridge.value()
    if self.SP_MaxSize.value()      != 100   : self.commande+=" --max_size %f"   %self.SP_MaxSize.value()
    if self.SP_MinSize.value()      != 5     : self.commande+=" --min_size %f"   %self.SP_MinSize.value()
    if self.SP_Gradation.value()    != 1.3   : self.commande+=" --gradation %f"  %self.SP_Gradation.value()
    if self.SP_Memory.value()       != 0     : self.commande+=" --max_memory %d" %self.SP_Memory.value()
    if self.SP_Verbosity.value()    != 3     : self.commande+=" --verbose %d" %self.SP_Verbosity.value()

    self.commande+=" --in "  + self.fichierIn
    self.commande+=" --out " + self.fichierOut
    
    print self.commande
    return True

  def clean(self):
    self.RB_0.setChecked(True)
    #no need: exlusives QRadioButton
    #self.RB_G.setChecked(False)
    #self.RB_U.setChecked(False)
    #self.RB_S.setChecked(False)
    #self.RB_2.setChecked(False)
    #self.RB_1.setChecked(False)
    self.RB_Relative.setChecked(True)
    #no need: exlusives QRadioButton
    #self.RB_Absolute.setChecked(False)
    self.SP_Tolerance.setProperty("text", "0.001")
    self.SP_Geomapp.setProperty("value", 15.0)
    self.SP_Ridge.setProperty("value", 45.0)
    self.SP_Gradation.setProperty("value", 1.3)
    self.CB_Ridge.setChecked(True)
    self.CB_Point.setChecked(True)
    self.CB_SplitEdge.setChecked(False)
    self.SP_MaxSize.setProperty("value", 100)
    self.SP_MinSize.setProperty("value", 5)
    self.SP_Verbosity.setProperty("value", 3)
    self.SP_Memory.setProperty("value", 0)
    self.PBMeshSmeshPressed()
    self.TWOptions.setCurrentIndex(0) # Reset current active tab to the first tab

__dialog=None
def getDialog():
  """
  This function returns a singleton instance of the plugin dialog.
  It is mandatory in order to call show without a parent ...
  """
  global __dialog
  if __dialog is None:
    __dialog = MonYamsPlugDialog()
  else :
    __dialog.clean()
  return __dialog

#
# ==============================================================================
# Basic use cases and unit test functions
# ==============================================================================
#
def TEST_MonYamsPlugDialog():
  import sys
  from PyQt4.QtGui import QApplication
  from PyQt4.QtCore import QObject, SIGNAL, SLOT
  app = QApplication(sys.argv)
  QObject.connect(app, SIGNAL("lastWindowClosed()"), app, SLOT("quit()"))

  dlg=MonYamsPlugDialog()
  dlg.show()
  sys.exit(app.exec_())

if __name__ == "__main__":
  TEST_MonYamsPlugDialog()
  pass
