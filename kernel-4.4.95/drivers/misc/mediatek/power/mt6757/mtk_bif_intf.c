
#include <mt-plat/upmu_common.h>
#include <mt-plat/charging.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include "mtk_bif_intf.h"

#ifdef CONFIG_MTK_BIF_SUPPORT
static bool bif_exist;
static bool bif_checked;

/* BIF related functions */
#define BC (0x400)
#define SDA (0x600)
#define ERA (0x100)
#define WRA (0x200)
#define RRA (0x300)
#define WD  (0x000)

/* Bus commands */
#define BUSRESET (0x00)
#define RBL2 (0x22)
#define RBL4 (0x24)

/* BIF slave address */
#define MW3790 (0x00)
#define MW3790_VBAT (0x0114)
#define MW3790_TBAT (0x0193)

static int bif_set_cmd(u16 bif_cmd[], int bif_cmd_len)
{
	int i = 0, con_index = 0;
	u32 ret = 0;

	for (i = 0; i < bif_cmd_len; i++) {
		ret = pmic_config_interface(MT6351_BIF_CON0 + con_index,
			bif_cmd[i], 0x07FF, 0);
		if (ret != 0) {
			battery_log(BAT_LOG_CRTI, "%s: failed, bif_cmd[%d]\n",
				__func__, i);
			return -EIO;
		}
		con_index += 0x2;
	}
	battery_log(BAT_LOG_FULL, "%s: OK\n", __func__);

	return 0;
}

static int bif_reset_irq(void)
{
	int ret = 0;
	u32 reg_irq = 0, retry_cnt = 0;

	battery_log(BAT_LOG_FULL, "%s: starts\n", __func__);
	pmic_set_register_value(PMIC_BIF_IRQ_CLR, 1);

	/* Wait until IRQ is cleared */
	do {
		reg_irq = pmic_get_register_value(PMIC_BIF_IRQ);
		retry_cnt++;
		usleep_range(50, 100);
	} while ((reg_irq != 0) && (retry_cnt < 5));

	if (reg_irq == 0)
		battery_log(BAT_LOG_FULL, "%s: OK\n", __func__);
	else {
		ret = -EIO;
		battery_log(BAT_LOG_CRTI,
			"%s: failed, PMIC_BIF_IRQ_CLR = 0x%02X\n",
			__func__, reg_irq);
	}

	pmic_set_register_value(PMIC_BIF_IRQ_CLR, 0);

	return ret;
}

static int bif_waitfor_slave(void)
{
	int ret = 0;
	u32 reg_irq = 0, retry_cnt = 0;

	battery_log(BAT_LOG_FULL, "%s: starts\n", __func__);
	do {
		reg_irq = pmic_get_register_value(PMIC_BIF_IRQ);
		retry_cnt++;
		usleep_range(100, 500);
	} while ((reg_irq == 0) && (retry_cnt < 5));


	/* Success */
	if (reg_irq == 1)
		battery_log(BAT_LOG_FULL,
			"%s: OK, retry_cnt = %d\n", __func__, retry_cnt);
	else { /* Failed */
		ret = -EIO;
		battery_log(BAT_LOG_CRTI,
			"%s: failed, PMIC_BIF_IRQ = 0x%02X, retry_cnt = %d\n",
			__func__, reg_irq, retry_cnt);
	}

	/* Reset IRQ */
	ret = bif_reset_irq();

	return ret;
}

static int bif_powerup_slave(void)
{
	int ret = 0;
	u32 bat_lost = 0, total_valid = 0, timeout = 0, retry_cnt = 0;

	battery_log(BAT_LOG_FULL, "%s: starts\n", __func__);
	do {
		pmic_set_register_value(PMIC_BIF_POWER_UP, 1);
		pmic_set_register_value(PMIC_BIF_TRASACT_TRIGGER, 1);

		ret = bif_waitfor_slave();
		if (ret < 0)
			goto _err;

		pmic_set_register_value(PMIC_BIF_TRASACT_TRIGGER, 0);
		pmic_set_register_value(PMIC_BIF_POWER_UP, 0);

		bat_lost = pmic_get_register_value(PMIC_BIF_BAT_LOST);
		total_valid = pmic_get_register_value(PMIC_BIF_TOTAL_VALID);
		timeout = pmic_get_register_value(PMIC_BIF_TIMEOUT);

		retry_cnt++;
	} while ((bat_lost == 1 || total_valid == 1 || timeout == 1)
		&& (retry_cnt < 50));

	/* Success */
	if (bat_lost == 0 && total_valid == 0 && timeout == 0) {
		battery_log(BAT_LOG_FULL,
			"%s: OK, retry_cnt = %d\n", __func__, retry_cnt);
		return ret;
	}

_err:
	/* Failed */
	battery_log(BAT_LOG_CRTI, "%s: failed, retry_cnt = %d, ret = %d\n",
		__func__, retry_cnt, ret);
	return ret;
}

static int bif_reset_slave(void)
{
	int ret = 0;
	u16 bif_cmd[1] = {0};
	u32 bat_lost = 0, total_valid = 0, timeout = 0, retry_cnt = 0;

	battery_log(BAT_LOG_FULL, "%s: starts\n", __func__);

	/* Set command sequence */
	bif_cmd[0] = BC | BUSRESET;
	ret = bif_set_cmd(bif_cmd, 1);
	if (ret < 0)
		goto _err;

	do {
		/* Command setting : 1 write command */
		pmic_set_register_value(PMIC_BIF_TRASFER_NUM, 1);
		pmic_set_register_value(PMIC_BIF_COMMAND_TYPE, 0);

		/* Command set trigger */
		pmic_set_register_value(PMIC_BIF_TRASACT_TRIGGER, 1);

		/* Command sent, wait for slave */
		ret = bif_waitfor_slave();
		if (ret < 0)
			goto _err;

		/* Command clear trigger */
		pmic_set_register_value(PMIC_BIF_TRASACT_TRIGGER, 0);

		/* Check transaction completeness */
		bat_lost = pmic_get_register_value(PMIC_BIF_BAT_LOST);
		total_valid = pmic_get_register_value(PMIC_BIF_TOTAL_VALID);
		timeout = pmic_get_register_value(PMIC_BIF_TIMEOUT);

		retry_cnt++;
	} while ((bat_lost == 1 || total_valid == 1 || timeout == 1)
		&& (retry_cnt < 50));


	if (bat_lost == 0 && total_valid == 0 && timeout == 0) {
		battery_log(BAT_LOG_FULL, "%s: OK, retry_cnt = %d\n",
			__func__, retry_cnt);
		return ret;
	}


_err:
	battery_log(BAT_LOG_CRTI, "%s: failed, retry_cnt = %d, ret = %d\n",
		__func__, retry_cnt, ret);
	return ret;
}

/* BIF write 8 transaction */
static int bif_write8(u16 addr, u8 *data)
{
	int ret = 0;
	u8 era, wra;
	u16 bif_cmd[4] = {0, 0, 0, 0};
	u32 bat_lost = 0, total_valid = 0, timeout = 0, retry_cnt = 0;

	battery_log(BAT_LOG_FULL, "%s: starts\n", __func__);


	/* Set Extended & Write Register Address */
	era = (addr & 0xFF00) >> 8;
	wra = addr & 0x00FF;
	battery_log(BAT_LOG_FULL, "%s: ERA = 0x%02x, WRA = 0x%02x\n",
		__func__, era, wra);

	/* Set command sequence */
	bif_cmd[0] = SDA | MW3790;
	bif_cmd[1] = ERA | era;	/* Extended Register Address [15:8] */
	bif_cmd[2] = WRA | wra;	/* Write Register Address [7:0] */
	bif_cmd[3] = WD  | *data; /* Data */


	ret = bif_set_cmd(bif_cmd, 4);
	if (ret < 0)
		goto _err;

	do {
		/* Set 1 byte write command */
		pmic_set_register_value(PMIC_BIF_TRASFER_NUM, 4);
		pmic_set_register_value(PMIC_BIF_COMMAND_TYPE, 0);

		/* Set trigger to start transaction */
		pmic_set_register_value(PMIC_BIF_TRASACT_TRIGGER, 1);

		/* Command sent, wait for slave */
		ret = bif_waitfor_slave();
		if (ret < 0)
			goto _err;

		/* Clear trigger */
		pmic_set_register_value(PMIC_BIF_TRASACT_TRIGGER, 0);

		/* Check transaction completeness */
		bat_lost = pmic_get_register_value(PMIC_BIF_BAT_LOST);
		total_valid = pmic_get_register_value(PMIC_BIF_TOTAL_VALID);
		timeout = pmic_get_register_value(PMIC_BIF_TIMEOUT);

		retry_cnt++;
	} while ((bat_lost == 1 || total_valid == 1 || timeout == 1)
		&& (retry_cnt < 50));


	if (bat_lost == 0  && total_valid == 0 && timeout == 0) {
		battery_log(BAT_LOG_FULL, "%s: OK, retry_cnt = %d\n",
			__func__, retry_cnt);
		return ret;
	}

_err:
	battery_log(BAT_LOG_CRTI, "%s: failed, retry_cnt = %d, ret = %d",
			__func__, retry_cnt, ret);
	battery_log(BAT_LOG_CRTI,
			"%s: bat_lost = %d, timeout = %d, total_valid = %d\n",
			__func__, bat_lost, timeout, total_valid);

	return ret;
}

/* BIF read 8 transaction */
static int bif_read8(u16 addr, u8 *data)
{
	int ret = 0;
	u8 era, rra, _data;
	u16 bif_cmd[3] = {0, 0, 0};
	u32 bat_lost = 0, total_valid = 0, timeout = 0, retry_cnt = 0;

	battery_log(BAT_LOG_FULL, "%s: starts\n", __func__);

	/* Set Extended & Read Register Address */
	era = (addr & 0xFF00) >> 8;
	rra = addr & 0x00FF;
	battery_log(BAT_LOG_FULL, "%s: ERA = 0x%02x, RRA = 0x%02x\n",
		__func__, era, rra);

	/* Set command sequence */
	bif_cmd[0] = SDA | MW3790;
	bif_cmd[1] = ERA | era;	/* [15:8] */
	bif_cmd[2] = RRA | rra;	/* [ 7:0] */

	ret = bif_set_cmd(bif_cmd, 3);
	if (ret < 0)
		goto _err;
	do {
		/* Command setting : 3 transactions for 1 byte read command(1) */
		pmic_set_register_value(PMIC_BIF_TRASFER_NUM, 3);
		pmic_set_register_value(PMIC_BIF_COMMAND_TYPE, 1);
		pmic_set_register_value(PMIC_BIF_READ_EXPECT_NUM, 1);

		/* Command set trigger */
		pmic_set_register_value(PMIC_BIF_TRASACT_TRIGGER, 1);

		/* Command sent; wait for slave */
		ret = bif_waitfor_slave();
		if (ret < 0)
			goto _err;

		/* Command clear trigger */
		pmic_set_register_value(PMIC_BIF_TRASACT_TRIGGER, 0);

		/* Check transaction completeness */
		bat_lost = pmic_get_register_value(PMIC_BIF_BAT_LOST);
		total_valid = pmic_get_register_value(PMIC_BIF_TOTAL_VALID);
		timeout = pmic_get_register_value(PMIC_BIF_TIMEOUT);

		retry_cnt++;
	} while ((bat_lost == 1 || total_valid == 1 || timeout == 1)
		&& (retry_cnt < 50));


	/* Read data */
	if (bat_lost == 0 && total_valid == 0 && timeout == 0) {
		_data = pmic_get_register_value(PMIC_BIF_DATA_0);
		battery_log(BAT_LOG_FULL,
			"%s: OK, data = 0x%02X, retry_cnt = %d\n",
			__func__, _data, retry_cnt);
		*data = _data & 0xFF;
		return ret;
	}

_err:
	battery_log(BAT_LOG_CRTI, "%s: failed, retry_cnt = %d, ret = %d\n",
		__func__, retry_cnt, ret);
	battery_log(BAT_LOG_CRTI,
		"%s: failed, bat_lost = %d, timeout = %d, totoal_valid = %d\n",
		__func__, bat_lost, timeout, total_valid);

	return ret;
}

/* Bif read 16 transaction*/
static int bif_read16(u16 addr, u16 *data)
{
	int ret = 0;
	u8 era, rra;
	u16 bif_cmd[4] = {0, 0, 0, 0};
	u8 _data[2] = {0, 0}; /* 2 bytes data */
	u32 bat_lost = 0, total_valid = 0, timeout = 0, retry_cnt = 0;

	battery_log(BAT_LOG_FULL, "%s: starts\n", __func__);

	/* Set Extended & Read Register Address */
	era = (addr & 0xFF00) >> 8;
	rra = addr & 0x00FF;
	battery_log(BAT_LOG_FULL, "%s: ERA = 0x%02x, RRA= 0x%02x\n",
		__func__, era, rra);

	/* Set command sequence */
	bif_cmd[0] = SDA | MW3790;
	bif_cmd[1] = BC | RBL2;	/* Read back 2 bytes */
	bif_cmd[2] = ERA | era;	/* [15:8] */
	bif_cmd[3] = RRA | rra;	/* [ 7:0] */


	ret = bif_set_cmd(bif_cmd, 4);
	if (ret < 0)
		goto _err;
	do {
		/* Command setting : 4 transactions for 2 byte read command(1) */
		pmic_set_register_value(PMIC_BIF_TRASFER_NUM, 4);
		pmic_set_register_value(PMIC_BIF_COMMAND_TYPE, 1);
		pmic_set_register_value(PMIC_BIF_READ_EXPECT_NUM, 2);

		/* Command set trigger */
		pmic_set_register_value(PMIC_BIF_TRASACT_TRIGGER, 1);

		/* Command sent; wait for slave */
		ret = bif_waitfor_slave();
		if (ret < 0)
			goto _err;

		/* Command clear trigger */
		pmic_set_register_value(PMIC_BIF_TRASACT_TRIGGER, 0);

		/* Check transaction completeness */
		bat_lost = pmic_get_register_value(PMIC_BIF_BAT_LOST);
		total_valid = pmic_get_register_value(PMIC_BIF_TOTAL_VALID);
		timeout = pmic_get_register_value(PMIC_BIF_TIMEOUT);

		retry_cnt++;
	} while ((bat_lost == 1 || total_valid == 1 || timeout == 1)
		&& (retry_cnt < 50));


	/* Read data */
	if (bat_lost == 0 && total_valid == 0 && timeout == 0) {
		_data[0] = pmic_get_register_value(PMIC_BIF_DATA_0);
		_data[1] = pmic_get_register_value(PMIC_BIF_DATA_1);
		*data = ((_data[0] & 0xFF) << 8) | (_data[1] & 0xFF);
		battery_log(BAT_LOG_FULL,
			"%s: OK, data = 0x%02x, 0x%02x, retry_cnt = %d\n",
			__func__, _data[0], _data[1], retry_cnt);
		return ret;
	}

_err:
	battery_log(BAT_LOG_CRTI, "%s: failed, retry_cnt = %d, ret = %d\n",
		__func__, retry_cnt, ret);
	battery_log(BAT_LOG_CRTI,
		"%s: bat_lost = %d, timeout = %d, totoal_valid = %d\n",
		__func__, bat_lost, timeout, total_valid);

	return ret;
}

static int bif_adc_enable(void)
{
	int ret = 0;
	u8 reg = 0x18;

	battery_log(BAT_LOG_FULL, "%s: starts\n", __func__);
	ret = bif_write8(0x0110, &reg);
	if (ret < 0)
		goto _err;

	mdelay(50);

	reg = 0x98;
	ret = bif_write8(0x0110, &reg);
	if (ret < 0)
		goto _err;

	mdelay(50);

	battery_log(BAT_LOG_FULL, "%s: OK\n", __func__);
	return ret;

_err:
	battery_log(BAT_LOG_CRTI, "%s: failed, ret = %d\n", __func__, ret);
	return ret;
}

/* BIF init function called only at the first time*/
static int bif_init(void)
{
	int ret = 0;

	/* Disable BIF interrupt */
	pmic_set_register_value(PMIC_INT_CON0_CLR, 0x4000);

	/* Enable BIF clock */
	pmic_set_register_value(PMIC_TOP_CKPDN_CON2_CLR, 0x0070);

	/* Enable HT protection */
	pmic_set_register_value(PMIC_RG_BATON_HT_EN, 1);

	mdelay(50);

	/* Enable RX filter function */
	pmic_set_register_value(MT6351_PMIC_BIF_RX_DEG_EN, 0x8000);
	pmic_set_register_value(MT6351_PMIC_BIF_RX_DEG_WND, 0x17);
	pmic_set_register_value(PMIC_RG_BATON_EN, 0x1);
	pmic_set_register_value(PMIC_BATON_TDET_EN, 0x1);
	pmic_set_register_value(PMIC_RG_BATON_HT_EN_DLY_TIME, 0x1);


	/* Wake up BIF slave */
	ret = bif_powerup_slave();
	if (ret < 0)
		goto _err;

	mdelay(10);

	/* Reset BIF slave */
	ret = bif_reset_slave();
	if (ret < 0)
		goto _err;

	mdelay(50);

	battery_log(BAT_LOG_FULL, "%s: OK\n", __func__);
	return ret;

_err:
	battery_log(BAT_LOG_CRTI, "%s: failed, ret = %d", __func__, ret);
	return ret;
}

int mtk_bif_init(void)
{
	int ret = 0;
	u32 vbat = 0;

	battery_log(BAT_LOG_FULL, "%s: starts\n", __func__);

	if (bif_checked)
		return ret;

	/* Initiate BIF */
	ret = bif_init();
	if (ret < 0)
		goto _err;

	/* Read VBAT to check existence of BIF */
	ret = mtk_bif_get_vbat(&vbat);
	if (ret == 0 && vbat != 0) {
		battery_log(BAT_LOG_CRTI, "%s: BIF battery detected\n",
			__func__);
		bif_exist = true;
		bif_checked = true;
		return ret;
	}

_err:
	battery_log(BAT_LOG_CRTI, "%s: BIF battery _NOT_ detected\n", __func__);
	bif_exist = false;
	bif_checked = true;

	return ret;
}

int mtk_bif_get_vbat(unsigned int *vbat)
{
	int ret = 0;
	u16 _vbat = 0;

	/* Prevent ADC & BIF read at the same time */
	lockadcch3();
	battery_log(BAT_LOG_FULL, "%s: starts\n", __func__);

	if (bif_checked && !bif_exist) {
		ret = -ENOTSUPP;
		goto _err;
	}

	/* Enable ADC */
	ret = bif_adc_enable();
	if (ret < 0)
		goto _err;

	/* Read Data */
	ret = bif_read16(MW3790_VBAT, &_vbat);
	if (ret < 0 || _vbat == 0)
		goto _err;

	*vbat = _vbat;
	battery_log(BAT_LOG_FULL, "%s: OK, vbat = %dmV\n", __func__, _vbat);
	unlockadcch3();
	return ret;

_err:
	*vbat = 0;
	battery_log(BAT_LOG_CRTI, "%s: failed, ret = %d\n", __func__, ret);
	unlockadcch3();
	return ret;
}

int mtk_bif_get_tbat(int *tbat)
{
	int ret = 0;
	s8 _tbat = 0;

	/* Prevent ADC & BIF read at the same time */
	lockadcch3();
	battery_log(BAT_LOG_FULL, "%s: starts\n", __func__);

	if (!bif_exist) {
		ret = -ENOTSUPP;
		goto _err;
	}

	/* Enable ADC */
	ret = bif_adc_enable();
	if (ret < 0)
		goto _err;

	/* Read Data */
	ret = bif_read8(MW3790_TBAT, &_tbat);
	if (ret < 0)
		goto _err;

	*tbat = _tbat;
	battery_log(BAT_LOG_FULL, "%s: OK, tbat = %d(degree)\n",
		__func__, _tbat);
	unlockadcch3();
	return ret;

_err:
	*tbat = 0;
	battery_log(BAT_LOG_CRTI, "%s: failed, ret = %d\n", __func__, ret);
	unlockadcch3();
	return ret;
}

bool mtk_bif_is_hw_exist(void)
{
	return bif_exist;
}
#endif
