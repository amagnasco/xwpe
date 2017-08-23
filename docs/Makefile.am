if HAVE_DOXYGEN
directory = $(top_builddir)/docs

man_MANS = $(directory)/man/man3
$(directory)/man/man3: doxyfile.stamp

doxyfile.stamp: Doxyfile
	$(DOXYGEN) $^
	cd latex && $(MAKE) > latexrun.log
	echo Timestamp > $@
	cp ./latex/refman.pdf $(directory)/xwpe-refman.pdf

CLEANFILES = doxyfile.stamp $(directory)/xwpe-refman.pdf

all-local: doxyfile.stamp
clean-local:
	rm -rf $(directory)/man/man3
	rm -rf $(directory)/latex
	rm -rf $(directory)/html

endif
