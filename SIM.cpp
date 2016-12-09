#include <vector>
#include <queue>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdlib.h>
#include <limits>
#include <algorithm>
#include <map>
#include <iomanip>

using namespace std;

const int PEOPLE = 100;
const int ELE_CAP = 10;
const double START_TIME = 1.0;
const double boardingTime[11] = {0, 3/60.0, 5/60.0,	7/60.0,	9/60.0,	11/60.0,	13/60.0,	15/60.0,	17/60.0,	19/60.0,	22/60.0};
int FLOORS;
int ELEVATORS;
int DAYS;

double G;
double A;
double B;
double GAP;
ifstream pRNG;

struct Event{
  enum Type{ARRIVE, BOARD, UNBOARD, GROUND };

  Type type;
  double time;
  int floor;
  int stopNum; // To store how many stops before go back to ground;
  int elevator;
};

struct EventComparator {
  bool operator()(const Event* e1, const Event* e2) {
    return e1->time >= e2->time;
  }
};

priority_queue<Event*, std::vector<Event*>, EventComparator> events;

vector<int> totalPeoplePerFloor; // Make sure do not generate more than PEOPLE per floor

vector<pair<int, double> > employeesWaiting; //Stores the poeple waiting on the ground floor by storing the floor the employee works at. <floor#, arrivalTime>

struct Elevator{
  int index;
  int currentFloor;
  int numPeople;
  // map<int, int> peoplePerFloor; // <floorIndex, peoplePerFloor>
  // map<int, int>::key_compare mycomp = mymap.key_comp();
  vector<pair<int, int> > peoplePerFloor; // <floorIndex, peoplePerFloor>
};

vector<Elevator*> elevators;


vector<double> waitTimes;

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
      board->time = e->time;
      board->elevator = i;
      board->floor = 0;
      events.push(board);
      break;
    }
  }

}

bool myComparison(const pair<int,int> &a,const pair<int,int> &b){
  return a.first < b.first;
}



void handleBoard(Event* e){
  events.pop();

  int currentEleIndex = e->elevator;
  Elevator* currentEle = elevators[currentEleIndex];

  // int currentFloor =  0;
  // currentEle->currentFloor = currentFloor;
  if (currentEle->currentFloor != 0) {
    cout << "WROOOONG" << endl;
  }

  int peopleWaiting = employeesWaiting.size();

  if (peopleWaiting > 0) {
    int peopleBoarding; // = (ELE_CAP > peopleWaiting ? peopleWaiting : ELE_CAP);
    if (ELE_CAP > peopleWaiting){
      peopleBoarding = peopleWaiting;
    }
    else{
      peopleBoarding = ELE_CAP;
    }

    // update people will board
    currentEle->numPeople = peopleBoarding;
    // mark the elevator used
    currentEle->currentFloor = FLOORS;
    // clean
    currentEle->peoplePerFloor.clear();

    // board the first ten (if not all) to current Elevator
    for (int i = 0; i < peopleBoarding; i++){
      int targetFloor = employeesWaiting[i].first;

      bool insertedBefore = false;
      for (unsigned int i = 0; i < currentEle->peoplePerFloor.size(); i++) {
        if (currentEle->peoplePerFloor[i].first == targetFloor) {
          currentEle->peoplePerFloor[i].second++;
          insertedBefore = true;
        }
      }
      if (!insertedBefore) {
        currentEle->peoplePerFloor.push_back(make_pair(targetFloor, 1));
      }

    }


    sort(currentEle->peoplePerFloor.begin(), currentEle->peoplePerFloor.end(),myComparison);


    int stopNum = currentEle->peoplePerFloor.size();
    vector<double> unboardTimes;
    unboardTimes.resize(stopNum);


    unboardTimes[0] = e->time + boardingTime[peopleBoarding] + eTime(0, currentEle->peoplePerFloor[0].first) + boardingTime[currentEle->peoplePerFloor[0].second];
    for (int i = 1; i < stopNum; i++) {
      unboardTimes[i] = unboardTimes[i-1] + eTime(currentEle->peoplePerFloor[i-1].first, currentEle->peoplePerFloor[i].first) + boardingTime[currentEle->peoplePerFloor[i].second];
    }

    // calculate wait times
    for (int i = 0; i < peopleBoarding; i++) {
      int targetFloor = employeesWaiting[i].first;

      unsigned index = 0;
      for (; index < currentEle->peoplePerFloor.size(); index++) {
        if (currentEle->peoplePerFloor[index].first == targetFloor) {
          break;
        }
      }
      double wTime = unboardTimes[index] - employeesWaiting[i].second - (boardingTime[1] + boardingTime[1] + eTime(0, employeesWaiting[i].first));
      waitTimes.push_back(wTime);
    }

    // update the waiting list
    employeesWaiting.erase(employeesWaiting.begin(), employeesWaiting.begin() + peopleBoarding);

    //Elevator should go to ground
    Event* ground = new Event;
    ground->type = Event::GROUND;
    ground->elevator = e->elevator;
    ground->time = unboardTimes[stopNum-1] + eTime(currentEle->peoplePerFloor[stopNum-1].first, 0);
    ground->stopNum = stopNum + 1;
    ground->floor = currentEle->peoplePerFloor[stopNum-1].first; // store the highest floor traveled
    events.push(ground);
  }
}

void handleGround(Event* e) {
  events.pop();

  //Reinitializing the Elevator -- should not be necessary
  elevators[e->elevator]->numPeople = 0;
  elevators[e->elevator]->currentFloor = 0; //should be 0
  // for (unsigned int i = 0; i < elevators[e->elevator]->peoplePerFloor.size(); i++){
  //   elevators[e->elevator]->peoplePerFloor[i] = 0;
  // }
  elevators[e->elevator]->peoplePerFloor.clear();

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
    ele->peoplePerFloor.clear();
    // ele->peoplePerFloor.resize(FLOORS + 1); // floors indexed from 0
    // ele->peoplePerFloor.assign(FLOORS + 1, 0);
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


  unsigned int maxPeepsWaiting = 0;
  int stops = 0;
  int totalPedestrians = 0;
  int traveled = 0;
  for(int day = 0; day < DAYS; day++){
    //Initialize simulation
    initializeSim();

    // cout << "Simulation finishes" << endl << endl;
    // int temp;
    while(!events.empty()){
      //cout << "Number of people waiting: " << employeesWaiting.size() << endl;
      if(maxPeepsWaiting < employeesWaiting.size())
      maxPeepsWaiting = employeesWaiting.size();

      Event* currentEvent = events.top();
      switch(currentEvent->type){
        case Event::ARRIVE:
        totalPedestrians++;
        //cout << "Current Event Time " << currentEvent->time << ":  ";
        //cout << "arrival" << endl;
        // cin >> temp;
        handleArrival(currentEvent);
        break;

        case Event::BOARD:
        //cout << "Current Event Time " << currentEvent->time << ":  ";
        //cout << "board" << endl;
        // cin >> temp;
        handleBoard(currentEvent);
        break;

        case Event::GROUND:
        stops += currentEvent->stopNum;
        traveled += (2* currentEvent->floor);
        //cout << "Current Event Time " << currentEvent->time << ":  ";
        //cout << "ground" << endl;
        // cin >> temp;
        handleGround(currentEvent);
        break;

        default:
        return -2;
      }

      //cout << "Number of people waiting now: " << employeesWaiting.size() << endl << endl;
    }
  }
  cout << fixed << setprecision(5);
  cout << "*******************RUNNING RESULTS********************" << endl;
  // cout << totalPedestrians << " total pedestrians" << endl;
  cout << "OUTPUT stops " << (double) stops/DAYS/ELEVATORS << endl;
  cout << "OUTPUT floors "  << (double) traveled/DAYS/ELEVATORS << endl;
  cout << "OUTPUT maxpedq " << maxPeepsWaiting << endl;

  // cout << "size of waitTimes " << waitTimes.size() << endl;

  map<int, int> bins;

  for (unsigned int i = 0; i < waitTimes.size(); i++) {
    int bin = floor(waitTimes[i]);
    if (abs(waitTimes[i]) < 0.000001 ) {
      bin = 0;
    }
    pair<map<int,int>::iterator, bool> ret;
    ret = bins.insert(pair<int, int>(bin, 1));
    if (ret.second == false) {
      ret.first->second++;
    }
  }

  // showing contents:
  cout << "our histogram:\n";
  map<int,int>::iterator it = bins.begin();
  for (it=bins.begin(); it!=bins.end(); ++it) {
    // cout << it->first << " => " << it->second << '\n';
    cout << "OUTPUT /" << it->first << "/ minutes "  << (double)it->second/totalPedestrians << endl;

  }
  cout << endl << endl;
}
