proc stable_weight {value unit} {
  global state_stable_results
  set state_stable_results 1
}

proc calibration_setting_show { parameter mode weight valueunit } {
  set str_mode "Unknown"
  set str_weight "Unknown"
  switch -- $mode {
    0 {set str_mode "Manual" }
    1 {set str_mode "Auto (Done)" }
    2 {set str_mode "Auto (Required)" }
  }
  switch -- $weight {
    0 {set str_weight "Internal" }
    1 {set str_weight "External" }
  }
  puts "calibration setting: mode=$str_mode weight=$str_weight value_unit=$valueunit"
}

proc calibration_set { parameter } {
  global state_calibration
  
  switch -- $parameter {
      "A" {
        puts "Calibration setting set"
        set state_calibration 1
        request_internal_calibration
      }
      "L" {
        puts "Cannot set Calibration setting (wrong paramter)"
      }
      "I" {
        puts "Cannot set Calibration setting (busy)"
      }
  }

}

proc internal_calibration { parameter } {
  global state_calibration
  global email
  
  switch -- $parameter {
    "B" {
      puts "Internal calibration started"
      if { $state_calibration == 1 } {
        set state_calibration 2
      }
    }
    "A" {
      puts "Internal calibration completed successfully"
      if { $state_calibration == 2 } {
        set state_calibration 3
        puts "Mailing calibration result"
        email $email "Digital Scale" "SG-32001 Internal Calibration OK\n-- Mettler-Toledo"
      }
    }
    "I" {
      puts "Internal calibration aborted"
      set state_calibration 0
      email $email "Digital Scale" "SG-32001 Internal Calibration Aborted\n-- Mettler-Toledo"
    }
    "L" {
      puts "Internal calibration not possible"
      set state_calibration 0
      email $email "Digital Scale" "SG-32001 Internal Calibration Not Possible\n-- Mettler-Toledo"
    }
  }
}

proc interprete { line } {
  if { [regexp "^PWR (\[ALI\])$$" $line -> m1] } {
    switch -- $m1 {
      "A" {
        puts "power on"
      }
      "L" {
        puts "wrong paramter for power on"
      }
      "I" {
        puts "unable to execute power on (busy)"
      }
    }
  } elseif { [regexp "^S S +(\[-.0-9\]+) +(\[lbkmgoz\]+)$$" $line -> value unit] } {
    puts "Stable weight: $value $unit"
    puts [format "01A%+08.1f\r" $value]
    stable_weight $value $unit
  } elseif { [regexp "^S (\[-+LI\])$$" $line -> m1] } {
    switch -- $m1 {
      "-" {
        puts "Balance in NEGATIVE OVERLOAD range"
      }
      "+" {
        puts "Balance in OVERLOAD range"
      }
      "L" {
        puts "Send stable weight error (wrong paramter)"
      }
      "I" {
        puts "Unable to execute send stable weight (busy)"
      }
    }
  } elseif { [regexp "^C3 (\[BALI\])$$" $line -> m1] } {
    internal_calibration $m1
  } elseif { [regexp "^I4 A (\[\"0-9\]+)$$" $line -> serial] } {
    puts "Balance serial no: $serial"
    reset_state
    initialize_scale $serial
  } elseif { [regexp "^C0 (\[ALI\])$$" $line -> m1] } {
    calibration_set $m1
  } elseif { [regexp "^C0 (\[ALI\]) (\[0-2\]) (\[0-1\]) \"(.*)\"$$" $line -> m1 mode weight valueunit ] } {
    calibration_setting_show $m1 $mode $weight $valueunit
  } elseif { [regexp "^$$" $line] } {
    # puts "empty line"
  } elseif { [regexp "^ser2net port (.*) device (.+) \[\[\](.+)\[\]\] \[(\](.+)\[)\]$$" $line -> port device mode name ] } {
    puts "connected balance: $name"
  } elseif { [regexp "^EL$$" $line] } {
    puts "error response"
  } elseif { [regexp "^(.*)$$" $line -> m1 ] } {
    puts "unknown response: $m1"
  }

}
