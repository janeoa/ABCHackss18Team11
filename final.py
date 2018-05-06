import serial, os, time, random, math
from Tkinter import *
from threading import Thread
import subprocess
import tkMessageBox

port = "/dev/"+os.popen("dmesg | egrep ttyACM | cut -f3 -d: | tail -n1").read().strip()
try:
	ser = serial.Serial(port,9600,timeout=1)
except:
	print("Cannoct find USB device");
	sys.exit(0)
print("ready!")

root = Tk()
root.title('Auth @ team 11')
root.wm_state('iconic')
root.iconify()
tkMessageBox.showinfo("ready!", "program is running in the background")

fRow = Frame(root, height=50, width=300)
fRow.pack_propagate(0) # don't shrink
fRow.pack()

sRow = Frame(root, height=50, width=300)
sRow.pack_propagate(0) # don't shrink
sRow.pack()

tRow = Frame(root, height=50, width=300)
tRow.pack_propagate(0) # don't shrink
tRow.pack()

pinLabel = Label(fRow, text="pincode")
pinLabel.pack(fill=BOTH, expand=1, side=LEFT)
uidLabel = Label(sRow, text="UID")
uidLabel.pack(fill=BOTH, expand=1, side=LEFT)
uidLabel = Label(tRow, text="Bluetooth\npassword")
uidLabel.pack(fill=BOTH, expand=1, side=LEFT)

bPV = StringVar()
bluePassEntry = Entry(tRow, width=4, textvariable=bPV, show="*")
bluePassEntry.pack(fill=X, expand=1, side=LEFT)
bluePassEntry.delete(0,4)
bPV.set("0000")

def changePin():
    ser.write(str(2)+"\r\n");
def changeUid():
    ser.write(str(1)+"\r\n");
def changeBlu():
    print("click!")
    bluetoothPassword = bPV.get()
    #bluetoothPassword = bluetoothPassword[0:4]
    print(bluetoothPassword)
    try:
	math.isnan(int(bluetoothPassword))
	if(not len(bluetoothPassword)==4):
		tkMessageBox.showerror("Error","Bluetooth password must be 4 digits")
    except:
        tkMessageBox.showerror("Error","Bluetooth password must be a NUMBER")
    #ser.write(str(bluetoothPassword)+"\r\n");
def on_closing():
    ser.write(str(3)+"\r\n")
    print("toExit")
    root.destroy()

pinButton = Button(fRow, text="change", command=changePin)
pinButton.pack(fill=BOTH, expand=1, side=LEFT)
uidButton = Button(sRow, text="change", command=changeUid)
uidButton.pack(fill=BOTH, expand=1, side=LEFT)
bluButton = Button(tRow, text="change\n(Alpha)", command=changeBlu)
bluButton.pack(fill=BOTH, expand=1, side=LEFT)

authors = Label(root, text="Yernar & Asset\nABC Hack 'SS 18")
authors.pack(fill=BOTH, expand=1)

pinButton.pack()
pinLabel.pack()
uidLabel.pack()

root.protocol("WM_DELETE_WINDOW", on_closing)


def unlock():
	locked = False;
	while True:
		txt = ser.readline()
		if(txt=="setNewPin\r\n"):
			tkMessageBox.showinfo("New pin code", "ready to enter new six button password")
		if(txt=="newPinIsSet\r\n"):
			tkMessageBox.showinfo("pin is set", "new pin code was successfully set!")
		if(txt=="end\r\n"):
			break
		if(txt=="lock\r\n"):
			print("lock")
			if(locked):
				locked = True
			else:
				os.popen('gnome-screensaver-command --lock')
		if(txt=="unlock\r\n"):
			rnd = random.randint(1,100)
			ser.write(str(rnd)+"\r\n")
			#ser.write("\r\n")
			ser.flush()
			altered = rnd*37+317
			txt=ser.readline()
			if(str(txt) == str(altered)+"\r\n"):
				locked = False
				print("Token was accepted")
				os.popen('gnome-screensaver-command --deactivate')
			else:
				#print("the num was "+str(rnd)+" alt was "+str(altered)+" got "+str(txt))
				print("bad token!")

try:
	t1 = Thread(target=unlock, args=())
	t1.start()
	
except Exception as e:
	print("unlock is failing", str(e))

root.mainloop()
#t1.join()


