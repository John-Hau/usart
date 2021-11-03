#ifndef __MODBUS_RTU_H__
#define __MODBUS_RTU_H__
void _error_print(modbus_t *ctx, const char *context);
void _sleep_response_timeout(modbus_t *ctx);
int _modbus_rtu_flush(modbus_t *ctx);
int _modbus_set_slave(modbus_t *ctx, int slave);
int _modbus_rtu_build_request_basis(modbus_t *ctx, int function,int addr, int nb, uint8_t *req);


int _modbus_rtu_build_response_basis(sft_t *sft, uint8_t *rsp);

int _modbus_rtu_prepare_response_tid(const uint8_t *req, int *req_length);

int _modbus_rtu_send_msg_pre(uint8_t *req, int req_length);


ssize_t _modbus_rtu_send(modbus_t *ctx, const uint8_t *req, int req_length);


int _modbus_rtu_check_integrity(modbus_t *ctx, uint8_t *msg,const int msg_length);

int _modbus_rtu_pre_check_confirmation(modbus_t *ctx, const uint8_t *req, const uint8_t *rsp, int rsp_length);

ssize_t _modbus_rtu_recv(modbus_t *ctx, uint8_t *rsp, int rsp_length);

int _modbus_rtu_receive(modbus_t *ctx, uint8_t *req);

void _modbus_init_common(modbus_t *ctx);

int _modbus_receive_msg(modbus_t *ctx, uint8_t *msg, msg_type_t msg_type);

void _modbus_rtu_close(modbus_t *ctx);

int _modbus_rtu_select(modbus_t *ctx, fd_set *rset, struct timeval *tv, int length_to_read);

void _modbus_rtu_free(modbus_t *ctx);

int _modbus_rtu_connect(modbus_t *ctx);



#endif
