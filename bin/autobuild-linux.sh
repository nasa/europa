#!/bin/bash

##Author: Mark Roberts
##runs all possible builds and places output into files based on the build name.
## TODO PROFILE, SHARED is not actually in the official autobuild

libraries=(  \
    SHARED \
    STATIC
)

variants=(  \
    DEV \
    OPTIMIZED \
    PROFILE
)

for variant in ${variants[@]}
do
    for library in ${libraries[@]}
    do
	echo This will build  $variant $library
    done
done

for variant in ${variants[@]}
do
    for library in ${libraries[@]}
    do
	$ANT_HOME/bin/ant -Djam.variant=$variant -Djam.libraries=$library clean
    done
done



for variant in ${variants[@]}
do
    for library in ${libraries[@]}
    do
	echo Working on  $variant $library
	rm autobuild.$variant.$library.*
	touch autobuild.$variant.$library.start
	touch autobuild.$variant.$library.log
	xterm -title "Working on $variant.$library" -geometry 144x24 -e tail -f autobuild.$variant.$library.log &
	$ANT_HOME/bin/ant -Djam.variant=$variant -Djam.libraries=$library autobuild > autobuild.$variant.$library.log 2>&1
	touch autobuild.$variant.$library.end
    done
done
 
