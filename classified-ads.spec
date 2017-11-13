Name:		classified-ads
Version:	0.12
Release:	1%{?dist}
Summary:	Classified ads is distributed, server-less messaging system

Group:		Applications/Internet
License:	LGPLv2
URL:		http://katiska.org/classified_ads/
Source0:	https://github.com/operatornormal/%{name}/archive/%{version}.tar.gz#/%{name}-%{version}.tar.gz
Source1:	https://github.com/operatornormal/classified-ads/blob/graphics/preprocessed.tar.gz?raw=true#/%{name}-graphics-%{version}.tar.gz
BuildRequires:	qt5-qtbase-devel
BuildRequires:	qt5-qtmultimedia-devel
BuildRequires:	openssl-devel
BuildRequires:	libnatpmp-devel
BuildRequires:	miniupnpc-devel
BuildRequires:	gettext
BuildRequires:	libappstream-glib
BuildRequires:	desktop-file-utils
BuildRequires:	opus-devel
BuildRequires:	tcl-devel
BuildRequires:	tk-devel
BuildRequires:	zlib-devel

%description
Classified ads is an attempt to re-produce parts of the functionality
that went away when Usenet news ceased to exist. This attempt tries to
fix the problem of disappearing news-servers so that there is no servers
required and no service providers needed; data storage is implemented
inside client applications that users are running. Main feature is
public posting. Other features include private messages, group messages,
basic operator data, search, voice calls between nodes, UI extensions
with TCL language and general-purpose database shared between nodes of the 
application. 
%prep
%setup -q -a 1

%build
qmake-qt5 QMAKE_STRIP=echo
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}
appstream-util validate-relax --nonet %{buildroot}/%{_datadir}/appdata/classified-ads.appdata.xml
desktop-file-validate %{buildroot}/%{_datadir}/applications/classified-ads.desktop
%find_lang %{name}

%post
/usr/bin/update-desktop-database &> /dev/null || :
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :

%postun
/usr/bin/update-desktop-database &> /dev/null || :
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :

%files -f %{name}.lang
%doc README.TXT
%{_bindir}/classified-ads
%{_datadir}/applications/classified-ads.desktop
%dir %{_datadir}/app-install
%dir %{_datadir}/app-install/icons
%{_datadir}/app-install/icons/turt-transparent-128x128.png
%{_mandir}/man1/classified-ads.1.*
%{_datadir}/appdata/classified-ads.appdata.xml
%license LICENSE
%{_datadir}/doc/classified-ads/examples/sysinfo.tcl
%{_datadir}/doc/classified-ads/examples/luikero.tcl
%{_datadir}/doc/classified-ads/examples/calendar.tcl

%changelog
* Sun Nov 12 2017 Antti Jarvinen <antti.jarvinen@katiska.org> - 0.12-1
- New upstream version: new features, many new translations

* Sun Jul 3 2016 Antti Jarvinen <antti.jarvinen@katiska.org> - 0.11-1
- New upstream version: bugfixes and support for OpenSSL 1.1 API

* Fri Apr 8 2016 Antti Jarvinen <antti.jarvinen@katiska.org> - 0.10-1
- New upstream version: audio capabilities and translation additions

* Sat Oct 10 2015 Antti Jarvinen <antti.jarvinen@katiska.org> - 0.09-1
- Fixes to serious networking bugs
- Translation additions

* Mon Sep 28 2015 Antti Jarvinen <antti.jarvinen@katiska.org> - 0.08-1
- Links against qt5 instead of qt4
- Translation system is gnu gettext instead of qm files of Qt.
- Better tracking of changing local network addresses
- Numerous small bugfixes, mostly in networking code

* Sun Apr 12 2015 Antti Jarvinen <classified-ads.questions@katiska.org> - 0.07-1
- Removed intermediate PNG files into separate tarball
- Included code to generate intermediate PNG files manually

* Mon Apr 6 2015 Antti Jarvinen <classified-ads.questions@katiska.org> - 0.06-1
- Included original high-res bitmaps in different format, conversion routines.
- Fixed potential SIGSEGV appearing in debug build
- Code indented + typos fixed

* Wed Mar 25 2015 Antti Jarvinen <classified-ads.questions@katiska.org> - 0.05-1
- spec-file changes due to review comments.
- tagged a new version to allow building of latest version.
- added copyright notice to FrontWidget.cpp.
- included LGPL_EXCEPTION.txt from Nokia alongside sources.

* Tue Mar 17 2015 Antti Jarvinen <classified-ads.questions@katiska.org> - 0.04-2
- Changed packaging to happen in more civilized way.
  Lot of changes into spec file. 
- Package name has changed classified_ads -> classified-ads.
- Added appdata, re-wrote the small manpage in less personal tone. 

* Sat Mar 14 2015 Antti Jarvinen <classified-ads.questions@katiska.org> - 0.04-1
- License change GPL->LGPL due to OpenSSL license incompatibility.
- Minor UI changes as some bitmaps removed due to licensing issues

* Tue Feb 24 2015 Antti Jarvinen <classified-ads.questions@katiska.org> - 0.03-1
- Rpm build fixes for fedora linux
- Slower connection attempts to unreachable nodes
- Better file extenstion handling
- UI tweaks for small vertical screen resolutions, affects at least 
   users of some mac models
- Classifications entered by user now actually work too 
- If no local search avail, still offer network search
- Possible crash case fix. Still having random crashes inside
  json serialize..
- Links between documents, still does not work with profile comments
- Try miniupnpc also in unix build
- Basic support for trust-tree based on trust lists
- Translations
* Sun Jan 04 2015 Antti Jarvinen <classified-ads.questions@katiska.org> - 0.02-1
- Libnatpmp is not used in windows build,
- Profile comment document width setting was missing
- Made main window slightly smaller vertically.
- Now compiles also under Qt5
- Fix for grave bug: only message sender (not recipient) can read binary attachments
- Error messages for some file operations
- Fix for situation where published private profile breaks every node in network
- Require c++ compiler for compilation of c++, when building rpm.

* Wed Dec 31 2014 Tuomas Haarala <tuoppi@hieno.net> - 0.01-1
- initial spec file scribbled together
