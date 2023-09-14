#ifndef __VEHICLE_BASE_CPP__
#define __VEHICLE_BASE_CPP__

#include "VehicleBase.h"

using namespace::std;

int VehicleBase::vehicleCount = 0;

// common use constructor
VehicleBase::VehicleBase(VehicleType type, Direction direction, Turn turn) : vehicleID(VehicleBase::vehicleCount++), vehicleType(type), vehicleDirection(direction), vehicleTurn(turn)
{
    bool isStopping = false;
}

// copy constructor
VehicleBase::VehicleBase(const VehicleBase& other) : vehicleID(other.vehicleID), vehicleType(other.vehicleType), vehicleDirection(other.vehicleDirection), vehicleTurn(other.vehicleTurn)
{
    
}

// copy assignment operator
VehicleBase& VehicleBase::operator=(const VehicleBase& other)
{
    if (this == &other) 
    {
        return *this;
    }
    vehicleID = other.vehicleID;
    vehicleType = other.vehicleType;
    vehicleDirection = other.vehicleDirection;
    vehicleTurn = other.vehicleTurn;
    return *this;
}

// move constructor
VehicleBase::VehicleBase(VehicleBase&& other)noexcept : vehicleID(other.vehicleID), vehicleType(other.vehicleType), vehicleDirection(other.vehicleDirection), vehicleTurn(other.vehicleTurn)
{
    other.vehicleID = 0;
    other.vehicleType = VehicleType::destructible;
    other.vehicleDirection = Direction::destructible;
    other.vehicleTurn = Turn::destructible;
}

// move assignment operator
VehicleBase& VehicleBase::operator=(VehicleBase&& other)noexcept
{
    if (this == &other) 
    {
        return *this;
    }
    vehicleID = other.vehicleID;
    vehicleType = other.vehicleType;
    vehicleDirection = other.vehicleDirection;
    vehicleTurn = other.vehicleTurn;
    other.vehicleID = 0;
    other.vehicleType = VehicleType::destructible;
    other.vehicleDirection = Direction::destructible;
    other.vehicleTurn = Turn::destructible;
    return *this;
}

VehicleBase::~VehicleBase() 
{
    
}

#endif