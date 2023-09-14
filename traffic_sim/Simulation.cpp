// Authors: Brett Barnes and Jack DuPuy
// Date: 4/18/2023
// Purpose: Create a traffic simulator where cars follow light logic and turn logic without crashing
// into each other, splitting in half, or doing anything else abnormal.

#include <iostream>
#include <vector>
#include <random>
#include <map>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include "VehicleBase.h"
#include "Animator.h"

using namespace::std;

// method prototypes:
vector<VehicleType> generate();
void loadVehicles(vector<VehicleType> newVehicles, vector<VehicleBase*>& v, Direction d);
void readInput(int argc, char* argv[]);
void movePassed(vector<VehicleBase*> &v, int num_sec);
void movePre(vector<VehicleBase*> &v, int num_sec);
void moveThrough(vector<VehicleBase*> &v, vector<VehicleBase*> &r, vector<VehicleBase*> &l, vector<VehicleBase*> &o, int num_sec, int currentTimeLeft);

// instance variables of the class:
// from input file
int maximum_simulated_time;
int number_of_sections_before_intersection;
int green_north_south;
int yellow_north_south;
int green_east_west;
int yellow_east_west;
double prob_new_vehicle_northbound;
double prob_new_vehicle_southbound;
double prob_new_vehicle_eastbound;
double prob_new_vehicle_westbound;
double proportion_of_cars;
double proportion_of_SUVs;
double proportion_right_turn_cars;
double proportion_left_turn_cars;
double proportion_right_turn_SUVs;
double proportion_left_turn_SUVs;
double proportion_right_turn_trucks;
double proportion_left_turn_trucks;
// created by us
int currentNS;
int currentEW;
bool goEW;
int genAmts[4];
char moveOn;

std::mt19937 rng; // creates instance of mt19937 for random number generation
std::uniform_real_distribution<double> rand_double(0.0, 1.0);

int main(int argc, char* argv[])
{
    int initialSeed = atoi(argv[2]); // sets initial seed to the third command line argument
    rng.seed(initialSeed); // sets seed - call rand_double(rng) every time you want to get a new random.
    readInput(argc, argv); // read in the input file & assign instance variables their proper values (per the input file)
    
    Animator animator(number_of_sections_before_intersection); // construct an Animator
    
    // set the initial light colors
    animator.setLightNorthSouth(LightColor::red);
    currentNS = 0; // time left until NS is red
    animator.setLightEastWest(LightColor::green);
    currentEW = green_east_west + yellow_east_west; // time left until EW is red 
    // no distinction for the vehicles between green and yellow

    // initialize vectors of type VehicleBase* for all sections
    vector<VehicleBase*> westbound(number_of_sections_before_intersection * 2 + 2, nullptr);
    vector<VehicleBase*> eastbound(number_of_sections_before_intersection * 2 + 2, nullptr);
    vector<VehicleBase*> southbound(number_of_sections_before_intersection * 2 + 2, nullptr);
    vector<VehicleBase*> northbound(number_of_sections_before_intersection * 2 + 2, nullptr);
    
    goEW = true; // EW light is initially green so goEW is initialized to true

    for(int i = 0; i < maximum_simulated_time; i++)
    {
        // move passed vehicles, including those in the second phase of the intersection (past the point of no return)
        movePassed(northbound, number_of_sections_before_intersection);
        movePassed(southbound, number_of_sections_before_intersection);
        movePassed(eastbound, number_of_sections_before_intersection);
        movePassed(westbound, number_of_sections_before_intersection);

        // move through intersection and turn if appropriate - only go if there's enough time to make it through
        // pass all 4 vehicle vectors to method in order to handle left turns
        if (goEW == true)
        {
            moveThrough(eastbound, southbound, northbound, westbound, number_of_sections_before_intersection, currentEW);
            moveThrough(westbound, northbound, southbound, eastbound, number_of_sections_before_intersection, currentEW);
        }
        else
        {
            moveThrough(northbound, eastbound, westbound, southbound, number_of_sections_before_intersection, currentNS);
            moveThrough(southbound, westbound, eastbound, northbound, number_of_sections_before_intersection, currentNS);
        }
        
        // move pre-intersection vehicles
        movePre(northbound, number_of_sections_before_intersection);
        movePre(southbound, number_of_sections_before_intersection);
        movePre(eastbound, number_of_sections_before_intersection);
        movePre(westbound, number_of_sections_before_intersection);
        
        // decrease currentEW or NS by 1 until it equals 0 and then change lights/direction of traffic flow
        // change green to yellow during that process depending on length of yellow light
        if (goEW == true) // EW is green or yellow
        {
            if (currentEW == 0) // lights are about to change
            {
                animator.setLightEastWest(LightColor::red);
                goEW = 0; // time for north and south to move
                animator.setLightNorthSouth(LightColor::green);
                currentNS = green_north_south + yellow_north_south; // time left until NS is red
            }
            else if (currentEW == yellow_east_west)
            {
                animator.setLightEastWest(LightColor::yellow);
                currentEW--;
            }
            else
                currentEW--;
        }
        else // NS is green or yellow
        {
            if (currentNS == 0) // lights are about to change
            {
                animator.setLightNorthSouth(LightColor::red);
                animator.setLightEastWest(LightColor::green);
                goEW = 1; // time for east and west to move
                currentEW = green_east_west + yellow_east_west; // time left until EW is red
            }
            else if (currentNS == yellow_north_south)
            {
                animator.setLightNorthSouth(LightColor::yellow);
                currentNS--;
            }
            else
                currentNS--;
        }

        // randomly generates which vehicles are to be created (if there is space for them, which is checked in loadVehicles)
        vector<VehicleType> newVehicles = generate(); 

        // checks if there is space for a vehicle in that direction and if appropriate generates a vehicle with type and turn
        // allows for continuous generation for the following parts of a vehicle
        loadVehicles(newVehicles, northbound, Direction::north); 
        loadVehicles(newVehicles, southbound, Direction::south);
        loadVehicles(newVehicles, eastbound, Direction::east);  
        loadVehicles(newVehicles, westbound, Direction::west);

        // place vehicles in animator and draw the intersection
        animator.setVehiclesNorthbound(northbound);
        animator.setVehiclesWestbound(westbound);
        animator.setVehiclesSouthbound(southbound);
        animator.setVehiclesEastbound(eastbound);
        animator.draw(i);

        // move to next tick with each input click
        cin.get(moveOn);
    }    
}

vector<VehicleType> generate()
{
    vector<VehicleType> generatedVehicles;

    // north section:
    if(rand_double(rng) < prob_new_vehicle_northbound) // checks if a car should be generated
    {
        double nRand = rand_double(rng); // generates a new random number to be used for north vehicle type calculations
         if (nRand < proportion_of_cars) // checks if a car should be created
            generatedVehicles.push_back(VehicleType::car); // push it to a vector that will be returned back to the main method
        else if (nRand < proportion_of_SUVs + proportion_of_cars) // checks if a suv should be created
            generatedVehicles.push_back(VehicleType::suv);
        else // if not car or suv, create a truck
            generatedVehicles.push_back(VehicleType::truck);
    }
    else // if no car should be generated, push a 'none' vehicle type (added to the enum class) to maintain proper ordering in the vector
        generatedVehicles.push_back(VehicleType::none);
    
    // south section:
    if(rand_double(rng) < prob_new_vehicle_southbound)
    {
        double sRand = rand_double(rng);
         if (sRand < proportion_of_cars)
            generatedVehicles.push_back(VehicleType::car);
        else if (sRand < proportion_of_SUVs + proportion_of_cars)
            generatedVehicles.push_back(VehicleType::suv);
        else
            generatedVehicles.push_back(VehicleType::truck);
    }
    else
        generatedVehicles.push_back(VehicleType::none);
    
    // east section:
    if(rand_double(rng) < prob_new_vehicle_eastbound)
    {
        double eRand = rand_double(rng);
         if (eRand < proportion_of_cars)
            generatedVehicles.push_back(VehicleType::car);
        else if (eRand < proportion_of_SUVs + proportion_of_cars)
            generatedVehicles.push_back(VehicleType::suv);
        else
            generatedVehicles.push_back(VehicleType::truck);
    }
    else
        generatedVehicles.push_back(VehicleType::none);

    // west section:
    if(rand_double(rng) < prob_new_vehicle_westbound)
    {
        double wRand = rand_double(rng);
         if (wRand < proportion_of_cars)
            generatedVehicles.push_back(VehicleType::car);
        else if (wRand < proportion_of_SUVs + proportion_of_cars)
            generatedVehicles.push_back(VehicleType::suv);
        else
            generatedVehicles.push_back(VehicleType::truck);
    }
    else
        generatedVehicles.push_back(VehicleType::none);
    
    return generatedVehicles; // return vector to main method
}


void loadVehicles(vector<VehicleType> newVehicles, vector<VehicleBase*> &v, Direction d)
{
    int dirInt = static_cast<underlying_type<Direction>::type>(d); // looks at direction to know which number in newVehicles to check?
    // @Brett please add clear docs for what dirInt and genAmts are (ik its related to not generating new vehicles if one is already there but be specific)

    if(v[0] == nullptr)
    {
        if(genAmts[dirInt] != 0)
        {
            v[0] = v[1];
            genAmts[dirInt]--;
        }
        else if(newVehicles[dirInt] == VehicleType::car)
        {
            double turnRand = rand_double(rng);  // generates a random number to determine if vehicle will turn
            // create new vehicle of specified type (car in this case) and turn (depends on turnRand)
            if (turnRand < proportion_right_turn_cars)
                v[0] = new VehicleBase(VehicleType::car, d, Turn::right);
            else if(turnRand < proportion_right_turn_cars + proportion_left_turn_cars)
                v[0] = new VehicleBase(VehicleType::car, d, Turn::left);
            else
                v[0] = new VehicleBase(VehicleType::car, d, Turn::straight);
            genAmts[dirInt] = 1; // how many sections are left in the generated car/suv/truck
        }
        // repeat for suvs and trucks, set genAmts to the correct number
        else if(newVehicles[dirInt] == VehicleType::suv)
        {
            double turnRand = rand_double(rng); 
            if (turnRand < proportion_right_turn_SUVs)
                v[0] = new VehicleBase(VehicleType::suv, d, Turn::right);
            else if(turnRand < proportion_right_turn_SUVs + proportion_left_turn_SUVs)
                v[0] = new VehicleBase(VehicleType::suv, d, Turn::left);
            else
                v[0] = new VehicleBase(VehicleType::suv, d, Turn::straight);
            genAmts[dirInt] = 2;
        }
        else if(newVehicles[dirInt] == VehicleType::truck)
        {
            double turnRand = rand_double(rng);
            if (turnRand < proportion_right_turn_trucks)
                v[0] = new VehicleBase(VehicleType::truck, d, Turn::right);
            else if(turnRand < proportion_right_turn_trucks + proportion_left_turn_trucks)
                v[0] = new VehicleBase(VehicleType::truck, d, Turn::left);
            else
                v[0] = new VehicleBase(VehicleType::truck, d, Turn::straight);
            genAmts[dirInt] = 3;
        }
    }

}

void movePassed(vector<VehicleBase*> &v, int num_sec)
{
    int length = num_sec * 2 + 2;
    v[length-1] = nullptr; // remove vehicle sections from the end

    // move each vehicle one section forward from back to front to avoid overwriting sections
    for(int i = length-2; i >= num_sec + 1; i--)
    {
        v[i+1] = v[i];
        v[i] = nullptr;
    }
}

void movePre(vector<VehicleBase*> &v, int num_sec)
{
    // move vehicle sections forward if there's no vehicle in front of it
    // moved forward from back to front to avoid overwriting sections
    for(int i = num_sec - 2; i >= 0; i--)
    {
        if(v[i+1] == nullptr)
        {
            v[i+1]=v[i];
            v[i] = nullptr;
        }
    }
}

void moveThrough(vector<VehicleBase*> &v, vector<VehicleBase*> &r, vector<VehicleBase*> &l, vector<VehicleBase*> &o, int num_sec, int currentTimeLeft)
{
    // handle vehicles in 1st section of intersection (where they will either turn straight, right, or left)
    if(v[num_sec] != nullptr)
    {
        // send vehicle forward if it's going straight
        if(v[num_sec]->getVehicleTurn() == Turn::straight)
        {
            v[num_sec+1]=v[num_sec];
            v[num_sec] = nullptr;
        }
        // send vehicle to the right if it's going right
        else if (v[num_sec]->getVehicleTurn() == Turn::right)
        {
            r[num_sec+2]=v[num_sec];
            v[num_sec] = nullptr;
        } 
        // send vehicle to the left if it's going left
        else
        {
            l[num_sec+1] = v[num_sec];
            v[num_sec] = nullptr;
        }       
    }
    
    // handle vehicles in section right before intersection
    // determine if they can go based on how much time is left before their light turns red
    // for left turns, also consider what happens when both directions want to turn left
    if(v[num_sec-1] != nullptr)
    {
        int lengthLeft = 0; // number of sections until vehicle is fully into the intersection
        int i = num_sec-2;
        while(v[i]==v[num_sec-1]) // determine how much of the vehicle is left before the intersection
        {
            lengthLeft++;
            i--;
        }
        // determine if vehicle can make it through before light turns red and move accordingly
        // takes one less tick to get through right turn so right turn uses counter + 1 instead of + 2
        if(v[num_sec-1]->getVehicleTurn() == Turn::straight && lengthLeft + 2 <= currentTimeLeft)
        {
            v[num_sec]=v[num_sec-1];
            v[num_sec-1] = nullptr;
        }
        else if(v[num_sec-1]->getVehicleTurn() == Turn::right && lengthLeft + 1 <= currentTimeLeft)
        {
            v[num_sec]=v[num_sec-1];
            v[num_sec-1] = nullptr;
        }
        else if(v[num_sec-1]->getVehicleTurn() == Turn::left && lengthLeft + 2 <= currentTimeLeft)
        {
            // find closest oncoming vehicle and determine if a collision will happen
            bool collision = false;
            for(int j = num_sec + 1; j > num_sec - lengthLeft - 3; j--)
            {
                if(j >= 0)
                {
                    if(o[j] != nullptr)
                    {
                        int oppLengthLeft = 0;
                        int k = j-1;
                        while(v[k]==v[j]) // determine how much of the oncoming vehicle is left before the intersection
                        {
                            oppLengthLeft++;
                            k--;
                        }

                        // determine how much time the oncoming vehicle will take to go through the intersection
                        int oppTimeUntilThrough;
                        if(o[j]->getVehicleTurn() == Turn::right)
                            oppTimeUntilThrough = oppLengthLeft + 1 + (num_sec - j - 1);
                        else
                            oppTimeUntilThrough = oppLengthLeft + 2 + (num_sec - j - 1);

                        // if oncoming vehicle turning left, give northbound and eastbound vehicles priority
                        // if oncoming vehicle is also turning left and my vehicle is north or east, ignore it and go
                        if(o[j]->getVehicleTurn() == Turn::left &&
                        (v[num_sec-1]->getVehicleOriginalDirection() == Direction::north ||
                        v[num_sec-1]->getVehicleOriginalDirection() == Direction::east))    
                        {
                            j = -1;
                        }
                        // if oncoming vehicle does not have enough time to get through light, ignore it and go
                        else if(oppTimeUntilThrough > currentTimeLeft)           
                        { 
                            j = -1;
                        }
                        // oncoming vehicle will be going through the intersection, so stop
                        else
                        {
                            collision = true;
                            j = -1;
                        }    
                    }
                } 
            }
            // if there won't be a collision, move forward into intersection (actual left turn handled on next tick)
            if(!collision)
            {
                // check to make sure next space is not already occupied before moving
                if(r[num_sec+1] == nullptr)
                {
                    v[num_sec]=v[num_sec-1]; 
                    v[num_sec-1] = nullptr;
                }
            }

        }
        
    }
}

void readInput(int argc, char* argv[])
{
    // checks for the correct number of CLA's and prints a useful error message if that number is incorrect
    if (argc != 3)
    {
        cerr << "Incorrect number of command line arguments. Please enter " << argv[0] 
        << " and then your input file and then your initial seed for a total of 3 command line arguments." << endl;
        exit(0);
    }
    
    // ensures the input file opens correctly
    ifstream infile {argv[1]};
    if (!infile)
    {
        cerr << "Unable to open file: " << argv[1] << endl;
        exit(0);
    }

    // create input dictionary to store input variables
    map<string, double> input_dict;
    string line;
    string input_spec;
    double input_value;

    // read input line by line regardless of order and whitespace
    bool colonFound = false;
    int i = 0;
    while (getline(infile, line))
    {
        // remove spaces from line, skip blank lines (length == 0)
        line.erase(std::remove_if(line.begin(), line.end(), [](char c) { return std::isspace(c); }), line.end());
        if (line.length() != 0)
        {
            while(!colonFound)
            {
                // iterate through the line until colon is found, then set input_spec equal to all non-ws characters before colon and 
                // input_value equal to all non-ws characters after colon and then add key-value pair to the input dictionary
                if (line[i] == ':')
                {
                    colonFound = true;
                    input_spec = line.substr(0, i);
                    input_value = stod(line.substr(i+1,line.length() - 1));
                    input_dict[input_spec] = input_value;
                }
                i++;
            }
            colonFound = false;
            i = 0;
        }
    }

    // add values from input dictionary to the instance variables created at the beginning
    maximum_simulated_time = input_dict["maximum_simulated_time"];
    number_of_sections_before_intersection = input_dict["number_of_sections_before_intersection"];
    green_north_south = input_dict["green_north_south"];
    yellow_north_south = input_dict["yellow_north_south"];
    green_east_west = input_dict["green_east_west"];
    yellow_east_west = input_dict["yellow_east_west"];
    prob_new_vehicle_northbound = input_dict["prob_new_vehicle_northbound"];
    prob_new_vehicle_southbound = input_dict["prob_new_vehicle_southbound"];
    prob_new_vehicle_eastbound = input_dict["prob_new_vehicle_eastbound"];
    prob_new_vehicle_westbound = input_dict["prob_new_vehicle_westbound"];
    proportion_of_cars = input_dict["proportion_of_cars"];
    proportion_of_SUVs = input_dict["proportion_of_SUVs"];
    proportion_right_turn_cars = input_dict["proportion_right_turn_cars"];
    proportion_left_turn_cars = input_dict["proportion_left_turn_cars"];
    proportion_right_turn_SUVs = input_dict["proportion_right_turn_SUVs"];
    proportion_left_turn_SUVs = input_dict["proportion_left_turn_SUVs"];
    proportion_right_turn_trucks = input_dict["proportion_right_turn_trucks"];
    proportion_left_turn_trucks = input_dict["proportion_left_turn_trucks"];

    infile.close(); // close input file
}