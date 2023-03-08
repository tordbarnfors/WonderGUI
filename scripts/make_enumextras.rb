#!/usr/bin/ruby
# Update wg_enumextras.h/cpp with functions for all enums in wg_types.h marked "autoExtras".


	Task = Struct.new( :input, :header_out, :source_out )

	$tasks = []

	$tasks << Task.new( 'src/gear/wg_geartypes.h', 'src/gear/wg_gearenumextras.h', 'src/gear/wg_gearenumextras.cpp' )
	$tasks << Task.new( 'src/gfx/wg_gfxtypes.h', 'src/gfx/wg_gfxenumextras.h', 'src/gfx/wg_gfxenumextras.cpp' )
	$tasks << Task.new( 'src/gui/base/wg_types.h', 'src/gui/base/wg_enumextras.h', 'src/gui/base/wg_enumextras.cpp' )


def replaceAutoSection( path, newContent )

  lines = IO.readlines(path)

  f = File.new( path, "w")

  i = 0
  until ( lines[i] =~ /^\s*\/\/\./ and lines[i] =~ /startAutoSection/ )
		f.puts lines[i]
		i+=1
  end

 	f.puts '//. startAutoSection'
	f.puts '/*========================================================================='
	f.puts ''
	f.puts '				>>> START OF AUTO GENERATED SECTION <<<'
	f.puts ''
	f.puts '				Any modifications here will be overwritten!'
	f.puts ''
	f.puts '=========================================================================*/'
	f.puts ''
	f.puts newContent
	f.puts ''
	f.puts '//========================================================================='
 	f.puts '//. endAutoSection'

	i += 1 until ( lines[i] =~ /^\s*\/\/\./ and lines[i] =~ /endAutoSection/ )
	i += 1

	f.puts lines[i..lines.size]

	f.close
end

def processInputFile( path, header, source )

	lines = IO.readlines(path)

	enumName = nil
	enumBlock = ""
	inside = false;

	toStringDeclarations = []
	minDeclarations = []
	maxDeclarations = []
	sizeDeclarations = []

	for line in lines
		if( line =~ /^\s*enum\s*/ and line =~ /\/\/\.\s*autoExtras/ )
				# Name is the last word before any special character (like : or { or \)
				enumName = line.slice( /^[\sa-zA-Z0-9]+/ ).rstrip.slice( /[a-zA-Z0-9]+$/)
		end

		if( enumName != nil )

			part = line;
			part.gsub!( /\/\/.*$/, '' )					# Remove one line comments
			part.gsub!( /\/\*.*\*\//, '' )			# Remove comment blocks

			start = part.index('{')
			stop = part.index('}')

			if( start != nil )
				part = part[start+1..part.length]
				inside = true
			end

			if( stop != nil )
				part = part[0..stop-1]
				inside = false
			end

			if( inside )
				enumBlock += part.chomp.strip
			end

			if( stop != nil )
				params = enumBlock.split( ',')

				params.each { |param| param.gsub!( /\s*=.*$/, '') }

				minDeclarations << "\tconst static #{enumName} ".ljust(30," ") + "#{enumName}_min".ljust(20," ") + "= #{enumName}::#{params.first};"
				maxDeclarations << "\tconst static #{enumName} ".ljust(30," ") + "#{enumName}_max".ljust(20," ") + "= #{enumName}::#{params.last};"
				sizeDeclarations << "\tconst static int ".ljust(30," ") + "#{enumName}_size".ljust(20," ") + "= (int)#{enumName}::#{params.last} + 1;"


				toStringDeclarations << "\tconst char * toString(#{enumName});"

				source << "\tconst char * toString(#{enumName} i)"
				source << "\t{"
				source << "\t\tstatic const char * names[] = { "

				for i in 0..params.length-2
					source << "\t\t\t\"#{params[i]}\","
				end
				source << "\t\t\t\"#{params.last}\" };"

				source << ""
				source << "\t\treturn names[(int)i];"
				source << "\t}"
				source << ""

				enumName = nil
				enumBlock.clear
			end
		end
	end

		header << minDeclarations
		header << ""
		header << maxDeclarations
		header << ""
		header << sizeDeclarations
		header << ""
		header << toStringDeclarations

end



	headerContent = []
	sourceContent = []


	# Scan inputFiles and auto generate content.

	for task in $tasks
		processInputFile( task[:input], headerContent, sourceContent )
		replaceAutoSection( task[:header_out], headerContent )
		replaceAutoSection( task[:source_out], sourceContent )
		headerContent = []
		sourceContent = []
	end


	# Save modified header and source



