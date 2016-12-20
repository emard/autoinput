
proc email { to subject message } {
  # puts "sending mail\nsubject: $subject\nmessage: $message\n"

  # set pid [eval spawn -noecho "cat"]
  set pid [eval spawn -noecho "/usr/sbin/rmail $to"]
  set sid $spawn_id

  puts $sid "Subject: $subject\n$message"
  # send ctrl-d
  puts $sid "\x04"
  flush $sid
  wait -i $sid
}
