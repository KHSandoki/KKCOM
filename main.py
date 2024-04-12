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
        self.serial_port.timeout = 1

        ctk.set_appearance_mode("dark")  # Modes: system (default), light, dark
    
        #//////////
        # frames
        #//////////

        ###upper frame
        self.upper_frame=ctk.CTkFrame(master)
        self.upper_frame.pack( padx=10, pady=10,fill=tk.BOTH, expand=True)

        ###bottom Frame 
        self.bottom_frame = ctk.CTkFrame(master)
        self.bottom_frame.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

        ###left Frame 
        self.left_frame = ctk.CTkFrame(self.upper_frame )
        self.left_frame.pack(side=tk.RIGHT, fill=tk.Y)

        ###bottom wiget Frame 
        self.bottom_widget_frame = ctk.CTkFrame(self.bottom_frame)
        self.bottom_widget_frame.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

        ###bottom Col_1 Frame 
        self.bottom_c1_frame = ctk.CTkFrame(self.bottom_frame)
        self.bottom_c1_frame.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

        ###bottom Col_2 Frame 
        self.bottom_c2_frame = ctk.CTkFrame(self.bottom_frame)
        self.bottom_c2_frame.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)


        #//////////
        # styles 
        #//////////

        self.style = ttk.Style()
        self.style.configure('send_button.TButton', 
                background='white', 
                foreground='gray', 
                font=('Arial', 12, 'bold'), 
                borderwidth=0, 
                relief='raised')
        
        self.style.configure('connect_button.TButton', 
                background='green', 
                foreground='green', 
                font=('Arial', 12, 'bold'), 
                borderwidth=0, 
                relief='raised')

        #//////////
        # elements 
        #//////////

        self.text_area = ctk.CTkTextbox(self.upper_frame, wrap=tk.WORD)
        self.text_area.pack( fill=tk.BOTH, expand=True)
        self.text_area.configure(font=("Consolas", 13,"normal"))

        self.entry = ctk.CTkEntry(self.bottom_c1_frame)
        self.entry.pack(side=tk.LEFT,padx=10, pady=10, fill=tk.BOTH, expand=True)
        self.entry.configure(font=("Consolas", 15,"normal"))

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

        self.baudrate_label = ctk.CTkLabel(self.bottom_c2_frame, text="Select Baudrate:")
        self.baudrate_label.pack(side=tk.LEFT,padx=10, pady=5)

        self.baudrate_combobox = ctk.CTkComboBox(self.bottom_c2_frame, values=["300", "600", "1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200" ,"921600"])
        self.baudrate_combobox.pack(side=tk.LEFT,padx=10, pady=5)
        self.baudrate_combobox.set("115200")  # Set default baudrate

        self.filter_string = ""  # 過濾條件的字串
        self.filter_active = False  # 過濾條件是否啟用的布爾變量


        self.button1 = ttk.Button(self.left_frame, text="lte debug on", command=lambda: self.send_command("lte debug on"))
        self.button1.pack(pady=5)
        self.button2 = ttk.Button(self.left_frame, text="system shutdown", command=lambda: self.send_command("system shutdown"))
        self.button2.pack(pady=5)

    def send_command(self, command):
        if self.serial_port.is_open:
            data = command+'\n'
            self.serial_port.write(data.encode())

    def send_input_data(self, event=None):  # 添加 event 參數以處理事件綁定
        data = self.entry.get() + '\n'  # 在字符串末尾添加換行符號
        if self.serial_port.is_open:
            self.serial_port.write(data.encode())
        self.entry.delete(0, tk.END)  # 發送後清空輸入框

    def set_filter(self):
        self.filter_string = simpledialog.askstring("Input", "Enter the string to filter by:",
                                                    parent=self.master)
        if self.filter_string is not None:
            self.filter_active = True

    def receive_data(self):
        while self.receive_thread_running:
            if self.serial_port.is_open:
                data = self.serial_port.readline()
                if data:
                    decoded_data = data.decode()
                    if self.filter_active and self.filter_string in decoded_data:
                        self.text_area.insert(tk.END, decoded_data)
                    elif not self.filter_active:
                        self.text_area.insert(tk.END, decoded_data)
                    self.text_area.yview(tk.END)

    def get_ports(self):
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

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

if __name__ == "__main__":
    root = ctk.CTk()
    app = SerialApp(root)
    root.mainloop()
