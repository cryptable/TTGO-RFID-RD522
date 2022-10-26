Introduction
============

TTGo T-Beam has a more resticten IO-lines available and memory. Due to its architecture.

Setup of the project
--------------------

![Diagram](./TTGO-T-BEAM-RC522_bb.png)

Arduino-sketch
--------------

Using Arduino 2.0:

- Goto Files -> Preferences and fill into *Additional boards manager URLs*:
  ```
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  ```
- Goto *Tools -> Boards -> Board Manager...*. In the board manager search for ESP32 and install the latest version
- Goto *Tools -> Boards -> esp32*. Search and select T-Beam
- Goto *Tools -> Manage Libraries...*. Search MFRC522 library and install *MFRC522 by GithubCommunity*.

You can ofcourse upload the basic example, which reads out a RFID tag.

- Goto *Files -> Examples -> MFRC522* and select the DumpInfo() sketch

You need to adapt it to use the non-default SPI lines (SCK, MISO, MOSI, CS).

PlatformIO
----------

Using PlatformIO in VSCode. Start a new project using TTGO T-beam, Arduino platform.

- Goto Home of PlatformIO, New Project: 
  Name: TTGO-RC522
  Board: TTGO T-Beam
  Platform: Arduino
- Goto Home of PlatformIO, Select Libraries and search for MFRC522 
- Select the MFRC522
- Click the **Add to Project** button
- Select the project you want to add it to and press **OK**
- Copy/Past the DumpInfo example to test connectivity with RC522 board.

Now you can build it and test the RFID reader.

MicroPython
-----------

Install Micro Python on ESP32:

Follow the instrutcions from the MicroPython site as described on:
- https://docs.micropython.org/en/latest/esp32/tutorial/intro.html

You can just upload the micropython file to ESP-board using the build in terminal of PlatformIO.

Some links to help:
- https://docs.micropython.org/en/latest/esp32/quickref.html#webrepl-web-browser-interactive-prompt
- https://pythonforundergradengineers.com/how-to-install-micropython-on-an-esp32.html
- https://micropython.org/download/esp32/
- https://pythonforundergradengineers.com/upload-py-files-to-esp8266-running-micropython.html

In the platformIO-terminal, we will install ampy to run and upload python files to T-Beam board.

- https://www.digikey.be/en/maker/projects/micropython-basics-load-files-run-code/fb1fcedaf11e4547943abfdd8ad825ce

Do not open a serial terminal to your ESP32 when you use ampy, because it doesn't allow ampy to work.


To use the RC522:

- https://github.com/wendlers/micropython-mfrc522

Download the mfrc522.py and upload it to your T-Beam.

RC522-RFID-MQTT
---------------

### Setup a Mosquitto MQTT Server

For testing you need to setup a MQTT server, maybe a teacher or friend can help to set it up.

- MQTT broker installation: http://www.steves-internet-guide.com/install-mosquitto-linux/
- MQTT Set Username/Password http://www.steves-internet-guide.com/mqtt-username-password-example/
- MQTT python client http://www.steves-internet-guide.com/into-mqtt-python-client/
- MQTT client for ESP32 https://pubsubclient.knolleary.net/api

### Python based central server

Check out the code using git: [Python Server](https://github.com/cryptable/mqtt-rfid)

Change the settings in server.py to point to the IP address of the MQTT server. Fill in you student ID, so it scans for the messages for your student ID.

#### Security Tests

- Check what happens, when you use someone elses student ID

### RC522-RFID-MQTT project

The idea is that you RC522 RFID reader will send some information from the card to a central system. This central server uses a message broker (MQTT Server). The TTGO-RFID-MQTT will publish the information from the RFID reader to the message broker (MQTT-Server), where the server will read the messages, process them and return a response to the message broker. The TTGO-RFID will read its messages from the queue of the message broker to see which action it has to take.

![Architecture](./IoT-RC522-RFID-MQTT.png)

Checkout the code using git: [TTGO-RFID-MQTT](https://github.com/cryptable/TTGO-RFID-RD522)

1) Change the Wifi settings
1) Change the IP address to point the MQTT Server
1) Change the student ID to point to your student ID message queue
1) Fill in the username/password for MQTT Server

Check if all works...

#### Security Tests

- Check what happens, when you use someone elses student ID
- Use Wireshark or tcpdump on the MQTT server to check the messages received on the server

References:
-----------

- https://www.youtube.com/watch?v=fK4YQROD9Ps (overview of the board)
- https://www.youtube.com/watch?v=X63bKZInf_s (buttons)
- http://www.lilygo.cn/claprod_view.aspx?TypeId=62&Id=1281&FId=t28:62:28 (documentation)
- https://esp32io.com/tutorials/esp32-rfid-nfc
- https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/getting_started.html
- https://randomnerdtutorials.com/esp32-spi-communication-arduino/
- https://www.youtube.com/watch?v=ENMul9eAB00
- https://randomnerdtutorials.com/esp32-spi-communication-arduino/
- https://github.com/LilyGO/TTGO-T-Beam/issues/9
- https://www.hackster.io/news/the-ttgo-t-beam-an-esp32-lora-board-d44b08f18628

