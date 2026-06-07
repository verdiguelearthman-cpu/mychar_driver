//头文件
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
//全局变量
#define DEVICE_NAME "mychar"  //设备名称
#define BUF_SIZE    1024

static dev_t dev_num;  //设备号
static struct cdev my_cdev;
static struct class* my_class;

static char kernel_buf[BUF_SIZE];
static int data_len = 0;

//四个操作函数
static int my_open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "[mychar] 设备被打开\n");
	return 0;
}

static ssize_t my_read(struct file* filp, char __user* buf, size_t count, loff_t* ppos)
{
	int ret;
    //偏移位置已经超过数据长度,无数据可读
	if (*ppos >= data_len)
		return 0;
    //读的字节数超过了剩余数据，就只读剩余的部分
	if (count > data_len - *ppos)
		count = data_len - *ppos;

	ret = copy_to_user(buf, kernel_buf + *ppos, count);
	if (ret != 0) {
		printk(KERN_ERR "[mychar] copy_to_user 失败，还剩 %d 字节未拷贝\n", ret);
		return -EFAULT;  //-EFAULT 表示地址错误 
	}
	//更新偏移位置
	*ppos += count;

	printk(KERN_INFO "[mychar] 用户读取了 %zu 字节\n", count);
	return count;
}

static ssize_t my_write(struct file* filp, const char __user* buf,
    size_t count, loff_t* ppos)
{
    int ret;

    //如果用户想写的数据超过了缓冲区大小，就截断
    if (count > BUF_SIZE)
        count = BUF_SIZE;

    ret = copy_from_user(kernel_buf, buf, count);
    if (ret != 0) {
        printk(KERN_ERR "[mychar] copy_from_user 失败，还剩 %d 字节未拷贝\n", ret);
        return -EFAULT;
    }

	//记录缓冲区还有多少数据
    data_len = count;

    printk(KERN_INFO "[mychar] 用户写入了 %zu 字节\n", count);
    return count;
}

static int my_release(struct inode* inode, struct file* filp)
{
	printk(KERN_INFO "[mychar] 设备被关闭\n");
	return 0;
}

//文件操作结构体，打包四个函数
static struct file_operations my_fops = {
    .owner = THIS_MODULE,     /* 标记这个模块的所有者，防止模块在使用中被卸载 */
    .open = my_open,
    .read = my_read,
    .write = my_write,
    .release = my_release,
};

//驱动加载函数
static int __init my_init(void)
{
    int ret;

    //申请设备号
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    //自动分配一个没被占用的主设备号，存进dev_num
    //次设备号从0开始
    //只申请一个设备号
    if (ret < 0) 
    {
        printk(KERN_ERR "[mychar] 申请设备号失败\n");
        return ret;
    }
    printk(KERN_INFO "[mychar] 申请到设备号：主=%d，次=%d\n",
        MAJOR(dev_num), MINOR(dev_num));

    //初始化并注册字符设备
    cdev_init(&my_cdev, &my_fops);
    my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) 
    {
        printk(KERN_ERR "[mychar] 注册字符设备失败\n");
        goto fail_cdev;  //失败了则回滚
    }

    //创建设备类和设备节点
    my_class = class_create(DEVICE_NAME);
    if (IS_ERR(my_class)) 
    {
        printk(KERN_ERR "[mychar] 创建设备类失败\n");
        ret = PTR_ERR(my_class);
        goto fail_class;
    }

    device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "[mychar] 驱动加载成功！设备文件：/dev/%s\n", DEVICE_NAME);
    return 0;

fail_class:
    cdev_del(&my_cdev);
fail_cdev:
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

//驱动卸载函数
static void __exit my_exit(void)
{
    //怎么安装的就怎么倒着卸载
    device_destroy(my_class, dev_num);       
    class_destroy(my_class);                 
    cdev_del(&my_cdev);                      
    unregister_chrdev_region(dev_num, 1);    

    printk(KERN_INFO "[mychar] 驱动卸载成功\n");
}

//模块信息声明
module_init(my_init);                   
module_exit(my_exit);                   

MODULE_LICENSE("GPL");                 //可证声明，必须是 GPL，否则内核会报警告
MODULE_AUTHOR("Student");              //作者信息
MODULE_DESCRIPTION("一个简单的字符设备驱动示例");  
