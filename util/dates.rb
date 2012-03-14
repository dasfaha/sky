#!/usr/bin/ruby

require 'date'

# Generate a table of dates with timestamps (ISO, DECIMAL, HEX).
year = 2012

puts "DATE                 | DECIMAL              | HEX"
puts "--------------------------------------------------------------"

(1..12).each do |month|
  str = "#{year}-#{'%02d' % month}-01T00:00:00Z"
  date = DateTime.parse(str)
  timestamp = date.strftime('%Q').to_i
  hex = [timestamp].pack('q').bytes.map{|x| "%02x" % x}.join('')
  
  puts "%s | %-20d | %s" % [str, timestamp, hex]
end

puts