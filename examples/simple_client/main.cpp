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
#include <cmath>
#include <limits>

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

        std::cout << "Total spawn points available: " << spawn_points.size() << std::endl;
        
        // Target location near the curve
        cg::Location target(-83.5462f, 131.02f, 0.0f);
        
        // Find closest spawn point to target
        size_t spawn_index = 0;
        float min_distance = std::numeric_limits<float>::max();
        
        for (size_t i = 0; i < spawn_points.size(); ++i) {
            auto& sp = spawn_points[i];
            float dx = sp.location.x - target.x;
            float dy = sp.location.y - target.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance < min_distance) {
                min_distance = distance;
                spawn_index = i;
            }
        }
        
        cg::Transform spawn_point = spawn_points[spawn_index];
        std::cout << "\nTarget location: (" << target.x << ", " << target.y << ", " << target.z << ")" << std::endl;
        std::cout << "Closest spawn point [" << spawn_index << "] at distance " << min_distance << "m: (" 
                  << spawn_point.location.x << ", "
                  << spawn_point.location.y << ", "
                  << spawn_point.location.z << ")" << std::endl;

        // Spawn the vehicle
        std::cout << "Spawning vehicle..." << std::endl;
        auto actor = world.SpawnActor(*vehicle_bp, spawn_point);
        auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
        std::cout << "Vehicle spawned with ID: " << vehicle->GetId() << std::endl;

        // Enable autopilot for easier visibility
        std::cout << "\nEnabling autopilot for 60 seconds..." << std::endl;
        vehicle->SetAutopilot(true);
        
        // Monitor vehicle position for 60 seconds
        for (int i = 0; i < 45; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            auto location = vehicle->GetLocation();
            auto velocity = vehicle->GetVelocity();
            float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
            std::cout << "  [" << (i+1) << "s] Position: (" 
                      << location.x << ", " << location.y << ", " << location.z 
                      << ") Speed: " << speed << " m/s" << std::endl;
        }
        
        std::cout << "\nDisabling autopilot..." << std::endl;
        vehicle->SetAutopilot(false);
        
        // Drive manually with strong throttle
        std::cout << "Manual driving for 5 seconds..." << std::endl;
        crpc::VehicleControl control;
        control.hand_brake = false;
        control.reverse = false;
        control.manual_gear_shift = false;
        control.throttle = 0.8f;
        control.steer = 0.0f;
        control.brake = 0.0f;
        
        vehicle->ApplyControl(control);
        
        for (int i = 0; i < 5; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            auto location = vehicle->GetLocation();
            auto velocity = vehicle->GetVelocity();
            float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
            std::cout << "  [" << (i+1) << "s] Position: (" 
                      << location.x << ", " << location.y << ", " << location.z 
                      << ") Speed: " << speed << " m/s" << std::endl;
        }

        // Stop the vehicle
        std::cout << "Stopping vehicle..." << std::endl;
        control.throttle = 0.0f;
        control.steer = 0.0f;
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
