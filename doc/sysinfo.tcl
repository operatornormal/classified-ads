#
# small tcl/tk program that just displays values of platform strings
#
set sysinfostring platform=$tcl_platform(platform)
set sysinfostring $sysinfostring\nbyteorder=$tcl_platform(byteOrder)
set sysinfostring $sysinfostring\nengine=$tcl_platform(engine)
set sysinfostring $sysinfostring\nwordSize=$tcl_platform(wordSize)
set sysinfostring $sysinfostring\npointerSize=$tcl_platform(pointerSize)
set sysinfostring $sysinfostring\nTcl_Version=$tcl_version
set sysinfostring $sysinfostring\nTcl_patchlevel=[info patchlevel]
set sysinfostring $sysinfostring\nTk_Version=$tk_version
set sysinfostring $sysinfostring\nTk_patchLevel=$tk_patchLevel
label .displaylabel -justify left -text $sysinfostring
pack .displaylabel
