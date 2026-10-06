#!/usr/bin/ruby

def printUsage()
  puts "hicolordef input output"
  puts
end



if( ARGV.length < 2 )
  printUsage()
  exit
end


inputFile = ARGV[0]
outputFile = ARGV[1]

input = IO.readlines(inputFile)
output = []

for line in input
  valuestring = line[ /0x\h+/ ] 

  value = valuestring.to_i(16)

  blue = (value % 256) / 255.0;
  green = ((value >> 8) % 256) / 255.0
  red = ((value >> 16) % 256) / 255.0
  alpha = ((value >> 24) % 256) / 255.0

  # HiColor is sRGB just like the hex value, so channels are only rescaled.

  outstring = (red*4096+0.5).to_i.to_s + ', ' + (green*4096+0.5).to_i.to_s + ', ' + (blue*4096+0.5).to_i.to_s + ', ' + (alpha*4096+0.5).to_i.to_s
  
  output << line.sub(valuestring,outstring)
end

f = File.new( outputFile, "w")
f.puts output
f.close


