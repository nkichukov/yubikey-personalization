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

#include "okcore_lcl.h"
#include "okcore_backend.h"
#include "oktsd.h"
#include "okbzero.h"

/* To get modhex and crc16 */
#include <yubikey.h>

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

#ifdef OK_DEBUG
#define _ok_hexdump(buffer, size) \
	do { \
		unsigned char *p = buffer; \
		int i; \
		fprintf(stderr, "%25s: ", __func__); \
		for(i = 0; i < size; i++) { \
			fprintf(stderr, "%02x ", *p++); \
		} \
		fprintf(stderr, "\n"); \
	} while(0)
#endif

/*
 * Yubikey low-level interface section 2.4 (Report arbitration polling) specifies
 * a 600 ms timeout for a Yubikey to process something written to it.
 * Where can that document be found?
 * It has been discovered that for swap 600 is not enough, swapping can worst
 * case take 920 ms, which we then add 25% to for safety margin, arriving at
 * 1150 ms.
 */
#define WAIT_FOR_WRITE_FLAG	1150

int ok_init(void)
{
	return _okusb_start();
}

int ok_release(void)
{
	return _okusb_stop();
}

OK_KEY *ok_open_first_key(void)
{
	return ok_open_key(0);
}

OK_KEY *ok_open_key(int index)
{
	int pids[] = {YUBIKEY_PID, NEO_OTP_PID, NEO_OTP_CCID_PID,
		NEO_OTP_U2F_PID, NEO_OTP_U2F_CCID_PID, OK4_OTP_PID,
		OK4_OTP_U2F_PID, OK4_OTP_CCID_PID, OK4_OTP_U2F_CCID_PID,
		PLUS_U2F_OTP_PID, 0x60fc}; //Added 3rd party PID

	OK_KEY *ok = _okusb_open_device(YUBICO_VID, pids, sizeof(pids) / sizeof(int), index);
	if (!ok) { //If no Yubikey is found search for compatible 3rd party devices
		ok_errno = 0;
		ok = _okusb_open_device(0x1d50, pids, sizeof(pids) / sizeof(int), index); 
	}
	int rc = ok_errno;

	if (ok) {
		OK_STATUS st;

		if (!ok_get_status(ok, &st)) {
			rc = ok_errno;
			ok_close_key(ok);
			ok = NULL;
		}
	}
	ok_errno = rc;
	return ok;
}

int ok_close_key(OK_KEY *ok)
{
	return _okusb_close_device(ok);
}

int ok_check_firmware_version(OK_KEY *k)
{
	OK_STATUS st;

	if (!ok_get_status(k, &st))
		return 0;

	return ok_check_firmware_version2(&st);
}


int ok_check_firmware_version2(OK_STATUS *st)
{
	return 1;
}

int ok_get_status(OK_KEY *k, OK_STATUS *status)
{
	unsigned int status_count = 0;

	if (!ok_read_from_key(k, 0, status, sizeof(OK_STATUS), &status_count))
		return 0;

	if (status_count != sizeof(OK_STATUS)) {
		ok_errno = OK_EWRONGSIZ;
		return 0;
	}

	status->touchLevel = ok_endian_swap_16(status->touchLevel);

	return 1;
}

/* Read the factory programmed serial number from a YubiKey.
 * The possibility to retreive the serial number might be disabled
 * using configuration, so it should not be considered a fatal error
 * to not be able to read the serial number using this function.
 *
 * Serial number reading might also be configured to require user
 * interaction (YubiKey button press) on startup, in which case flags
 * might have to have OK_FLAG_MAYBLOCK set - haven't tried that.
 *
 * The slot parameter is here for future purposes only.
 */
int ok_get_serial(OK_KEY *ok, uint8_t slot, unsigned int flags, unsigned int *serial)
{
	unsigned char buf[FEATURE_RPT_SIZE * 2];
	unsigned int response_len = 0;
	unsigned int expect_bytes = 0;

	memset(buf, 0, sizeof(buf));

	if (!ok_write_to_key(ok, SLOT_DEVICE_SERIAL, &buf, 0))
		return 0;

	expect_bytes = 4;

	if (! ok_read_response_from_key(ok, slot, flags,
					&buf, sizeof(buf),
					expect_bytes,
					&response_len))
		return 0;

	/* Serial number is stored in big endian byte order, despite
	 * everything else in the YubiKey being little endian - for
	 * some good reason I don't remember.
	 */
	*serial =
		(buf[0] << 24) +
		(buf[1] << 16) +
		(buf[2] << 8) +
		(buf[3]);

	return 1;
}

int ok_get_capabilities(OK_KEY *ok, uint8_t slot, unsigned int flags,
		unsigned char *capabilities, unsigned int *len)
{
	unsigned int response_len = 0;

	if (!ok_write_to_key(ok, SLOT_OK4_CAPABILITIES, capabilities, 0))
		return 0;

	if (! ok_read_response_from_key(ok, slot, flags,
					capabilities, *len, 0, /* we have no idea how much data we'll get */
					&response_len))
		return 0;

	/* the first data of the capabilities string is the length */
	response_len = capabilities[0];
	response_len++;

	/* validate the length we got back from the hardware */
	if (response_len > *len) {
		ok_errno = OK_EWRONGSIZ;
		return 0;
	}

	*len = response_len;
	return 1;
}

static int _ok_write(OK_KEY *ok, uint8_t ok_cmd, unsigned char *buf, size_t len)
{
	OK_STATUS stat;
	int seq;

	/* Get current sequence # from status block */

	if (!ok_get_status(ok, &stat /*, 0*/))
		return 0;

	seq = stat.pgmSeq;

	/* Write to Yubikey */
	if (!ok_write_to_key(ok, ok_cmd, buf, len))
		return 0;

	/* When the Yubikey clears the SLOT_WRITE_FLAG, it has processed the last write.
	 * This wait can't be done in ok_write_to_key since some users of that function
	 * want to get the bytes in the status message, but when writing configuration
	 * we don't expect any data back.
	 */
	if(!ok_wait_for_key_status(ok, ok_cmd, 0, WAIT_FOR_WRITE_FLAG, false, SLOT_WRITE_FLAG, NULL))
		return 0;

	/* Verify update */

	if (!ok_get_status(ok, &stat /*, 0*/))
		return 0;

	ok_errno = OK_EWRITEERR;

	/* when both configurations from a YubiKey is erased it will return
	 * pgmSeq 0, if one is still configured after an erase pgmSeq is
	 * counted up as usual. */
	if((stat.touchLevel & (CONFIG1_VALID | CONFIG2_VALID)) == 0 && stat.pgmSeq == 0) {
		return 1;
	}
	return stat.pgmSeq != seq;
}

int ok_write_device_info(OK_KEY *ok, unsigned char *buf, unsigned int len)
{
	return _ok_write(ok, SLOT_OK4_SET_DEVICE_INFO, buf, len);
}


int ok_write_command(OK_KEY *ok, OK_CONFIG *cfg, uint8_t command,
		    unsigned char *acc_code)
{
	int ret;
	unsigned char buf[sizeof(OK_CONFIG) + ACC_CODE_SIZE];

	/* Update checksum and insert config block in buffer if present */

	memset(buf, 0, sizeof(buf));

	if (cfg) {
		cfg->crc = ~yubikey_crc16 ((unsigned char *) cfg,
					   sizeof(OK_CONFIG) - sizeof(cfg->crc));
		cfg->crc = ok_endian_swap_16(cfg->crc);
		memcpy(buf, cfg, sizeof(OK_CONFIG));
	}

	/* Append current access code if present */

	if (acc_code)
		memcpy(buf + sizeof(OK_CONFIG), acc_code, ACC_CODE_SIZE);

	ret = _ok_write(ok, command, buf, sizeof(buf));
	insecure_memzero(buf, sizeof(buf));
	return ret;
}

int ok_write_config(OK_KEY *ok, OK_CONFIG *cfg, int confnum,
		    unsigned char *acc_code)
{
	uint8_t command;
	switch(confnum) {
	case 1:
		command = SLOT_CONFIG;
		break;
	case 2:
		command = SLOT_CONFIG2;
		break;
	default:
		ok_errno = OK_EINVALIDCMD;
		return 0;
	}
	if(!ok_write_command(ok, cfg, command, acc_code)) {
		return 0;
	}
	return 1;
}

int ok_write_ndef(OK_KEY *ok, OK_NDEF *ndef)
{
	/* just wrap ok_write_ndef2() with confnum 1 */
	return ok_write_ndef2(ok, ndef, 1);
}

int ok_write_ndef2(OK_KEY *ok, OK_NDEF *ndef, int confnum)
{
	unsigned char buf[sizeof(OK_NDEF)];
	uint8_t command;

	switch(confnum) {
		case 1:
			command = SLOT_NDEF;
			break;
		case 2:
			command = SLOT_NDEF2;
			break;
		default:
			ok_errno = OK_EINVALIDCMD;
			return 0;
	}

	/* Insert config block in buffer */

	memset(buf, 0, sizeof(buf));
	memcpy(buf, ndef, sizeof(OK_NDEF));

	return _ok_write(ok, command, buf, sizeof(OK_NDEF));
}

int ok_write_device_config(OK_KEY *ok, OK_DEVICE_CONFIG *device_config)
{
	unsigned char buf[sizeof(OK_DEVICE_CONFIG)];

	memset(buf, 0, sizeof(buf));
	memcpy(buf, device_config, sizeof(OK_DEVICE_CONFIG));

	return _ok_write(ok, SLOT_DEVICE_CONFIG, buf, sizeof(OK_DEVICE_CONFIG));
}

int ok_write_scan_map(OK_KEY *ok, unsigned char *scan_map)
{
	return _ok_write(ok, SLOT_SCAN_MAP, scan_map, strlen(SCAN_MAP));
}

/*
 * This function is for doing HMAC-SHA1 or Yubico challenge-response with a key.
 */
int ok_challenge_response(OK_KEY *ok, uint8_t ok_cmd, int may_block,
		unsigned int challenge_len, const unsigned char *challenge,
		unsigned int response_len, unsigned char *response)
{
	unsigned int flags = 0;
	unsigned int bytes_read = 0;
	unsigned int expect_bytes = 0;

	switch(ok_cmd) {
	case SLOT_CHAL_HMAC1:
	case SLOT_CHAL_HMAC2:
		expect_bytes = 20;
		break;
	case SLOT_CHAL_OTP1:
	case SLOT_CHAL_OTP2:
		expect_bytes = 16;
		break;
	default:
		ok_errno = OK_EINVALIDCMD;
		return 0;
	}

	if (may_block)
		flags |= OK_FLAG_MAYBLOCK;

	if (! ok_write_to_key(ok, ok_cmd, challenge, challenge_len)) {
		return 0;
	}

	if (! ok_read_response_from_key(ok, ok_cmd, flags,
				response, response_len,
				expect_bytes,
				&bytes_read)) {
		return 0;
	}

	return 1;
}

int * _ok_errno_location(void)
{
	static int tsd_init = 0;
	static int nothread_errno = 0;
	OK_DEFINE_TSD_METADATA(errno_key);
	int rc = 0;

	if (tsd_init == 0) {
		if ((rc = OK_TSD_INIT(errno_key, free)) == 0) {
			tsd_init = 1;
		} else {
			tsd_init = -1;
		}
	}

	if(OK_TSD_GET(int *, errno_key) == NULL) {
		void *p = calloc(1, sizeof(int));
		if (!p) {
			tsd_init = -1;
		} else {
			OK_TSD_SET(errno_key, p);
		}
	}
	if (tsd_init == 1) {
		return OK_TSD_GET(int *, errno_key);
	}
	return &nothread_errno;
}

static const char *errtext[] = {
	"",
	"USB error",
	"wrong size",
	"write error",
	"timeout",
	"no yubikey present",
	"unsupported firmware version",
	"out of memory",
	"no status structure given",
	"not yet implemented",
	"checksum mismatch",
	"operation would block",
	"invalid command for operation",
	"expected only one YubiKey but several present",
	"no data returned from device",
};
const char *ok_strerror(int errnum)
{
	if (errnum < sizeof(errtext)/sizeof(errtext[0]))
		return errtext[errnum];
	return NULL;
}
const char *ok_usb_strerror(void)
{
	return _okusb_strerror();
}

/* This function would've been better named 'ok_read_status_from_key'. Because
 * it disregards the first byte in each feature report, it can't be used to read
 * generic feature reports from the Yubikey, and this behaviour can't be changed
 * without breaking compatibility with existing programs.
 *
 * See ok_read_response_from_key() for a generic purpose data reading function.
 *
 * The slot parameter is here for future purposes only.
 */
int ok_read_from_key(OK_KEY *ok, uint8_t slot,
		     void *buf, unsigned int bufsize, unsigned int *bufcount)
{
	unsigned char data[FEATURE_RPT_SIZE];

	if (bufsize > FEATURE_RPT_SIZE - 1) {
		ok_errno = OK_EWRONGSIZ;
		return 0;
	}

	memset(data, 0, sizeof(data));

	if (!_okusb_read(ok, REPORT_TYPE_FEATURE, 0, (char *)data, FEATURE_RPT_SIZE))
		return 0;

	/* This makes it apparent that there's some mysterious value in
	   the first byte...  I wonder what...  /Richard Levitte */
	memcpy(buf, data + 1, bufsize);
	*bufcount = bufsize;

	return 1;
}

/* Wait for the Yubikey to either set or clear (controlled by the boolean logic_and)
 * the bits in mask.
 *
 * The slot parameter is here for future purposes only.
 */
int ok_wait_for_key_status(OK_KEY *ok, uint8_t slot, unsigned int flags,
			   unsigned int max_time_ms,
			   bool logic_and, unsigned char mask,
			   unsigned char *last_data)
{
	unsigned char data[FEATURE_RPT_SIZE];

	unsigned int sleepval = 1;
	unsigned int slept_time = 0;
	int blocking = 0;

	/* Non-zero slot breaks on Windows (libusb-1.0.8-win32), while working fine
	 * on Linux (and probably MacOS X).
	 *
	 * The YubiKey doesn't support per-slot status anyways at the moment (2.2),
	 * so we just set it to 0 (meaning slot 1).
	 */
	slot = 0;

	while (slept_time < max_time_ms) {
		Sleep(sleepval);
		slept_time += sleepval;
		/* exponential backoff, up to 500 ms */
		sleepval *= 2;
		if (sleepval > 500)
			sleepval = 500;

		/* Read a status report from the key */
		memset(data, 0, sizeof(data));
		if (!_okusb_read(ok, REPORT_TYPE_FEATURE, slot, (char *) &data, FEATURE_RPT_SIZE))
			return 0;
#ifdef OK_DEBUG
		_ok_hexdump(data, FEATURE_RPT_SIZE);
#endif

		if (last_data != NULL)
			memcpy(last_data, data, sizeof(data));

		/* The status byte from the key is now in last byte of data */
		if (logic_and) {
			/* Check if Yubikey has SET the bit(s) in mask */
			if ((data[FEATURE_RPT_SIZE - 1] & mask) == mask) {
				return 1;
			}
		} else {
			/* Check if Yubikey has CLEARED the bit(s) in mask */
			if (! (data[FEATURE_RPT_SIZE - 1] & mask)) {
				return 1;
			}
		}

		/* Check if Yubikey says it will wait for user interaction */
		if ((data[FEATURE_RPT_SIZE - 1] & RESP_TIMEOUT_WAIT_FLAG) == RESP_TIMEOUT_WAIT_FLAG) {
			if ((flags & OK_FLAG_MAYBLOCK) == OK_FLAG_MAYBLOCK) {
				if (! blocking) {
					/* Extend timeout first time we see RESP_TIMEOUT_WAIT_FLAG. */
					blocking = 1;
					max_time_ms += 256 * 1000;
				}
			} else {
				/* Reset read mode of Yubikey before aborting. */
				ok_force_key_update(ok);
				ok_errno = OK_EWOULDBLOCK;
				return 0;
			}
		} else {
			if (blocking) {
				/* YubiKey timed out waiting for user interaction */
				break;
			}
		}
	}

	ok_errno = OK_ETIMEOUT;
	return 0;
}

/* Read one or more feature reports from a Yubikey and put them together.
 *
 * Bufsize must be able to hold at least 2 more bytes than you are expecting
 * (the CRC), but since all read requests return 7 bytes of data bufsize needs
 * to be up to 7 bytes more than you expect.
 *
 * If the key returns more data than bufsize, we fail and set ok_errno to
 * OK_EWRONGSIZ. If that happens there will be partial data in buf.
 *
 * If we read a response from a Yubikey that is configured to block and wait for
 * a button press (in challenge response), this function will abort unless
 * flags contain OK_FLAG_MAYBLOCK, in which case it might take up to 15 seconds
 * for this function to return.
 *
 * The slot parameter is here for future purposes only.
 */
int ok_read_response_from_key(OK_KEY *ok, uint8_t slot, unsigned int flags,
			      void *buf, unsigned int bufsize, unsigned int expect_bytes,
			      unsigned int *bytes_read)
{
	unsigned char data[FEATURE_RPT_SIZE];
	memset(data, 0, sizeof(data));

	memset(buf, 0, bufsize);
	*bytes_read = 0;

#ifdef OK_DEBUG
	fprintf(stderr, "OK_DEBUG: Read %i bytes from YubiKey :\n", expect_bytes);
#endif
	/* Wait for the key to turn on RESP_PENDING_FLAG */
	if (! ok_wait_for_key_status(ok, slot, flags, 1000, true, RESP_PENDING_FLAG, (unsigned char *) &data))
		return 0;

	/* The first part of the response was read by ok_wait_for_key_status(). We need
	 * to copy it to buf.
	 */
	memcpy((char*)buf + *bytes_read, data, sizeof(data) - 1);
	*bytes_read += sizeof(data) - 1;

	while (*bytes_read + FEATURE_RPT_SIZE <= bufsize) {
		memset(data, 0, sizeof(data));

		if (!_okusb_read(ok, REPORT_TYPE_FEATURE, 0, (char *)data, FEATURE_RPT_SIZE))
			return 0;
#ifdef OK_DEBUG
		_ok_hexdump(data, FEATURE_RPT_SIZE);
#endif
		if (data[FEATURE_RPT_SIZE - 1] & RESP_PENDING_FLAG) {
			/* The lower five bits of the status byte has the response sequence
			 * number. If that gets reset to zero we are done.
			 */
			if ((data[FEATURE_RPT_SIZE - 1] & 31) == 0) {
				if (expect_bytes > 0) {
					/* Size of response is known. Verify CRC. */
					expect_bytes += 2;
					int crc = yubikey_crc16(buf, expect_bytes);
					if (crc != OK_CRC_OK_RESIDUAL) {
						ok_errno = OK_ECHECKSUM;
						return 0;
					}

					/* since we get data in chunks of 7 we need to round expect bytes out to the closest higher multiple of 7 */
					if(expect_bytes % 7 != 0) {
						expect_bytes += 7 - (expect_bytes % 7);
					}

					if (*bytes_read != expect_bytes) {
						ok_errno = OK_EWRONGSIZ;
						return 0;
					}
				}

				/* Reset read mode of Yubikey before returning. */
				ok_force_key_update(ok);

				return 1;
			}

			memcpy((char*)buf + *bytes_read, data, sizeof(data) - 1);
			*bytes_read += sizeof(data) - 1;
		} else {
			/* Reset read mode of Yubikey before returning. */
			ok_force_key_update(ok);

			return 0;
		}
	}

	/* We're out of buffer space, abort reading */
	ok_force_key_update(ok);

	ok_errno = OK_EWRONGSIZ;
	return 0;
}

/*
 * Send something to the YubiKey. The command, as well as the slot, is
 * given in the 'slot' parameter (e.g. SLOT_CHAL_HMAC2 to send a HMAC-SHA1
 * challenge to slot 2).
 */
int ok_write_to_key(OK_KEY *ok, uint8_t slot, const void *buf, int bufcount)
{
	OK_FRAME frame;
	unsigned char repbuf[FEATURE_RPT_SIZE];
	int i, seq;
	int ret = 0;
	unsigned char *ptr, *end;

	if (bufcount > sizeof(frame.payload)) {
		ok_errno = OK_EWRONGSIZ;
		return 0;
	}

	/* Insert data and set slot # */

	memset(&frame, 0, sizeof(frame));
	memcpy(frame.payload, buf, bufcount);
	frame.slot = slot;

	/* Append slot checksum */

	i = yubikey_crc16 (frame.payload, sizeof(frame.payload));
	frame.crc = ok_endian_swap_16(i);

	/* Chop up the data into parts that fits into the payload of a
	   feature report. Set the sequence number | 0x80 in the end
	   of the feature report. When the Yubikey has processed it,
	   it will clear this byte, signaling that the next part can be
	   sent */

	ptr = (unsigned char *) &frame;
	end = (unsigned char *) &frame + sizeof(frame);

#ifdef OK_DEBUG
	fprintf(stderr, "OK_DEBUG: Write %i bytes to YubiKey :\n", bufcount);
#endif
	for (seq = 0; ptr < end; seq++) {
		int all_zeros = 1;
		/* Ignore parts that are all zeroes except first and last
		   to speed up the transfer */

		for (i = 0; i < (FEATURE_RPT_SIZE - 1); i++) {
			if ((repbuf[i] = *ptr++)) all_zeros = 0;
		}
		if (all_zeros && (seq > 0) && (ptr < end))
			continue;

		/* sequence number goes into lower bits of last byte */
		repbuf[i] = seq | SLOT_WRITE_FLAG;

		/* When the Yubikey clears the SLOT_WRITE_FLAG, the
		 * next part can be sent.
		 */
		if (! ok_wait_for_key_status(ok, slot, 0, WAIT_FOR_WRITE_FLAG,
					     false, SLOT_WRITE_FLAG, NULL))
			goto end;
#ifdef OK_DEBUG
		_ok_hexdump(repbuf, FEATURE_RPT_SIZE);
#endif
		if (!_okusb_write(ok, REPORT_TYPE_FEATURE, 0,
				  (char *)repbuf, FEATURE_RPT_SIZE))
			goto end;
	}

	ret = 1;
end:
	insecure_memzero(&frame, sizeof(OK_FRAME));
	insecure_memzero(repbuf, sizeof(repbuf));
	return ret;
}

int ok_force_key_update(OK_KEY *ok)
{
	unsigned char buf[FEATURE_RPT_SIZE];

	memset(buf, 0, sizeof(buf));
	buf[FEATURE_RPT_SIZE - 1] = DUMMY_REPORT_WRITE; /* Invalid sequence = update only */
	if (!_okusb_write(ok, REPORT_TYPE_FEATURE, 0, (char *)buf, FEATURE_RPT_SIZE))
		return 0;

	return 1;
}

int ok_get_key_vid_pid(OK_KEY *ok, int *vid, int *pid) {
	return _okusb_get_vid_pid(ok, vid, pid);
}

uint16_t ok_endian_swap_16(uint16_t x)
{
	static int testflag = -1;

	if (testflag == -1) {
		uint16_t testword = 0x0102;
		unsigned char *testchars = (unsigned char *)&testword;
		if (*testchars == '\1')
			testflag = 1; /* Big endian arch, swap needed */
		else
			testflag = 0; /* Little endian arch, no swap needed */
	}

	if (testflag)
		x = (x >> 8) | ((x & 0xff) << 8);

	return x;
}
