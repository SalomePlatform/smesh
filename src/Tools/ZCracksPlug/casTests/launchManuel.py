
import os, tempfile

directory=tempfile.mktemp(prefix='tmpZcracks')
print "directory=", tmpdir

# Tous les cas
listCas=['1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16','17','18','19','20','21']

# Cas sans les aretes ou faces sur la fissure :
listCas=['1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16','17','20']

synthese={}

for cas in listCas:
  result=os.path.join(directory,'cracked'+cas+'.geo')
  if os.path.isfile(result):  os.remove(result)
  try:
    os.remove(os.path.join(directory,'cracked'+cas+'.geo'))
  except:
    pass
  os.system('cd '+directory+';Zrun -zp insert'+cas+'.z7p')

  synthese[cas]= os.path.isfile(result)

print synthese

