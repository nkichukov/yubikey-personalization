/* -*- mode:C; c-file-style: "bsd" -*- */
/*
 * Copyright (c) 2008-2014 Yubico AB
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

#ifndef	__OKPERS_H_INCLUDED__
#define	__OKPERS_H_INCLUDED__

#include <stddef.h>
#include <stdbool.h>
#include <okstatus.h>
#include <okdef.h>

# ifdef __cplusplus
extern "C" {
# endif

typedef struct okp_config_t OKP_CONFIG;

/* This only works with Yubikey 1 unless it's fed with a OK_STATUS using
   okp_configure_for(). */
OKP_CONFIG *okp_create_config(void);
int okp_free_config(OKP_CONFIG *cfg);

/* allocate an empty OKP_CONFIG, use okp_configure_version() to set
   version information. */
OKP_CONFIG *okp_alloc(void);

/* Set the version information in st in cfg. */
void okp_configure_version(OKP_CONFIG *cfg, OK_STATUS *st);

/* This is used to tell what YubiKey version we're working with and what
   command we want to send to it. If this isn't used YubiKey 1 only will
   be assumed. */
int okp_configure_command(OKP_CONFIG *cfg, uint8_t command);
/* wrapper function for okp_configure_command */
int okp_configure_for(OKP_CONFIG *cfg, int confnum, OK_STATUS *st);

int okp_AES_key_from_hex(OKP_CONFIG *cfg, const char *hexkey);
int okp_AES_key_from_raw(OKP_CONFIG *cfg, const char *key);
int okp_AES_key_from_passphrase(OKP_CONFIG *cfg, const char *passphrase,
				const char *salt);
int okp_HMAC_key_from_hex(OKP_CONFIG *cfg, const char *hexkey);
int okp_HMAC_key_from_raw(OKP_CONFIG *cfg, const char *key);

/* Functions for constructing the OK_NDEF struct before writing it to a neo */
OK_NDEF *okp_alloc_ndef(void);
int okp_free_ndef(OK_NDEF *ndef);
int okp_construct_ndef_uri(OK_NDEF *ndef, const char *uri);
int okp_construct_ndef_text(OK_NDEF *ndef, const char *text, const char *lang, bool isutf16);
int okp_set_ndef_access_code(OK_NDEF *ndef, unsigned char *access_code);
int okp_ndef_as_text(OK_NDEF *ndef, char *text, size_t len);

OK_DEVICE_CONFIG *okp_alloc_device_config(void);
int okp_free_device_config(OK_DEVICE_CONFIG *device_config);
int okp_set_device_mode(OK_DEVICE_CONFIG *device_config, unsigned char mode);
int okp_set_device_chalresp_timeout(OK_DEVICE_CONFIG *device_config, unsigned char timeout);
int okp_set_device_autoeject_time(OK_DEVICE_CONFIG *device_config, unsigned short eject_time);

int okp_set_access_code(OKP_CONFIG *cfg, unsigned char *access_code, size_t len);
int okp_set_fixed(OKP_CONFIG *cfg, unsigned char *fixed, size_t len);
int okp_set_uid(OKP_CONFIG *cfg, unsigned char *uid, size_t len);
int okp_set_oath_imf(OKP_CONFIG *cfg, unsigned long imf);
unsigned long okp_get_oath_imf(const OKP_CONFIG *cfg);

int okp_set_tktflag_TAB_FIRST(OKP_CONFIG *cfg, bool state);
int okp_set_tktflag_APPEND_TAB1(OKP_CONFIG *cfg, bool state);
int okp_set_tktflag_APPEND_TAB2(OKP_CONFIG *cfg, bool state);
int okp_set_tktflag_APPEND_DELAY1(OKP_CONFIG *cfg, bool state);
int okp_set_tktflag_APPEND_DELAY2(OKP_CONFIG *cfg, bool state);
int okp_set_tktflag_APPEND_CR(OKP_CONFIG *cfg, bool state);
int okp_set_tktflag_PROTECT_CFG2(OKP_CONFIG *cfg, bool state);
int okp_set_tktflag_OATH_HOTP(OKP_CONFIG *cfg, bool state);
int okp_set_tktflag_CHAL_RESP(OKP_CONFIG *cfg, bool state);

int okp_set_cfgflag_SEND_REF(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_TICKET_FIRST(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_PACING_10MS(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_PACING_20MS(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_ALLOW_HIDTRIG(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_STATIC_TICKET(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_SHORT_TICKET(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_STRONG_PW1(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_STRONG_PW2(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_MAN_UPDATE(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_OATH_HOTP8(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_OATH_FIXED_MODHEX1(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_OATH_FIXED_MODHEX2(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_OATH_FIXED_MODHEX(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_CHAL_YUBICO(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_CHAL_HMAC(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_HMAC_LT64(OKP_CONFIG *cfg, bool state);
int okp_set_cfgflag_CHAL_BTN_TRIG(OKP_CONFIG *cfg, bool state);

int okp_set_extflag_SERIAL_BTN_VISIBLE(OKP_CONFIG *cfg, bool state);
int okp_set_extflag_SERIAL_USB_VISIBLE(OKP_CONFIG *cfg, bool state);
int okp_set_extflag_SERIAL_API_VISIBLE (OKP_CONFIG *cfg, bool state);
int okp_set_extflag_USE_NUMERIC_KEYPAD (OKP_CONFIG *cfg, bool state);
int okp_set_extflag_FAST_TRIG (OKP_CONFIG *cfg, bool state);
int okp_set_extflag_ALLOW_UPDATE (OKP_CONFIG *cfg, bool state);
int okp_set_extflag_DORMANT (OKP_CONFIG *cfg, bool state);
int okp_set_extflag_LED_INV (OKP_CONFIG *cfg, bool state);

bool okp_get_tktflag_TAB_FIRST(const OKP_CONFIG *cfg);
bool okp_get_tktflag_APPEND_TAB1(const OKP_CONFIG *cfg);
bool okp_get_tktflag_APPEND_TAB2(const OKP_CONFIG *cfg);
bool okp_get_tktflag_APPEND_DELAY1(const OKP_CONFIG *cfg);
bool okp_get_tktflag_APPEND_DELAY2(const OKP_CONFIG *cfg);
bool okp_get_tktflag_APPEND_CR(const OKP_CONFIG *cfg);
bool okp_get_tktflag_PROTECT_CFG2(const OKP_CONFIG *cfg);
bool okp_get_tktflag_OATH_HOTP(const OKP_CONFIG *cfg);
bool okp_get_tktflag_CHAL_RESP(const OKP_CONFIG *cfg);

bool okp_get_cfgflag_SEND_REF(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_TICKET_FIRST(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_PACING_10MS(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_PACING_20MS(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_ALLOW_HIDTRIG(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_STATIC_TICKET(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_SHORT_TICKET(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_STRONG_PW1(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_STRONG_PW2(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_MAN_UPDATE(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_OATH_HOTP8(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_OATH_FIXED_MODHEX1(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_OATH_FIXED_MODHEX2(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_OATH_FIXED_MODHEX(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_CHAL_YUBICO(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_CHAL_HMAC(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_HMAC_LT64(const OKP_CONFIG *cfg);
bool okp_get_cfgflag_CHAL_BTN_TRIG(const OKP_CONFIG *cfg);

bool okp_get_extflag_SERIAL_BTN_VISIBLE(const OKP_CONFIG *cfg);
bool okp_get_extflag_SERIAL_USB_VISIBLE(const OKP_CONFIG *cfg);
bool okp_get_extflag_SERIAL_API_VISIBLE (const OKP_CONFIG *cfg);
bool okp_get_extflag_USE_NUMERIC_KEYPAD (const OKP_CONFIG *cfg);
bool okp_get_extflag_FAST_TRIG (const OKP_CONFIG *cfg);
bool okp_get_extflag_ALLOW_UPDATE (const OKP_CONFIG *cfg);
bool okp_get_extflag_DORMANT (const OKP_CONFIG *cfg);
bool okp_get_extflag_LED_INV (const OKP_CONFIG *cfg);

int okp_clear_config(OKP_CONFIG *cfg);

int okp_write_config(const OKP_CONFIG *cfg,
		     int (*writer)(const char *buf, size_t count,
				   void *userdata),
		     void *userdata);
int okp_read_config(OKP_CONFIG *cfg,
		    int (*reader)(char *buf, size_t count,
				  void *userdata),
		    void *userdata);

OK_CONFIG *okp_core_config(OKP_CONFIG *cfg);
int okp_command(OKP_CONFIG *cfg);
int okp_config_num(OKP_CONFIG *cfg);

int okp_export_config(const OKP_CONFIG *cfg, char *buf, size_t len, int format);
int okp_import_config(OKP_CONFIG *cfg, const char *buf, size_t len, int format);

#define OKP_FORMAT_LEGACY	0x01
#define OKP_FORMAT_YCFG		0x02

void okp_set_acccode_type(OKP_CONFIG *cfg, unsigned int type);
unsigned int okp_get_acccode_type(const OKP_CONFIG *cfg);

#define OKP_ACCCODE_NONE	0x01
#define OKP_ACCCODE_RANDOM	0x02
#define OKP_ACCCODE_SERIAL	0x03

int okp_get_supported_key_length(const OKP_CONFIG *cfg);

extern int * _okp_errno_location(void);
#define okp_errno (*_okp_errno_location())
const char *okp_strerror(int errnum);

#define OKP_ENOTYETIMPL	0x01
#define OKP_ENOCFG	0x02
#define OKP_EYUBIKEYVER	0x03
#define OKP_EOLDYUBIKEY	0x04
#define OKP_EINVCONFNUM	0x05
#define OKP_EINVAL	0x06
#define OKP_ENORANDOM	0x07

# ifdef __cplusplus
}
# endif

#endif	/* __OKPERS_H_INCLUDED__ */
