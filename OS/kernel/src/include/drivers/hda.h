#ifndef _HDA_H
#define _HDA_H
#include <stdint.h>

#define HDA_UNINITALIZED 0
#define HDA_CORB_RIRB 1

#define HDA_WIDGET_AUDIO_OUTPUT 0x0
#define HDA_WIDGET_AUDIO_INPUT 0x1
#define HDA_WIDGET_AUDIO_MIXER 0x2
#define HDA_WIDGET_AUDIO_SELECTOR 0x3
#define HDA_WIDGET_PIN_COMPLEX 0x4
#define HDA_WIDGET_POWER_WIDGET 0x5
#define HDA_WIDGET_VOLUME_KNOB 0x6
#define HDA_WIDGET_BEEP_GENERATOR 0x7
#define HDA_WIDGET_VENDOR_DEFINED 0xF

#define HDA_PIN_LINE_OUT 0x0
#define HDA_PIN_SPEAKER 0x1
#define HDA_PIN_HEADPHONE_OUT 0x2
#define HDA_PIN_CD 0x3
#define HDA_PIN_SPDIF_OUT 0x4
#define HDA_PIN_DIGITAL_OTHER_OUT 0x5
#define HDA_PIN_MODEM_LINE_SIDE 0x6
#define HDA_PIN_MODEM_HANDSET_SIDE 0x7
#define HDA_PIN_LINE_IN 0x8
#define HDA_PIN_AUX 0x9
#define HDA_PIN_MIC_IN 0xA
#define HDA_PIN_TELEPHONY 0xB
#define HDA_PIN_SPDIF_IN 0xC
#define HDA_PIN_DIGITAL_OTHER_IN 0xD
#define HDA_PIN_RESERVED 0xE
#define HDA_PIN_OTHER 0xF

#define HDA_OUTPUT_NODE 0x1
#define HDA_INPUT_NODE 0x2

void hda_init(uintptr_t bar_tbl_addr);
uint32_t hda_send_verb(uint32_t codec, uint32_t node, uint32_t verb, uint32_t command);
void hda_init_codec(uint32_t codec_number);
void hda_init_afg(uint32_t codec_number, uint32_t afg_node_number);
uint8_t hda_get_node_type(uint32_t codec, uint32_t node);
uint16_t hda_get_node_connection_entry(uint32_t codec, uint32_t node, uint32_t cen);
void hda_init_output_pin(uint32_t codec_number, uint32_t pin_node_number);
void hda_set_node_gain(uint32_t codec, uint32_t node, uint32_t node_type, uint32_t capabilities, uint32_t gain);
void hda_init_audio_output(uint32_t codec_number, uint32_t audio_output_node_number);
void hda_play_pcm_data(uintptr_t addr);
uint16_t hda_return_sound_data_format(uint32_t sample_rate, uint32_t channels, uint32_t bits_per_sample);

#endif
