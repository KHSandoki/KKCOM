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

        self.text_area = scrolledtext.ScrolledText(master, wrap=tk.WORD, bg='white', fg='black')
        self.text_area.pack(padx=10, pady=10)

        self.entry = tk.Entry(master, bg='white', fg='black')
        self.entry.pack(padx=10, pady=10)

        self.send_button = tk.Button(master, text="Send", command=self.send_data, bg='white', fg='black')
        self.send_button.pack(padx=10, pady=10)
        self.entry.bind('<Return>', self.send_data)  # 綁定 Enter 鍵到 send_data 方法

        self.port_label = tk.Label(master, text="Select COM Port:", bg='white', fg='black')
        self.port_label.pack(padx=10, pady=5)

        self.combobox = ttk.Combobox(master, values=self.get_ports(), state='readonly')
        self.combobox.pack(padx=10, pady=5)

        self.baudrate_label = tk.Label(master, text="Select Baudrate:", bg='white', fg='black')
        self.baudrate_label.pack(padx=10, pady=5)

        self.baudrate_combobox = ttk.Combobox(master, values=[300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 ,921600], state='readonly')
        self.baudrate_combobox.pack(padx=10, pady=5)
        self.baudrate_combobox.set(9600)  # Set default baudrate

        self.connect_button = tk.Button(master, text="Connect", command=self.toggle_connection, bg='white', fg='black')
        self.connect_button.pack(padx=10, pady=10)

        self.filter_button = tk.Button(master, text="Set Filter", command=self.set_filter, bg='white', fg='black')
        self.filter_button.pack(padx=10, pady=10)

        self.filter_string = ""  # 過濾條件的字串
        self.filter_active = False  # 過濾條件是否啟用的布爾變量

    def send_data(self, event=None):  # 添加 event 參數以處理事件綁定
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
