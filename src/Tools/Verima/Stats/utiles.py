#-----------------------------#
def writeFile( fn, txt = None):
#-----------------------------#

   if txt == None : return
   if fn  == None : return
   fn = str(fn)
   try:
      f = open(fn, 'w')
      f.write(txt)
      f.close()
      return 1
   except IOError:
      return 0


#---------------------#
def readFile( fn, txt):
#---------------------#

   if fn  == None : return
   try:
      f = open(fn, 'w')
      txt=f.read()
      f.close()
   except IOError:
      txt=""
   return txt

