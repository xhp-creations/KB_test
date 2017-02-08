#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <wiiu/vpad.h>
#include <wiiu/os.h>
#include <wiiu/nsyskbd.h>
#include <sys/socket.h>
#include "system/exception_handler.h"
#include "system/logger.h"
#include "system/wiiu_dbg.h"

static int pline = 0;

void flipBuffers()
{
	//Grab the buffer size for each screen (TV and gamepad)
	int buf0_size = OSScreenGetBufferSizeEx(0);
	int buf1_size = OSScreenGetBufferSizeEx(1);
	//Flush the cache
	DCFlushRange((void *)0xF4000000 + buf0_size, buf1_size);
	DCFlushRange((void *)0xF4000000, buf0_size);
	//Flip the buffer
	OSScreenFlipBuffersEx(0);
	OSScreenFlipBuffersEx(1);
}

void fillScreen(char r,char g,char b,char a)
{
	uint32_t num = (r << 24) | (g << 16) | (b << 8) | a;
	OSScreenClearBufferEx(0, num);
	OSScreenClearBufferEx(1, num);
}

void clearScreen()
{
	int ii;
	for( ii = 0; ii < 2; ii++ )
	{
		fillScreen(0, 0, 0, 0);
		flipBuffers();
	}
}

void screenInit()
{
    //Grab the buffer size for each screen (TV and gamepad)
    int buf0_size = OSScreenGetBufferSizeEx(SCREEN_TV);
    int buf1_size = OSScreenGetBufferSizeEx(SCREEN_DRC);

    OSScreenSetBufferEx(SCREEN_TV, (void *)0xF4000000);
    OSScreenSetBufferEx(SCREEN_DRC, ((void *)0xF4000000 + buf0_size));

    OSScreenEnableEx(SCREEN_TV, 1);
    OSScreenEnableEx(SCREEN_DRC, 1);
}

void plog(char* txt) { //print log line to screen
	if (pline==16) {
		for (int a = 0; a < 2; a++) {
			fillScreen(0,0,0,0);
			flipBuffers();
		}
		pline=0;
	}
    OSScreenPutFontEx(1, 0, pline, txt);
    flipBuffers();
    OSScreenPutFontEx(1, 0, pline, txt);
    flipBuffers();
	pline++;
}

void kb_connection_callback(KBDKeyEvent *key) {
	char buf3[255];
	__os_snprintf(buf3,255,"kb connected at channel %d",key->channel);
	plog(buf3);
}

void kb_disconnection_callback(KBDKeyEvent *key) {
	char buf4[255];
	__os_snprintf(buf4,255,"kb disconnected at channel %d",key->channel);
	plog(buf4);
}

void kb_key_callback(KBDKeyEvent *key) {
	if(key->state>0) {
		char buf5[255]; // ch = keyboard channel, state = key state, mod = key modifier, scode = scancode, char = printed char, uc = unicode value
		__os_snprintf(buf5,255,"ch:%d state:%d mod:%04x scode:%02x char: %lc uc:%04x",key->channel, key->state, key->modifier, key->scancode, key->UTF16, key->UTF16);
		plog(buf5);
	}
}

int main(int argc, char **argv) {
   setup_os_exceptions();
   socket_lib_init();
   
#if defined(LOGGER_IP) && defined(LOGGER_TCP_PORT)
   log_init(LOGGER_IP, LOGGER_TCP_PORT);
#endif

    VPADInit();
	OSScreenInit();
	screenInit();
	clearScreen();

	char kb_ret_value=KBDSetup(&kb_connection_callback,&kb_disconnection_callback,&kb_key_callback);

	char buf2[255];
	__os_snprintf(buf2,255,"KBDSetup called; ret: %d",kb_ret_value);
	plog(buf2);

	int error;
	VPADStatus vpad_data;
	while (1) {
		VPADRead(0, &vpad_data, 1, &error); //Get the status of the gamepad
		if (vpad_data.hold&VPAD_BUTTON_HOME) break; //To exit the game
	}

	char kbt_ret_value=KBDTeardown();
	char buf1[255];
	__os_snprintf(buf1,255,"KBDTeardown called; ret: %d",kbt_ret_value);
	plog(buf1);
    
	unsigned int t1 = 0x60000000;
	while(t1--) ;
    
	clearScreen();
    
#if defined(LOGGER_IP) && defined(LOGGER_TCP_PORT)
   log_deinit();
#endif
	return 0;
}

