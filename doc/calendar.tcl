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
#  This file implements gregorian calendar with shared calendar events. 
#

#
#  Workflows in operations: 
#  - Operation when starting up:
#  -- Init UI
#  -- Load local settings
#  -- Set day to display (current time, in practice)
#  -- Fetch events of selected day. As Db records in db contain events so
#     that from each operator there is one months records in single record
#     the search is done by month and all events found for month are
#     appended into global array "::allEvents"
#  -- Events from "::allEvents" that occur on selected day, are drawn into
#     canvas for user to ogle
#  -- Fetch event comments from storage like what is done to events.
#     Comments are stored in a dictionary containing list of dictionaries, 
#     key to first dictionary is the event that is commented, inside
#     is a list of comments, each being a dictionary. In storage 
#     (local and published records) comments are saved in similar
#     structure. 
#
#  - Operations when user selects another day to be shown or changes
#    event collection to display
#  -- Fetch events of selected day. The database implementation of 
#     classified ads will not re-send db record queries to remote 
#     nodes if query criteria remains the same, so re-doing same
#     query multiple times is only small waste of resources. 
#     Anyway, by checking whether month changed and querying the actual
#     database only when month changes will speed up operations
#  -- Events from "::allEvents" that occur on selected day, are drawn into
#     canvas for user to ogle
#
#  - Operations when user posts a new event
#  -- Event is appended into "::allEvents". 
#  -- Local database (the application specific non-shared db) is
#     updated to contain the event (and retain events of other operator
#     profiles that may be stored in same database record)
#  -- Depending on publicity setting of the event (public, by profile setting)
#     a shared database record is created/updated to contain the new event
#     and that record is published to network
# 
#  - Operations when user deletes an event
#  -- Same as "post" put event is removed from storages and records. Publish
#     needs to occur if deleted event as public or by profile settings publicity.
#
#  - Operations when new database record is received into system
#  -- This record may or may not contain records from collection in display,
#     it may or may not contain events that belong to our selected month/day.
#     If there is events for selected collection it is easiest to prune all
#     events from "::allEvents" that originate from newly-received record,
#     then do a re-display of selected day. 
#
# procedure for initializing global variables
proc initGlobals {} {
    # Month on display, global variable
    set ::monthOnDisplay [ clock seconds ]
    # Selected day on display, global variable:
    set ::dayOnDisplay $::monthOnDisplay
    set ::leftMarginInPixels 100
    # This is used in canvas resize event to not try updating until
    # initialization is wholly done
    set ::initReady false
    # Timer for delaying re-draw events when user keeps on scaling the window
    set ::resizeEventTimer -1
    # Global list for events in calendar
    set ::allEvents [ list ]
    # Global list of events to store locally
    set ::allLocalEvents [ list ]
    # Global dictionary for db records whose events are in ::allEvents.
    # Key in dict is the record id, value is publish time. Idea is that
    # if we fetch same record again and the publish time is same 
    # as previously then content has not changed and there is no need
    # to process it at all.
    set ::allDbRecords [ dict create ]
    # Program settings is a dictionary
    set ::settings [ dict create ]
    # What is the intended audience to new events:
    set ::postPublicity public
    # What is the default collection for events
    set ::collectionInUse "GlobalEventCollection"
    # Dictionary for comments, key is the commented event, 
    # inside is a list of comments
    set ::allComments [ dict create ]
    # in similar manner as there is ::allEvents and ::allLocalEvents,
    # lets have also
    set ::allLocalComments [ dict create ]
}

# 
# Init procedure for user interface widgets. Contains several sub-procedures
# that are called by this proc.
#
proc initUI {} {
    # figure out initial size for widgets, something smaller than screen size
    set ::initialWidth [ expr int ( [ winfo screenwidth . ] * 0.90 ) ]
    set ::initialHeight [ expr int ( [ winfo screenheight . ] * 0.85 ) ]
    # frame for display day-selection calendar    
    frame .s -borderwidth 2 -relief ridge -width $::initialWidth -height $::initialHeight
    grid .s -row 0 -column 0 -sticky news
    # frame for buttons on top
    frame .rightframe
    grid .rightframe -row 0 -column 1 -sticky news
    # configure grid for expanding:
    grid rowconfigure . 0 -weight 1
    grid columnconfigure . .rightframe -weight 1
    frame .rightframe.buttons -borderwidth 2 -relief ridge
    button .rightframe.buttons.dayview -text {Calendar view} -command showDayView
    button .rightframe.buttons.settings -text {Settings} -command showSettings
    button .rightframe.buttons.newEvent -text {Create event} -command showCreateEvent
    pack .rightframe.buttons -side top
    pack .rightframe.buttons.settings -padx 5 -side right
    pack .rightframe.buttons.dayview -padx 5 -side right
    pack .rightframe.buttons.newEvent -padx 5 -side right
    frame .rightframe.dayframe -borderwidth 2 -relief ridge
    set dayFrameWidth [ expr int($::initialWidth  * 0.75) ]
    set dayFrameHeight [ expr int($::initialHeight  * 0.90) ]
    canvas .rightframe.dayframe.daycanvas -relief ridge -borderwidth 2 -height $dayFrameHeight -width $dayFrameWidth -scrollregion " 0 0 $dayFrameWidth 1000 "
    # ask for resize events:
    bind .rightframe.dayframe.daycanvas <Configure> {
        daycanvasResizeEvent %w %h
    }
    scrollbar .rightframe.dayframe.hscroll -command scrollDayview
    .rightframe.dayframe.hscroll set 0 0.3333

    # set up things visible in settings frame
    frame .rightframe.settingsframe 
    label .rightframe.settingsframe.collectionText -text {Name of event collection in use: }
    entry .rightframe.settingsframe.collectionEntry
    button .rightframe.settingsframe.changeCollectionBtn -text Change -command changeCollectionCallback
    pack .rightframe.settingsframe.changeCollectionBtn -side right -anchor sw
    pack .rightframe.settingsframe.collectionEntry -side right -anchor sw
    pack .rightframe.settingsframe.collectionText -side right -expand y -fill x  -anchor sw
    .rightframe.settingsframe.collectionEntry insert 0 $::collectionInUse
    # UI frame for writing and posting a new event
    frame .rightframe.newEventFrame
    constructNewEventView .rightframe.newEventFrame
}

#
# Procedure called when "new event" is selected ; initializes start+end time 
# labels.  See also displayNewEventDates. 
#
proc initializeNewEventDates {} {
    set currentLabelText [ lindex [ .rightframe.newEventFrame.timeFrame.startDayLabel configure -text ] 4 ]
    if { $currentLabelText == {} } {

        set month [ clock format $::dayOnDisplay -format {%N} ]
        set year [ clock format $::dayOnDisplay -format {%Y} ]
        set day [ clock format $::dayOnDisplay -format {%d} ]
        set hour [ clock format [ clock seconds ] -format {%H} ]
        set min 00

	# remove possible leading zeroes:
	regsub {^[0]} $month {\1} month
	regsub {^[0]} $day {\1} day
	regsub {^[0]} $hour {\1} hour
	regsub {^[0]} $min {\1} min
	
        set ::newEventStart [ clock scan [ format "%04d %02d %02d %02d %02d 00" $year $month $day $hour $min] -format {%Y %m %d %H %M %M} -gmt false ]
        # push start time to next hour
        set ::newEventStart [ clock add $::newEventStart 1 hour ]
        # and end time one hour after that:
        set ::newEventEnd [ clock add $::newEventStart 1 hour ]
        displayNewEventDates
    }
}
#
# displays $::newEventStart on labels
#
proc displayNewEventDates {} {
    .rightframe.newEventFrame.timeFrame.startDayLabel configure -text [ clock format $::newEventStart -format {%d} ]
    .rightframe.newEventFrame.timeFrame.startMonthLabel configure -text [ clock format $::newEventStart -format {%B} -locale system ]
    .rightframe.newEventFrame.timeFrame.startYearLabel configure -text [ clock format $::newEventStart -format {%Y} ]
    .rightframe.newEventFrame.timeFrame.startHourLabel configure -text [ clock format $::newEventStart -format {%H} -locale system ]
    .rightframe.newEventFrame.timeFrame.startMinuteLabel configure -text [ clock format $::newEventStart -format {%M} ]

    .rightframe.newEventFrame.timeFrame.endDayLabel configure -text [ clock format $::newEventEnd -format {%d} ]
    .rightframe.newEventFrame.timeFrame.endMonthLabel configure -text [ clock format $::newEventEnd -format {%B} -locale system ]
    .rightframe.newEventFrame.timeFrame.endYearLabel configure -text [ clock format $::newEventEnd -format {%Y} ]
    .rightframe.newEventFrame.timeFrame.endHourLabel configure -text [ clock format $::newEventEnd -format {%H} -locale system ]
    .rightframe.newEventFrame.timeFrame.endMinuteLabel configure -text [ clock format $::newEventEnd -format {%M} ]
}

#
# button callback called from plus/minus buttons at event date selection
#
proc modifyEventTime { startOrEnd modification } {
    if { $startOrEnd == "start" } {
        set dateToModify $::newEventStart
    } else {
        set dateToModify $::newEventEnd
    }
    switch $modification {
        +d {
            set dateToModify [ clock add $dateToModify 1 day ]
        }
        +mo {
            set dateToModify [ clock add $dateToModify 1 month ]
        }
        +y {
            set dateToModify [ clock add $dateToModify 1 year ]
        }
        +h {
            set dateToModify [ clock add $dateToModify 1 hour ]
        }
        +mi {
            set dateToModify [ clock add $dateToModify 1 minute ]
        }
        -d {
            set dateToModify [ clock add $dateToModify -1 day ]
        }
        -mo {
            set dateToModify [ clock add $dateToModify -1 month ]
        }
        -y {
            set dateToModify [ clock add $dateToModify -1 year ]
        }
        -h {
            set dateToModify [ clock add $dateToModify -1 hour ]
        }
        -mi {
            set dateToModify [ clock add $dateToModify -1 minute ]
        }
    }
    if { $startOrEnd == "start" } {
        set ::newEventStart $dateToModify
    } else {
        set ::newEventEnd $dateToModify
    }
    displayNewEventDates
}

proc showDayView {} {
    pack forget .rightframe.settingsframe
    pack forget .rightframe.newEventFrame
    pack .rightframe.dayframe.hscroll -side right -fill y 
    pack .rightframe.dayframe.daycanvas -fill both -expand yes
    pack .rightframe.dayframe -side bottom -expand yes -fill both
}

proc showSettings {} {
    pack forget .rightframe.dayframe
    pack forget .rightframe.newEventFrame
    pack .rightframe.settingsframe -side top
}

proc showCreateEvent {} {
    pack forget .rightframe.dayframe
    pack forget .rightframe.settingsframe
    pack .rightframe.newEventFrame -side bottom -fill both -expand yes
    initializeNewEventDates
}


#
# basic structure: fields on top of each others
# first is start time, then end time, then title + desc
# followed by possible attendees and publish options
# times are constructed as grids with fields for date-values
# and +- -buttons
#
proc constructNewEventView { baseWidget } {
    # buttons for start time
    frame $baseWidget.timeFrame
    pack $baseWidget.timeFrame -fill both -expand y
    button $baseWidget.timeFrame.startDayPlusBtn -text "+" -command {modifyEventTime start +d} -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.startMonthPlusBtn -text "+" -command {modifyEventTime start +mo}  -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.startYearPlusBtn -text "+" -command {modifyEventTime start +y}  -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.startHourPlusBtn -text "+" -command {modifyEventTime start +h}  -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.startMinutePlusBtn -text "+" -command {modifyEventTime start +mi}  -repeatdelay 500 -repeatinterval 150
    
    grid $baseWidget.timeFrame.startDayPlusBtn -row 0 -column 1
    grid $baseWidget.timeFrame.startMonthPlusBtn -row 0 -column 2
    grid $baseWidget.timeFrame.startYearPlusBtn -row 0 -column 3
    grid $baseWidget.timeFrame.startHourPlusBtn -row 0 -column 4
    grid $baseWidget.timeFrame.startMinutePlusBtn -row 0 -column 6

    label $baseWidget.timeFrame.startTimeLabel -text "Event start: "
    label $baseWidget.timeFrame.startDayLabel 
    label $baseWidget.timeFrame.startMonthLabel 
    label $baseWidget.timeFrame.startYearLabel 
    label $baseWidget.timeFrame.startHourLabel 
    label $baseWidget.timeFrame.startTimeColonLabel -text ":"
    label $baseWidget.timeFrame.startMinuteLabel 

    grid $baseWidget.timeFrame.startTimeLabel -row 1 -column 0 -sticky e
    grid $baseWidget.timeFrame.startDayLabel -row 1 -column 1
    grid $baseWidget.timeFrame.startMonthLabel -row 1 -column 2
    grid $baseWidget.timeFrame.startYearLabel -row 1 -column 3
    grid $baseWidget.timeFrame.startHourLabel -row 1 -column 4
    grid $baseWidget.timeFrame.startTimeColonLabel -row 1 -column 5
    grid $baseWidget.timeFrame.startMinuteLabel -row 1 -column 6

    button $baseWidget.timeFrame.startDayMinusBtn -text "-" -command {modifyEventTime start -d}  -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.startMonthMinusBtn -text "-" -command {modifyEventTime start -mo}  -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.startYearMinusBtn -text "-" -command {modifyEventTime start -y}  -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.startHourMinusBtn -text "-" -command {modifyEventTime start -h}  -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.startMinuteMinusBtn -text "-" -command {modifyEventTime start -mi}  -repeatdelay 500 -repeatinterval 150

    grid $baseWidget.timeFrame.startDayMinusBtn -row 2 -column 1
    grid $baseWidget.timeFrame.startMonthMinusBtn -row 2 -column 2
    grid $baseWidget.timeFrame.startYearMinusBtn -row 2 -column 3
    grid $baseWidget.timeFrame.startHourMinusBtn -row 2 -column 4
    grid $baseWidget.timeFrame.startMinuteMinusBtn -row 2 -column 6

    #end time selector:
    button $baseWidget.timeFrame.endDayPlusBtn -text "+" -command {modifyEventTime end +d}  -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.endMonthPlusBtn -text "+" -command {modifyEventTime end +mo}  -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.endYearPlusBtn -text "+" -command {modifyEventTime end +y}  -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.endHourPlusBtn -text "+" -command {modifyEventTime end +h} -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.endMinutePlusBtn -text "+" -command {modifyEventTime end +mi} -repeatdelay 500 -repeatinterval 150
    
    grid $baseWidget.timeFrame.endDayPlusBtn -row 3 -column 1
    grid $baseWidget.timeFrame.endMonthPlusBtn -row 3 -column 2
    grid $baseWidget.timeFrame.endYearPlusBtn -row 3 -column 3
    grid $baseWidget.timeFrame.endHourPlusBtn -row 3 -column 4
    grid $baseWidget.timeFrame.endMinutePlusBtn -row 3 -column 6

    label $baseWidget.timeFrame.endTimeLabel -text "Event end: "
    label $baseWidget.timeFrame.endDayLabel 
    label $baseWidget.timeFrame.endMonthLabel 
    label $baseWidget.timeFrame.endYearLabel 
    label $baseWidget.timeFrame.endHourLabel 
    label $baseWidget.timeFrame.endTimeColonLabel -text ":"
    label $baseWidget.timeFrame.endMinuteLabel 

    grid $baseWidget.timeFrame.endTimeLabel -row 4 -column 0 -sticky e
    grid $baseWidget.timeFrame.endDayLabel -row 4 -column 1
    grid $baseWidget.timeFrame.endMonthLabel -row 4 -column 2
    grid $baseWidget.timeFrame.endYearLabel -row 4 -column 3
    grid $baseWidget.timeFrame.endHourLabel -row 4 -column 4
    grid $baseWidget.timeFrame.endTimeColonLabel -row 4 -column 5
    grid $baseWidget.timeFrame.endMinuteLabel -row 4 -column 6

    button $baseWidget.timeFrame.endDayMinusBtn -text "-" -command {modifyEventTime end -d} -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.endMonthMinusBtn -text "-" -command {modifyEventTime end -mo} -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.endYearMinusBtn -text "-" -command {modifyEventTime end -y} -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.endHourMinusBtn -text "-" -command {modifyEventTime end -h} -repeatdelay 500 -repeatinterval 150
    button $baseWidget.timeFrame.endMinuteMinusBtn -text "-" -command {modifyEventTime end -mi} -repeatdelay 500 -repeatinterval 150

    grid $baseWidget.timeFrame.endDayMinusBtn -row 5 -column 1
    grid $baseWidget.timeFrame.endMonthMinusBtn -row 5 -column 2
    grid $baseWidget.timeFrame.endYearMinusBtn -row 5 -column 3
    grid $baseWidget.timeFrame.endHourMinusBtn -row 5 -column 4
    grid $baseWidget.timeFrame.endMinuteMinusBtn -row 5 -column 6

    frame $baseWidget.titleFrame
    pack $baseWidget.titleFrame -fill x -expand yes

    label $baseWidget.titleFrame.titleLabel -text {Event title: }
    entry $baseWidget.titleFrame.titleEntry
    pack $baseWidget.titleFrame.titleEntry -side right -expand y -fill x
    pack $baseWidget.titleFrame.titleLabel -side right

    frame $baseWidget.descFrame
    pack $baseWidget.descFrame -fill both -expand yes
    label $baseWidget.descFrame.descLabel -text {Event description: }
    text $baseWidget.descFrame.descText
    pack $baseWidget.descFrame.descText -side right -fill both -expand yes
    pack $baseWidget.descFrame.descLabel -side right -anchor n 

    frame $baseWidget.postOptionsFrame
    pack $baseWidget.postOptionsFrame -fill x -expand yes
    label $baseWidget.postOptionsFrame.publicityLabel -text "Intended audience of the event: "
    radiobutton $baseWidget.postOptionsFrame.publicPost -value public -text "Public to all users having same event collection" -variable ::postPublicity 
    radiobutton $baseWidget.postOptionsFrame.accordingToProfile -value byprofile -text "Follows your profile privacy setting" -variable ::postPublicity 
    radiobutton $baseWidget.postOptionsFrame.privatePost -value private -text "Private to local calendar only" -variable ::postPublicity 
    pack $baseWidget.postOptionsFrame.publicityLabel -anchor w
    pack $baseWidget.postOptionsFrame.publicPost -anchor w
    pack $baseWidget.postOptionsFrame.accordingToProfile -anchor w
    pack $baseWidget.postOptionsFrame.privatePost -anchor w

    frame $baseWidget.buttonFrame
    pack $baseWidget.buttonFrame -fill x -expand yes
    button $baseWidget.buttonFrame.okButton -text {Post event} -command postNewEventCmd
    button $baseWidget.buttonFrame.cancelButton -text {Cancel} -command cancelNewEventCmd
    pack $baseWidget.buttonFrame.cancelButton  -side right 
    pack $baseWidget.buttonFrame.okButton  -side right 
    label $baseWidget.buttonFrame.errorLabel
    pack $baseWidget.buttonFrame.errorLabel -side right 
}



#
# called when user changes size of tk window ; we must scale some of the
# widgets too and most notably re-draw the actual events-view
#
proc daycanvasResizeEvent { newWidth newHeight } {
    puts [ format "resize canvas %d %d" $newWidth $newHeight ]
    # update scrollbar:
    set nowat [.rightframe.dayframe.daycanvas yview]
    eval ".rightframe.dayframe.hscroll set $nowat"
    .rightframe.dayframe.daycanvas configure -scrollregion " 0 0 $newWidth 1000 "
    # if init is already done:
    if { $::initReady != false } {
	# dont't draw straight away, because usually we get multiple
	# events in succession and drawing is heavy operation: instead
	# set up 200ms timer and if it expires (meaning no resize event
	# seen in 200ms) then do the redraw in there, in procedure
	# redrawAfterResizeEvent
	if { $::resizeEventTimer != -1 } {
	    after cancel $::resizeEventTimer
	}
	set ::resizeEventTimer [ after 200 redrawAfterResizeEvent ]
    }
}
#
# procedure called in async via $::resizeEventTimer
#
proc redrawAfterResizeEvent { } {
    displayDay [ clock format $::dayOnDisplay -format "%d" ]
    set ::resizeEventTimer -1 
}


proc scrollDayview {args} {
    eval ".rightframe.dayframe.daycanvas yview $args"
    set nowat [.rightframe.dayframe.daycanvas yview]
    eval ".rightframe.dayframe.hscroll set $nowat"
}

#
# procedure that updates buttons in selection calendar.
# in practice it shows selected button as "lowered" and
# the rest as "raised"
#
# selectedDay is date whose button is pressed
#
proc updateSelectionButtonsState { selectedDay } {
    set currentDayNum [ clock format $selectedDay -format "%d" ]
    foreach btn [ winfo children .s.selectionFrame ] {
        if { [ string first .s.selectionFrame.button $btn ] != -1 } {
            # yes is button a button, check if it is button for to-day:
            if { [ lindex [ $btn configure -text ] 4 ] == $currentDayNum } {
                # yes, is for to-day
                $btn configure -relief sunken -bg green
            } else {
                $btn configure -relief raised -bg lightgray
            }
        }
    }
}


proc displaySelectionCalendar {starttime} {
    frame .s.selectionFrame
    set month [ clock format $starttime -format {%N} ]
    set day [ string trim [ clock format $starttime -format {%e} ] ]
    set monthFirstDay [ expr $starttime - [ expr ( $day -1 ) * 24 * 60 * 60 ] ]
    set row 0
    set weekCounter 0 
    button .s.selectionFrame.minusYear -text {<<} -command {updateSelection [ clock add $::monthOnDisplay -1 year ]}
    button .s.selectionFrame.plusYear -text {>>} -command {updateSelection [ clock add $::monthOnDisplay 1 year ]}
    button .s.selectionFrame.minusMonth -text {<} -command {updateSelection [ clock add $monthOnDisplay -1 month ]}
    button .s.selectionFrame.plusMonth -text {>} -command {updateSelection [ clock add $monthOnDisplay 1 month ]}
    grid .s.selectionFrame.minusMonth -row 0 -column 1 
    grid .s.selectionFrame.minusYear -row 0 -column 0 
    grid .s.selectionFrame.plusYear -row 0 -column 5 
    grid .s.selectionFrame.plusMonth -row 0 -column 4
    label .s.selectionFrame.yearNumber -text [ clock format $starttime -format {%Y} ]
    grid .s.selectionFrame.yearNumber -row 0 -column 3
    label .s.selectionFrame.monthName -text  [ clock format $starttime -format {%b} -locale system]
    grid .s.selectionFrame.monthName -row 0 -column 2
    incr row
    for {set currentDay $monthFirstDay} {$weekCounter < 7} {set currentDay [ expr $currentDay + ( 24 * 60 * 60 ) ]} {
        incr weekCounter
        set labelDayOfWeek [ expr [ clock format $currentDay -format {%u} ] - 1 ]
        label .s.selectionFrame.l$labelDayOfWeek -text [ clock format $currentDay -format {%a} -locale system]
        grid .s.selectionFrame.l$labelDayOfWeek -row $row -column $labelDayOfWeek
    }
    incr row
    for {set currentDay $monthFirstDay} {[clock format $currentDay -format {%N} ] == $month } {set currentDay [ expr $currentDay + ( 24 * 60 * 60 ) ]} {
        set buttonDayNumber [ string trim [ clock format $currentDay -format {%e} ] ]
        button .s.selectionFrame.button$buttonDayNumber -text [ format {%02d} $buttonDayNumber ] -command "displayDay $buttonDayNumber"
        set buttonDayOfWeek [ expr [ clock format $currentDay -format {%u} ] - 1 ]
        grid .s.selectionFrame.button$buttonDayNumber -row $row -column $buttonDayOfWeek
        if { $buttonDayOfWeek == 6 } {
            incr row
        }
    }
    pack .s.selectionFrame
}

#
# This procedure is called when user selects another month/year from
# selection calendar. This causes re-draw of the calendar under
# month/year buttons
#
proc updateSelection { newtime } {
    destroy .s.selectionFrame
    displaySelectionCalendar $newtime
    set ::monthOnDisplay $newtime
    # fetch events of newly-selected month
    eventsByTimeAndCollection $::monthOnDisplay
    # and update calendar view:
    displayDay [ clock format $newtime -format {%d} ]
}
#
# procedure that is called when user selects a day from selection
# calendar ; will force selected day on display and update its 
# events
#
proc displayDay { dayNumber } {
    puts [ format "display day %s" $dayNumber ]
    # figure out start and end time of the day
    set month [ clock format $::monthOnDisplay -format {%N} ]
    set year [ clock format $::monthOnDisplay -format {%Y} ]
    # remove possible leading zeroes:
    regsub {^[0]} $month {\1} month
    regsub {^[0]} $dayNumber {\1} dayNumber
    set start [ clock scan [ format "%04d %02d %02d 00 00 00" $year $month $dayNumber ] -format {%Y %m %d %H %M %S} -gmt false ]
    set end [clock scan [ format "%04d %02d %02d 23 59 59" $year $month $dayNumber ] -format {%Y %m %d %H %M %S} -gmt false ]
    puts [ clock format $start ]
    puts [ clock format $end ]
    updateSingleDayCanvas .rightframe.dayframe.daycanvas $start $end
    updateSelectionButtonsState $start
    set ::dayOnDisplay $start
}

#
# procedure for drawing single event into canvas, this is called from 
# proc packevents below when it is known which event collide with
# each others
#
# c is canvas
# ev is the event
# startOfDay is [ clock seconds ] from midnight when day started
#
proc drawSingleEvent { c ev startOfDay } {
    set secondsFromStartToPrevMidnight [ expr [ dict get $ev startTime ]-$startOfDay ]
    set startPos [ expr 25.0 + ( ( $secondsFromStartToPrevMidnight / ( 60.0 * 60.0 ) ) * $::heightForSingleHour ) ]
    set secondsFromEndToPrevMidnight [ expr [ dict get $ev endTime ]-$startOfDay ]
    set endPos [ expr 25.0 + ( ( $secondsFromEndToPrevMidnight / ( 60.0 * 60.0 ) ) * $::heightForSingleHour ) ]
    set scSizeOption [ $c configure -scrollregion ]
    set regSize [ lindex $scSizeOption 4 ]
    set width [ expr [ lindex $regSize 2 ] - $::leftMarginInPixels ]
    if { $width > 6 } {
        set left [ expr {$::leftMarginInPixels + int ( $width * [ dict get $ev left ] )} ]
        set right [ expr {$::leftMarginInPixels + int ( $width * [ dict get $ev right ] )} ]
        set eventRectangle [ $c create rectangle $left $startPos $right $endPos -fill green ]
        
        set eventTitleText [ $c create text [ expr $left + 5 ] [ expr $startPos + 5 ] -width [ expr ( $right - $left ) -5 ] -fill white -text [ dict get $ev title ] -anchor nw ]

        $c bind $eventRectangle <ButtonRelease-1> "displaySingleEvent [ dict get $ev identifier ]"
        $c bind $eventTitleText <ButtonRelease-1> "displaySingleEvent [ dict get $ev identifier ]"
    }
}
# procedure for drawing times+events into single canvas that
# re-presents single day
#
proc updateSingleDayCanvas {c start end} {
    #figure out canvas scroll-region size
    set scSizeOption [ $c configure -scrollregion ]
    set regSize [ lindex $scSizeOption 4 ]
    set width [ lindex $regSize 2 ] 
    set height [ lindex $regSize 3 ]
    puts [ format "scroll area w %d h %d" $width $height ]
    # this same variable is used also event drawing procedure
    # assumption is that all day-specific canvases have same
    # $height
    set ::heightForSingleHour [ expr ( $height - 30.0 ) / 24.0 ]
    # clear canvas
    $c delete "all"
    # print day number on top
    $c create text [ expr $width / 2 ] 15 -text [ clock format $start -format "%A %d" -locale system]
    # figure out dimensions
    set hourCounter 0
    while { $hourCounter < 24 } {
        set month [ clock format $start -format {%N} ]
        set year [ clock format $start -format {%Y} ]
        set dayNumber [ clock format $start -format {%d} ]
        set currentHour [clock scan [ format "%s %s %s %s 00 00" $year $month $dayNumber $hourCounter ] -format {%Y %m %d %H %M %S} -gmt false ]
        set textPos [ expr 25 + ( $hourCounter * $::heightForSingleHour ) ]
        $c create text 35 $textPos -text [ clock format $currentHour -format "%X" -locale system]
        $c create line $::leftMarginInPixels $textPos $width $textPos
        incr hourCounter
    }
    # http://stackoverflow.com/questions/11311410/visualization-of-calendar-events-algorithm-to-layout-events-with-maximum-width
    set eventsOfDay [ lsort -increasing -command startComparator [ eventsBetween $start $end ] ]
    set columns [ list ]
    set lastEndTime -1
    foreach ev $eventsOfDay {
        if { [ dict get $ev startTime ] >= $lastEndTime &&
             $lastEndTime != -1 } {
            packevents $c $columns $start
            set columns [ list ]
            set lastEndTime -1
        }
        set placed false
        set colCounter 0 
        for { set colCount [ llength $columns ] } { $colCounter < $colCount } { incr colCounter } {
            set col [ lindex $columns $colCounter ]
            set lastItem [ lindex $col [ expr [ llength $col ] - 1 ] ]
            if { [ overlaps $lastItem $ev ] == false } {
                lappend col $ev
                set placed true
                # lets put col back to columns
                set columns [ lreplace $columns $colCounter $colCounter $col ]
                break
            }
        }
        if { $placed == false } {
            set col [ list ]
            lappend col $ev
            lappend columns $col
        }
        if { $lastEndTime == -1 || [ dict get $ev endTime ] > $lastEndTime } {
            set lastEndTime [ dict get $ev endTime ]
        }
    }
    if { [ llength $columns ] > 0 } {
        packevents $c $columns $start
    }
}
proc packevents { c columns startOfDay } {
    set numcolumns [ llength $columns ]
    set numcolumns [ format "%0.2f" $numcolumns ]
    set iCol 0
    foreach col $columns {
        foreach ev $col {
            set colSpan [ expandEvent $ev $iCol $columns ]
            dict set ev left [ expr  $iCol  / $numcolumns ]
            dict set ev right [ expr (  $iCol + $colSpan ) / $numcolumns ]
            # position of event is now known: draw
            drawSingleEvent $c $ev $startOfDay
        }
        incr iCol
    }
}
#
# procedure that checks if it is ok to extend event over multiple columns
#
proc expandEvent { ev iCol columns } {
    set colSpan 1
    for { set i [ expr $iCol + 1 ] } { $i < [ llength $columns ] } { incr i } {
        set column [ lindex $columns $i ]
        foreach listEvent $column {
            if { [ overlaps $listEvent $ev ] == true } {
                return $colSpan
            } 
        }
        incr colSpan
    }
    return $colSpan
}

#
# procedure returns true if event a overlaps with b
#
proc overlaps { a b } {
    set aStart [ dict get $a startTime ]
    set aEnd [ dict get $a endTime ]
    set bStart [ dict get $b startTime ]
    set bEnd [ dict get $b endTime ]
    if { ($aStart == $bStart ) ||
         ( $aEnd > $bStart && $aEnd <= $bEnd ) ||
         ( $aStart > $bStart && $aStart < $bEnd ) ||
         ( $bStart > $aStart && $bStart < $aEnd ) ||
         ( $aStart > $bStart && $aStart < $bEnd ) } {
        return true
    } else {
        return false
    }
}

#
# procedure called when user presses button saving a new event
#
proc postNewEventCmd {} {
    set duration [ expr {$::newEventEnd-$::newEventStart} ]
    if { $duration < 1 } {
        .rightframe.newEventFrame.buttonFrame.errorLabel configure -text {Event end date too early}
    } elseif { $duration > [ expr 60 * 60 * 24 * 14 ] } {
        .rightframe.newEventFrame.buttonFrame.errorLabel configure -text {Event max duration is 14 days}
    } elseif { [ string length [ string trim [ .rightframe.newEventFrame.titleFrame.titleEntry get ] ] ] < 1 } {
        .rightframe.newEventFrame.buttonFrame.errorLabel configure -text {Title is mandatory}
    } else {
        .rightframe.newEventFrame.buttonFrame.errorLabel configure -text {}
        # so, construct the new event
        dict set ev startTime $::newEventStart
        dict set ev endTime $::newEventEnd
        dict set ev title [ .rightframe.newEventFrame.titleFrame.titleEntry get ]
	dict set ev senderId $::profileInUse
        if { [ string length [ string trim [ .rightframe.newEventFrame.descFrame.descText get 1.0 ] ] ]  > 0 } {
            dict set ev description [ string trim [ .rightframe.newEventFrame.descFrame.descText get 1.0 end ] ]
        }
        # invent identifier for the event
        dict set ev identifier [ calculateSHA1 [ format {%s-%s-%s-%s} $::newEventStart $::newEventEnd $::profileInUse [ clock seconds ] ] ]
        # include variable telling if event is supposed to be local, private or public
        if { $::postPublicity == {local} } {
            dict set ev publicity $::postPublicity
        } else {
            if { [ isProfilePrivate ] } {
                dict set ev publicity {public}
            } else {
                dict set ev publicity $::postPublicity
            }
        }
        # timestamp of creation for the event
        dict set ev createdOn [ clock seconds ]
        # set collection name into event. in our local storage we may
        # have events belonging to multiple collections but at
        # publish we may publish only events that belong to selected
        # collection
        dict set ev collection $::collectionInUse
        # and store event into datamodel
        storeNewEvent $ev
        # and clear fields using "cancel" methods
        cancelNewEventCmd 
        # and update the calendar view:
        displayDay [ clock format $::dayOnDisplay -format "%d" ]
    }
}
#
# cancel creation of new event by clearing input fields and navigating
# to calendar view
#
proc cancelNewEventCmd {} {
    .rightframe.newEventFrame.timeFrame.startDayLabel configure -text {}
    .rightframe.newEventFrame.titleFrame.titleEntry delete 0 end
    .rightframe.newEventFrame.descFrame.descText delete 1.0 end
    showDayView
}
#
# Procedure called from button, changes event database (collection) name. 
# All events in memory need to be discarded, and re-fetched from the
# newly-selected collection.
#
proc changeCollectionCallback {} {
    # set collection into settings:
    set collection [ .rightframe.settingsframe.collectionEntry get ]
    if { [ string length $collection ] > 0 } {
        set ::collectionInUse $collection
        dict set ::settings collectionInUse $collection
        # save settings into permanent storage:
        persistLocalData
        # remove all events
        set ::allEvents [ list ]
        # set all records as un-seen
        set ::allDbRecords [ dict create ]
        # re-load local events
        loadLocalDataFromStorage
        # load events of other operators from shared database:
        eventsByTimeAndCollection $::monthOnDisplay
        # and display currently selected day
        displayDay [ clock format $::monthOnDisplay -format "%d" ]
    }
}
#
# Callback procedure for adding a comment to an event
#
proc addCommentCallback { eventIdentifier } {
    puts [ format "addCommentCallback %s" $eventIdentifier ]
    # first see if we have any text..
    set commentText [ string trim [ .rightframe.dayframe.daycanvas.singleEventFrame.commentsFrame.newCommentEntry get ] ]
    if { [ string length $commentText ] < 1 } {
        return
    }
    # we had some text, so lets construct the comment.
    dict set newComment commentText $commentText
    dict set newComment commentTime [ clock seconds ]
    dict set newComment commentedEvent $eventIdentifier
    dict set newComment identifier [ calculateSHA1 [ format {%s-%s-%s} $eventIdentifier $::profileInUse [ clock seconds ] ] ]
    # fetch the event
    set ev [ eventById $eventIdentifier ]
    # comment publicity follows publicity of the event
    dict set newComment publicity [ dict get $ev publicity ]
    dict set newComment collection [ dict get $ev collection ]
    # in profile comment also set start time of the event commented,
    # it will help in deciding which is the right db record 
    dict set newComment eventStartTime [ dict get $ev startTime ]
    dict set newComment senderId $::profileInUse
    # getProfile will fail if operators profile is not published:
    if { [ catch {
	set p [ getProfile $::profileInUse ]
	dict set newComment senderName [ dict get $p displayName ]
    } fid ] } {
	dict set newComment senderName $::profileInUse
    }
    # see if this event already had local comments:
    if { [ dict exists $::allLocalComments $eventIdentifier ] } {
        # yes, this event has already been commented
        set eventCommentList [ dict get $::allLocalComments $eventIdentifier ]
        lappend eventCommentList $newComment
        # and put the list back into dictionary
        dict set ::allLocalComments $eventIdentifier $eventCommentList
        puts [ format "comment appended to existing event comment list of %s" $eventIdentifier ]
    } else {
        # No, event did not have any local comments. Create the first one
        set eventCommentList [ list ]
        lappend eventCommentList $newComment
        dict set ::allLocalComments $eventIdentifier $eventCommentList
        puts [ format "comment appended to new comment list of %s" $eventIdentifier ]
    }
    # see if this event already had any comments from any operator
    if { [ dict exists $::allComments $eventIdentifier ] } {
        # yes, this event has already been commented
        set eventCommentList [ dict get $::allComments $eventIdentifier ]
        lappend eventCommentList $newComment
        dict set ::allComments $eventIdentifier $eventCommentList
    } else {
        # this is first comment for given event
        set eventCommentList [ list ]
        lappend eventCommentList $newComment
        dict set ::allComments $eventIdentifier $eventCommentList
    }
    # then persist locally
    persistLocalData
    # and conditionally publish
    if { [ dict get $newComment publicity ] != {private} } {
        publishEvents $ev
    }
    updateEventCommentsDisplay $eventIdentifier 
}
#
# Procedure for updating comments of event on display. This updates
# the comments-section of event display.
#
proc updateEventCommentsDisplay { eventIdentifier } {
    if { [ dict exists $::allComments $eventIdentifier ] } {
        # yes, this event has already been commented
        .rightframe.dayframe.daycanvas.singleEventFrame.commentsDisplayFrame.oldCommentText configure -state normal
        # wipe out old content:
        .rightframe.dayframe.daycanvas.singleEventFrame.commentsDisplayFrame.oldCommentText delete 1.0 end
        set eventCommentList [ dict get $::allComments $eventIdentifier ]
        puts [ format "updateEventCommentsDisplay: %d comments for %s" [ llength $eventCommentList ] $eventIdentifier ]
        foreach c $eventCommentList {
            .rightframe.dayframe.daycanvas.singleEventFrame.commentsDisplayFrame.oldCommentText insert 1.0 [ format "%s (%s) %s:\n%s\n" [ dict get $c senderName ] [ dict get $c senderId ] [ clock format [ dict get $c commentTime ] -locale system ] [ dict get $c commentText ] ]
        }
        .rightframe.dayframe.daycanvas.singleEventFrame.commentsDisplayFrame.oldCommentText configure -state disabled
    } else {
        puts [ format "updateEventCommentsDisplay: no comments for %s" $eventIdentifier ]
    }
}

#
# data-model part.
# 
# data-model here is more or less the calendar-events. events may be
# created and deleted. there might be a "comment" on event, maybe something
# like "Yes, I will attend" 
#
# Event is a dictionary with values
# startTime : number, something that [ clock seconds ] returns. mandatory
# endTime : number, mandatory
# title : title for the event, mandatory. 
# description : longer description for the event, optional
# invitees : list of classified-ads operator profile addresses, SHA1-format, 
#            optional
#

#
# Procedure that returns events in currently open collection between start- 
# and end-date. Events are returned as a list of dictionaries. Resultset
# includes events in selected collection and events posted by operator. 
# From usage point of view that makes sense ; user always sees her own events
# and then additionally events of other operators that are in the 
# selected collection
#
proc eventsBetween { start end } {

    set retval [ list ]

    dict set dummyComparatorEvent startTime $start
    dict set dummyComparatorEvent endTime $end
    
    set eventsInStorage [ llength $::allEvents ]
    set i 0 
    while { $i < $eventsInStorage } {
        set evInStorage [ lindex $::allEvents $i ]
        if { [ overlaps $evInStorage $dummyComparatorEvent ] == true } { 
            # also, for display purposes select events that either belong
            # to selected collection or are posted by current operator
            if { ( [ dict get $evInStorage collection ] == $::collectionInUse ) ||
                 ( [ dict exists $evInStorage senderId ] &&
                   [ dict get $evInStorage senderId ] == $::profileInUse ) } {
                lappend retval $evInStorage
            }
        }
        incr i
    }
    return $retval
}
#
# Procedure that is called when user clicks on event on display.
# This displays details of the event for user to see.
#
proc displaySingleEvent { eventIdentifier } {
    puts [ format "displaySingleEvent %s" $eventIdentifier ]
    # get rid of possible previous display:
    catch closeSingleEventDisplay
    # which canvas to use
    set c .rightframe.dayframe.daycanvas
    # calculate size of canvas to create:

    # figure out canvas scroll-region size, first ask size of the
    # canvas, including possible unvisible parts
    set scSizeOption [ $c configure -scrollregion ]
    set regSize [ lindex $scSizeOption 4 ]
    set width [ lindex $regSize 2 ] 
    set height [ lindex $regSize 3 ]
    puts [ format "scroll area w %d h %d" $width $height ]
    # then figure out the visible rectangle
    set visibleYStart [ expr int ( $height * [ lindex [ $c yview ] 0 ] ) ]
    set visibleYEnd [ expr int ( $height * [ lindex [ $c yview ] 1 ] ) ]
    set visibleY [ expr ( $visibleYEnd - $visibleYStart ) - 50 ]
    set width [ expr $width - 50 ]
    puts [ format "frame size %d %d" $width $visibleY ]
    # first create frame to host contents of event display
    frame $c.singleEventFrame -borderwidth 2 -relief raised -width $width -height $visibleY

    # fetch the event
    set ev [ eventById $eventIdentifier ]
    if { $ev != {null} } {
        # then populate things into frame:
        label $c.singleEventFrame.titleLabel -text [ dict get $ev title ]
        # see if we know the event sender profile:
        set p [ getProfileAndEmptyIfNotFound [ dict get $ev senderId ] ]
        if { [ dict size $p ] == 0 } {
            # profile not found:
            label $c.singleEventFrame.senderLabel -text [ format "Posted by %s" [ dict get $ev senderId ] ]
        } else {
            label $c.singleEventFrame.senderLabel -text [ format "Posted by %s (%s)" [ dict get $p displayName ] [ dict get $ev senderId ] ]
        }
        label $c.singleEventFrame.startTime -text [ format "Start time is %s" [ clock format [ dict get $ev startTime ] -locale system ] ]
        label $c.singleEventFrame.endTime -text [ format "End time is %s" [ clock format [ dict get $ev endTime ] -locale system ] ]
        if { [ dict exists $ev description ] == false } {
            label $c.singleEventFrame.description -text { }
        } else {
            label $c.singleEventFrame.description -text [dict get $ev description]
        }
        frame $c.singleEventFrame.buttonFrame
        button $c.singleEventFrame.buttonFrame.closeButton -text {Close} -command closeSingleEventDisplay
        if { [ dict get $ev senderId ] == $::profileInUse } {
            button $c.singleEventFrame.buttonFrame.deleteButton -text {Delete event} -command "deleteEvent $eventIdentifier"
        }
        pack $c.singleEventFrame.titleLabel -anchor nw
        pack $c.singleEventFrame.senderLabel -anchor nw
        pack $c.singleEventFrame.startTime -anchor nw
        pack $c.singleEventFrame.endTime -anchor nw
        pack $c.singleEventFrame.description -anchor nw -ipadx 15 -ipady 15
        pack $c.singleEventFrame.buttonFrame.closeButton -side left
        if { [ dict get $ev senderId ] == $::profileInUse } {
            pack $c.singleEventFrame.buttonFrame.deleteButton -side left
        }
        pack $c.singleEventFrame.buttonFrame

        # comments-section of the event:
        frame $c.singleEventFrame.commentsFrame
        label $c.singleEventFrame.commentsFrame.newCommentLabel -text {New comment: }
        entry $c.singleEventFrame.commentsFrame.newCommentEntry
        button $c.singleEventFrame.commentsFrame.newCommentBtn -text Add -command "addCommentCallback $eventIdentifier"
        pack $c.singleEventFrame.commentsFrame.newCommentBtn -side right -anchor sw
        pack $c.singleEventFrame.commentsFrame.newCommentEntry -side right -anchor sw -fill x -expand yes
        pack $c.singleEventFrame.commentsFrame.newCommentLabel -side right -anchor sw
        pack $c.singleEventFrame.commentsFrame -fill x -expand yes
        # next is one single entry for all the comments posted by all
        # users:
        frame $c.singleEventFrame.commentsDisplayFrame
        label $c.singleEventFrame.commentsDisplayFrame.oldCommentLabel -text {Comments: }
        text $c.singleEventFrame.commentsDisplayFrame.oldCommentText -state disabled
        pack $c.singleEventFrame.commentsDisplayFrame.oldCommentText -side right -anchor sw -fill both -expand yes
        pack $c.singleEventFrame.commentsDisplayFrame.oldCommentLabel -side top -anchor sw
        pack $c.singleEventFrame.commentsDisplayFrame -fill both -expand yes

        # populate possible comments regarding the event:
        updateEventCommentsDisplay $eventIdentifier 

        # put the frame into window that is in the canvas:
        $c create window 25 [ expr $visibleYStart + 25 ] -window $c.singleEventFrame -width $width -height $visibleY -anchor nw -tags singleEventWindow
    } else {
        catch closeSingleEventDisplay
    }
}
#
# Procedure for closing single event display. UI-button callback. 
#
proc closeSingleEventDisplay { } {
    set c .rightframe.dayframe.daycanvas
    destroy $c.singleEventFrame
    $c delete singleEventWindow
}
#
# Procedure for deleting a previously posted event. This is UI-button
# callback. 
#
proc deleteEvent { eventIdentifier } {
    puts [ format "deleteEvent %s" $eventIdentifier ]
    # first delete from storage, will cause publish also if public event
    deleteEventFromStorage [ eventById $eventIdentifier ]
    # close the dialog showing the event about to be deleted
    closeSingleEventDisplay
    # and update the event display, now with one event missing
    displayDay [ clock format $::dayOnDisplay -format "%d" ]
}
#
# Procedure for comparing durations of 2 events
#
# a and b are events from procedure eventsBetween and
# this returns 1 if a has longer duration than b, 0
# if durations are equal.
#
proc durationComparator { a b } {
    set aStart [ dict get $a startTime ]
    set aEnd [ dict get $a endTime ]
    set bStart [ dict get $b startTime ]
    set bEnd [ dict get $b endTime ]
    set aDuration [ expr { $aEnd - $aStart } ]
    set bDuration [ expr { $bEnd - $bStart } ]
    if { $aDuration == $bDuration } {
        return 0 
    } 
    if { $aDuration > $bDuration } {
        return 1
    }
    return -1
}
#
# Procedure for comparing start-times of 2 events
#
# Used for sorting by start-time order. If 2 events
# have same start time, then by end-time
#
proc startComparator { a b } {
    set aStart [ dict get $a startTime ]
    set aEnd [ dict get $a endTime ]
    set bStart [ dict get $b startTime ]
    set bEnd [ dict get $b endTime ]
    if { $aStart == $bStart && $bEnd == $aEnd } {
        return 0 
    } 
    if { $aStart > $bStart } {
        return 1
    }
    if { $aStart < $bStart } {
        return -1
    }
    # start-times did not differ, next is to compare end times
    if { $aEnd > $bEnd } {
        return 1
    }
    if { $aEnd < $bEnd } {
        return -1
    }
    # following line should not be reached
    return 0
}
#
# Procedure for storing a single event
# 
proc storeNewEvent { ev } {
    # check if that particular event was already in storage
    set found false
    set eventsInStorage [ llength $::allEvents ]
    set i 0 
    while { $i < $eventsInStorage } {
        set evInStorage [ lindex $::allEvents $i ]
        if { [ dict get $ev identifier ] == [ dict get $evInStorage identifier ] } {
            set found true
            set ::allEvents [ lreplace $::allEvents $i $i $ev ]
            break
        }
        incr i
    }
    if { $found == false } {
        lappend ::allEvents $ev
    }
    puts [ format "after storeNewEvent num of events = %d" [ llength $::allEvents ] ]
    storeEventsLocally
    if { $::postPublicity == "byprofile" || $::postPublicity == "public" } {
        publishEvents $ev
    }
}
#
# Procedure for deleteting a single event
# 
proc deleteEventFromStorage { ev } {
    # check if that particular event was already in storage
    set found false
    set eventsInStorage [ llength $::allEvents ]
    set i 0 
    while { $i < $eventsInStorage } {
        set evInStorage [ lindex $::allEvents $i ]
        if { [ dict get $ev identifier ] == [ dict get $evInStorage identifier ] } {
            set found true
            # replace with empty -> is removal
            set ::allEvents [ lreplace $::allEvents $i $i ]
            break
        }
        incr i
    }
    puts [ format "after deleteEvent num of events = %d" [ llength $::allEvents ] ]
    if { $found == true } {
        storeEventsLocally
        if { $::postPublicity == "byprofile" || $::postPublicity == "public" } {
            publishEvents $ev
        }
    }
}
#
# Method for locally storing own events in program-specific database.
# Now, a caveat. TCL programs are stored only once per classified-ads
# installation. Locally stored data is a single file per program.
# If user has multiple profiles in use and uses calendar app on many
# of those, we must be careful to not accidentally delete events
# posted by other profiles the operator may be operating. 
# 
proc storeEventsLocally {} {
    # set list empty:
    set ::allLocalEvents [ list ]
    # pick own events 
    foreach event $::allEvents {
	if { [ dict get $event senderId ] == $::profileInUse } {
	    lappend ::allLocalEvents $event
	}
    }
    persistLocalData
}
#
# Method for persisting locally stored data (events and settings, in practice)
#
# Structure of the data storage is this:
# at top level there is dictionary. Keys into dictionary are of form
#  <profileId>-settings
#  <profileId>-events
# because there may be several operator profiles and idea is to keep
# settings and events of each operator intact
#
proc persistLocalData {} {
    # store local data in dictionary. one item is settings. another is event data.
    # yet one more are event comments
    set blob [ retrieveLocalData ]
    if { [ string length $blob ] == 0 } {
        set blob [ dict create ]
    } 
    dict set blob [ format "%s-settings" $::profileInUse ] $::settings
    dict set blob [ format "%s-events" $::profileInUse ] $::allLocalEvents
    dict set blob [ format "%s-comments" $::profileInUse ] $::allLocalComments
    # this extension method writes data to local database, specific to this particular program:
    storeLocalData $blob
}
#
# Method for loading locally stored data upon startup. See format description
# at procedure "persistLocalData" that is counterpart to this.
#
proc loadLocalDataFromStorage {} {
    set blob [ retrieveLocalData ]
    if { [ string length $blob ] > 0 } {
        if { [ dict exists $blob [ format "%s-settings" $::profileInUse ] ] } {
            set ::settings [ dict get $blob [ format "%s-settings" $::profileInUse ] ]
        }
        if { [ dict exists $blob [ format "%s-events" $::profileInUse ] ] } {
            set ::allLocalEvents [ dict get $blob [ format "%s-events" $::profileInUse ] ]
        }
        if { [ dict exists $blob [ format "%s-comments" $::profileInUse ] ] } {
            set ::allLocalComments [ dict get $blob [ format "%s-comments" $::profileInUse ] ]
        }
	# because this procedure is called only at start-up there is no need
	# merge events. the locally stored events are at this stage
	# all that we've got
	set ::allEvents $::allLocalEvents
        set ::allComments $::allLocalComments
    }
}
#
# Procedure that applies settings that must have been previously
# loaded using procedure loadLocalDataFromStorage - settings are
# stored in global dictionary ::settings
proc loadSettings {} {
    if { [ dict exists $::settings collectionInUse ] } {
        set ::collectionInUse [ dict get $::settings collectionInUse ]
        .rightframe.settingsframe.collectionEntry delete 0 end
        .rightframe.settingsframe.collectionEntry insert 0 $::collectionInUse
    }
}
#
# method that returns true if selected profile is public profile
#
proc isProfilePrivate {} {
    # getProfile will fail if operators profile is not published:
    if { [ catch {    
	set p [ getProfile $::profileInUse ]
	# profile is a dictionary
	if { [ dict get $p isPrivate ] > 0 } {
	    return true
	} else {
	    return false
	}
    } fid ] } {
	# if failed during query, assume it is unpublished and
	# assume that user wants to keep the profile secret. 
	return true
    }
}
#
# Publish procedures - publishes all public events in same month
# with given event. It could be possible to post every single
# event as db-record of its own but that would lead to huge
# number of records - lets instead list all events belonging to 
# same month into single database record and publish that. That
# will lead each user having about 12 db records per year. This
# method also makes it easier to detect situation where event
# is deleted by user.
#
# If user has public profile, all published events will be
# published with "public" setting because that is users
# selected setting. Otherwise, if user has "private" profile
# setting, and event is set to be sent with "byprofile" 
# setting, then event will be encrypted only to readers of 
# the current profile. 
#
# Additionally this procedure takes care of including 
# event comments into published record. 
#
proc publishEvents { newOrDeletedEvent } {
    # check out publicity setting of the new event, it determines
    # the database record where it will be published. 
    set publicity [ dict get $newOrDeletedEvent publicity ]

    # for db record pick up every public event by me
    # whose start time is in the same month. 
    set startTime [ dict get $newOrDeletedEvent startTime ]
    set monthStart [ startOfMonth $startTime ]
    set monthEnd [ endOfMonth $startTime ]
    # pad start+end with one day to get around events not
    # beloning to same month in all timezones:
    set monthStart [ clock add $monthStart -1 day ]
    set monthEnd [ clock add $monthEnd 1 day ]
    # fetch all events of month, will include events from any operator:
    set allEventsOfMonth [ eventsBetween $monthStart $monthEnd ]
    
    set myEventsToBePublished [ list ]
    set myCommentsToBePublished [ list ]
    # pick own events that go selected collection
    foreach event $allEventsOfMonth {
        if { [ dict get $event publicity ] != $publicity } {
            # not our kind of event, don't publish
            continue
        }
        if { [ dict get $event collection ] != $::collectionInUse } {
            # event belongs to other collection than the one we're
            # publishing now
            continue
        }
        # at this stage we've noticed that we're dealing with
        # event that has suitable publicity setting and belongs
        # to right collection. lets see if there is comments
        # regarding that event that should go to same db record:
        if { [ dict exists $::allLocalComments [ dict get $event identifier ] ] } {
            # yes, this event has already been commented
            set eventCommentList [ dict get $::allLocalComments [ dict get $event identifier ] ]
            puts [ format "For publish event %s has %d comments" [ dict get $event identifier ] [ llength $eventCommentList ] ]
            foreach c $eventCommentList {
                lappend myCommentsToBePublished $c
            }
        } else {
            puts [ format "For publish event %s had no comments" [ dict get $event identifier ] ]
        }
        # then check by sender id if this was our event, and decide
        # if that should be published
        if { [ dict get $event senderId ] != $::profileInUse } {
            # not my event, don't publish
            continue
        }
        lappend myEventsToBePublished $event
    }
    # fetch possible previous version of the months record
    dict set searchDict searchPhrase [ searchString [ dict get $newOrDeletedEvent startTime ] ]
    dict set searchDict senderId $::profileInUse 
    dict set searchDict collectionId [ calculateSHA1 $::collectionInUse ]
    # getDbRecord returns a list if dictionaries. each dictionary has
    # key "data" that is our actual payload
    set previousDbRecords [ getDbRecord $searchDict ]
    set previousRecordFound false
    foreach r $previousDbRecords {
        if { [ dict exists $r data ] } {
            set recordData [ dict get $r data ]
            if { [ dict exists $recordData publicity ] } {
                set publicityOfRecord [ dict get $recordData publicity ]
                if { $publicityOfRecord == $publicity } {
                    set existingRecord $r
                    set previousRecordFound true
                    break
                }
            }
        }
    }
    if { $previousRecordFound == true } {
        puts "In publish found previous record whose data is updated"
        set recordToSave $existingRecord
        set recordData [ dict get $existingRecord data ]
        # replace the events in data:
        dict set recordData events $myEventsToBePublished
        dict set recordData comments $myCommentsToBePublished
        dict set recordToSave data $recordData
    } else {
        # construct a brand new record, record is a dictionary
        puts "In publish no previous record found"
        dict set recordToSave collectionId [ calculateSHA1 $::collectionInUse ]
        dict set recordToSave searchPhrase [ searchString [ dict get $event startTime ] ]
        if { $publicity == {public} || ( [ isProfilePrivate ] != true && $publicity == {byprofile} ) } {
            dict set recordToSave encrypted false
        } else {
            dict set recordToSave encrypted true
        }
        # construct another dictionary for the record data
        dict set recordData publicity $publicity
        dict set recordData startTime $monthStart
        dict set recordData endTime $monthEnd
        dict set recordData events $myEventsToBePublished
        dict set recordData comments $myCommentsToBePublished
        # and finally the events+comments:
        dict set recordToSave data $recordData
    }
    # record is ready, publish:
    publishDbRecord $recordToSave
}

# 
# method that returns beginning of month where given time belongs
# to
#
proc startOfMonth { timeStamp } {
    set yearNumber [ clock format $timeStamp -format {%Y} ]
    set monthNumber [ clock format $timeStamp -format {%N} ]
    set timeOfstartOfMonth [clock scan [ format "%s %s 01 00 00 00" $yearNumber $monthNumber ] -format {%Y %m %d %H %M %S} -gmt false ]
    return $timeOfstartOfMonth
}
# 
# method that returns end of month where given time belongs to
#
proc endOfMonth { timeStamp } {
    set timeOfStartOfMonth [startOfMonth $timeStamp ]
    set timeOfEndOfMonth [ clock add $timeOfStartOfMonth 1 month ]
    return $timeOfEndOfMonth
}
#
# method that produces db-record search string based on event start time.
# search string has structure "monXX-YYYY" where XX is month number
# and YYYY is year and it is calculated according to event start 
# time, using UTC timezone. Because events starting first hours of 
# month may appear in previous month in some timezones, it is 
# convenient to pad each month to contain also events from start+end
# of next month+prev month. 
# 
proc searchString { startTime } {
    return [ clock format  $startTime -format {mon%m-%Y} -locale UTC ]
}
#
# Procedure for fetching events from database by collection
# and time. Updates in-memory catalog of already seen db records
# to make it easier to prune deleted events when same db-record
# is published again by another operator. Does not return anything
# but updates the global ::allEvents list.
#
# parameter startTime determines the time, global variable ::collectionInUse
# the collection
#
proc eventsByTimeAndCollection {startTime} {
    # set search phrase to search dict
    dict set searchDict searchPhrase [ searchString $startTime ]
    # set collection id to search dict
    dict set searchDict collectionId [ calculateSHA1 $::collectionInUse ]
    # do actual search
    set dbRecords [ getDbRecord $searchDict ]
    # then loop through results. note bad algorithm .. this goes through
    # $::allEvents as many times as there are records in db so this is
    # O(n*m) while we could extract all changes from records in first pass
    # and then loop the $::allEvents with all changes to reduce the load
    # slightly
    foreach r $dbRecords {
        # if throws an error, $fid will contain error message
        if { [ catch {
            updateOneDbRecordIntoEventCollection $r
        } fid ] } {
            puts [ format {updateOneDbRecordIntoEventCollection: %s} $fid ]
        }
    }
}

#
# Proc that updates content of one db record into in-memory event
# collection. Should be called via "catch" method because input comes
# from other nodes and can be potentially *anything* 
#
proc updateOneDbRecordIntoEventCollection { r } {
    # see if this record has been seen already:
    set recordIsSeenAndNew false
    if { [ dict exists $::allDbRecords [ dict get $r recordId ] ] } {
        # yes, lets see if timestamp is the same
        set timeOfPreviouslyPublished [ dict get $::allDbRecords [ dict get $r recordId ] ]
        set timeOfNewRecord [ dict get $r timeOfPublish ]
        if { $timeOfNewRecord < $timeOfPreviouslyPublished } {
            # record has been seen and is same, or older:
            return
        } else {
            set recordIsSeenAndNew true
        }
    }
    # if my own record, we can skip because all my events should
    # be in local storage also so lets not bring them in twice:
    if { [ dict get $r senderId ] == $::profileInUse } {
        return
    }

    # stupid validity check:
    if { [ dict exists $r data ] == false } {
        # ugh, malformed record it must be, yoda ask for details
        return
    }

    set recordData [ dict get $r data ]

    # first remove all events by operator of record and matching publicity

    # each operator for given month (== searchPhrase) has either public or
    # private record, or both but no more than that, lets remove events
    # from in-memory list based on operator and publicity
    if { $recordIsSeenAndNew == true } {
        removeEventsByOperatorAndPublicity [ dict get $r senderId ] [ dict get $recordData publicity ]
        removeEventsByRecordId [ dict get $r recordId ]
    }
    # then, after events from that data record are not in 
    # our in-memory collection any more, lets add them
    # back
    foreach e [ dict get $recordData events ] {
        # before appending event to in-memory collection
        # set event owner to event, key in dictionary is senderId:
        dict set e senderId [ dict get $r senderId ]
        # before appending event to in-memory collection
        # set id of the originating db record
        dict set e recordId [ dict get $r recordId ]
        puts [ format "Adding from record %s event %s" [ dict get $r recordId ] [ dict get $e identifier ] ]
        lappend ::allEvents $e
    }
    # After events from record have been processed, lets continue 
    # with comments. Note how ::allComments is a dict where 
    # key is the commented event id ; published comments in turn
    # are just a list of dictionaries and the event id is
    # inside each comments dictionary. 
    if { [ dict exists $recordData comments ] } {
        foreach c [ dict get $recordData comments ] {
            # before appending comment to in-memory collection
            # set event owner to comment, key in dictionary is senderId:
            dict set c senderId [ dict get $r senderId ]
            # before appending comment to in-memory collection
            # set id of the originating db record
            dict set c recordId [ dict get $r recordId ]
            puts [ format "Adding from record %s comment %s" [ dict get $r recordId ] [ dict get $c identifier ] ]
            updateOneCommentIntoDictionary $c
        }
    }
    # update dictionary that keeps cache about which dbrecord has been seen
    # and which not:
    dict set ::allDbRecords [ dict get $r recordId ] [ dict get $r timeOfPublish ]
}
#
# procedure that removes from event collection "all events" by operator+publicity
#
proc removeEventsByOperatorAndPublicity { operator publicity } {
    # loop through events from end to start, removing
    # every event matching search criteria
    set listLen [ expr [ llength $::allEvents ] - 1 ]
    for { set i $listLen} { $i >= 0 } { set i [ expr $i - 1 ] } {
        set eventInList [ lindex $::allEvents $i ]
	if { [ dict exists $eventInList senderId ] && [ dict exists $eventInList publicity ] } { 
	    set eventOperator [ dict get $eventInList senderId ]
	    set eventPublicity [ dict get $eventInList publicity ]
	    if { $eventOperator == $operator && $eventPublicity == $publicity } {
		# remove
                puts [ format "Removing by operator event %s" [ dict get $eventInList identifier ] ]
		set ::allEvents [ lreplace $::allEvents $i $i ]
	    }
	}
    } 
}

#
# procedure that removes from event collection "all events" by record identifier
#
proc removeEventsByRecordId { recordIdToRemove } {
    # loop through events from end to start, removing
    # every event matching given record id
    set listLen [ expr [ llength $::allEvents ] - 1 ]
    for { set i $listLen} { $i >= 0 } { set i [ expr $i - 1 ] } {
        set eventInList [ lindex $::allEvents $i ]
	if { [ dict exists $eventInList recordId ] } { 
	    set eventRecordId [ dict get $eventInList recordId ]
	    if { $eventRecordId == $recordIdToRemove } {
		# remove
		set ::allEvents [ lreplace $::allEvents $i $i ]
	    }
	}
    } 
}
#
# Procedure for digging out one event by its id
#
proc eventById { identifier } {
    foreach event $::allEvents {
	if { [ dict get $event identifier ] == $identifier } {
	    return $event
	}
    }
    return null
}
#
# procedure that returns operator profile or if no profile is
# found then empty dictionary
#
proc getProfileAndEmptyIfNotFound { identifier } {
    if { [ catch { 
	set p [ getProfile $identifier ]
    } fid ] } {
        # error
        return [ dict create ]
    } else {
        return $p 
    }
}
#
# procedure that updates one event comment into in-memory dictionary
#
proc updateOneCommentIntoDictionary { comment } {
    if { [ dict exists $comment identifier ] && [ dict exists $comment commentedEvent ] } {
        set eventIdentifier [ dict get $comment commentedEvent ]
        set commentIdentifier [ dict get $comment identifier ]
        if { [ dict exists $::allComments $eventIdentifier ] } {
            # yes, this event has already been commented
            set eventCommentList [ dict get $::allComments $eventIdentifier ]
            # see if this particular comment already appears in the list:
            set commentAlreadySeen false
            foreach c $eventCommentList {
                if { [ dict get $c identifier ] == $commentIdentifier } {
                    # yes, is seen
                    return
                }
            }
            # if we got here, this particular comment is not yet on list:
            lappend eventCommentList $comment
            dict set ::allComments $eventIdentifier $eventCommentList
            puts [ format "Appending comment %s from published record" $commentIdentifier ]
        } else {
            # this is first comment for given event
            set eventCommentList [ list ]
            lappend eventCommentList $comment
            dict set ::allComments $eventIdentifier $eventCommentList
            puts [ format "Creating comment list for comment %s from published record" $commentIdentifier ]
        }
    }
}
# 
# Notification procedure that classified-ads host program calls whenever
# there is new content added to database. 
#
proc dataItemChanged { itemHash itemType } {
    if { $itemType != {dbrecord} } {
        return ; 
    }
    # it was db record, see if we're supposed to be interested:
    set currentSearchPhrase [ searchString $::monthOnDisplay ]
    set collectionHash [ calculateSHA1 $::collectionInUse ]
    dict set searchDict collectionId $collectionHash
    dict set searchDict recordId $itemHash
    set dbRecords [ getDbRecord $searchDict ]
    foreach r $dbRecords {
        set recordCollectionHash [ dict get $r collectionId ]
        if { $recordCollectionHash == $collectionHash } {
            if { [ dict exists $r data ] && [ dict exists $r searchPhrase ] } {
                set recordSearchPhrase [ dict get $r searchPhrase ]
                if { [ string last $currentSearchPhrase $recordSearchPhrase ] != -1 } {
                    # if throws an error, $fid will contain error message
                    if { [ catch {
                        updateOneDbRecordIntoEventCollection $r
                    } fid ] } {
                        puts [ format {updateOneDbRecordIntoEventCollection: %s} $fid ]
                    } else {
                        # update display
                        displayDay [ clock format $::dayOnDisplay -format "%d" ]
                    }
                } 
            } 
        } 
    }    
}
#
# execution starts here
# 
initGlobals
initUI
# load local events, not posted in public:
loadLocalDataFromStorage
# process settings
loadSettings
# load events of other operators from shared database:
eventsByTimeAndCollection $::monthOnDisplay
showDayView
displaySelectionCalendar $::monthOnDisplay
set ::initReady true
displayDay [ clock format $::monthOnDisplay -format "%d" ]
updateSelectionButtonsState $::monthOnDisplay
