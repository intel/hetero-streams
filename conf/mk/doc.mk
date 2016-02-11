#                                                                            #
# Hetero Streams Library - A streaming library for heterogeneous platforms   #
# Copyright (c) 2014 - 2016, Intel Corporation.                              #
#                                                                            #
# This program is free software; you can redistribute it and/or modify it    #
# under the terms and conditions of the GNU Lesser General Public License,   #
# version 2.1, as published by the Free Software Foundation.                 #
#                                                                            #
# This program is distributed in the hope it will be useful, but WITHOUT ANY #
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  #
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for   #
# more details.                                                              #
#                                                                            #

DOC_FILES := hStreams_Overview.pdf \
   hStreams_Reference.pdf \
   hStreams_Release_Notes.pdf \
   hStreams_Reference_Codes.pdf \
   hStreams_Porting_Guide.pdf
DOCS := $(addprefix $(DOC_DIR), $(DOC_FILES))

MAN_PAGES_INSTALL_DIR:=/usr/share/man/man3/
DOCS_INSTALL_DIR:=/usr/share/doc/hStreams/
REF_CODE_INSTALL_DIR:=$(DOCS_INSTALL_DIR)/ref_code/

# Last command will clean up some man links to class members
.PHONY: build-doc
build-doc:
	mkdir -p $(BLD_DIR)doxygen
	cd $(TOP_DIR) ; $(DOXYGEN) $(CFG_DIR)doxygen.rc
	$(MAKE) -C $(BLD_DIR)doxygen/latex pdf
	rm -rf $(DOC_DIR)hStreams_Reference.pdf
	mv $(BLD_DIR)doxygen/latex/refman.pdf $(DOC_DIR)hStreams_Reference.pdf
	find $(BLD_DIR)doxygen/man/man3/ -type f ! -iname '*HSTR*' -delete

.PHONY: clean-doc
clean-doc:
	$(RM) -r $(BLD_DIR)doxygen

# FIXME We don't install the freshly generated PDF reference manual here.
#       Instead, a stale copy embedded in the git repository is used.
.PHONY: install-doc
install-doc: build-doc
	install -d $(DESTDIR)$(DOCS_INSTALL_DIR)
	install -m644 $(DOCS) $(DESTDIR)$(DOCS_INSTALL_DIR)
	install -d $(DESTDIR)$(REF_CODE_INSTALL_DIR)
	cp -dr --no-preserve=ownership $(REF_CODE_DIR)* $(DESTDIR)$(REF_CODE_INSTALL_DIR)
	$(RM) -r $(DESTDIR)$(REF_CODE_INSTALL_DIR)windows
	$(RM) -r $(DESTDIR)$(REF_CODE_INSTALL_DIR)*/*.bat
	install -d $(DESTDIR)$(MAN_PAGES_INSTALL_DIR)
	cp -dr --no-preserve=ownership $(BLD_DIR)doxygen/man/man3/* $(DESTDIR)$(MAN_PAGES_INSTALL_DIR)
