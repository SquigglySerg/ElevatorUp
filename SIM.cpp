#include <vector>
#include <queue>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdlib.h>
#include <limits>
#include <map>

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
  vector<int> peoplePerFloor; // <#people, arr>
  vector < pair<int, double> > peopleInElevator;

};

vector<Elevator*> elevators;
priority_queue<Event*, std::vector<Event*>, EventComparator> events;
vector<pair<int, double> > employeesWaiting; //Stores the poeple waiting on the ground floor by storing the floor the employee works at. <floor#, arrivalTime>

int FLOORS;
int ELEVATORS;
int DAYS;
const int PEOPLE = 100;
const int ELE_CAP = 10;
const double START_TIME = 1.0;
const double boardingTime[11] = {0, 3/60.0, 5/60.0,	7/60.0,	9/60.0,	11/60.0,	13/60.0,	15/60.0,	17/60.0,	19/60.0,	22/60.0};

double G;
double A;
double B;
double GAP;
ifstream pRNG;

vector<int> totalPeoplePerFloor; // Make sure do not generate more than PEOPLE per floor

double eTime (int startFloor, int endFloor) {
  int floorsTraveled = abs(endFloor - startFloor);
  if (floorsTraveled == 0) {
    return 0;
  } else if (floorsTraveled == 1) {
    return 8.0/60.0;
  } else {
    return (2.0 * 8 + 5 * ( floorsTraveled - 2))/60.0;
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
  totalPeoplePerFloor[e->floor]++;

  employeesWaiting.push_back(make_pair(e->floor, e->time));

  double nextA = generateArrival(e->time, e->floor);

  if( nextA <= START_TIME+GAP*(e->floor-1)+B+A && totalPeoplePerFloor[e->floor] < PEOPLE){
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
	  board->floor = 0;
      events.push(board);
      break;
    }
  }

}

void handleBoard(Event* e){
  events.pop();
  
  int currentEleIndex = e->elevator;
  Elevator* currentEle = elevators[currentEleIndex];

  
  int peopleWaiting = employeesWaiting.size();
  if (peopleWaiting > 0 && currentEle->currentFloor == 0) {
	int peopleBoarding; // = (ELE_CAP > peopleWaiting ? peopleWaiting : ELE_CAP);
	if (ELE_CAP > peopleWaiting){
		peopleBoarding = peopleWaiting;
	}
	else{
		peopleBoarding = ELE_CAP;
	}
	int currentFloor = 0;
	currentEle->currentFloor = currentFloor;

    currentEle->numPeople = peopleBoarding;

    // board the first ten to current Elevator
    for (int i = 0; i < peopleBoarding; i++){
      int targetFloor = employeesWaiting[i].first;
      currentEle->peoplePerFloor[targetFloor] ++;
	  currentEle->peopleInElevator.push_back(employeesWaiting[i]);
    }

    // update the waiting list
    employeesWaiting.erase(employeesWaiting.begin(), employeesWaiting.begin() + peopleBoarding);

    double bTime = boardingTime[peopleBoarding];
    // enqueue first UNBOARD
    Event* firstUnboard = new Event;
    firstUnboard->type = Event::UNBOARD;

    int firstUnbFloor = 0;
    double unbTime;
    for (firstUnbFloor = currentFloor; firstUnbFloor <= FLOORS; firstUnbFloor++) {
      int pPerFloor = currentEle->peoplePerFloor[firstUnbFloor];
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
  }
}

void handleUnboard(Event* e, vector<double> &waitTimes) {
  events.pop();

  int peopleOff = elevators[e->elevator]->peoplePerFloor[e->floor];
  int currentFloor = e->floor;

  elevators[e->elevator]->peoplePerFloor[e->floor] = 0;
  elevators[e->elevator]->numPeople -= peopleOff;
  elevators[e->elevator]->currentFloor = e->floor;

  //Calculate wait times
  for (unsigned int i = 0; i < elevators[e->elevator]->peopleInElevator.size(); i++){
	  if (elevators[e->elevator]->peopleInElevator[i].first == currentFloor){
		  waitTimes.push_back( (e->time - elevators[e->elevator]->peopleInElevator[i].second) - (eTime(0,currentFloor) + 2*boardingTime[1] ) ); //Normalize
		  elevators[e->elevator]->peopleInElevator.erase(elevators[e->elevator]->peopleInElevator.begin() + i);
		  i--;
	  }
  }

  if(elevators[e->elevator]->numPeople > 0){
	//Another Unboard Event
	Event* unboard = new Event;
	unboard->type = Event::UNBOARD;
	unboard->elevator = e->elevator;

	//Find next floor
	for (unsigned int i = currentFloor; i < elevators[e->elevator]->peoplePerFloor.size(); i++){
		if (elevators[e->elevator]->peoplePerFloor[i] > 0){
			unboard->floor = i;
			unboard->time = e->time + boardingTime[elevators[e->elevator]->peoplePerFloor[i]] + eTime(currentFloor, i);
		}
	}

	events.push(unboard);

  }
  else{
	//Elevator shoudl go to ground
	Event* ground = new Event;
	ground->type = Event::GROUND;
	ground->elevator = e->elevator;
	ground->time = e->time + eTime(currentFloor, 0);
	ground->floor = e->floor; // store the highest floor traveled
	events.push(ground);
  }
}

void handleGround(Event* e) {
  events.pop();

  //Reinitializing the Elevator -- should not be necessary
  elevators[e->elevator]->numPeople = 0;
  elevators[e->elevator]->currentFloor = 0; //should be 0
  for (unsigned int i = 0; i < elevators[e->elevator]->peoplePerFloor.size(); i++){
	  elevators[e->elevator]->peoplePerFloor[i] = 0;
  }

  // make new boarding event
  Event* board = new Event;
  board->type = Event::BOARD;
  board->time = e->time;
  board->elevator = e->elevator;
  events.push(board);
}

void initializeSim() {
  // generate first ARRIVALs for each floor
  for (int i = 1; i <= FLOORS; i++) {
    Event* firstArrival = new Event;
    firstArrival->type = Event::ARRIVE;
    firstArrival->time = START_TIME + GAP * (i - 1);
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

  totalPeoplePerFloor.resize(FLOORS + 1);
  totalPeoplePerFloor.assign(FLOORS + 1, 0);
}

void print () {

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


  int maxPeepsWaiting = 0;
  int stops = 0;
  int totalPedestrians = 0;
  int traveled = 0;
  vector<double> waitTimes;

  for(int day = 0; day < DAYS; day++){
    //Initialize simulation
    initializeSim();

    while(!events.empty()){
      if(maxPeepsWaiting < employeesWaiting.size())
        maxPeepsWaiting = employeesWaiting.size();

      Event* currentEvent = events.top();
      switch(currentEvent->type){
        case Event::ARRIVE:
        totalPedestrians++;
        handleArrival(currentEvent);
        break;
        case Event::BOARD:
        handleBoard(currentEvent);
        break;
        case Event::UNBOARD:
        stops++;
		handleUnboard(currentEvent, waitTimes);
        break;
        case Event::GROUND:
        stops++;
        traveled += (2* currentEvent->floor);
        handleGround(currentEvent);
        break;
        default:
        return -2;
      }
    }
  }
  cout << "\n*******************RUNNING RESULTS********************\n" << endl;
  cout << totalPedestrians << " total pedestrians" << endl << endl;
  cout << "OUTPUT stops " << (double) stops/DAYS/ELEVATORS << endl << endl;
  cout << "OUTPUT floors "  << (double) traveled/DAYS/ELEVATORS << endl << endl;
  cout << "OUTPUT maxpedq " << maxPeepsWaiting << endl << endl;

  //Calculate Bins
  cout << waitTimes.size() << endl;
  map<int, int> hist;
  for (unsigned int i = 0; i < waitTimes.size(); i++){
	  double delay = waitTimes[i];
	  int bin = floor(delay);
	  
	  try{
		hist.at(bin) = hist[bin] + 1;
	  }
	  catch(const std::out_of_range& m){
	    	if(bin < 0)
			bin = 0;
		hist[bin] = 0;
	  }
  }

  for (std::map<int,int>::iterator it = hist.begin(); it != hist.end(); ++it){
	  cout << it->second << " people waited for about: " << it->first << " mins" << endl;
  }


}

