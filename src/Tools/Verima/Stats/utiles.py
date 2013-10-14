#-----------------------------#
def writeFile( fn, txt = None):
#-----------------------------#

   if txt == None : return
   if fn  == None : return
   fn = unicode(fn)
   try:
      f = open(fn, 'wb')
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
      f = open(fn, 'wb')
      txt=f.read()
      f.close()
   except IOError:
      txt=""
   return txt

