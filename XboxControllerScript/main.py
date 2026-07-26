import pygame
import socket
import time

pygame.init()
j = pygame.joystick.Joystick(0)
j.init()
print("Controlador:", j.get_name())

UDP_IP = "192.168.0.114"  # IP do ESP32
UDP_PORT = 4210
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

last_axes = (0.0, 0.0)
last_btn = 0
last_sent_toggle = 0

# ajuste deadzone e taxa
DEADZONE = 0.12
SEND_INTERVAL = 0.04

# qual botão você quer usar pra trocar modos? (ex: BACK = 6, START = 7)
BUTTON_TOGGLE = 6


def norm(x):
    if abs(x) < DEADZONE:
        return 0         # zona morta (deadzone)
    return x * abs(x)    # curva exponencial suave


while True:
    pygame.event.pump()
    lx = norm(j.get_axis(0))   # left stick horizontal (não usado)
    ly = -norm(j.get_axis(1))   # left stick vertical (velocidade)
    rx = norm(j.get_axis(3))   # right stick horizontal (steer)

    print("LY raw:", j.get_axis(1), " -> norm:", ly)

    # formato de mensagem: "AX:ly,rx;BTN:0" (ly negado para frente+)
    # invert ly para que push-forward seja positivo
    msg_axes = f"AX:{ly:.3f},{rx:.3f}"

    btn = j.get_button(BUTTON_TOGGLE)
    msg_btn = ""
    # envia toggle só quando botão pressionado e não repetido (debounce)
    if btn and not last_btn:
        msg_btn = "BTN:TOGGLE"
        # envia toggle imediatamente
        sock.sendto(msg_btn.encode(), (UDP_IP, UDP_PORT))
        print("ENVIADO:", msg_btn)
        last_sent_toggle = time.time()

   
    sock.sendto(msg_axes.encode(), (UDP_IP, UDP_PORT))
    #print("ENVIADO:", msg_axes)
    last_axes = (-ly, rx)

    last_btn = btn
    time.sleep(SEND_INTERVAL)