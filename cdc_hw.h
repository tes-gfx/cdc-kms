/*
 * cdc_hw.h  --  CDC Display Controller Hardware Interface
 *
 * Copyright (C) 2016 TES Electronic Solutions GmbH
 * Author: Christian Thaler <christian.thaler@tes-dst.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CDC_HW_H_
#define CDC_HW_H_

#include <cdc.h>

void cdc_irq_set(struct cdc_device *cdc, cdc_irq_type irq, bool enable);

#endif /* CDC_HW_H_ */
