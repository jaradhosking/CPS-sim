#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "sim.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//
// One Pump Gas Station Simulation
// Arriving vehicles use the gas pump; queue if pump being used
//
/////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
//
// State variables  and other global information
//
/////////////////////////////////////////////////////////////////////////////////////////////


double EndTime = -1;
int numQueues = 0;

// Event types
#define	ARRIVAL     1
#define	DEPARTURE   2






/////////////////////////////////////////////////////////////////////////////////////////////
//
// Data structures for event data
//
/////////////////////////////////////////////////////////////////////////////////////////////
//
// Each event can have any number of parameters, each of arbitrary data type. The number and types are
// dependent on the simulation application. This information is defined here. This information is
// stored in each event that is scheduled. Note that the simulation engine does not need to know
// the number and type of the parameters, so this information is hidden from the engine. The simulation
// engine is only given a pointer to event parameters.
// For this simple application, events only have one parameter, indicating the kind of event
// (ARRIVAL or DEPARTURE)

struct EventData {
    int EventType;
    int componentID;
    int customerID;
};


// Data structure which contains information about a queueing station
typedef struct station {
    int ID;
    double P;
    double *probabilities;
    double *destinations;
} station;


// Data structure which contains information about a customer in the system and points to the next customer
struct customer {
    int ID;
    double *times;
    struct customer *Next;
};


// Data structure for a node in a linked list
struct node {
    int ID;
    struct node *Next;
};


// Exit Points List
// Holds the int IDs of any exit points
struct node exits = {-1, NULL};


// In Queue List
// Holds the number of people in each queue
int *inQueue;

/////////////////////////////////////////////////////////////////////////////////////////////
//
// Function prototypes
//
/////////////////////////////////////////////////////////////////////////////////////////////

// prototypes for event handlers
void Arrival (struct EventData *e);		// arrival event
void Departure (struct EventData *e);	// departure event




// This function initializes the queue via information provided in the configuration file configFilename
void readConfig(char *configFilename);

// Creates a generator and produces new event arrival times from the created generator
// P is the average interarrival time, D is the id of the component where generated items go,
void createGenerator(double P, int D);

// Creates an exit point
void createExit(int ID);

// Creates a queueing station with ID ID and average queueing time P, it sends customers
// to destinations with given probabilities.
void createStation(int ID, double P, double *probabilities, int *destinations);

// This function writes to outputFilename the results of the simulation
void writeResults(char *outputFilename);


/////////////////////////////////////////////////////////////////////////////////////////////
//
// Model functions internal to this module
//
/////////////////////////////////////////////////////////////////////////////////////////////

void readConfig(char *configFilename) {
    FILE *ifp = fopen(configFilename,"r");
    if (ifp==NULL) {
        printf("Error opening input file\n");
        exit(1);
    }
    int numComponents;
    char *numComponentsStr = NULL;
    fscanf(ifp,"%s",numComponentsStr);
    numComponents = strtol(numComponentsStr,NULL,10);
    if (numComponents == 0) {
        printf("Error: the first line of the configuration file should be a positive integer value representing"
               " the number of components in the queueing network.");
        exit(1);
    }
    for (int i = 0; i < numComponents; i++) {
        int id;
        char *type = NULL;
        fscanf(ifp,"%d %s",&id,type);
        if (strcmp(type,"G") == 0) {
            double avgInterarrivalTime;
            int destination;
            fscanf(ifp,"%lf %d",&avgInterarrivalTime,&destination);
            createGenerator(avgInterarrivalTime,destination);
        }
        else if (strcmp(type,"E") == 0) {
            createExit(id);
        }
        else if (strcmp(type,"Q") == 0) {
            double avgServiceTime;
            int numRoutes;
            fscanf(ifp,"%lf %d",&avgServiceTime,&numRoutes);
            int *destinations = (int *)malloc(numRoutes*sizeof(int));
            double *probs = (double *)malloc(numRoutes*sizeof(double));
            for (int j = 0; j < numRoutes; j++) {
                fscanf(ifp,"%lf %d",&probs[j],&destinations[j]);
            }
            createStation(id,avgServiceTime,probs,destinations);
            numQueues += 1;
        }
        else {
            printf("Error: One of the component types is invalid.  Component types should be one of G, E, or "
                   "Q, capitalized.");
            exit(1);
        }
    }
    int *inQueue = (int *)malloc(numQueues*sizeof(int));
}



/////////////////////////////////////////////////////////////////////////////////////////////
//
// Event Handlers
// Parameter is a pointer to the data portion of the event
//
/////////////////////////////////////////////////////////////////////////////////////////////
//
// Note the strategy used for dynamic memory allocation. The simulation application is responsible
// for freeing any memory it allocates, and is ONLY responsible for freeing memory allocated
// within the simulation application (i.e., it is NOT responsioble for freeing memory allocated
// within the simulation engine). Here, the simulation dynamically allocates memory
// for the PARAMETERS of each event. Whenever a new event is scheduled, memory for the event's paramters
// is allocated using malloc. This storage is released within the event handler when it is done
// processing the event.
// Because we know each event is scheduled exactly once, and is processed exactly once, we know that
// memory dynamically allocated for each event'a parameters will be released exactly once.
// Note that within the simulation engine memory is also dynamically allocated and released. That memory
// is different from the memory allocated here, and the simulation application is not concerned with
// memory allocated in the simulation engine.
//

// General Event Handler Procedure define in simulation engine interface
// This function is called by the simulation engine to process an event removed from the future event list
void EventHandler (void *data)
{
    struct EventData *d;

    // coerce type so the compiler knows the type of information pointed to by the parameter data.
    d = (struct EventData *) data;
    // call an event handler based on the type of event
    if (d->EventType == ARRIVAL) Arrival (d);
    else if (d->EventType == DEPARTURE) Departure (d);
    else {fprintf (stderr, "Illegal event found\n"); exit(1); }
    free (d); // Release memory for event paramters
}


// event handler for arrival events




///////////////////////////////////////////////////////////////////////////////////////
//////////// MAIN PROGRAM
///////////////////////////////////////////////////////////////////////////////////////

int main (void)
{
    struct EventData *d;
    double ts;

    // initialize event list with first arrival
    if ((d=malloc (sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
    ts = 10.0;
    Schedule (ts, d);

    RunSim(50.0);
}
