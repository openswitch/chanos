#ifdef __cplusplus
extern "C"
{
#endif

long as6612c_npd_os_upgrade(unsigned int slot_index)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);					
	return NPD_SUCCESS;
}

long as6612c_board_ready_config(unsigned int slot_index)
{	
	return chasm_board_ready_config(slot_index);
}

ipp_fix_param_t  as6612c_ipp_param =
{
    .ipp_portnum = 2,
    .ipp_phyport_map = {"eth0", "eth1"},
    .ipp_board_map = {-1, -2}
};

sub_board_fix_param_t as6612c_subboard_param = 
{
    .sub_slotnum = 1,     // more than 1
    .sub_slot_portnum = 0
};

temper_fix_param_t as6612c_temper_param =
{
    .num = 0,
    .name = {"", ""}
};

board_feature_t as6612c_feature = {0};

int as6612c_support_product[] =
{
    PRODUCT_T9010,
    PRODUCT_T9006,
    PRODUCT_T9003,
    0
};


board_fix_param_t as6612c_param =
{
    .board_code = PPAL_BOARD_HWCODE_AS6612C,
    .board_type = PPAL_BOARD_TYPE_AS6612C,
    .full_name = "AUTELAN AS SERIES MULTI-LAYER SWITCH Wireless AC BOARD",
    .short_name = "ASG6612C-WAC",

    .have_pp = TRUE,
    .master_flag = FALSE,
    .service_flag = SERVICE_AS_INDEPENDENT_SYSTEM,
    .panel_portnum = 0,	/* */

    .ipp_fix_param = &as6612c_ipp_param,
    .subboard_fix_param = &as6612c_subboard_param,
    .temper_fix_param = &as6612c_temper_param,
    .os_upgrade = &as6612c_npd_os_upgrade,
    .board_ready_config = &as6612c_board_ready_config,
    .feature = &as6612c_feature,
    .sdk_type = SDK_MARVELL,
    .board_support_product = (int*)as6612c_support_product
};

#ifdef __cplusplus
extern "C"
}
#endif

