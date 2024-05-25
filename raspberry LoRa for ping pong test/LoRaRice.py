import datetime
import os
import time
import re
import socket
from time import sleep
from SX127x.LoRa import *
from SX127x.board_config import BOARD

BOARD.setup()

def check_internet(host="8.8.8.8", port=53, timeout=3):
    try:
        # สร้างเชื่อมต่อ socket
        socket.setdefaulttimeout(timeout)
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((host, port))
        return True
    except socket.error as ex:
        print(ex)
        return False

# Check internet connection
while not check_internet():
    print("No internet connection. Waiting for internet connection...")
    sleep(10)

# If internet connection is available, run your program
print("Internet connection detected. Running your program...")

class LoRaDevice(LoRa):
    def __init__(self, verbose=False):
        super(LoRaDevice, self).__init__(verbose)
        self.set_mode(MODE.SLEEP)
        self.set_dio_mapping([0]*6)
        self.local_address = 0x23
        self.destination_address = 0xaf
        self.localaddr_rx = 0
        self.destinationaddr_rx = 0
        self.data_lenght_rx = 0
        self.stateRx = False
        self.stateTx = False
        self.confirmReceive = False
        self.rssiNode = 0
        self.snrNode = 0

    def set_tx_power(self, tx_power):
        self.set_mode(MODE.TX)
        self.set_pa_config(pa_select=1)
        self.set_pa_config(output_power=tx_power)
        self.set_mode(MODE.STDBY)

    def send(self):
        lora.set_tx_power(18)
        sleep(0.2)
        data_to_send = "Test"
        self.write_payload(data_to_send)
        self.set_mode(MODE.TX)
        self.clear_irq_flags(TxDone=1)
        sleep(0.5)
        self.set_mode(MODE.STDBY)
        self.set_mode(MODE.RXCONT)

    def on_rx_done(self):
        self.clear_irq_flags(RxDone=1)
        payload = self.read_payload(nocheck=True)
        self.rssiNode = self.get_pkt_rssi_value()
        self.snrNode = self.get_pkt_snr_value()
        print("Received",payload)
        print("Rssi : ",self.rssiNode)
        print("Snr : ",self.snrNode)
        if payload is not None:
            self.stateRx = True
            self.received_data = ''.join(chr(byte) for byte in payload)
            if re.search(r'[A-Za-z,.[\]]',self.received_data):
                print("Received : ",self.received_data)
                current_time = datetime.datetime.now()
                current_date = current_time.date()
                current_data = "Time" + str(current_time)
                base_file_name = '/home/LoRa/Desktop/raspberry LoRa test/Log'
                file_name = f"{base_file_name}_{current_date}.txt"
                #write data into file
                with open(file_name,'a') as file:
                    file.write('\n')
                    file.write(current_data)
                    file.write('\nReceive as String	: ' + self.received_data)
                    file.write('\nRSSI 				: ' + str(self.rssiNode))
                    file.write('\nSNR  				: ' + str(self.snrNode))

    def reset_value(self):
        self.local_address = hex(0x23)
        self.destination_address = hex(0xaf)
        self.received_data = 0
        self.localaddr_rx = 0
        self.destinationaddr_rx = 0
        self.data_lenght_rx = 0
        self.stateRx = False

    def start(self):
        self.reset_ptr_rx()
        self.set_mode(MODE.RXCONT)
        state = "ONE"  #assign state
        count = 0
        start_time = time.time()*1000
        while True:
            current_time = int(round(time.time()*1000))
            if current_time - start_time >= 1000:
                if state == "ONE":
                        sys.stdout.flush()
                        if self.stateRx == True:
                                self.stateRx = False
                                state = "TWO"
                        else:
                                print("Can not receive")
                                start_time = current_time
                elif state == "TWO":
                        lora.send()
                        state = "ONE"
                start_time = current_time


lora = LoRaDevice(verbose=False)
lora.set_mode(MODE.STDBY)
lora.set_pa_config(pa_select=1)
lora.set_freq(915.0)      
lora.set_spreading_factor(7)

try:
    lora.start()
except KeyboardInterrupt:
    print("KeyboardInterrupt")
finally:
    BOARD.teardown()
