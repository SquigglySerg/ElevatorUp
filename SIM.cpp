#include <vector>
#include <queue>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdlib.h>

using namespace std;


struct Event{
  enum Type{ARRIVE, BOARD, UNBOARD, EMPTY };

  Type type;
  double time;
};

struct EventComparator {
  bool operator()(const Event* e1, const Event* e2) {
     return e1->time >= e2->time;
  }

};

vector<vector<int> > states;
priority_queue<Event*, std::vector<const Event*>, EventComparator> events;
queue<int> employeesWaiting; //Stores the poeple waiting on the ground floor by storing the floor the employee works at.

int FLOORS;
int ELEVATORS;
int DAYS;
const int PEOPLE = 100;
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

void generateArrivals(){
  
  
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
  
  
  
}












