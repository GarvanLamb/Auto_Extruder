
import serial
import time
import tkinter as tk
import serial.tools.list_ports
import warnings
class A:
    def __init__(self, master):
        # Code to get arduino com from computer
        arduino_ports = [
            p.device
            for p in serial.tools.list_ports.comports()
            if 'Arduino' in p.description  # may need tweaking to match new arduinos
        ]
        if not arduino_ports:
            raise IOError("No Arduino found")
        if len(arduino_ports) > 1:
            warnings.warn('Multiple Arduinos found - using the first')
        self.ser = serial.Serial(arduino_ports[0])
        #self.ser = serial.Serial('COM4', 9600)
        time.sleep(2)

        #Start of GUI Code
        self.master =master
        self.label_1 = tk.Label(self.master,text = 'Auto Extruder',pady=10,font = ('Arial','14'))
        self.label_1.grid(row=0, column=0,padx = 20,pady = 20)

        self.label_2 = tk.Label(self.master,font = ('Arial','12'), text='Temp (C) Enter 0 for Room Temp')
        self.label_2.grid(row=1, column=0,pady = 10)
        self.entry_1 = tk.Entry(self.master)
        self.entry_1.grid(row=1, column=1)

        self.label_3 = tk.Label(self.master, text='Velocity (mm/s)',font = ('Arial','12'), pady=10)
        self.label_3.grid(row=2, column=0,padx = 20,pady = 20)
        self.entry_2 = tk.Entry(self.master)
        self.entry_2.grid(row=2, column=1,padx = 20,pady = 20)

        self.label_5 = tk.Label(self.master, text='Cycles ', font=('Arial', '12'), pady=10)
        self.label_5.grid(row=3, column=0, padx=20, pady=20)
        self.entry_3 = tk.Entry(self.master)
        self.entry_3.grid(row=3, column=1, padx=20, pady=20)

        self.button_1 = tk.Button(self.master, text='Start',font = ('Arial','12'), command=self.on_start)
        self.button_1.grid(row=4, column=0,pady=10,padx=20)

        self.button_2 = tk.Button(self.master, text='Pause',font = ('Arial','12'), command=self.on_off)
        self.button_2.grid(row=4, column=1,pady=10,padx=20)

        self.button_3 = tk.Button(self.master, text='Reset', font=('Arial', '12'), command=self.on_reset)
        self.button_3.grid(row=5, column=0, pady=5, padx=20)

        self.button_4 = tk.Button(self.master, text='Resume', font=('Arial', '12'), command=self.on_resume)
        self.button_4.grid(row=5, column=1, pady=5, padx=20)

        #self.label_4 = tk.Label(self.master, text='Temp (C)', font=('Arial', '12'), pady=10)
        #self.label_4.grid(row=0, column=2, padx=20, pady=20)
        #self.text_1 = tk.Text(self.master,height = 10,width = 40)
        #self.text_1.grid(row =1,column = 2,pady =10)

        self.label_5 = tk.Label(self.master, text='Press reset before start when you first open app ', font=('Arial', '11'), pady=2)
        self.label_5.grid(row=6, columnspan=2, padx=20, pady=1)
        self.label_6 = tk.Label(self.master, text='Enter in data press start and wait 10 seconds', font=('Arial', '11'),pady=2)
        self.label_6.grid(row=7, columnspan=2, padx=20, pady=1)
        self.label_7 = tk.Label(self.master, text='The red led flashing means its heating up and is working', font=('Arial', '11'), pady=5)
        self.label_7.grid(row=8, columnspan=2, padx=20, pady=1)
        self.label_8 = tk.Label(self.master, text='If it does not run right away hit start again and wait  ',font=('Arial', '11'), pady=5)
        self.label_8.grid(row=9, columnspan=2, padx=20, pady=1)
        self.label_10 = tk.Label(self.master, text='Exit out and restart if problem persists', font=('Arial', '11'), pady=2)
        self.label_10.grid(row=10, columnspan=2, padx=20, pady=1)
        self.label_10 = tk.Label(self.master, text='Remove plug from PC for emergency stop', font=('Arial', '11'),pady=2)
        self.label_10.grid(row=10, columnspan=2, padx=20, pady=1)
        #self.text_2 = tk.Text(self.master, height=10, width=40)
        #self.text_2.grid(row =3, column = 2, pady = 10,padx = 20)

        self.resetPressed = False;
        #self.remaining = 10

    def on_start(self):
        Cycles = self.entry_3.get()
        Temp = self.entry_1.get()
        Speed = self.entry_2.get()
        Steps= 12800*(float(Speed)/25.4)
        Steps = int(Steps)
        Steps = str(Steps)
        self.RunMotor(Steps,Temp,Cycles)
    def on_off(self):
        self.ser.write(b'H')
    def on_resume(self):
        self.ser.write(b'L')
    def on_reset(self):
        self.resetPressed = True;
        Cycles = str(1)
        Choice = str(3)
        Speed = str(1200)
        Temp = str(0)
        temp = "<H, " + Cycles + ", " + Choice + ", " + Temp + ", " + Speed + ">"
        tempbyte = str.encode(temp)
        self.ser.write(tempbyte)
        time.sleep(1)
        print('done')
    def RunMotor(self,Speed,Temp,Cycles):

        if (self.resetPressed==True):
            Choice = str(2)
            temp = "<H, "+ Cycles +", "+Choice+", "+Temp+", "+Speed+">"
            tempbyte = str.encode(temp)
            print(tempbyte)

            self.ser.write(tempbyte)
            time.sleep(1)


       # while (self.ser.in_waiting < 0):
         #  print(self.ser.readline())

        #for i in range(int(Cycles)):
        #self.readOutput();



        #mylist = list()
        #for i in range(4):
        #    output = self.ser.readline()
        #    print(output)
        #    outputStr = output.decode()
        #    print(outputStr)
        #    mylist.append(outputStr)


        #print(mylist)
    def readOutput(self):

        while (self.ser.in_waiting>0):
            output = self.ser.readline()
            #print(output)
            outputStr = output.decode()
            #print(type("f"))
            print(outputStr)
            #self.update()
            self.master.after(500,self.readOutput())
            if (outputStr == "f\r\n"):
                print('hit f')
                return
            else:
                self.text_1.insert(tk.END, outputStr)
    def off(self):
        self.ser.close()


if __name__ == "__main__":
    root = tk.Tk()
    A(root)
    root.mainloop()