import kivy
kivy.require('1.11.1') # replace with your current kivy version !
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.app import App
from kivy.core.window import Window
from kivy.uix.popup import Popup
from kivy.uix.anchorlayout import AnchorLayout
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.widget import Widget
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.uix.progressbar import ProgressBar
from kivy.properties import ObjectProperty
from kivy.lang import Builder
from kivy.clock import Clock

import requests
import datetime
import os
import time



#global variable definitions
Num = 0
command = ""
errorHandle = 0
currFood = 0.0
currWater = 0.0
maxFood = .16 #.29 is the actual value
maxWater = .55

url = "http://10.0.0.48:80/body"
payload = ""

curr_Dir = os.getcwd()

kv = Builder.load_file("anim.kv")


#sets the command for what data will be communicated to esp8266
def setCommand(Num):
    if type(Num) is int:
        if(Num > 0 and Num < 3):
            switcher={           
                1: "FillFood",
                2: "FillWater"
            }
            return switcher[Num]
        else:
            return "NoCommand"
    elif type(Num) is list:
        if(len(Num) == 2):
            return "Quantities: " + str(Num)
        else:
            return "Parameters:" + str(Num)
    else:
            return str(Num)


#calls the set command function in order to set a command and concatenate with a string in order to communicate with esp8266
def comFun(action):
    command = setCommand(action)

    payload = "\r\t" + command + "\r"
    print(payload)

    headers = {
        'Content-Type': 'text/plain'
    }
   
    #try to obtain a response, if it cannot a popup will appear and the app will not crash
    try:    
        response = requests.request("POST", url, headers=headers, data = payload)
        ESPResponse = response.text.encode('utf8')
        print(ESPResponse)
        
        #if action is get time, convert the incoming weights into floats and determine the value of the progress bar by dividing by the max
        if(action == "get_Time"):
            global currFood 
            global currWater
            
            foodList = ESPResponse[:4]
            waterList = ESPResponse[5:9]
            
            currFood = float(foodList) / maxFood
            if(currFood > 1):
                currFood = 1
            currFood = currFood * 100  #used because progress bar is 0 - 100
            
            currWater = float(waterList) / maxWater
            if(currWater > 1):
                currWater = 1
            currWater = currWater * 100
            
    except:
        global errorHandle
        errorHandle = 1
        Alert(title = 'Error', text = 'Could Not Connect to Device')
    
    

#popup used for errors, if any
class Alert(Popup):
    def __init__(self, title, text):
        super(Alert, self).__init__()
        content = AnchorLayout(anchor_x='center', anchor_y='bottom')

        content.add_widget(
            Label(text=text, halign='left', valign='top')
        )
        ok_button = Button(background_color = (.149,.05,.05,.51), text='Go Back', size_hint=(.75, .1), pos_hint=(.125,.025))
        content.add_widget(ok_button)

        popup = Popup(
            title=title,
            separator_color = (.149,.05,.05,.51),
            background = 'popupbck.jpg',
            content=content,
            size_hint=(None, None),
            size=((2*Window.width) / 3, (2*Window.height) / 3),
            auto_dismiss=True,
        )
        ok_button.bind(on_press=popup.dismiss)
        popup.open()
        


#start of main classes for the app
class MainScreen(Screen):
    fillwater = ObjectProperty(None)
    fillfood = ObjectProperty(None)
    foodStatus = ObjectProperty(None)
    waterStatus = ObjectProperty(None)
    
    def on_pre_enter(self):
        time.sleep(.1)
        comFun("get_Time")
        self.foodStatus.value = currFood
        self.waterStatus.value = currWater
    
    #calls comFun to fill foodbowl immedietely
    def btnff(self):
        comFun(1)

     #calls comFun to fill waterbowl immedietely
    def btnfw(self):
        comFun(2)
    pass


class SettingsScreen(Screen):
    pass


#screen used to set the parameters for the animal feeder. The current times are written to a text file and can be found in the cwd
class ParameterScreen(Screen):
    food1 = ObjectProperty(None)
    food2 = ObjectProperty(None)
    food3 = ObjectProperty(None)
    food4 = ObjectProperty(None)
    water1 = ObjectProperty(None)
    water2 = ObjectProperty(None)
    water3 = ObjectProperty(None)
    water4 = ObjectProperty(None)
    importTime = ObjectProperty(None)
    
    #user input stripped down and converted to military time, get rid of semicolon(if any) and also ensure the number is not above 12 and does proper error checking
    def inputParameters(self):   
        #set the parameters to the input text then clear the text boxes
        params = [self.food1.text, self.food2.text, self.food3.text, self.food4.text, self.water1.text, self.water2.text, self.water3.text, self.water4.text]
        
        i = 0
        
        for parameters in params:
            if parameters == '':
                parameters = "0000AM"
            if parameters[-2:].lower() != 'am' and parameters[-2:].lower() !='pm':
                Alert(title = 'Error', text = 'Invalid Inputs')
                return False
            if len(parameters) < 5:
                Alert(title = 'Error', text = 'Invalid Inputs')   
                return False
            if ":" in parameters:
                parameters = parameters.replace(":","")
            print(parameters)
            #if pm number, convert to integer add 12 to it, also does error checking
            if parameters[-2:].lower() == 'pm':
                params[i] = parameters[:-2]
                if params[i].isnumeric():
                    if "12" in params[i]:
                        params[i] = int(params[i]) 
                    else:
                        params[i] = int(params[i]) + 1200
                else:    
                    Alert(title = 'Error', text = 'Invalid Inputs')   
                    return False
                if(params[i] > 2359):
                    Alert(title = 'Error', text = 'Invalid Inputs')   
                    return False
                if(int(parameters[2]) > 5):
                    Alert(title = 'Error', text = 'Invalid Inputs')   
                    return False
                
            #if am, convert to integer and do error checking    
            else:
                params[i] = parameters[:-2]
                if params[i].isnumeric():
                    if "12" in params[i]:
                        params[i] = int(params[i]) - 1200
                    else:
                        params[i] = int(params[i])
                else:
                    Alert(title = 'Error', text = 'Invalid Inputs')
                    return False
                if(params[i] > 1159):
                    Alert(title = 'Error', text = 'Invalid Inputs')   
                    return False
                if(int(parameters[2]) > 5):
                    Alert(title = 'Error', text = 'Invalid Inputs')   
                    return False  
            i+=1    
       
        global errorHandle
        comFun(params)
        if(errorHandle == 0):
            self.writeToFile(params)
            self.food1.text = ""
            self.food2.text = ""
            self.food3.text = ""
            self.food4.text = ""
            self.water1.text = ""
            self.water2.text = ""
            self.water3.text = ""
            self.water4.text = ""
        else:
            errorHandle = 0
            return

    #write the time parameters to a file in the cwd
    def writeToFile(self, params):
        global curr_Dir
        
        #open up a file parameters to write to, write parameters to it
        file = open("parameters.txt",'w')
        
        for parameters in params:
            file.write(str(parameters) + "\n")
        file.close()

class ContactScreen(Screen):
    pass

#see the current configuration of the programmed parameters
class CurrentConfigScreen(Screen):
    time1 = ObjectProperty(None)
    time2 = ObjectProperty(None)
    time3 = ObjectProperty(None)
    time4 = ObjectProperty(None)
    time5 = ObjectProperty(None)
    time6 = ObjectProperty(None)
    time7 = ObjectProperty(None)
    time8 = ObjectProperty(None)
            
    def on_pre_enter(self):
        time.sleep(.5)
        self.openScreen()

    def openScreen(self):
        if(os.path.isfile("parameters.txt")):
            with open("parameters.txt") as params:
                line = params.readline()
                count = 1
                
                while line:
                    print(line)
                    if(str(line) == "0\n"):
                        line = params.readline()
                        count+=1
                        continue
                    else:
                        if(count == 1):
                            self.time1.text = self.convertTime(str(line))
                            line = params.readline()
                            count+=1
                            continue
                        if(count == 2):
                            self.time2.text = self.convertTime(str(line))
                            line = params.readline()
                            count+=1
                            continue
                        if(count == 3):
                            self.time3.text = self.convertTime(str(line))
                            line = params.readline()
                            count+=1
                            continue
                        if(count == 4):
                            self.time4.text = self.convertTime(str(line))
                            line = params.readline()
                            count+=1
                            continue
                        if(count == 5):
                            self.time5.text = self.convertTime(str(line))
                            line = params.readline()
                            count+=1
                            continue
                        if(count == 6):
                            self.time6.text = self.convertTime(str(line))
                            line = params.readline()                            
                            count+=1
                            continue
                        if(count == 7):
                            self.time7.text = self.convertTime(str(line))
                            line = params.readline()
                            count+=1
                            continue
                        if(count == 8):
                            self.time8.text = self.convertTime(str(line))
                            line = params.readline()
                            count+=1
                            continue
        else:
            print("File Not Available")
            pass
        
    #convert military time to standard format time with AM or PM
    def convertTime(self, time):
        if(len(time) == 4):
            time1 = time[0:1]
            time2 = time[1:3]
        else:
            time1 = time[0:2]
            time2 = time[2:4]
        
        
        if(int(time1) == 12):
            return(time1 + ":" + time2 + " PM")
        elif(int(time1) > 12):
            time1 = str(int(time1) - 12)
            return(time1 + ":" + time2 + " PM")
        else:
            return(time1 + ":" + time2 + " AM")


#sends over the amount of food and water the user wants in their animals bowls
class AmountScreen(Screen):
    foodSlider = ObjectProperty(None)
    waterSlider = ObjectProperty(None)
    submit = ObjectProperty(None)
    
    def inputAmount(self):
        amount = [self.foodSlider.value, self.waterSlider.value]
        comFun(amount)
    
    pass


# Create the screen manager
sm = ScreenManager()
sm.add_widget(MainScreen(name='main'))
sm.add_widget(SettingsScreen(name='settings'))
sm.add_widget(ParameterScreen(name='parameter'))
sm.add_widget(ContactScreen(name='contact'))
sm.add_widget(CurrentConfigScreen(name = 'currentConfig'))
sm.add_widget(AmountScreen(name = 'amount'))


class ANIMApp(App):     
    def build(self):
        #find the current OS time and save it in string variables then send the time to the MCU
        currentDT = datetime.datetime.now()
        currMin = str(currentDT.minute)
        currHour = str(currentDT.hour)
        
        if(len(currHour) < 2 and len(currMin) < 2):
            comFun("Current Time(Hours): " + "0" + currHour + "0" + currMin)
        elif(len(currHour) < 2):  
            comFun("Current Time(Hours): " + "0" + currHour + currMin)
        elif(len(currMin) < 2):
            comFun("Current Time(Hours): " + currHour + "0" + currMin)
        else:
            comFun("Current Time(Hours): " + currHour + currMin)
        return sm
        

if __name__ == '__main__':
    ANIMApp().run()
