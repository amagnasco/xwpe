if HAVE_DOXYGEN
directory = $(top_srcdir)/docs

dist_man_MANS = $(directory)/man/man3
$(directory)/man3: doxyfile.stamp

doxyfile.stamp: Doxyfile
	$(DOXYGEN) $^
	cd latex && $(MAKE)
	echo Timestamp > $@

CLEANFILES = doxyfile.stamp

all-local: doxyfile.stamp
clean-local:
	rm -rf $(directory)/man

endif
