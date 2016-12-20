trap sighup_handler SIGHUP

proc sighup_handler { } {
  puts "SIGHUP"
  calibration_configure
}



