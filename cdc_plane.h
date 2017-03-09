/*
 * cdc_plane.h  --  CDC Display Controller Plane
 *
 * Copyright (C) 2017 TES Electronic Solutions GmbH
 * Author: Christian Thaler <christian.thaler@tes-dst.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CDC_PLANE_H_
#define CDC_PLANE_H_

int cdc_planes_init(struct cdc_device *cdc);
void cdc_plane_compute_base(struct cdc_plane *plane, struct drm_framebuffer *fb);
int cdc_plane_disable(struct drm_plane *plane);
void cdc_plane_setup(struct cdc_plane *plane);
void cdc_plane_update_base(struct cdc_plane *plane);

#endif /* CDC_PLANE_H_ */
