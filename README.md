# Image upload and resizing test with stb libraries

- main.c : image handling code.
- web.h : minimal web server.

## Usage:

```
cc -g main.c -lm
./a.out
curl -F "file=@image.png" localhost:8080
```

Creates these files:

- output_orig.png: Original image stored directly from the request buffer as PNG.
- output.jpg: Image stored as JPEG.
- output_thumb.jpg: Image resized and stored as JPEG.
