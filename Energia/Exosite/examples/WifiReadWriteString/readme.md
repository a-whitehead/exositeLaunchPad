Exosite LaunchPad + CC3000 WiFi BoosterPack Quick Start
=======================================================

1. Download the Energia toolchain and development environment

    1. http://energia.nu/download/

2. Copy the Exosite library folder to the directory "...\hardware\msp430\libraries\". You should then see "File->Examples->Exosite"

3. (Restart Energia if already running, and) Open "File->Examples->Exosite->WifiReadWriteString"

4. Sign up for a free account at https://ti.exosite.com.

5. Go to the Devices page (https://ti.exosite.com/manage/devices) and add a device

	1. Click on "+ Add Device"

	2. Select "I want to create a generic device."

	3. Leave Device Type as generic, enter your timezone, and optionally add a location. You may also limit the resources your device may use.

	4. Name your device.

6. Click on the device you just created to open the Device Information window.

7. Add the fourteen datasources "adc0", "adc1", "adc2", "adc3", "adc4", "p36", "p37", "p38", "p39", "p40", "led1_ctrl", "led2_ctrl", "sw1", and "sw2". (For Each)

	1. Click "+ Add Data"

	2. Give your device a human readable name (eg. "Analog-to-Digital 2"), set it to format "integer", and set the corresponding alias (eg. "adc2").

8. Copy your new device's 40 character CIK from the Device Information window and paste it into line 50 of the example opened in step 4.

9. Edit the SSID and security key on lines 51 and 42 to match your wireless network.

10. Connect your LaunchPad to your computer using a USB cable.

11. Go to "Tools->Serial" to select the serial port your LaunchPad board is connected to

12. Go to "Tools->Board" to select the corresponding LaunchPad board type
 
13. In the Energia software, compile and verify there are no errors

14. Upload the program to your board.

15. When "Done uploading" is displayed, go to https://ti.exosite.com to see your data in the cloud!

For more information on this project and other examples, checkout the Exosite Garage github page at http://exosite-garage.github.com