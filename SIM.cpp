#include <vector>
#include <queue>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdlib.h>
#include <limits>
using namespace std;


struct Event{
  enum Type{ARRIVE, BOARD, UNBOARD, GROUND };

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
const int boardingTime[11] = {0, 3, 5,	7,	9,	11,	13,	15,	17,	19,	22};

double G;
double A;
double B;
double GAP;
ifstream pRNG;


int eTime (int startFloor, int endFloor) {
  int floorsTraveled = abs(endFloor - startFloor);
  if (floorsTraveled == 0) {
    return 0;
  } else if (floorsTraveled == 1) {
    return 8;
  } else {
    return (2 * 8 + 5 * ( floorsTraveled - 2));
  }
}

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

  employeesWaiting.push_back(e->floor);

  double nextA = generateArrival(e->time, e->floor);

  if( nextA <= GAP*(e->floor-1)+B+A ){
    Event* ne = new Event;
    ne->type = Event::ARRIVE;
    ne->time = nextA;
    ne->floor = e->floor;
    events.push(ne);
  }

  // check elevators, try to generate BOARD;
  for (int i = 0; i < ELEVATORS; i++) {
    Elevator* ele = elevators[i];
    if (ele->currentFloor == 0) {
      // BOARD
      Event* board = new Event;
      board->type = Event::BOARD;
      board->time = e->time; // numeric_limits<double>::min(); // smallest double > 0
      board->elevator = i;
      events.push(board);
      break;
    }
  }

}

void handleBoard(Event* e){
  events.pop();

  int currentEleIndex = e->elevator;
  Elevator* currentEle = elevators[currentEleIndex];

  int currentFloor =  0; currentEle->currentFloor;
  int peopleWaiting = employeesWaiting.size();
  if (peopleWaiting > 0) {
    int peopleBoarding = (ELE_CAP > peopleWaiting? peopleWaiting : ELE_CAP);
    currentEle->numPeople = peopleBoarding;

    // board the first ten to current Elevator
    for (int i = 0; i < peopleBoarding; i++){
      int targetFloor = employeesWaiting[i];
      currentEle->peoplePerFloor[targetFloor] ++;
    }

    // update the waiting list
    employeesWaiting.erase(employeesWaiting.begin(), employeesWaiting.begin() + peopleWaiting);

    int bTime = boardingTime[peopleBoarding];
    // enqueue first UNBOARD
    Event* firstUnboard = new Event;
    firstUnboard->type = Event::UNBOARD;
    
    int firstUnbFloor = 0;
    int unbTime;
    for (firstUnbFloor = currentFloor; firstUnbFloor <= FLOORS; firstUnbFloor++) {
      int pPerFloor = currentEle->peoplePerFloor[firstUnbFloor];
      // cout << "pPerFloor " << pPerFloor << " at floor " << firstUnbFloor << endl;
      if (pPerFloor != 0 ) {
        unbTime = boardingTime[pPerFloor];
        break;
      }
    }
    firstUnboard->elevator = currentEleIndex;
    firstUnboard->time = e->time + bTime + unbTime + eTime(currentFloor, firstUnbFloor);
    firstUnboard->floor = firstUnbFloor;
    events.push(firstUnboard);

    // mark the elevator used
    currentEle->currentFloor = firstUnbFloor;
    cout << "Unboard created at time = " << firstUnboard->time << endl;
  }
}

void handleUnboard(Event* e) {
  events.pop();
  
  int peopleOff = elevators[e->elevator]->peoplePerFloor[e->floor];
  int currentFloor = e->floor;
  
  if(currentFloor !=  elevators[e->elevator]->currentFloor){
    cerr << "WRONG FLOOR!! " << currentFloor << " != " << elevators[e->elevator]->currentFloor << endl;
  }
  
  
  
  elevators[e->elevator]->peoplePerFloor[e->floor] = 0;
  elevators[e->elevator]->numPeople -= peopleOff;
  elevators[e->elevator]->currentFloor = e->floor;
  
  if(elevators[e->elevator]->numPeople > 0){
	//Another Unboard Event
  }
  else{
	//Elevator shoudl go to ground
	Event* ground = new Event;
	ground->type = Event::GROUND;
	ground->elevator = e->elevator;
	ground->time = e->time + eTime(currentFloor, 0);
	ground->floor = 0;
	events.push(ground);
  }
  
  
  
  /*int currentEleIndex = e->elevator;
  Elevator* currentEle = elevators[currentEleIndex];
  int currentFloor = currentEle->currentFloor;

  // handle UNBOARD
  int peopleToGo = currentEle->peoplePerFloor[currentFloor];
  currentEle->peoplePerFloor[currentFloor] = 0;
  currentEle->numPeople -= peopleToGo;

  // add new UNBOARD, if empty add GROUND

  if (currentEle->numPeople == 0) {
    // GROUND
    Event* ground = new Event;
    ground->type = Event::GROUND;
    ground->elevator = currentEleIndex;
    ground->time =  e->time + eTime(currentFloor, 0);
    ground->floor = 0;
    events.push(ground);
  } else {
    // NEXT UNBOARD
    Event* unboard = new Event;
    unboard->type = Event::UNBOARD;
    int firstUnbFloor = 0;
    int unbTime;
    for (int firstUnbFloor = currentFloor; firstUnbFloor <= FLOORS; firstUnbFloor++) {
      int pPerFloor = currentEle->peoplePerFloor[firstUnbFloor];
      if (pPerFloor != 0 ) {
        unbTime = boardingTime[pPerFloor];
        break;
      }
    }
    unboard->elevator = currentEleIndex;
    unboard->time =  e->time + unbTime + eTime(currentFloor, firstUnbFloor);
    unboard->floor = firstUnbFloor;
    events.push(unboard);

    currentEle->currentFloor = firstUnbFloor;
  }*/
}

void handleGround(Event* e) {
  events.pop();

  int currentEleIndex = e->elevator;
  Elevator* currentEle = elevators[currentEleIndex];
  int currentFloor = e->floor;
  currentEle->currentFloor = currentFloor;

  // make new boarding event
  Event* board = new Event;
  board->type = Event::BOARD;
  board->time = e->time;
  board->elevator = currentEleIndex;
  events.push(board);
}

void initializeSim() {
  // generate first ARRIVALs for each floor
  for (int i = 1; i <= FLOORS; i++) {
    Event* firstArrival = new Event;
    firstArrival->type = Event::ARRIVE;
    firstArrival->time = GAP * (i - 1);
    firstArrival->floor = i;

    events.push(firstArrival);
  }



  for (int i = 0; i < ELEVATORS; i++) {
    // initialze elevators
    Elevator* ele = new Elevator;
    ele->index = i;
    ele->currentFloor = 0;
    ele->numPeople = 0;
    ele->peoplePerFloor.resize(FLOORS + 1); // floors indexed from 0
    ele->peoplePerFloor.assign(FLOORS + 1, 0);
    elevators.push_back(ele);
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

    int temp;
    while(!events.empty()){
      Event* currentEvent = events.top();
      switch(currentEvent->type){
        case Event::ARRIVE:
        cout << "Current Event Time " << currentEvent->time << ":  ";
        cout << "arrival" << endl;
        // cin >> temp;
        handleArrival(currentEvent);
        break;
        case Event::BOARD:
        cout << "Current Event Time " << currentEvent->time << ":  ";
        cout << "board" << endl;
        // cin >> temp;
        handleBoard(currentEvent);
        break;
        case Event::UNBOARD:
        cout << "Current Event Time " << currentEvent->time << ":  ";
        cout << "unboard" << endl;
        cin >> temp;
        handleUnboard(currentEvent);
        break;
        case Event::GROUND:
        // cout << "ground" << endl;
        // cin >> temp;
        handleGround(currentEvent);
        break;
        default:
        return -2;
      }
    }
  }


}
