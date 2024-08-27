#ifndef _DGILIB_H_
#define _DGILIB_H_

#include <stdint.h>
#include <stddef.h>

// Types
typedef uint32_t dgi_handle_t;
typedef uint32_t power_handle_t;
typedef void (*DeviceStatusChangedCallBack)(const char*, const char*, bool);

#ifdef _WIN32
	#if defined DLL_EXPORT
	#define DECLDIR __declspec(dllexport)
	#else
	#define DECLDIR __declspec(dllimport)
	#endif
#else
	#define DECLDIR
#endif
 
extern "C"
{
	// Discovery commands
	DECLDIR void Initialize(uint32_t* handlep);
	DECLDIR void UnInitialize(uint32_t handle);
	DECLDIR void initialize_status_change_notification(uint32_t* handlep);
	DECLDIR void uninitialize_status_change_notification(uint32_t handle);

	DECLDIR void RegisterForDeviceStatusChangeNotifications(uint32_t handle, DeviceStatusChangedCallBack edbgDeviceStatusChangedCallBack);
	DECLDIR void UnRegisterForDeviceStatusChangeNotifications(uint32_t handle, DeviceStatusChangedCallBack edbgDeviceStatusChangedCallBack);
	DECLDIR void register_for_device_status_change_notifications(uint32_t handle, DeviceStatusChangedCallBack deviceStatusChangedCallBack);
	DECLDIR void unregister_for_device_status_change_notifications(uint32_t handle, DeviceStatusChangedCallBack deviceStatusChangedCallBack);
	
	DECLDIR void discover(void);

	DECLDIR int get_device_count(void);
	DECLDIR int get_device_serial(int index, char* sn);
	DECLDIR int get_device_name(int index, char* name);
	DECLDIR int get_gpio_map(uint32_t dgi_hndl, uint8_t* gpio_map);
	
	DECLDIR int is_msd_mode(char* serial_number);
	DECLDIR int set_mode(char* sn, int nmbed);

	// Connection commands
	DECLDIR int connection_status(uint32_t dgi_hndl);
	DECLDIR int connect(char* sn, uint32_t* dgi_hndl);
	DECLDIR int disconnect(uint32_t dgi_hndl);

	DECLDIR int start_polling(uint32_t dgi_hndl);
	DECLDIR int stop_polling(uint32_t dgi_hndl);
	
	DECLDIR int get_fw_version(uint32_t dgi_hndl, unsigned char* major, unsigned char* minor);

	// Device control
	DECLDIR int target_reset(uint32_t dgi_hndl, bool hold_reset);
	
	// Version information
	DECLDIR int get_major_version();
	DECLDIR int get_minor_version();
	DECLDIR int get_build_number();

	// Interface commands
	DECLDIR int interface_list(uint32_t dgi_hndl, unsigned char* interfaces, unsigned char* count);
	DECLDIR int interface_enable(uint32_t dgi_hndl, int interface_id, bool timestamp);
	DECLDIR int interface_disable(uint32_t dgi_hndl, int interface_id);
	DECLDIR int interface_set_configuration(uint32_t dgi_hndl, int interface_id, unsigned int* config_id, unsigned int* config_value, unsigned int config_cnt);
	DECLDIR int interface_get_configuration(uint32_t dgi_hndl, int interface_id, unsigned int* config_id, unsigned int* config_value, unsigned int* config_cnt);
	DECLDIR int interface_clear_buffer(uint32_t dgi_hndl, int interface_id);
	DECLDIR int interface_read_data(uint32_t dgi_hndl, int interface_id, 
						unsigned char* buffer, unsigned long long* timestamp, int* length, 
						unsigned int* ovf_index, unsigned int* ovf_length, unsigned int* ovf_entry_count);
	DECLDIR int interface_write_data(uint32_t dgi_hndl, int interface_id, unsigned char* buffer, int* length);
	DECLDIR int interface_get_health(uint32_t dgi_hndl, int interface_id, unsigned int* health);
	
	// Auxiliary commands
	DECLDIR int auxiliary_power_initialize(dgi_handle_t *handle, uint32_t dgi_hndl);
	DECLDIR int auxiliary_power_uninitialize(dgi_handle_t handle);

	DECLDIR int auxiliary_power_start(dgi_handle_t handle, int mode, int parameter);
	DECLDIR int auxiliary_power_stop(dgi_handle_t handle);

	DECLDIR int auxiliary_power_get_status(dgi_handle_t handle);

	DECLDIR int auxiliary_power_trigger_calibration(dgi_handle_t handle, int type);
	DECLDIR int auxiliary_power_get_circuit_type(dgi_handle_t handle, int* circuit);
	DECLDIR int auxiliary_power_get_calibration(dgi_handle_t handle, uint8_t data[], size_t length);
	DECLDIR bool auxiliary_power_calibration_is_valid(dgi_handle_t handle);

	DECLDIR int auxiliary_power_register_buffer_pointers(dgi_handle_t handle, float* buffer, double* timestamp, size_t* length, size_t max_length, int channel, int type);
	DECLDIR int auxiliary_power_unregister_buffer_pointers(dgi_handle_t handle, int channel, int type);
	DECLDIR int auxiliary_power_copy_data(dgi_handle_t handle, float* buffer, double* timestamp, size_t* length, size_t max_length, int channel, int type);
	DECLDIR int auxiliary_power_lock_data_for_reading(dgi_handle_t handle);
	DECLDIR int auxiliary_power_free_data(dgi_handle_t handle);

	// DEPRECATED COMMANDS
	DECLDIR int start_session(uint32_t dgi_hndl);
	DECLDIR int stop_session(uint32_t dgi_hndl);

	DECLDIR int clear_buffer(uint32_t dgi_hndl, int intf);

	DECLDIR int is_mbed_kit(char* serial_number);

	DECLDIR int enable_interface(uint32_t dgi_hndl, int interface_id);
	DECLDIR int disable_interface(uint32_t dgi_hndl, int interface_id);
	DECLDIR int clear_interface_buffer(uint32_t dgi_hndl, int interface_id);
	DECLDIR int read_interface_data(uint32_t dgi_hndl, int interface_id, unsigned char* buffer, int* length, unsigned int* ovf_index, unsigned int* ovf_entry_count, unsigned int* health);
	DECLDIR int read_interface_timestamp(uint32_t dgi_hndl, int interface_id, unsigned long long* timestamp, int* length);
	DECLDIR int write_interface_data(uint32_t dgi_hndl, int interface_id, unsigned char* buffer, int* length);

	DECLDIR int enable_power(uint32_t dgi_hndl, int buffer_size);
	DECLDIR int disable_power(uint32_t dgi_hndl);
	DECLDIR int clear_power(uint32_t dgi_hndl);
	DECLDIR int poll_power(uint32_t dgi_hndl, unsigned char* buffer, int* length, unsigned int* ovf_index, unsigned int* ovf_length, unsigned int* ovf_entry_count, unsigned int* health);
	DECLDIR int poll_power_sync(uint32_t dgi_hndl, unsigned char* buffer, unsigned long long* timestamp, int* length);
	DECLDIR int get_power_calibration(uint32_t dgi_hndl, unsigned char* buffer, int* length);

	DECLDIR int enable_spi(uint32_t dgi_hndl, int buffer_size, unsigned char char_length, unsigned char mode, bool forcesync, bool timestamp);
	DECLDIR int disable_spi(uint32_t dgi_hndl);
	DECLDIR int clear_spi(uint32_t dgi_hndl);
	DECLDIR int poll_time_spi(uint32_t dgi_hndl, unsigned char* buffer, unsigned long long* timestamp, int* length);
	DECLDIR int poll_spi(uint32_t dgi_hndl, unsigned char* buffer, int* length);
	DECLDIR int send_spi(uint32_t dgi_hndl, unsigned char* buffer, int* length);

	DECLDIR int enable_timestamp(uint32_t dgi_hndl, int buffer_size);
	DECLDIR int disable_timestamp(uint32_t dgi_hndl);
	DECLDIR int poll_timestamp(uint32_t dgi_hndl, unsigned char* buffer, int* length);
	DECLDIR int get_timestamp_base(uint32_t dgi_hndl, unsigned long long* timestamp);

	DECLDIR int enable_gpio(uint32_t dgi_hndl, unsigned char inputs, unsigned char outputs);
	DECLDIR int disable_gpio(uint32_t dgi_hndl);
	DECLDIR int clear_gpio(uint32_t dgi_hndl);
	DECLDIR int set_gpio(uint32_t dgi_hndl, unsigned char* buffer, int* length);
	DECLDIR int poll_time_gpio(uint32_t dgi_hndl, unsigned char* buffer, unsigned long  long* timestamp, int* length);
	
	DECLDIR int reconfigure_usart(uint32_t dgi_hndl, unsigned int baud, unsigned char mode);
	DECLDIR int enable_usart(uint32_t dgi_hndl, int buffer_size, unsigned int baud, unsigned char char_length, unsigned char parity, unsigned char stop_bits, unsigned char mode, bool timestamp);
	DECLDIR int disable_usart(uint32_t dgi_hndl);
	DECLDIR int clear_usart(uint32_t dgi_hndl);
	DECLDIR int poll_time_usart(uint32_t dgi_hndl, unsigned char* buffer, unsigned long  long* timestamp, int* length);
	DECLDIR int poll_usart(uint32_t dgi_hndl, unsigned char* buffer, int* length);
	DECLDIR int send_usart(uint32_t dgi_hndl, unsigned char* buffer, int* length);

	DECLDIR int enable_twi(uint32_t dgi_hndl, int buffer_size, unsigned int speed, unsigned char address, bool timestamp);
	DECLDIR int disable_twi(uint32_t dgi_hndl);
	DECLDIR int clear_twi(uint32_t dgi_hndl);
	DECLDIR int poll_time_twi(uint32_t dgi_hndl, unsigned char* buffer, unsigned long  long* timestamp, int* length);
	DECLDIR int poll_twi(uint32_t dgi_hndl, unsigned char* buffer, int* length);
	DECLDIR int send_twi(uint32_t dgi_hndl, unsigned char* buffer, int* length);

	DECLDIR int enable_debug(uint32_t dgi_hndl);
	DECLDIR int disable_debug(uint32_t dgi_hndl);
	DECLDIR int clear_debug(uint32_t dgi_hndl);
	DECLDIR int poll_time_debug(uint32_t dgi_hndl, unsigned char* buffer, unsigned long  long* timestamp, int* length);
	DECLDIR int get_debug_addresses(uint32_t dgi_hndl, unsigned char* buffer, int* length);

	DECLDIR int enable_print(uint32_t dgi_hndl, int buffer_size);
	DECLDIR int disable_print(uint32_t dgi_hndl);
	DECLDIR int clear_print(uint32_t dgi_hndl);
	DECLDIR int poll_print(uint32_t dgi_hndl, unsigned char* buffer, int* length, unsigned int* ovf_index, unsigned int* ovf_length, unsigned int* ovf_entry_count, unsigned int* health);

	DECLDIR int get_interfaces(uint32_t dgi_hndl, unsigned int* interfaces);
	DECLDIR int get_coprocessors(uint32_t dgi_hndl, unsigned int* coprocessors);

	DECLDIR unsigned int get_buffer_size(uint32_t dgi_hndl);

	DECLDIR int execute_pam_cmd(uint32_t dgi_hndl, unsigned char* cmd, unsigned int cmd_len, unsigned char* resp, unsigned int* resp_len);
}
 
#endif