# -*- coding: utf-8 -*-

import logging

# -----------------------------------------------------------------------------
# --- maillage sain sans la zone defaut

def enleveDefaut(maillageSain, zoneDefaut, zoneDefaut_skin, zoneDefaut_internalFaces, zoneDefaut_internalEdges):
  """
  Maillage sain sans la zone de defaut
  TODO: a completer
  """
  logging.info('start')

  maillageSain.RemoveGroupWithContents(zoneDefaut)
  if zoneDefaut_skin is not None:
    maillageSain.RemoveGroupWithContents(zoneDefaut_skin)
  if zoneDefaut_internalFaces is not None:
    maillageSain.RemoveGroupWithContents(zoneDefaut_internalFaces)
  if zoneDefaut_internalEdges is not None:
    maillageSain.RemoveGroupWithContents(zoneDefaut_internalEdges)
  nbRemoved = maillageSain.RemoveOrphanNodes()
  return maillageSain

