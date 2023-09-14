#ifndef __VEHICLE_BASE_H__
#define __VEHICLE_BASE_H__

// enum: see http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-enum
enum class Direction   {north, south, east, west, destructible};
enum class VehicleType {car, suv, truck, none, destructible};
enum class LightColor  {green, yellow, red, destructible};
enum class Turn        {left, right, straight, destructible};

class VehicleBase
{
   public:
      static int vehicleCount;

   private:
      int         vehicleID;    
      bool isStopping;
      VehicleType vehicleType;
      Direction   vehicleDirection;
      Turn        vehicleTurn;

   public:
      VehicleBase(VehicleType type, Direction originalDirection, Turn turn);
      VehicleBase(const VehicleBase& other);
      VehicleBase& operator=(const VehicleBase& other);
      VehicleBase(VehicleBase&& other)noexcept;
      VehicleBase& operator=(VehicleBase&& other)noexcept;
      ~VehicleBase();

      inline int getVehicleID() const { return this->vehicleID; }
      inline bool        getVehicleStop() const {return this->isStopping; }
      inline void        setVehicleStop(bool s) {this->isStopping = s; }
      inline VehicleType getVehicleType() const { return this->vehicleType; }
      inline Direction   getVehicleOriginalDirection() const { return this->vehicleDirection; }
      inline Turn        getVehicleTurn() const {return this->vehicleTurn; }
      
};

#endif
