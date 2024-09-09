require 'socket'
startTime = Process.clock_gettime(Process::CLOCK_MONOTONIC)

# Connect to the server running on localhost at port 8989
s = TCPSocket.new 'localhost', 8989

# Send the file path to the server, based on the argument passed from the command line
s.write("testfiles/#{ARGV[0]}.c\n")

# Receive and print the response from the server
# s.each_line do |line|
#   puts line
# end

# Close the socket
s.close
endTime = Process.clock_gettime(Process::CLOCK_MONOTONIC)
elapsed = endTime - startTime;
puts "Elapsed: #{elapsed} (#{ARGV[0]})"