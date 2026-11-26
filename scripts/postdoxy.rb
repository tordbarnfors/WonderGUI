#!/usr/bin/ruby

begin 
    require 'nokogiri'
rescue LoadError
    puts "ERROR: Ruby module nokogiri not found!!!"
    puts "postdoxy.rb will fail and documentation will not be correctly formatted."
    gets
end

$prioTable = {
  'Creation' => 10,
  'Parts' => 20,
  'Properties' => 30,
  'Identification' => 35,
  'Appearance' => 40,
  'Content' => 50,
  'Ordering' => 55,
  'Control' => 60,
  'Behavior' => 70,
  'State' => 80,
  'Rendering' => 90,
  'Geometry' => 100,
  'Hierarchy' => 110,
  'Debug' => 120,
  'Misc' => 140,
  'Operators' => 150,
  'Definitions' => 160,
  'Internal' => 170,
  'Deprecated' => 180,
}

$unknownHeaders = Hash.new(0)


def memberTableHeader( memberTableNode )
  return memberTableNode.css('tr')[0].css('td h2').text.gsub( /\n/, '' )
end

def memberTablePrio( memberTableNode )

	header = memberTableHeader(memberTableNode)

	prio = $prioTable[header]
	if( prio == nil )
		prio = 1000
		$unknownHeaders[header] = $unknownHeaders[header]+1
	end

	return prio
end

def mergeMemberTables( tables )
  for index in 0..tables.size-1
	for prev in 0..index-1
	  if( memberTableHeader( tables[prev]) == memberTableHeader( tables[index]) )
#        puts 'Found duplicate ' + memberTableHeader( memberTables[prev] )

		rows = tables[index].css('tr')
		for row in 1..rows.size-1
		  rows[row].parent = tables[prev]
		end
		tables[index].remove
	  end
	end
  end
end

def sortMembers( table )

#  rows = tables[index].css('tr')
#  rows.sort_by! { |row| row.}

end

def sortMemberTables( tables )

	sorted_tables = tables.sort_by { |node| memberTablePrio( node ) }


	for i in 0..sorted_tables.length-2
  		sorted_tables[i].add_next_sibling(sorted_tables[i+1])
	end

#  rows = tables[index].css('tr')
#  rows.sort_by! { |row| row.}

end


for fileName in $*
  puts "Processing: " + fileName

  file = File.new(fileName)
  doc = Nokogiri::HTML(file)

  memberTables = doc.css("table[class=memberdecls]")
  mergeMemberTables( memberTables )

  memberTables = doc.css("table[class=memberdecls]")
  sortMemberTables( memberTables );

  memberTables.each { |table| sortMembers(table) }

  fi = File.new( fileName, "w")
  fi.puts doc
  fi.close

end

$unknownHeaders.each { |key, value| puts "Warning: Unknow Section '#{key}' was found #{value} times." }
