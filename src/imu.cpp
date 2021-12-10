#include <Wire.h>
#include <I2Cdev.h>
#include <MPU9250.h>
#include "imu.h"

static MPU9250 imu;
static I2Cdev   I2C_M;

static uint8_t buffer_m[6];


static int16_t ax, ay, az;
static int16_t gx, gy, gz;
static int16_t mx, my, mz;



static float heading;
static float tiltheading;

static float Axyz[3];
static float Mxyz[3];


#define sample_num_mdate  5000

static volatile float mx_sample[3];
static volatile float my_sample[3];
static volatile float mz_sample[3];

static float mx_centre = 0;
static float my_centre = 0;
static float mz_centre = 0;

static volatile int mx_max =0;
static volatile int my_max =0;
static volatile int mz_max =0;

static volatile int mx_min =0;
static volatile int my_min =0;
static volatile int mz_min =0;

void initImu() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin();
    
    // initialize device
    Serial.println("Initializing I2C devices...");
    imu.initialize();
    
    // verify connection
    Serial.println("Testing device connections...");
    bool haveIMU = imu.testConnection();
    Serial.println(haveIMU ? "MPU9250 connection successful" : "MPU9250 connection failed");
    
    delay(1000);
    Serial.println("     ");
    
    /*if (haveIMU) {
       Mxyz_init_calibrated ();
    }*/
}



void getHeading(void)
{
    heading=180*atan2(Mxyz[1],Mxyz[0])/PI;
    if(heading <0) heading +=360;
}

void getTiltHeading(void)
{
    float pitch = asin(-Axyz[0]);
    float roll = asin(Axyz[1]/cos(pitch));
    
    float xh = Mxyz[0] * cos(pitch) + Mxyz[2] * sin(pitch);
    float yh = Mxyz[0] * sin(roll) * sin(pitch) + Mxyz[1] * cos(roll) - Mxyz[2] * sin(roll) * cos(pitch);
    float zh = -Mxyz[0] * cos(roll) * sin(pitch) + Mxyz[1] * sin(roll) + Mxyz[2] * cos(roll) * cos(pitch);
    tiltheading = 180 * atan2(yh, xh)/PI;
    if(yh<0)    tiltheading +=360;
}



void getCompass_Data(void)
{
    I2C_M.writeByte(MPU9150_RA_MAG_ADDRESS, 0x0A, 0x01); //enable the magnetometer
    delay(10);
    I2C_M.readBytes(MPU9150_RA_MAG_ADDRESS, MPU9150_RA_MAG_XOUT_L, 6, buffer_m);
    
    mx = ((int16_t)(buffer_m[1]) << 8) | buffer_m[0] ;
    my = ((int16_t)(buffer_m[3]) << 8) | buffer_m[2] ;
    mz = ((int16_t)(buffer_m[5]) << 8) | buffer_m[4] ;
    
    Mxyz[0] = (double) mx * 1200 / 4096;
    Mxyz[1] = (double) my * 1200 / 4096;
    Mxyz[2] = (double) mz * 1200 / 4096;
}


void get_one_sample_date_mxyz()
{
    getCompass_Data();
    mx_sample[2] = Mxyz[0];
    my_sample[2] = Mxyz[1];
    mz_sample[2] = Mxyz[2];
}






void get_calibration_Data ()
{
    for (int i=0; i<sample_num_mdate;i++)
    {
        get_one_sample_date_mxyz();
        /*
        Serial.print(mx_sample[2]);
        Serial.print(" ");
        Serial.print(my_sample[2]);                            //you can see the sample data here .
        Serial.print(" ");
        Serial.println(mz_sample[2]);
        */
        
        
        
        if (mx_sample[2]>=mx_sample[1])mx_sample[1] = mx_sample[2];
        if (my_sample[2]>=my_sample[1])my_sample[1] = my_sample[2]; //find max value
        if (mz_sample[2]>=mz_sample[1])mz_sample[1] = mz_sample[2];
        
        if (mx_sample[2]<=mx_sample[0])mx_sample[0] = mx_sample[2];
        if (my_sample[2]<=my_sample[0])my_sample[0] = my_sample[2];//find min value
        if (mz_sample[2]<=mz_sample[0])mz_sample[0] = mz_sample[2];
        
    }
    
    mx_max = mx_sample[1];
    my_max = my_sample[1];
    mz_max = mz_sample[1];
    
    mx_min = mx_sample[0];
    my_min = my_sample[0];
    mz_min = mz_sample[0];
    
    
    
    mx_centre = (mx_max + mx_min)/2;
    my_centre = (my_max + my_min)/2;
    mz_centre = (mz_max + mz_min)/2;
    
}


void Mxyz_init_calibrated ()
{
    
    Serial.println(F("Before using 9DOF,we need to calibrate the compass frist,It will takes about 2 minutes."));
    Serial.print("  ");
    Serial.println(F("During  calibratting ,you should rotate and turn the 9DOF all the time within 2 minutes."));
    Serial.print("  ");
    Serial.println(F("If you are ready ,please sent a command data 'ready' to start sample and calibrate."));
    while(!Serial.find("ready"));
    Serial.println("  ");
    Serial.println("ready");
    Serial.println("Sample starting......");
    Serial.println("waiting ......");
    
    get_calibration_Data ();
    
    Serial.println("     ");
    Serial.println("compass calibration parameter ");
    Serial.print(mx_centre);
    Serial.print("     ");
    Serial.print(my_centre);
    Serial.print("     ");
    Serial.println(mz_centre);
    Serial.println("    ");
}

void getAccel_Data(void)
{
    imu.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
    Axyz[0] = (double) ax / 16384;
    Axyz[1] = (double) ay / 16384;
    Axyz[2] = (double) az / 16384;
}

void getIMUGyro(Vec3D& rotations)
{
    imu.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
    rotations.x = (double) gx * 250 / 32768;
    rotations.y = (double) gy * 250 / 32768;
    rotations.z = (double) gz * 250 / 32768;
}

void getCompassDate_calibrated ()
{
    getCompass_Data();
    Mxyz[0] = Mxyz[0] - mx_centre;
    Mxyz[1] = Mxyz[1] - my_centre;
    Mxyz[2] = Mxyz[2] - mz_centre;
}
