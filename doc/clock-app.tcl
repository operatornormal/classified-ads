#
# hugely useful program
#
proc timerprocedure {} {
    set ::timervariable [clock format [clock seconds] ]
    after 1000 timerprocedure
}
timerprocedure
pack [label .l -textvariable ::timervariable]
pack [button .b -command exit -text Exit]

