
//test
#include "Calculation/AhrsCalculation.h"
#include "MMC56x3/MMC56x3.h"
#include "TDK40607P/ICM42670P.h"
#include "concurrentqueue/concurrentqueue.h"
#include "websocket/websocket_client.hpp"
#include <csignal>
#include <iostream>
#include <rapidcsv.h>
#include <string>
#include <unistd.h>
//
const std::string i2cDevice         = "/dev/i2c-1";
uint8_t           deviceAddress_mmc = 0x30;
uint8_t           deviceAddress_imu = 0x69;
//
rapidcsv::Document csv_doc_;
WebSocketClient    client;
AhrsCalculation    ahrs_calculation_;

//
void init_out_csv( rapidcsv::Document& csv_doc )
{
    //
    csv_doc.SetColumnName( 0, "Time (s)" );
    //
    csv_doc.SetColumnName( 1, "Gyroscope X (deg/s)" );
    csv_doc.SetColumnName( 2, "Gyroscope Y (deg/s)" );
    csv_doc.SetColumnName( 3, "Gyroscope Z (deg/s)" );
    //
    csv_doc.SetColumnName( 4, "Accelerometer X (g)" );
    csv_doc.SetColumnName( 5, "Accelerometer Y (g)" );
    csv_doc.SetColumnName( 6, "Accelerometer Z (g)" );
    //
    csv_doc.SetColumnName( 7, "Magnetometer X (uT)" );
    csv_doc.SetColumnName( 8, "Magnetometer Y (uT)" );
    csv_doc.SetColumnName( 9, "Magnetometer Z (uT)" );
    //
    csv_doc.SetColumnName( 10, "quate X (d)" );
    csv_doc.SetColumnName( 11, "quate Y (d)" );
    csv_doc.SetColumnName( 12, "quate Z (d)" );
    csv_doc.SetColumnName( 13, "quate W (d)" );

    csv_doc.SetColumnName( 14, "Roll (d)" );
    csv_doc.SetColumnName( 15, "Pitch (d)" );
    csv_doc.SetColumnName( 16, "Yaw (d)" );
    //
    csv_doc.SetColumnName( 17, "Estimated Accelerometer X (g)" );
    csv_doc.SetColumnName( 18, "Estimated Accelerometer Y (g)" );
    csv_doc.SetColumnName( 19, "Estimated Accelerometer Z (g)" );
    //
    csv_doc.SetColumnName( 20, "Estimated Velocity X (m/s)" );
    csv_doc.SetColumnName( 21, "Estimated Velocity Y (m/s)" );
    csv_doc.SetColumnName( 22, "Estimated Velocity Z (m/s)" );
    //
    csv_doc.SetColumnName( 23, "Estimated Position X (m)" );
    csv_doc.SetColumnName( 24, "Estimated Position Y (m)" );
    csv_doc.SetColumnName( 25, "Estimated Position Z (m)" );
}
void update_out_csv( int& index, rapidcsv::Document& csv_doc, const SENSOR_DB& sensor_data )
{

    // 添加数据行
    csv_doc.SetCell< float >( 0, index, sensor_data.time );
    csv_doc.SetCell< float >( 1, index, sensor_data.gyro_x );
    csv_doc.SetCell< float >( 2, index, sensor_data.gyro_y );
    csv_doc.SetCell< float >( 3, index, sensor_data.gyro_z );
    csv_doc.SetCell< float >( 4, index, sensor_data.acc_x );
    csv_doc.SetCell< float >( 5, index, sensor_data.acc_y );
    csv_doc.SetCell< float >( 6, index, sensor_data.acc_z );
    csv_doc.SetCell< float >( 7, index, sensor_data.mag_x );
    csv_doc.SetCell< float >( 8, index, sensor_data.mag_y );
    csv_doc.SetCell< float >( 9, index, sensor_data.mag_z );
    csv_doc.SetCell< float >( 10, index, sensor_data.quate_x );
    csv_doc.SetCell< float >( 11, index, sensor_data.quate_y );
    csv_doc.SetCell< float >( 12, index, sensor_data.quate_z );
    csv_doc.SetCell< float >( 13, index, sensor_data.quate_w );
    csv_doc.SetCell< float >( 14, index, sensor_data.roll );
    csv_doc.SetCell< float >( 15, index, sensor_data.pitch );
    csv_doc.SetCell< float >( 16, index, sensor_data.yaw );
    csv_doc.SetCell< float >( 17, index, sensor_data.eacc_x );
    csv_doc.SetCell< float >( 18, index, sensor_data.eacc_y );
    csv_doc.SetCell< float >( 19, index, sensor_data.eacc_z );
    csv_doc.SetCell< float >( 20, index, sensor_data.vel_x );
    csv_doc.SetCell< float >( 21, index, sensor_data.vel_y );
    csv_doc.SetCell< float >( 22, index, sensor_data.vel_z );
    csv_doc.SetCell< float >( 23, index, sensor_data.pos_x );
    csv_doc.SetCell< float >( 24, index, sensor_data.pos_y );
    csv_doc.SetCell< float >( 25, index, sensor_data.pos_z );
    //
    index++;
}
void close_out_csv( rapidcsv::Document& csv_doc )
{
    try
    {
        csv_doc.Save( "out.csv" );
        std::cout << "数据已成功写入 output.csv 文件。" << std::endl;
    }
    catch ( const std::exception& e )
    {
        std::cerr << "写入文件时出错: " << e.what() << std::endl;
    }
}
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
    sensor_data.time = getMicrosecondTimestamp();
    // printf( "Time: %ld\n", sensor_data.time );
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
// 信号处理函数
void signalHandler_for_gloab( int signum )
{
    std::cout << "收到 Ctrl+C 信号，程序即将退出..." << std::endl;
    // 关闭 CSV 文件
    close_out_csv( csv_doc_ );
    // 退出程序
    exit( signum );
}

//
// 子线程函数，处理控制台输入响应
void handleInput()
{
    std::string input;
    while ( true )
    {
        std::cout << "请输入一些内容（输入 'exit' 退出）: ";
        std::cin >> input;
        if ( input == "exit" )
        {
            client.stop();
            break;
        }
        else if ( input == "reset" )
        {
            ahrs_calculation_.ResetInitial();
        }
        std::cout << "你输入了: " << input << std::endl;
    }
}
//
int main()
{

    init_out_csv( csv_doc_ );
    int index = 0;
    // 注册信号处理函数，处理 SIGINT 信号（Ctrl+C 产生的信号）
    std::signal( SIGINT, signalHandler_for_gloab );
    //
    std::string host = "192.168.254.116";
    std::string port = "18080";
    std::string text = "Hello, WebSocket Server!";

    //
    MMC56x3  sensor_mmc_;
    ICM42670 sensor_imu_;

    //
    SENSOR_DB sensor_data_;
    //

    client.setHost( host );
    client.setPort( port );
    client.start();
    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
    //
    init_sensor( sensor_mmc_, sensor_imu_ );
    //
    client.handleSend( "Start" );
    //
    // 创建子线程
    std::thread inputThread( handleInput );
    //
    while ( client.running_ )
    {
        bool ret = read_sensor_data( sensor_mmc_, sensor_imu_, ahrs_calculation_, sensor_data_ );
        if ( ret )
        {
            client.handleSend( sensor_data_.to_string() );
            //
            update_out_csv( index, csv_doc_, sensor_data_ );
        }
    }

    //
    client.stop();
    close_out_csv( csv_doc_ );
    //
    return 0;
}
