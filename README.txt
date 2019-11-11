Authors		: Jarad Hosking & Cullen Stockmeyer & Richard Fujimoto
Created		: October 21, 2019
Last Modified	: November 11, 2019

Affiliation	: Georgia Institute of Technology


Description
-------------
The graphgen program constructs a scale free graph and
outputs topology information (size, edges, degrees) into
a file.  Can also create a node-degree histogram, based
on parameters.
The analysis program finds the diameter of a scale free
network using dijkstra’s shortest path algorithm over all
shortest paths.



Installation
-------------
To install the cpssim program, run
gcc model.c engine.c sim.h -std=c99 -lm -o cpssim





Execution
-------------
Run the graphgen program with

./cpssim endTime config outfile

where
1. endTime is the total number of time units the simulation should
run for.  The units should be the same as those used in the
configuration file
2. config is the filename (such as "config.txt") with


Run the analysis program with

./analysis fInName -o output

where fInName is the file that holds the topology
of the network to be analyzed, and "output" is where
information regarding the diameter of the network and
minimum distances is output.  This parameter is optional
and if used should be called with the -o flag.  If not included,
this information is just printed to the screen and not
output into a file.