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


int customerIDiterator = 0; // Number of customers in the system

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
    struct customer *customerPtr;
};



// Data structure which contains information about a customer in the system and points to the next customer
struct customer {
    int ID;
    double entryTime;
    double exitTime;
    double queueArrivalTime;
    struct customer *Next;
};


// Linked list of customers as a FIFO queue
struct customerQueue {
    struct customer *first;     // pointer to first customer in queue
    struct customer *last;      // pointer to last customer in queue
};

// Data structure which contains information about a queueing station
typedef struct station {
    int ID;
    int isExit;
    double P; // average service time of this queue
    double *probabilities;
    int *destinations;
    int inQueue;
    struct customerQueue line;
    double minWait;
    double maxWait;
    double avgWait;
    int processedCustomers;
} station;



// Stations Array
// Holds pointers to stations
station* stations;


// Customers linked list
// Holds points to all customers in the system
struct customerQueue *customers;

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

// Returns a random number corresponding to the exponential distribution with parameter lambda
double rand_exp(double lambda);

/////////////////////////////////////////////////////////////////////////////////////////////
//
// Model functions internal to this module
//
/////////////////////////////////////////////////////////////////////////////////////////////

void readConfig(char *configFilename) {
    customers = (struct customerQueue *)malloc(sizeof(struct customerQueue));
    FILE *ifp = fopen(configFilename,"r");
    if (ifp==NULL) {
        printf("Error opening input file\n");
        exit(1);
    }
    int numComponents;
    char numComponentsStr[100];
    fscanf(ifp," %s ",numComponentsStr);
    numComponents = strtol(numComponentsStr,NULL,10);
    if (numComponents == 0) {
        printf("Error: the first line of the configuration file should be a positive integer value representing"
               " the number of components in the queueing network.");
        exit(1);
    }
    stations = (station *)malloc(numComponents*sizeof(station));
    for (int i = 0; i < numComponents; i++) {
        int id;
        char type[1];
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
        }
        else {
            printf("Error: One of the component types is invalid.  Component types should be one of G, E, or "
                   "Q, capitalized.");
            exit(1);
        }
    }
}




double rand_exp(double lambda){
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1 - u) * lambda;
}



void createGenerator(double P, int D) {
    double total_time = 0.0;
    double new_arrival;
    while (total_time <= EndTime){
        new_arrival = rand_exp(P);
        total_time += new_arrival;
        printf("%f %f %f\n",new_arrival,total_time,EndTime);
        if (total_time <= EndTime) {
            struct customer *new_customer = (struct customer *)malloc(sizeof(struct customer));
            struct EventData *new_event = (struct EventData *)malloc(sizeof(struct EventData));
            new_customer->entryTime = total_time;
            new_customer->exitTime = -1;
            new_customer->Next = NULL;
            new_customer->ID = customerIDiterator++;
            new_event->EventType = ARRIVAL;
            new_event->componentID = D;
            new_event->customerPtr = new_customer;
            Schedule(total_time, new_event);
            if (customerIDiterator == 1) {
                customers->first = new_customer;
                customers->last = new_customer;
            } else {
                customers->last->Next = new_customer;
                customers->last = new_customer;
            }
        }
    }
}



void createExit(int ID) {
    station* new_station = (station *)malloc(sizeof(station));
    new_station->ID=ID;
    new_station->isExit=1;
    new_station->P=-1;
    new_station->probabilities=NULL;
    new_station->destinations=NULL;
    new_station->inQueue=-1;
    stations[ID] = *new_station;
}


void createStation(int ID, double P, double *probabilities, int *destinations) {
    struct customerQueue *line = (struct customerQueue *)malloc(sizeof(struct customerQueue));
    station* new_station = (station *)malloc(sizeof(station));
    new_station->ID=ID;
    new_station->isExit=0;
    new_station->P=P;
    new_station->probabilities=probabilities;
    new_station->destinations=destinations;
    new_station->inQueue=0;
    new_station->line = *line;
    new_station->maxWait = INFINITY;
    new_station->minWait = 0;
    new_station->avgWait = -1;
    new_station->processedCustomers = 0;
    stations[ID] = *new_station;
}


int randAssign(double *probabilities, int *destinations) {
    double P = rand() / RAND_MAX;
    double curProb = 0;
    int i = 0;
    while (curProb <= 1) {
        curProb += probabilities[i];
        if (P <= curProb) {
            return destinations[i];
        }
        i++;
    }
    fprintf(stderr,"Error in randAssign, probability out of range");
    exit(1);
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
void Arrival (struct EventData *e)
{
    struct EventData *d;
    double ts;
    int componentID = e->componentID;
    struct customer *customerPtr = e->customerPtr;
    station curStation = stations[componentID];

    if (e->EventType != ARRIVAL) {fprintf (stderr, "Unexpected event type\n"); exit(1);}

    if (curStation.isExit == 1) {
        printf ("Processing Arrival event at time %f of customer %d in exit component with ID %d\n",
                CurrentTime(), customerPtr->ID, componentID);
        customerPtr->exitTime = CurrentTime();

    } else if (curStation.isExit == 0) {
        printf ("Processing Arrival event at time %f of customer %d in queue %d which now has %d in line\n",
                CurrentTime(), customerPtr->ID, componentID, ++(curStation.inQueue));

        customerPtr->queueArrivalTime = CurrentTime();

        if (curStation.inQueue == 1) {
            // schedule next departure event
            if((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
            d->EventType = DEPARTURE;
            d->customerPtr = customerPtr;
            d->componentID = componentID;
            double serviceTime = rand_exp(curStation.P);
            ts = CurrentTime() + serviceTime;
            Schedule(ts, d);
            curStation.line.first = customerPtr;
            curStation.line.last = customerPtr;
        } else {
            curStation.line.last->Next = customerPtr;
            curStation.line.last = customerPtr;
        }
    }



}



// event handler for departure events
void Departure (struct EventData *e)
{
    struct EventData *d;
    double ts;
    int componentID = e->componentID;
    struct customer *customerPtr = e->customerPtr;
    station curStation = stations[componentID];


    if (e->EventType != DEPARTURE) {fprintf (stderr, "Unexpected event type\n"); exit(1);}

    printf ("Processing Departure event at time %f of customer %d in queue %d which now has %d in line",
            CurrentTime(), customerPtr->ID, componentID, --(curStation.inQueue));


    // update stats
    double customerQueueTime = CurrentTime() - customerPtr->queueArrivalTime;
    curStation.maxWait = curStation.maxWait > customerQueueTime ? curStation.maxWait : customerQueueTime;
    curStation.minWait = curStation.minWait < customerQueueTime ? curStation.minWait : customerQueueTime;
    curStation.avgWait = ((curStation.avgWait * (double)curStation.processedCustomers++)+customerQueueTime) /
            (double)curStation.processedCustomers;



    // schedule arrival of customer leaving the queue
    if((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
    d->EventType = ARRIVAL;
    d->customerPtr = customerPtr;
    int destinationID = randAssign(curStation.probabilities,curStation.destinations);
    d->componentID = destinationID;
    Schedule(CurrentTime(), d);
    curStation.line.first = curStation.line.first->Next;


    // schedule departure of next customer in queue
    if (curStation.inQueue >= 1) {
        // schedule next departure event
        if((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
        d->EventType = DEPARTURE;
        d->customerPtr = curStation.line.first;
        d->componentID = componentID;
        double serviceTime = rand_exp(curStation.P);
        ts = CurrentTime() + serviceTime;
        Schedule(ts, d);
    }

}



///////////////////////////////////////////////////////////////////////////////////////
//////////// MAIN PROGRAM
///////////////////////////////////////////////////////////////////////////////////////

/*
int main(int argc, char* argv[]) {
    srand(time(0));
    EndTime = strtof(argv[1], NULL);
    char *configFilename = argv[2];
    char *outputFilename = argv[3];
    readConfig(configFilename);
    printf("finished reading\n");
    RunSim(EndTime);
    printf("finished all");
    return(0);
}
 */
int main(int argc, char* argv[]) {
    srand(time(0));
    EndTime = 50;
    char *configFilename = "sampleConfig.txt";
    char *outputFilename = "output.txt";
    readConfig(configFilename);
    RunSim(EndTime);
    return(0);
}