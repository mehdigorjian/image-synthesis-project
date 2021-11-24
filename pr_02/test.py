import pygame
c2 = pygame.image.load('background.jpg')
for i in range(c2.get_width()):
    for j in range(c2.get_height()):
        print(c2.get_at((i,j)))