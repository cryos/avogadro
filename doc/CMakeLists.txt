# find the required tools
find_program(DOCBOOK2PDF_EXECUTABLE docbook2pdf)
find_program(DOCBOOK2HTML_EXECUTABLE docbook2html)

if( DOCBOOK2PDF_EXECUTABLE AND DOCBOOK2HTML_EXECUTABLE )
   # create a target to build pdf docs
   add_custom_target(pdfdocs COMMAND ${DOCBOOK2PDF_EXECUTABLE} -o pdf
      ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook )

   # create a target to build html docs
   add_custom_target(htmldocs COMMAND ${DOCBOOK2HTML_EXECUTABLE} -o html
      ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook )

   # create a helper target to build both
   # if this is always wanted, the two custom targets above
   # can also be combined into one custom target with two COMMANDs
   #add_custom_target(alldocs ALL)
   add_custom_target(alldocs)
   add_dependencies(alldocs pdfdocs htmldocs)

   #FILE(GLOB avogadro_HTML RELATIVE ${CMAKE_CURRENT_BINARY_DIR}/html/ "*.html")
   #INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/pdf/index.pdf  DESTINATION share/doc/avogadro/pdf/ OPTIONAL)
   #INSTALL(FILES ${avogadro_HTML}  DESTINATION share/doc/avogadro/html/ OPTIONAL)

else( DOCBOOK2PDF_EXECUTABLE AND DOCBOOK2HTML_EXECUTABLE )
   message(STATUS "docbook tools not found, doc targets disabled")
endif( DOCBOOK2PDF_EXECUTABLE AND DOCBOOK2HTML_EXECUTABLE )

# Manpages
if(UNIX)
   file(GLOB man_1 *.1)
   install(FILES ${man_1} DESTINATION share/man/man1)
endif(UNIX)
