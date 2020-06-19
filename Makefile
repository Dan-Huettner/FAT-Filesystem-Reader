all: readfat

readfat: readfat.c error/* file_system/* storage_device/* user_interface/*
	gcc ./readfat.c ./error/*.c ./file_system/*.c ./storage_device/*.c ./user_interface/*.c -I./error -I./file_system -I./storage_device -I./user_interface -o readfat

.PHONY: all clean

clean:
	rm -f readfat

