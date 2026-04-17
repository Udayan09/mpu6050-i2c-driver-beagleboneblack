/*Register Addresses*/

/*Sampling Rate Register*/
#define MPU6500_REG_SMPLRT_DIV	    	0x19

/*Config Registers*/
#define MPU6500_REG_CONFIG          	0x1A
#define MPU6500_REG_GYRO_CFG        	0x1B
#define MPU6500_REG_ACCEL_CFG       	0x1C
#define MPU6500_REG_ACCEL_CFG2      	0x1D

/*Interrupt Registers*/
#define MPU6500_REG_INT_PIN_CFG     	0x37
#define MPU6500_REG_INT_EN          	0x38
#define MPU6500_REG_INT_STATUS      	0x3A

/*Sensor Output Registers*/
#define MPU6500_REG_ACCEL_X_OUT		0x3B
#define MPU6500_REG_ACCEL_Y_OUT		0x3D
#define MPU6500_REG_ACCEL_Z_OUT		0x3F
#define MPU6500_REG_TEMP_OUT		0x41
#define MPU6500_REG_GYRO_X_OUT		0x43
#define MPU6500_REG_GYRO_Y_OUT		0x45
#define MPU6500_REG_GYRO_Z_OUT		0x47

/*Power Management Registers*/
#define MPU6500_REG_PWR_MGMT1       	0x6B
#define MPU6500_REG_PWR_MGMT2       	0x6C

/*Fifo Registers*/
#define MPU6500_REG_FIFO_EN         	0x23
#define MPU6500_REG_FIFO_COUNT_H    	0x72
#define MPU6500_REG_FIFO_COUNT_L    	0x73
#define MPU6500_REG_FIFO_RW         	0x74

/*Who am i Register (For identification)*/
#define MPU6500_REG_WHO_AM_I        	0x75
#define MPU6500_WHOAMI_VALUE	    	0x70


/*Register Value Macros*/
#define MPU_PWR_RST			0x80
#define MPU_PWR_AUTO_CLK0		0x01

/*Scale Values*/
#define MPU6500_ACCEL_DEF_SCALE		2	//Default accel scale value: 2g
#define MPU6500_GYRO_DEF_SCALE		250	//Default gyro scale value: 250dps

/*MPU6500 Scan order for fifo scan*/
enum MPU6500_SCAN {
	MPU6500_SCAN_ACCEL_X,
	MPU6500_SCAN_ACCEL_Y,
	MPU6500_SCAN_ACCEL_Z,
	MPU6500_SCAN_TEMP,
	MPU6500_SCAN_GYRO_X,
	MPU6500_SCAN_GYRO_Y,
	MPU6500_SCAN_GYRO_Z,
	MPU6500_SCAN_TIMESTAMP,
};

/*Device Data Struct*/
struct mpu6500_data 
{
	struct device *dev;
	struct mutex lock;		//Mutex Lock
	struct regmap *regmap;

    	struct mpu6500_chip_info *chip_info;

	s32 t_fine;
};


struct mpu6500_chip_info {
    	unsigned int id_reg;            //Chip id reg addr
    	const u8 chip_id;              //Chip id

    	const struct regmap_config *regmap_config;      //regmap configurations
   	const struct iio_chan_spec *channels;           //iio spec channels
    	const int num_channels;         //Channel count                      
};


static const struct reg_sequence mpu6500_init_seq[] = {
	{ MPU6500_REG_PWR_MGMT1, MPU_PWR_RST, 100000 },		//PWM_MGMT1: Reset, wait 100ms
	{ MPU6500_REG_PWR_MGMT1, MPU_PWR_AUTO_CLK0, 0 },	//PWM_MGMT1: Wake up, Set clock sources to auto
	{ MPU6500_REG_SMPLRT_DIV, 0x07, 0 },			//SMPLRT_DIV: Set divisor as 7: 125Hz sampling rate
	{ MPU6500_REG_CONFIG, 0x03, 0 },			//CONFIG_DLPF 41Hz low pass filter for stabilitys
	// { MPU6500_REG_INT_PIN_CFG, 0x20, 0 },			//Latch mode
	// { MPU6500_REG_INT_EN, 0x01, 0 }, 			//Enable data ready interrupt
};