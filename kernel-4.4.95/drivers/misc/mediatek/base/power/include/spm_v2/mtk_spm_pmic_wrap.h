

#ifndef __MT_SPM_PMIC_WRAP__H__
#define __MT_SPM_PMIC_WRAP__H__

#if defined(CONFIG_MACH_MT6757) || defined(CONFIG_MACH_KIBOPLUS)

#include "mtk_spm_pmic_wrap_mt6757.h"

#elif defined(CONFIG_MACH_MT6797)

#include "mt_spm_pmic_wrap_mt6797.h"

#endif

#if defined(CONFIG_MTK_PMIC_CHIP_MT6355)

#define VOLT_TO_PMIC_VAL_BASE(volt, base)  (((volt) - base + 625 - 1) / 625)
#define PMIC_VAL_TO_VOLT_BASE(pmic, base)  (((pmic) * 625) + base)

#define VOLT_TO_PMIC_VAL(volt) VOLT_TO_PMIC_VAL_BASE(volt, 40625)
#define PMIC_VAL_TO_VOLT(pmic) PMIC_VAL_TO_VOLT_BASE(pmic, 40625)

#else

#define VOLT_TO_PMIC_VAL(volt)  (((volt) - 60000 + 625 - 1) / 625)	/* ((((volt) - 700 * 100 + 625 - 1) / 625) */
#define PMIC_VAL_TO_VOLT(pmic)  (((pmic) * 625) + 60000)	/* (((pmic) * 625) / 100 + 700) */

#endif

/* APIs */
enum pmic_wrap_phase_id;
extern int mt_spm_pmic_wrap_init(void);
extern void mt_spm_pmic_wrap_set_phase(enum pmic_wrap_phase_id phase);
extern void mt_spm_pmic_wrap_set_cmd(enum pmic_wrap_phase_id phase, int idx,
				     unsigned int cmd_wdata);
extern void mt_spm_pmic_wrap_apply_cmd(int idx);
extern void mt_spm_pmic_wrap_get_cmd_full(enum pmic_wrap_phase_id phase, int idx,
					  unsigned int *p_cmd_addr, unsigned int *p_cmd_wdata);
extern void mt_spm_pmic_wrap_set_cmd_full(enum pmic_wrap_phase_id phase, int idx,
					  unsigned int cmd_addr, unsigned int cmd_wdata);


#endif /* __MT_SPM_PMIC_WRAP__H__ */
