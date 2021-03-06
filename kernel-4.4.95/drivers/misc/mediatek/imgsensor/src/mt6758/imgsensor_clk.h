
#ifndef __IMGSENSOR_CLK_H__
#define __IMGSENSOR_CLK_H__

#include <linux/atomic.h>
#include <linux/platform_device.h>
#include <kd_imgsensor_define.h>
#include "imgsensor_common.h"

enum IMGSENSOR_CCF {
	IMGSENSOR_CCF_MCLK_TG_MIN_NUM,
	IMGSENSOR_CCF_MCLK_TOP_CAMTG_SEL = IMGSENSOR_CCF_MCLK_TG_MIN_NUM,
	IMGSENSOR_CCF_MCLK_TOP_CAMTG2_SEL,
	IMGSENSOR_CCF_MCLK_TOP_CAMTG3_SEL,
	IMGSENSOR_CCF_MCLK_TOP_CAMTG4_SEL,
	IMGSENSOR_CCF_MCLK_TG_MAX_NUM,
	IMGSENSOR_CCF_MCLK_FREQ_MIN_NUM = IMGSENSOR_CCF_MCLK_TG_MAX_NUM,
	IMGSENSOR_CCF_MCLK_TOP_UNIVPLL_D416 = IMGSENSOR_CCF_MCLK_FREQ_MIN_NUM,
	IMGSENSOR_CCF_MCLK_TOP_UNIVPLL_D208,
	IMGSENSOR_CCF_MCLK_TOP_UNIVPLL_D104,
	IMGSENSOR_CCF_MCLK_TOP_CLK26M,
	IMGSENSOR_CCF_MCLK_TOP_UNIVPLL_D52,
	IMGSENSOR_CCF_MCLK_TOP_UNIVPLL2_D16,
	IMGSENSOR_CCF_MCLK_FREQ_MAX_NUM,

	IMGSENSOR_CCF_CG_MIN_NUM = IMGSENSOR_CCF_MCLK_FREQ_MAX_NUM,
	IMGSENSOR_CCF_CG_SENINF = IMGSENSOR_CCF_CG_MIN_NUM,
	IMGSENSOR_CCF_CG_MAX_NUM,
	IMGSENSOR_CCF_MAX_NUM = IMGSENSOR_CCF_CG_MAX_NUM,
};

struct IMGSENSOR_CLK {
	struct clk *imgsensor_ccf[IMGSENSOR_CCF_MAX_NUM];
	atomic_t    enable_cnt[IMGSENSOR_CCF_MAX_NUM];
};

extern unsigned int mt_get_ckgen_freq(int ID);
enum IMGSENSOR_RETURN imgsensor_clk_init(struct IMGSENSOR_CLK *pclk);
int  imgsensor_clk_set(struct IMGSENSOR_CLK *pclk, ACDK_SENSOR_MCLK_STRUCT *pmclk);
void imgsensor_clk_enable_all(struct IMGSENSOR_CLK *pclk);
void imgsensor_clk_disable_all(struct IMGSENSOR_CLK *pclk);
int imgsensor_clk_ioctrl_handler(void *pbuff);
extern struct platform_device *gpimgsensor_hw_platform_device;

#endif

