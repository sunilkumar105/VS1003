"# VS1003" 
This library is written for VS1003, one of the famous CODEC CHIP.

HOW TO USE Import Notes :
1. Place VS1003.h and VS1003.c file in include and src folder respectively.
2. Navigate to VS1003 and Change the GPIO pins definition according to your mapped GPIO in the program or cubemx.
3. I have used hspi2 and UART 2, UART is used for the debug purpose, incase you need print of the registers, you can use UART.
4. If you are using another SPI or UART, change it in the VS1003.c file.
5. In the main src file, include the library 
6. Use the functions below for the initialization
	vs1003_begin();
	vs1003_setVolume(0x00);
7. Use function vs1003_playChunk(pcm_data, sizeof(pcm_data)); for playing audio data.


How to get the byte array of the specific Audio data.
1. You can use Audacity[opensource]  application for converting the audio format to one of the supported format by VS103.
2. Use HXD software to convert the exported file into bytes.
3. Copy the byte in the buffer to be played into your program.
