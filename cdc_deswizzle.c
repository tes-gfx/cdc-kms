#include <linux/module.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#define DSWZ_REG_ID       0x00
#define DSWZ_REG_MODE     0x01
#define DSWZ_REG_FB_ADDR  0x02
#define DSWZ_REG_FB_DIM   0x03
#define DSWZ_REG_FB_PITCH 0x04

struct dswz_device {
	struct device *dev;

	void __iomem *mmio;
};

static u32 dswz_read_reg (struct dswz_device *dswz, u32 reg)
{
	return ioread32(dswz->mmio + (reg * 4));
}

static void dswz_write_reg(struct dswz_device *dswz, u32 reg, u32 val)
{
	iowrite32(val, dswz->mmio + (reg * 4));
}

void dswz_set_mode(struct dswz_device *dswz, int mode)
{
	dswz_write_reg(dswz, DSWZ_REG_MODE, mode);
}

void dswz_set_fb_addr(struct dswz_device *dswz, u32 addr)
{
	dswz_write_reg(dswz, DSWZ_REG_FB_ADDR, addr);
}

void dswz_set_fb_config(struct dswz_device *dswz, u16 width, u16 height, u32 pitch, u8 bpp)
{
	dswz_write_reg(dswz, DSWZ_REG_FB_DIM, (height << 16) | width);
	dswz_write_reg(dswz, DSWZ_REG_FB_PITCH, (bpp << 24) | pitch);
}

static int dswz_remove (struct platform_device *pdev)
{
	return 0;
}

static int dswz_probe (struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct dswz_device *dswz;
	struct resource *mem;
	u32 reg_id;

	if (np == NULL) {
		dev_err(&pdev->dev, "no platform data\n");
		return -ENODEV;
	}

	dswz = devm_kzalloc(&pdev->dev, sizeof(*dswz), GFP_KERNEL);
	if (dswz == NULL) {
		dev_err(&pdev->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, dswz);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dswz->mmio = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(dswz->mmio)) {
		return PTR_ERR(dswz->mmio);
	}
	dev_dbg(&pdev->dev, "Mapped IO from 0x%x to 0x%p\n", mem->start,
		dswz->mmio);

	reg_id = dswz_read_reg(dswz, DSWZ_REG_ID);
	dev_info(&pdev->dev, "TES Deswizzler rev. %x\n", reg_id);

	return 0;
}

static const struct of_device_id dswz_of_table[] = {
	{ .compatible = "tes,dswz"},
	{ /* end node */ }
};
MODULE_DEVICE_TABLE(of, dswz_of_table);

struct platform_driver dswz_driver = {
	.probe = dswz_probe,
	.remove = dswz_remove,
	.driver = {
		.name = "tes-dswz",
		.of_match_table = dswz_of_table,
	},
};
