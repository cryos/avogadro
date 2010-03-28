from PyQt4.Qt import *
import Avogadro

class Tool(QObject):
  # constructor
  def __init__(self):
    QObject.__init__(self)

  # widget = GLWidget
  def paint(self, widget):
    # Painter
    # print("paint(", widget, ")")
    return None

  # widget = GLWidget
  # mouseEvent = QMouseEvent
  def mousePressEvent(self, widget, mouseEvent):
    # print("mousePressEvent(", widget, ",", mouseEvent, ")")
    # mouseEvent.accept()
    return None
  
  def mouseMoveEvent(self, widget, mouseEvent):
    # print("mouseMoveEvent(", widget, ",", mouseEvent, ")")
    # mouseEvent.accept()
    return None

  def mouseReleaseEvent(self, widget, mouseEvent):
    # print("mouseReleaseEvent(", widget, ",", mouseEvent, ")")
    # mouseEvent.accept()
    return None

  def wheelEvent(self, widget, wheelEvent):
    # print("wheelEvent(", widget, ",", wheelEvent, ")")
    # wheelEvent.accept()
    return None
