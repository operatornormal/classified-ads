# -*- Makefile -*- -*-coding: utf-8-unix;-*-
#
#    Classified Ads is Copyright (c) Antti Järvinen 2013.
#
#    This file is part of Classified Ads.
#
#    Classified Ads is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    Classified Ads is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with Classified Ads; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
# The file turt-transparent.xcf is highest-resolution version available
# of the turtle logo. It is not used in runtime in that high-res but instead
# converted to 2 smaller resolutions. This makefile handles the resizing
# using imagemagick. If turt-transparent.xcf is modified, say "make all"
# in this directory to generate the small-resolution versions that are
# used at build stage.
#
all: ../ui/turt558.png ../ui/turt-transparent-128x128.png

../ui/turt558.png: turt-transparent.xcf
	convert turt-transparent.xcf -resize 558 +set date:create +set date:modify -define png:exclude-chunk=time ../ui/turt558.png

../ui/turt-transparent-128x128.png: turt-transparent.xcf
	convert turt-transparent.xcf -resize 128x128\>   -gravity center -background none -extent 128x128 +set date:create +set date:modify -define png:exclude-chunk=time ../ui/turt-transparent-128x128.png

clean:
	rm -f ../ui/turt558.png ../ui/turt-transparent-128x128.png
