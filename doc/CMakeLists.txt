file(GLOB _docs *.docbook)

set(_htmlDoc ${CMAKE_CURRENT_SOURCE_DIR}/index.html)
set(_doc ${CMAKE_CURRENT_BINARY_DIR}/index.cache.bz2)
set(_proc /usr/bin/meinproc4)

#here is missing the command to fianlly run proc to "compile" the PDF/HTML output...
