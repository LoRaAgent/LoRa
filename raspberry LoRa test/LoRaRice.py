import datetime
import os
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
        self.confirmReceive = False
        self.rssiNode = 0
        self.snrNode = 0

    def set_tx_power(self, tx_power):
        self.set_mode(MODE.TX)
        self.set_pa_config(pa_select=1)
        self.set_pa_config(output_power=tx_power)
        self.set_mode(MODE.STDBY)

    def send(self):
        lora.set_tx_power(12)
        sleep(0.2)
        data_to_send = "Test"
        data_lenght = len(data_to_send)
        data = str(int(self.destination_address,16))+" "+str(int(self.local_address,16))+" "+str(data_lenght)+" "+data_to_send
        self.write_payload(data)
        self.set_mode(MODE.TX)
        self.clear_irq_flags(TxDone=1)
        sleep(0.5)
        self.set_mode(MODE.STDBY)
        self.set_mode(MODE.RXCONT)

    def on_rx_done(self):
        self.clear_irq_flags(RxDone=1)
        payload = self.read_payload(nocheck=True)
        print("Received",payload)
        if payload is not None:
            self.stateRx = True
            self.rssiNode = self.get_pkt_rssi_value()
            self.snrNode = self.get_pkt_snr_value()
            current_time = datetime.datetime.now()
            current_date = current_time.date()
            base_file_name = ''
            file_name = f"{base_file_name}_{current_date}.txt"
            with open(file_name,'a') as file:
                file.write('\n')
                file.write('\nDate : '+ current_time)
                file.write('\nRssi: '+ str(self.rssiNode))
                file.write('\nSnr : '+ str(self.snrNode))
            print("----------------------------------Save Log----------------------------------")
            # self.localaddr_rx = payload[0]
            # self.destinationaddr_rx = payload[1]
            # self.data_lenght_rx = payload[2]
            # if hex(self.localaddr_rx) == self.local_address and hex(self.destinationaddr_rx)== self.destination_address:
            #     payload_without_first_three = payload[3:]
            #     self.received_data = ''.join([chr(num) for num in payload_without_first_three])
            #     if re.search(r'[A-Za-z,.[\]]',self.received_data):
            #         print("Received: ",self.received_data)
            #         sleep(1)
            #         self.stateRx = True
            #         self.rssiNode = self.get_pkt_rssi_value()
            #         self.snrNode = self.get_pkt_snr_value()
            #         current_time = datetime.datetime.now()
            #         current_date = current_time.date()  
            #         current_data = "Time" + str(current_time)
            #         payload_as_str = [str(item) for item in payload]
            #         # สร้างชื่อไฟล์ใหม่โดยมีการลงท้ายด้วยวันที่ปัจจุบัน
            #         base_file_name = ''
            #         file_name = f"{base_file_name}_{current_date}.txt"
            #         # ตรวจสอบว่าไฟล์ใหม่สร้างขึ้นในวันเดียวกันหรือไม่
            #         if not os.path.exists(file_name):
			# 			# สร้างไฟล์ใหม่
            #             with open(file_name, 'a') as file:
            #                 file.write("เริ่มต้นสร้างไฟล์ใหม่...\n")

			# 		# เขียนข้อมูลลงในไฟล์
            #         with open(file_name, 'a') as file:
            #             file.write('\n')
            #             file.write(current_data)
            #             file.write('\nReceive as byte		: ' + ','.join(payload_as_str))
            #             file.write('\nReceive as String	: ' + self.received_data)
            #             file.write('\nRSSI 				: ' + str(self.rssiNode))
            #             file.write('\nSNR  				: ' + str(self.snrNode))
            #         print("--------------------------save log----------------------------")
            #     else:
            #         print("false")

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
        while True:
            try:
                if state == "ONE":
                    lora.send()
                    state = "TWO"
                elif state == "TWO":
                    if self.stateRx == True:
                        print("Received.")
                        state = "ONE"
                    else:
                        count+=1
                        print("Can not receive round: "+count)
            except Exception as e:
                sleep(5)
                print("System Errer : ",e)

lora = LoRaDevice(verbose=False)
lora.set_mode(MODE.STDBY)
lora.set_pa_config(pa_select=1)
lora.set_freq(915.0)      

try:
    lora.start()
except KeyboardInterrupt:
    print("KeyboardInterrupt")
finally:
    BOARD.teardown()