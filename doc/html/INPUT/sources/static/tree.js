foldersTree = gFld("<b>SALOME v.1.3.0 </b>", "", "")
     insDoc(foldersTree, gLnk("Main Page", "", "main.html"))

aux1 = insFld(foldersTree, gFld("TUI Reference Guide", ""))
  aux2 = insFld(aux1, gFld("Modules", ""))
    aux3 = insFld(aux2, gFld("SALOME SMESH module", ""))
/*!             insDoc(aux3, gLnk("Overview", "", "overview_SMESH.html"))*/
      aux4 = insFld(aux3, gFld("Packages", "")) 		
               insDoc(aux4, gLnk("SMESH", "", "namespaceSMESH.html"))
/*!             insDoc(aux3, gLnk("Examples", "", "examples_SMESH.html"))
*/

         insDoc(aux1, gLnk("Data Structures", "", "annotated.html"))
         insDoc(aux1, gLnk("Class Hierarchy", "", "hierarchy.html"))
         insDoc(aux1, gLnk("Class methods list", "", "functions.html"))
         insDoc(aux1, gLnk("Namespace Members", "", "namespacemembers.html"))
         insDoc(aux1, gLnk("File List", "", "files.html"))

aux1 = insFld(foldersTree, gFld("IDL/Python mapping", ""))
         insDoc(aux1, gLnk("Mapping of SMESH IDL definitions to Python language", "", "page2.html"))
