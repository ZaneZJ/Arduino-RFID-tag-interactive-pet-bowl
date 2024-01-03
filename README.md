# Arduino RFID tag interactive pet bowl

In this project the RFID systems reader will read the unique ID number (UID) of the RFID tag, recognize the type of the RFID tag and display the information through the I2C LCD1602 display. In this project we used three Push button Switches, one for adding new cards, one for clearing the card register and a third one for opening and closing the top of the bowl. The Stepper motor is used to open the cover of the food bowl when the RFID reader recognizes the added RFID tag. All devices are connected between themselves with Jumper cables. In order for the system to work as a power source we use a 9V battery.

[![Arduino](https://img.youtube.com/vi/z8QucEJFlo8/0.jpg)](https://www.youtube.com/watch?v=z8QucEJFlo8)

## Setting up the system

#### Setting up The Freenove Control Board

Connect the control board to the computer with USB cable.

For the project we downloaded Arduino IDE which is an open-source electronics platform based on easy-to-use hardware and software. It's intended for anyone making interactive projects. Arduino IDE uses the C/C++ programming language.

#### Setting up RFID

Before writing code, we needed to import the library needed to make it easy to operate RFID modules. This code will read the unique ID number (UID) of the card, recognize the type of the card and display the information through the serial port.

#### Setting up Step Motor

The Stepper Motor rotates once at a certain angle, which is called a “step”. By controlling the number of rotational steps, we can then control the Stepper Motor’s rotation angle. By defining the time between two steps, we can control the Stepper Motor’s rotation speed. Which will allow the box to open to the desired point.

The step motor rotates a full turn, and then repeats this process in a reverse direction.

#### Setting up I2C LCD1602 display

Before writing code, we need to import the library needed. For printing characters, we need to set the coordinate of the printed character, that is, in which line and which column they would be displayed.

#### Setting up Push button Switch

Arduino IDE provides a function digitalRead(pin) to obtain the state of the port pin. The return value is HIGH or LOW, that is, high level or low level. We will use the pin of the control board to get the status of the push button switch. When the button is not pressed, low level V will be detected by the control board port, and high level V when the button is pressed. The role of the resistor here is to prevent the port from being set to output high level by accident. Without a resistor, the port could be connected directly to the cathode and cause a short circuit when the button is pressed. 

#### Setting up 9V battery cable

A 9V battery cable can connect a 9 V battery, which can supply power for the control board.

## Interaction

In order for the project to work we need a power supply that would provide 7V to 12V, that’s why we use a 9 V battery. As we use it for the first time, the I2C LCD1602 display will display text “Put the card to the scanner!” going in a loop on the top line of the I2C LCD1602 display.

In order to register the RFID tag we need to press the yellow button. And we’ll see the text “Register card:” appear on the I2C LCD1602 display.

For the RFID tag to be registered we need to place it on the RFID systems reader. When we place the RFID tag onto the RFID systems reader we’ll see the text “Recognised” appear on the I2C LCD1602 display.

If we press the yellow button again then we can add another RFID tag. In the code we set the max amount to 10 RFID tags that we can add to the system. There is a possibility to clear RFID tags added in the system, we can do it by pressing the red button. As we press the red button we’ll see the text “Cleared registered cards” going in a loop on the top line of the I2C LCD1602 display, so the previously added RFID tags will be deleted from the system.

If the RFID systems reader won’t recognise the RFID tag then the I2C LCD1602 display will display the text “Unknown”.

On the other hand if the RFID systems reader will recognize the RFID tag then the I2C LCD1602 display will display the text “Recognised”.

As the RFID systems reader recognised the RFID tag the cover of the food bowl will open with the help of Step Motor. The step motor is set to 128 (have to recalculate the value when we place it in the food box) steps one in 2ms. RFID systems reader reads the RFID tags constantly so as long as the RFID tag is close to the RFID systems reader the cover will stay open. Since the RFID systems reader has operating frequency only of 13.56MHz that means that the RFID tag has to be close all the time to the RFID systems reader so the box won’t close while the pet is still eating. In order to fix this issue we added the keep open duration, that would keep the cover open for 350 ticks that is approximately equal to 10 seconds after the last time when RFIDs systems reader was in contact with the recognised RFID tag. When the system reaches 350 ticks the Step Motor will close the cover of the food bowl.

If we press the blue button the cover of the food bowl will automatically open without manually placing the RFID tag on the RFID systems reader. The I2C LCD1602 display will display the text “Cover Opened”. This is useful if you want to fill the food bowl for your pet and it won’t automatically close the cover after 350 ticks.

As well as opening the cover of the food bowl with the blue button the same way we can close the cover pressing it once again. The I2C LCD1602 display will display the text “Cover Closed”.

