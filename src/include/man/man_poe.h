#ifndef _MAN_POE_H_
#define _MAN_POE_H_

#define POE_SHOW_TIME_SIZE (128 * 1024)

int dcli_global_poe_endis(unsigned char isEnable);

int dcli_port_poe_endis(unsigned int port_index, unsigned char isEnable);

int dcli_poe_max_power_config(unsigned int port_index, unsigned int max_power);

int dcli_poe_priority_config(unsigned int port_index, unsigned char port_poe_priority);

int dcli_poe_power_manage_mode_config(unsigned char poe_power_manage);

int dcli_poe_interface_mode_config(unsigned int port_index, unsigned char poe_mode);

int dcli_port_poe_config_legacy_check(unsigned int netif_index, unsigned char is_enable);

int dcli_get_poe_interface(unsigned int port_index, poe_intf_db_t *poe_intf);

int dcli_poe_get_next_interface(unsigned int port_index, poe_intf_db_t *poe_intf);

/*目前盒式设备只有一个PSE设备，移植到机架时需要修改*/
int dcli_get_poe_pse(unsigned int pse_id, poe_pse_db_t *poe_pse);

#endif

