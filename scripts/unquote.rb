#!/usr/bin/ruby
# Replaces keyword in template file with quoted lines of specified file.
# Saves to file with same name without .template extension.

def printUsage()
  puts "insert_quoted file.template"
  puts
end

for fileName in $*
  
   input = IO.readlines(fileName)
   output = []

   outputName = fileName + ".unquoted";

    for line in input

      line.chomp!
      if( line =~ /^".*"$/ )
          line = line[1..-2];

          line.gsub! '\n', "\n" 
          output << line
      end
    end

    f = File.new( outputName, "w")
    f.puts output
    f.close


end

