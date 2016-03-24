# -*- coding: utf-8 -*-

class fissError(Exception):
  """
  usage:
  try:
    instructions()
  except:
    raise fissError(traceback.extract_stack(),"mon message")  
  """
  def __init__(self, pile, msg):
    self.pile = pile    
    self.msg = msg

  def __str__(self):
    return 'msg=%s\npile=%s\n'%(self.msg, repr(self.pile))
  
