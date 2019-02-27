/* -*- mode:C; c-file-style: "bsd" -*- */
/*
 * Copyright (c) 2008-2014 Yubico AB
 * Copyright (c) 2010 Tollef Fog Heen <tfheen@err.no>
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

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <okpers.h>
#include <okdef.h>
#include <okpers-version.h>

#include "okpers-args.h"

int main(int argc, char **argv)
{
	FILE *inf = NULL; const char *infname = NULL;
	FILE *outf = NULL; const char *outfname = NULL;
	int data_format = OKP_FORMAT_LEGACY;
	bool verbose = false;
	unsigned char access_code[256];
	char *acc_code = NULL;
	char *new_acc_code = NULL;
	unsigned char scan_codes[sizeof(SCAN_MAP)];
	unsigned char device_info[128];
	size_t device_info_len = 0;
	OK_KEY *ok = 0;
	OKP_CONFIG *cfg = okp_alloc();
	OK_STATUS *st = okds_alloc();
	bool autocommit = false;
	char data[1024];
	bool dry_run = false;

	/* Options */
	char oathid[128] = {0};
	char ndef_string[128] = {0};
	char ndef_type = 0;
	unsigned char usb_mode = 0;
	unsigned char cr_timeout = 0;
	unsigned short autoeject_timeout = 0;
	int num_modes_seen = 0;
	bool zap = false;
	int key_index = 0;

	/* Assume the worst */
	bool error = true;
	int exit_code = 0;

	int c;

	okp_errno = 0;
	ok_errno = 0;

	while((c = getopt(argc, argv, optstring)) != -1) {
		switch(c) {
			case 'h':
				fputs(usage, stderr);
				exit(0);
			case 'N':
				key_index = atoi(optarg);
				break;
			case 'V':
				fputs(OKPERS_VERSION_STRING "\n", stderr);
				return 0;
			case ':':
				switch(optopt) {
					case 'S':
						continue;
					case 'a':
						continue;
					case 'c':
						continue;
				}
				fprintf(stderr, "Option %c requires an argument.\n", optopt);
				exit(1);
				break;
			default:
				continue;
		}
	}
	optind = 1;

	if (!ok_init()) {
		exit_code = 1;
		goto err;
	}

	if (!(ok = ok_open_key(key_index))) {
		exit_code = 1;
		goto err;
	}

	if (!ok_get_status(ok, st)) {
		exit_code = 1;
		goto err;
	}

	printf("Firmware version %d.%d.%d Touch level %d ",
	       okds_version_major(st),
	       okds_version_minor(st),
	       okds_version_build(st),
	       okds_touch_level(st));
	if (okds_pgm_seq(st))
		printf("Program sequence %d\n",
		       okds_pgm_seq(st));
	else
		printf("Unconfigured\n");

	if (!(ok_check_firmware_version2(st))) {
		if (ok_errno == OK_EFIRMWARE) {
			printf("Unsupported firmware revision - some "
			       "features may not be available\n"
			       "Please see \n"
			       "https://developers.yubico.com/yubikey-personalization/doc/Compatibility.html\n"
			       "for more information.\n");
		} else {
			goto err;
		}
	}

	/* Parse all arguments in a testable way */
	if (! args_to_config(argc, argv, cfg, oathid, sizeof(oathid),
			     &infname, &outfname,
			     &data_format, &autocommit,
			     st, &verbose, &dry_run,
			     &acc_code, &new_acc_code,
			     &ndef_type, ndef_string, sizeof(ndef_string),
			     &usb_mode, &zap, scan_codes, &cr_timeout,
			     &autoeject_timeout, &num_modes_seen,
					 device_info, &device_info_len, &exit_code)) {
		goto err;
	}

	if (oathid[0] != 0) {
		set_oath_id(oathid, cfg, ok, st);
	}

	if (acc_code) {
		size_t access_code_len = 0;
		int rc = hex_modhex_decode(access_code, &access_code_len,
				acc_code, strlen(acc_code),
				12, 12, false);
		if (rc <= 0) {
			fprintf(stderr,
					"Invalid access code string: %s\n",
					optarg);
			exit_code = 1;
			goto err;
		}
		if (!new_acc_code) {
			okp_set_access_code(cfg,
					access_code,
					access_code_len);
		}
	}
	if(new_acc_code) {
		unsigned char accbin[256];
		size_t accbinlen = 0;
		int rc = hex_modhex_decode (accbin, &accbinlen,
				new_acc_code, strlen(new_acc_code),
				12, 12, false);
		if (rc <= 0) {
			fprintf(stderr,
					"Invalid access code string: %s\n",
					new_acc_code);
			exit_code = 1;
			goto err;
		}
		okp_set_access_code(cfg, accbin, accbinlen);
	}

	if (verbose && (okds_version_major(st) > 2 ||
			(okds_version_major(st) == 2 &&
			 okds_version_minor(st) >= 2) ||
			(okds_version_major(st) == 2 && // neo has serial functions
			 okds_version_minor(st) == 1 &&
			 okds_version_build(st) >= 4))) {
		unsigned int serial;
		if (! ok_get_serial(ok, 0, 0, &serial)) {
			printf ("Failed to read serial number (serial-api-visible disabled?).\n");

		} else {
			printf ("Serial number : %i\n", serial);
		}
	}

	printf ("\n");

	if (infname) {
		if (strcmp(infname, "-") == 0)
			inf = stdin;
		else
			inf = fopen(infname, "r");
		if (inf == NULL) {
			fprintf(stderr,
				"Couldn't open %s for reading: %s\n",
				infname,
				strerror(errno));
			exit_code = 1;
			goto err;
		}
	}

	if (outfname) {
		if (strcmp(outfname, "-") == 0)
			outf = stdout;
		else
			outf = fopen(outfname, "w");
		if (outf == NULL) {
			fprintf(stderr,
				"Couldn't open %s for writing: %s\n",
				outfname,
				strerror(errno));
			exit(1);
		}
	}

	if (inf) {
		if(!okp_clear_config(cfg))
			goto err;
		if(!fread(data, 1, 1024, inf))
			goto err;
		if (!okp_import_config(cfg, data, strlen(data), data_format))
			goto err;
	}
	if (outf) {
		if(!(okp_export_config(cfg, data, 1024, data_format))) {
			goto err;
		}
		if(!(fwrite(data, 1, strlen(data), outf))) {
			goto err;
		}
	} else {
		char commitbuf[256]; size_t commitlen;

		if (okp_command(cfg) == SLOT_SWAP) {
			fprintf(stderr, "Configuration in slot 1 and 2 will be swapped\n");
		} else if(okp_command(cfg) == SLOT_NDEF || okp_command(cfg) == SLOT_NDEF2) {
			fprintf(stderr, "New NDEF will be written as:\n%s\n", ndef_string);
		} else if(okp_command(cfg) == SLOT_DEVICE_CONFIG) {
			fprintf(stderr, "The USB mode will be set to: 0x%x\n", usb_mode);
			if(num_modes_seen > 1) {
				fprintf(stderr, "The challenge response timeout will be set to: %d\n", cr_timeout);
				if(num_modes_seen > 2) {
					fprintf(stderr, "The smartcard autoeject timeout will be set to: %d\n", autoeject_timeout);
				}
			}
		} else if(okp_command(cfg) == SLOT_SCAN_MAP) {
			fprintf(stderr, "A new scanmap will be written.\n");
		} else if(okp_command(cfg) == SLOT_OK4_SET_DEVICE_INFO) {
			fprintf(stderr, "New device information will be written.\n");
		} else if(zap) {
			fprintf(stderr, "Configuration in slot %d will be deleted\n", okp_config_num(cfg));
		} else {
			if (okp_command(cfg) == SLOT_CONFIG || okp_command(cfg) == SLOT_CONFIG2) {
				fprintf(stderr, "Configuration data to be written to key configuration %d:\n\n", okp_config_num(cfg));
			} else {
				fprintf(stderr, "Configuration data to be updated in key configuration %d:\n\n", okp_command(cfg) == SLOT_UPDATE1 ? 1 : 2);
			}
			okp_export_config(cfg, data, 1024, OKP_FORMAT_LEGACY);
			fwrite(data, 1, strlen(data), stderr);
		}
		fprintf(stderr, "\nCommit? (y/n) [n]: ");
		if (autocommit) {
			strcpy(commitbuf, "yes");
			puts(commitbuf);
		} else {
			if (!fgets(commitbuf, sizeof(commitbuf), stdin))
			{
				perror ("fgets");
				goto err;
			}
		}
		commitlen = strlen(commitbuf);
		if (commitlen > 0 && commitbuf[commitlen - 1] == '\n')
			commitbuf[commitlen - 1] = '\0';
		if (strcmp(commitbuf, "y") == 0
		    || strcmp(commitbuf, "yes") == 0) {
			exit_code = 2;

			if (verbose)
				printf("Attempting to write configuration to the yubikey...");
			if (dry_run) {
				printf("Not writing anything to key due to dry_run requested.\n");
			}
			else if(okp_command(cfg) == SLOT_NDEF || okp_command(cfg) == SLOT_NDEF2) {
				OK_NDEF *ndef = okp_alloc_ndef();
				int confnum = 1;
				int res = 0;
				if(ndef_type == 'U') {
					res = okp_construct_ndef_uri(ndef, ndef_string);
				} else if(ndef_type == 'T') {
					res = okp_construct_ndef_text(ndef, ndef_string, "en", false);
				}
				if(!res) {
					if(verbose) {
						printf(" failure to construct ndef\n");
					}
					goto err;
				}
				if(acc_code) {
					if(!okp_set_ndef_access_code(ndef, access_code)) {
						if(verbose) {
							printf(" failure to set ndef accesscode\n");
						}
						goto err;
					}
				}
				if(okp_command(cfg) == SLOT_NDEF2) {
					confnum = 2;
				}
				if (!ok_write_ndef2(ok, ndef, confnum)) {
					if (verbose)
						printf(" failure to write ndef\n");
					goto err;
				}
				okp_free_ndef(ndef);
			} else if(okp_command(cfg) == SLOT_DEVICE_CONFIG) {
				OK_DEVICE_CONFIG *device_config = okp_alloc_device_config();
				okp_set_device_mode(device_config, usb_mode);
				if(num_modes_seen > 1) {
					okp_set_device_chalresp_timeout(device_config, cr_timeout);
					if(num_modes_seen > 2) {
						okp_set_device_autoeject_time(device_config, autoeject_timeout);
					}
				}

				if((usb_mode & 0xf) == MODE_CCID || (usb_mode & 0xf) == MODE_U2F ||
						(usb_mode & 0xf) == MODE_U2F_CCID) {
					fprintf(stderr, "WARNING: Changing mode will require you to use another tool (okneomgr or u2f-host) to switch back if OTP mode is disabled, really commit? (y/n) [n]: ");
					if (autocommit) {
						strcpy(commitbuf, "yes");
						puts(commitbuf);
					} else {
						if (!fgets(commitbuf, sizeof(commitbuf), stdin))
						{
							perror ("fgets");
							goto err;
						}
					}
					commitlen = strlen(commitbuf);
					if (commitlen > 0 && commitbuf[commitlen - 1] == '\n')
						commitbuf[commitlen - 1] = '\0';
					if (strcmp(commitbuf, "y") != 0
							&& strcmp(commitbuf, "yes") != 0) {
						exit_code = 0;
						error = false;
						goto err;
					}
				}

				if(!ok_write_device_config(ok, device_config)) {
					if(verbose)
						printf(" failure\n");
					goto err;
				}
				okp_free_device_config(device_config);


			} else if(okp_command(cfg) == SLOT_SCAN_MAP) {
				if(!ok_write_scan_map(ok, scan_codes)) {
					if(verbose)
						printf(" failure\n");
					goto err;
				}
			} else if(okp_command(cfg) == SLOT_OK4_SET_DEVICE_INFO) {
				if(!ok_write_device_info(ok, device_info, device_info_len)) {
					if(verbose)
						printf(" failure\n");
					goto err;
				}
			} else {
				OK_CONFIG *ycfg = NULL;
				/* if we're deleting a slot we send the configuration as NULL */
				if (!zap) {
					ycfg = okp_core_config(cfg);
				}
				if (!ok_write_command(ok,
							ycfg, okp_command(cfg),
							acc_code ? access_code : NULL)) {
					if (verbose)
						printf(" failure\n");
					goto err;
				}
			}

			if (verbose && !dry_run)
				printf(" success\n");
		}
	}

	exit_code = 0;
	error = false;

err:
	if (error) {
		report_ok_error();
	}

	if (st)
		free(st);
	if (inf)
		fclose(inf);
	if (outf)
		fclose(outf);

	if (ok && !ok_close_key(ok)) {
		report_ok_error();
		exit_code = 2;
	}

	if (!ok_release()) {
		report_ok_error();
		exit_code = 2;
	}

	if (cfg)
		okp_free_config(cfg);

	free(acc_code);
	free(new_acc_code);

	exit(exit_code);
}
