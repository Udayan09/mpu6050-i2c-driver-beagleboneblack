#define pr_fmt(fmt) "mpu6500: " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/regmap.h>


#include "mpu6500-udayan.h"



/*IIO Channel specification definition*/
static const struct iio_chan_spec mpu6500_channels[] = {
	{
		.type = IIO_ACCEL,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),	
	},
	{
		.type = IIO_ANGL,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),	
	},
};


/*mpu6500 Functions*/

/*Chip config function*/
// static int mpu6500_chip_config(struct mpu6500_data *data){
// 	u8 osrs = FIELD_PREP(mpu6500_OSRS_PRESS_MASK, data->oversampling_pressure) | 
// 		  FIELD_PREP(mpu6500_OSRS_TEMP_MASK, data->oversampling_temp) | 
// 		  mpu6500_MODE_FORCED;

// 	int ret;

// 	ret = regmap_write_bits(data->regmap, mpu6500_REG_CTRL_MEAS, mpu6500_OSRS_TEMP_MASK |
// 				mpu6500_OSRS_PRESS_MASK| mpu6500_MODE_MASK, osrs);

// 	if (ret) {	//ret = 0 for success 
// 		dev_err(data->dev, "failed to write ctrl_meas register\n");
// 		return ret;
// 	}

// 	dev_info(data->dev, "Config succesful\n");

// 	return ret;
// }


/*IIO Read raw implementation for temperature and pressure*/
// static int mpu6500_read_raw_impl(struct iio_dev *indio_dev,
// 				struct iio_chan_spec const *chan,
// 				int *val, int *val2, long mask){
	
// 	struct mpu6500_data *data = iio_priv(indio_dev);
// 	int chan_value;
// 	int ret;

// 	guard(mutex)(&data->lock);		//New api for mutex locking. Auto unlocks

// 	switch (mask)
// 	{
// 	case IIO_CHAN_INFO_RAW:
// 		switch (chan->type)
// 		{
// 		case IIO_TEMP:
// 			ret = mpu6500_chip_config(data);
// 			if (ret) 
// 				return ret;

// 			msleep(100);		//45ms sleep for measurement to complete

// 			ret = mpu6500_read_temp(data, &chan_value);
// 			if (ret)
// 				return ret;
// 			*val = chan_value;
// 			return IIO_VAL_INT;
// 			break;
// 		case IIO_PRESSURE:
// 			ret = mpu6500_chip_config(data);
// 			if (ret) 
// 				return ret;

// 			msleep(100);		//45ms sleep for measurement to complete

// 			ret = mpu6500_read_pressure(data, &chan_value);
// 			if (ret)
// 				return ret;
// 			*val = chan_value;
// 			return IIO_VAL_INT;
// 			break;
// 		default:
// 			return -EINVAL;
// 		}
	
// 	default:
// 		return -EINVAL;
// 	}
// 	return -EINVAL;
// }

/*mpu6500 IIO Functions*/

/*Read raw in iio*/
static int mpu6500_read_raw(struct iio_dev *indio_dev,
                           struct iio_chan_spec const *chan,
                           int *val, int *val2, long mask){
	
	//struct mpu6500_data *data = iio_priv(indio_dev);
	int ret;

	// ret = mpu6500_read_raw_impl(indio_dev, chan, val, val2, mask);

	return 0;
}

static int mpu6500_write_raw(struct iio_dev *indio_dev,
                           struct iio_chan_spec const *chan,
                           int val, int val2, long mask){
	return 0;
}



/*IIO Info struct for mpu6500*/
static const struct iio_info mpu6500_info = {		//Containts const info about iio device such as function pointers
	.read_raw = &mpu6500_read_raw,
	.write_raw = &mpu6500_write_raw,
};


/*Regmap config struct Setup*/
// static bool mpu6500_is_writeable_reg(struct device *dev, unsigned int reg)
// {
// 	switch(reg) {
// 	// case mpu6500_REG_CONFIG:
// 		return true;
// 	default:
// 		return false;
// 	}
// }

// static bool mpu6500_is_volatile_reg(struct device *dev, unsigned int reg)
// {
// 	switch(reg) {
// 	// case mpu6500_REG_TEMP_LSB:
// 		return true;
// 	default:
// 		return false;
// 	}
// }

/*regmap config for mpu6500*/
const struct regmap_config mpu6500_regmap_config = {
	.reg_bits = 8,		//Register addr size = 8bits
	.val_bits = 8,		//Register Value size = 8bits		

	.max_register = mpu6500_REG_ID,
	.cache_type = REGCACHE_RBTREE,			//RB Tree better for non contiguous reg

	// .writeable_reg = mpu6500_is_writeable_reg,
	// .volatile_reg = mpu6500_is_volatile_reg,
};

const struct mpu6500_chip_info mpu6500_chip_info = {
	.id_reg = mpu6500_REG_ID,
	.chip_id = mpu6500_CHIP_ID,

	.regmap_config = &mpu6500_regmap_config,
	.channels = mpu6500_channels,
	.num_channels = 2,
};


/*Driver Probe Function*/
static int mpu6500_i2c_probe(struct i2c_client *client)
{
	int ret;
	struct device *dev = &client->dev;		//Points to current dev

	const struct i2c_device_id *id;			//i2c device id
	const struct mpu6500_chip_info *chip_info;	//mpu6500 private data
	struct iio_dev *indio_dev;			//Industrial io device
	struct mpu6500_data *data;			//mpu6500 device private data
	struct regmap *regmap;				//Regmap pointer
	
	unsigned int chip_id;		//For regmap test
	const char *name;			//Name of device (string)
	
	/*Get device id from id table (used to get device name string)*/
	id = i2c_client_get_device_id(client);
	if (!id){
		dev_err(dev, "No device ID found\n");
		return -ENODEV;
	}
	name = id->name;		//fetch name from id

	/*Getting chip info from of_tree or id_tree*/
	chip_info = i2c_get_match_data(client);			//Get i2c match data. Can be either dt or id table
	if (!chip_info){
		dev_err(dev, "Device not found\n");
		return -ENODEV;	
	}
	
	/*Regmap init with required configuration*/
	regmap = devm_regmap_init_i2c(client, chip_info->regmap_config);	//Inits Regmap for bus type i2c		
	if (IS_ERR(regmap)) {
		dev_err(dev, "failed to allocate register map\n");		//failed to allocate regmap
		return PTR_ERR(regmap);
	}

	/*IIO configuration*/
	indio_dev = devm_iio_device_alloc(dev, sizeof(*data));		//Allocate iio_dev for a driver. Memory is allocateds
	if (!indio_dev)
		return -ENOMEM;

	data = iio_priv(indio_dev);		//Fetches address of device data from allocated memory

	mutex_init(&data->lock);		//Initialise mutex
	data->dev = dev;			//Set device in private data

	indio_dev->name = name;				//Set iio device name
	indio_dev->info = &mpu6500_info;			//contains pointers to certain functions ie. read_raw, write_raw
	indio_dev->modes = INDIO_DIRECT_MODE;		//Single shot read mode

	data->chip_info = chip_info;			//Set private data chip info

	indio_dev->channels = chip_info->channels;		//iio_chan_spec array
	indio_dev->num_channels = chip_info->num_channels;	//Channel count: Temperature and Pressure channel

	data->regmap = regmap;			//Set private data regmap

	/*Attempting to read chip id*/
	ret = regmap_read(regmap, chip_info->id_reg, &chip_id);		//Reads value at mpu6500_CHIP_ID and stores in chip_id
	if (ret) {
		dev_err(data->dev, "failed to read chip id\n");
		return ret;
	}

	if (chip_id == data->chip_info->chip_id) 
		dev_info(dev, "0x%x is the correct chip id for %s\n", chip_id, name);
	else
		dev_warn(dev, "bad chip id: 0x%x is not known\n", chip_id);


	// ret = mpu6500_chip_config(data);
	// if (ret)
	// 	return ret;

	
	// ret = mpu6500_read_calib(data);
	// if (ret)
	// 	return ret;

	//Stores device private data
	dev_set_drvdata(dev, indio_dev);		

	dev_info(&client->dev, "mpu6500 Probe Complete\n");

	
	return devm_iio_device_register(dev, indio_dev);
}


/*Device Tree Match Table*/
static const struct of_device_id mpu6500_of_i2c_match[] = {
	{ .compatible = "invensence,mpu6500", .data = &mpu6500_chip_info },
	{ },
};
MODULE_DEVICE_TABLE(of, mpu6500_of_i2c_match);

/*ID Match Table*/
static const struct i2c_device_id mpu6500_i2c_id[] = {
	{"mpu6500", (kernel_ulong_t)&mpu6500_chip_info },
	{ },
};
MODULE_DEVICE_TABLE(i2c, mpu6500_i2c_id);

/*Driver Structure*/
static struct i2c_driver mpu6500_i2c_driver = {
	.driver = {
		.name = "mpu6500-udayan",
		.of_match_table = mpu6500_of_i2c_match,
	},
	.probe          = mpu6500_i2c_probe,
	.id_table       = mpu6500_i2c_id,
};
module_i2c_driver(mpu6500_i2c_driver);

MODULE_AUTHOR("Udayan Borah");
MODULE_DESCRIPTION("mpu6500 Driver");
MODULE_LICENSE("GPL v2");