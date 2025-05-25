/*
// Hojuix HD Audio Driver
// Ported from NetBSD (Commit ff5b3bc5fab4a56deaec4825fb1b8a154e873deb)
// The original license is below
*/

/* $NetBSD: hdaudio.c,v 1.18 2022/04/07 19:33:37 andvar Exp $ */
/*
 * Copyright (c) 2009 Precedence Technologies Ltd <support@precedence.co.uk>
 * Copyright (c) 2009 Jared D. McNeill <jmcneill@invisible.ca>
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Precedence Technologies Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "netbsd_hdaudiovar.h"
#include "netbsd_hdaudioreg.h"

#define HDAUDIO_RESET_TIMEOUT   5000
#define HDAUDIO_CORB_TIMEOUT    1000
#define HDAUDIO_RIRB_TIMEOUT    5000

#define HDAUDIO_CODEC_DELAY 1000    /* spec calls for 250 */

static void hdaudio_init(struct hdaudio_softc *sc)
{
    const uint8_t vmaj = hda_read1(sc, HDAUDIO_MMIO_VMAJ);
    const uint8_t vmin = hda_read1(sc, HDAUDIO_MMIO_VMIN);
    const uint16_t gcap = hda_read2(sc, HDAUDIO_MMIO_GCAP);
    const int nis = HDAUDIO_GCAP_ISS(gcap);
    const int nos = HDAUDIO_GCAP_OSS(gcap);
    const int nbidir = HDAUDIO_GCAP_BSS(gcap);
    const int nsdo = HDAUDIO_GCAP_NSDO(gcap);
    const int addr64 = HDAUDIO_GCAP_64OK(gcap);

    hda_print(sc, "HDA ver. %d.%d, OSS %d, ISS %d, BSS %d, SDO %d%s\n",
        vmaj, vmin, nos, nis, nbidir, nsdo, addr64 ? ", 64-bit" : "");

    /* Initialize codecs and streams */
    hdaudio_codec_init(sc);
    hdaudio_stream_init(sc, nis, nos, nbidir);
}