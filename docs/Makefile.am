if HAVE_DOXYGEN
directory = $(top_srcdir)/docs

man_MANS = $(directory)/man/man3
$(directory)/man/man3: doxyfile.stamp
	%.3

doxyfile.stamp: Doxyfile
	$(DOXYGEN) $^
	cd latex && $(MAKE)
	echo Timestamp > $@

CLEANFILES = doxyfile.stamp

all-local: doxyfile.stamp
clean-local:
	rm -rf $(directory)/man/man3

endif
