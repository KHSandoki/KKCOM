import time
import tkinter as tk
from tkinter import scrolledtext, ttk
import serial
import threading
import serial.tools.list_ports
import tkinter.messagebox as msg
from tkinter import simpledialog
import customtkinter as ctk

class SerialApp:
    def __init__(self, master):
        self.master = master
        master.title("KKCOM")
        master.configure(bg='white')

        self.serial_port = serial.Serial()
        self.serial_port.timeout = 0.04

        ctk.set_appearance_mode("dark")  # Modes: system (default), light, dark
    
        #//////////
        # frames
        #//////////

        ###upper frame
        self.upper_frame=ctk.CTkFrame(master)
        self.upper_frame.pack( padx=10, pady=10,fill=tk.BOTH, expand=True)

        ###bottom Frame 
        self.bottom_frame = ctk.CTkFrame(master)
        self.bottom_frame.pack(padx=10, pady=5, fill=tk.BOTH, expand=True)

        ###left Frame 
        self.left_frame = ctk.CTkFrame(self.upper_frame )
        self.left_frame.pack(side=tk.RIGHT, fill=tk.Y)

        ###bottom wiget Frame 
        self.bottom_widget_frame = ctk.CTkFrame(self.bottom_frame)
        self.bottom_widget_frame.pack(padx=10, pady=5, fill=tk.BOTH, expand=True)

        ###bottom Col_1 Frame 
        self.bottom_c1_frame = ctk.CTkFrame(self.bottom_frame)
        self.bottom_c1_frame.pack(padx=10, pady=5, fill=tk.BOTH, expand=True)

        ###bottom Col_2 Frame 
        self.bottom_c2_frame = ctk.CTkFrame(self.bottom_frame)
        self.bottom_c2_frame.pack(padx=10, pady=5, fill=tk.BOTH, expand=True)

        ###EXT_tab
        self.ext_tab = ctk.CTkTabview(self.left_frame, width=250)
        self.ext_tab.pack(padx=5,pady=5)
        self.ext_tab.add("EXT 1")
        self.ext_tab.add("EXT 2")
        self.ext_tab.add("EXT 3")
        self.ext_tab.tab("EXT 1").grid_columnconfigure(0, weight=1)  # configure grid of individual tabs
        self.ext_tab.tab("EXT 2").grid_columnconfigure(0, weight=1)

        ###EXT1 Frame 
        self.ext1_scrollable_frame = ctk.CTkScrollableFrame(self.ext_tab.tab("EXT 1"))
        self.ext1_scrollable_frame.pack(fill=tk.BOTH, expand=True)

        ###EXT2 Frame 
        self.ext2_scrollable_frame = ctk.CTkScrollableFrame(self.ext_tab.tab("EXT 2"))
        self.ext2_scrollable_frame.pack(fill=tk.BOTH, expand=True)

        ###EXT3 Frame 
        self.ext3_scrollable_frame = ctk.CTkScrollableFrame(self.ext_tab.tab("EXT 3"))
        self.ext3_scrollable_frame.pack(fill=tk.BOTH, expand=True)

        #//////////
        # elements 
        #//////////

        self.text_area = ctk.CTkTextbox(self.upper_frame, wrap=tk.WORD)
        self.text_area.pack( fill=tk.BOTH, expand=True)
        self.text_area.configure(font=("Consolas", 13,"normal"))

        self.entry = ctk.CTkEntry(self.bottom_c1_frame,font=("Consolas", 15,"normal"))
        self.entry.pack(side=tk.LEFT,padx=10, pady=10, fill=tk.BOTH, expand=True)
        self.entry.configure(font=("Consolas", 15,"normal"))

        self.send_every_sw = ctk.CTkSwitch(self.bottom_c1_frame, text=" ",command=self.send_every_event)
        self.send_every_sw.pack(side=tk.RIGHT,padx=10, pady=10)

        self.send_every_entry = ctk.CTkEntry(self.bottom_c1_frame, placeholder_text="Send Every..sec",width=100)
        self.send_every_entry.pack(side=tk.RIGHT,padx=10, pady=10)

        self.send_button = ctk.CTkButton(self.bottom_c1_frame, text="Send", command=self.send_input_data)
        self.send_button.pack(side=tk.RIGHT,padx=10, pady=10)
        self.entry.bind('<Return>', self.send_input_data)  # 綁定 Enter 鍵到 send_input_data 方法

        self.clear_button = ctk.CTkButton(self.bottom_widget_frame, text="Clear", command=self.clear_text_area,fg_color="transparent", border_width=2)
        self.clear_button.pack(side=tk.LEFT)

        self.filter_button = ctk.CTkButton(self.bottom_widget_frame, text="Set Filter", command=self.set_filter,fg_color="transparent", border_width=2)
        self.filter_button.pack(side=tk.LEFT,padx=2)

        self.connect_button = tk.Button(self.bottom_c2_frame, text="Connect", command=self.toggle_connection,  width=10, height=4, bd=0, bg='#16825D', fg='white', activeforeground='white', relief='flat',font=("Consolas", 11,"bold"))
        self.connect_button.pack(side=tk.LEFT,padx=10, pady=10)

        self.port_label = ctk.CTkLabel(self.bottom_c2_frame, text="Select COM Port:")
        self.port_label.pack(side=tk.LEFT,padx=10, pady=5)

        self.combobox = ctk.CTkComboBox(self.bottom_c2_frame, values=self.get_ports())
        self.combobox.pack(side=tk.LEFT,padx=10, pady=5)

        self.com_port_refresh = ctk.CTkButton(self.bottom_c2_frame, text="Refresh", command=self.update_ports,width=50,fg_color="transparent",border_width=2)
        self.com_port_refresh.pack(side=tk.LEFT,padx=5, pady=5)

        self.baudrate_label = ctk.CTkLabel(self.bottom_c2_frame, text="Select Baudrate:")
        self.baudrate_label.pack(side=tk.LEFT,padx=10, pady=5)

        self.baudrate_combobox = ctk.CTkComboBox(self.bottom_c2_frame, values=["300", "600", "1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200" ,"921600"])
        self.baudrate_combobox.pack(side=tk.LEFT,padx=10, pady=5)
        self.baudrate_combobox.set("115200")  # Set default baudrate

        self.filter_string = ""  # 過濾條件的字串
        self.filter_active = False  # 過濾條件是否啟用的布爾變量

        self.ext1_entry_list    = []
        self.ext1_button_list   = []
        for i in range(100):
            entry = ctk.CTkEntry(master=self.ext1_scrollable_frame)
            entry.pack(padx=5, pady=5)
            self.ext1_entry_list.append(entry)

            button = ctk.CTkButton(master=self.ext1_scrollable_frame ,text=f"{i}")
            button.pack(padx=5, pady=5)
            button.configure(command=lambda i=i: self.ext_button_event(i))  # Pass the index to the callback
            self.ext1_button_list.append(button)

    def send_command(self, command):
        if self.serial_port.is_open:
            data = command+'\n'
            self.serial_port.write(data.encode())

    def send_input_data(self, event=None):  # 添加 event 參數以處理事件綁定
        data = self.entry.get() + '\n'  # 在字符串末尾添加換行符號
        if self.serial_port.is_open:
            self.serial_port.write(data.encode())
        #self.entry.delete(0, tk.END)  # 發送後清空輸入框

    def set_filter(self):
        self.filter_string = simpledialog.askstring("Input", "Enter the string to filter by:",
                                                    parent=self.master)
        if self.filter_string is not None:
            self.filter_active = True

    def receive_data(self):
        while self.receive_thread_running:
            if self.serial_port.is_open:
                data = self.serial_port.readall()
                if data:
                    decoded_data = data.decode()
                    if self.filter_active and self.filter_string in decoded_data:
                        self.text_area.insert(tk.END, decoded_data)
                    elif not self.filter_active:
                        self.text_area.insert(tk.END, decoded_data)
                    self.text_area.yview(tk.END)

    def get_ports(self):
        ports = serial.tools.list_ports.comports()
        if not [port.device for port in ports]:
            return ["No port found"]
        else:
            return [port.device for port in ports]
        
    def update_ports(self):
        port_list=self.get_ports()
        self.combobox.configure(values=port_list)
        self.combobox.set(port_list[0])

    def toggle_connection(self):
        if self.serial_port.is_open:
            self.disconnect()
        else:
            self.connect_port()

    def connect_port(self):
        selected_port = self.combobox.get()
        selected_baudrate = self.baudrate_combobox.get()
        try:
            self.serial_port.port = selected_port
            self.serial_port.baudrate = int(selected_baudrate)
            self.serial_port.open()
            #msg.showinfo("Success", "Connected to " + selected_port + " at " + selected_baudrate + " baudrate.")
            self.connect_button.config(text="Disconnect", bg='red')  # 更新按鈕狀態
        except serial.SerialException as e:
            msg.showerror("Connection Error", "Error: could not connect to " + selected_port + ". " + str(e))

        if self.serial_port.is_open:
            self.receive_thread = threading.Thread(target=self.receive_data)
            self.receive_thread.daemon = True
            self.receive_thread_running = True  # 新增一個標誌來控制線程的運行
            self.receive_thread.start()

    def disconnect(self):
        if self.serial_port.is_open:
            self.receive_thread_running = False  # 設置標誌為 False 來停止線程
            self.serial_port.reset_input_buffer()
            self.serial_port.reset_output_buffer()
            self.serial_port.close()
            self.receive_thread.join()  # 等待線程結束
            self.connect_button.config(text="Connect", bg='#16825D')  # 恢復按鈕狀態

    def clear_text_area(self):
        # Clear the text area
        self.text_area.delete(1.0, tk.END)
    
    def send_every(self):
        while self.send_every_thread_running:
            send_every_sec=self.send_every_entry.get()
            try:
                send_every_sec = int(send_every_sec)
            except:
                msg.showerror("Value invalid","Value invalid! Please input correct value (in second).")
                self.send_every_thread_running = False
                self.send_every_sw.toggle() #turn off switch if value is invalid

            self.send_input_data()
            time.sleep(send_every_sec)
            print("send_every")

    def send_every_event(self):
        send_every_sw_status=self.send_every_sw.get()
        #print(send_every_sw_status)
        if   send_every_sw_status == 0 :
            self.send_every_thread_running = False
        elif send_every_sw_status == 1 :
            self.send_every_thread_running = True
            self.send_every_thread = threading.Thread(target=self.send_every)
            self.send_every_thread.start()
            print("send_every_event send_every_sw_status =1")

    def ext_button_event(self,index):
        # Get the text from the corresponding entry
        entry_text = self.ext1_entry_list[index].get()
        print(f"Button {index} pressed. Entry text: {entry_text}")
        self.send_command(entry_text)

if __name__ == "__main__":
    root = ctk.CTk()
    app = SerialApp(root)
    root.mainloop()
