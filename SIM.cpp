#include <vector>
#include <queue>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdlib.h>

using namespace std;


struct Event{
  enum Type{ARRIVE, BOARD, UNBOARD, EMPTY, GROUND };

  Type type;
  double time;
  int floor;
  int elevator;
};

struct EventComparator {
  bool operator()(const Event* e1, const Event* e2) {
     return e1->time >= e2->time;
  }

};

struct Elevator{
  int index;
  int currentFloor;
  int numPeople;
  vector<int> peoplePerFloor;

};

vector<Elevator*> elevators;
priority_queue<Event*, std::vector<Event*>, EventComparator> events;
vector<int> employeesWaiting; //Stores the poeple waiting on the ground floor by storing the floor the employee works at.

int FLOORS;
int ELEVATORS;
int DAYS;
const int PEOPLE = 100;
const int ELE_CAP = 10;
double G;
double A;
double B;
double GAP;
ifstream pRNG;



double bigLambda(double t, int floor){
  double p = B + ((floor-1)*GAP);
  double m = (2.0/(A+B))*PEOPLE;

  if( t < p-B ){
    return 0;
  }
  else if( p-B <= t && t < p ){
    return ( .5*( m*pow(t+B-p,2)/B ) );
  }
  else if( p <= t && t < p + A ){
    return ( PEOPLE - ( 0.5*( m*pow(p+A-t,2.0)/(double) A ) ) );
  }
  else{
    return PEOPLE;
  }
}

double inverseLambda(double y, int floor){
  double p = B + ((floor-1)*GAP);
  double m = (2.0/(A+B))*PEOPLE;

  if( y <= .5*m*B){
    return sqrt(2.0*y*B/m)+p-B;
  }
  else{
    return p+A-sqrt(2.0*A*(PEOPLE-y)/m);
  }
}

double generateArrival(double previousArrival, int floor){
  double u = bigLambda(previousArrival, floor);
  double randNum;
  pRNG >> randNum;

  u = u + (-1*log(randNum));
  return inverseLambda(u, floor);
}

void handleArrival(Event* e){
  events.pop();

  double nextA = generateArrival(e->time, e->floor);
  if( nextA <= GAP*(e->floor-1)+B+A ){
    Event* ne = new Event;
    ne->type = Event::ARRIVE;
    ne->time = nextA;
    ne->floor = e->floor;

    events.push(ne);
    employeesWaiting.push_back(ne->floor);
  }


}

void handleBoard(Event* e){
  int currentEle = e->elevator;
  int peopleWaiting = employeesWaiting.size();
  if (peopleWaiting > 0) {
      // board the first ten to current Elevator
      int peopleBoarding = (ELE_CAP > peopleWaiting? peopleWaiting : ELE_CAP);
      elevators[currentEle]->numPeople = peopleBoarding;
      for (int i = 0; i < ELE_CAP && i < peopleWaiting; i++) {
          // modify the elevators
          int targetFloor = employeesWaiting[i];
          elevators[currentEle]->peoplePerFloor[targetFloor -1] ++;

      }
  }
}


void initializeSim() {
    Event* firstArrival = new Event;
    firstArrival->type = Event::ARRIVE;
    firstArrival->time = 0;
    firstArrival->floor = 1;

    events.push(firstArrival);
    employeesWaiting.push_back(1);

    // initialze elevators
    for (int i = 0; i < ELEVATORS; i++) {
        Elevator* ele = new Elevator;
        ele->index = i;
        ele->currentFloor = 0;
        ele->numPeople = 0;
        ele->peoplePerFloor.resize(FLOORS);
        ele->peoplePerFloor.assign(FLOORS, 0);
        elevators.push_back(ele);

        Event* firstBoard = new Event;
        firstBoard->type = Event::BOARD;
        firstBoard->time = 0;
        firstBoard->elevator = i;
        events.push(firstBoard);
    }



}

int main(int argc, char* argv[]){
  if(argc != 8){
    cerr << "Invailid number of arguments! Expected 7, got: " << argc-1 << endl;
    return -1;
  }

  FLOORS = atoi(argv[1]);
  ELEVATORS = atoi(argv[2]);
  G = atof(argv[3]);
  B = atof(argv[4]);
  A = atof(argv[5]);
  GAP = G*(A+B);

  pRNG.open(argv[6]);

  if(!pRNG){
    cerr << "ERROR OPENING: " << argv[6] << endl;
  }

  DAYS = atoi(argv[7]);


  for(int day = 0; day < DAYS; day++){
    //Initialize simulation
    initializeSim();


    while(!events.empty()){
      Event* currentEvent = events.top();

      switch(currentEvent->type){
        case Event::ARRIVE:
           handleArrival(currentEvent);
	       break;
        case Event::BOARD:
	       handleBoard(currentEvent);
	       break;
	    default:
	       return -2;
      }
    }
  }


}
