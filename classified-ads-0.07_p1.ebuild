# Copyright 1999-2015 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/net-p2p/classified-ads/classified-ads-0.07.ebuild,v 1.1 2015/06/29 05:39:09 idella4 Exp $

EAPI=5
PLOCALES="en fi sv"
PLOCALE_BACKUP="en"
inherit qt4-r2

DESCRIPTION="Program for displaying classified advertisement items"
HOMEPAGE="http://katiska.org/classified-ads/"
SRC_URI="https://github.com/operatornormal/classified-ads/archive/dd2017f3be9d5d68d5fb85af2d626424b3e712e0.tar.gz\
 -> classified-ads-${PV}.tar.gz \
https://github.com/operatornormal/classified-ads/blob/graphics/preprocessed.tar.gz?raw=true\
 -> classified-ads-graphics-${PV}.tar.gz"
#github uses full digest if downloaded by hash:
S="$WORKDIR/classified-ads-dd2017f3be9d5d68d5fb85af2d626424b3e712e0"

LICENSE="LGPL-2.1"
SLOT="0"
KEYWORDS="~amd64 ~x86"

IUSE="debug doc test"

DEPEND="dev-libs/openssl:0
	dev-libs/qjson
	>=net-libs/libnatpmp-20130911
	<=net-libs/libnatpmp-20140401-r1
	>=net-libs/miniupnpc-1.8
	sys-apps/file
	dev-qt/qtgui:4[debug?]
	test? (	dev-libs/libgcrypt:0
		dev-qt/qttest:4
		sys-devel/gdb:0 )
	doc? ( app-doc/doxygen[dot] )"
RDEPEND="dev-libs/openssl:0
	dev-libs/qjson
	>=net-libs/libnatpmp-20130911
	<=net-libs/libnatpmp-20140401-r1
	>=net-libs/miniupnpc-1.8
	sys-apps/file
	dev-qt/qtgui:4[debug?]"

src_prepare() {
	# preprocessed graphics are unpacked into wrong directory
	# so lets move them into correct location:
	mv ../ui/* ui/ || die
	# then just run qmake
	qt4-r2_src_prepare
}

src_compile() {
	qt4-r2_src_compile
	if use doc; then
		cd doc || die
		doxygen || die
	fi
}

src_test() {
	cd test || die "test suite missing"
	qmake || die "test suite configure failed"
	make

	if [ -e $HOME/.classified_ads/sqlite_db ]; then
		mv $HOME/.classified_ads/sqlite_db $HOME/.classified_ads/sqlite_db.backup \
		|| die "datafile backup failed"
	fi
	./testca
	result=$?
	rm $HOME/.classified_ads/sqlite_db || true

	if [ -e $HOME/.classified_ads/sqlite_db.backup ]; then
		mv $HOME/.classified_ads/sqlite_db.backup $HOME/.classified_ads/sqlite_db \
		|| die "datafile restore failed"
	fi

	if [ $result != "0" ]; then
		die "test failed with code $result"
	fi

	return $result
}

src_install() {
	emake install INSTALL_ROOT="${D}"
	use doc && dodoc -r doc/doxygen.generated/html/
}
