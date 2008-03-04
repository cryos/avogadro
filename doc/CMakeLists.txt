# find the required tools
find_program(DOCBOOK2PDF_EXECUTABLE docbook2pdf)
find_program(DOCBOOK2HTML_EXECUTABLE docbook2html)

if("${DOCBOOK2PDF_EXECUTABLE}" AND "${DOCBOOK2HTML_EXECUTABLE}" )
   # create a target to build pdf docs
   add_custom_target(pdfdocs COMMAND ${DOCBOOK2PDF_EXECUTABLE} -o pdf
      ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook )

   # create a target to build html docs
   add_custom_target(htmldocs COMMAND ${DOCBOOK2PDF_EXECUTABLE} -o html
      ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook )

   # create a helper target to build both
   # if this is always wanted, the two custom targets above
   # can also be combined into one custom target with two COMMANDs
   add_custom_target(alldocs)
   add_dependencies(alldocs pdfdocs htmldocs)

else("${DOCBOOK2PDF_EXECUTABLE}" AND "${DOCBOOK2HTML_EXECUTABLE}" )
   message(STATUS "docbook tools not found, doc targets disabled")
endif("${DOCBOOK2PDF_EXECUTABLE}" AND "${DOCBOOK2HTML_EXECUTABLE}" )
