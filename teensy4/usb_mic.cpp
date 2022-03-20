/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Arduino.h>
#include "usb_dev.h"
#include "usb_mic.h"
#include "debug/printf.h"
#include "AudioStream.h"

#ifdef USB_MIC_INTERFACE

// bool AudioInputUSBMic::update_responsibility;
// audio_block_t * AudioInputUSBMic::incoming_left;
// audio_block_t * AudioInputUSBMic::incoming_right;
// audio_block_t * AudioInputUSBMic::ready_left;
// audio_block_t * AudioInputUSBMic::ready_right;
// uint16_t AudioInputUSBMic::incoming_count;
// uint8_t AudioInputUSBMic::receive_flag;

struct usb_mic_features_struct AudioInputUSBMic::features = {0,0,FEATURE_MAX_VOLUME/2};

extern volatile uint8_t usb_high_speed;
// static void rx_event(transfer_t *t);
static void txmic_event(transfer_t *t);

// /*static*/ transfer_t rx_transfer __attribute__ ((used, aligned(32)));
// /*static*/ transfer_t sync_transfer __attribute__ ((used, aligned(32)));
/*static*/ transfer_t tx_transfer __attribute__ ((used, aligned(32)));
// DMAMEM static uint8_t rx_buffer[AUDIO_RX_SIZE] __attribute__ ((aligned(32)));
DMAMEM static uint8_t tx_buffer[AUDIO_TX_SIZE] __attribute__ ((aligned(32)));
// DMAMEM uint32_t usb_mic_sync_feedback __attribute__ ((aligned(32)));

uint8_t usb_mic_receive_setting=0;
uint8_t usb_mic_transmit_setting=0;
// uint8_t usb_mic_sync_nbytes;
// uint8_t usb_mic_sync_rshift;

// uint32_t feedback_accumulator; //samplerate adujusted to get a continuous asynchonous data flow

// volatile uint32_t usb_mic_underrun_count;
// volatile uint32_t usb_mic_overrun_count;


// static void rx_event(transfer_t *t)
// {
// 	if (t) {
// 		int len = AUDIO_RX_SIZE - ((rx_transfer.status >> 16) & 0x7FFF);
// 		printf("rx %u\n", len);
// 		usb_mic_receive_callback(len);
// 	}
// 	usb_prepare_transfer(&rx_transfer, rx_buffer, AUDIO_RX_SIZE, 0);
// 	arm_dcache_delete(&rx_buffer, AUDIO_RX_SIZE);
// 	usb_receive(AUDIO_RX_ENDPOINT, &rx_transfer);
// }

// static void sync_event(transfer_t *t)
// {
// 	// USB 2.0 Specification, 5.12.4.2 Feedback, pages 73-75
// 	//printf("sync %x\n", sync_transfer.status); // too slow, can't print this much
// 	// usb_mic_sync_feedback = feedback_accumulator >> usb_mic_sync_rshift;
// 	// usb_prepare_transfer(&sync_transfer, &usb_mic_sync_feedback, usb_mic_sync_nbytes, 0);
// 	// arm_dcache_flush(&usb_mic_sync_feedback, usb_mic_sync_nbytes);
// 	// usb_transmit(AUDIO_SYNC_ENDPOINT, &sync_transfer);
// }

void usb_mic_configure(void)
{
	// printf("usb_mic_configure\n");
	// usb_mic_underrun_count = 0;
	// usb_mic_overrun_count = 0;
	// feedback_accumulator = (uint32_t) ((AUDIO_SAMPLE_RATE_EXACT / 1000.0f) * (1<<24)); // 44.1 * 2^24
	// if (usb_high_speed) {
	// 	// usb_mic_sync_nbytes = 4; //USB's asynchronous rate feedback is 3 bytes at full speed and 4 bytes at high speed. https://forum.pjrc.com/threads/60557-48kHz-8i80-USB-Audio
	// 	usb_mic_sync_rshift = 8;
	// } else {
	// 	// usb_mic_sync_nbytes = 3;
	// 	usb_mic_sync_rshift = 10;
	// }
	// memset(&rx_transfer, 0, sizeof(rx_transfer));
	// usb_config_rx_iso(AUDIO_RX_ENDPOINT, AUDIO_RX_SIZE, 1, rx_event);
	// rx_event(NULL);
	// memset(&sync_transfer, 0, sizeof(sync_transfer));
	// usb_config_tx_iso(AUDIO_SYNC_ENDPOINT, usb_mic_sync_nbytes, 1, sync_event);
	// sync_event(NULL);
	memset(&tx_transfer, 0, sizeof(tx_transfer));
	usb_config_mic_iso(AUDIO_TX_ENDPOINT, AUDIO_TX_SIZE, 1, txmic_event);
	txmic_event(NULL);
}

// void AudioInputUSBMic::begin(void)
// {
// 	incoming_count = 0;
// 	incoming_left = NULL;
// 	incoming_right = NULL;
// 	ready_left = NULL;
// 	ready_right = NULL;
// 	receive_flag = 0;
// 	update_responsibility = update_setup();
// 	TODO: update responsibility is tough, partly because the USB
// 	interrupts aren't sychronous to the audio library block size,
// 	but also because the PC may stop transmitting data, which
// 	means we no longer get receive callbacks from usb.c
// 	update_responsibility = false;
// }

// static void copy_to_buffers(const uint32_t *src, int16_t *left, int16_t *right, unsigned int len)
// {
// 	uint32_t *target = (uint32_t*) src + len; 
// 	while ((src < target) && (((uintptr_t) left & 0x02) != 0)) {
// 		uint32_t n = *src++;
// 		*left++ = n & 0xFFFF;
// 		*right++ = n >> 16;
// 	}

// 	while ((src < target - 2)) {
// 		uint32_t n1 = *src++;
// 		uint32_t n = *src++;
// 		*(uint32_t *)left = (n1 & 0xFFFF) | ((n & 0xFFFF) << 16);
// 		left+=2;
// 		*(uint32_t *)right = (n1 >> 16) | ((n & 0xFFFF0000)) ;
// 		right+=2;
// 	}

// 	while ((src < target)) {
// 		uint32_t n = *src++;
// 		*left++ = n & 0xFFFF;
// 		*right++ = n >> 16;
// 	}
// }

// Called from the USB interrupt when an isochronous packet arrives
// we must completely remove it from the receive buffer before returning
//
// #if 1
// void usb_mic_receive_callback(unsigned int len)
// {
// 	unsigned int count, avail;
// 	audio_block_t *left, *right;
// 	const uint32_t *data;

// 	AudioInputUSBMic::receive_flag = 1;
// 	len >>= 2; // 1 sample = 4 bytes: 2 left, 2 right
// 	data = (const uint32_t *)rx_buffer;

// 	count = AudioInputUSBMic::incoming_count;
// 	left = AudioInputUSBMic::incoming_left;
// 	right = AudioInputUSBMic::incoming_right;
// 	if (left == NULL) {
// 		left = AudioStream::allocate();
// 		if (left == NULL) return;
// 		AudioInputUSBMic::incoming_left = left;
// 	}
// 	if (right == NULL) {
// 		right = AudioStream::allocate();
// 		if (right == NULL) return;
// 		AudioInputUSBMic::incoming_right = right;
// 	}
// 	while (len > 0) {
// 		avail = AUDIO_BLOCK_SAMPLES - count;
// 		if (len < avail) {
// 			copy_to_buffers(data, left->data + count, right->data + count, len);
// 			AudioInputUSBMic::incoming_count = count + len;
// 			return;
// 		} else if (avail > 0) {
// 			copy_to_buffers(data, left->data + count, right->data + count, avail);
// 			data += avail;
// 			len -= avail;
// 			if (AudioInputUSBMic::ready_left || AudioInputUSBMic::ready_right) {
// 				// buffer overrun, PC sending too fast
// 				AudioInputUSBMic::incoming_count = count + avail;
// 				if (len > 0) {
// 					usb_mic_overrun_count++;
// 					printf("!");
// 					//serial_phex(len);
// 				}
// 				return;
// 			}
// 			send:
// 			AudioInputUSBMic::ready_left = left;
// 			AudioInputUSBMic::ready_right = right;
// 			//if (AudioInputUSBMic::update_responsibility) AudioStream::update_all();
// 			left = AudioStream::allocate();
// 			if (left == NULL) {
// 				AudioInputUSBMic::incoming_left = NULL;
// 				AudioInputUSBMic::incoming_right = NULL;
// 				AudioInputUSBMic::incoming_count = 0;
// 				return;
// 			}
// 			right = AudioStream::allocate();
// 			if (right == NULL) {
// 				AudioStream::release(left);
// 				AudioInputUSBMic::incoming_left = NULL;
// 				AudioInputUSBMic::incoming_right = NULL;
// 				AudioInputUSBMic::incoming_count = 0;
// 				return;
// 			}
// 			AudioInputUSBMic::incoming_left = left;
// 			AudioInputUSBMic::incoming_right = right;
// 			count = 0;
// 		} else {
// 			if (AudioInputUSBMic::ready_left || AudioInputUSBMic::ready_right) return;
// 			goto send; // recover from buffer overrun
// 		}
// 	}
// 	AudioInputUSBMic::incoming_count = count;
// }
// #endif

// void AudioInputUSBMic::update(void)
// {
// 	audio_block_t *left, *right;

// 	__disable_irq();
// 	left = ready_left;
// 	ready_left = NULL;
// 	right = ready_right;
// 	ready_right = NULL;
// 	uint16_t c = incoming_count;
// 	uint8_t f = receive_flag;
// 	receive_flag = 0;
// 	__enable_irq();
// 	if (f) {
// 		int diff = AUDIO_BLOCK_SAMPLES/2 - (int)c;
// 		feedback_accumulator += diff * 1;
// 		//uint32_t feedback = (feedback_accumulator >> 8) + diff * 100;
// 		//usb_mic_sync_feedback = feedback;

// 		//printf(diff >= 0 ? "." : "^");
// 	}
// 	//serial_phex(c);
// 	//serial_print(".");
// 	if (!left || !right) {
// 		usb_mic_underrun_count++;
// 		//printf("#"); // buffer underrun - PC sending too slow
// 		if (f) feedback_accumulator += 3500; //check for another method https://forum.pjrc.com/threads/61142-USB-Audio-Frame-Sync-on-Teensy-4-0
// 	}
// 	if (left) {
// 		transmit(left, 0);
// 		release(left);
// 	}
// 	if (right) {
// 		transmit(right, 1);
// 		release(right);
// 	}
// }



#if 1
bool AudioOutputUSBMic::update_responsibility;
audio_block_t * AudioOutputUSBMic::left_1st;
audio_block_t * AudioOutputUSBMic::left_2nd;
// audio_block_t * AudioOutputUSBMic::right_1st;
// audio_block_t * AudioOutputUSBMic::right_2nd;
uint16_t AudioOutputUSBMic::offset_1st;

/*DMAMEM*/ uint16_t usb_mic_transmit_buffer[AUDIO_TX_SIZE/2] __attribute__ ((used, aligned(32))); //TODO why devided by 2


static void txmic_event(transfer_t *t)
{
	serial_print("t");
	int len = usb_mic_transmit_callback();
	// usb_mic_sync_feedback = feedback_accumulator >> usb_mic_sync_rshift;
	usb_prepare_transfer(&tx_transfer, usb_mic_transmit_buffer, len, 0);
	arm_dcache_flush_delete(usb_mic_transmit_buffer, len);
	usb_transmit_mic(AUDIO_TX_ENDPOINT, &tx_transfer);
}


void AudioOutputUSBMic::begin(void)
{
	update_responsibility = false;
	left_1st = NULL;
	// right_1st = NULL;
}

static void copy_from_buffers(uint32_t *dst, int16_t *left, unsigned int len)
{
	// TODO: optimize...
	// int i = 0;

	// 	while (len > 0) {
	// 		if (i==0) {
	// 			*dst = ((*left++ << 16) & 0xFFFF0000);
	// 			i++;
	// 		}
	// 		else {
	// 			*dst++ = (*left++  & 0xFFFF);
	// 			i = 0;
	// 		}
	// 		len--;
	// 	}
		// 	int16_t temp_cfb=0;
		// 	while (len > 0) {
		// 	temp_cfb = *left++;
		// 	*dst++ =  (temp_cfb << 16) | ( temp_cfb & 0xFFFF);
		// 	len--;
		// }
		// 	int16_t temp_cfb=0;
		// 	while (len > 0) {
		// 	temp_cfb = *left++;
		// 	*dst++ =  ( temp_cfb & 0xFFFF);
		// 	len--;
		// }

		// 	while (len > 0) {
		// 	*dst++ =  ( *left++ & 0xFFFF);
		// 	len--;
		// }

		//
		int16_t temp1 = 0;
		int16_t temp2 = 0;

		while (len > 0) {
			temp1 = *left++;
			temp2 = *left++;
			*dst++ = (temp1 << 16) | ( temp2 & 0xFFFF);
			len--;len--;
		}
}

void AudioOutputUSBMic::update(void)
{

	audio_block_t *left;
	// audio_block_t *left, *right;

	// TODO: we shouldn't be writing to these......
	//left = receiveReadOnly(0); // input 0 = left channel
	//right = receiveReadOnly(1); // input 1 = right channel
	left = receiveWritable(0); // input 0 = left channel
	// right = receiveWritable(1); // input 1 = right channel

	if (usb_mic_transmit_setting == 0) {
		// serial_print("3");
		if (left) release(left);
		// if (right) release(right);
		if (left_1st) { release(left_1st); left_1st = NULL; }
		if (left_2nd) { release(left_2nd); left_2nd = NULL; }
		// if (right_1st) { release(right_1st); right_1st = NULL; }
		// if (right_2nd) { release(right_2nd); right_2nd = NULL; }
		offset_1st = 0;
		return;
	}

	if (left == NULL) {
		left = allocate();
		// if (left == NULL) {
		// 	if (right) release(right);
		// 	return;
		// }
		memset(left->data, 0, sizeof(left->data));
		// serial_print("4");
	}
	// if (right == NULL) {
	// 	right = allocate();
	// 	if (right == NULL) {
	// 		release(left);
	// 		return;
	// 	}
	// 	memset(right->data, 0, sizeof(right->data));
	// }
	__disable_irq();
	if (left_1st == NULL) {
		left_1st = left;
		// serial_print("5");
		// right_1st = right;
		offset_1st = 0;
	} else if (left_2nd == NULL) {
		left_2nd = left;
		// right_2nd = right;
	} else {
		// buffer overrun - PC is consuming too slowly
		serial_print("z");
		audio_block_t *discard1 = left_1st;
		left_1st = left_2nd;
		left_2nd = left;
		// audio_block_t *discard2 = right_1st;
		// right_1st = right_2nd;
		// right_2nd = right;
		offset_1st = 0; // TODO: discard part of this data?
		//serial_print("*");
		release(discard1);
		// release(discard2);
	}
	serial_print("1");
	__enable_irq();
}


// Called from the USB interrupt when ready to transmit another
// isochronous packet.  If we place data into the transmit buffer,
// the return is the number of bytes.  Otherwise, return 0 means
// no data to transmit
unsigned int usb_mic_transmit_callback(void)
{
		serial_print("2");
	uint32_t avail, num, target, offset, len=0;
		audio_block_t *left;
		// audio_block_t *left, *right;
	const int ctarget = ((int)(AUDIO_SAMPLE_RATE_EXACT)) / 1000;

    if ((int)(AUDIO_SAMPLE_RATE_EXACT) == 44100 ||
	    (int)(AUDIO_SAMPLE_RATE_EXACT) == 88200 || //TODO : don't think this is working for 88.2 or 176.4
	    (int)(AUDIO_SAMPLE_RATE_EXACT) == 176400) 
	{
		static uint32_t count = 0;

		if (++count < 10) //allow to get the 0.1kHz (for 44.1kHz)
		{ 
			target = ctarget;
		} 
		else 
		{
			target = ctarget + 1;
			count = 0;
		}
	} else target = ctarget;

	while (len < target) {

		num = target - len;
		left = AudioOutputUSBMic::left_1st;

		if (left == NULL) {
			// buffer underrun - PC is consuming too quickly
			memset(usb_mic_transmit_buffer + (len), 0, num * 2);
			serial_print("%");
			break;
		}
		//digitalWrite(13,HIGH); no
		// right = AudioOutputUSBMic::right_1st;
		offset = AudioOutputUSBMic::offset_1st;

		avail = AUDIO_BLOCK_SAMPLES - offset;
		if (num > avail) num = avail;
 //digitalWrite(13,HIGH);
 	// Serial1.write(num);
	//  serial_print(".");
		copy_from_buffers((uint32_t *)usb_mic_transmit_buffer + (len),left->data + offset, num);
		// copy_from_buffers((uint32_t *)usb_mic_transmit_buffer + len,left->data + offset, right->data + offset, num);
		len += num;

	  if ( len % 2 == 0)
    serial_print("e");
  else
    serial_print("d");

		offset += num;
		if (offset >= AUDIO_BLOCK_SAMPLES) {
			AudioStream::release(left);
			// AudioStream::release(right);
			AudioOutputUSBMic::left_1st = AudioOutputUSBMic::left_2nd;
			AudioOutputUSBMic::left_2nd = NULL;
			// AudioOutputUSBMic::right_1st = AudioOutputUSBMic::right_2nd;
			// AudioOutputUSBMic::right_2nd = NULL;
			AudioOutputUSBMic::offset_1st = 0;
		} else {
			AudioOutputUSBMic::offset_1st = offset;
		}
	}
		// serial_print("5");
	return target * 2;
}
#endif




struct setup_struct {
  union {
    struct {
	uint8_t bmRequestType;
	uint8_t bRequest;
	union {
		struct {
			uint8_t bChannel;  // 0=main, 1=left, 2=right
			uint8_t bCS;       // Control Selector
		};
		uint16_t wValue;
	};
	union {
		struct {
			uint8_t bIfEp;     // type of entity
			uint8_t bEntityId; // UnitID, TerminalID, etc.
		};
		uint16_t wIndex;
	};
	uint16_t wLength;
    };
  };
};

int usb_mic_get_feature(void *stp, uint8_t *data, uint32_t *datalen)
{
	struct setup_struct setup = *((struct setup_struct *)stp);
	if (setup.bmRequestType==0xA1) { // should check bRequest, bChannel, and UnitID
			if (setup.bCS==0x01) { // mute
				data[0] = AudioInputUSBMic::features.mute;  // 1=mute, 0=unmute
				*datalen = 1;
				return 1;
			}
			// else if (setup.bCS==0x02) { // volume
			// 	if (setup.bRequest==0x81) { // GET_CURR
			// 		data[0] = AudioInputUSBMic::features.volume & 0xFF;
			// 		data[1] = (AudioInputUSBMic::features.volume>>8) & 0xFF;
			// 	}
			// 	else if (setup.bRequest==0x82) { // GET_MIN
			// 		//serial_print("vol get_min\n");
			// 		data[0] = 0;     // min level is 0
			// 		data[1] = 0;
			// 	}
			// 	else if (setup.bRequest==0x83) { // GET_MAX
			// 		data[0] = FEATURE_MAX_VOLUME;  // max level, for range of 0 to MAX
			// 		data[1] = 0;
			// 	}
			// 	else if (setup.bRequest==0x84) { // GET_RES
			// 		data[0] = 1; // increment vol by by 1
			// 		data[1] = 0;
			// 	}
			// 	else { // pass over SET_MEM, etc.
			// 		return 0;
			// 	}
			// 	*datalen = 2;
			// 	return 1;
			// }
	}
	return 0;
}

int usb_mic_set_feature(void *stp, uint8_t *buf) 
{
	struct setup_struct setup = *((struct setup_struct *)stp);
	if (setup.bmRequestType==0x21) { // should check bRequest, bChannel and UnitID
			if (setup.bCS==0x01) { // mute
				if (setup.bRequest==0x01) { // SET_CUR
					AudioInputUSBMic::features.mute = buf[0]; // 1=mute,0=unmute
					AudioInputUSBMic::features.change = 1;
					return 1;
				}
			}
			// else if (setup.bCS==0x02) { // volume
			// 	if (setup.bRequest==0x01) { // SET_CUR
			// 		AudioInputUSBMic::features.volume = buf[0];
			// 		AudioInputUSBMic::features.change = 1;
			// 		return 1;
			// 	}
			// }
	}
	return 0;
}


#endif // USB_MIC_INTERFACE
