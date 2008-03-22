#!/usr/bin/env ruby

# Small Ruby-script to run Krazy over all directories
#
# (c) 2007, 2008 Carsten Niehaus <cniehaus at kde dot org>
#
# License: GPL V2

# in OpenSUSE 10.x
# export PATH=$PATH:/usr/local/Krazy2/bin

# in Mandriva 2008.1
# export PATH=$PATH:/usr/local/Krazy2/local/bin

def dir2filename( dir )
	dir = dir.gsub( "/", "-" )
	dir
end


directories = [ "libavogadro/src" , "libavogadro/src/tools", "libavogadro/src/engines" , "avogadro/src" , "libavogadro/src/extensions" ]

for dir in directories
	dir.chomp!
	filename = dir2filename( dir )

	command = "krazy2 --export html --explain --exclude qclasses,license,copyright ../#{dir}/*.{cpp,h} > #{filename}.html"

	puts command

	puts "Running Krazy on #{dir}".center(70, "-")
	puts "Results are written in #{filename}.html".center(70, "-")
	`#{command}`
end

