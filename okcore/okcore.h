/* -*- mode:C; c-file-style: "bsd" -*- */
/*
 * Copyright (c) 2008-2015 Yubico AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef	__OKCORE_H_INCLUDED__
#define	__OKCORE_H_INCLUDED__

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

# ifdef __cplusplus
extern "C" {
# endif

/*************************************************************************
 **
 ** N O T E :  For all functions that return a value, 0 and NULL indicates
 ** an error, other values indicate success.
 **
 ************************************************************************/

/*************************************************************************
 *
 * Structures used.  They are further defined in okdef.h
 *
 ****/

typedef struct ok_key_st OK_KEY;	/* Really a USB device handle. */
typedef struct ok_status_st OK_STATUS;	/* Status structure,
					   filled by ok_get_status(). */

typedef struct ok_ticket_st OK_TICKET;	/* Ticket structure... */
typedef struct ok_config_st OK_CONFIG;	/* Configuration structure.
					   Other libraries provide access. */
typedef struct ok_nav_st OK_NAV;	/* Navigation structure.
					   Other libraries provide access. */
typedef struct ok_frame_st OK_FRAME;	/* Data frame for write operation */
typedef struct ndef_st OK_NDEF;
typedef struct ok_device_config_st OK_DEVICE_CONFIG;

/*************************************************************************
 *
 * Library initialisation functions.
 *
 ****/
extern int ok_init(void);
extern int ok_release(void);

/*************************************************************************
 *
 * Functions to get and release the key itself.
 *
 ****/
/* opens first key available. For backwards compatability */
extern OK_KEY *ok_open_first_key(void);
extern OK_KEY *ok_open_key(int);	/* opens nth key available */
extern int ok_close_key(OK_KEY *k);		/* closes a previously opened key */

/*************************************************************************
 *
 * Functions to get data from the key.
 *
 ****/
/* fetches key status into the structure given by `status' */
extern int ok_get_status(OK_KEY *k, OK_STATUS *status /*, int forceUpdate */);
/* checks that the firmware revision of the key is supported */
extern int ok_check_firmware_version(OK_KEY *k);
extern int ok_check_firmware_version2(OK_STATUS *status);
/* Read the factory set serial number from a YubiKey 2.0 or higher. */
extern int ok_get_serial(OK_KEY *ok, uint8_t slot, unsigned int flags, unsigned int *serial);
/* Wait for the key to either set or clear bits in it's status byte */
extern int ok_wait_for_key_status(OK_KEY *ok, uint8_t slot, unsigned int flags,
				  unsigned int max_time_ms,
				  bool logic_and, unsigned char mask,
				  unsigned char *last_data);
/* Read the response to a command from the YubiKey */
extern int ok_read_response_from_key(OK_KEY *ok, uint8_t slot, unsigned int flags,
				     void *buf, unsigned int bufsize, unsigned int expect_bytes,
				     unsigned int *bytes_read);

/*************************************************************************
 *
 * Functions to write data to the key.
 *
 ****/

/* writes the given configuration to the key.  If the configuration is NULL,
   zap the key configuration.
   acc_code has to be provided of the key has a protecting access code. */
extern int ok_write_command(OK_KEY *k, OK_CONFIG *cfg, uint8_t command,
			   unsigned char *acc_code);
/* wrapper function of ok_write_command */
extern int ok_write_config(OK_KEY *k, OK_CONFIG *cfg, int confnum,
			   unsigned char *acc_code);
/* writes the given ndef to the key as SLOT_NDEF */
extern int ok_write_ndef(OK_KEY *ok, OK_NDEF *ndef);
/* writes the given ndef to the key. */
extern int ok_write_ndef2(OK_KEY *ok, OK_NDEF *ndef, int confnum);
/* writes a device config block to the key. */
extern int ok_write_device_config(OK_KEY *ok, OK_DEVICE_CONFIG *device_config);
/* writes a scanmap to the key. */
extern int ok_write_scan_map(OK_KEY *ok, unsigned char *scan_map);
/* Write something to the YubiKey (a command that is). */
extern int ok_write_to_key(OK_KEY *ok, uint8_t slot, const void *buf, int bufcount);
/* Do a challenge-response round with the key. */
extern int ok_challenge_response(OK_KEY *ok, uint8_t ok_cmd, int may_block,
				 unsigned int challenge_len, const unsigned char *challenge,
				 unsigned int response_len, unsigned char *response);

extern int ok_force_key_update(OK_KEY *ok);
/* Get the VID and PID of an opened device. */
extern int ok_get_key_vid_pid(OK_KEY *ok, int *vid, int *pid);
/* Get the OK4 capabilities */
int ok_get_capabilities(OK_KEY *ok, uint8_t slot, unsigned int flags,
			unsigned char *capabilities, unsigned int *len);
/* Set the device info (TLV string) */
int ok_write_device_info(OK_KEY *ok, unsigned char *buf, unsigned int len);


/*************************************************************************
 *
 * Error handling fuctions
 *
 ****/
extern int * _ok_errno_location(void);
#define ok_errno (*_ok_errno_location())
const char *ok_strerror(int errnum);
/* The following function is only useful if ok_errno == OK_EUSBERR and
   no other USB-related operations have been performed since the time of
   error.  */
const char *ok_usb_strerror(void);


/* Swaps the two bytes between little and big endian on big endian machines */
extern uint16_t ok_endian_swap_16(uint16_t x);

#define OK_EUSBERR	0x01	/* USB error reporting should be used */
#define OK_EWRONGSIZ	0x02
#define OK_EWRITEERR	0x03
#define OK_ETIMEOUT	0x04
#define OK_ENOKEY	0x05
#define OK_EFIRMWARE	0x06
#define OK_ENOMEM	0x07
#define OK_ENOSTATUS	0x08
#define OK_ENOTYETIMPL	0x09
#define OK_ECHECKSUM	0x0a	/* checksum validation failed */
#define OK_EWOULDBLOCK	0x0b	/* operation would block */
#define OK_EINVALIDCMD	0x0c	/* supplied command is invalid for this operation */
#define OK_EMORETHANONE	0x0d    /* expected to find only one key but found more */
#define OK_ENODATA	0x0e	/* no data was returned from a read */

/* Flags for response reading. Use high numbers to not exclude the possibility
 * to combine these with for example SLOT commands from okdef.h in the future.
 */
#define OK_FLAG_MAYBLOCK	0x01 << 16

#define OK_CRC_OK_RESIDUAL	0xf0b8

# ifdef __cplusplus
}
# endif

#endif	/* __OKCORE_H_INCLUDED__ */
