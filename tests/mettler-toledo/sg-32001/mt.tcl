#!/usr/bin/expect

source "./config.inc.tcl"
source "./comm.inc.tcl"
source "./email.inc.tcl"
source "./interprete.inc.tcl"
source "./exception.inc.tcl"

log_user 0
# important: do not remove null chars by default
exp_remove_nulls -d 0

global state_stable_results
global state_calibration

proc reset_state {} {
  global state_stable_results
  set state_stable_results 0
  set state_calibration 0
}

proc command { query } {
  global sid fid

  puts "command: $query"
  puts $fid $query
}

proc poweron {} {
  global sid fid

  set query "PWR 1"
  puts "query: $query"
  puts $fid $query
}

proc implemented {} {
  global sid fid

  set query "I0"
  puts "query: $query"
  puts $fid $query
}

proc request_stable_results {} {
  global sid fid

  set query "SNR"
  puts "query: $query"
  puts $fid $query
}

proc request_stable_results_on_keypress {} {
  global sid fid

  set query "ST 1"
  puts "query: $query"
  puts $fid $query
}

proc initialize_scale { serialno } {
  global scale_serial
  set scale_serial $serialno
  # request_stable_results
  request_stable_results_on_keypress
}

proc calibration_configure {} {
  set mode 0
  command "C0 1 $mode"
}

proc request_internal_calibration {} {
  command "C3"
}

proc watchdog {} {
  request_stable_results_on_keypress
  after 60000 watchdog
}

# set sid sidSimulator
opencomm

## uncomment those for stdin-based simulator
#set pidSimulator [eval spawn -noecho "./simulator-stdin.tcl"]
#set sidSimulator $spawn_id
#sleep 1

#after 7000 configure 1
#after 12000 configure 0
reset_state

after 1000 poweron
after 2000 {command "I4"}
after 3000 watchdog
#after 6000 {calibration_configure}

vwait forever
