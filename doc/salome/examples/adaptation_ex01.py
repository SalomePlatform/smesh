# Remeshing with MG-Adapt
import os
import salome
salome.salome_init()

import SMESH
from salome.smesh import smeshBuilder
smesh = smeshBuilder.New()

# directory
rootdir = ...

# Test 1 : remeshing with a local size, surface
#--- Creation of the object for the adaptation ---
objet_adapt = smesh.Adaptation('MG_Adapt')

#--- Initial mesh ---
objet_adapt.setMEDFileIn(os.path.join(rootdir, "maill.01.med"))
#--- Final mesh ---
objet_adapt.setMEDFileOut(os.path.join(rootdir, "maill.01.adapt.med"))

#--- Creation of the hypothesis ---
hypo = smesh.CreateAdaptationHypothesis()
#    Options
hypo.setSizeMapType('Local')
hypo.setSizeMapFieldName('TAILLE')
hypo.setTimeStepRankLast()
hypo.setOptionValue("adaptation", "surface")

#--- Link between hypothesis and object ---
objet_adapt.AddHypothesis(hypo)

#--- Compute without publication
err = objet_adapt.Compute(False)



# Test 2 : remeshing with a background size, volume
#--- Creation of the object for the adaptation ---
objet_adapt = smesh.Adaptation('MG_Adapt')

#--- Initial mesh ---
objet_adapt.setMEDFileIn(os.path.join(rootdir, "maill.02.med"))
#--- Background mesh ---
objet_adapt.setMEDFileBackground(rootdir, "maill.size.02.med"))
#--- Final mesh ---
objet_adapt.setMEDFileOut(os.path.join(rootdir, "maill.02.adapt.med"))

#--- Creation of the hypothesis ---
hypo = smesh.CreateAdaptationHypothesis()
#    Options
hypo.setSizeMapType('Background')
hypo.setSizeMapFieldName('TAILLE')
hypo.setTimeStepRank(1,1)
hypo.setOptionValue("adaptation", "both")

#--- Link between hypothesis and object ---
objet_adapt.AddHypothesis(hypo)

#--- Compute without publication
err = objet_adapt.Compute(False)

