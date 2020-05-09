# ANIMIO
IOT Animal Feeder with Android App Control

Video Demonstration:
https://www.youtube.com/watch?v=lcOCRH56Vb8

The IOT Animal Feeder is currently a prototype targeted toward typical American Homes of families on the go. It includes a sleek designed housing. The components for the housing were all 3D printed. Electrical components are all development boards/modules, further pursuing of this project would be implementation of a full Printed Circuit Board. The firmware development on the MCU was done through Arduino. To tie everything together, an application was created using the Kivy cross-platform Python framework which gives the ability of Android and IOS users to access the application. The design process, assembly, and software/firmware explanation will be included within the detailed description.

## Design Process
The success of any project, regardless of scale, is usually decided during the design process. The design for the prototype was inspired for smaller animals, such as a domestic cat. When considering the actual product, different sizes may be possible, or a larger product to accommodate for larger animals. Shown below is the finalized prototype of the capstone project .

![](https://github.com/matt001k/ANIMIO/blob/master/Images/design2.jpg)



The system works by storing programmed time parameters of when the animal will be fed and watered throughout the day. With these parameters it will determine if the bowls need more food/water or is there is enough already within the bowl. Feedback to the controller, in order to determine the food and water contents, are done so by weight. When the weight is determined to be at a specific level(programmed again by the user), the system will stop dispensing food and water. All user interaction is interfaced through the application. There are *up to* four programmable times for feeding **and** watering the animal. The user also has the ability to feed/water the animal NOW. This is useful in case the animal should be fed without having to reprogram feed/water times and change settings around. 

## Mechanical Design
From a mechanical standpoint, the design was all done using the 3D CAD software **Fusion 360**. Further details of the more significant components will be explained. 

Discussion will begin on the base:

![](https://github.com/matt001k/ANIMIO/blob/master/Images/Base.PNG)

The base was designed to hold all of the electrical components. Each board's mounting holes were taken into consideration, so they may be mounted onto the base with appropriate hardware(self-tapping screws were used). There are through holes to support two load cells used for weight feedback of the bowls, these were secured using M5 bolts. On top of the load cells, are another 3D printed component to hold the bowls, this was secured onto the load cell using M4 Bolts. On the back of the base, a small compartment was used to hold a DC Male Jack Connector for incoming power to the system, this can be seen below.

![](https://github.com/matt001k/ANIMIO/blob/master/Images/IncomePow.PNG)

Next, will be on the food dispensing mechanism:

![](https://github.com/matt001k/ANIMIO/blob/master/Images/Food%20Dispensor%20Front.PNG)![](https://github.com/matt001k/ANIMIO/blob/master/Images/Food%20Dispensor.PNG)

This mechanism combines a lot of components. From the rear view, the motor, with shaft can be seen. The motor holder was designed to the motor's dimensions to hold it during operation, it was secured using M3 bolts. The shaft was designed to fit within the housing of the dispenser. This was all secured with epoxy. From the front view, the impeller, used to move the food, is visible.

![](https://github.com/matt001k/ANIMIO/blob/master/Images/Imp.PNG)

The water dispensing mechanism will now be explained:

![](https://github.com/matt001k/ANIMIO/blob/master/Images/Water%20Dispenser%20Front.PNG)![](https://github.com/matt001k/ANIMIO/blob/master/Images/Water%20Dispenser%20Rear.PNG)

As more clearly seen by the rear view, a solenoid valve is secured into place using the 1.4" NPT fitting design, embedded into a sloped surface. This valve will open, dependant on when the programmed times/user control, in order to fill the food bowl with water. Water will drain from the valve on the sloped surface designed below the valves holding point.

## Electrical Design
Next, the schematics and design explanation for all electrical connections and component. 

First, the components for power distribution are drafted. The CAD drawing for power distribution is seen below. Incoming power is distributed to the system by a 12 Volt wall plug adapter *(L6R36-120)*. This is then stepped down further using a buck converter *(COM15208)* to 5 Volts. The 12 Volts is used to operate a solenoid valve, the buck converted is used to power a motor, the *ROB14450*, a relay as well as the MCU. 3.3 Volt outputs from the MCU are used to power the *HX711* modules. 

![](https://github.com/matt001k/ANIMIO/blob/master/Images/cad1.png)

Now, designing the wire diagram of the unit's MCU. The controller of choice for this project is the *ESP8266* by Espressif Systems. This SOC is a WiFi enabled, 32bit RISC processor. The *ESP8266* was chosen due to it's flexibility well documented data that made learning the system easy, and its ability to provide enough processing power to be able to carryout all of the functionalities of the animal feeding system.

![](https://github.com/matt001k/ANIMIO/blob/master/Images/IO.PNG)


Next, how the load cells are connected. As can be seen the load cells are connected to the *HX711* modules. These ADC modules are used to amplify the signal from the load cell, convert it into a digital signal that can be read from the *ESP8266* in SPI *(Serial Peripheral Interface)*.

![](https://github.com/matt001k/ANIMIO/blob/master/Images/Load%20Cells.PNG)

The schematic of solenoid valve *(JFSV00065)*, which is used to allow water to flow into the water bowl, is shown below. The valve is opened when the relay *(VMA406)* is activated. When a signal is sent from the MCU to the relay, the Normally Open contact closes and allows 12 Volts to flow through the solenoid. 

![](https://github.com/matt001k/ANIMIO/blob/master/Images/Solenoid.PNG)

Finally, the how the electrical motor is wired and controlled. The driver module for the motor *(ROB14450)* utilizes a *TB6612FNG* IC that is used for DC motor control. This chip uses an H-Bridge configuration of MOSFET's to control current flow through the motor. Within the design, the motor is chosen to rotate in one direction, forward, to dispense the food. 

![](https://github.com/matt001k/ANIMIO/blob/master/Images/Motor.PNG)

## Firmware
The firmware was programmed through Arduino. The full code can be viewed at:

[GitHub](https://github.com/matt001k/ANIMIO)

## Application Design
The application will be described in this section page by page. The Python and Kivy code can be found at:

[GitHub](https://github.com/matt001k/ANIMIO)

First, the home page. The home page is consists of 2 main buttons, one to fill the water immediately and one to fill the food immediately. Below each button are the current food and water levels indicated by progress bars. A GET command is used to receive the current values of the load cells, this is used to update the progress bars. The other pages are accessible from the buttons at the bottom.

![](https://github.com/matt001k/ANIMIO/blob/master/Images/Main%20Screen.PNG)

The configuration page is shown next. This page has 4 text boxes on both the feeding times and drinking times, used to program times for the animal feeder to feed/water the animal. The user will input values, it does not have to be 4 for each but rather any combination up to 4 values, and then hit submit. These values are POSTed to the MCU, which stores the values into EEPROM to determine the times. 

![](https://github.com/matt001k/ANIMIO/blob/master/Images/Configure%20Screen.PNG)

Moving to the settings page, which consists of three main buttons. The *Current Configuration* button and *Configure Amounts* button will open up the current configuration screen and configure amounts screen respectively. The *Zero Scale* button will zero the load cells on the animal feeder. This button should only be pressed if there are no contents within the bowl as it will make whatever weight on the load cells the referenced zero. This is useful as moving the feeder to different locations causes a possibility of messing up the current zero, depending on the surface it is placed on.

![](https://github.com/matt001k/ANIMIO/blob/master/Images/Settings%20Screen.PNG)

To view the current configuration time of animal feeder, this screen is displayed. When the user inputs time parameters in the configuration page, a text file is written to store the current times. When the current configuration button is pressed, data is extracted from the text file, parsed and displayed on the application. This provides feedback to the user, just in case the programmed parameters are forgotten.

![](https://github.com/matt001k/ANIMIO/blob/master/Images/Current%20Configuration%20View.PNG)

Finally, the last page shown is the configure amount page. This page is used to fill up the respective bowl based upon the percentage the user wants the bowls to be filled. This data is sent to the MCU with the send button and then written to EEPROM to be saved.

![](https://github.com/matt001k/ANIMIO/blob/master/Images/Configure%20Weights%20View.PNG)
