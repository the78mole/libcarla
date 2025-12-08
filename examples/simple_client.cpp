/**
 * Simple CARLA Client Example
 * 
 * This example demonstrates how to:
 * - Connect to a CARLA server
 * - Get world information
 * - Spawn a vehicle
 * - Control the vehicle
 */

#include <carla/client/Client.h>
#include <carla/client/World.h>
#include <carla/client/Map.h>
#include <carla/client/BlueprintLibrary.h>
#include <carla/client/Actor.h>
#include <carla/client/Vehicle.h>
#include <carla/geom/Transform.h>
#include <carla/geom/Location.h>
#include <carla/rpc/VehicleControl.h>

#include <iostream>
#include <thread>
#include <chrono>

namespace cc = carla::client;
namespace cg = carla::geom;
namespace crpc = carla::rpc;

int main(int argc, char* argv[]) {
    try {
        // Default connection parameters
        std::string host = "localhost";
        uint16_t port = 2000;
        
        if (argc > 1) {
            host = argv[1];
        }
        if (argc > 2) {
            port = static_cast<uint16_t>(std::stoi(argv[2]));
        }

        std::cout << "Connecting to CARLA server at " << host << ":" << port << "..." << std::endl;

        // Create a CARLA client and connect to the server
        cc::Client client(host, port);
        client.SetTimeout(std::chrono::seconds(10));

        // Get the server version
        std::string version = client.GetServerVersion();
        std::cout << "Connected to CARLA server version: " << version << std::endl;

        // Load the world
        auto world = client.GetWorld();
        std::cout << "World loaded: " << world.GetMap()->GetName() << std::endl;

        // Get the blueprint library
        auto blueprint_library = world.GetBlueprintLibrary();
        std::cout << "Blueprint library contains " << blueprint_library->size() << " blueprints" << std::endl;

        // Find a vehicle blueprint
        auto vehicle_bp = blueprint_library->Find("vehicle.tesla.model3");
        if (!vehicle_bp) {
            std::cerr << "Could not find vehicle.tesla.model3 blueprint!" << std::endl;
            return 1;
        }
        std::cout << "Found vehicle blueprint: " << vehicle_bp->GetId() << std::endl;

        // Get a spawn point
        auto spawn_points = world.GetMap()->GetRecommendedSpawnPoints();
        if (spawn_points.empty()) {
            std::cerr << "No spawn points available!" << std::endl;
            return 1;
        }

        cg::Transform spawn_point = spawn_points[0];
        std::cout << "Spawn point: (" 
                  << spawn_point.location.x << ", "
                  << spawn_point.location.y << ", "
                  << spawn_point.location.z << ")" << std::endl;

        // Spawn the vehicle
        std::cout << "Spawning vehicle..." << std::endl;
        auto actor = world.SpawnActor(*vehicle_bp, spawn_point);
        auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
        std::cout << "Vehicle spawned with ID: " << vehicle->GetId() << std::endl;

        // Drive the vehicle forward for 5 seconds
        std::cout << "Driving vehicle forward..." << std::endl;
        crpc::VehicleControl control;
        control.throttle = 0.5f;
        control.steer = 0.0f;
        control.brake = 0.0f;
        control.hand_brake = false;
        control.reverse = false;
        control.manual_gear_shift = false;

        vehicle->ApplyControl(control);
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Stop the vehicle
        std::cout << "Stopping vehicle..." << std::endl;
        control.throttle = 0.0f;
        control.brake = 1.0f;
        vehicle->ApplyControl(control);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Destroy the vehicle
        std::cout << "Destroying vehicle..." << std::endl;
        vehicle->Destroy();

        std::cout << "Example completed successfully!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
