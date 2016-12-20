
# --------- layer P (physical) user configurable --------

# for using local serial port on this machine
set port "/dev/ttyUSB.MT"
set mode "9600,e,7,1"

# for using remote serial port over tcp (set2net):
#set host "tux"
#set port 16551

# uncomment just one:
set comm "serial"
#set comm "tcp"

# ---------- end layer P (physical)

# blocking of the port
set blocking 1
# use fileevent
set useevent 1
# is the port active (open)
set active 0
# reopen the port if it closes
set reopen 1

# ----------------------------------

proc logtext { text } {
    global t0
    set t [expr [clock clicks -milliseconds] - $t0]
    puts stdout [format "%s@%d: %s" "log" $t $text ]
}


proc openserial {} {
  global port mode
  global blocking useevent active
  global fid pid sid

  if { $blocking } {
    set fid [open $port {RDWR}]
  } else {
    set fid [open $port {RDWR NONBLOCK}]
  }
  set active 1
  # fconfigure $fid -mode $mode -buffering none -buffersize 4096 -translation binary -blocking $blocking -encoding binary -eofchar {}
  fconfigure $fid -mode $mode -buffering none -buffersize 4096 -translation crlf -blocking $blocking -encoding binary -eofchar {}
  if { $useevent } {
    fileevent  $fid readable [list read_layer0_fileevent 0]
  } else {
    set pid [eval spawn -noecho -open $fid]
    set sid $spawn_id
  }
}

proc opentcp {} {
  global host port
  global fid pid sid
  global blocking useevent active

  set fid [socket $host $port]
  fconfigure $fid -buffering none -buffersize 4096 -translation crlf -blocking $blocking -encoding binary -eofchar {}
  if { $useevent } {
    fileevent  $fid readable [list read_layer0_fileevent 0]
  } else {
    set pid [eval spawn -noecho -open $fid]
    set sid $spawn_id
  }
  set active 1
}

proc opencomm {} {
  global comm

  if { $comm == "serial" } {
    openserial
  }
  if { $comm == "tcp" } {
    opentcp
  }  
}

proc read_layer0_fileevent { param1 } {
  global fid other port reopen t0 rxcount active packstart

  # set t [expr [clock clicks -milliseconds] - $t0]
  # if { [catch {set data [read $fid 512]} probvar] } 
  if { [catch {set data [gets $fid]} probvar] } {
    logtext $probvar
    set active 0
    close $fid
    if { $reopen } {
      opencomm
    }
  } else {
    # puts "'$data'"
    interprete $data
    #puts stdout [format "%s@%d: %s" $port($i) $t [str2hex $data] ]
    #incr rxcount($i) [string length $data]
  }
}
