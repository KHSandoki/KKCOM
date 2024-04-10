import tkinter as tk
from tkinter import scrolledtext, ttk
import serial
import threading
import serial.tools.list_ports
import tkinter.messagebox as msg
from tkinter import simpledialog

class SerialApp:
    def __init__(self, master):
        self.master = master
        master.title("KKCOM")
        master.configure(bg='white')

        self.serial_port = serial.Serial()
        self.serial_port.timeout = 1
    
        #//////////
        # frames
        #//////////

        ###upper frame
        self.upper_frame=tk.Frame(master, bg='white')
        self.upper_frame.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

        ###bottom Frame 
        self.bottom_frame = tk.Frame(master, bg='white')
        self.bottom_frame.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

        ###left Frame 
        self.left_frame = tk.Frame(self.upper_frame, bg='white')
        self.left_frame.pack(side=tk.RIGHT, fill=tk.Y, padx=10)

        ###bottom Col_1 Frame 
        self.bottom_c1_frame = tk.Frame(self.bottom_frame, bg='white')
        self.bottom_c1_frame.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

        ###bottom Col_2 Frame 
        self.bottom_c2_frame = tk.Frame(self.bottom_frame, bg='white')
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

        self.text_area = scrolledtext.ScrolledText(self.upper_frame, wrap=tk.WORD, bg='white', fg='black')
        self.text_area.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)
        self.text_area.configure(font=("Consolas", 11,"normal"))

        self.entry = tk.Entry(self.bottom_c1_frame, bg='white', fg='black')
        self.entry.pack(side=tk.LEFT,padx=10, pady=10, fill=tk.BOTH, expand=True)
        self.entry.configure(font=("Consolas", 15,"normal"))

        self.send_button = ttk.Button(self.bottom_c1_frame, text="Send", command=self.send_input_data, style='send_button.TButton')
        self.send_button.pack(side=tk.RIGHT,padx=10, pady=10)
        self.entry.bind('<Return>', self.send_input_data)  # 綁定 Enter 鍵到 send_input_data 方法

        self.connect_button = tk.Button(self.bottom_c2_frame, text="Connect", command=self.toggle_connection, bg='white', fg='black')
        self.connect_button.pack(side=tk.LEFT,padx=10, pady=10)

        self.port_label = tk.Label(self.bottom_c2_frame, text="Select COM Port:", bg='white', fg='black')
        self.port_label.pack(side=tk.LEFT,padx=10, pady=5)

        self.combobox = ttk.Combobox(self.bottom_c2_frame, values=self.get_ports(), state='readonly')
        self.combobox.pack(side=tk.LEFT,padx=10, pady=5)

        self.baudrate_label = tk.Label(self.bottom_c2_frame, text="Select Baudrate:", bg='white', fg='black')
        self.baudrate_label.pack(side=tk.LEFT,padx=10, pady=5)

        self.baudrate_combobox = ttk.Combobox(self.bottom_c2_frame, values=[300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 ,921600], state='readonly')
        self.baudrate_combobox.pack(side=tk.LEFT,padx=10, pady=5)
        self.baudrate_combobox.set(115200)  # Set default baudrate

        self.filter_button = ttk.Button(self.bottom_c2_frame, text="Set Filter", command=self.set_filter)
        self.filter_button.pack(side=tk.LEFT,padx=10, pady=10)

        self.filter_string = ""  # 過濾條件的字串
        self.filter_active = False  # 過濾條件是否啟用的布爾變量

        self.button1 = ttk.Button(self.left_frame, text="Command 1", command=lambda: self.send_command("CMD1"))
        self.button1.pack(pady=5)
        self.button2 = ttk.Button(self.left_frame, text="Command 2", command=lambda: self.send_command("CMD2"))
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
        self.receive_thread = threading.Thread(target=self.receive_data)
        self.receive_thread.daemon = True
        self.receive_thread_running = True  # 新增一個標誌來控制線程的運行
        self.receive_thread.start()
        
        selected_port = self.combobox.get()
        selected_baudrate = self.baudrate_combobox.get()
        try:
            self.serial_port.port = selected_port
            self.serial_port.baudrate = int(selected_baudrate)
            self.serial_port.open()
            msg.showinfo("Success", "Connected to " + selected_port + " at " + selected_baudrate + " baudrate.")
            self.connect_button.config(text="Disconnect", bg='red')  # 更新按鈕狀態
        except serial.SerialException as e:
            msg.showerror("Connection Error", "Error: could not connect to " + selected_port + ". " + str(e))

    def disconnect(self):
        if self.serial_port.is_open:
            self.receive_thread_running = False  # 設置標誌為 False 來停止線程
            self.serial_port.close()
            self.receive_thread.join()  # 等待線程結束
            self.connect_button.config(text="Connect", bg='white')  # 恢復按鈕狀態

if __name__ == "__main__":
    root = tk.Tk()
    app = SerialApp(root)
    root.mainloop()
