# Losless compression of Grayscale images using Huffman Coding in OpenCV

### About

The project implements the lossless compression algorithm named Huffman coding for a
grayscale image. Then the compressed image is saved as a binary file. The application is able to
load a previously saved binary file and construct the original image without quality loss(that&#39;s
why it is called lossless). Finally the compression ratio is calculated.

### Motivation

The code for a pixel in a Grayscale image(8 bit depth) has a constant size of 8 because there
are a possible 2 8 values. But for images, where only some intensities are used, the image size
becomes large. In this case Huffman encoding can be used to reduce the image size. This helps
in the reduction of disk storage. Or if many images are manipulated in the RAM, then it helps to
reduce the amount of RAM needed(but as a disadvantage in this case the manipulation
becomes CPU intensive……).
