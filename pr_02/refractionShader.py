from math import cos, sqrt
from PIL import Image, ImageDraw
import pygame
t0, t1 = .10, .80
# c4 = (1, 0, 0)
d, ks = 20, .55
n = .9
n_img = pygame.image.load('bottle.png')
w = n_img.get_width()
h = n_img.get_height()
W, H = w//3, h//3
n_img = pygame.transform.scale(n_img, (W, H))
output_image = pygame.Surface((W, H))
c0_img = pygame.image.load('black.png')
c0_img = pygame.transform.scale(c0_img, (W,H))
c1_img = pygame.image.load('white.png')
c1_img = pygame.transform.scale(c1_img, (W,H))
CB = pygame.image.load('background.png')
CB = pygame.transform.scale(CB, (W,H))
CE = pygame.image.load('environment.png')
CE = pygame.transform.scale(CE, (W,H))

pygame.init()
win = pygame.display.set_mode((W,H))
pygame.display.set_caption('Render')
clock = pygame.time.Clock()
mouse_X, mouse_Y = 10, 20
# L = (int(20*mouse_X/W), int(20*mouse_Y/H), 0)
L = (mouse_X, mouse_Y, 0)
# L = (3, 5, 0)
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
            
            kr = int(i+(2*d*getpixel_bound(n_img,i,j)[0]*getpixel_bound(n_img,i,j)[2])/(-1+2*getpixel_bound(n_img,i,j)[2]*getpixel_bound(n_img,i,j)[2])) + mouse_X
            mr = int(j+(2*d*getpixel_bound(n_img,i,j)[1]*getpixel_bound(n_img,i,j)[2])/(-1+2*getpixel_bound(n_img,i,j)[2]*getpixel_bound(n_img,i,j)[2])) + mouse_Y
            
            kt = int(i + d*getpixel_bound(n_img,i,j)[2]*n*getpixel_bound(n_img,i,j)[0]/(1 - n + n*getpixel_bound(n_img,i,j)[2]))
            mt = int(j + d*getpixel_bound(n_img,i,j)[2]*n*getpixel_bound(n_img,i,j)[0]/(1 - n + n*getpixel_bound(n_img,i,j)[2]))
            
            kr, mr = kr % W, mr % H
            kt, mt = kt % W, mt % H
            b = 1 - getpixel_bound(n_img,i,j)[2]
            b = (b - 0.9)/(0.95 - 0.9)
            if b < 0: b = 0
            if b > 1: b = 1
            f = b
            
            t = compute_t(getpixel_bound(n_img, i,j), L)
            
            cr2 = (1 - f) * CB.get_at((kt,mt))[0]/255 + f * CE.get_at((kr,mr))[0]/255
            cg2 = (1 - f) * CB.get_at((kt,mt))[1]/255 + f * CE.get_at((kr,mr))[1]/255
            cb2 = (1 - f) * CB.get_at((kt,mt))[2]/255 + f * CE.get_at((kr,mr))[2]/255

            cr = c0_img.get_at((i,j))[0]/255 * (1 - t) + c1_img.get_at((i,j))[0]/255 * t
            cg = c0_img.get_at((i,j))[1]/255 * (1 - t) + c1_img.get_at((i,j))[1]/255 * t
            cb = c0_img.get_at((i,j))[2]/255 * (1 - t) + c1_img.get_at((i,j))[2]/255 * t
            
            cr = (1 - ks) * cr + ks * cr2
            cg = (1 - ks) * cg + ks * cg2
            cb = (1 - ks) * cb + ks * cb2

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
    # win.fill((0,0,0))
    # control()
    action = True
    while action:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                action = False
            if event.type == pygame.MOUSEBUTTONDOWN:
                mouse_X, mouse_Y = pygame.mouse.get_pos()
        redraw()
main_loop()