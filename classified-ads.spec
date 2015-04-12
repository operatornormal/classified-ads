Name:		classified-ads
Version:	0.06
Release:	1%{?dist}
Summary:	Classified ads is a program for posting ads online

Group:		Applications/Internet
License:	LGPLv2
URL:		http://katiska.org/classified_ads/
Source0:	https://github.com/operatornormal/%{name}/archive/%{version}.tar.gz#/%{name}-%{version}.tar.gz
Source1:	https://github.com/operatornormal/classified-ads/blob/graphics/preprocessed.tar.gz?raw=true#/%{name}-graphics-%{version}.tar.gz
BuildRequires:	qt-devel >= 4
BuildRequires:	openssl-devel, libnatpmp-devel, qjson-devel, miniupnpc-devel, file-devel
BuildRequires:	libappstream-glib, desktop-file-utils
%description
Classified ads is an attempt to re-produce parts of the functionality
that went away when Usenet news ceased to exist. This attempt tries to
fix the problem of disappearing news-servers so that there is no servers
required and no service providers needed; data storage is implemented
inside client applications that users are running.
%prep
%setup -q -a 1

%build
qmake-qt4 
make %{?_smp_mflags}

%install
INSTALL_ROOT=%{buildroot} make install
appstream-util validate-relax --nonet %{buildroot}/%{_datadir}/appdata/classified-ads.appdata.xml
desktop-file-validate %{buildroot}/%{_datadir}/applications/classified-ads.desktop
%files
%doc README.TXT
%{_bindir}/classified-ads
%{_datadir}/applications/classified-ads.desktop
%dir %{_datadir}/app-install
%dir %{_datadir}/app-install/icons
%{_datadir}/app-install/icons/turt-transparent-128x128.png
%dir %{_datarootdir}/classified-ads
%{_datarootdir}/classified-ads/classified_ads_fi.qm
%{_datarootdir}/classified-ads/classified_ads_sv.qm
%{_mandir}/man1/classified-ads.1.gz
%{_datadir}/appdata/classified-ads.appdata.xml
%license LICENSE
%changelog
%changelog
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
