import pygame
import time

# Inicializa o pygame e o módulo de joystick
pygame.init()
pygame.joystick.init()

# Verifica se há controle conectado
if pygame.joystick.get_count() == 0:
    print("Nenhum controle detectado. Conecta o controle de Xbox e reinicia o script.")
    exit()

# Usa o primeiro controle detectado
joystick = pygame.joystick.Joystick(0)
joystick.init()
print(f"Controle detectado: {joystick.get_name()}")

# Mapeamento simples dos botões
botao_map = {
    0: "A",
    1: "B",
    2: "X",
    3: "Y",
    4: "LB",
    5: "RB",
    6: "Back",
    7: "Start",
    8: "R Stick",
    9: "L Stick",
    10: "Xbox",
    11: "Share",
}

print("Iniciando leitura dos comandos. Pressiona Ctrl+C para sair.")

try:
    while True:
        pygame.event.pump()

        # Leitura dos botões
        for i in range(joystick.get_numbuttons()):
            if joystick.get_button(i):
                print(f"Botão pressionado: {botao_map.get(i, f'Botão {i}')}")

        # Leitura dos eixos (sticks analógicos)
        left_y = joystick.get_axis(1)  # vertical do analógico esquerdo
        right_x = joystick.get_axis(3)  # horizontal do analógico direito

        if abs(left_y) > 0.2:
            if left_y < 0:
                print("Analógico Esquerdo: CIMA")
            else:
                print("Analógico Esquerdo: BAIXO")

        if abs(right_x) > 0.2:
            if right_x < 0:
                print("Analógico Direito: ESQUERDA")
            else:
                print("Analógico Direito: DIREITA")

        # Leitura do direcional digital (d-pad)
        hat = joystick.get_hat(0)
        if hat != (0, 0):
            print(f"Direcional Digital (D-Pad): {hat}")

        time.sleep(0.1)

except KeyboardInterrupt:
    print("\nSaindo...")
    joystick.quit()
    pygame.quit()