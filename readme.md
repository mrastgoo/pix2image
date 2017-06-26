#pix2image 

## Description

pix2image script converts the raw images captured with DoFP polar camera to :

* angle images (I0, I45, I135, I90)
* stokes image (S0, S1, S2)
* polar images (AoP, DoP)

##  Compilation

1. cmake .
2. make 

## Use

You need to specify:

* method : `angle`, `stokes`, or `polar`
* the path where the images are stored and the initial name of the image with their extention:
	* `Path/Name*.extention`

* the path to save the images 

```
	./pix2image 'method [angle, stokes, polar]' 'PathtoLoad/' 'PathtoSave'

```

### example 

```
	./pix2image 'angle' 'home/Dataset/exp1/image*.tiff' 'home/Dataset/exp1-angle/'

```



