
#ifndef __MTK_USB_PHY_NEW_H
#define __MTK_USB_PHY_NEW_H
#include <linux/types.h>
#include <linux/clk.h>

/* helpers for direct access thru low-level io interface */

#ifdef CONFIG_PHY_MTK_SSUSB
extern int usb_mtkphy_switch_to_usb(struct phy *phy);
extern int usb_mtkphy_switch_to_uart(struct phy *phy);
extern int usb_mtkphy_check_in_uart_mode(struct phy *phy);
extern int usb_mtkphy_u3_loop_back_test(struct phy *phy);
extern int usb_mtkphy_sib_enable_switch(struct phy *phy, bool enable);
extern int usb_mtkphy_switch_to_bc11(struct phy *phy, bool on);
extern int usb_mtkphy_lpm_enable(struct phy *phy, bool on);
extern int usb_mtkphy_host_mode(struct phy *phy, bool on);
extern int usb_mtkphy_io_read(struct phy *phy, int mode, u32 reg);
extern int usb_mtkphy_io_write(struct phy *phy, int mode, u32 val, u32 reg);
#else
static inline int usb_mtkphy_switch_to_usb(struct phy *phy)
{
	return -ENODEV;
}
static inline int usb_mtkphy_switch_to_uart(struct phy *phy)
{
	return -ENODEV;
}

static inline int usb_mtkphy_check_in_uart_mode(struct phy *phy)
{
	return -ENODEV;
}

static inline int usb_mtkphy_u3_loop_back_test(struct phy *phy)
{
	return -ENODEV;
}

static inline int usb_mtkphy_sib_enable_switch(struct phy *phy, bool enable)
{
	return -ENODEV;
}

static inline int usb_mtkphy_switch_to_bc11(struct phy *phy, bool on)
{
	return -ENODEV;
}
static inline int usb_mtkphy_lpm_enable(struct phy *phy, bool on)
{
	return -ENODEV;
}
static inline int usb_mtkphy_host_mode(struct phy *phy, bool on)
{
	return -ENODEV;
}
static inline int usb_mtkphy_io_read(struct phy *phy, int mode, u32 reg)
{
	return -ENODEV;
}
static inline int usb_mtkphy_io_write(struct phy *phy, int mode, u32 val, u32 reg)
{
	return -ENODEV;
}

#endif

#endif

