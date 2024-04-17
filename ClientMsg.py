import socket
import threading
import tkinter as tk
from tkinter import simpledialog, scrolledtext

def receive_messages(sock, chat):
    while True:
        try:
            message = sock.recv(1024).decode('utf-8')
            if message:
                chat.config(state=tk.NORMAL)
                chat.insert(tk.END, message + '\n')
                chat.config(state=tk.DISABLED)
                chat.see(tk.END)
            else:
                break
        except Exception as e:
            print("An error occurred:", e)
            sock.close()
            break

def send_message(event=None):
    global username
    message_text = message_entry.get()
    if message_text:  # Ensuring non-empty messages are sent
        message = f"{username}: {message_text}"
        message_entry.delete(0, tk.END)
        try:
            client_socket.send(message.encode('utf-8'))
            chat_box.config(state=tk.NORMAL)
            chat_box.insert(tk.END, message + '\n')  # Display the user's message locally
            chat_box.config(state=tk.DISABLED)
            chat_box.see(tk.END)
            if message_text == "/quit":
                client_socket.close()
                window.quit()
        except:
            print("Connection closed!")
            client_socket.close()

def join_chatroom():
    # Implement function to join chatroom
    pass

def on_closing(event=None):
    """This function is called when the window is closed."""
    try:
        if client_socket:
            message_entry.delete(0, tk.END)
            message_entry.insert(0, "/quit")
            send_message()
    finally:
        window.destroy()  # Ensure the window closes even if the socket has already been closed

def setup_gui():
    global window, chat_box, message_entry, client_socket, username

    window = tk.Tk()
    window.title("Chat Client")

    username = simpledialog.askstring("Username", "Enter your username:", parent=window)
    if not username:
        username = "Anonymous"  # Default username if none provided

    frame = tk.Frame(window)
    scrollbar = tk.Scrollbar(frame)
    chat_box = scrolledtext.ScrolledText(frame, height=20, width=50, yscrollcommand=scrollbar.set, state=tk.DISABLED)
    scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
    chat_box.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
    frame.pack()

    message_entry = tk.Entry(window, width=50)
    message_entry.bind("<Return>", send_message)
    message_entry.pack()

    send_button = tk.Button(window, text="Send", command=send_message)
    send_button.pack()

    join_button = tk.Button(window, text="Join Chatroom", command=join_chatroom)
    join_button.pack()

    window.protocol("WM_DELETE_WINDOW", on_closing)

def main():
    global client_socket
    host = '127.0.0.1'
    port = 3000

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client_socket.connect((host, port))
    except Exception as e:
        print("Connection to the server failed:", e)
        return

    setup_gui()

    # Start the thread to receive messages
    threading.Thread(target=receive_messages, args=(client_socket, chat_box), daemon=True).start()

    # Start the Tkinter event loop
    tk.mainloop()

if __name__ == "__main__":
    main()
