# HidePNG

Stores arbitrary files into PNG images.

```
$ hidepng encode image.png secretfile.txt   #can be repeated to store more than 1 file in an image
$ hidepng decode image.png   #creates the hidden files in the working directory
```

It stores these files in ancillery PNG chunks with the tag xtRa.