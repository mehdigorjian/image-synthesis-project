from math import cos, sqrt
from PIL import Image, ImageDraw
import pygame
t0, t1 = -.10, .80
c4 = (1, 0, 0)
d, ks = 20, .5

n_img = pygame.image.load('bottle.png')
w = n_img.get_width()
h = n_img.get_height()
W, H = w//3, h//3
n_img = pygame.transform.scale(n_img, (W, H))
output_image = pygame.Surface((W, H))
c0_img = pygame.image.load('c0.png')
c0_img = pygame.transform.scale(c0_img, (W,H))
c1_img = pygame.image.load('c11.png')
c1_img = pygame.transform.scale(c1_img, (W,H))
background_img = pygame.image.load('background.png')
background_img = pygame.transform.scale(background_img, (W,H))
env_img = pygame.image.load('environment.png')
env_img = pygame.transform.scale(env_img, (W,H))

pygame.init()
win = pygame.display.set_mode((W,H))
pygame.display.set_caption('Render')
clock = pygame.time.Clock()
mouse_X, mouse_Y = 10, 20

L = (3, 5, 0)
R = sqrt(L[0]**2 + L[1]**2 + L[2]**2)
L = (L[0]/R, L[1]/R, L[2]/R)


def compute_t(N, L):
    t = L[0]*N[0] + L[1]*N[1] + L[2]*N[2]
    t = (t - t0)/(t1 - t0)
    if t > 1: t = 1
    if t < 0: t = 0
    # print('t: ', t)
    return t

def getpixel_bound(image, a, b):
    x = 2 * (image.get_at((a,b))[0]/255) - 1
    y = 2 * (image.get_at((a,b))[1]/255) - 1
    z = 2 * (image.get_at((a,b))[2]/255) - 1
    if z < 0: z = 1
    return (x, y, z)

def control():
    global L, mouse_X, mouse_Y
    for i in range(W):
        for j in range(H):
            
            k = int(i+(2*d*getpixel_bound(n_img,i,j)[0]*getpixel_bound(n_img,i,j)[2])/(-1+2*getpixel_bound(n_img,i,j)[2]*getpixel_bound(n_img,i,j)[2])) + mouse_X
            m = int(j+(2*d*getpixel_bound(n_img,i,j)[1]*getpixel_bound(n_img,i,j)[2])/(-1+2*getpixel_bound(n_img,i,j)[2]*getpixel_bound(n_img,i,j)[2])) + mouse_Y
            
            k, m = k % W, m % H
            
            t = compute_t(getpixel_bound(n_img, i,j), L)

            cr = c0_img.get_at((i,j))[0]/255 * (1 - t) + c1_img.get_at((i,j))[0]/255 * t
            cg = c0_img.get_at((i,j))[1]/255 * (1 - t) + c1_img.get_at((i,j))[1]/255 * t
            cb = c0_img.get_at((i,j))[2]/255 * (1 - t) + c1_img.get_at((i,j))[2]/255 * t
            
            cr = (1 - ks) * cr + ks * env_img.get_at((k,m))[0]/255
            cg = (1 - ks) * cg + ks * env_img.get_at((k,m))[1]/255
            cb = (1 - ks) * cb + ks * env_img.get_at((k,m))[2]/255

            b = 1 - getpixel_bound(n_img,i,j)[2]
            b = (b - 0.9)/(0.95 - 0.9)
            if b < 0: b = 0
            if b > 1: b = 1

            # cr = (1 - b) * cr + c4[0] * b
            # cg = (1 - b) * cg + c4[1] * b
            # cb = (1 - b) * cb + c4[2] * b
            
            if cr < 0: cr = 0
            if cg < 0: cg = 0
            if cb < 0: cb = 0
            if cr > 1: cr = 1
            if cg > 1: cg = 1
            if cb > 1: cb = 1
            output_image.set_at((i,j), (int(cr*255), int(cg*255), int(cb*255)))
    win.blit(output_image, (0,0))
    pygame.image.save(output_image, 'output_reflection.png')

def redraw():
    win.fill((0,0,0))
    control()
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