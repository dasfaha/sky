#!/usr/bin/ruby

require 'date'

# Converts an ISO 8601 date to little-endian byte code.
str = ARGV.first
if str.nil?
  puts "usage: date.rb [ISO8601_DATE]\n\n"
  exit(0)
end


date = DateTime.parse(str)
timestamp = date.strftime('%Q').to_i
hex = [timestamp].pack('q').bytes.map{|x| "%02x" % x}.join('')
puts "TIMESTAMP: #{timestamp}"
puts "BYTES:     #{hex}"
puts
