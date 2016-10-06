# -*- coding: iso-8859-1 -*-
# Copyright (C) 2011-2016  EDF R&D
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
# Author : Guillaume Boulant (EDF)
#

from qtsalome import QDialog, QIcon, Qt

from plugindialog_ui import Ui_PluginDialog
from inputdialog import InputDialog
from inputdata import InputData
# __GBO__: uncomment this line and comment the previous one to use the
# demo input dialog instead of the real one.
#from demoinputdialog import InputDialog

import os
import salome
from salome.kernel import studyedit
from salome.kernel.uiexception import AdminException

from omniORB import CORBA
import SMESH
from salome.smesh import smeshBuilder
smesh = smeshBuilder.New(salome.myStudy)
import MESHJOB

gui_states = ["CAN_SELECT", "CAN_COMPUTE", "CAN_REFRESH", "CAN_PUBLISH"]

run_states = ["CREATED", "IN_PROCESS", "QUEUED", "RUNNING", "PAUSED"];
end_states = ["FINISHED", "ERROR"]
all_states = run_states+end_states;

# The SALOME launcher resource is specified by its name as defined in
# the file CatalogResources.xml (see root directory of the
# application). We could have a check box in the dialog to specify
# whether we want a local execution or a remote one.
resource_name = "localhost"
from salome.smesh.spadder.configreader import ConfigReader


class PluginDialog(QDialog):

    def __init__(self,parent = None,name = None,modal = 0,fl = 0):
        QDialog.__init__(self,parent)
        # Set up the user interface from Designer.
        self.__ui = Ui_PluginDialog()
        self.__ui.setupUi(self)

        # The default display strategy is to use a separate dialog box
        # to select the input data.
        self.viewInputFrame(False)

        # The icon are supposed to be located in the plugin folder,
        # i.e. in the same folder than this python module file
        iconfolder=os.path.dirname(os.path.abspath(__file__))
        icon = QIcon()
        icon.addFile(os.path.join(iconfolder,"input.png"))
        self.__ui.btnInput.setIcon(icon)
        icon = QIcon()
        icon.addFile(os.path.join(iconfolder,"compute.png"))
        self.__ui.btnCompute.setIcon(icon)
        icon = QIcon()
        icon.addFile(os.path.join(iconfolder,"refresh.png"))
        self.__ui.btnRefresh.setIcon(icon)
        icon = QIcon()
        icon.addFile(os.path.join(iconfolder,"publish.png"))
        self.__ui.btnPublish.setIcon(icon)
        icon = QIcon()
        icon.addFile(os.path.join(iconfolder,"clear.png"))
        self.__ui.btnClear.setIcon(icon)

        # Then, we can connect the slot to there associated button event
	self.__ui.btnInput.clicked.connect( self.onInput )
        self.__ui.btnCompute.clicked.connect( self.onCompute )
        self.__ui.btnRefresh.clicked.connect( self.onRefresh )
        self.__ui.btnPublish.clicked.connect( self.onPublish )
        self.__ui.btnClear.clicked.connect( self.onClear )

        self.clear()

        self.setupJobManager()
        

    def setupJobManager(self):
        '''
        This function configures the jobmanager by transmiting the
        parameters required for a local execution and a remote
        execution. The choice between "local" and "remote" is done at
        the initialize step, by specifing the name of the resource to
        be used.
        '''
        # We first 
        
        configReader = ConfigReader()
        config = configReader.getLocalConfig()
        configId = config.resname
        self.__getJobManager().configure(configId, config)
        # Note that the resname parameter is used as the key identifier of
        # the configuration in the job manager. As is, there can be then
        # only one configuration for each machine defined in the resources
        # catalog (no need to have further, I thing)
        config = configReader.getRemoteConfig()
        configId = config.resname
        self.__getJobManager().configure(configId, config)

        # We specify the default configuration identifier as the
        # resource name of the default configuration
        self.__configId = configReader.getDefaultConfig().resname


    def viewInputFrame(self, view=True):
        # By default, the top input frame is visible and the input
        # button is not visible.
        if view is False:
            self.__ui.frameInput.setVisible(False)
            self.__ui.btnInput.setVisible(True)
            # We create the input dialog that will be displayed when
            # button input is pressed:
            self.__inputDialog = InputDialog(self)
            # The window is kept on the top to ease the selection of
            # items in the object browser:
            self.__inputDialog.setWindowFlags(
                self.__inputDialog.windowFlags() | Qt.WindowStaysOnTopHint)
            # The signal inputValidated emited from inputDialog is
            # connected to the slot function onProcessInput:
    	    self.__inputDialog.inputValidated.connect( self.onProcessInput )
            
        else:
            self.__ui.frameInput.setVisible(True)
            self.__ui.btnInput.setVisible(False)
            # This case is NOT IMPLEMENTED YET (not really). It could
            # be used to draw the input frame directly in the frame
            # frameInput of this dialog box.

    def getInputFrame(self):
        return self.__ui.frameInput
        
    def __setGuiState(self,states=["CAN_SELECT"]):
        if "CAN_SELECT" in states:
            self.__ui.btnInput.setEnabled(True)
        else:
            self.__ui.btnInput.setEnabled(False)
            
        if "CAN_COMPUTE" in states:
            self.__ui.btnCompute.setEnabled(True)
        else:
            self.__ui.btnCompute.setEnabled(False)

        if "CAN_REFRESH" in states:
            self.__ui.btnRefresh.setEnabled(True)
        else:
            self.__ui.btnRefresh.setEnabled(False)

        if "CAN_PUBLISH" in states:
            self.__ui.btnPublish.setEnabled(True)
        else:
            self.__ui.btnPublish.setEnabled(False)

    def __getJobManager(self):
        """
        This function requests a pointer to the MeshJobManager
        servant. Note that the component is loaded on first demand,
        and then the reference is recycled.
        """
        if self.__dict__.has_key("__jobManager") and self.__jobManager is not None:
            return self.__jobManager

        # WARN: we first have to update the SALOME components catalog
        # with the SPADDER components (because they are not defined in
        # the SMESH catalog, and then they are not in the default
        # catalog)
        from salome.smesh import spadder
        spadder.loadSpadderCatalog()
        # Then we can load the MeshJobManager component
        component=salome.lcc.FindOrLoadComponent("FactoryServer","MeshJobManager")
        if component is None:
            msg="ERR: the SALOME component MeshJobManager can't be reached"
            self.__log(msg)
            raise AdminException(msg)

        self.__jobManager = component
        return self.__jobManager

    def __log(self, message):
        """
        This function prints the specified message in the log area
        """ 
        self.__ui.txtLog.append(message)

    def __exportMesh(self, meshName, meshObject):
        '''
        This function exports the specified mesh object to a med
        file whose name (basepath) is built from the specified mesh
        name. This returns the filename.
        '''
        filename=str("/tmp/padder_inputfile_"+meshName+".med")
        meshObject.ExportToMEDX( filename, 0, SMESH.MED_V2_2, 1, 1 )
        return filename

    def clear(self):
        """
        This function clears the log area and the states of the buttons
        """
        self.__listInputData = []
        self.__ui.txtLog.clear()
        self.__setGuiState(["CAN_SELECT"])
        self.__isRunning = False
        self.__ui.lblStatusBar.setText("Ready")

    def update(self):
        '''
        This function can be used to programmatically force the
        refresh of the dialog box, the job state in particular.
        '''
        if self.__isRunning:
            self.onRefresh()

    def onInput(self):
        '''
        This function is the slot connected to the Input button
        (signal clicked()). It opens the dialog window to input
        data. The dialog is opened in a window modal mode so that the
        SALOME study objects can be selected. In conterpart, this
        class must listen to signals emitted by the child dialog
        windows to process the validation event (see the slot
        onProcessInput which is connected to this event).
        '''
        self.__inputDialog.setData(self.__listInputData)
        self.__inputDialog.open()

    def onProcessInput(self):
        """
        This function is the slot connected to the signal
        inputValidated(), emit by the input dialog window when it's
        validated, i.e. OK is pressed and data are valid.
        """
        # The processing simply consists in requesting the input data
        # from the dialog window.
        self.__listInputData = self.__inputDialog.getData()
        self.__ui.lblStatusBar.setText("Input data OK")
        self.__log("INF: Press \"Compute\" to start the job")
        self.__setGuiState(["CAN_SELECT", "CAN_COMPUTE"])
        
    def onCompute(self):
        '''
        This function is the slot connected to the Compute button. It
        initializes a mesh computation job and start it using the
        SALOME launcher.  
        '''
        # We first have to create the list of parameters for the
        # initialize function. For that, we have to create the files
        # from the mesh objects:
        meshJobParameterList=[]
        concreteIndex=0
        for inputData in self.__listInputData:
            # For each input data, we have to create a
            # MeshJobParameter and add it to the list.
            filename  = self.__exportMesh(inputData.meshName, inputData.meshObject)
            if inputData.meshType == InputData.MESHTYPES.CONCRETE:
                filetype = MESHJOB.MED_CONCRETE
            else:
                filetype = MESHJOB.MED_STEELBAR

            parameter = MESHJOB.MeshJobParameter(
                file_name  = filename,
                file_type  = filetype,
                group_name = inputData.groupName)
            meshJobParameterList.append(parameter)

        jobManager = self.__getJobManager()
        self.__jobid = jobManager.initialize(meshJobParameterList, self.__configId)
        if self.__jobid < 0:
            self.__log("ERR: the job can't be initialized")
            self.__log("ERR: %s"%jobManager.getLastErrorMessage())
            return
        self.__log("INF: the job has been initialized with jobid = "+str(self.__jobid))
        
        startOk = jobManager.start(self.__jobid)
        if not startOk:
            self.__log("ERR: the job with jobid = "+str(self.__jobid)+" can't be started")
            self.__log("ERR: %s"%jobManager.getLastErrorMessage())
            return
        self.__log("INF: the job "+str(self.__jobid)+" has been started")
        self.__ui.lblStatusBar.setText("Submission OK")
        self.__setGuiState(["CAN_REFRESH"])
        self.__isRunning = True

    def onRefresh(self):
        """
        This function is the slot connected on the Refresh button. It
        requests the mesh job manager to get the state of the job and
        display it in the log area.
        """
        jobManager = self.__getJobManager()
        state = jobManager.getState(self.__jobid)
        self.__log("INF: job state = "+str(state))
        self.__ui.lblStatusBar.setText("")
        if state in run_states:
            return

        self.__isRunning = False
        if state == "FINISHED":
            self.__setGuiState(["CAN_PUBLISH"])
        else:
            self.__setGuiState(["CAN_SELECT"])


    def onPublish(self):
        """
        This function is the slot connected on the Publish button. It
        requests the mesh job manager to download the results data
        from the computation resource host and load the med file in
        the SALOME study. 
        """
        jobManager = self.__getJobManager()
        state = jobManager.getState(self.__jobid)
        if state in run_states:
            self.__log("WRN: jobid = "+str(self.__jobid)+" is not finished (state="+str(state)+")")
            return

        if state not in end_states:
            self.__log("ERR: jobid = "+str(self.__jobid)+" ended abnormally with state="+str(state))
            self.__log("ERR: %s"%jobManager.getLastErrorMessage())
            return

        meshJobResults = jobManager.finalize(self.__jobid)
        logsdirname = os.path.join(meshJobResults.results_dirname, "logs")
        if state == "ERROR" or meshJobResults.status is not True:
            msgtemp = "ERR: jobid = %s ended with error: %s"
            self.__log(msgtemp%(str(self.__jobid),jobManager.getLastErrorMessage()))
            self.__log("ERR: see log files in %s"%logsdirname)
            return

        self.__log("INF:  jobid=%s ended normally (see log files in %s)"%(str(self.__jobid),logsdirname))

        medfilename = os.path.join(meshJobResults.results_dirname,
                                   meshJobResults.outputmesh_filename)

        smesh.SetCurrentStudy(studyedit.getActiveStudy())
        ([outputMesh], status) = smesh.CreateMeshesFromMED(medfilename)

        # By convention, the name of the output mesh in the study is
        # build from a constant string 'padder' and the session jobid
        meshname = 'padder_'+str(self.__jobid)
        smesh.SetName(outputMesh.GetMesh(), meshname)
        if salome.sg.hasDesktop():
            salome.sg.updateObjBrowser(False)

        self.__ui.lblStatusBar.setText("Publication OK")
        self.__setGuiState(["CAN_SELECT"])

    def onClear(self):
        """
        This function is the slot connected on the Clear button. It
        erases data in the dialog box and cancel the current job if
        one is running.
        """
        self.clear()
        


__dialog=None
def getDialog():
    """
    This function returns a singleton instance of the plugin dialog. 
    """
    global __dialog
    if __dialog is None:
        __dialog = PluginDialog()
    return __dialog

#
# ==============================================================================
# Basic use cases and unit test functions
# ==============================================================================
#
def TEST_PluginDialog():
    import sys
    from qtsalome import QApplication
    app = QApplication(sys.argv)
    app.lastWindowClosed.connect( app.quit )

    dlg=PluginDialog()
    dlg.exec_()

if __name__ == "__main__":
    TEST_PluginDialog()

        

