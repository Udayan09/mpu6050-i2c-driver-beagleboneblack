#define pr_fmt(fmt) "mpu6500: " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/regmap.h>
#include <linux/interrupt.h>
#include <linux/iio/triggered.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/trigger_consumer.h>

#include "mpu6500-udayan.h"



/*IIO Channel specification definition*/

static const struct iio_chan_spec mpu6500_channels[] = {
	{
		.type = IIO_ACCEL,
		.modified = 1,
		.channel2 = IIO_MOD_X,
		.scan_index = MPU6500_SCAN_ACCEL_X,
		.scan_type = {
			.sign = 's',
			.realbits = 16,
			.storagebits = 16,
			.endianness = IIO_BE,
		},
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
		.address = MPU6500_REG_ACCEL_X_OUT,
	},
	{
		.type = IIO_ACCEL,
		.modified = 1,
		.channel2 = IIO_MOD_Y,
		.scan_index = MPU6500_SCAN_ACCEL_Y,
		.scan_type = {
			.sign = 's',
			.realbits = 16,
			.storagebits = 16,
			.endianness = IIO_BE,
		},
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
		.address = MPU6500_REG_ACCEL_Y_OUT,
	},
	{
		.type = IIO_ACCEL,
		.modified = 1,
		.channel2 = IIO_MOD_Z,
		.scan_index = MPU6500_SCAN_ACCEL_Z,
		.scan_type = {
			.sign = 's',
			.realbits = 16,
			.storagebits = 16,
			.endianness = IIO_BE,
		},
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
		.address = MPU6500_REG_ACCEL_Z_OUT,
	},
	{
		.type = IIO_TEMP,
		.scan_index = MPU6500_SCAN_TEMP,
		.scan_type = {
			.sign = 's',
			.realbits = 16,
			.storagebits = 16,
			.endianness = IIO_BE,
		},
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_OFFSET),
		.address = MPU6500_REG_TEMP_OUT,
	},
	{
		.type = IIO_ANGL_VEL,
		.modified = 1,
		.channel2 = IIO_MOD_X,
		.scan_index = MPU6500_SCAN_GYRO_X,
		.scan_type = {
			.sign = 's',
			.realbits = 16,
			.storagebits = 16,
			.endianness = IIO_BE,
		},
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
		.address = MPU6500_REG_GYRO_X_OUT,
	},
	{
		.type = IIO_ANGL_VEL,
		.modified = 1,
		.channel2 = IIO_MOD_Y,
		.scan_index = MPU6500_SCAN_GYRO_Y,
		.scan_type = {
			.sign = 's',
			.realbits = 16,
			.storagebits = 16,
			.endianness = IIO_BE,
		},
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
		.address = MPU6500_REG_GYRO_Y_OUT,
	},
	{
		.type = IIO_ANGL_VEL,
		.modified = 1,
		.channel2 = IIO_MOD_Z,
		.scan_index = MPU6500_SCAN_GYRO_Z,
		.scan_type = {
			.sign = 's',
			.realbits = 16,
			.storagebits = 16,
			.endianness = IIO_BE,
		},
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),
		.address = MPU6500_REG_GYRO_Z_OUT,
	},
	IIO_CHAN_SOFT_TIMESTAMP(MPU6500_SCAN_TIMESTAMP),
};


/*mpu6500 Functions*/

/*Chip config function*/
static int mpu6500_chip_config(struct mpu6500_data *data){
	int ret;

	ret = regmap_multi_reg_write(data->regmap, mpu6500_init_seq, ARRAY_SIZE(mpu6500_init_seq));

	if (ret){
		dev_err(data->dev, "Failed to write init sequence!\n");
		return ret;
	}

	dev_info(data->dev, "Config succesful\n");

	return 0;
}


/*mpu6500 IIO Functions*/

/*Read raw in iio*/
static int mpu6500_read_raw(struct iio_dev *indio_dev,
                           struct iio_chan_spec const *chan,
                           int *val, int *val2, long mask){
	
	struct mpu6500_data *data = iio_priv(indio_dev);
	int ret;
	__be16 raw_val;
	unsigned int pwr_val;

	regmap_read(data->regmap, MPU6500_REG_PWR_MGMT1, &pwr_val);
	dev_info(data->dev, "PWR REG Value: %u", pwr_val);

	guard(mutex)(&data->lock);
	
	switch (mask)
	{
	case IIO_CHAN_INFO_RAW:

		ret = regmap_bulk_read(data->regmap, chan->address, &raw_val, sizeof(raw_val));
		if (ret){
			dev_err(data->dev, "Failed to read sensor value");
			return ret;
		}

		*val = be16_to_cpu(raw_val);
		return IIO_VAL_INT;
	
	case IIO_CHAN_INFO_SCALE:
		switch (chan->type)	
		{
		case IIO_ACCEL:
			*val = 0;
			*val2 = 598550;
			return IIO_VAL_INT_PLUS_MICRO;
		
		case IIO_ANGL_VEL:
			*val = 0;
			*val2 = 133232;
			return IIO_VAL_INT_PLUS_MICRO;
			
		case IIO_TEMP:
			*val = 2;
			*val2 = 995177;
			return IIO_VAL_INT_PLUS_MICRO;

		default:
			return -EINVAL;
		}
	
	case IIO_CHAN_INFO_OFFSET:
		if (chan->type == IIO_TEMP) {
			*val = 7011;
			return IIO_VAL_INT;
		}
		return -EINVAL;

	default:
		return -EINVAL;
	}
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
static bool mpu6500_is_volatile_reg(struct device *dev, unsigned int reg)
{
	if ((reg >= 0x3B) || (reg >= 0x48))
		return true;
	else	
		return false;
}

/*regmap config for mpu6500*/
const struct regmap_config mpu6500_regmap_config = {
	.reg_bits = 8,		//Register addr size = 8bits
	.val_bits = 8,		//Register Value size = 8bits		

	.max_register = MPU6500_REG_WHO_AM_I,
	.volatile_reg = mpu6500_is_volatile_reg,

};

const struct mpu6500_chip_info mpu6500_chip_info = {
	.id_reg = MPU6500_REG_WHO_AM_I,
	.chip_id = MPU6500_WHOAMI_VALUE,

	.regmap_config = &mpu6500_regmap_config,
	.channels = mpu6500_channels,
	.num_channels = 8,
};

/*Interrupt Code*/

/*Bottom Half: IRQ Thread. Can sleep*/
static irqreturn_t mpu6500_trigger_handler(int irq, void *p){
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct mpu6500_data *data = iio_priv(indio_dev);
	int ret;

	guard(mutex)(&data->lock);		//Lock mutex to block access during interrupt handling

	//Bulk read starting from ACCEL_X_OUT register, Autoincrements register pointer and stores byte wise into scan.channels
	ret = regmap_bulk_read(data->regmap, MPU6500_REG_ACCEL_X_OUT, &data->scan.channels, sizeof(data->scan.channels));

	if (ret) {
		dev_err(data->dev, "Failed to read sensor data in trigger\n");
	}
	else{
		iio_push_to_buffers_with_timestamp(indio_dev, &data->scan, pf->timestamp);	//Pushes sensor data read along with timestamp to IIO subsystem (KFIFO ring buffer)
	}

	iio_trigger_notify_done(indio_dev->trig);

	return IRQ_HANDLED;
}


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


	ret = mpu6500_chip_config(data);
	if (ret)
		return ret;

	if (client->irq > 0) {

		data->trig = devm_iio_trigger_alloc(dev, "%s-dev%d", indio_dev->name, iio_device_id(indio_dev));	//Allocate iio trigger
		if (!data->trip) {
			dev_err(dev, "IIO Trigger not allocated\n");
			return -ENOMEM;
		}

		data->trig->dev.parent = dev;
		iio_trigger_set_drvdata(data->trig, indio_dev);		//Adds a indio dev reference to the iio trig struct

		ret = devm_request_threaded_irq(dev, client->irq,			//Requests IRQ Thread. Runs top half and passes bottom half work to iio
						iio_trigger_generic_data_rdy_poll,
						NULL,
						IRQF_TRIGGER_RISING | IRQF_ONESHOT,	//Rising trigger and one shot blocks pin until iio notifies done
						data->trig->name,
						data->trig);

		if (ret) {
			dev_err(dev, "Failed to request IRQ %d\n",client->irq);
			return ret;
		}

		ret = devm_iio_trigger_register(dev, data->trig);		//Registers trigger with kernel
		if (ret) {
			dev_err(dev, "Failed to register trigger with device\n");
			return ret;
		}

		indio_dev->trig = iio_trigger_get(data->trig);			//Set device default trigger

		ret = devm_iio_triggered_buffer_setup(dev, indio_dev, iio_pollfunc_store_time, mpu6500_trigger_handler, NULL);	//Sets up KFIFO tied to this device
		if (ret) {
			dev_err(dev, "Failed t setup triggered buffer\n");
			return ret;
		}

	}
	else {
		dev_info(dev, "No IRQ found in device tree, polling mode only\n");
	}

	// ret = mpu6500_read_calib(data);
	// if (ret)
	// 	return ret;

	//Stores device private data
	dev_set_drvdata(dev, indio_dev);		

	dev_info(dev, "mpu6500 Probe Complete\n");

	
	return devm_iio_device_register(dev, indio_dev);
}


/*Device Tree Match Table*/
static const struct of_device_id mpu6500_of_i2c_match[] = {
	{ .compatible = "invensense,mpu6500", .data = &mpu6500_chip_info },
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