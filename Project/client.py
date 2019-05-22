"""
This is a simple example of a client program written in Python.
Again, this is a very basic example to complement the 'basic_server.c' example.


When testing, start by initiating a connection with the server by sending the "init" message outlined in 
the specification document. Then, wait for the server to send you a message saying the game has begun. 

Once this message has been read, plan out a couple of turns on paper and hard-code these messages to
and from the server (i.e. play a few rounds of the 'dice game' where you know what the right and wrong 
dice rolls are). You will be able to edit this trivially later on; it is often easier to debug the code
if you know exactly what your expected values are. 

From this, you should be able to bootstrap message-parsing to and from the server whilst making it easy to debug.
Then, start to add functions in the server code that actually 'run' the game in the background. 
"""

import socket
import signal
from time import sleep
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def alarm_handler(signum, frame):
    raise TimeoutError

# client side timeout
def input_with_timeout(prompt, time, mess):
    # set signal handler
    signal.signal(signal.SIGALRM, alarm_handler)
    signal.alarm(time) # produce SIGALRM in `timeout` seconds

    try:
        answer = input(prompt)
        signal.alarm(0)
        return answer
    except TimeoutError:   
        if mess:
            print(mess)
        signal.signal(signal.SIGALRM, signal.SIG_IGN)
        return None

def send_move(user_id):
    print("Please make your moves! 1 for Even, 2 for Odd, 3 for Double, 4 for Contain\n")
    timeout = 5
    timeoutmsg = "No Time Left!.\n"
    prompt = "You have %d seconds to pick! Hurry up!\n"%timeout
    move = input_with_timeout(prompt,timeout,timeoutmsg)
    if move is not None:
        try:
            val = int(move)

            if(val == 1):
                mess = user_id+",MOV,"+"EVEN"
                sock.sendall(mess.encode())
            elif(val == 2):
                mess = user_id+",MOV,"+"ODD"
                sock.sendall(mess.encode())
            elif(val == 3):
                mess = user_id+",MOV,"+"DOUB"
                sock.sendall(mess.encode())
            elif(val == 4):
                prompt="Now pick a number again.\n"
                timeout = 5
                timeoutmsg = "No Time Left!"
                con_mess = input_with_timeout(prompt,timeout,timeoutmsg)
                if(con_mess is not None):
                    try:
                        var = int(con_mess)
                        if (int(con_mess)>=0 and int(con_mess)<=6):
                            mess = user_id+",MOV,"+"CON,"+con_mess
                            sock.sendall(mess.encode())
                    except ValueError:
                        print("Please enter number 1 - 6!\n")
                        send_move(user_id)
        except ValueError:
            print("Please enter number 1 - 4!\n")
            send_move(user_id)

def init():
    sock.connect(('localhost', 7777))
    sock.sendall('INIT'.encode())
    data = sock.recv(14).decode("utf-8")
    try:
        data = data.split(',')
    except:
        return -1
    if data[0] == 'WELCOME':
        return data[1];
    else:
        return -1

def game_state():
    data=sock.recv(14).decode();
    try:
        data = data.split(',')
    finally:
        if data[0] == 'START':
            print('There are ' + data[1] + ' player.')
            print('You have ' + data[2] + ' lives.')
            return('START')

def main():
    user_id = init()
    print("Your user id is "+str(user_id)+".")

    if user_id!=-1 and game_state()=='START':
        send_move(user_id)
        while True:
            data = sock.recv(14).decode("utf-8")
            if "PASS" in data:
                print("Your move was right.\n")
                send_move(user_id)
            elif "FAIL" in data:
                print("Your move was wrong.\n")
                send_move(user_id)
            elif "ELIM" in data:
                print("You move was wrong.\n")
                print("You lost, closing connection.")
                sock.close()
                break

if __name__ == "__main__" :
    main()








