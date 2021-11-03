#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <termios.h>
#include "modbus_tiny.h"
#include "modbus_rtu.h"
#include "modbus.h"

void _error_print(modbus_t *ctx, const char *context)
{
	if (ctx->debug) {
		fprintf(stderr, "ERROR %s", modbus_strerror(errno));
		if (context != NULL) {
			fprintf(stderr, ": %s\n", context);
		} else {
			fprintf(stderr, "\n");
		}
	}
}


void _sleep_response_timeout(modbus_t *ctx)
{
	/* Response timeout is always positive */
	/* usleep source code */
	struct timespec request, remaining;
	request.tv_sec = ctx->response_timeout.tv_sec;
	request.tv_nsec = ((long int)ctx->response_timeout.tv_usec) * 1000;
	while (nanosleep(&request, &remaining) == -1 && errno == EINTR) {
		request = remaining;
	}
}



int _modbus_rtu_flush(modbus_t *ctx)
{
	return tcflush(ctx->s, TCIOFLUSH);
}





/* Define the slave ID of the remote device to talk in master mode or set the
 * internal slave ID in slave mode */
int _modbus_set_slave(modbus_t *ctx, int slave)
{
	/* Broadcast address is 0 (MODBUS_BROADCAST_ADDRESS) */
	if (slave >= 0 && slave <= 247)
	{
		ctx->slave = slave;
	}
	else 
	{
		errno = EINVAL;
		return -1;
	}

	return 0;
}



/* Builds a RTU request header */
int _modbus_rtu_build_request_basis(modbus_t *ctx, int function,
		int addr, int nb,
		uint8_t *req)
{
	assert(ctx->slave != -1);
	req[0] = ctx->slave;
	req[1] = function;
	req[2] = addr >> 8;
	req[3] = addr & 0x00ff;
	req[4] = nb >> 8;
	req[5] = nb & 0x00ff;

	return _MODBUS_RTU_PRESET_REQ_LENGTH;
}




/* Builds a RTU response header */
int _modbus_rtu_build_response_basis(sft_t *sft, uint8_t *rsp)
{
	/* In this case, the slave is certainly valid because a check is already
	 * done in _modbus_rtu_listen */
	rsp[0] = sft->slave;
	rsp[1] = sft->function;

	return _MODBUS_RTU_PRESET_RSP_LENGTH;
}


int _modbus_rtu_prepare_response_tid(const uint8_t *req, int *req_length)
{
	(*req_length) -= _MODBUS_RTU_CHECKSUM_LENGTH;
	/* No TID */
	return 0;
}


int _modbus_rtu_send_msg_pre(uint8_t *req, int req_length)
{
	uint16_t crc = crc16(req, req_length);
	req[req_length++] = crc >> 8;
	req[req_length++] = crc & 0x00FF;

	return req_length;
}



ssize_t _modbus_rtu_send(modbus_t *ctx, const uint8_t *req, int req_length)
{
#if HAVE_DECL_TIOCM_RTS
	modbus_rtu_t *ctx_rtu = ctx->backend_data;
	if (ctx_rtu->rts != MODBUS_RTU_RTS_NONE) {
		ssize_t size;

		if (ctx->debug) {
			fprintf(stderr, "Sending request using RTS signal\n");
		}

		ctx_rtu->set_rts(ctx, ctx_rtu->rts == MODBUS_RTU_RTS_UP);
		usleep(ctx_rtu->rts_delay);

		size = write(ctx->s, req, req_length);

		usleep(ctx_rtu->onebyte_time * req_length + ctx_rtu->rts_delay);
		ctx_rtu->set_rts(ctx, ctx_rtu->rts != MODBUS_RTU_RTS_UP);

		return size;
	} else {
#endif
		return write(ctx->s, req, req_length);
#if HAVE_DECL_TIOCM_RTS
	}
#endif
}



/* The check_crc16 function shall return 0 is the message is ignored and the
   message length if the CRC is valid. Otherwise it shall return -1 and set
   errno to EMBBADCRC. */
int _modbus_rtu_check_integrity(modbus_t *ctx, uint8_t *msg,
		const int msg_length)
{
	uint16_t crc_calculated;
	uint16_t crc_received;
	int slave = msg[0];

	/* Filter on the Modbus unit identifier (slave) in RTU mode to avoid useless
	 * CRC computing. */
	if (slave != ctx->slave && slave != MODBUS_BROADCAST_ADDRESS) {
		if (ctx->debug) {
			printf("Request for slave %d ignored (not %d)\n", slave, ctx->slave);
		}
		/* Following call to check_confirmation handles this error */
		return 0;
	}

	crc_calculated = crc16(msg, msg_length - 2);
	crc_received = (msg[msg_length - 2] << 8) | msg[msg_length - 1];

	/* Check CRC of msg */
	if (crc_calculated == crc_received) {
		return msg_length;
	} else {
		if (ctx->debug) {
			fprintf(stderr, "ERROR CRC received 0x%0X != CRC calculated 0x%0X\n",
					crc_received, crc_calculated);
		}

		if (ctx->error_recovery & MODBUS_ERROR_RECOVERY_PROTOCOL) {
			_modbus_rtu_flush(ctx);
		}
		errno = EMBBADCRC;
		return -1;
	}
}




int _modbus_rtu_pre_check_confirmation(modbus_t *ctx, const uint8_t *req,
		const uint8_t *rsp, int rsp_length)
{
	/* Check responding slave is the slave we requested (except for broacast
	 * request) */
	if (req[0] != rsp[0] && req[0] != MODBUS_BROADCAST_ADDRESS) {
		if (ctx->debug) {
			fprintf(stderr,
					"The responding slave %d isn't the requested slave %d\n",
					rsp[0], req[0]);
		}
		errno = EMBBADSLAVE;
		return -1;
	} else {
		return 0;
	}
}




ssize_t _modbus_rtu_recv(modbus_t *ctx, uint8_t *rsp, int rsp_length)
{
    return read(ctx->s, rsp, rsp_length);
}






int _modbus_rtu_receive(modbus_t *ctx, uint8_t *req)
{
    int rc;
    modbus_rtu_t *ctx_rtu = ctx->backend_data;

    if (ctx_rtu->confirmation_to_ignore) {
        _modbus_receive_msg(ctx, req, MSG_CONFIRMATION);
        /* Ignore errors and reset the flag */
        ctx_rtu->confirmation_to_ignore = FALSE;
        rc = 0;
        if (ctx->debug) {
            printf("Confirmation to ignore\n");
        }
    } else {
        rc = _modbus_receive_msg(ctx, req, MSG_INDICATION);
        if (rc == 0) {
            /* The next expected message is a confirmation to ignore */
            ctx_rtu->confirmation_to_ignore = TRUE;
        }
    }
    return rc;
}



void _modbus_init_common(modbus_t *ctx)
{
    /* Slave and socket are initialized to -1 */
    ctx->slave = -1;
    ctx->s = -1;

    ctx->debug = FALSE;
    ctx->error_recovery = MODBUS_ERROR_RECOVERY_NONE;

    ctx->response_timeout.tv_sec = 0;
    ctx->response_timeout.tv_usec = _RESPONSE_TIMEOUT;

    ctx->byte_timeout.tv_sec = 0;
    ctx->byte_timeout.tv_usec = _BYTE_TIMEOUT;

    ctx->indication_timeout.tv_sec = 0;
    ctx->indication_timeout.tv_usec = 0;
}





/* Waits a response from a modbus server or a request from a modbus client.
   This function blocks if there is no replies (3 timeouts).

   The function shall return the number of received characters and the received
   message in an array of uint8_t if successful. Otherwise it shall return -1
   and errno is set to one of the values defined below:
   - ECONNRESET
   - EMBBADDATA
   - EMBUNKEXC
   - ETIMEDOUT
   - read() or recv() error codes
   */

int _modbus_receive_msg(modbus_t *ctx, uint8_t *msg, msg_type_t msg_type)
{
	int rc;
	fd_set rset;
	struct timeval tv;
	struct timeval *p_tv;
	int length_to_read;
	int msg_length = 0;
	_step_t step;

	if (ctx->debug) {
		if (msg_type == MSG_INDICATION) {
			printf("Waiting for an indication...\n");
		} else {
			printf("Waiting for a confirmation...\n");
		}
	}

	/* Add a file descriptor to the set */
	FD_ZERO(&rset);
	FD_SET(ctx->s, &rset);

	/* We need to analyse the message step by step.  At the first step, we want
	 * to reach the function code because all packets contain this
	 * information. */
	step = _STEP_FUNCTION;
	length_to_read = ctx->backend->header_length + 1;

	if (msg_type == MSG_INDICATION) {
		/* Wait for a message, we don't know when the message will be
		 * received */
		if (ctx->indication_timeout.tv_sec == 0 && ctx->indication_timeout.tv_usec == 0) {
			/* By default, the indication timeout isn't set */
			p_tv = NULL;
		} else {
			/* Wait for an indication (name of a received request by a server, see schema) */
			tv.tv_sec = ctx->indication_timeout.tv_sec;
			tv.tv_usec = ctx->indication_timeout.tv_usec;
			p_tv = &tv;
		}
	} else {
		tv.tv_sec = ctx->response_timeout.tv_sec;
		tv.tv_usec = ctx->response_timeout.tv_usec;
		p_tv = &tv;
	}

	while (length_to_read != 0) {
		rc = ctx->backend->select(ctx, &rset, p_tv, length_to_read);
		if (rc == -1) {
			_error_print(ctx, "select");
			if (ctx->error_recovery & MODBUS_ERROR_RECOVERY_LINK) {
				int saved_errno = errno;

				if (errno == ETIMEDOUT) {
					_sleep_response_timeout(ctx);
					modbus_flush(ctx);
				} else if (errno == EBADF) {
					modbus_close(ctx);
					modbus_connect(ctx);
				}
				errno = saved_errno;
			}
			return -1;
		}

		rc = ctx->backend->recv(ctx, msg + msg_length, length_to_read);
		if (rc == 0) {
			errno = ECONNRESET;
			rc = -1;
		}

		if (rc == -1) {
			_error_print(ctx, "read");
			if ((ctx->error_recovery & MODBUS_ERROR_RECOVERY_LINK) &&
					(errno == ECONNRESET || errno == ECONNREFUSED ||
					 errno == EBADF)) {
				int saved_errno = errno;
				modbus_close(ctx);
				modbus_connect(ctx);
				/* Could be removed by previous calls */
				errno = saved_errno;
			}
			return -1;
		}

		/* Display the hex code of each character received */
		if (ctx->debug) {
			int i;
			for (i=0; i < rc; i++)
				printf("<%.2X>", msg[msg_length + i]);
		}

		/* Sums bytes received */
		msg_length += rc;
		/* Computes remaining bytes */
		length_to_read -= rc;

		if (length_to_read == 0) {
			switch (step) {
				case _STEP_FUNCTION:
					/* Function code position */
					length_to_read = compute_meta_length_after_function(
							msg[ctx->backend->header_length],
							msg_type);
					if (length_to_read != 0) {
						step = _STEP_META;
						break;
					} /* else switches straight to the next step */
				case _STEP_META:
					length_to_read = compute_data_length_after_meta(
							ctx, msg, msg_type);
					if ((msg_length + length_to_read) > (int)ctx->backend->max_adu_length) {
						errno = EMBBADDATA;
						_error_print(ctx, "too many data");
						return -1;
					}
					step = _STEP_DATA;
					break;
				default:
					break;
			}
		}

		if (length_to_read > 0 &&
				(ctx->byte_timeout.tv_sec > 0 || ctx->byte_timeout.tv_usec > 0)) {
			/* If there is no character in the buffer, the allowed timeout
			   interval between two consecutive bytes is defined by
			   byte_timeout */
			tv.tv_sec = ctx->byte_timeout.tv_sec;
			tv.tv_usec = ctx->byte_timeout.tv_usec;
			p_tv = &tv;
		}
		/* else timeout isn't set again, the full response must be read before
		   expiration of response timeout (for CONFIRMATION only) */
	}

	if (ctx->debug)
		printf("\n");

	return ctx->backend->check_integrity(ctx, msg, msg_length);
}





void _modbus_rtu_close(modbus_t *ctx)
{
	/* Restore line settings and close file descriptor in RTU mode */
	modbus_rtu_t *ctx_rtu = ctx->backend_data;

	if (ctx->s != -1) {
		tcsetattr(ctx->s, TCSANOW, &ctx_rtu->old_tios);
		close(ctx->s);
		ctx->s = -1;
	}
}



int _modbus_rtu_select(modbus_t *ctx, fd_set *rset,
		struct timeval *tv, int length_to_read)
{
	int s_rc;
	while ((s_rc = select(ctx->s+1, rset, NULL, NULL, tv)) == -1) {
		if (errno == EINTR) {
			if (ctx->debug) {
				fprintf(stderr, "A non blocked signal was caught\n");
			}
			/* Necessary after an error */
			FD_ZERO(rset);
			FD_SET(ctx->s, rset);
		} else {
			return -1;
		}
	}

	if (s_rc == 0) {
		/* Timeout */
		errno = ETIMEDOUT;
		return -1;
	}

	return s_rc;
}




void _modbus_rtu_free(modbus_t *ctx) {
	if (ctx->backend_data) {
		free(((modbus_rtu_t *)ctx->backend_data)->device);
		free(ctx->backend_data);
	}

	free(ctx);
}

/* Sets up a serial port for RTU communications */
int _modbus_rtu_connect(modbus_t *ctx)
{

	struct termios tios;
	speed_t speed;
	int flags;

	modbus_rtu_t *ctx_rtu = ctx->backend_data;

	if (ctx->debug) {
		printf("Opening %s at %d bauds (%c, %d, %d)\n",
				ctx_rtu->device, ctx_rtu->baud, ctx_rtu->parity,
				ctx_rtu->data_bit, ctx_rtu->stop_bit);
	}

#if defined(_WIN32)

#else
	/* The O_NOCTTY flag tells UNIX that this program doesn't want
	   to be the "controlling terminal" for that port. If you
	   don't specify this then any input (such as keyboard abort
	   signals and so forth) will affect your process

	   Timeouts are ignored in canonical input mode or when the
	   NDELAY option is set on the file via open or fcntl */
	flags = O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL;
#ifdef O_CLOEXEC
	flags |= O_CLOEXEC;
#endif

	ctx->s = open(ctx_rtu->device, flags);
	if (ctx->s == -1) {
		if (ctx->debug) {
			fprintf(stderr, "ERROR Can't open the device %s (%s)\n",
					ctx_rtu->device, strerror(errno));
		}
		return -1;
	}

	/* Save */
	tcgetattr(ctx->s, &ctx_rtu->old_tios);

	memset(&tios, 0, sizeof(struct termios));

	/* C_ISPEED     Input baud (new interface)
	   C_OSPEED     Output baud (new interface)
	   */
	switch (ctx_rtu->baud) {
		case 1200:
			speed = B1200;
			break;
		case 2400:
			speed = B2400;
			break;
		case 9600:
			speed = B9600;
			break;
		case 38400:
			speed = B38400;
			break;
#ifdef B115200
		case 115200:
			speed = B115200;
			break;
#endif
		default:
			speed = B9600;
			if (ctx->debug) {
				fprintf(stderr,
						"WARNING Unknown baud rate %d for %s (B9600 used)\n",
						ctx_rtu->baud, ctx_rtu->device);
			}
	}

	/* Set the baud rate */
	if ((cfsetispeed(&tios, speed) < 0) ||
			(cfsetospeed(&tios, speed) < 0)) {
		close(ctx->s);
		ctx->s = -1;
		return -1;
	}

	/* C_CFLAG      Control options
	   CLOCAL       Local line - do not change "owner" of port
	   CREAD        Enable receiver
	   */
	tios.c_cflag |= (CREAD | CLOCAL);
	/* CSIZE, HUPCL, CRTSCTS (hardware flow control) */

	/* Set data bits (5, 6, 7, 8 bits)
	   CSIZE        Bit mask for data bits
	   */
	tios.c_cflag &= ~CSIZE;
	switch (ctx_rtu->data_bit) {
		case 7:
			tios.c_cflag |= CS7;
			break;
		case 8:
		default:
			tios.c_cflag |= CS8;
			break;
	}

	/* Stop bit (1 or 2) */
	if (ctx_rtu->stop_bit == 1)
		tios.c_cflag &=~ CSTOPB;
	else /* 2 */
		tios.c_cflag |= CSTOPB;

	/* PARENB       Enable parity bit
	   PARODD       Use odd parity instead of even */
	if (ctx_rtu->parity == 'N') {
		/* None */
		tios.c_cflag &=~ PARENB;
	} else if (ctx_rtu->parity == 'E') {
		/* Even */
		tios.c_cflag |= PARENB;
		tios.c_cflag &=~ PARODD;
	} else {
		/* Odd */
		tios.c_cflag |= PARENB;
		tios.c_cflag |= PARODD;
	}

	/* Read the man page of termios if you need more information. */

	/* This field isn't used on POSIX systems
	   tios.c_line = 0;
	   */

	/* C_LFLAG      Line options

	   ISIG Enable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals
	   ICANON       Enable canonical input (else raw)
	   XCASE        Map uppercase \lowercase (obsolete)
	   ECHO Enable echoing of input characters
	   ECHOE        Echo erase character as BS-SP-BS
	   ECHOK        Echo NL after kill character
	   ECHONL       Echo NL
	   NOFLSH       Disable flushing of input buffers after
	   interrupt or quit characters
	   IEXTEN       Enable extended functions
	   ECHOCTL      Echo control characters as ^char and delete as ~?
	   ECHOPRT      Echo erased character as character erased
	   ECHOKE       BS-SP-BS entire line on line kill
	   FLUSHO       Output being flushed
	   PENDIN       Retype pending input at next read or input char
	   TOSTOP       Send SIGTTOU for background output

	   Canonical input is line-oriented. Input characters are put
	   into a buffer which can be edited interactively by the user
	   until a CR (carriage return) or LF (line feed) character is
	   received.

	   Raw input is unprocessed. Input characters are passed
	   through exactly as they are received, when they are
	   received. Generally you'll deselect the ICANON, ECHO,
	   ECHOE, and ISIG options when using raw input
	   */

	/* Raw input */
	tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	/* C_IFLAG      Input options

	   Constant     Description
	   INPCK        Enable parity check
	   IGNPAR       Ignore parity errors
	   PARMRK       Mark parity errors
	   ISTRIP       Strip parity bits
	   IXON Enable software flow control (outgoing)
	   IXOFF        Enable software flow control (incoming)
	   IXANY        Allow any character to start flow again
	   IGNBRK       Ignore break condition
	   BRKINT       Send a SIGINT when a break condition is detected
	   INLCR        Map NL to CR
	   IGNCR        Ignore CR
	   ICRNL        Map CR to NL
	   IUCLC        Map uppercase to lowercase
	   IMAXBEL      Echo BEL on input line too long
	   */
	if (ctx_rtu->parity == 'N') {
		/* None */
		tios.c_iflag &= ~INPCK;
	} else {
		tios.c_iflag |= INPCK;
	}

	/* Software flow control is disabled */
	tios.c_iflag &= ~(IXON | IXOFF | IXANY);

	/* C_OFLAG      Output options
	   OPOST        Postprocess output (not set = raw output)
	   ONLCR        Map NL to CR-NL

	   ONCLR ant others needs OPOST to be enabled
	   */

	/* Raw ouput */
	tios.c_oflag &=~ OPOST;

	/* C_CC         Control characters
	   VMIN         Minimum number of characters to read
	   VTIME        Time to wait for data (tenths of seconds)

	   UNIX serial interface drivers provide the ability to
	   specify character and packet timeouts. Two elements of the
	   c_cc array are used for timeouts: VMIN and VTIME. Timeouts
	   are ignored in canonical input mode or when the NDELAY
	   option is set on the file via open or fcntl.

	   VMIN specifies the minimum number of characters to read. If
	   it is set to 0, then the VTIME value specifies the time to
	   wait for every character read. Note that this does not mean
	   that a read call for N bytes will wait for N characters to
	   come in. Rather, the timeout will apply to the first
	   character and the read call will return the number of
	   characters immediately available (up to the number you
	   request).

	   If VMIN is non-zero, VTIME specifies the time to wait for
	   the first character read. If a character is read within the
	   time given, any read will block (wait) until all VMIN
	   characters are read. That is, once the first character is
	   read, the serial interface driver expects to receive an
	   entire packet of characters (VMIN bytes total). If no
	   character is read within the time allowed, then the call to
	   read returns 0. This method allows you to tell the serial
	   driver you need exactly N bytes and any read call will
	   return 0 or N bytes. However, the timeout only applies to
	   the first character read, so if for some reason the driver
	   misses one character inside the N byte packet then the read
	   call could block forever waiting for additional input
	   characters.

	   VTIME specifies the amount of time to wait for incoming
	   characters in tenths of seconds. If VTIME is set to 0 (the
	   default), reads will block (wait) indefinitely unless the
	   NDELAY option is set on the port with open or fcntl.
	   */
	/* Unused because we use open with the NDELAY option */
	tios.c_cc[VMIN] = 0;
	tios.c_cc[VTIME] = 0;

	if (tcsetattr(ctx->s, TCSANOW, &tios) < 0) {
		close(ctx->s);
		ctx->s = -1;
		return -1;
	}
#endif

	return 0;
}

