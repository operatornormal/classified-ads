#     -*-TCL-*- -*-coding: utf-8-unix;-*-
#  Classified Ads is Copyright (c) Antti Järvinen 2013-2017.
#
#  This file is part of Classified Ads.
#
#  Classified Ads is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License, or (at your option) any later version.
#
#  Classified Ads is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with Classified Ads; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#  This file implements the classic worm game in TCL
#
proc game_init {} {
    set ::area_max_x 100
    set ::area_max_y 100
    # after how many steps the worm gets longer
    set ::worm_extension_speed 5
    # counter where we're now regarding extension:
    set ::worm_extension_counter $::worm_extension_speed
    label .l -text {The game is to slither}
    pack .l
    canvas .c -width [ expr $::area_max_x * 3 ] -height [ expr $::area_max_y * 3 ]
    pack .c
    frame .f
    button .f.b -text {Stop stop} -command button_procedure
    pack .f.b -side right -padx 10
    set ::game_speed 300
    scale .f.s -orient horizontal -showvalue false -label Speed -variable ::game_speed -from 100 -to 500
    pack .f.s -side left -padx 10
    set ::wurm_len {Len 0}
    label .f.l -textvariable ::wurm_len
    pack .f.l -side left -padx 10
    pack .f
    # initialize worm at beginning
    set ::wurm_dir up
    set ::color_counter 0
    set ::color_increment 1
    # on-off -switch here, if game is on or not
    set ::wurm_moves 1
}
# function that draws frontside of the worm
proc display_worm_head { pos_x pos_y } {
    set actual_x [ expr $pos_x * 3 ]
    set actual_y [ expr $pos_y * 3 ]
    set tag [ format {%d_%d} $pos_x $pos_y ]
    set wurm_color [ format "\#%02X%02X%02X" $::color_counter 167 50 ]
    set ::color_counter [ expr $::color_counter + $::color_increment ]
    if { $::color_counter > 254 } {
        set ::color_increment -1 
    }
    if { $::color_counter < 1 } {
        set ::color_increment 1
    }
    .c create rectangle $actual_x $actual_y [ expr $actual_x + 3 ] [ expr $actual_y + 3 ] -outline $wurm_color -fill $wurm_color -tag $tag
    # last item in list always head, first item then the tail
    if { [ lsearch $::wurm_tail $tag ] == -1 } {
        lappend ::wurm_tail $tag
        incr ::worm_extension_counter 
        if { $::worm_extension_counter > $::worm_extension_speed } {
            # do not remove tail, reset counter:
            set ::worm_extension_counter 0
        } else {
            # retrieve item:
            set tail_item [ lindex $::wurm_tail 0 ]
            # remove item frontmost item:
            set ::wurm_tail [ lreplace $::wurm_tail 0 0 ]
            # un-draw the tail:
            .c delete [ .c find withtag $tail_item ]
        }
    } else {
        # game over, worm eats its own tail
        set ::wurm_moves 0 
        .f.b configure -text {Restart}
    }
}
# non-slithering version
proc advance_worm {} {
    switch $::wurm_dir {
        up {
            set ::wurm_y [ expr $::wurm_y - 1 ]
        }
        down {
            set ::wurm_y [ expr $::wurm_y + 1 ]
        }
        left {
            set ::wurm_x [ expr $::wurm_x - 1 ]
        }
        right {
            set ::wurm_x [ expr $::wurm_x + 1 ]
        }
    }
    # keep body visible:
    if { $::wurm_y < 0 } {
        set ::wurm_y $::area_max_y
    }
    if { $::wurm_y > $::area_max_y } {
        set ::wurm_y 0
    }
    if { $::wurm_x < 0 } {
        set ::wurm_x $::area_max_x
    }
    if { $::wurm_x > $::area_max_x } {
        set ::wurm_x 0
    }
    set ::wurm_len [ format {Len %d} [ llength $::wurm_tail ] ]
}

proc button_procedure {} {
    if { $::wurm_moves == 0 } {
        set ::wurm_moves 1
        .c delete "all"
        worm_init
        .f.b configure -text {Stop stop}
    } else {
        exit
    } 
}

proc worm_init {} {
    # keep the animal in a list so we know where it is:
    set ::wurm_tail [ list ]
    set ::wurm_x 50
    set ::wurm_y 50
    display_worm_head $::wurm_x $::wurm_y 
    advance_worm
    display_worm_head $::wurm_x $::wurm_y 
    advance_worm
}
proc keys_init {} {
    bind . <Key-Up> {set ::wurm_dir up ; break}
    bind . <Key-Down> {set ::wurm_dir down ; break}
    bind . <Key-Left> {set ::wurm_dir left ; break}
    bind . <Key-Right> {set ::wurm_dir right ; break}
    bind . <q> {exit}
    # every time we gain focus, set keyboard focus to . (e.g. all windows)
    bind . <Enter> {focus .}
}

# game is played here:
proc game_loop {} {
    if { $::wurm_moves == 1 } {
        advance_worm
        display_worm_head $::wurm_x $::wurm_y 
    }
    after $::game_speed game_loop
}

# execution starts here:
game_init
worm_init
keys_init
game_loop
