import curses
import time

GRID_SIZE = 30
OBSTACLE_DISTANCE = 20  # cm

# Inicializa o grid
# 0 = livre, 1 = obstáculo, 2 = robô
class GridMap:
    def __init__(self, size):
        self.size = size
        self.grid = [[0 for _ in range(size)] for _ in range(size)]
        self.robot_x = size // 2
        self.robot_y = size // 2
        self.heading = 0  # 0=norte, 1=leste, 2=sul, 3=oeste

    def update(self, dist_front, dist_left, dist_right):
        dx = [0, 1, 0, -1]
        dy = [1, 0, -1, 0]
        # Marca obstáculos
        if dist_front < OBSTACLE_DISTANCE:
            tx = self.robot_x + dx[self.heading]
            ty = self.robot_y + dy[self.heading]
            if 0 <= tx < self.size and 0 <= ty < self.size:
                self.grid[ty][tx] = 1
        left_heading = (self.heading + 3) % 4
        if dist_left < OBSTACLE_DISTANCE:
            tx = self.robot_x + dx[left_heading]
            ty = self.robot_y + dy[left_heading]
            if 0 <= tx < self.size and 0 <= ty < self.size:
                self.grid[ty][tx] = 1
        right_heading = (self.heading + 1) % 4
        if dist_right < OBSTACLE_DISTANCE:
            tx = self.robot_x + dx[right_heading]
            ty = self.robot_y + dy[right_heading]
            if 0 <= tx < self.size and 0 <= ty < self.size:
                self.grid[ty][tx] = 1

    def move_forward(self):
        dx = [0, 1, 0, -1]
        dy = [1, 0, -1, 0]
        nx = self.robot_x + dx[self.heading]
        ny = self.robot_y + dy[self.heading]
        if 0 <= nx < self.size and 0 <= ny < self.size:
            self.robot_x = nx
            self.robot_y = ny

    def turn_left(self):
        self.heading = (self.heading + 3) % 4

    def turn_right(self):
        self.heading = (self.heading + 1) % 4

    def draw(self, stdscr):
        stdscr.clear()
        for y in range(self.size):
            for x in range(self.size):
                if x == self.robot_x and y == self.robot_y:
                    stdscr.addstr(y, x, 'R', curses.color_pair(2))
                elif self.grid[y][x] == 1:
                    stdscr.addstr(y, x, 'O', curses.color_pair(1))
                else:
                    stdscr.addstr(y, x, '.', curses.color_pair(3))
        stdscr.refresh()

# Exemplo de função para receber dados dos sensores
# Substitua por leitura serial ou outro método real
import random
def get_sensor_data():
    # Simula dados dos sensores
    return random.randint(10, 30), random.randint(10, 30), random.randint(10, 30)

def main(stdscr):
    curses.start_color()
    curses.init_pair(1, curses.COLOR_RED, curses.COLOR_BLACK)   # Obstáculo
    curses.init_pair(2, curses.COLOR_GREEN, curses.COLOR_BLACK) # Robô
    curses.init_pair(3, curses.COLOR_WHITE, curses.COLOR_BLACK) # Livre
    grid = GridMap(GRID_SIZE)
    while True:
        dist_front, dist_left, dist_right = get_sensor_data()
        grid.update(dist_front, dist_left, dist_right)
        grid.draw(stdscr)
        # Exemplo de movimentação
        grid.move_forward()
        if random.random() < 0.2:
            grid.turn_left()
        elif random.random() < 0.2:
            grid.turn_right()
        time.sleep(0.05)

if __name__ == '__main__':
    curses.wrapper(main)
