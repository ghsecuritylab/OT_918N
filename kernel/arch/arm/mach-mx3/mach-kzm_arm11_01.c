

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/smsc911x.h>
#include <linux/types.h>

#include <asm/irq.h>
#include <asm/mach-types.h>
#include <asm/setup.h>
#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>

#include <mach/board-kzmarm11.h>
#include <mach/clock.h>
#include <mach/common.h>
#include <mach/imx-uart.h>
#include <mach/iomux-mx3.h>
#include <mach/memory.h>

#include "devices.h"

#define KZM_ARM11_IO_ADDRESS(x) (					\
	IMX_IO_ADDRESS(x, MX31_CS4) ?:					\
	IMX_IO_ADDRESS(x, MX31_CS5) ?:					\
	MX31_IO_ADDRESS(x))

#if defined(CONFIG_SERIAL_8250) || defined(CONFIG_SERIAL_8250_MODULE)
static struct plat_serial8250_port serial_platform_data[] = {
	{
		.membase	= KZM_ARM11_IO_ADDRESS(KZM_ARM11_16550),
		.mapbase	= KZM_ARM11_16550,
		.irq		= IOMUX_TO_IRQ(MX31_PIN_GPIO1_1),
		.irqflags	= IRQ_TYPE_EDGE_RISING,
		.uartclk	= 14745600,
		.regshift	= 0,
		.iotype		= UPIO_MEM,
		.flags		= UPF_BOOT_AUTOCONF | UPF_SKIP_TEST |
				  UPF_BUGGY_UART,
	},
	{},
};

static struct resource serial8250_resources[] = {
	{
		.start	= KZM_ARM11_16550,
		.end	= KZM_ARM11_16550 + 0x10,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= IOMUX_TO_IRQ(MX31_PIN_GPIO1_1),
		.end	= IOMUX_TO_IRQ(MX31_PIN_GPIO1_1),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device serial_device = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev		= {
				.platform_data = serial_platform_data,
			  },
	.num_resources	= ARRAY_SIZE(serial8250_resources),
	.resource	= serial8250_resources,
};

static int __init kzm_init_ext_uart(void)
{
	u8 tmp;

	/*
	 * GPIO 1-1: external UART interrupt line
	 */
	mxc_iomux_mode(IOMUX_MODE(MX31_PIN_GPIO1_1, IOMUX_CONFIG_GPIO));
	gpio_request(IOMUX_TO_GPIO(MX31_PIN_GPIO1_1), "ext-uart-int");
	gpio_direction_input(IOMUX_TO_GPIO(MX31_PIN_GPIO1_1));

	/*
	 * Unmask UART interrupt
	 */
	tmp = __raw_readb(KZM_ARM11_IO_ADDRESS(KZM_ARM11_CTL1));
	tmp |= 0x2;
	__raw_writeb(tmp, KZM_ARM11_IO_ADDRESS(KZM_ARM11_CTL1));

	return platform_device_register(&serial_device);
}
#else
static inline int kzm_init_ext_uart(void)
{
	return 0;
}
#endif

#if defined(CONFIG_SMSC911X) || defined(CONFIG_SMSC911X_MODULE)
static struct smsc911x_platform_config kzm_smsc9118_config = {
	.phy_interface	= PHY_INTERFACE_MODE_MII,
	.irq_polarity	= SMSC911X_IRQ_POLARITY_ACTIVE_HIGH,
	.irq_type	= SMSC911X_IRQ_TYPE_PUSH_PULL,
	.flags		= SMSC911X_USE_32BIT | SMSC911X_SAVE_MAC_ADDRESS,
};

static struct resource kzm_smsc9118_resources[] = {
	{
		.start	= MX31_CS5_BASE_ADDR,
		.end	= MX31_CS5_BASE_ADDR + SZ_128K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= IOMUX_TO_IRQ(MX31_PIN_GPIO1_2),
		.end	= IOMUX_TO_IRQ(MX31_PIN_GPIO1_2),
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	},
};

static struct platform_device kzm_smsc9118_device = {
	.name		= "smsc911x",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(kzm_smsc9118_resources),
	.resource	= kzm_smsc9118_resources,
	.dev		= {
				.platform_data = &kzm_smsc9118_config,
			  },
};

static int __init kzm_init_smsc9118(void)
{
	/*
	 * GPIO 1-2: SMSC9118 interrupt line
	 */
	mxc_iomux_mode(IOMUX_MODE(MX31_PIN_GPIO1_2, IOMUX_CONFIG_GPIO));
	gpio_request(IOMUX_TO_GPIO(MX31_PIN_GPIO1_2), "smsc9118-int");
	gpio_direction_input(IOMUX_TO_GPIO(MX31_PIN_GPIO1_2));

	return platform_device_register(&kzm_smsc9118_device);
}
#else
static inline int kzm_init_smsc9118(void)
{
	return 0;
}
#endif

#if defined(CONFIG_SERIAL_IMX) || defined(CONFIG_SERIAL_IMX_MODULE)
static struct imxuart_platform_data uart_pdata = {
	.flags = IMXUART_HAVE_RTSCTS,
};

static void __init kzm_init_imx_uart(void)
{
	mxc_register_device(&mxc_uart_device0, &uart_pdata);

	mxc_register_device(&mxc_uart_device1, &uart_pdata);
}
#else
static inline void kzm_init_imx_uart(void)
{
}
#endif

static int kzm_pins[] __initdata = {
	MX31_PIN_CTS1__CTS1,
	MX31_PIN_RTS1__RTS1,
	MX31_PIN_TXD1__TXD1,
	MX31_PIN_RXD1__RXD1,
	MX31_PIN_DCD_DCE1__DCD_DCE1,
	MX31_PIN_RI_DCE1__RI_DCE1,
	MX31_PIN_DSR_DCE1__DSR_DCE1,
	MX31_PIN_DTR_DCE1__DTR_DCE1,
	MX31_PIN_CTS2__CTS2,
	MX31_PIN_RTS2__RTS2,
	MX31_PIN_TXD2__TXD2,
	MX31_PIN_RXD2__RXD2,
	MX31_PIN_DCD_DTE1__DCD_DTE2,
	MX31_PIN_RI_DTE1__RI_DTE2,
	MX31_PIN_DSR_DTE1__DSR_DTE2,
	MX31_PIN_DTR_DTE1__DTR_DTE2,
};

static void __init kzm_board_init(void)
{
	mxc_iomux_setup_multiple_pins(kzm_pins,
				      ARRAY_SIZE(kzm_pins), "kzm");
	kzm_init_ext_uart();
	kzm_init_smsc9118();
	kzm_init_imx_uart();

	pr_info("Clock input source is 26MHz\n");
}

static struct map_desc kzm_io_desc[] __initdata = {
	{
		.virtual	= MX31_CS4_BASE_ADDR_VIRT,
		.pfn		= __phys_to_pfn(MX31_CS4_BASE_ADDR),
		.length		= MX31_CS4_SIZE,
		.type		= MT_DEVICE
	},
	{
		.virtual	= MX31_CS5_BASE_ADDR_VIRT,
		.pfn		= __phys_to_pfn(MX31_CS5_BASE_ADDR),
		.length		= MX31_CS5_SIZE,
		.type		= MT_DEVICE
	},
};

static void __init kzm_map_io(void)
{
	mx31_map_io();
	iotable_init(kzm_io_desc, ARRAY_SIZE(kzm_io_desc));
}

static void __init kzm_timer_init(void)
{
	mx31_clocks_init(26000000);
}

static struct sys_timer kzm_timer = {
	.init   = kzm_timer_init,
};

MACHINE_START(KZM_ARM11_01, "Kyoto Microcomputer Co., Ltd. KZM-ARM11-01")
	.phys_io        = MX31_AIPS1_BASE_ADDR,
	.io_pg_offst    = (MX31_AIPS1_BASE_ADDR_VIRT >> 18) & 0xfffc,
	.boot_params    = MX3x_PHYS_OFFSET + 0x100,
	.map_io         = kzm_map_io,
	.init_irq       = mx31_init_irq,
	.init_machine   = kzm_board_init,
	.timer          = &kzm_timer,
MACHINE_END
