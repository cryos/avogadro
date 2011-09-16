#
#	wiiuse Makefile
#

all clean install:
	@$(MAKE) -C src $@
	@$(MAKE) -C example $@
	@$(MAKE) -C pywii $@

wiiusecpp:
	@$(MAKE) -C src

ex:
	@$(MAKE) -C example

pywii:
	@$(MAKE) -C pywii

distclean:
	@$(MAKE) -C src $@
	@$(MAKE) -C example $@
	@$(MAKE) -C pywii $@
	@find . -type f \( -name "*.save" -or -name "*~" -or -name gmon.out \) -delete &> /dev/null
