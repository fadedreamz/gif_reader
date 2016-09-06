//============================================================================
// Name        : gif_reader.cpp
// Author      : fadedreamz
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstdio>
using namespace std;

typedef struct __attribute__((packed)) _gif_data_subblock {
	unsigned char size;
	unsigned char data[0];
}gif_data_subblock;

typedef struct __attribute__((packed)) _gif_header {
	char ID[3];
	char VERSION[3];
}gif_header;

typedef struct __attribute__((packed)) _rgb {
	unsigned char r;
	unsigned char g;
	unsigned char b;
}rgb;

typedef struct __attribute__((packed)) _gif_screen_descriptor {
	unsigned short width;
	unsigned short height;
	unsigned char global_color_table_size : 3;
	unsigned char sorted : 1;
	unsigned char color_resolution : 3;
	unsigned char global_table_enabled : 1;
	char bg_color_index;
	char aspect_ratio;
}gif_screen_descriptor;

unsigned int get_global_table_size(gif_screen_descriptor * sd) {
	int size = 1;
		for(int i=0; i<= sd->global_color_table_size; i++)
			size = size << 1;
		size = 3 * size;
	return size;
}

rgb * create_global_color_table(gif_screen_descriptor * sd) {
	return new rgb[get_global_table_size(sd)];
}

void print_screen_descriptor_info(gif_screen_descriptor * sd) {
	cout << "width = " << sd->width << ", height = " << sd->height << endl;
	cout << "Global table " << (sd->global_table_enabled == 1 ? "enabled" : "disabled") << endl;
	cout << "No of color bits " << sd->color_resolution + 1 << endl;
	cout << "Sorting " << (sd->sorted == 1 ? "enabled" : "disabled") << endl;
	cout << "Global table size " << (int)sd->global_color_table_size << endl;
}

int main() {
	FILE * file = fopen("icon.gif", "rb");
	char header[7];
	rgb * global_table;
	gif_screen_descriptor sd;
	cout << "data subblock size : " << sizeof(gif_data_subblock) << endl;
	if (file != NULL) {
		size_t count = fread(header, 1, 6, file);
		if (count < 6) {
			cout << "unable to read header" << endl;
		} else {
			header[6] = '\0';
			cout << "gif version : " << header << ", sd size : " << sizeof(sd) << endl;
			count = fread(&sd, 1, sizeof(sd), file);
			if (count >= sizeof(sd)) {
				char identifier[2];
				cout << "file read size : " << ftell(file) << endl;
				print_screen_descriptor_info(&sd);
				global_table = create_global_color_table(&sd);
				fread(global_table, 1, get_global_table_size(&sd), file);
				cout << "file read size : " << ftell(file) << endl;
				fread(identifier, 1, 2, file);
				if (identifier[0] == 0x21 && identifier[1] == 0xFF) {
					cout << "application extension detected" << endl;
				}
				delete [] global_table;
			}
		}
		fclose(file);
	}
	return 0;
}
