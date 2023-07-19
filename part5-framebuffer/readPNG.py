#!/usr/bin/env python3

import png
import sys

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("./readPNG infile.png")

    infile  = sys.argv[1]

    image = []
    png_reader = png.Reader(infile)
    image_data = png_reader.asRGBA8()

    genColor = lambda r, g, b, a : (a << 24) | (r << 16) | (g << 8) | b

    for row in image_data[2]:
        for r, g, b, a in zip(row[::4], row[1::4], row[2::4], row[3::4]):
            image.append(genColor(r, g, b, a))

    with open("img.h", 'w') as file:
        file.write("uint32_t imgWidth  = {};\n".format(image_data[0]))
        file.write("uint32_t imgHeight = {};\n".format(image_data[1]))
        
        file.write("uint32_t array[{}] = ".format(len(image)))
        file.write("{ ")

        for value in image:
            file.write(str(value) + ", ")

        file.write("};")
