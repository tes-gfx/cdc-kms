/*
 * altera_pll.h  --  CDC Display Controller Altera PLL driver
 *
 * Copyright (C) 2016 TES Electronic Solutions GmbH
 * Author: Christian Thaler <christian.thaler@tes-dst.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __ALTERA_PLL_H__
#define __ALTERA_PLL_H__

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>


struct altera_pll {
  struct clk_hw hw;

  struct device *dev;
  unsigned long rate;
  void __iomem  *mmio;
};

#define to_altera_pll(p) container_of(p, struct altera_pll, hw)


struct clk* altera_pll_clk_create(struct device *dev, struct device_node *node);

#endif /* __ALTERA_PLL_H__ */
