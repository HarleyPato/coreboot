/*
 * This file is part of the coreboot project.
 *
 *  Copyright 2000  AG Electronics Ltd.
 * Copyright 2003-2004 Linux Networx
 * Copyright 2004 Tyan 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <io.h>
#include <lib.h>
#include <device/device.h>
#include <device/pnp.h>
#include <console.h>
#include <string.h>
//#include <bitops.h>
#include <uart8250.h>
#include <keyboard.h>
// #include <pc80/mc146818rtc.h>
#include <statictree.h>
#include "w83627thg.h"

static void w83627thg_enter_ext_func_mode(struct device * dev) 
{
        outb(0x87, dev->path.pnp.port);
        outb(0x87, dev->path.pnp.port);
}
static void w83627thg_exit_ext_func_mode(struct device * dev) 
{
        outb(0xaa, dev->path.pnp.port);
}

static void w83627thg_init(struct device * dev)
{
	struct superio_winbond_w83627thg_config *conf;
	struct resource *res0, *res1;
	/* Wishlist handle well known programming interfaces more
	 * generically.
	 */
	if (!dev->enabled) {
		return;
	}
	conf = dev->device_configuration;
	switch(dev->path.pnp.device) {
	case W83627THG_SP1: 
		res0 = find_resource(dev, PNP_IDX_IO0);
//		init_uart8250(res0->base, &conf->com1);
		break;
	case W83627THG_SP2:
		res0 = find_resource(dev, PNP_IDX_IO0);
//		init_uart8250(res0->base, &conf->com2);
		break;
	case W83627THG_KBC:
		res0 = find_resource(dev, PNP_IDX_IO0);
		res1 = find_resource(dev, PNP_IDX_IO1);
//		init_pc_keyboard(res0->base, res1->base, &conf->keyboard);
		break;
	}
}

static void w83627thg_set_resources(struct device * dev)
{
	w83627thg_enter_ext_func_mode(dev);
	pnp_set_resources(dev);
	w83627thg_exit_ext_func_mode(dev);
}

static void w83627thg_enable_resources(struct device * dev)
{
	w83627thg_enter_ext_func_mode(dev);
	pnp_enable_resources(dev);
	w83627thg_exit_ext_func_mode(dev);
}

static void w83627thg_enable(struct device * dev)
{
	w83627thg_enter_ext_func_mode(dev);   
	pnp_enable(dev);
	w83627thg_exit_ext_func_mode(dev);  
}

static void phase3_chip_setup_dev(struct device *dev);
struct device_operations w83627thg_ops = {
	.phase3_chip_setup_dev   = phase3_chip_setup_dev,
	.phase3_enable           = w83627thg_enable,
	.phase4_read_resources   = pnp_read_resources,
	.phase4_set_resources    = w83627thg_set_resources,
	.phase5_enable_resources = w83627thg_enable_resources,
	.phase6_init             = w83627thg_init,
};

/* TODO: this device is not at all filled out. Just copied from v2. */
static struct pnp_info pnp_dev_info[] = {
        { &w83627thg_ops, W83627THG_FDC, 0, PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, { 0x07f8, 0}, },
        { &w83627thg_ops, W83627THG_PP, 0, PNP_IO0 | PNP_IRQ0 | PNP_DRQ0, { 0x07f8, 0}, },
        { &w83627thg_ops, W83627THG_SP1, 0, PNP_IO0 | PNP_IRQ0, { 0x7f8, 0 }, },
        { &w83627thg_ops, W83627THG_SP2, 0, PNP_IO0 | PNP_IRQ0, { 0x7f8, 0 }, },
        // No 4 { 0,},
        { &w83627thg_ops, W83627THG_KBC, 0, PNP_IO0 | PNP_IO1 | PNP_IRQ0 | PNP_IRQ1, { 0x7ff, 0 }, { 0x7ff, 0x4}, },
        { &w83627thg_ops, W83627THG_GAME_MIDI_GPIO1, 0, PNP_IO0 | PNP_IO1 | PNP_IRQ0, { 0x7ff, 0 }, {0x7fe, 4} },
        { &w83627thg_ops, W83627THG_GPIO2,},
        { &w83627thg_ops, W83627THG_GPIO3,},
        { &w83627thg_ops, W83627THG_ACPI, 0, PNP_IRQ0,  },
        { &w83627thg_ops, W83627THG_HWM, 0, PNP_IO0 | PNP_IRQ0, { 0xff8, 0 } },
};

static void phase3_chip_setup_dev(struct device *dev)
{
	/* Get dts values and populate pnp_dev_info. */
	const struct superio_winbond_w83627thg_dts_config * const conf = dev->device_configuration;

#if 0 
These are not set up at all v2. Ignore for now. */
	/* Floppy */
	pnp_dev_info[W83627THG_FDC].enable = conf->floppyenable;
	pnp_dev_info[W83627THG_FDC].io0.val = conf->floppyio;
	pnp_dev_info[W83627THG_FDC].irq0.val = conf->floppyirq;
	pnp_dev_info[W83627THG_FDC].drq0.val = conf->floppydrq;

	/* Parallel port */
	pnp_dev_info[W83627THG_PP].enable = conf->ppenable;
	pnp_dev_info[W83627THG_PP].io0.val = conf->ppio;
	pnp_dev_info[W83627THG_PP].irq0.val = conf->ppirq;

	/* Consumer IR */
	pnp_dev_info[W83627THG_CIR].enable = conf->cirenable;

	/* Game port */
	pnp_dev_info[W83627THG_GAME_MIDI_GPIO1].enable = conf->gameenable;
	pnp_dev_info[W83627THG_GAME_MIDI_GPIO1].io0.val = conf->gameio;
	pnp_dev_info[W83627THG_GAME_MIDI_GPIO1].io1.val = conf->gameio2;
	pnp_dev_info[W83627THG_GAME_MIDI_GPIO1].irq0.val = conf->gameirq;

	/* GPIO2 */
	pnp_dev_info[W83627THG_GPIO2].enable = conf->gpio2enable;

	/* GPIO3 */
	pnp_dev_info[W83627THG_GPIO3].enable = conf->gpio3enable;

	/* ACPI */
	pnp_dev_info[W83627THG_ACPI].enable = conf->acpienable;

	/* Hardware Monitor */
	pnp_dev_info[W83627THG_HWM].enable = conf->hwmenable;
	pnp_dev_info[W83627THG_HWM].io0.val = conf->hwmio;
	pnp_dev_info[W83627THG_HWM].irq0.val = conf->hwmirq;

#endif

	/* COM1 */
	pnp_dev_info[W83627THG_SP1].enable = conf->com1enable;
	pnp_dev_info[W83627THG_SP1].io0.val = conf->com1io;
	pnp_dev_info[W83627THG_SP1].irq0.val = conf->com1irq;

	/* COM2 */
	pnp_dev_info[W83627THG_SP2].enable = conf->com2enable;
	pnp_dev_info[W83627THG_SP2].io0.val = conf->com2io;
	pnp_dev_info[W83627THG_SP2].irq0.val = conf->com2irq;

	/* Keyboard */
	pnp_dev_info[W83627THG_KBC].enable = conf->kbenable;
	pnp_dev_info[W83627THG_KBC].io0.val = conf->kbio;
	pnp_dev_info[W83627THG_KBC].io1.val = conf->kbio2;
	pnp_dev_info[W83627THG_KBC].irq0.val = conf->kbirq;
	pnp_dev_info[W83627THG_KBC].irq1.val = conf->kbirq2;

	/* Initialize SuperIO for PNP children. */
	if (!dev->links) {
		dev->links = 1;
		dev->link[0].dev = dev;
		dev->link[0].children = NULL;
		dev->link[0].link = 0;
	}

	/* Call init with updated tables to create children. */
	pnp_enable_devices(dev, &w83627thg_ops, ARRAY_SIZE(pnp_dev_info), pnp_dev_info);
}
