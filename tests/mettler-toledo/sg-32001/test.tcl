#!/usr/bin/expect

source "./config.inc.tcl"
source "./exception.inc.tcl"
source "./email.inc.tcl"

email $email "Digital Scale" "Internal Calibration OK\n\n-- Mettler-Toledo\n"
#vwait forever
