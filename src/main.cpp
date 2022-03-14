#include <cmath>
#include "arduinoSystem.h"
#include <Stepper.h>
#include "imu.h"


/** The number of individual steps that make up a full revolution of the stepper motor. */
#define MOTOR_STEPS_PER_REVOLUTION 2048

/**
 * The time to wait between updates in microseconds.
 * The datasheet (https://www.gotronic.fr/pj-1136.pdf) specifies a maximum response frequency of
 * 900 phases per seconds, e.g ~1111 microseconds need to be waited before switching to the next
 * phase (the next step).
 * To overcome initial resistance, the maximum change in phases per seconds at startup is 500,
 * e.g 2000 milliseconds between steps.
 */
#define MOTOR_UPDATE_PERIOD_MICRO_S 2000

/** The motor that is used to turn the base plate of the laser. */
static Stepper baseMotor(MOTOR_STEPS_PER_REVOLUTION, MOTOR_UPDATE_PERIOD_MICRO_S, 8, 9, 10, 11);

/** The time in milliseconds since boot when the gyroscope was last read. */
static unsigned long lastMeasurementMillis = 0;

/** The target angle of the base motor. */
static double targetBaseAngle = 0;
/** The radius of the Earth in meter. */
#define  Rt 6378.137e3
/** Geometrical parameters for coordinates calculation in the LTP */
double a = 6378137.0;
double b= 6356752.3142;
double f=(a-b)/a;
double e= sqrt(f*(2-f));


void setup() {
    // Initialize the serial port.
    Serial.begin(9600);
    Serial.println("Boot");
    initImu();
    Serial.println("Boot complete");
    lastMeasurementMillis = millis();
}

/**
 * Normalize the angle to be between 0 and 360 degrees.
 * @param angle The angle to normalize in degrees.
 * @return The normalized angle.
 */
static double normalizeAngle(double angle) {
    angle = fmod(angle, 360);
    if (angle < 0) {
        angle += 360;
    }
    return angle;
}


struct coord_GPS {
    /** Angles have to be transform in radian before assignation !!! **/
    double longitude;
    double latitude;
    double altitude;
};

coord_GPS laser_GPS = {
        /** Angle in radian !! **/
        .longitude=23,
        .latitude=45,
        .altitude= 23,
};


struct coord_xyz {
    double x;
    double y;
    double z;
};

float N_laser = a/ sqrt(1-pow(e,2)*pow(sin(laser_GPS.latitude),2));

coord_xyz laser_ECEF{
    .x=(laser_GPS.altitude+N_laser)*cos(laser_GPS.latitude)*cos(laser_GPS.longitude),
    .y=(laser_GPS.altitude+N_laser)*
                cos(laser_GPS.latitude)*sin(laser_GPS.longitude),
    .z=(laser_GPS.altitude+(1-pow(e,2))*N_laser)*sin(laser_GPS.latitude)
};



static coord_xyz conversion_GPS_to_LTP(coord_GPS balloon_GPS) {
    /** This function convert the coordinates given by the GPS to the Local Tangent Place reference frame. The center of this reference frame being the laser.**/

    float N = a/ sqrt(1-pow(e,2)*pow(sin(balloon_GPS.latitude),2));

    float lat=balloon_GPS.latitude;
    float lon=balloon_GPS.longitude;
    float alt=balloon_GPS.altitude;
    coord_xyz b_ECEF;
    coord_xyz b_LTP;
    coord_xyz Transition; /*This is a transition vector used for calculation*/

    b_ECEF.x=(alt+N)*cos(lat)*
                cos(lon);
    b_ECEF.y=(alt+N)*cos(lat)*sin(lon);
    b_ECEF.z=(alt+(1-pow(e,2))*N)*sin(lat);

    Transition.x=b_ECEF.x-laser_ECEF.x;
    Transition.y=b_ECEF.y-laser_ECEF.y;
    Transition.z=b_ECEF.z-laser_ECEF.z;

    b_LTP.x=Transition.x*(-sin(lat))+Transition.y*cos(lon);
    b_LTP.y=Transition.x*(-cos(lon)*sin(lat))+Transition.y*(-sin(lat)*sin(lon))+Transition.z*cos(lat);
    b_LTP.z=Transition.x*cos(lat)*cos(lon)+Transition.y*cos(lat)*sin(lon)+Transition.z*sin(lat);
    
    return b_LTP;

}


void loop() {
    // Measure the rotation.
    Vec3D rotations;
    getIMUGyro(rotations);
    unsigned long currentTime = millis();
    
    // Calculate the angular change since the last iteration.
    targetBaseAngle += rotations.z * ((currentTime - lastMeasurementMillis) / 1000.0);
    targetBaseAngle = normalizeAngle(targetBaseAngle);
    
    Serial.println(targetBaseAngle);
    // Move the motor to compensate for the rotation.
    baseMotor.setTargetAngle(targetBaseAngle);
    lastMeasurementMillis = currentTime;
}
