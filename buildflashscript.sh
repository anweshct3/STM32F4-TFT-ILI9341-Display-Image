echo "Exporing gcc-arm path"
export PATH="/opt/gcc-arm/bin:$PATH"
echo "Making libopencm3 library and the src directory files"
make
echo "Generating bin files"
arm-none-eabi-objcopy -Obinary src/colourlcdtest.elf src/colourlcdtest.bin
echo "Flashing bitmap image to the TFT LCD's flash memory - location 0x8020000"
st-flash write images/img1.bmp 0x8020000
echo "Flashing binary to the microcontroller"
st-flash write src/colourlcdtest.bin 0x8000000
