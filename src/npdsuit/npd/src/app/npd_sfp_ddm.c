
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "nbm/nbm_api.h"
#include "board/ts_product_feature.h"
#include "nbm/nbm_cpld.h"
#include "npd/nbm/npd_cplddef.h"
#include "npd/product_feature.h"
#include "npd/npd_sfp_ddm.h"

static int npd_get_sfp_checksum_stat(unsigned char *buf)
{	
	int i, sum;

	if (buf == NULL)		
		return -1;

	/* Check A0 code for Base ID Fields (addresses 0 to 62) */	
	for (i = SFP_A0H_IDENTIFIER_OFFSET, sum = 0; i < SFP_A0H_CC_BASE_OFFSET; i++)		
		sum += buf[i];
	sum &= 0xFF;
	if (sum != buf[SFP_A0H_CC_BASE_OFFSET])
		return -1;
	
	/* Check A0 code for the Extended ID Fields (addresses 64 to 94) */
	for (i = SFP_A0H_OPTIONS_IMPLEMENTED_OFFSET, sum = 0; i < SFP_A0H_CC_EXT_OFFSET; i++)		
		sum += buf[i];
	sum &= 0xFF;
	if (sum != buf[SFP_A0H_CC_EXT_OFFSET])
		return -1;
	
	return 0;
}

static int npd_sfp_get_smf_km_length(unsigned char *buf)
{
	if (buf == NULL)		
		return -1;

	/* Normalize length to meters */	
	return buf[SFP_A0H_LENGTH_SMF_KM_OFFSET] * 1000;
}

static int npd_sfp_get_smf_100m_length(unsigned char *buf)
{	
	if (buf == NULL)		
		return -1;

	/* Normalize length to meters */	
	return buf[SFP_A0H_LENGTH_SMF_100M_OFFSET] * 100;
}

static int npd_sfp_get_om2_length(unsigned char *buf)
{
	if (buf == NULL)		
		return -1;

	/* Normalize length to meters */	
	return buf[SFP_A0H_LENGTH_OM2_10M_OFFSET] * 10;
}

static int npd_sfp_get_om1_length(unsigned char *buf)
{
	if (buf == NULL)		
		return -1;

	/* Normalize length to meters */	
	return buf[SFP_A0H_LENGTH_OM1_10M_OFFSET] * 10;
}

static int npd_sfp_get_om4_or_copper_length(unsigned char *buf)
{
#define ACTIVE_CABLE 1
#define COPPER_CABLE 2
	int length = 0;	
	int cable_type = ACTIVE_CABLE;
		
	if (buf == NULL)		
		return -1;

	/* Normalize length to meters. Optical link is measured in units of 10 meters	 
	 * and copper link is measured in units of 1 meter. */
	if(cable_type == ACTIVE_CABLE)
		length = buf[SFP_A0H_LENGTH_OM4_10M_COPPER_1M_OFFSET] * 10;
	else
		length = buf[SFP_A0H_LENGTH_OM4_10M_COPPER_1M_OFFSET];	
	
	return length;
}

static int npd_sfp_get_om3_length(unsigned char *buf)
{	
	if (buf == NULL)		
		return -1;

	/* Normalize length to meters */	
	return buf[SFP_A0H_LENGTH_OM3_10M_OFFSET] * 10;
}

static int npd_sfp_get_wavelength(unsigned char *buf)
{	
	int length = 0;

	if (buf == NULL)
		return -1;

	/* Wavelength is valid only when SFP+ cable technology is zero */	
	if ((buf[SFP_A0H_TRANSCEIVER_DESCR_OFFSET+5] & 0x0C) != 0)		
		return 0;

	/* The first address is the high part of the 16 bit wavelength and the next	 
	 * address is the low part */	
	length |= buf[SFP_A0H_WAVELENGTH_OFFSET] << 8;
	length |= buf[SFP_A0H_WAVELENGTH_OFFSET + 1];

	return length;
}

static int npd_sfp_get_ddm_implemented(unsigned char *buf)
{
	if (buf == NULL)
		return -1;

	if (buf[SFP_A0H_DIAG_MONITORING_TYPE_OFFSET] & DIGITAL_DIAGNOSTIC_MASK)		
		return 1;
	else
		return 0;
}

static int npd_sfp_get_ascii(unsigned char *buf, int offset, int size, char *out)
{
	char *p = NULL;
	int i;

	if (buf == NULL || out == NULL)		
		return -1;	

	p = &buf[offset];
	
	/* ASCII String, right-padded with 0x20 */
	for (i = 0; i < size; i++) {
		if (p[i] == 0x20) {
			p[i] = '\0';
			break;
		}
	}
		
	return snprintf(out, size, "%s", p);
}

static int npd_sfp_get_vendor_date_code(unsigned char *buf, unsigned char offset, char *out)
{
	char *p = NULL;

	if (buf == NULL || out == NULL)
		return -1;	

	p = &buf[offset];
	
	/* Date code, see Table 3.8 for description */
	return sprintf(out, "20%c%c-%c%c-%c%c", p[0],p[1],p[2],p[3],p[4],p[5]);
}

static int npd_get_sfp_alarm_type(unsigned char *buf)
{	
	unsigned int alarm_flag = 0;
	int i;
	
	if (buf == NULL)		
		return -1;

	alarm_flag = (buf[SFP_A2H_OPTIONAL_STATUS_OFFSET]<<16) |
				 (buf[SFP_A2H_ALARM_FLAG_BITS_OFFSET]<< 8) | 
				 (buf[SFP_A2H_ALARM_FLAG_BITS_OFFSET+1]);

	alarm_flag = alarm_flag >> 6;
	
	for (i = SFP_NO_ALARM; i < SFP_RX_POWER_LOW; i++) {
		if((alarm_flag >> i) & 0x1) {
			return (SFP_RX_POWER_LOW - i);
		}
	}

	return SFP_NO_ALARM;
}

int npd_get_sfp_info(unsigned long panel_port, int port_type, fiber_module_man_param_t *param)
{
	unsigned char a0[128] = {0};
	unsigned char a2[128] = {0};
	int err = 0;
	
	err += nbm_sfp_read(panel_port, 0x50, 0x00, a0, 128);
	err += nbm_sfp_read(panel_port, 0x51, 0x00, a2, 128);
	if (err < 0) {
		param->alarm_type = SFP_IO_ERROR;
		return 0;
	}
	
	if (npd_get_sfp_checksum_stat(a0) < 0) {
		//param->alarm_type = SFP_CHECKSUM_ERROR;
		//return 0;
		nbm_sfp_read(panel_port, 0x50, 0x00, a0, 3);
		nbm_sfp_read(panel_port, 0x50, 0x00, a0, 128);
	}

	param->port_type = port_type;
	param->alarm_type = npd_get_sfp_alarm_type(a2);

	param->identifier = a0[SFP_A0H_IDENTIFIER_OFFSET];
	param->connector = a0[SFP_A0H_CONNECTOR_OFFSET];
	param->transceiver_class = (a0[SFP_A0H_TRANSCEIVER_DESCR_OFFSET]<<8) | a0[SFP_A0H_TRANSCEIVER_DESCR_OFFSET+3];
	param->wavelength = npd_sfp_get_wavelength(a0);
	param->ddm_implemented = npd_sfp_get_ddm_implemented(a0); // yes

	param->transmission_media = a0[SFP_A0H_TRANSCEIVER_DESCR_OFFSET+6];
	param->smf_km_length = npd_sfp_get_smf_km_length(a0);
	param->smf_100m_length = npd_sfp_get_smf_100m_length(a0);
	param->om2_length = npd_sfp_get_om2_length(a0);
	param->om1_length = npd_sfp_get_om1_length(a0);
	param->om4_or_copper_length = npd_sfp_get_om4_or_copper_length(a0);
	param->om3_length = npd_sfp_get_om3_length(a0);
	
	npd_sfp_get_ascii(a0, SFP_A0H_VENDOR_NAME_OFFSET, SFP_A0H_VENDOR_NAME_SIZE, param->vendor_name); // yes
	npd_sfp_get_ascii(a0, SFP_A0H_VENDOR_PN_OFFSET,   SFP_A0H_VENDOR_PN_SIZE, param->vendor_pn); // yes
	npd_sfp_get_ascii(a0, SFP_A0H_VENDOR_SN_OFFSET,   SFP_A0H_VENDOR_SN_SIZE,   param->vendor_sn); // yes
	npd_sfp_get_vendor_date_code(a0, SFP_A0H_DATE_CODE_OFFSET, param->date_code); // yes

	param->temperature = (a2[SFP_A2H_TEMPERATURE_OFFSET]<<8) | a2[SFP_A2H_TEMPERATURE_OFFSET+1]; // yes
	param->voltage     = (a2[SFP_A2H_VOLTAGE_OFFSET]<<8) | a2[SFP_A2H_VOLTAGE_OFFSET+1]; // yes
	param->tx_bias[0]  = (a2[SFP_A2H_TX_BIAS_OFFSET]<<8) | a2[SFP_A2H_TX_BIAS_OFFSET+1]; // yes
	param->tx_power[0] = (a2[SFP_A2H_TX_POWER_OFFSET]<<8) | a2[SFP_A2H_TX_POWER_OFFSET+1]; // yes
	param->rx_power[0] = (a2[SFP_A2H_RX_POWER_OFFSET]<<8) | a2[SFP_A2H_RX_POWER_OFFSET+1]; // yes
	
	return 0;
}

static int npd_get_qsfp_checksum_stat(unsigned char *buf)
{
	int i, sum;

	if (buf == NULL)		
		return -1;

	/* QSFP/QSFP+: Identifier must be 0x0C/0x0D */
	if((buf[QSFP_IDENTIFIER_OFFSET] & 0xFE) != 0x0C)
		return -1;
	
	/* Check A0_00H code for Base ID Fields (addresses 128 to 190) */	
	for (i = QSFP_IDENTIFIER1_OFFSET, sum = 0; i < QSFP_CC_BASE_OFFSET; i++)		
		sum += buf[i];
	sum &= 0xFF;
	if (sum != buf[QSFP_CC_BASE_OFFSET])
		return -1;
	
	/* Check A0_00H code for Extended ID Fields (addresses 192 to 220) */	
	for (i = QSFP_CC_BASE_OFFSET+1, sum = 0; i < QSFP_CC_EXT_OFFSET; i++)		
		sum += buf[i];
	sum &= 0xFF;
	if (sum != buf[QSFP_CC_EXT_OFFSET])
		return -1;
	
	return 0;
}

static int npd_get_sqfp_wavelength(unsigned char *buf)
{	
	int length = 0;

	if (buf == NULL)
		return -1;

	length |= buf[QSFP_WAVELENGTH_OFFSET] << 8;
	length |= buf[QSFP_WAVELENGTH_OFFSET + 1];

	length /= 20;
	
	return length;
}

static int npd_get_qsfp_smf_km_length(unsigned char *buf)
{
	if (buf == NULL)		
		return -1;

	/* Normalize length to meters */	
	return buf[QSFP_LENGTH_SMF_KM_OFFSET] * 1000;	
}

static int npd_get_qsfp_om3_length(unsigned char *buf)
{	
	if (buf == NULL)		
		return -1;

	/* Normalize length to meters */	
	return buf[QSFP_LENGTH_OM3_10M_OFFSET] * 2;
}

static int npd_get_qsfp_om2_length(unsigned char *buf)
{
	if (buf == NULL)		
		return -1;

	/* Normalize length to meters */	
	return buf[QSFP_LENGTH_OM2_10M_OFFSET];
}

static int npd_get_qsfp_om1_length(unsigned char *buf)
{
	if (buf == NULL)		
		return -1;

	/* Normalize length to meters */	
	return buf[QSFP_LENGTH_OM1_10M_OFFSET];
}

static int npd_get_qsfp_om4_or_copper_length(unsigned char *buf)
{
#define ACTIVE_CABLE 1
#define COPPER_CABLE 2
	int length = 0;	
	int cable_type = ACTIVE_CABLE;
		
	if (buf == NULL)		
		return -1;

	/* Normalize length to meters. Optical link is measured in units of 10 meters	 
	 * and copper link is measured in units of 1 meter. */
	if(cable_type == ACTIVE_CABLE)
		length = buf[QSFP_LENGTH_OM4_10M_COPPER_1M_OFFSET] * 2;
	else
		length = buf[QSFP_LENGTH_OM4_10M_COPPER_1M_OFFSET];	
	
	return length;
}

int npd_get_qsfp_info(unsigned long panel_port, int port_type, fiber_module_man_param_t *param)
{
	unsigned char a0[256] = {0};
	int err = 0;
	int i;

	err = nbm_sfp_read(panel_port, 0x50, 0x00, a0, 128);
	if (err < 0) {
		param->alarm_type = SFP_IO_ERROR;
		return 0;
	}

	/* QSFP/QSFP+: Identifier must be 0x0C/0x0D */
	if ((a0[QSFP_IDENTIFIER_OFFSET] & 0xFE) != 0x0C) {
		param->alarm_type = SFP_CHECKSUM_ERROR;
		return 0;
	}

	err = nbm_sfp_read(panel_port, 0x50, 0x80, &a0[128], 128);
	if (err < 0) {
		param->alarm_type = SFP_IO_ERROR;
		return 0;
	}

	//if (npd_get_qsfp_checksum_stat(a0) < 0) {
	if ((a0[QSFP_IDENTIFIER1_OFFSET] & 0xFE) != 0x0C) {
		param->alarm_type = SFP_CHECKSUM_ERROR;
		return 0;
	}
	
	param->port_type = port_type;
	param->alarm_type = SFP_TYPE_UNSUPPORT;

	param->identifier = a0[QSFP_IDENTIFIER_OFFSET];
	param->connector = a0[QSFP_CONNECTOR_OFFSET];
	param->transceiver_class = (a0[QSFP_TRANSCEIVER_DESCR_OFFSET]<<8) | a0[QSFP_TRANSCEIVER_DESCR_OFFSET+3];
	param->wavelength = npd_get_sqfp_wavelength(a0);
	param->ddm_implemented = 1;

	param->transmission_media = a0[QSFP_TRANSCEIVER_DESCR_OFFSET+6];
	param->smf_km_length = npd_get_qsfp_smf_km_length(a0);
	param->om3_length = npd_get_qsfp_om3_length(a0);
	param->om2_length = npd_get_qsfp_om2_length(a0);
	param->om1_length = npd_get_qsfp_om2_length(a0);
	param->om4_or_copper_length = npd_get_qsfp_om4_or_copper_length(a0);
	
	npd_sfp_get_ascii(a0, QSFP_VENDOR_NAME_OFFSET, QSFP_VENDOR_NAME_SIZE, param->vendor_name); // yes
	npd_sfp_get_ascii(a0, QSFP_VENDOR_PN_OFFSET,   QSFP_VENDOR_NAME_SIZE, param->vendor_pn); // yes
	npd_sfp_get_ascii(a0, QSFP_VENDOR_SN_OFFSET,   QSFP_VENDOR_SN_SIZE,   param->vendor_sn); // yes
	npd_sfp_get_vendor_date_code(a0, QSFP_DATE_CODE_OFFSET, param->date_code); // yes

	param->temperature = (a0[QSFP_TEMPERATURE_OFFSET]<<8) | a0[QSFP_TEMPERATURE_OFFSET+1]; // yes
	param->voltage     = (a0[QSFP_SUPPLY_VOLTAGE_OFFSET]<<8) | a0[QSFP_SUPPLY_VOLTAGE_OFFSET+1]; // yes
	param->tx_bias[0]  = (a0[QSFP_TX1_BIAS_OFFSET]<<8) | a0[QSFP_TX1_BIAS_OFFSET+1]; // yes
	param->tx_bias[1]  = (a0[QSFP_TX2_BIAS_OFFSET]<<8) | a0[QSFP_TX2_BIAS_OFFSET+1]; // yes
	param->tx_bias[2]  = (a0[QSFP_TX3_BIAS_OFFSET]<<8) | a0[QSFP_TX3_BIAS_OFFSET+1]; // yes
	param->tx_bias[3]  = (a0[QSFP_TX4_BIAS_OFFSET]<<8) | a0[QSFP_TX4_BIAS_OFFSET+1]; // yes
	param->tx_power[0] = (a0[QSFP_TX1_POWER_OFFSET]<<8) | a0[QSFP_TX1_POWER_OFFSET+1]; // yes
	param->tx_power[1] = (a0[QSFP_TX2_POWER_OFFSET]<<8) | a0[QSFP_TX2_POWER_OFFSET+1]; // yes
	param->tx_power[2] = (a0[QSFP_TX3_POWER_OFFSET]<<8) | a0[QSFP_TX3_POWER_OFFSET+1]; // yes
	param->tx_power[3] = (a0[QSFP_TX4_POWER_OFFSET]<<8) | a0[QSFP_TX4_POWER_OFFSET+1]; // yes
	param->rx_power[0] = (a0[QSFP_RX1_POWER_OFFSET]<<8) | a0[QSFP_RX1_POWER_OFFSET+1]; // yes
	param->rx_power[1] = (a0[QSFP_RX2_POWER_OFFSET]<<8) | a0[QSFP_RX2_POWER_OFFSET+1]; // yes
	param->rx_power[2] = (a0[QSFP_RX3_POWER_OFFSET]<<8) | a0[QSFP_RX3_POWER_OFFSET+1]; // yes
	param->rx_power[3] = (a0[QSFP_RX4_POWER_OFFSET]<<8) | a0[QSFP_RX4_POWER_OFFSET+1]; // yes
	
	return 0;
}

int npd_get_dump_sfp_info(unsigned long panel_port, int port_type, fiber_module_man_param_t *param)
{
	param->port_type = SFP_MISMATCH_ERROR;
	return 0;
}

#ifdef __cplusplus
}
#endif