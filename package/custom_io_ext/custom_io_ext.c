#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>      // For character device
#include <linux/uaccess.h> // For copying data between user and kernel space
#include <linux/cdev.h>    // For character device registration
#include <linux/device.h>  // For automatically creating device nodes
#include <linux/of.h>      // For device tree support

#define DEVICE_NAME "custom_io_ext"
#define CLASS_NAME "custom_io"

// Define DEBUG_MODE to enable or disable debug output
#define DEBUG_MODE 1

#if DEBUG_MODE
#define DEBUG_PRINTK(fmt, args...) printk(KERN_INFO fmt, ##args)
#else
#define DEBUG_PRINTK(fmt, args...) /* no debugging: nothing */
#endif

static int majorNumber;
static struct class *custom_io_class = NULL;
static struct device *custom_io_device = NULL;
static struct i2c_client *custom_io_client; // Global I2C client pointer

// Open device
static int custom_io_open(struct inode *inodep, struct file *filep)
{
    DEBUG_PRINTK("Custom IO: Device has been opened\n");
    return 0;
}

// Close device
static int custom_io_release(struct inode *inodep, struct file *filep)
{
    DEBUG_PRINTK("Custom IO: Device successfully closed\n");
    return 0;
}

// Read data from device
static ssize_t custom_io_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    char data[256]; // Temporary data buffer
    int ret;
    int i;
    DEBUG_PRINTK("Custom IO: Read requested for %zu bytes\n", len);

    // Read data from I2C device
    ret = i2c_master_recv(custom_io_client, data, len);
    if (ret < 0)
    {
        printk(KERN_ERR "Custom IO: Failed to read from I2C device, error: %d\n", ret);
        return ret;
    }

    // Print the data read from the device
    DEBUG_PRINTK("Custom IO: Read %d bytes from I2C device: ", ret);
    for (i = 0; i < ret; i++)
    {
        DEBUG_PRINTK(KERN_CONT "0x%02x ", data[i]);
    }
    DEBUG_PRINTK(KERN_CONT "\n");

    // Copy data to user space
    if (copy_to_user(buffer, data, len))
    {
        printk(KERN_ERR "Custom IO: Failed to send data to user\n");
        return -EFAULT;
    }

    return len;
}

// Write data to device
static ssize_t custom_io_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    char data[256];
    int ret;
    int i;
    DEBUG_PRINTK("Custom IO: Write requested for %zu bytes\n", len);

    // Copy data from user space to kernel space
    if (copy_from_user(data, buffer, len))
    {
        printk(KERN_ERR "Custom IO: Failed to receive data from user\n");
        return -EFAULT;
    }

    // Print the data to be written to the device
    DEBUG_PRINTK("Custom IO: Writing %zu bytes to I2C device: ", len);
    for (i = 0; i < len; i++)
    {
        DEBUG_PRINTK(KERN_CONT "0x%02x ", data[i]);
    }
    DEBUG_PRINTK(KERN_CONT "\n");

    // Write data to I2C device
    ret = i2c_master_send(custom_io_client, data, len);
    if (ret < 0)
    {
        printk(KERN_ERR "Custom IO: Failed to write to I2C device, error: %d\n", ret);
        return ret;
    }

    return len;
}

// file_operations structure
static struct file_operations fops = {
    .open = custom_io_open,
    .read = custom_io_read,
    .write = custom_io_write,
    .release = custom_io_release,
};

// Driver probe function
static int custom_io_ext_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0;
    DEBUG_PRINTK("Custom IO: Starting probe function\n");
    custom_io_client = client; // Save I2C client pointer

    // Register character device
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0)
    {
        printk(KERN_ALERT "Custom IO: Failed to register a major number\n");
        return majorNumber;
    }
    DEBUG_PRINTK("Custom IO: Registered major number: %d\n", majorNumber);

    // Register device class
    custom_io_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(custom_io_class))
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Custom IO: Failed to register device class\n");
        return PTR_ERR(custom_io_class);
    }
    DEBUG_PRINTK("Custom IO: Device class created\n");

    // Create device node
    custom_io_device = device_create(custom_io_class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(custom_io_device))
    {
        class_destroy(custom_io_class);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Custom IO: Failed to create the device\n");
        return PTR_ERR(custom_io_device);
    }
    DEBUG_PRINTK("Custom IO: Device node created\n");

    DEBUG_PRINTK("Custom IO: Probe function completed successfully\n");
    return ret;
}

// Driver remove function
static int custom_io_ext_remove(struct i2c_client *client)
{
    device_destroy(custom_io_class, MKDEV(majorNumber, 0)); // Destroy device node
    class_destroy(custom_io_class);                         // Destroy device class
    unregister_chrdev(majorNumber, DEVICE_NAME);            // Unregister character device

    printk(KERN_INFO "Custom IO: Device class removed\n");
    return 0;
}

// Device tree match table
static const struct of_device_id custom_io_ext_of_match[] = {
    {.compatible = "custom,io-ext"},
    {}};

MODULE_DEVICE_TABLE(of, custom_io_ext_of_match);

// I2C device ID table
static const struct i2c_device_id custom_io_ext_id[] = {
    {"custom_io_ext", 0},
    {}};

// I2C driver structure
static struct i2c_driver custom_io_ext_driver = {
    .driver = {
        .name = "custom_io_ext",
        .of_match_table = of_match_ptr(custom_io_ext_of_match),
    },
    .probe = custom_io_ext_probe,
    .remove = custom_io_ext_remove,
    .id_table = custom_io_ext_id,
};

// Module initialization function
static int __init custom_io_ext_init(void)
{
    printk(KERN_INFO "Custom IO: Initializing the Custom IO Extension Device\n");
    return i2c_add_driver(&custom_io_ext_driver);
}

// Module exit function
static void __exit custom_io_ext_exit(void)
{
    printk(KERN_INFO "Custom IO: Exiting the Custom IO Extension Device\n");
    i2c_del_driver(&custom_io_ext_driver);
}

module_init(custom_io_ext_init);
module_exit(custom_io_ext_exit);

MODULE_AUTHOR("Will");
MODULE_DESCRIPTION("Custom IO Extension Device Driver with read/write support");
MODULE_LICENSE("GPL");
