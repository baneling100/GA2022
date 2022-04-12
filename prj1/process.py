import numpy as np
from PIL import Image
from matplotlib import pyplot as plt

def main():
    rgb = []
    for i in range(256):
        rgb.append((255, i, 0))
    for i in range(254, -1, -1):
        rgb.append((i, 255, 0))
    for i in range(1, 256):
        rgb.append((0, 255, i))
    for i in range(254, -1 -1):
        rgb.append((0, i, 255))
    for i in range(1, 256):
        rgb.append((i, 0, 255))
    img1 = np.zeros((640, 4001, 3), dtype = np.uint8)
    img2 = np.zeros((320, 4001, 3), dtype = np.uint8)
    img3 = np.zeros((320, 4001, 3), dtype = np.uint8)
    with open('rr.out', 'r') as f:
        optimal = f.readline()
        for j in range(4001):
            line = f.readline()
            for i in range(80):
                if optimal[8 * i : 8 * (i + 1)] == line[8 * i : 8 * (i + 1)]:
                    img1[8 * i : 8 * (i + 1), j,] = rgb[len(rgb) * i // 80]
                    if i % 2 == 0:
                        img2[4 * i : 4 * i + 8, j,] = rgb[len(rgb) * i // 80]
                    else:
                        img3[4 * i - 4 : 4 * i + 4, j,] = rgb[len(rgb) * i // 80]
    plt.subplot(311)
    plt.title('whole schema')
    plt.imshow(img1)
    plt.subplot(312)
    plt.title('odd schema')
    plt.imshow(img2)
    plt.subplot(313)
    plt.title('even schema')
    plt.imshow(img3)
    plt.savefig('figure.png')

if __name__ == '__main__':
    main()
