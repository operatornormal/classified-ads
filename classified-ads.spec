Name:		classified_ads
Version:	0.01
Release:	1%{?dist}
Summary:	Classified ads is a program for posting ads online

Group:		Applications/Internet
License:	GPLv3
URL:		https://github.com/operatornormal/classified_ads/releases/tag/0.01
Source0:	classified_ads-0.01.tar.gz

BuildRequires:	qt-devel >= 4
BuildRequires:	openssl-devel, libnatpmp-devel, qjson-devel, gcc-c++
Requires:	qt >= 4
Requires:	bzip2-libs, expat, fontconfig, freetype, openssl-libs
Requires:	pcre, qjson, libnatpmp, qjson-devel, xz-libs, zlib

BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
Classified ads is an attempt to re-produce parts of the functionality that went away when usenet news ceased to exist. This attempt tries to fix the problem of disappearing news-servers so that there is no servers required ; data storage is implemented inside client applications that you and me are running.


%prep
%setup -q

%build
PREFIX="/tmp/foo"
QMAKE_ARGS+="INCLUDEPATH+=${LOCALBASE}/include/miniupnpc/ LIBS+=-L${LOCALBASE}/lib"; qmake-qt4 PREFIX=$RPM_BUILD_ROOT
make

%install
INSTALL_ROOT=$RPM_BUILD_ROOT make install 

%clean
rm -rf $RPM_BUILD_ROOT


%files
%doc README.TXT
%{_bindir}/classified-ads
%{_datadir}/applications/classified_ads.desktop
%{_datadir}/app-install/icons/turt-transparent-128x128.png

%changelog
* Wed Dec 31 2014 Tuomas Haarala <tuoppi@hieno.net> - 0.01-1
- initial spec file scribbled together
