# FAT Filesystem Reader

## Overview
This is a program I implemented on my own time to read from a storage device with a FAT12 or FAT32 filesystem.  This program will read the contents and output a list of all the files in the volume along with the following information:
* File name
* File type (i.e. regular file or directory)
* Size (in Bytes)
* Sequence of cluster addresses comprising the file, as read from the File Allocation Table of the device.

## What Does It Support?
I have implemented support for:
* FAT12 and FAT32 file systems.
* Long file names.

## 3-Tiered Organizational Structure
This program has been designed using a 3-tiered organizational structure:
user_interface
      |
	  V
 file_system
      |
	  V
storage_device

The functions defined in one layer can only call functions in the same layer or in the layer directly below.  The only exception is that any function can call the handleError function defined in error/error.h.

## Sample Output
There are two sample files included with this program:
* sample_output.txt:  The output from running the program on the "sample_output_source_image.dat" image file.
* sample_output_source_image.dat:  An image of a sample FAT12 filesystem, on which the program can be run.

## Compiling the Program
A makefile has been provided.  Hence, to compile the program, simply run
```bash
make
```
and voila!

## Running the Program
Assuming your currently in the directory containing the readfat executable file and a readable FAT image, the command to run it is:
	./readfat file_name.dat

You may not need the "./" before the readfat file name, depending on whether or not the current working directory (.) is in your PATH environment variable.


## The "more" Command
Since this program produces a lot of output, I recommend piping the output into the 'more' command as follows:
	./readfat file_name.dat | more

## System Requirements
A recent version of Ubuntu, or likely almost any other UNIX-like operating system, should work just fine.