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

artifacts: rpm

rpm: $(TARGZ_FILE) $(SPEC_FILE) | $(RPM_BUILDROOT_SUBDIRS)
	rpmbuild -ba --define '_topdir $(RPM_TOPDIR)' --define 'version $(version)' $(SPEC_FILE)

$(TARGZ_FILE): $(ALL_FILES) | $(RPM_BUILDROOT_SUBDIRS)
	$(dir_create)
	tar czf $@ -T $(MANIFEST_FILE) --transform="s|^|$(name)-$(version)/|"

.PHONY: $(RPM_BUILDROOT_SUBDIRS)
$(RPM_BUILDROOT_SUBDIRS):
	mkdir -p $@

.PHONY: clean-artifacts
clean-artifacts:
	$(RM_rf) $(RPM_TOPDIR)

