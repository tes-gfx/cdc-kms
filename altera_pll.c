/*
 * altera_pll.c  --  CDC Display Controller Altera PLL driver
 *
 * Copyright (C) 2016 TES Electronic Solutions GmbH
 * Author: Christian Thaler <christian.thaler@tes-dst.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/bug.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/of_address.h>

#include "altera_pll.h"


/* Register defines */
#define ALTERA_PLL_REGIDX_MODE        (0)
#define ALTERA_PLL_REGIDX_START       (2)
#define ALTERA_PLL_REGIDX_COUNT_N     (3)
#define ALTERA_PLL_REGIDX_COUNT_M     (4)
#define ALTERA_PLL_REGIDX_COUNT_C     (5)
#define ALTERA_PLL_REGIDX_BANDWITH    (8)
#define ALTERA_PLL_REGIDX_CHARGE_PUMP (9)

/* Register masks */
#define COUNTER_BYPASS_ENABLE         (1<<16)
#define COUNTER_ODD_DIVIDE_ENABLE     (1<<17)


struct pll_config {
  unsigned long rate;
  u32 count_m;
  u32 count_n;
  u32 count_c;
  u32 bandwith;
  u32 charge_pump;
};

/* The configurations are valid for f_in of 50 MHz only */
struct pll_config configs[] = {
    /* 25.2 MHz */
    {
        .rate        = 25200000,
        .count_m     = 0x201f | COUNTER_ODD_DIVIDE_ENABLE,
        .count_n     = 0x0302 | COUNTER_ODD_DIVIDE_ENABLE,
        .count_c     = 0x0d0c | COUNTER_ODD_DIVIDE_ENABLE,
        .bandwith    = 0x7,
        .charge_pump = 0x1,
    },
    /* 40 MHz */
    {
        .rate        = 40000000,
        .count_m     = 0x1010,
        .count_n     = 0x0302 | COUNTER_ODD_DIVIDE_ENABLE,
        .count_c     = 0x0404,
        .bandwith    = 0x7,
        .charge_pump = 0x1,
    },
    /* 65 MHz */
    {
        .rate        = 65000000,
        .count_m     = 0x0706 | COUNTER_ODD_DIVIDE_ENABLE,
        .count_n     = 0x0101,
        .count_c     = 0x0302,
        .bandwith    = 0x7,
        .charge_pump = 0x2,
    },
    /* 108 MHz */
    {
        .rate        = 108000000,
        .count_m     = 0x1b1b,
        .count_n     = 0x0302 | COUNTER_ODD_DIVIDE_ENABLE,
        .count_c     = 0x0302 | COUNTER_ODD_DIVIDE_ENABLE,
        .bandwith    = 0x6,
        .charge_pump = 0x1,
    },
    /* 154 MHz */
    {
        .rate        = 154000000,
        .count_m     = 0x2726 | COUNTER_ODD_DIVIDE_ENABLE,
        .count_n     = 0x0302 | COUNTER_ODD_DIVIDE_ENABLE,
        .count_c     = 0x0302 | COUNTER_ODD_DIVIDE_ENABLE,
        .bandwith    = 0x4,
        .charge_pump = 0x1,
    },
    /* 172.78 MHz */
    {
        .rate        = 172780000,
        .count_m     = 0x3d3c | COUNTER_ODD_DIVIDE_ENABLE,
        .count_n     = 0x0403 | COUNTER_ODD_DIVIDE_ENABLE,
        .count_c     = 0x0302 | COUNTER_ODD_DIVIDE_ENABLE,
        .bandwith    = 0x4,
        .charge_pump = 0x1,
    }
};


static void write_reg32(struct altera_pll *pll, int idx, u32 val)
{
  iowrite32(val, pll->mmio + (idx * 4));
}


static void write_config(struct altera_pll *pll, const struct pll_config *config)
{
  dev_dbg(pll->dev, "Setting up PLL to %lu Hz\n", config->rate);

  write_reg32(pll, ALTERA_PLL_REGIDX_MODE, 0x0); // Set waitrequest mode

  write_reg32(pll, ALTERA_PLL_REGIDX_COUNT_M,     config->count_m);
  write_reg32(pll, ALTERA_PLL_REGIDX_COUNT_N,     config->count_n);
  write_reg32(pll, ALTERA_PLL_REGIDX_COUNT_C,     config->count_c);
  write_reg32(pll, ALTERA_PLL_REGIDX_BANDWITH,    config->bandwith);
  write_reg32(pll, ALTERA_PLL_REGIDX_CHARGE_PUMP, config->charge_pump);


  write_reg32(pll, ALTERA_PLL_REGIDX_START, 0x1); // Start reconfiguration
}


static const struct pll_config* find_config(unsigned long rate)
{
  int i;
  const struct pll_config *config = NULL;

  for(i=0; i < ARRAY_SIZE(configs); ++i)
  {
    if(rate == configs[i].rate)
      config = &configs[i];
  }

  return config;
}


/*********************/
/* clk_ops functions */
/*********************/

unsigned long altera_pll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
  struct altera_pll *pll = to_altera_pll(hw);
  dev_dbg(pll->dev, "ALTERA_PLL: %s = %lu\n", __func__, pll->rate);

  /* todo: In case of a reconfigurable PLL, we should get this from the hardware */

  return pll->rate;
}


long altera_pll_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
  struct altera_pll *pll = to_altera_pll(hw);
  const struct pll_config *config;
  dev_dbg(pll->dev, "ALTERA_PLL: %s(%lu)\n", __func__, rate);

  /* Return initial rate if reconfiguration is not available */
  if(pll->mmio == NULL)
  {
    if(rate == pll->rate)
      return rate;
    else
      return -1;
  }

  config = find_config(rate);
  if(config != NULL)
  {
    return config->rate;
  }

  return -1;
}


int altera_pll_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
  struct altera_pll *pll = to_altera_pll(hw);
  const struct pll_config *config;
  dev_dbg(pll->dev, "ALTERA_PLL: %s\n", __func__);

  /* Skip if reconfiguration is not available */
  if(pll->mmio == NULL)
  {
    return -1;
  }

  config = find_config(rate);
  if(config != NULL)
  {
    write_config(pll, config);
    pll->rate = config->rate;

    return 0;
  }

  return -1;
}


int altera_pll_enable(struct clk_hw *hw)
{
  struct altera_pll *pll = to_altera_pll(hw);
  dev_dbg(pll->dev, "ALTERA_PLL: %s\n", __func__);

  return 0;
}


void altera_pll_disable(struct clk_hw *hw)
{
  struct altera_pll *pll = to_altera_pll(hw);
  dev_dbg(pll->dev, "ALTERA_PLL: %s\n", __func__);
}


static struct clk_ops altera_pll_reconf_ops = {
    .enable = altera_pll_enable,
    .disable = altera_pll_disable,
    .set_rate = altera_pll_set_rate,
    .recalc_rate = altera_pll_recalc_rate,
    .round_rate = altera_pll_round_rate,
};


static struct clk* altera_pll_register_clk(struct altera_pll *pll, const char *name)
{
  struct clk *clk;
  struct clk_init_data init;

  init.name  = name;
  init.ops = &altera_pll_reconf_ops;
  init.flags = 0;
  init.num_parents = 0;
  init.parent_names = NULL;
  pll->hw.init = &init;

  dev_dbg(pll->dev, "Registering clock %s...\n", init.name);
  clk = devm_clk_register(pll->dev, &pll->hw);
  if(WARN_ON(IS_ERR(clk))) {
    dev_err(pll->dev, "Registering clock failed!\n");
    return NULL;
  }

  return clk;
}

struct clk* altera_pll_clk_create(struct device *dev, struct device_node *node)
{
  struct altera_pll *pll = NULL;
  struct clk *clk;
  struct device_node *reconf_node;
  struct device_node *slcr;
  struct resource res;
  u32 rate;

  pll = devm_kzalloc(dev, sizeof(*pll), GFP_KERNEL);
  if(pll == NULL)
  {
    dev_err(dev, "failed to allocate pll data\n");
    return NULL;
  }

  pll->dev = dev;
  pll->mmio = NULL;

  /* Get initial clock rate */
  if(of_property_read_u32(node, "clock-frequency", &rate))
  {
    dev_err(dev, "Failed to get PLL's initial frequency\n");
  }
  pll->rate = rate;

  /* Now, check if we have a reconf core for the PLL available */
  reconf_node = of_parse_phandle(node, "pll-reconfig", 0);
  if(reconf_node)
  {
    dev_dbg(dev, "Found PLL reconfiguration core\n");

    slcr = of_get_parent(reconf_node);
    if(of_address_to_resource(reconf_node, 0, &res))
    {
      dev_err(dev, "Failed to get PLL reconfiguration resource\n");
    }
    dev_dbg(dev, "Found PLL reconfiguration core @%p\n", slcr->data + res.start);

    pll->mmio = devm_ioremap_resource(dev, &res);
    if(IS_ERR(pll->mmio))
    {
      dev_err(dev, "Failed to map resource\n");
      return NULL;
    }

    dev_dbg(dev, "Mapped PLL reconfiguration core IO from 0x%x to 0x%p\n", res.start, pll->mmio);
  }

  clk = altera_pll_register_clk(pll, node->name);

  return clk;
}
