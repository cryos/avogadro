%Module AvoSIP 0

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtOpenGL/QtOpenGLmod.sip

%If (Qt_4_3_0 -)

%Include openbabel.sip
%Include eigen.sip
%Include primitive.sip
%Include primitivelist.sip
%Include tool.sip
%Include toolgroup.sip
%Include color.sip
%Include camera.sip
%Include painter.sip
%Include painterdevice.sip
%Include engine.sip
%Include glwidget.sip


%End
