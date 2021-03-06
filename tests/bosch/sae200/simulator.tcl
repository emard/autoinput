#!/usr/bin/expect

# --------- layer P (physical) user configurable --------
set port "/dev/pts/10"
# --------- end user config
set mode "1200,o,7,1"
# ---------- end layer P (physical)

# simple example
# type C0<ctrl-m><ctrl-j>
# nothing will be echoed until all is typed exactly as above
# then it should respond with C0

set blocking 0
set useevent 0
set active 0


log_user 0
# important: do not remove null chars by default
exp_remove_nulls -d 0

# fconfigure stdin -blocking 1 -buffering none -buffersize 4096 -encoding binary -translation binary -eofchar {}
# trap exit SIGINT
## this is used in stdin mode
# stty raw -echo

set success 1
set responsive 0

# balance state vars
set send_data_on_keypress 1
set stable_weight  12.3456
set dynamic_weight 12.1021
set serial_number "0123456789"

proc openport {} {
  global port mode
  global blocking useevent active
  global fid pid sid

  if { $blocking } {
    set fid [open $port {RDWR}]
  } else {
    set fid [open $port {RDWR NONBLOCK}]
  }
  set active 1
  fconfigure $fid -mode $mode -buffering none -buffersize 4096 -translation binary -blocking $blocking -encoding binary -eofchar {}
  if { $useevent } {
    fileevent  $fid readable [list read_layer0_fileevent $i]
  } else {
    set pid [eval spawn -noecho -open $fid]
    set sid $spawn_id
  }
}

proc calibration {} {
  global sid
  global success
  global responsive
  set responsive 0  

  puts -nonewline $sid "C3 B\x0d\x0a"
  if {$success} {
    	  sleep 2
    	  puts -nonewline $sid "C3 A\x0d\x0a"
  } else {
    	  sleep 1
    	  puts -nonewline $sid "C3 I\x0d\x0a"
  }

  set responsive 1
}

# daemon which will autorespawn, sending stable weight every 20 sec
proc sendstableweight {} {
  global sid
  global stable_weight
  global send_data_on_keypress
  
  if {$send_data_on_keypress} {
    set value [format "%s %s" \
      [string range           [format "%+f"    $stable_weight] 0 0] \
      [string map { "-" " " } [format "%8.4f"  $stable_weight] ] ]
    puts -nonewline $sid [format "%s g  \x0d\x0a" $value]
    puts [format "sending stable weight \"%s g  \"" $value]
  }
  after 500 sendstableweight
}

openport

# start daemon
sendstableweight

while {0} {
  if {$responsive} {
    expect {
      -i $sid
      -re "C0\x0d\x0a" {
    	puts -nonewline $sid "C0\x0d\x0a"
      }
      -re "C0 (\[0-1\]) (\[0-1\])\x0d\x0a" {
        if {$expect_out(2,string) == 0} {
          set success 1
        } else {
          set success 0
        }
    	puts -nonewline $sid "C0 A\x0d\x0a"
      }
      -re "C3\x0d\x0a" {
        calibration
      }
      -re "I4\x0d\x0a" {
    	puts -nonewline $sid [format "I4 A \"%s\"\x0d\x0a" $serial_number]
      }
      -re "I4\x0d\x0a" {
    	puts -nonewline $sid [format "I4 A \"%s\"\x0d\x0a" $serial_number]
      }
      -re "TIM\x0d\x0a" {
        puts -nonewline $sid [clock format [clock seconds] -format "TIM A %H %M %S\x0d\x0a" ]
      }
      -re "DAT\x0d\x0a" {
        puts -nonewline $sid [clock format [clock seconds] -format "DAT A %d %m %Y\x0d\x0a" ]
      }
      -re "S\x0d\x0a" {
        # send stable weight
    	puts -nonewline $sid [format "S S %9.1f g\x0d\x0a" $stable_weight]
      }
      -re "SI\x0d\x0a" {
        # send immediately (stable or dynamic) weight
    	puts -nonewline $sid [format "S D %9.1f g\x0d\x0a" $dynamic_weight]
      }
      -re "ST\x0d\x0a" {
    	puts -nonewline $sid [format "ST A %d\x0d\x0a" $send_data_on_keypress]
      }
      -re "ST 1\x0d\x0a" {
        set send_data_on_keypress 1
    	puts -nonewline $sid "ST A\x0d\x0a"
      }
      -re "ST 0\x0d\x0a" {
        set send_data_on_keypress 0
    	puts -nonewline $sid "ST A\x0d\x0a"
      }
      -re "Z\x0d\x0a" {
        # zero the balance (tare)
        set stable_weight 0.0
    	puts -nonewline $sid "Z A\x0d\x0a"
      }
      -re "@\x0d\x0a" {
        # reset the balance
    	puts -nonewline $sid [format "I4 A \"%s\"\x0d\x0a" $serial_number]
      }
    }
  } else {
    expect {
      -re ".*" {
      }
    }  
  }
  
}


set forever 0
vwait forever
