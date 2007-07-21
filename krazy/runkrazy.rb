#!/usr/bin/env ruby

# Small Ruby-script to run Krazy over all directories
#
# (c) 2007 Carsten Niehaus <cniehaus at kde dot org>
#
# License: GPL V2

#you need to run a command like this:
# export PATH=$PATH:/usr/local/Krazy/bin

def dir2filename( dir )
	dir = dir.gsub( "/", "-" )
	dir
end


directories = [ "libavogadro" , "libavogadro/src" , "libavogadro/src/tools", "libavogadro/src/engines" , "avogadro/src" , "avogadro/src/extensions" ]

for dir in directories
	dir.chomp!
	filename = dir2filename( dir )

	command = "krazy --export html --explain --exclude qclasses ../#{dir}/* > #{filename}.html"

	puts "Running Krazy on #{dir}".center(70, "-")
	puts "Results are written in #{filename}.html".center(70, "-")
	`#{command}`
end

