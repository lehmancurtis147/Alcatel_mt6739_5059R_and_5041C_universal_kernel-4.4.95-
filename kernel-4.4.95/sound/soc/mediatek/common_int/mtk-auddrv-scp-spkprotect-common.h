


#ifndef AUDIO_SPKPROCT_COMMON_H
#define AUDIO_SPKPROCT_COMMON_H

#include <linux/kernel.h>
#include "mtk-auddrv-common.h"
#include "mtk-soc-pcm-common.h"
#include "mtk-auddrv-def.h"
#include "mtk-auddrv-afe.h"
#include "mtk-auddrv-kernel.h"
#include "mtk-soc-afe-control.h"
#include "audio_spkprotect_msg_id.h"


#ifdef CONFIG_MTK_AUDIO_SCP_SPKPROTECT_SUPPORT
#include <audio_task_manager.h>
#include <audio_ipi_client_spkprotect.h>
#include <audio_dma_buf_control.h>
#endif

struct spk_dump_ops {
	void (*spk_dump_callback)(struct ipi_msg_t *ipi_msg);
};

struct aud_spk_message {
	uint16_t msg_id;
	uint32_t param1;
	uint32_t param2;
	char *payload;
};

void init_spkscp_reserved_dram(void);
struct audio_resv_dram_t *get_reserved_dram_spkprotect(void);
char *get_resv_dram_spkprotect_vir_addr(char *resv_dram_phy_addr);
void spkproc_service_set_spk_dump_message(struct spk_dump_ops *ops);
void spkproc_service_ipicmd_received(struct ipi_msg_t *ipi_msg);
void spkproc_service_ipicmd_send(
				 uint8_t  data_type,  /* see audio_ipi_msg_data_t */
				 uint8_t  ack_type,   /* see audio_ipi_msg_ack_t */
				 uint16_t msg_id,
				 uint32_t param1,
				 uint32_t param2,
				 char *payload);
#endif
