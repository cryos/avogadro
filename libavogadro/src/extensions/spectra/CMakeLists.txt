include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LINK_LIBS avogadrowidget avogadrocore)
set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

### Vibrations
avogadro_plugin_nogl(vibrationextension
  "vibrationextension.cpp;vibrationwidget.cpp"
  vibrationwidget.ui)

### Spectra
if(OPENBABEL_IS_NEWER_THAN_2_2_99)
  avogadro_plugin_nogl(spectraextension
  "spectraextension.cpp;spectradialog.cpp;spectratype.cpp;abstract_ir.cpp;ir.cpp;nmr.cpp;dos.cpp;uv.cpp;cd.cpp;raman.cpp"
  "spectradialog.ui;tab_ir_raman.ui;tab_nmr.ui;tab_dos.ui;tab_uv.ui;tab_cd.ui")
else(OPENBABEL_IS_NEWER_THAN_2_2_99)
  avogadro_plugin_nogl(spectraextension
  "spectraextension.cpp;spectradialog.cpp;spectratype.cpp;abstract_ir.cpp;ir.cpp;nmr.cpp"
  "spectradialog.ui;tab_ir_raman.ui;tab_nmr.ui")
endif(OPENBABEL_IS_NEWER_THAN_2_2_99)
