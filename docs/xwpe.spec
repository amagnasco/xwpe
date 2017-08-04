Summary: X Windows Programming Environment
Name: xwpe
%define	version	1.5.30a
Version: %{version}
Release: 1
License: GPL
Url: http://www.identicalsoftware.com/xwpe
Group: Development/Tools
#Source: ftp://ftp.rrzn.uni-hannover.de/pub/systems/unix/xwpe/xwpe-1.4.2.tar.Z
Source: http://www.identicalsoftware.com/xwpe/xwpe-%{version}.tar.gz
Prefix: /usr
BuildRoot: %{_tmppath}/%{name}-root

%description
XWPE is actually a package of four programs: we, wpe, xwe, and xwpe.
They are different versions of the same basic programmers editor and
development environment. If you have used some of the Microsoft
Windows programming IDE's and longed for an X Windows equivalent, this
is what you have been looking for! Also included are the text-mode
equivalents of the X programs, enabling you to use xwpe no matter what
your development environment may be.

This package includes the basic xwpe libraries and the text-mode programs;
the X Windows programs are contained in the 'xwpe-X11' package.

%package X11
Summary: X Windows Programming Environment - X11 programs
Group: X11/Applications/Development
Requires: xwpe

%description X11

Includes the 'xwpe' and 'xwe' programs from the xwpe package that are
specific to X Windows.

%prep
%setup -q

%build
patch -p1 <xwpe-conf.patch
autoconf
./configure --prefix=%{prefix} --mandir=%{_mandir}
make 

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr

make prefix=$RPM_BUILD_ROOT/usr MANDIR=$RPM_BUILD_ROOT%{_mandir} install
desktop-file-install --vendor=identical --dir=%{buildroot}%{_datadir}/applications *.desktop
( cd $RPM_BUILD_ROOT/usr/bin
  strip we
)

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README CHANGELOG
/usr/bin/we
/usr/bin/wpe
/usr/lib/xwpe/help.key
/usr/lib/xwpe/help.xwpe
/usr/lib/xwpe/syntax_def
/usr/lib/xwpe/libxwpe-term.so
%{_mandir}/man1/wpe.1*
%{_mandir}/man1/we.1*
%{_mandir}/man1/xwpe.1*
%{_mandir}/man1/xwe.1*

%files X11
%defattr(-,root,root)
/usr/bin/xwe
/usr/bin/xwpe
/usr/lib/xwpe/libxwpe-x11.so
%{_datadir}/applications/*.desktop

