# Removing Elements

from mechanic import *

# remove three elements: #850, #859 and #814
res = mesh.RemoveElements([850, 859, 814])
if res == 1: print("Elements removing is OK!")
else:        print("KO Elements removing.")
