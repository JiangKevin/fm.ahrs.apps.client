
//
#include "Calculation/AhrsCalculation.h"
#include "MMC56x3/MMC56x3.h"
#include "TDK40607P/ICM42670P.h"
#include "concurrentqueue/concurrentqueue.h"
#include "websocket/websocket_client.hpp"
#include <iostream>
#include <rapidcsv.h>
#include <string>
//
const std::string i2cDevice         = "/dev/i2c-1";
uint8_t           deviceAddress_mmc = 0x30;
uint8_t           deviceAddress_imu = 0x69;
//
void init_sensor( MMC56x3& sensor_mmc, ICM42670& sensor_imu )
{
    // Initializing the MMC56x3
    if ( ! sensor_mmc.begin( deviceAddress_mmc, i2cDevice.c_str() ) )
    {
        printf( "Failed to initialize MMC56x3 sensor\n" );
    }
    // Initializing the ICM42670
    int ret;
    ret = sensor_imu.begin( false, deviceAddress_imu, i2cDevice.c_str() );
    if ( ret != 0 )
    {
        printf( "Failed to initialize ICM42670 sensor\n" );
    }
    // Accel ODR = 100 Hz and Full Scale Range = 16G
    sensor_imu.startAccel( 100, 16 );
    // Gyro ODR = 100 Hz and Full Scale Range = 2000 dps
    sensor_imu.startGyro( 100, 2000 );
    // Wait IMU to start
    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
}
//
bool read_sensor_data( MMC56x3& sensor_mmc, ICM42670& sensor_imu, AhrsCalculation& ahrs_calculation, SENSOR_DB& sensor_data )
{
    //
    sensor_data.time = clock();
    // MMC56x3
    float x, y, z;
    if ( sensor_mmc.getEvent( x, y, z ) )
    {
        sensor_data.mag_x = x;
        sensor_data.mag_y = y;
        sensor_data.mag_z = z;
    }
    else
    {
        return false;
    }
    float temp = sensor_mmc.readTemperature();
    if ( std::isnan( temp ) )
    {
        return false;
    }
    // TDK42607
    inv_imu_sensor_event_t imu_event;
    // Get last event
    sensor_imu.getDataFromRegisters( imu_event );
    //
    sensor_data.acc_x  = imu_event.accel[ 0 ] / 2048.0;
    sensor_data.acc_y  = imu_event.accel[ 1 ] / 2048.0;
    sensor_data.acc_z  = imu_event.accel[ 2 ] / 2048.0;
    sensor_data.gyro_x = imu_event.gyro[ 0 ] / 16.4;
    sensor_data.gyro_y = imu_event.gyro[ 1 ] / 16.4;
    sensor_data.gyro_z = imu_event.gyro[ 2 ] / 16.4;
    //
    ahrs_calculation.SolveAnCalculation( &sensor_data );
    // Run @ ODR 100Hz:10
    std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
    //
    return true;
}
//
int main()
{
    std::string host = "192.168.254.116";
    std::string port = "8080";
    std::string text = "Hello, WebSocket Server!";
    //
    MMC56x3         sensor_mmc_;
    ICM42670        sensor_imu_;
    AhrsCalculation ahrs_calculation_;
    //
    SENSOR_DB sensor_data_;
    //
    WebSocketClient client;
    client.setHost( host );
    client.setPort( port );
    client.start();
    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
    //
    client.handleSend( "Periodic message from client" );
    //
    init_sensor( sensor_mmc_, sensor_imu_ );
    //
    while ( client.running_ )
    {
        bool ret = read_sensor_data( sensor_mmc_, sensor_imu_, ahrs_calculation_, sensor_data_ );
        if ( ret )
        {
            client.handleSend( sensor_data_.to_string() );
        }
    }

    //
    client.stop();
    //
    return 0;
}
