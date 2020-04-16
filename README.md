# ANIMIO
IOT Animal Feeder with Android App Control

Detailed description of your work including design with detailed schematic diagram and flow charts for software. (15 Points)
The IOT Animal Feeder is currently a prototype targeted toward typical American Homes of families on the go. It includes a sleekly designed housing. The components for the housing were all 3D printed. Electrical components are all development boards/modules, further pursuing of this project would be implementation of a full Printed Circuit Board. The firmware development on the MCU was done through Arduino. To tie everything together, an application was created using the Kivy cross-platform Python framework which gives the ability of Android and IOS users to access the application. The design process, assembly, and software/firmware explanation will be included within the detailed description.

## Design Process
The success of any project, regardless of scale, is usually decided during the design process. The design for the prototype was inspired for smaller animals, such as a domestic cat. When considering the actual product, different sizes may be possible, or a larger product to accommodate for larger animals. In Figure \ref{fig:design1}, the finalized concept of the capstone project is shown. 

![GitHub Logo](/images/design2.jpg)
Format: ![Alt Text]


The system works by storing programmed time parameters of when the animal will be fed and watered throughout the day. With these parameters it will determine if the bowls need more food/water or is there is enough already within the bowl. Feedback to the controller, in order to determine the food and water contents, are done so by weight. When the weight is determined to be at a specific level(programmed again by the user), the system will stop dispensing food and water. All user interaction is interfaced through the application. There are \textit{up to} four programmable times for feeding \textit{and} watering the animal. The user also has the ability to feed/water the animal NOW. This is useful in case the animal should be fed without having to reprogram feed/water times and change settings around. 

\subsection*{Mechanical Design}
From a mechanical standpoint, the design was all done using the 3D CAD software \textbf{Fusion 360}. Further details of the more significant components will be explained. 

Discussion will begin on the base:

\begin{figure}[H]
\centering
\includegraphics[scale=.3333]{Pictures/6_Det/base.PNG}
\caption{3D Base Model.}
\label{fig:base}
\end{figure}

The base was designed to hold all of the electrical components. Each board's mounting holes were taken into consideration, so they may be mounted onto the base with appropriate hardware(self-tapping screws were used). There are through holes to support two load cells used for weight feedback of the bowls, these were secured using M5 bolts. On top of the load cells, are another 3D printed component to hold the bowls, this was secured onto the load cell using M4 Bolts. On the back of the base, a small compartment was used to hold a DC Male Jack Connector for incoming power to the system, this can be seen in figure \ref{fig:jack}

\begin{figure}[H]
\centering
\includegraphics[scale=.333]{Pictures/6_Det/IncomePow.PNG}
\caption{DC Male Jack Holding Point.}
\label{fig:jack}
\end{figure}

Next, will be on the food dispensing mechanism:

\begin{figure}[H]
\centering
\begin{subfigure}{.5\textwidth}
  \centering
  \includegraphics[scale=.5]{Pictures/6_Det/Food Dispensor.PNG}
  \caption{Rear View}
  \label{fig:sub1}
\end{subfigure}%
\begin{subfigure}{.5\textwidth}
  \centering
  \includegraphics[scale=.7]{Pictures/6_Det/Food Dispensor Front.PNG}
  \caption{Front View}
  \label{fig:sub2}
\end{subfigure}
\caption{Food Dispensing Mechanism.}
\label{fig:food}
\end{figure}

This mechanism combines a lot of components. From the rear view, the motor, with shaft can be seen. The motor holder was designed to the motor's dimensions to hold it during operation, it was secured using M3 bolts. The shaft was designed to fit within the housing of the dispenser. From the front view, the impeller used to move the food is visible.

\begin{figure}[H]
\centering
\includegraphics[scale=.333]{Pictures/6_Det/Imp.PNG}
\caption{Impeller.}
\label{fig:Imp}
\end{figure}

The water dispensing mechanism will now be explained:

\begin{figure}[H]
\centering
\begin{subfigure}{.5\textwidth}
  \centering
  \includegraphics[scale=.5]{Pictures/6_Det/Water Dispenser Rear.PNG}
  \caption{Rear View}
  \label{fig:sub11}
\end{subfigure}%
\begin{subfigure}{.5\textwidth}
  \centering
  \includegraphics[scale=.5]{Pictures/6_Det/Water Dispenser Front.PNG}
    \caption{Front View}
  \label{fig:sub21}
\end{subfigure}
\caption{Water Dispensing Mechanism.}
\label{fig:water}
\end{figure}

As more clearly seen by the rear view, a solenoid valve is secured into place using the 1.4" NPT fitting design, embedded into a sloped surface. This valve will open, dependant on when the programmed times/user control, in order to fill the food bowl with water. Water will drain from the valve on the sloped surface designed below the valves holding point. 

\subsection*{Electrical Design}
Next, the schematics and design explanation for all electrical connections and component. 

First, the components for power distribution are drafted. The CAD drawing for power distribution is located in Figure \ref{fig:cad1}. Incoming power is distributed to the system by a 12 Volt wall plug adapter(\textit{L6R36-120}). This is then stepped down further using a buck converter(\textit{COM15208}) to 5 Volts. The 12 Volts is used to operate a solenoid valve, the buck converted is used to power a motor, the \textit{ROB14450}, a relay as well as the MCU. 3.3 Volt outputs from the MCU are used to power the \textit{HX711} modules. 

\begin{figure}[H]
\centering
\includegraphics[width=\textwidth]{Pictures/6_Det/cad1.png}
\caption{Power Distribution Schematic.}
\label{fig:cad1}
\end{figure}

\newpage
Now, designing the wire diagram of the unit's MCU. The controller of choice for this project is the \textit{ESP8266} by Espressif Systems. This SOC is a WiFi enabled, 32bit RISC processor. The \textit{ESP8266} was chosen due to it's flexibility well documented data that made learning the system easy, and its ability to provide enough processing power to be able to carryout all of the functionalities of the animal feeding system. The schematic of the micro-controller's IO configuration is located in Figure \ref{fig:cad2}.

\begin{figure}[H]
\centering
\includegraphics[width=\textwidth]{Pictures/6_Det/IO.PNG}
\caption{Microcontroller Wiring Diagram.}
\label{fig:cad2}
\end{figure}

\newpage
Next, how the load cells are connected. As can be seen the load cells are connected to the \textit{HX711} modules. These ADC modules are used to amplify the signal from the load cell into a digital signal that can be read from the \textit{ESP8266} in SPI(\textit{Serial Peripheral Interface}).

\begin{figure}[H]
\centering
\includegraphics[width=\textwidth]{Pictures/6_Det/Load Cells.PNG}
\caption{Loading Cells Wiring Diagram.}
\label{fig:cad3}
\end{figure}

\newpage
The schematic of solenoid valve(\textit{JFSV00065}), which is used to allow water to flow into the water bowl, is shown in Figure \ref{fig:cad4}. The valve is opened when the relay(\textit{VMA406}) is activated. When a signal is sent from the MCU to the relay, the contact closes and allows 12 Volts to flow through the solenoid. 

\begin{figure}[H]
\centering
\includegraphics[width=\textwidth]{Pictures/6_Det/Solenoid.PNG}
\caption{Solenoid Wiring Diagram.}
\label{fig:cad4}
\end{figure}

\newpage
Finally, the how the electrical motor is wired and controlled. The driver module for the motor(\textit{ROB14450}) utilizes a \textit{TB6612FNG} IC that is used for DC motor control. This chip uses an H-Bridge configuration of MOSFET's to control current flow through the motor. Within the design, the motor is chosen to rotate in one direction, forward, to dispense the food. 

\begin{figure}[H]
\centering
\includegraphics[width=\textwidth]{Pictures/6_Det/Motor.PNG}
\caption{Motor Wiring Diagram.}
\label{fig:cad5}
\end{figure}

\subsection*{Firmware}
The firmware was programmed through Arduino. The full code can be viewed at:

\begin{center}
\href{https://github.com/matt001k/ANIMIO}{ANIMIO GitHub Repository}
 \end{center}
 
\subsection*{Application Design}
The application will be described in this section page by page. The Python and Kivy code can be found at:
\begin{center}
\href{https://github.com/matt001k/ANIMIO}{ANIMIO GitHub Repository}
\end{center}

First, the home page. The home page is consists of 2 main buttons, one to fill the water immediately and one to fill the food immediately. Below each button are the current food and water levels indicated by progress bars. A GET command is used to receive the current values of the load cells, this is used to update the progress bars. The other pages are accessible from the buttons at the bottom.

\begin{figure}[H]
\centering
\includegraphics[scale = .8]{Pictures/6_Det/Main Screen.PNG}
\caption{Main Page.}
\label{fig:Main}
\end{figure}

The configuration page is shown next. This page has 4 text boxes on both the feeding times and drinking times, used to program times for the animal feeder to feed/water the animal. The user will input values, it does not have to be 4 for each but rather any combination up to 4 values, and then hit submit. These values are POSTed to the MCU, which stores the values into EEPROM to determine the times. 

\begin{figure}[H]
\centering
\includegraphics[scale = .8]{Pictures/6_Det/Configure Screen.PNG}
\caption{Configure Page.}
\label{fig:Config}
\end{figure}

\newpage
Moving to the settings page, which consists of three main buttons. The \textit{Current Configuration} button and \textit{Configure Amounts} button will open up the current configuration screen and configure amounts screen respectively. The \textit{Zero Scale} button will zero the load cells on the animal feeder. This button should only be pressed if there are no contents within the bowl as it will make whatever weight on the load cells the referenced zero. This is useful as moving the feeder to different locations causes a possibility of messing up the current zero, depending on the surface it is placed on.

\begin{figure}[H]
\centering
\includegraphics[scale = .8]{Pictures/6_Det/Settings Screen.PNG}
\caption{Settings Page.}
\label{fig:Settings}
\end{figure}

\newpage
To view the current configuration time of animal feeder, this screen is displayed. When the user inputs time parameters in the configuration page, a text file is written to store the current times. When the current configuration button is pressed, data is extracted from this file and displayed on the application. This provides feedback to the user, just in case the programmed parameters are forgotten.

\begin{figure}[H]
\centering
\includegraphics[scale = .8]{Pictures/6_Det/Current Configuration View.PNG}
\caption{Current Configuration Page.}
\label{fig:CurrConfig}
\end{figure}

\newpage
Finally, the last page shown is the configure amount page. This page is used to fill up the respective bowl based upon the percentage the user wants the bowls to be filled. This data is sent to the MCU with the send button and then written to EEPROM to be saved.

\begin{figure}[H]
\centering
\includegraphics[scale = .8]{Pictures/6_Det/Configure Weights View.PNG}
\caption{Configure Amount Page.}
\label{fig:ConfigWeights}
\end{figure}
