include_directories(
  ../
  ${CMAKE_CURRENT_BINARY_DIR}
)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS avogadro)
set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

### Vibrations
avogadro_plugin(vibrationextension
  "vibrationextension.cpp;vibrationwidget.cpp"
  vibrationwidget.ui)

### Spectra
if(OPENBABEL_IS_NEWER_THAN_2_2_99)
  avogadro_plugin(spectraextension
  "spectraextension.cpp;spectradialog.cpp;spectratype.cpp;spectratype_ir.cpp;spectratype_nmr.cpp;spectratype_dos.cpp;spectratype_uv.cpp;spectratype_cd.cpp;spectratype_raman.cpp"
  "spectradialog.ui;spectratab_ir_raman.ui;spectratabnmr.ui;spectratabdos.ui;spectratabuv.ui;spectratabcd.ui")
else(OPENBABEL_IS_NEWER_THAN_2_2_99)
  avogadro_plugin(spectraextension
  "spectraextension.cpp;spectradialog.cpp;spectratype.cpp;spectratype_ir.cpp;spectratype_nmr.cpp"
  "spectradialog.ui;spectratab_ir_raman.ui;spectratabnmr.ui")
endif(OPENBABEL_IS_NEWER_THAN_2_2_99)