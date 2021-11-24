from math import cos, sqrt
from PIL import Image, ImageDraw
import pygame
W, H = 300, 300 #w//2, h//2

pygame.init()
win = pygame.display.set_mode((W,H))
pygame.display.set_caption('Render')
clock = pygame.time.Clock()
mouse_X, mouse_Y = 10, 20

t0, t1 = 0.0, 1.0
s0, s1 = 0.0, 1.0
c2 = (0.0, 1.0, 0.0)
L = (0.0, 0.0, 1.0)

normal_map = pygame.image.load('img/f_1.png')
w = normal_map.get_width()
h = normal_map.get_height()
# W, H = w//2, h//2
normal_map = pygame.transform.scale(normal_map, (W, H))
normal_map_with_alpha = normal_map.convert_alpha()
output_image = pygame.Surface((W, H))

c0_img = pygame.image.load('img/f_1_d0.png')
c0_img = pygame.transform.scale(c0_img, (W,H))
c1_img = pygame.image.load('img/f_1_d1.png')
c1_img = pygame.transform.scale(c1_img, (W,H))
# c2_img = pygame.image.load('img/b.png')
# c2_img = pygame.transform.scale(c2_img, (W,H))

def clamp(num, min_value, max_value):
    return max(min(num, max_value), min_value)

def compute_t(N, L):
    t = L[0]*N[0] + L[1]*N[1] + L[2]*N[2]
    t = clamp(t, t0, t1)
    return t

def compute_s(N, L):
    s = -L[2] + 2 * (L[0]*N[0] + L[1]*N[1] + L[2]*N[2]) * N[2]
    s = clamp(s, s0, s1)
    return s

def compute_xyz(image, a, b):
    x = 2 * (image.get_at((a,b))[0]/255) - 1
    y = 2 * (image.get_at((a,b))[1]/255) - 1
    z = 2 * (image.get_at((a,b))[2]/255) - 1
    return (x, y, z)

def processing():
    global L, mouse_X, mouse_Y
    for i in range(W):
        for j in range(H):
            d = sqrt( (mouse_X - i)**2 + (mouse_Y - j)**2 + L[2]**2 )
            if d == 0: d = .01
            L = ( (mouse_X - i)/d, (mouse_Y - j)/d, L[2]/d )

            t = compute_t(compute_xyz(normal_map, i,j), L)
            s = compute_s(compute_xyz(normal_map, i,j), L)
            # -------------------------------------------------WITH COLOR ENVIRONMET AS C2
            # ks = 0.8
            # cr = (c0_img.get_at((i,j))[0]/255 * (1 - t) + c1_img.get_at((i,j))[0]/255 * t) * (1 - ks * s) + (c2[0] * ks * s)
            # cg = (c0_img.get_at((i,j))[1]/255 * (1 - t) + c1_img.get_at((i,j))[1]/255 * t) * (1 - ks * s) + (c2[1] * ks * s)
            # cb = (c0_img.get_at((i,j))[2]/255 * (1 - t) + c1_img.get_at((i,j))[2]/255 * t) * (1 - ks * s) + (c2[2] * ks * s)
            # -------------------------------------------------WITH IMAGE ENVIRONMET AS c2_img
            ks = 1 - normal_map_with_alpha.get_at((i,j))[3]/255.0
            cr = (c0_img.get_at((i,j))[0]/255 * (1 - t) + c1_img.get_at((i,j))[0]/255 * t) * (1 - ks * s) + (c2_img.get_at((i,j))[0]/255 * ks * s)
            cg = (c0_img.get_at((i,j))[1]/255 * (1 - t) + c1_img.get_at((i,j))[1]/255 * t) * (1 - ks * s) + (c2_img.get_at((i,j))[1]/255 * ks * s)
            cb = (c0_img.get_at((i,j))[2]/255 * (1 - t) + c1_img.get_at((i,j))[2]/255 * t) * (1 - ks * s) + (c2_img.get_at((i,j))[2]/255 * ks * s)
            # -------------------------------------------------
            cr = clamp(int((cr) * 255), 0, 255)
            cg = clamp(int((cg) * 255), 0, 255)
            cb = clamp(int((cb) * 255), 0, 255)

            output_image.set_at((i,j), (cr, cg, cb))
    win.blit(output_image, (0,0))
    pygame.image.save(output_image, 'output_specular.png')

def redraw():
    win.fill((0,0,0))
    processing()
    pygame.display.update()
    clock.tick(30)

def main_loop():
    global mouse_X, mouse_Y
    action = True
    while action:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                action = False
            if event.type == pygame.MOUSEBUTTONDOWN:
                mouse_X, mouse_Y = pygame.mouse.get_pos()
        redraw()

main_loop()