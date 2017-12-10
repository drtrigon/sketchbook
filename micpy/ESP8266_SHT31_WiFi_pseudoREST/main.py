import machine
import time
import os

#CONTENT = """\
#HTTP/1.0 200 OK
#Content-Type: text/plain
#
#Hello #{} from MicroPython!
#"""
CONTENT = """\
HTTP/1.0 200 OK
Content-Type: text/plain

{}
"""

# mimic Arduino IDE ESP8266 code aREST interface
aREST = {"DESC": "SHT31 Sensor I2C MicroPython Module",
         #"DESC": os.uname().machine,
         #"DATE": os.path.getmtime("main.py"),
         "DATE": str(machine.RTC().datetime()[:3]),   # alternatively use time.localtime()
         "TIME": str(machine.RTC().datetime()[3:6]),  #
         "IDE": "",
         "VERSION": os.uname().version,
         "RELEASE": os.uname().release,               # new
         "FILE": "",
         "ID": machine.unique_id(),
         "NAME": os.uname().nodename,
         #"NAME": os.uname().sysname,
         "variables": {}}

print("Executing main.py")

# http://docs.micropython.org/en/v1.8.7/esp8266/esp8266/tutorial/network_basics.html
# put in your boot.py file (but I want to change main.py only)
# switches sta network on, ap network stays on (see comments on how to switch off)
# NEEDS ONLY TO BE CALLED ONCE - E.G. FROM WEBREPL - AS ESP8266/MICROPYTHON STORES THESE SETTINGS
def do_connect():
    import network
    SSID = 'Buffalo-G-E69A'
    PASSWORD = '14661655'
    sta_if = network.WLAN(network.STA_IF)
    #ap_if = network.WLAN(network.AP_IF)
    if not sta_if.isconnected():
        print('connecting to network...')
        sta_if.active(True)                # switch sta network on (client)
        sta_if.connect(SSID, PASSWORD)
        while not sta_if.isconnected():
            pass
        #if ap_if.active():
        #    ap_if.active(False)            # switch ap network off (access point)
    print('network config:', sta_if.ifconfig())

# https://github.com/micropython/micropython/wiki/Board-Teensy-3.1-3.5-3.6
def info():
    import esp
    #pyb.info()
    print(machine.unique_id())
    print(machine.freq())
    print(machine.reset_cause())
    print(esp.check_fw())
    print(os.uname())
    print(dir())

# https://github.com/kfricke/micropython-sht31
# https://docs.micropython.org/en/latest/esp8266/esp8266/quickref.html#i2c-bus
def read_sensor():
    import sht31
    i2c = machine.I2C(scl=machine.Pin(2), sda=machine.Pin(0))  # scl:GPIO2, sda:GPIO0
    data = (-45.0, 100.0)
    for i in range(10):                 # max. 10 tries
        sht = sht31.SHT31(i2c)
        try:
            data = sht.get_temp_humi()  # may return (-45.0, 100.0) or fail on error
        except:
            pass
        if (data[0] > -45.0) or (data[1] < 100.0):
            break
        time.sleep(0.1)
    return data

# https://github.com/cswiger/wipy/blob/master/http-server.py
def run_server():
    import usocket as socket
    import network
    import json

    sta_if = network.WLAN(network.STA_IF)
    ip = sta_if.ifconfig()[0]              # get ip in sta mode, z.B. 192.168.11.13

    #ai = socket.getaddrinfo("192.168.1.235", 8080)
    #ai = socket.getaddrinfo("192.168.4.1", 8080)   # ap mode (access point)
    ai = socket.getaddrinfo(ip, 8080)      # sta mode (client)
    addr = ai[0][4]
    print(addr)

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(addr)
    s.listen(5)
    counter=0

    while True:
        res = s.accept()
        client_s = res[0]
        client_addr = res[1]
        print("Client address:", client_addr)
        print("Client socket:", client_s)
        #print("Request:")
        req = client_s.recv(4096)
        #print(req)
        #client_s.send(bytes(CONTENT.format(counter), "ascii"))
        dr = read_sensor()
        d  = dict(zip(('temperature', 'humidity', 'Vs'), dr + (0.,)))
        #d.update(aREST)
        aREST["variables"] = d
        #client_s.send(bytes(CONTENT.format(d), "ascii"))
        client_s.send(bytes(CONTENT.format(json.dumps(aREST)), "ascii"))
        client_s.close()
        parts = req.decode('ascii').split(' ')
        print(parts)
        if parts[1] == '/exit':
            s.close()
            print("exit!")
            break
        counter += 1

time.sleep(2.)  # some boot-up timeout

led = machine.Pin(1, machine.Pin.OUT)  # GPIO1

led.off()  # on
#machine.delay(100)
#time.sleep(0.1)
time.sleep(1.)
led.on()   # off
time.sleep(1.)
led.off()  # on
time.sleep(1.)
led.on()   # off

run_server()    # start and run server - blocking call therfore last

