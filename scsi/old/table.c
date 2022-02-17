#define VSCSI_NTYPES 20
#if VSCSI_STRICT
static unsigned char scsi_commands[VSCSI_NTYPES][32] = {
	{ /* 00 - Disk */
		0x99, 0x05, 0x24, 0x7C, 0x20, 0xC5, 0xB0, 0xD8, 
		0x02, 0x30, 0x27, 0xC4, 0x00, 0x00, 0x00, 0xC0, 
		0xF8, 0xFD, 0x0B, 0x00, 0x1F, 0xC5, 0xA0, 0xFC, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 01 - Tape */
		0x3B, 0x8D, 0x3F, 0x7E, 0x00, 0x08, 0x10, 0x18, 
		0x10, 0x30, 0x20, 0xC4, 0x00, 0x00, 0x00, 0x40, 
		0xDB, 0xB5, 0x0E, 0x00, 0x3D, 0x00, 0x20, 0x01, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 02 - Printer */
		0x19, 0x0C, 0xF5, 0x3C, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x00, 
		0x18, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 03 - Processor */
		0x09, 0x05, 0x04, 0x30, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 
		0xD8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 04 - WORM */
		0x89, 0x05, 0xE4, 0x7D, 0x20, 0xCD, 0x78, 0xDF, 
		0x01, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x00, 
		0xD8, 0xF5, 0x07, 0x00, 0xB9, 0xC5, 0x18, 0xFD, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 05 - CD/DVD */
		0x19, 0x08, 0x04, 0x48, 0x28, 0xDD, 0x20, 0x18, 
		0xEC, 0x4C, 0x3E, 0x3D, 0x00, 0x00, 0x00, 0x40, 
		0x20, 0x00, 0x00, 0x00, 0xFE, 0x3D, 0x60, 0xEE, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 06 - Scanner */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 07 - Optical */
		0x99, 0x05, 0xE4, 0x7D, 0x20, 0xFF, 0xF8, 0xFF, 
		0x01, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x00, 
		0xD8, 0xF5, 0x07, 0x00, 0xB9, 0xD5, 0x98, 0xFD, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 08 - Changer */
		0x89, 0x00, 0xE4, 0x7C, 0x00, 0x08, 0x80, 0x18, 
		0x00, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x40, 
		0xC0, 0x30, 0x00, 0x00, 0x79, 0x00, 0x60, 0xFD, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 09 - Communication */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 10 - Obsolete */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 11 - Obsolete */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 12 - Raid */
		0x09, 0x00, 0xE4, 0x3C, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x40, 
		0xC0, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0xFC, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 13 - Enclosure */
		0x09, 0x00, 0x24, 0x34, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0x20, 0xC4, 0x00, 0x00, 0x00, 0x40, 
		0xC0, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0xFC, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 14 - Simple Disk */
		0x09, 0x00, 0x04, 0x08, 0x20, 0x45, 0x20, 0x08, 
		0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x00, 0x40, 
		0xE0, 0xF5, 0x03, 0x00, 0x1A, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 15 - Card Reader */
		0x09, 0x00, 0xE4, 0x7C, 0x20, 0x4D, 0x70, 0x19, 
		0x00, 0x30, 0x20, 0x04, 0x00, 0x00, 0x00, 0x00, 
		0xD8, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 16 - Bridge */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 17 - OSD */
		0x09, 0x00, 0x04, 0x78, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0x20, 0xC4, 0x00, 0x00, 0x00, 0x80, 
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 18 - Automation */
		0x09, 0x00, 0x24, 0x3C, 0x00, 0x00, 0x00, 0x18, 
		0x10, 0x30, 0x20, 0x04, 0x00, 0x00, 0x00, 0x40, 
		0xD8, 0x30, 0x00, 0x80, 0x1D, 0x0A, 0x20, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 19 - Security */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
};
#endif

typedef int (*vscsi_opfunc_t)(struct vscsi_device *, struct scsi_cmnd *);

#define VSCSI_MAX_OP 0xbf
struct vscsi_op {
	char *name;
	vscsi_opfunc_t func;
} vscsi_ops[VSCSI_MAX_OP+1] = {
	{ "test_unit_ready",test_unit_ready },						/* 00 */
	{ "rewind",rewind },							/* 01 */
	{ 0,0 },								/* 02 */
	{ "request_sense",request_sense },						/* 03 */
	{ "format_unit",format_unit },						/* 04 */
	{ "read_block_limits",read_block_limits },						/* 05 */
	{ 0,0 },								/* 06 */
	{ "reassign_blocks",reassign_blocks },						/* 07 */
	{ "read_6",read_6 },							/* 08 */
	{ 0,0 },								/* 09 */
	{ "write_6",write_6 },							/* 0a */
	{ "seek_6",seek_6 },							/* 0b */
	{ 0,0 },								/* 0c */
	{ 0,0 },								/* 0d */
	{ 0,0 },								/* 0e */
	{ "read_reverse_6",read_reverse_6 },						/* 0f */
	{ "write_filemarks_6",write_filemarks_6 },						/* 10 */
	{ "space_6",space_6 },							/* 11 */
	{ "inquiry",inquiry },							/* 12 */
	{ "verify_6",verify_6 },							/* 13 */
	{ "recover_buffered_data",recover_buffered_data },					/* 14 */
	{ "mode_select_6",mode_select_6 },						/* 15 */
	{ "reserve_6",reserve_6 },							/* 16 */
	{ "release_6",release_6 },							/* 17 */
	{ "copy",copy },							/* 18 */
	{ "erase_6",erase_6 },							/* 19 */
	{ "mode_sense_6",mode_sense_6 },						/* 1a */
	{ "start_stop_unit",start_stop_unit },						/* 1b */
	{ "receive_diagnostic_results",receive_diagnostic_results },					/* 1c */
	{ "send_diagnostic",send_diagnostic },						/* 1d */
	{ "prevent_allow_medium_removal",prevent_allow_medium_removal },				/* 1e */
	{ 0,0 },								/* 1f */
	{ 0,0 },								/* 20 */
	{ 0,0 },								/* 21 */
	{ 0,0 },								/* 22 */
	{ "read_format_capacities",read_format_capacities },					/* 23 */
	{ 0,0 },								/* 24 */
	{ "read_capacity_10",read_capacity_10 },						/* 25 */
	{ 0,0 },								/* 26 */
	{ 0,0 },								/* 27 */
	{ "read_10",read_10 },							/* 28 */
	{ "read_generation",read_generation },						/* 29 */
	{ "write_10",write_10 },							/* 2a */
	{ "seek_10",seek_10 },							/* 2b */
	{ "erase_10",erase_10 },							/* 2c */
	{ "read_updated_block",read_updated_block },						/* 2d */
	{ "write_and_verify_10",write_and_verify_10 },					/* 2e */
	{ "verify_10",verify_10 },							/* 2f */
	{ 0,0 },								/* 30 */
	{ 0,0 },								/* 31 */
	{ 0,0 },								/* 32 */
	{ "set_limits_10",set_limits_10 },						/* 33 */
	{ "pre_fetch_10",pre_fetch_10 },						/* 34 */
	{ "synchronize_cache_10",synchronize_cache_10 },					/* 35 */
	{ "lock_unlock_cache_10",lock_unlock_cache_10 },					/* 36 */
	{ "read_defect_data_10",read_defect_data_10 },					/* 37 */
	{ "medium_scan",medium_scan },						/* 38 */
	{ "compare",compare },							/* 39 */
	{ "copy_and_verify",copy_and_verify },						/* 3a */
	{ "write_buffer",write_buffer },						/* 3b */
	{ "read_buffer",read_buffer },						/* 3c */
	{ "update_block",update_block },						/* 3d */
	{ "read_long_10",read_long_10 },						/* 3e */
	{ "write_long_10",write_long_10 },						/* 3f */
	{ "change_definition",change_definition },						/* 40 */
	{ "write_same_10",write_same_10 },						/* 41 */
	{ "read_sub_channel",read_sub_channel },						/* 42 */
	{ "read_toc_pma_atip",read_toc_pma_atip },						/* 43 */
	{ "report_density_support",report_density_support },					/* 44 */
	{ "play_audio_10",play_audio_10 },						/* 45 */
	{ "get_configuration",get_configuration },						/* 46 */
	{ "play_audio_msf",play_audio_msf },						/* 47 */
	{ 0,0 },								/* 48 */
	{ 0,0 },								/* 49 */
	{ "get_event_status_notification",get_event_status_notification },				/* 4a */
	{ "pause_resume",pause_resume },						/* 4b */
	{ "log_select",log_select },							/* 4c */
	{ "log_sense",log_sense },							/* 4d */
	{ "stop_play_scan",stop_play_scan },						/* 4e */
	{ 0,0 },								/* 4f */
	{ "xdwrite_10",xdwrite_10 },							/* 50 */
	{ "xpwrite_10",xpwrite_10 },							/* 51 */
	{ "xdread_10",xdread_10 },							/* 52 */
	{ "reserve_track",reserve_track },						/* 53 */
	{ "send_opc_information",send_opc_information },					/* 54 */
	{ "mode_select_10",mode_select_10 },						/* 55 */
	{ "reserve_10",reserve_10 },							/* 56 */
	{ "release_10",release_10 },							/* 57 */
	{ "repair_track",repair_track },						/* 58 */
	{ 0,0 },								/* 59 */
	{ "mode_sense_10",mode_sense_10 },						/* 5a */
	{ "close_track_session",close_track_session },					/* 5b */
	{ "read_buffer_capacity",read_buffer_capacity },					/* 5c */
	{ "send_cue_sheet",send_cue_sheet },						/* 5d */
	{ "persistent_reserve_in",persistent_reserve_in },					/* 5e */
	{ "persistent_reserve_out",persistent_reserve_out },					/* 5f */
	{ 0,0 },								/* 60 */
	{ 0,0 },								/* 61 */
	{ 0,0 },								/* 62 */
	{ 0,0 },								/* 63 */
	{ 0,0 },								/* 64 */
	{ 0,0 },								/* 65 */
	{ 0,0 },								/* 66 */
	{ 0,0 },								/* 67 */
	{ 0,0 },								/* 68 */
	{ 0,0 },								/* 69 */
	{ 0,0 },								/* 6a */
	{ 0,0 },								/* 6b */
	{ 0,0 },								/* 6c */
	{ 0,0 },								/* 6d */
	{ 0,0 },								/* 6e */
	{ 0,0 },								/* 6f */
	{ 0,0 },								/* 70 */
	{ 0,0 },								/* 71 */
	{ 0,0 },								/* 72 */
	{ 0,0 },								/* 73 */
	{ 0,0 },								/* 74 */
	{ 0,0 },								/* 75 */
	{ 0,0 },								/* 76 */
	{ 0,0 },								/* 77 */
	{ 0,0 },								/* 78 */
	{ 0,0 },								/* 79 */
	{ 0,0 },								/* 7a */
	{ 0,0 },								/* 7b */
	{ 0,0 },								/* 7c */
	{ 0,0 },								/* 7d */
	{ "extended_cdb",extended_cdb },						/* 7e */
	{ "variable_length_cdb",variable_length_cdb },					/* 7f */
	{ "write_filemarks_16",write_filemarks_16 },						/* 80 */
	{ "read_reverse_16",read_reverse_16 },						/* 81 */
	{ 0,0 },								/* 82 */
	{ "extended_copy",extended_copy },						/* 83 */
	{ "receive_copy_results",receive_copy_results },					/* 84 */
	{ "ata_command_pass_through_16",ata_command_pass_through_16 },				/* 85 */
	{ "access_control_in",access_control_in },						/* 86 */
	{ "access_control_out",access_control_out },						/* 87 */
	{ "read_16",read_16 },							/* 88 */
	{ 0,0 },								/* 89 */
	{ "write_16",write_16 },							/* 8a */
	{ "orwrite",orwrite },							/* 8b */
	{ "read_attribute",read_attribute },						/* 8c */
	{ "write_attribute",write_attribute },						/* 8d */
	{ "write_and_verify_16",write_and_verify_16 },					/* 8e */
	{ "verify_16",verify_16 },							/* 8f */
	{ "pre_fetch_16",pre_fetch_16 },						/* 90 */
	{ "synchronize_cache_16",synchronize_cache_16 },					/* 91 */
	{ "lock_unlock_cache_16",lock_unlock_cache_16 },					/* 92 */
	{ "write_same_16",write_same_16 },						/* 93 */
	{ 0,0 },								/* 94 */
	{ 0,0 },								/* 95 */
	{ 0,0 },								/* 96 */
	{ 0,0 },								/* 97 */
	{ 0,0 },								/* 98 */
	{ 0,0 },								/* 99 */
	{ 0,0 },								/* 9a */
	{ 0,0 },								/* 9b */
	{ 0,0 },								/* 9c */
	{ 0,0 },								/* 9d */
	{ 0,0 },								/* 9e */
	{ "service_action_out_16",service_action_out_16 },					/* 9f */
	{ "report_luns",report_luns },						/* a0 */
	{ "blank",blank },							/* a1 */
	{ "security_protocol_in",security_protocol_in },					/* a2 */
	{ "maintenance_in",maintenance_in },						/* a3 */
	{ "maintenance_out",maintenance_out },						/* a4 */
	{ "move_medium",move_medium },						/* a5 */
	{ "exchange_medium",exchange_medium },						/* a6 */
	{ "move_medium_attached",move_medium_attached },					/* a7 */
	{ "read_12",read_12 },							/* a8 */
	{ "service_action_out_12",service_action_out_12 },					/* a9 */
	{ "write_12",write_12 },							/* aa */
	{ "service_action_in_12",service_action_in_12 },					/* ab */
	{ "erase_12",erase_12 },							/* ac */
	{ "read_dvd_structure",read_dvd_structure },						/* ad */
	{ "write_and_verify_12",write_and_verify_12 },					/* ae */
	{ "verify_12",verify_12 },							/* af */
	{ 0,0 },								/* b0 */
	{ 0,0 },								/* b1 */
	{ 0,0 },								/* b2 */
	{ "set_limits_12",set_limits_12 },						/* b3 */
	{ "read_element_status_attached",read_element_status_attached },				/* b4 */
	{ "security_protocol_out",security_protocol_out },					/* b5 */
	{ "send_volume_tag",send_volume_tag },						/* b6 */
	{ "read_defect_data_12",read_defect_data_12 },					/* b7 */
	{ "read_element_status",read_element_status },					/* b8 */
	{ "read_cd_msf",read_cd_msf },						/* b9 */
	{ "redundancy_group_in",redundancy_group_in },					/* ba */
	{ "redundancy_group_out",redundancy_group_out },					/* bb */
	{ "spare_in",spare_in },							/* bc */
	{ "spare_out",spare_out },							/* bd */
	{ "volume_set_in",volume_set_in },						/* be */
	{ "volume_set_out",volume_set_out },						/* bf */
};
