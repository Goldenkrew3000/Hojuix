#include <stdio.h>
#include <drivers/hda.h>
#include <memory/vmmgr.h>
#include <memory/pmmgr.h>
#include <drivers/pci.h>
#include <drivers/i386/pit_timer.h>

#include <string.h>

//https://github.com/VendelinSlezak/BleskOS/blob/master/source/drivers/sound/hda.c#L339https://github.com/VendelinSlezak/BleskOS/blob/master/source/drivers/sound/hda.c

uint8_t hda_mmio_in8(uint64_t base);
void hda_mmio_out8(uint64_t base, uint8_t value);
uint16_t hda_mmio_in16(uint64_t base);
void hda_mmio_out16(uint64_t base, uint16_t value);
uint32_t hda_mmio_in32(uint64_t base);
void hda_mmio_out32(uint64_t base, uint32_t value);

uintptr_t hda_base = 0;
uintptr_t hda_input_stream_base = 0;
uintptr_t hda_output_stream_base = 0;
uintptr_t hda_output_buffer_list = 0;
uintptr_t hda_corb_mem = 0;
uint32_t* hda_corb_mem_ptr = NULL;
int hda_corb_number_of_entries = 0;
uint32_t hda_corb_pointer = 0;
uintptr_t hda_rirb_mem = 0;
int hda_rirb_number_of_entries = 0;
int hda_rirb_pointer = 0;
uint32_t* hda_rirb_mem_ptr = NULL;
uint8_t hda_communication_type = 0;

void hda_init(uintptr_t bar_tbl_addr) {
    printf("[HDA] Init.\n");

    // Load the PCI BAR Table
    pci_device_bar_table_t* pci_bar_tbl = (pci_device_bar_table_t*)bar_tbl_addr;

    // Enable DMA and MMIO in the PCI Command Register
    uint16_t pci_command_register = pci_readWord(pci_bar_tbl->bus, pci_bar_tbl->slot, pci_bar_tbl->func, PCI_OFFSET_COMMAND);
    pci_command_register &= ~(0x1 << 9);
    pci_command_register |= (0x3 << 1);
    pci_writeWord(pci_bar_tbl->bus, pci_bar_tbl->slot, pci_bar_tbl->func, PCI_OFFSET_COMMAND, pci_command_register);

    // Map the HDA controller
    //hda_base = pci_bar_tbl->bar0_addr; // TODO I think this is 64 bit sometimes (Maybe involving BAR1)
    //vmmgr_mmio_map_uncache(hda_base, hda_base, 8); // TODO Probably dont need 8 pages here, read spec for this probs
    uint64_t hda_base_inter = (uint64_t)(((uint64_t)pci_bar_tbl->bar1_full << 32) | (pci_bar_tbl->bar0_full & 0xFFFFFFF0));
    hda_base = (uintptr_t)hda_base_inter;
    printf("HDA Base: %llx\n", hda_base);
    vmmgr_mmio_map_uncache(hda_base, hda_base, 8); // TODO Dont need 4 pages
    //nvme_mmio = (uint32_t*)nvme_base_addr; // Assign MMIO pointer

    // Reset the HDA controller and set to operational state
    hda_mmio_out32(hda_base + 0x08, 0x0);
    while ((hda_mmio_in32(hda_base + 0x08) & 0x1) != 0x0) { } // TODO Add spin and VERIFY
    hda_mmio_out32(hda_base + 0x08, 0x1);
    while ((hda_mmio_in32(hda_base + 0x08) & 0x1) != 0x1) { } // TODO Add spin and VERIFY

    // Read capabilities
    printf("[HDA] Version %d.%d\n", hda_mmio_in8(hda_base + 0x03), hda_mmio_in8(hda_base + 0x02));
    hda_input_stream_base = hda_base + 0x80;
    hda_output_stream_base = (hda_base + 0x80 + (0x20 * ((hda_mmio_in16(hda_base + 0x00) >> 8) & 0xF)));
    hda_output_buffer_list = pmmgr_kmalloc(1); // Aligned to 0x80, 32 bytes needed

    // Disable interrupts
    hda_mmio_out32(hda_base + 0x20, 0);

    // Turn off DMA position transfer
    hda_mmio_out32(hda_base + 0x70, 0);
    hda_mmio_out32(hda_base + 0x74, 0);

    // Disable synchronization
    hda_mmio_out32(hda_base + 0x34, 0);
    hda_mmio_out32(hda_base + 0x38, 0);

    // Stop CORB and RIRB
    hda_mmio_out8(hda_base + 0x4C, 0x0);
    hda_mmio_out8(hda_base + 0x5C, 0x0);

    // Configure CORB
    hda_corb_mem = pmmgr_kmalloc(1); // Aligned to 0x80, 2048 bytes needed
    printf("CORB Mem: 0x%llx\n", hda_corb_mem);
    hda_corb_mem_ptr = (uint32_t*)(hda_corb_mem + 0xFFFF800000000000);
    hda_mmio_out32(hda_base + 0x40, (uint32_t)hda_corb_mem);
    hda_mmio_out32(hda_base + 0x44, 0);
    hda_corb_number_of_entries = 0;
    if ((hda_mmio_in8(hda_base + 0x4E) & 0x40) == 0x40) {
        hda_corb_number_of_entries = 256;
        hda_mmio_out8(hda_base + 0x4E, 0x2); // 256 entries
        printf("[HDA] CORB is 256 entries.\n");
    } else if ((hda_mmio_in8(hda_base + 0x4E) & 0x20) == 0x20) {
        hda_corb_number_of_entries = 16;
        hda_mmio_out8(hda_base + 0x4E, 0x1); // 16 entries
        printf("[HDA] CORB is 16 entries.\n");
    } else if ((hda_mmio_in8(hda_base + 0x4E) & 0x10) == 0x10) {
        hda_corb_number_of_entries = 2;
        hda_mmio_out8(hda_base + 0x4E, 0x0); // 2 entries
        printf("[HDA] CORB is 2 entries.\n");
    } else {
        printf("[HDA] No CORB size allowed.\n");
        // Go to using the PIO interface TODO
    }

    // Reset read pointer
    //hda_mmio_out16(hda_base + 0x4A, 0x8000);
    //while ((hda_mmio_in16(hda_base + 0x4A) & 0x8000) != 0x8000) { } // TODO add spin
    // If this does not succeed, use PIO interface
    //hda_mmio_out16(hda_base + 0x4A, 0x0000);
    //while ((hda_mmio_in16(hda_base + 0x4A) & 0x8000) != 0x0000) { } // TODO add spin
    // Same deal here

    hda_mmio_out16(hda_base + 0x4A, 0x8000);
    int ticks = 0;
    while (ticks < 500000) {
        asm volatile("nop");
        if ((hda_mmio_in16(hda_base + 0x4A) & 0x8000) == 0x8000) {
            break;
        }
        ticks++;
    }
    if ((hda_mmio_in16(hda_base + 0x4A) & 0x8000) == 0x0000) {
        printf("[HDA] CORB pointer cannot be put into reset state\n");
        // USE PIO
    }
    hda_mmio_out16(hda_base + 0x4A, 0x0000);
    ticks = 0;
    while (ticks < 500000) {
        asm volatile("nop");
        if ((hda_mmio_in16(hda_base + 0x4A) & 0x8000) == 0x0000) {
            break;
        }
        ticks++;
    }
    if ((hda_mmio_in16(hda_base + 0x4A) & 0x8000) == 0x8000) {
        printf("[HDA] CORB pointer cannot be put into reset state(2)\n");
        // USE PIO
    }
    printf("[HDA] Made it out\n");

    // Set write pointer
    hda_mmio_out16(hda_base + 0x48, 0);
    hda_corb_pointer = 1;

    // Configure RIRB
    hda_rirb_mem = pmmgr_kmalloc(1); // 2048 bytes, 0x80 aligned
    hda_rirb_mem_ptr = (uint32_t*)(hda_rirb_mem + 0xFFFF800000000000);
    hda_mmio_out32(hda_base + 0x50, (uint32_t)hda_rirb_mem);
    hda_mmio_out32(hda_base + 0x54, 0);
    hda_rirb_number_of_entries = 0;
    if ((hda_mmio_in8(hda_base + 0x5E) & 0x40) == 0x40) {
        hda_rirb_number_of_entries = 256;
        hda_mmio_out8(hda_base + 0x5E, 0x2); // 256 entries
        printf("[HDA] RIRB is 256 entries.\n");
    } else if ((hda_mmio_in8(hda_base + 0x5E) & 0x20) == 0x20) {
        hda_rirb_number_of_entries = 16;
        hda_mmio_out8(hda_base + 0x5E, 0x1); // 16 entries
        printf("[HDA] RIRB is 16 entries.\n");
    } else if ((hda_mmio_in8(hda_base + 0x5E) & 0x10) == 0x10) {
        hda_rirb_number_of_entries = 2;
        hda_mmio_out8(hda_base + 0x5E, 0x0); // 2 entries
        printf("[HDA] RIRB is 2 entries.\n");
    } else {
        printf("[HDA] No RIRB size allowed.\n");
        // Go to using the PIO interface TODO
    }

    // Reset write pointer
    hda_mmio_out16(hda_base + 0x58, 0x8000);
    timer_wait(10);

    // Disable interrupts
    hda_mmio_out16(hda_base + 0x5A, 0xFFFF); // Set to 0xFFFF for it to work - https://forum.osdev.org/viewtopic.php?t=56072
    hda_rirb_pointer = 1;

    // Start CORB and RIRB
    hda_mmio_out8(hda_base + 0x4C, 0x2);
    hda_mmio_out8(hda_base + 0x5C, 0x2);

    // Find coded and working communication interface
    hda_communication_type = 0;
    //for (uint32_t codec_number = 0, codec_id = 0; codec_number < 16; codec_number++) {
    for (uint32_t codec_number = 0, codec_id = 0; codec_number < 1; codec_number++) {
        hda_communication_type = HDA_CORB_RIRB;
        codec_id = hda_send_verb(codec_number, 0, 0xF00, 0);
        printf("Codec ID: %llx\n", codec_id);
        if (codec_id != 0) {
            hda_init_codec(codec_number);
        }
    }
}

uint32_t hda_send_verb(uint32_t codec, uint32_t node, uint32_t verb, uint32_t command) {
    uint32_t value = ((codec << 28) | (node << 20) | (verb << 8) | (command));

    if (hda_communication_type == HDA_CORB_RIRB) { // CORB/RIRB Interface
        // Write verb
        hda_corb_mem_ptr[hda_corb_pointer] = value;
        asm volatile("wbinvd"); // Flush CPU cache

        // Move write pointer
        hda_mmio_out16(hda_base + 0x48, hda_corb_pointer);

        // Wait for response
        uint32_t spin = 0;
        while (spin < 500000) {
            asm volatile("nop");
            if (hda_mmio_in16(hda_base + 0x58) == hda_corb_pointer) {
                break;
            }
            spin++;
        }
        if (hda_mmio_in16(hda_base + 0x58) != hda_corb_pointer) {
            //printf("[HDA] No response.\n");
            // Error here
        }

       // Read response
       value = hda_rirb_mem_ptr[hda_rirb_pointer * 2];

       // Move pointers
       hda_corb_pointer++;
       if (hda_corb_pointer == hda_corb_number_of_entries) { hda_corb_pointer = 0; }
       hda_rirb_pointer++;
       if (hda_rirb_pointer == hda_rirb_number_of_entries) { hda_rirb_pointer = 0; }
    }

    return value;
}

void hda_init_codec(uint32_t codec_number) {
    printf("Initializing codec %ld\n", codec_number);
    
    // Test if the codec actually exists
    uint32_t codec_id = hda_send_verb(codec_number, 0, 0xF00, 0);
    if (codec_id == 0x00000000) {
        printf("[HDA] Codec does not exist.\n");
    }

    // Find Audio Function Groups
    uint32_t subordinate_node_count_res = hda_send_verb(codec_number, 0, 0xF00, 0x04);
    printf("First group node: %d\n", (subordinate_node_count_res >> 16) & 0xFF);
    printf("Number of groups: %d\n", subordinate_node_count_res & 0xFF);
    for(uint32_t node = ((subordinate_node_count_res>>16) & 0xFF), last_node = (node+(subordinate_node_count_res & 0xFF)); node<last_node; node++) {
        if((hda_send_verb(codec_number, node, 0xF00, 0x05) & 0x7F)==0x01) { //this is Audio Function Group
            //hda_initalize_audio_function_group(sound_card_number, node); //initalize Audio Function Group
            printf("This is an audio func group\n");
            hda_init_afg(codec_number, node);
            // Return here
        }
    }
}

void hda_init_afg(uint32_t codec_number, uint32_t afg_node_number) {
    printf("Initing AFG %ld\n", afg_node_number);

    // Reset AFG
    hda_send_verb(codec_number, afg_node_number, 0x7FF, 0x00);

    // Enable power for AFG
    hda_send_verb(codec_number, afg_node_number, 0x705, 0x00);

    // Disable unsolicited responses
    hda_send_verb(codec_number, afg_node_number, 0x708, 0x00);

    // Read information
    uint32_t afg_node_sample_capabilities = hda_send_verb(codec_number, afg_node_number, 0xF00, 0x0A);
    uint32_t afg_node_stream_format_capabilities = hda_send_verb(codec_number, afg_node_number, 0xF00, 0x0B);
    uint32_t afg_node_input_amp_capabilities = hda_send_verb(codec_number, afg_node_number, 0xF00, 0x0D);
    uint32_t afg_node_output_amp_capabilities = hda_send_verb(codec_number, afg_node_number, 0xF00, 0x12);

    // Log the AFG info
    printf("[AFG] Sample Capabilities: 0x%lx\n", afg_node_sample_capabilities);
    printf("[AFG] Stream Format Capabilities: 0x%lx\n", afg_node_stream_format_capabilities);
    printf("[AFG] Input Amp Capabilities: 0x%lx\n", afg_node_input_amp_capabilities);
    printf("[AFG] Output Amp Capabilities: 0x%lx\n", afg_node_output_amp_capabilities);

    // Log all of the AFG nodes and find the useful PINs
    uint32_t subordinate_node_count_res = hda_send_verb(codec_number, afg_node_number, 0xF00, 0x04);
    uint32_t pin_alternative_output_node_number = 0;
    uint32_t pin_speaker_default_node_number = 0;
    uint32_t pin_speaker_node_number = 0;
    uint32_t pin_headphone_node_number = 0;

    for(uint32_t node = ((subordinate_node_count_res>>16) & 0xFF), last_node = (node+(subordinate_node_count_res & 0xFF)), type_of_node = 0; node < last_node; node++) {
        // Get the type of node
        type_of_node = hda_get_node_type(codec_number, node);

        // Process node
        if (type_of_node == HDA_WIDGET_AUDIO_OUTPUT) {
            printf("Node is audio output\n");

            // Disable every audio output by connecting it to Stream 0
            hda_send_verb(codec_number, node, 0x706, 0x00);
        } else if (type_of_node == HDA_WIDGET_PIN_COMPLEX) {
            printf("Node is a Pin complex\n");
            
            // Read type of PIN
            type_of_node = ((hda_send_verb(codec_number, node, 0xF1C, 0x00) >> 20) & 0xF);

            if (type_of_node == HDA_PIN_LINE_OUT) {
                printf("PIN is line out\n");
                pin_alternative_output_node_number = node;
            } else if (type_of_node == HDA_PIN_SPEAKER) {
                printf("PIN is speaker\n");
            } else if (type_of_node == HDA_PIN_HEADPHONE_OUT) {
                printf("PIN is headphone out\n");
            } else {
                printf("PIN is other\n");
            }
        } else {
            printf("Node is not audio output or pin complex\n");
        }

        // Log all of the connected nodes
        uint8_t connection_entry_number = 0;
        uint16_t connection_entry_node = hda_get_node_connection_entry(codec_number, node, 0);
        while (connection_entry_node != 0x0000)
        {
            printf("%d ", connection_entry_node);
            connection_entry_number++;
            connection_entry_node = hda_get_node_connection_entry(codec_number, node, connection_entry_number);
        }
    }

    uint32_t second_audio_output_node_number = 0;
    uint32_t second_audio_output_node_sample_capabilities = 0;
    uint32_t second_audio_output_node_stream_format_capabilities = 0;
    uint32_t second_output_amp_node_number = 0;
    uint32_t second_output_amp_node_capabilities = 0;
    
    if (pin_alternative_output_node_number != 0) {
        printf("Found line out i guess lmao qemu thing\n");
        hda_init_output_pin(codec_number, pin_alternative_output_node_number);
    }
}

uint8_t hda_get_node_type(uint32_t codec, uint32_t node) {
    return ((hda_send_verb(codec, node, 0xF00, 0x09) >> 20) & 0xF);
}

uint16_t hda_get_node_connection_entry(uint32_t codec, uint32_t node, uint32_t cen) { // cen -> Connection entry number
    // Read connection capabilities
    uint32_t connection_list_capabilities = hda_send_verb(codec, node, 0xF00, 0x0E);

    // Test if the connection exists
    if (cen >= (connection_list_capabilities & 0x7F)) {
        printf("CEF no exist\n");
        return 0x0000;
    }

    // Return number of connected nodes
    if ((connection_list_capabilities & 0x80) == 0x00) {
        // Short form
        printf("CEF short form\n");
        return ((hda_send_verb(codec, node, 0xF02, ((cen / 4) * 4)) >> ((cen % 4) * 8)) & 0xFF);

    } else {
        // Long form
        printf("CEF long form\n");
        return ((hda_send_verb(codec, node, 0xF02, ((cen / 2) * 2)) >> ((cen % 2) * 16)) & 0xFFFF);

    }
}

void hda_init_output_pin(uint32_t codec_number, uint32_t pin_node_number) {
    printf("Initing pin %ld\n", pin_node_number);

    // Turn on power for pin
    hda_send_verb(codec_number, pin_node_number, 0x705, 0x00);

    // Disable unsolicited responses
    hda_send_verb(codec_number, pin_node_number, 0x708, 0x00);

    // Disable any processing
    hda_send_verb(codec_number, pin_node_number, 0x703, 0x00);

    // Enable pin
    hda_send_verb(codec_number, pin_node_number, 0x707, (hda_send_verb(codec_number, pin_node_number, 0xF07, 0x00) | 0x80 | 0x40));

    // Enable EAPD + L-R Swap
    hda_send_verb(codec_number, pin_node_number, 0x70C, 0x6);

    // Set max volume on pin
    uint32_t pin_output_amp_capabilities = hda_send_verb(codec_number, pin_node_number, 0xF00, 0x12);
    hda_set_node_gain(codec_number, pin_node_number, HDA_OUTPUT_NODE, pin_output_amp_capabilities, 100);

    uint32_t length_of_node_path = 0;
    hda_send_verb(codec_number, pin_node_number, 0x701, 0x00); // select first node
    uint32_t first_connected_node_number = hda_get_node_connection_entry(codec_number, pin_node_number, 0); // get first node number
    uint32_t type_of_first_connected_node = hda_get_node_type(codec_number, first_connected_node_number);   // get type of first node
    if (type_of_first_connected_node == HDA_WIDGET_AUDIO_OUTPUT) {
        printf("Connected node is audio output\n");
        hda_init_audio_output(codec_number, first_connected_node_number);
    } else {
        printf("Connected node is fucked\n");
    }
}

void hda_set_node_gain(uint32_t codec, uint32_t node, uint32_t node_type, uint32_t capabilities, uint32_t gain)
{
    // this will apply to left and right
    uint32_t payload = 0x3000;

    // set type of node
    if ((node_type & HDA_OUTPUT_NODE) == HDA_OUTPUT_NODE)
    {
        payload |= 0x8000;
    }
    if ((node_type & HDA_INPUT_NODE) == HDA_INPUT_NODE)
    {
        payload |= 0x4000;
    }

    // set number of gain
    if (gain == 0 && (capabilities & 0x80000000) == 0x80000000)
    {
        payload |= 0x80; // mute
    }
    else
    {
        payload |= (((capabilities >> 8) & 0x7F) * gain / 100); // recalculate range 0-100 to range of node steps
    }

    // change gain
    hda_send_verb(codec, node, 0x300, payload);
}

void hda_init_audio_output(uint32_t codec_number, uint32_t audio_output_node_number) {
    printf("Initalizing Audio Output %d", audio_output_node_number);
    uint32_t audio_output_node_number_global = audio_output_node_number;

    // turn on power for Audio Output
    hda_send_verb(codec_number, audio_output_node_number, 0x705, 0x00);

    // disable unsolicited responses
    hda_send_verb(codec_number, audio_output_node_number, 0x708, 0x00);

    // disable any processing
    hda_send_verb(codec_number, audio_output_node_number, 0x703, 0x00);

    // connect Audio Output to stream 1 channel 0
    hda_send_verb(codec_number, audio_output_node_number, 0x706, 0x10);

    // Set max audio output volume
    uint32_t audio_output_amp_capabilities = hda_send_verb(codec_number, audio_output_node_number, 0xF00, 0x12);
    hda_set_node_gain(codec_number, audio_output_node_number, HDA_OUTPUT_NODE, audio_output_amp_capabilities, 100);

    // read info, if something is not present, take it from AFG node
    uint32_t audio_output_sample_capabilities = hda_send_verb(codec_number, audio_output_node_number, 0xF00, 0x0A);
    if (audio_output_sample_capabilities == 0)
    {
        printf("fucka\n");
    }
    uint32_t audio_output_stream_format_capabilities = hda_send_verb(codec_number, audio_output_node_number, 0xF00, 0x0B);
    if (audio_output_stream_format_capabilities == 0)
    {
        printf("fuckb\n");
    }
}

void hda_play_pcm_data(uintptr_t addr) {
    // stop stream
    hda_mmio_out8(hda_output_stream_base + 0x00, 0x00);
    long ticks = 0;
    while (ticks < 2)
    {
        asm("nop");
        if ((hda_mmio_in8(hda_output_stream_base + 0x00) & 0x2) == 0x0)
        {
            printf("hda stream stopped\n");
            break;
        }
    }
    if ((hda_mmio_in8(hda_output_stream_base + 0x00) & 0x2) == 0x2)
    {
        printf("\nHDA: can not stop stream");
        return;
    }

    // reset stream registers
    hda_mmio_out8(hda_output_stream_base + 0x00, 0x01);
    ticks = 0;
    while (ticks < 10)
    {
        asm("nop");
        if ((hda_mmio_in8(hda_output_stream_base + 0x00) & 0x1) == 0x1)
        {
            break;
        }
    }
    if ((hda_mmio_in8(hda_output_stream_base + 0x00) & 0x1) == 0x0)
    {
        printf("\nHDA: can not start resetting stream");
    }

    for (size_t i = 0; i < 100000; i++) { }

    hda_mmio_out8(hda_output_stream_base + 0x00, 0x00);
    ticks = 0;
    while (ticks < 10)
    {
        asm("nop");
        if ((hda_mmio_in8(hda_output_stream_base + 0x00) & 0x1) == 0x0)
        {
            break;
        }
    }
    if ((hda_mmio_in8(hda_output_stream_base + 0x00) & 0x1) == 0x1)
    {
        printf("\nHDA: can not stop resetting stream");
        return;
    }

    for (size_t i = 0; i < 100000; i++) { }

    // clear error bits
    hda_mmio_out8(hda_output_stream_base + 0x03, 0x1C);

    // fill buffer entries - there have to be at least two entries in buffer, so we fill second entry with zeroes
    memset((uint32_t*)(hda_output_buffer_list + 0xFFFF800000000000), 0x00, 16 * 2);
    uint32_t* output_bfr_list_ptr = (uint32_t*)(hda_output_buffer_list + 0xFFFF800000000000);
    //output_bfr_list_ptr[0] = ((uint32_t)vmmgr_virt_to_phys_ext(0xd0005000));
    output_bfr_list_ptr[0] = ((uint32_t)addr);
    output_bfr_list_ptr[2] = (23391265 * 2); // from point of view of HDA card there is only one buffer, but thread will recognize first half of buffer as SOUND_BUFFER_0 and second half as SOUND_BUFFER_2
    asm("wbinvd");

    // set buffer registers
    hda_mmio_out32(hda_output_stream_base + 0x18, (uint32_t)hda_output_buffer_list);
    hda_mmio_out32(hda_output_stream_base + 0x08, (23391265 * 2));
    hda_mmio_out16(hda_output_stream_base + 0x0C, 1); // there are two entries in buffer

    // set stream data format
    hda_mmio_out32(hda_output_stream_base + 0x12, hda_return_sound_data_format(44100, 2, 16));

    // set Audio Output node data format
    hda_send_verb(0, 15, 0x200, hda_return_sound_data_format(44100, 2, 16));
    for (size_t i = 0; i < 100000; i++) { }

    // start streaming to stream 1
    hda_mmio_out8(hda_output_stream_base + 0x02, 0x14);
    hda_mmio_out8(hda_output_stream_base + 0x00, 0x02);

    //while(1==1){} // TODO : Audio is playing at 48000hz instead of 44100, and DONT FORGET LE CONVERSION
    while (true) {
        asm volatile("pause");
    }
    // TODO Note that it needs contiguous memory
    // Seems to play any buffer as long as i want, but NEEDS contiguous
    // but fuck this works amazingly. lovely to hear a wav file play
}

uint16_t hda_return_sound_data_format(uint32_t sample_rate, uint32_t channels, uint32_t bits_per_sample)
{
    uint16_t data_format = 0;

    // channels
    data_format = (channels - 1);

    // bits per sample
    if (bits_per_sample == 16)
    {
        data_format |= ((0b001) << 4);
    }
    else if (bits_per_sample == 20)
    {
        data_format |= ((0b010) << 4);
    }
    else if (bits_per_sample == 24)
    {
        data_format |= ((0b011) << 4);
    }
    else if (bits_per_sample == 32)
    {
        data_format |= ((0b100) << 4);
    }

    // sample rate
    if (sample_rate == 48000)
    {
        data_format |= ((0b0000000) << 8);
    }
    else if (sample_rate == 44100)
    {
        data_format |= ((0b1000000) << 8);
    }
    else if (sample_rate == 32000)
    {
        data_format |= ((0b0001010) << 8);
    }
    else if (sample_rate == 22050)
    {
        data_format |= ((0b1000001) << 8);
    }
    else if (sample_rate == 16000)
    {
        data_format |= ((0b0000010) << 8);
    }
    else if (sample_rate == 11025)
    {
        data_format |= ((0b1000011) << 8);
    }
    else if (sample_rate == 8000)
    {
        data_format |= ((0b0000101) << 8);
    }
    else if (sample_rate == 88200)
    {
        data_format |= ((0b1001000) << 8);
    }
    else if (sample_rate == 96000)
    {
        data_format |= ((0b0001000) << 8);
    }
    else if (sample_rate == 176400)
    {
        data_format |= ((0b1011000) << 8);
    }
    else if (sample_rate == 192000)
    {
        data_format |= ((0b0011000) << 8);
    }

    return data_format;
}

uint8_t hda_mmio_in8(uint64_t base) {
    return *(volatile uint8_t *) base;
}

void hda_mmio_out8(uint64_t base, uint8_t value) {
    *(volatile uint8_t *) base = value;
}

uint16_t hda_mmio_in16(uint64_t base) {
    return *(volatile uint16_t *) base;
}

void hda_mmio_out16(uint64_t base, uint16_t value) {
    *(volatile uint16_t *) base = value;
}

uint32_t hda_mmio_in32(uint64_t base) {
    return *(volatile uint32_t *) base;
}

void hda_mmio_out32(uint64_t base, uint32_t value) {
    *(volatile uint32_t *) base = value;
}
