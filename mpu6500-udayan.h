/*Register Addresses*/

/*Config Registers*/
#define MPU6500_REG_CONFIG          0x1A
#define MPU6500_REG_GYRO_CFG        0x1B
#define MPU6500_REG_ACCEL_CFG       0x1C
#define MPU6500_REG_ACCEL_CFG2      0x1D

/*Interrupt Registers*/
#define MPU6500_REG_INT_PIN_CFG     0x37
#define MPU6500_REG_INT_EN          0x38
#define MPU6500_REG_INT_STATUS      0x3A

/*Power Management Registers*/
#define MPU6500_REG_PWR_MGMT1       0x6B
#define MPU6500_REG_PWR_MGMT2       0x6C

/*Fifo Registers*/
#define MPU6500_REG_FIFO_EN         0x23
#define MPU6500_REG_FIFO_COUNT_H    0x72
#define MPU6500_REG_FIFO_COUNT_L    0x73
#define MPU6500_REG_FIFO_RW         0x74

/*Who am i Register (For identification)*/
#define MPU6500_REG_WHO_AM_I        0x75
#define MPU6500_WHOAMI_VALUE	    0x70




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
