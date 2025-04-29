
//
#pragma once
//
#include "Fusion/Fusion.h"
#include "concurrentqueue/concurrentqueue.h"
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
//
#define SAMPLE_RATE ( 100 )  // replace this with actual sample rate
//

static std::vector< std::string > splitString( const std::string& str, char delimiter )
{
    std::vector< std::string > result;
    std::istringstream         iss( str );
    std::string                token;
    while ( std::getline( iss, token, delimiter ) )
    {
        result.push_back( token );
    }
    return result;
}
//
struct SENSOR_DB
{
    float time;
    float acc_x;
    float acc_y;
    float acc_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float mag_x;
    float mag_y;
    float mag_z;
    float quate_x;
    float quate_y;
    float quate_z;
    float quate_w;
    float roll;
    float pitch;
    float yaw;
    float pos_x;
    float pos_y;
    float pos_z;
    //
    std::string to_string()
    {
        return std::to_string( time ) + "," + std::to_string( acc_x ) + "," + std::to_string( acc_y ) + "," + std::to_string( acc_z ) + "," + std::to_string( gyro_x ) + "," + std::to_string( gyro_y ) + "," + std::to_string( gyro_z ) + "," + std::to_string( mag_x ) + "," + std::to_string( mag_y )
               + "," + std::to_string( mag_z ) + "," + std::to_string( quate_x ) + "," + std::to_string( quate_y ) + "," + std::to_string( quate_z ) + "," + std::to_string( quate_w ) + "," + std::to_string( roll ) + "," + std::to_string( pitch ) + "," + std::to_string( yaw ) + ","
               + std::to_string( pos_x ) + "," + std::to_string( pos_y ) + "," + std::to_string( pos_z );
    };
    //
    std::string to_info()
    {
        //
        std::string info = "Time: " + std::to_string( time ) + "\n";
        info += "Accelerometer: (" + std::to_string( acc_x ) + ", " + std::to_string( acc_y ) + ", " + std::to_string( acc_z ) + ")\n";
        info += "Gyroscope: (" + std::to_string( gyro_x ) + ", " + std::to_string( gyro_y ) + ", " + std::to_string( gyro_z ) + ")\n";
        info += "Magnetometer: (" + std::to_string( mag_x ) + ", " + std::to_string( mag_y ) + ", " + std::to_string( mag_z ) + ")\n";
        info += "Quaternion: (" + std::to_string( quate_x ) + ", " + std::to_string( quate_y ) + ", " + std::to_string( quate_z ) + ", " + std::to_string( quate_w ) + ")\n";
        info += "Roll: " + std::to_string( roll ) + " pitch: " + std::to_string( pitch ) + " yaw: " + std::to_string( yaw ) + "\n";
        info += "Position: (" + std::to_string( pos_x ) + ", " + std::to_string( pos_y ) + ", " + std::to_string( pos_z ) + ")\n";
        return info;
    }
    //
    void getValueFromString( std::string v )
    {
        char delimiter = ',';
        auto values    = splitString( v, delimiter );
        //
        if ( values.size() == 20 )
        {
            time    = std::stof( values[ 0 ] );
            acc_x   = std::stof( values[ 1 ] );
            acc_y   = std::stof( values[ 2 ] );
            acc_z   = std::stof( values[ 3 ] );
            gyro_x  = std::stof( values[ 4 ] );
            gyro_y  = std::stof( values[ 5 ] );
            gyro_z  = std::stof( values[ 6 ] );
            mag_x   = std::stof( values[ 7 ] );
            mag_y   = std::stof( values[ 8 ] );
            mag_z   = std::stof( values[ 9 ] );
            quate_x = std::stof( values[ 10 ] );
            quate_y = std::stof( values[ 11 ] );
            quate_z = std::stof( values[ 12 ] );
            quate_w = std::stof( values[ 13 ] );
            roll    = std::stof( values[ 14 ] );
            pitch   = std::stof( values[ 15 ] );
            yaw     = std::stof( values[ 16 ] );
            pos_x   = std::stof( values[ 17 ] );
            pos_y   = std::stof( values[ 18 ] );
            pos_z   = std::stof( values[ 19 ] );
        }
    }
};
// 验证字符串是否为数字
static bool isNumber( const std::string& str )
{
    for ( char c : str )
    {
        if ( ! std::isdigit( c ) )
        {
            return false;
        }
    }
    return true;
}
//
//
class AhrsCalculation
{
public:
    AhrsCalculation();
    ~AhrsCalculation(){};
public:
    // Define calibration (replace with actual calibration data if available)
    const FusionMatrix gyroscopeMisalignment     = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    const FusionVector gyroscopeSensitivity      = { 1.0f, 1.0f, 1.0f };
    const FusionVector gyroscopeOffset           = { 0.0f, 0.0f, 0.0f };
    const FusionMatrix accelerometerMisalignment = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    const FusionVector accelerometerSensitivity  = { 1.0f, 1.0f, 1.0f };
    const FusionVector accelerometerOffset       = { 0.0f, 0.0f, 0.0f };
    const FusionMatrix softIronMatrix            = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    const FusionVector hardIronOffset            = { 0.0f, 0.0f, 0.0f };
public:
    // Initialise algorithms
    FusionOffset offset;
    FusionAhrs   ahrs;
    // Set AHRS algorithm settings
    const FusionAhrsSettings settings = {
        .convention            = FusionConventionNwu,
        .gain                  = 0.5f,
        .gyroscopeRange        = 2000.0f, /* replace this with actual gyroscope range in degrees/s */
        .accelerationRejection = 10.0f,
        .magneticRejection     = 10.0f,
        .recoveryTriggerPeriod = 5 * SAMPLE_RATE, /* 5 seconds */
    };
public:
    void SolveAnCalculation( SENSOR_DB* sensor_data );
};
