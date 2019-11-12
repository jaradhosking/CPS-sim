//
//  Sample Discrete Event Simulation
//
//  Created by Richard Fujimoto on 9/24/17.
//  Copyright © 2017 Richard Fujimoto. All rights reserved.
//  Edits by Jarad Hosking & Cullen Stockmeyer
//  Last Modified on 11/11/2019.
//

#ifndef SAMPLESIMULATION_SIM_H
#define SAMPLESIMULATION_SIM_H

//
// Application Independent Simulation Engine Interface
//



//
// Functions defined in the simulation engine called by the simulation application
//

// Call this procedure to run the simulation indicating time to end simulation
void RunSim (double EndTime);

// Schedule an event with timestamp ts, event parameters *data
void Schedule (double ts, void *data);

// This function returns the current simulation time
double CurrentTime (void);



//
// Functions defined in the simulation application called by the simulation engine
//
//  Event handler function: called to process an event
void EventHandler (void *data);


#endif //SAMPLESIMULATION_SIM_H
