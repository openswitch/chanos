#ifndef _MAN_SFLOW_H_
#define _MAN_SFLOW_H_

#define SFLOW_DBUS_NO_AGTIP  	"nosflowagtip"
#define SFLOW_DBUS_NO_CLTIP  	"nosflowcltip"
#define SFLOW_DBUS_AGTIP_SET  	"sflowagtip"
#define SFLOW_DBUS_CLTIP_SET  	"sflowcltip"
#define SFLOW_DBUS_SAMPLER_SET  "sflowsampler"
#define SFLOW_DBUS_POLLER_SET 	"sflowpoller"

#define SFLOW_DBUS_BUSNAME "aw.sflow"
#define SFLOW_DBUS_OBJPATH "/aw/sflow"
#define SFLOW_DBUS_INTERFACE "aw.sflow"
#define SFLOW_CLT_NUM 3
#define SFLOW_DEFAULT_SAMPLING_RATE 100000

typedef struct sflport_info_s {
	unsigned int netif_index; 
	unsigned int is_enable;
	unsigned int sampling_rate;
}sflport_info_t;

typedef struct sfl_clt_s {
	unsigned int index; 
	unsigned long clt_ip; 
	unsigned int clt_port;
}sfl_clt_t;

int man_sflow_show_agt(unsigned long *glb_info);
int man_sflow_show_clt(sfl_clt_t *sfl_clt, unsigned int index);
int man_sflow_show_port(sflport_info_t *port_info, unsigned int ifindex, int num);

int man_sflow_endis(unsigned int netif_index, unsigned int is_enable);

int man_sflow_agtip(unsigned long agt_ip);
int man_sflow_agtip_protocol(unsigned long agt_ip);

int man_sflow_cltip(unsigned long clt_ip, unsigned int port, unsigned int clt_index);
int man_sflow_cltip_protocol(unsigned long clt_ip, unsigned int port, unsigned int clt_index);

int man_sflow_samplingrate_config(unsigned int netif_index, unsigned int rate);
int man_sflow_samplingrate_to_protocol(unsigned int netif_index, unsigned int rate);

int man_no_sflow_agtip(void);
int man_no_sflow_cltip(unsigned int clt_index);

int man_no_sflow_agtip_protocol(void);
int man_no_sflow_cltip_protocol(unsigned int clt_index);

#endif
