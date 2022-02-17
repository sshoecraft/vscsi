static unsigned char scsi_device_commands[16][32] = {
	{ /* 00 */
		0x99, 0x05, 0x24, 0x7C, 0x20, 0xC5, 0xB0, 0xD8, 
		0x02, 0x30, 0x27, 0xC4, 0x00, 0x00, 0x00, 0xC0, 
		0xF8, 0xFD, 0x0B, 0x00, 0x1F, 0xC5, 0xA0, 0xFC, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 01 */
		0x3B, 0x8D, 0x3F, 0x7E, 0x00, 0x08, 0x10, 0x18, 
		0x10, 0x30, 0x20, 0xC4, 0x00, 0x00, 0x00, 0x40, 
		0xDB, 0xB5, 0x0E, 0x00, 0x3D, 0x00, 0x20, 0x01, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 02 */
		0x19, 0x0C, 0xF5, 0x3C, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x00, 
		0x18, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 03 */
		0x09, 0x05, 0x04, 0x30, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 
		0xD8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 04 */
		0x89, 0x05, 0xE4, 0x7D, 0x20, 0xCD, 0x78, 0xDF, 
		0x01, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x00, 
		0xD8, 0xF5, 0x07, 0x00, 0xB9, 0xC5, 0x18, 0xFD, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 05 */
		0x19, 0x08, 0x04, 0x48, 0x28, 0xDD, 0x20, 0x18, 
		0xEC, 0x4C, 0x3E, 0x3D, 0x00, 0x00, 0x00, 0x40, 
		0x20, 0x00, 0x00, 0x00, 0xFE, 0x3D, 0x60, 0xEE, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 06 */
		0x99, 0x05, 0xE4, 0x7D, 0x20, 0xFF, 0xF8, 0xFF, 
		0x01, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x00, 
		0xD8, 0xF5, 0x07, 0x00, 0xB9, 0xD5, 0x98, 0xFD, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 07 */
		0x89, 0x00, 0xE4, 0x7C, 0x00, 0x08, 0x80, 0x18, 
		0x00, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x40, 
		0xC0, 0x30, 0x00, 0x00, 0x79, 0x00, 0x60, 0xFD, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 08 */
		0x09, 0x00, 0xE4, 0x3C, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x40, 
		0xC0, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0xFC, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 09 */
		0x09, 0x00, 0x24, 0x34, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0x20, 0xC4, 0x00, 0x00, 0x00, 0x40, 
		0xC0, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0xFC, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 10 */
		0x09, 0x00, 0x04, 0x08, 0x20, 0x45, 0x20, 0x08, 
		0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x00, 0x40, 
		0xE0, 0xF5, 0x03, 0x00, 0x1A, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 11 */
		0x09, 0x00, 0xE4, 0x7C, 0x20, 0x4D, 0x70, 0x19, 
		0x00, 0x30, 0x20, 0x04, 0x00, 0x00, 0x00, 0x00, 
		0xD8, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 12 */
		0x09, 0x00, 0x24, 0x3C, 0x00, 0x00, 0x00, 0x18, 
		0x10, 0x30, 0x20, 0x04, 0x00, 0x00, 0x00, 0x40, 
		0xD8, 0x30, 0x00, 0x80, 0x1D, 0x0A, 0x20, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 13 */
		0x09, 0x00, 0x04, 0x78, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0x20, 0xC4, 0x00, 0x00, 0x00, 0x80, 
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
};

extern void test_unit_ready(void);
extern void rewind(void);
extern void request_sense(void);
extern void format(void);
extern void read_block_limits(void);
extern void initialize_element_status(void);
extern void receive(void);
extern void print(void);
extern void slew_and_print(void);
extern void read_reverse_6(void);
extern void synchronize_buffer(void);
extern void space_6(void);
extern void inquiry(void);
extern void verify_6(void);
extern void recover_buffered_data(void);
extern void mode_select_6(void);
extern void reserve_6(void);
extern void release_6(void);
extern void copy(void);
extern void erase_6(void);
extern void mode_sense_6(void);
extern void open_close_import_export_element(void);
extern void receive_diagnostic_results(void);
extern void send_diagnostic(void);
extern void prevent_allow_medium_removal(void);
extern void read_format_capacities(void);
extern void read_card_capacity(void);
extern void read_10(void);
extern void read_generation(void);
extern void write_10(void);
extern void position_to_element(void);
extern void erase_10(void);
extern void read_updated_block(void);
extern void write_and_verify_10(void);
extern void verify_10(void);
extern void set_limits_10(void);
extern void read_position(void);
extern void synchronize_cache_10(void);
extern void lock_unlock_cache_10(void);
extern void initialize_element_status_with_range(void);
extern void medium_scan(void);
extern void compare(void);
extern void copy_and_verify(void);
extern void write_buffer(void);
extern void read_buffer(void);
extern void update_block(void);
extern void read_long_10(void);
extern void write_long_10(void);
extern void change_definition(void);
extern void write_same_10(void);
extern void read_sub_channel(void);
extern void read_toc_pma_atip(void);
extern void report_density_support(void);
extern void play_audio_10(void);
extern void get_configuration(void);
extern void play_audio_msf(void);
extern void get_event_status_notification(void);
extern void pause_resume(void);
extern void log_select(void);
extern void log_sense(void);
extern void stop_play_scan(void);
extern void xdwrite_10(void);
extern void read_disc_information(void);
extern void read_track_information(void);
extern void reserve_track(void);
extern void send_opc_information(void);
extern void mode_select_10(void);
extern void reserve_10(void);
extern void release_10(void);
extern void repair_track(void);
extern void mode_sense_10(void);
extern void close_track_session(void);
extern void read_buffer_capacity(void);
extern void send_cue_sheet(void);
extern void persistent_reserve_in(void);
extern void persistent_reserve_out(void);
extern void extended_cdb(void);
extern void variable_length_cdb(void);
extern void write_filemarks_16(void);
extern void read_reverse_16(void);
extern void extended_copy(void);
extern void receive_copy_results(void);
extern void ata_command_pass_through_16(void);
extern void access_control_in(void);
extern void access_control_out(void);
extern void read_16(void);
extern void write_16(void);
extern void orwrite(void);
extern void read_attribute(void);
extern void write_attribute(void);
extern void write_and_verify_16(void);
extern void verify_16(void);
extern void pre_fetch_16(void);
extern void space_16(void);
extern void locate_16(void);
extern void erase_16(void);
extern void service_action_out_16(void);
extern void report_luns(void);
extern void ata_command_pass_through_12(void);
extern void security_protocol_in(void);
extern void send_key(void);
extern void report_key(void);
extern void play_audio_12(void);
extern void load_unload_c_dvd(void);
extern void set_read_ahead(void);
extern void read_12(void);
extern void service_action_out_12(void);
extern void write_12(void);
extern void service_action_in_12(void);
extern void get_performance(void);
extern void read_dvd_structure(void);
extern void write_and_verify_12(void);
extern void verify_12(void);
extern void set_limits_12(void);
extern void read_element_status_attached(void);
extern void request_volume_element_address(void);
extern void set_streaming(void);
extern void read_defect_data_12(void);
extern void read_element_status(void);
extern void read_cd_msf(void);
extern void scan(void);
extern void set_cd_speed(void);
extern void spare_in(void);
extern void mechanism_status(void);
extern void read_cd(void);
extern void send_dvd_structure(void);

typedef void (*scsi_funcptr_t)(void);

#define MAX_FUNC 0xbf
static scsi_funcptr_t scsi_funcs[MAX_FUNC] = {
	test_unit_ready,			/* 00 */
	rewind,					/* 01 */
	0,					/* 02 */
	request_sense,				/* 03 */
	format,					/* 04 */
	read_block_limits,			/* 05 */
	0,					/* 06 */
	initialize_element_status,		/* 07 */
	receive,				/* 08 */
	0,					/* 09 */
	print,					/* 0a */
	slew_and_print,				/* 0b */
	0,					/* 0c */
	0,					/* 0d */
	0,					/* 0e */
	read_reverse_6,				/* 0f */
	synchronize_buffer,			/* 10 */
	space_6,				/* 11 */
	inquiry,				/* 12 */
	verify_6,				/* 13 */
	recover_buffered_data,			/* 14 */
	mode_select_6,				/* 15 */
	reserve_6,				/* 16 */
	release_6,				/* 17 */
	copy,					/* 18 */
	erase_6,				/* 19 */
	mode_sense_6,				/* 1a */
	open_close_import_export_element,	/* 1b */
	receive_diagnostic_results,		/* 1c */
	send_diagnostic,			/* 1d */
	prevent_allow_medium_removal,		/* 1e */
	0,					/* 1f */
	0,					/* 20 */
	0,					/* 21 */
	0,					/* 22 */
	read_format_capacities,			/* 23 */
	0,					/* 24 */
	read_card_capacity,			/* 25 */
	0,					/* 26 */
	0,					/* 27 */
	read_10,				/* 28 */
	read_generation,			/* 29 */
	write_10,				/* 2a */
	position_to_element,			/* 2b */
	erase_10,				/* 2c */
	read_updated_block,			/* 2d */
	write_and_verify_10,			/* 2e */
	verify_10,				/* 2f */
	0,					/* 30 */
	0,					/* 31 */
	0,					/* 32 */
	set_limits_10,				/* 33 */
	read_position,				/* 34 */
	synchronize_cache_10,			/* 35 */
	lock_unlock_cache_10,			/* 36 */
	initialize_element_status_with_range,	/* 37 */
	medium_scan,				/* 38 */
	compare,				/* 39 */
	copy_and_verify,			/* 3a */
	write_buffer,				/* 3b */
	read_buffer,				/* 3c */
	update_block,				/* 3d */
	read_long_10,				/* 3e */
	write_long_10,				/* 3f */
	change_definition,			/* 40 */
	write_same_10,				/* 41 */
	read_sub_channel,			/* 42 */
	read_toc_pma_atip,			/* 43 */
	report_density_support,			/* 44 */
	play_audio_10,				/* 45 */
	get_configuration,			/* 46 */
	play_audio_msf,				/* 47 */
	0,					/* 48 */
	0,					/* 49 */
	get_event_status_notification,		/* 4a */
	pause_resume,				/* 4b */
	log_select,				/* 4c */
	log_sense,				/* 4d */
	stop_play_scan,				/* 4e */
	0,					/* 4f */
	xdwrite_10,				/* 50 */
	read_disc_information,			/* 51 */
	read_track_information,			/* 52 */
	reserve_track,				/* 53 */
	send_opc_information,			/* 54 */
	mode_select_10,				/* 55 */
	reserve_10,				/* 56 */
	release_10,				/* 57 */
	repair_track,				/* 58 */
	0,					/* 59 */
	mode_sense_10,				/* 5a */
	close_track_session,			/* 5b */
	read_buffer_capacity,			/* 5c */
	send_cue_sheet,				/* 5d */
	persistent_reserve_in,			/* 5e */
	persistent_reserve_out,			/* 5f */
	0,					/* 60 */
	0,					/* 61 */
	0,					/* 62 */
	0,					/* 63 */
	0,					/* 64 */
	0,					/* 65 */
	0,					/* 66 */
	0,					/* 67 */
	0,					/* 68 */
	0,					/* 69 */
	0,					/* 6a */
	0,					/* 6b */
	0,					/* 6c */
	0,					/* 6d */
	0,					/* 6e */
	0,					/* 6f */
	0,					/* 70 */
	0,					/* 71 */
	0,					/* 72 */
	0,					/* 73 */
	0,					/* 74 */
	0,					/* 75 */
	0,					/* 76 */
	0,					/* 77 */
	0,					/* 78 */
	0,					/* 79 */
	0,					/* 7a */
	0,					/* 7b */
	0,					/* 7c */
	0,					/* 7d */
	extended_cdb,				/* 7e */
	variable_length_cdb,			/* 7f */
	write_filemarks_16,			/* 80 */
	read_reverse_16,			/* 81 */
	0,					/* 82 */
	extended_copy,				/* 83 */
	receive_copy_results,			/* 84 */
	ata_command_pass_through_16,		/* 85 */
	access_control_in,			/* 86 */
	access_control_out,			/* 87 */
	read_16,				/* 88 */
	0,					/* 89 */
	write_16,				/* 8a */
	orwrite,				/* 8b */
	read_attribute,				/* 8c */
	write_attribute,			/* 8d */
	write_and_verify_16,			/* 8e */
	verify_16,				/* 8f */
	pre_fetch_16,				/* 90 */
	space_16,				/* 91 */
	locate_16,				/* 92 */
	erase_16,				/* 93 */
	0,					/* 94 */
	0,					/* 95 */
	0,					/* 96 */
	0,					/* 97 */
	0,					/* 98 */
	0,					/* 99 */
	0,					/* 9a */
	0,					/* 9b */
	0,					/* 9c */
	0,					/* 9d */
	0,					/* 9e */
	service_action_out_16,			/* 9f */
	report_luns,				/* a0 */
	ata_command_pass_through_12,		/* a1 */
	security_protocol_in,			/* a2 */
	send_key,				/* a3 */
	report_key,				/* a4 */
	play_audio_12,				/* a5 */
	load_unload_c_dvd,			/* a6 */
	set_read_ahead,				/* a7 */
	read_12,				/* a8 */
	service_action_out_12,			/* a9 */
	write_12,				/* aa */
	service_action_in_12,			/* ab */
	get_performance,			/* ac */
	read_dvd_structure,			/* ad */
	write_and_verify_12,			/* ae */
	verify_12,				/* af */
	0,					/* b0 */
	0,					/* b1 */
	0,					/* b2 */
	set_limits_12,				/* b3 */
	read_element_status_attached,		/* b4 */
	request_volume_element_address,		/* b5 */
	set_streaming,				/* b6 */
	read_defect_data_12,			/* b7 */
	read_element_status,			/* b8 */
	read_cd_msf,				/* b9 */
	scan,					/* ba */
	set_cd_speed,				/* bb */
	spare_in,				/* bc */
	mechanism_status,			/* bd */
	read_cd,				/* be */
	send_dvd_structure,			/* bf */
};
