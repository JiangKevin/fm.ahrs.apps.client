#include "AhrsCalculation.h"
#include <cstdio>
#include <time.h>
//
// 带 Context* 参数的构造函数实现
AhrsCalculation::AhrsCalculation()
{
    FusionAhrsSetSettings( &ahrs, &settings );
}

//
void AhrsCalculation::SolveAnCalculation( SENSOR_DB* sensor_data )
{
    // Acquire latest sensor data
    const int64_t timestamp = sensor_data->time;
    // printf( "Timestamp Delta Time: %ld\n", timestamp );

    FusionVector gyroscope     = { sensor_data->gyro_x, sensor_data->gyro_y, sensor_data->gyro_z };
    FusionVector accelerometer = { sensor_data->acc_x, sensor_data->acc_y, sensor_data->acc_z };
    FusionVector magnetometer  = { sensor_data->mag_x, sensor_data->mag_y, sensor_data->mag_z };
    //
    // Apply calibration
    gyroscope     = FusionCalibrationInertial( gyroscope, gyroscopeMisalignment, gyroscopeSensitivity, gyroscopeOffset );
    accelerometer = FusionCalibrationInertial( accelerometer, accelerometerMisalignment, accelerometerSensitivity, accelerometerOffset );
    magnetometer  = FusionCalibrationMagnetic( magnetometer, softIronMatrix, hardIronOffset );

    // Update gyroscope offset correction algorithm
    gyroscope = FusionOffsetUpdate( &offset, gyroscope );

    // Calculate delta time (in seconds) to account for gyroscope sample clock error
    static long long previousTimestamp;
    const float      deltaTime = ( float )( timestamp - previousTimestamp ) / ( float )CLOCKS_PER_SEC;
    previousTimestamp          = timestamp;
    //
    // printf( "Delta Time: %f\n", deltaTime );
    // Update gyroscope AHRS algorithm
    FusionAhrsUpdate( &ahrs, gyroscope, accelerometer, magnetometer, deltaTime );

    // Print algorithm outputs
    auto               quate = FusionAhrsGetQuaternion( &ahrs );
    const FusionEuler  euler = FusionQuaternionToEuler( FusionAhrsGetQuaternion( &ahrs ) );
    const FusionVector earth = FusionAhrsGetEarthAcceleration( &ahrs );
    //
    sensor_data->quate_x = quate.element.x;
    sensor_data->quate_y = quate.element.y;
    sensor_data->quate_z = quate.element.z;
    sensor_data->quate_w = quate.element.w;
    //
    sensor_data->roll  = euler.angle.roll;
    sensor_data->pitch = euler.angle.pitch;
    sensor_data->yaw   = euler.angle.yaw;
    //
    sensor_data->eacc_x = earth.axis.x;
    sensor_data->eacc_y = earth.axis.y;
    sensor_data->eacc_z = earth.axis.z;

    //
    printf( "Quaternion: %f %f %f %f\n", sensor_data->quate_x, sensor_data->quate_y, sensor_data->quate_z, sensor_data->quate_w );
    printf( "Euler: %f %f %f\n", sensor_data->roll, sensor_data->pitch, sensor_data->yaw );
    // printf( "Earth Acceleration: %f %f %f\n", sensor_data->eacc_x, sensor_data->eacc_y, sensor_data->eacc_z );
    //

    calculateSurfaceVelocity( sensor_data, deltaTime );

    //
}
//
void AhrsCalculation::calculateSurfaceVelocity( SENSOR_DB* sensor_data, float dt )
{
    // Calculate the velocity using the trapezoidal rule
    sensor_data->vel_x += ( sensor_data->eacc_x * dt );
    sensor_data->vel_y += ( sensor_data->eacc_x * dt );
    sensor_data->vel_z += ( sensor_data->eacc_x * dt );
    //
    // Calculate the position using the trapezoidal rule
    sensor_data->pos_x += ( sensor_data->vel_x * dt );
    sensor_data->pos_y += ( sensor_data->vel_y * dt );
    sensor_data->pos_z += ( sensor_data->vel_z * dt );
}
