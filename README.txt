Authors		: Jarad Hosking & Cullen Stockmeyer & Richard Fujimoto
Created		: October 21, 2019
Last Modified	: November 11, 2019
Affiliation	: Georgia Institute of Technology



Description
-------------
The CPS Sim program allows someone to run a simulation
on a queueing network which the user configures through
a text file.



Installation
-------------
To install the cpssim program, run
gcc model.c engine.c sim.h -std=c99 -lm -o cpssim



Execution
-------------
Run the cpssim program with

./cpssim endTime config outfile

where
1. endTime is the total number of time units the simulation should
run for.  The units should be the same as those used in the
configuration file
2. config is the filename (such as "config.txt") with the information
necessary to create the queueing network.
3. outfile is the filename (such as "output.txt") with statistics
about the result of the simulation.