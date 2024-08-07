#include <linux/module.h>  
#include <linux/platform_device.h>  
#include <linux/uio_driver.h>  
#include <linux/slab.h> 
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

 
struct rk3399_uio_dev {
        struct uio_info *info;
        dma_addr_t ddr_paddr;
        void *ddr_vaddr;
};

struct rk3399_uio_dev *gdev;
  
static int rk3399_uio_probe(struct device *dev);  
static int rk3399_uio_remove(struct device *dev);  



#define CHARNAME                "rk_misc"

static ssize_t rk_char_read(struct file *file, char __user *buffer,
                                size_t count, loff_t *pos)
{
       int bytes_not_copied;

	bytes_not_copied = copy_to_user(buffer, gdev->ddr_vaddr, 64*1024);
 	if (bytes_not_copied < 0)
           return bytes_not_copied;

        return 64*1024;
}

#if 0
static int rk_char_open(struct inode *inode, struct file *filp)
{
        struct pti_masterchannel *mc;

        /*
         * We really do want to fail immediately if
         * pti_request_masterchannel() fails,
         * before assigning the value to filp->private_data.
         * Slightly easier to debug if this driver needs debugging.
         */
        mc = pti_request_masterchannel(0, NULL);
        if (mc == NULL)
                return -ENOMEM;
        filp->private_data = mc;
        return 0;
}
#endif

static int rk_char_release(struct inode *inode, struct file *filp)
{
#if 0
        pti_release_masterchannel(filp->private_data);
        filp->private_data = NULL;
#endif
        return 0;
}


static const struct file_operations rk_char_driver_ops = {
        .owner          = THIS_MODULE,
        .read           = rk_char_read,
        .open           = simple_open,
        .release        = rk_char_release,
};

static struct miscdevice rk_char_driver = {
        .minor          = MISC_DYNAMIC_MINOR,
        .name           = CHARNAME,
        .fops           = &rk_char_driver_ops
};




static struct device_driver rk3399_uio_driver = {  
        .name ="rk3399 uio", 
        .bus =&platform_bus_type,  
        .probe =rk3399_uio_probe,  
        .remove =rk3399_uio_remove,  
};  
  
static int rk3399_uio_probe(struct device *dev)  
{  
	struct uio_info *info;
	size_t extram_pool_size = SZ_128M;
        //struct rk3399_uio_dev *gdev;
        int ret = -ENODEV;
        //struct device *dev = &pdev->dev;
	 //void *buf = NULL;
   	 //dma_addr_t buf_phy = 0;


	printk("misc register \n");
	ret = misc_register(&rk_char_driver);
        if (ret) {
		 pr_err("misc: %s(%d): Error value returned: %d\n",
                        __func__, __LINE__, ret);
                return -1;
        }

	 of_dma_configure(dev, dev->of_node, true);

	 printk("rk3399_uio_probe 1\n");
	gdev = kzalloc(sizeof(struct rk3399_uio_dev), GFP_KERNEL);
        if (!gdev)
                return -ENOMEM;

	 printk("rk3399_uio_probe 2\n");
	info = kzalloc(sizeof(struct uio_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	dev_info(dev, "Allocated TX Buffer for channel\n");
	
	printk("sz 64M\n");
	gdev->ddr_vaddr = dma_alloc_coherent(dev, extram_pool_size, &(gdev->ddr_paddr), GFP_KERNEL);
        if (!gdev->ddr_vaddr) {
                dev_err(dev, "Could not allocate external memory\n");
                //goto out_free;
        }

	printk("rk_uio: ddr_vaddr = %p  ddr_paddr = 0x%llx\n", gdev->ddr_vaddr, gdev->ddr_paddr);
	
       info->name = "rk3399 uio";
       info->version = "0.0.1";
       info->irq = UIO_IRQ_NONE;

       info->mem[0].name = "rk3399 uio pci desc ";
       info->mem[0].addr= gdev->ddr_paddr;  
       if(info->mem[0].addr == 0)  
               return -ENOMEM;  

       info->mem[0].memtype = UIO_MEM_PHYS;  
       info->mem[0].size= extram_pool_size;  
  
       if(uio_register_device(dev, info))  
       		return -ENODEV;  

        return 0;  
}  

static int rk3399_uio_remove(struct device *dev)  
{  

   return 0;  
}  
  
//static struct platform_device * rk3399_uio_device;  

static struct platform_device rk3399_uio_device = {
        .name = "rk3399 uio",
        .id   = 0,
   	.dev = {
//                .dma_mask = DMA_BIT_MASK(32),
                .coherent_dma_mask = DMA_BIT_MASK(32),
        },

};

#if 0
static struct platform_device mic_platform_dev = {
        .name = mic_driver_name,
        .id   = 0,
        .num_resources = 0,
        .dev = {
                .dma_mask = &mic_dma_mask,
                .coherent_dma_mask = DMA_BIT_MASK(64),
        },
};
#endif
  
static int __init rk3399_uio_init(void)  
{  
	int ret;
        //rk3399_uio_device = platform_device_register_simple("rk3399 uio", -1, NULL, 0);  
	
	ret = platform_device_register(&rk3399_uio_device);
        if (ret) {
                pr_err("platform_device_register ret %d\n", ret);
        }

  
        return driver_register(&rk3399_uio_driver); 
}  
  
static void __exit rk3399_uio_exit(void)  
{  
       platform_device_unregister(&rk3399_uio_device);  
       driver_unregister(&rk3399_uio_driver);  
}  
  
module_init(rk3399_uio_init);  
module_exit(rk3399_uio_exit);  
  
MODULE_LICENSE("GPL");  
MODULE_DESCRIPTION("UIO dummy driver"); 

