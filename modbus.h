#ifndef __MODBUS_H__
#define __MODBUS_H__


uint16_t crc16(uint8_t *buffer, uint16_t buffer_length);
char *modbus_strerror(int errnum);
int modbus_get_response_timeout(modbus_t *ctx, uint32_t *to_sec, uint32_t *to_usec);
void modbus_free(modbus_t *ctx);
int compute_data_length_after_meta(modbus_t *ctx, uint8_t *msg, msg_type_t msg_type);
uint8_t compute_meta_length_after_function(int function, msg_type_t msg_type);
int modbus_flush(modbus_t *ctx);
void modbus_close(modbus_t *ctx);
int modbus_connect(modbus_t *ctx);
int modbus_set_slave(modbus_t *ctx, int slave);
unsigned int compute_response_length_from_request(modbus_t *ctx, uint8_t *req);
int send_msg(modbus_t *ctx, uint8_t *msg, int msg_length);
int check_confirmation(modbus_t *ctx, uint8_t *req, uint8_t *rsp, int rsp_length);

int read_io_status(modbus_t *ctx, int function, int addr, int nb, uint8_t *dest);
modbus_t* modbus_new_rtu(const char *device, int baud, char parity, int data_bit, int stop_bit);
int write_single(modbus_t *ctx, int function, int addr, const uint16_t value);
int modbus_set_debug(modbus_t *ctx, int flag);
int modbus_set_error_recovery(modbus_t *ctx, modbus_error_recovery_mode error_recovery);
int modbus_write_bit(modbus_t *ctx, int addr, int status);
int modbus_read_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);













#endif
