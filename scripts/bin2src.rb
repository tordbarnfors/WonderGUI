#!/usr/bin/ruby
# Generate c-file from a binary file.
# Saves to file with same name but with c-extension.


for fileName in $*
  
  data = File.open(fileName, 'rb') { |f| f.read }

  values = data.unpack("c*");


  
  arrayName = File.basename(fileName)

  replacements = { '-' => '_', ' ' => '_', '.' => '_' }

   arrayName.gsub!(Regexp.union(replacements.keys), replacements)


  sourceName = fileName + ".c";
  headerName = fileName + ".h";


  f = File.new( headerName, "w")

    f.puts "extern char " + arrayName + "[" + (values.length+1).to_s + "];"
    f.puts "#define " + arrayName + "_length " + values.length.to_s

  f.close


  rows = 0
  f = File.new( sourceName, "w")
  f.print "#pragma pack(push, 4)\n"
  f.print "char " + arrayName + "[" + (values.length+1).to_s + "] = { "


  for c in values

    f.print c.to_s + ", "
    rows += 1

    if( rows > 20 )
      f.print "\n"
      rows = 0
    end


  end

  f.print "0 };\n"
  f.print "#pragma pack(pop)\n"


  f.close

end
