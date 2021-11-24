from math import cos, sqrt
from PIL import Image, ImageDraw
import pygame
t0, t1 = .10, .80
s0, s1 = .10, .80
c2 = (0.1, 0.1, 0)
L = (.10, .10, 1.0)

normal_map = pygame.image.load('img/f.png')
w = normal_map.get_width()
h = normal_map.get_height()
W, H = 300, 300 #w//2, h//2
normal_map = pygame.transform.scale(normal_map, (W, H))

output_image = pygame.Surface((W, H))
c0, c1 = ((0.1, 0.03, 0.03)), (0.9, 0.8, 1.0)
c0_img = pygame.image.load('img/f_d0.png') # use black.png
c0_img = pygame.transform.scale(c0_img, (W,H))
c1_img = pygame.image.load('img/f_d1.png') # use white.png
c1_img = pygame.transform.scale(c1_img, (W,H))

pygame.init()
win = pygame.display.set_mode((W,H))
pygame.display.set_caption('Render')
clock = pygame.time.Clock()
mouse_X, mouse_Y = 10, 20

def clamp(num, min_value, max_value):
    return max(min(num, max_value), min_value)

def compute_t(N, L):
    t = L[0]*N[0] + L[1]*N[1] + L[2]*N[2]
    t = clamp(t, t0, t1)
    # t = (t - t0)/(t1 - t0)
    # if t > 1: t = 1
    # if t < 0: t = 0
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
            # -------------------------------------------------WITH IMAGE ENVIRONMET AS c2_img
            cl = (L[0]*t, L[1]*t, L[2]*t)
            cr = c0_img.get_at((i,j))[0]/255 * (1 - cl[0]) + c1_img.get_at((i,j))[0]/255 * cl[0]
            cg = c0_img.get_at((i,j))[1]/255 * (1 - cl[1]) + c1_img.get_at((i,j))[1]/255 * cl[1]
            cb = c0_img.get_at((i,j))[2]/255 * (1 - cl[2]) + c1_img.get_at((i,j))[2]/255 * cl[2]
            # -------------------------------------------------WITH COLOR ENVIRONMET AS C2
            # cr = c0[0] * (1 - cl[0]) + c1[0] * cl[0]
            # cg = c0[1] * (1 - cl[1]) + c1[1] * cl[1]
            # cb = c0[2] * (1 - cl[2]) + c1[2] * cl[2]
            # -------------------------------------------------

            # ks = 0.5
            # cr = (c0_img.get_at((i,j))[0]/255 * (1 - t) + c1_img.get_at((i,j))[0]/255 * t) * (1 - ks * s) + (c2[0] * ks * s)
            # cg = (c0_img.get_at((i,j))[1]/255 * (1 - t) + c1_img.get_at((i,j))[1]/255 * t) * (1 - ks * s) + (c2[1] * ks * s)
            # cb = (c0_img.get_at((i,j))[2]/255 * (1 - t) + c1_img.get_at((i,j))[2]/255 * t) * (1 - ks * s) + (c2[2] * ks * s)
            
            # cr = c0_img.get_at((i,j))[0]/255 * (1 - t_temp) + c1_img.get_at((i,j))[0]/255 * t_temp
            # cg = c0_img.get_at((i,j))[1]/255 * (1 - t_temp) + c1_img.get_at((i,j))[1]/255 * t_temp
            # cb = c0_img.get_at((i,j))[2]/255 * (1 - t_temp) + c1_img.get_at((i,j))[2]/255 * t_temp

            # cr = clamp(int((cr * (1 - s) + c2[0]) * 255), 0, 255)
            # cg = clamp(int((cg * (1 - s) + c2[1]) * 255), 0, 255)
            # cb = clamp(int((cb * (1 - s) + c2[2]) * 255), 0, 255)

            cr = clamp(int((cr) * 255), 0, 255)
            cg = clamp(int((cg) * 255), 0, 255)
            cb = clamp(int((cb) * 255), 0, 255)

            output_image.set_at((i,j), (cr, cg, cb))
    win.blit(output_image, (0,0))
    pygame.image.save(output_image, 'output_diffuse1.png')

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