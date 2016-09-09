//============================================================================
// Name        : gif_reader.cpp
// Author      : fadedreamz
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstdio>
#include "unity.h"
using namespace std;

typedef struct __attribute__((packed)) _gif_data_block {
	unsigned char size;
	unsigned char data[0];
}gif_data_block;

typedef struct __attribute__((packed)) _gif_graphics_ctrl_ext {
	unsigned char block_size;
	unsigned char reserved : 3;
	unsigned char disposal_method : 3;
	unsigned char user_input_flag : 1;
	unsigned char transparent_color_flag : 1;
	unsigned short delay_time;
	unsigned char transparent_color_index;
	gif_data_block block_terminator;
}gif_graphics_ctrl_ext;

typedef struct __attribute__((packed)) _gif_app_ext {
	unsigned char size;
	unsigned char app_identifier[8];
	unsigned char app_auth_code[3];
}gif_app_ext;

typedef struct __attribute__((packed)) _gif_header {
	char ID[3];
	char VERSION[3];
}gif_header;

typedef struct __attribute__((packed)) _rgb {
	unsigned char r;
	unsigned char g;
	unsigned char b;
}rgb;

typedef struct __attribute__((packed)) _gif_image_descriptor {
	unsigned short top;
	unsigned short left;
	unsigned short width;
	unsigned short height;
	unsigned char color_table_size : 3;
	unsigned char reserved : 2;
	unsigned char sorted : 1;
	unsigned char interlaced : 1;
	unsigned char table_enabled : 1;
}gif_image_descriptor;

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
//	int size = 1;
//		for(int i=0; i<= sd->global_color_table_size; i++)
//			size = size << 1;
//		size = 3 * size;
	return 3 *(1 << (sd->global_color_table_size + 1));
//	return size;
}

unsigned int get_local_table_size(gif_image_descriptor * id) {
	return 3 *(1 << (id->color_table_size + 1));
}

void test_get_global_table_size() {
	gif_screen_descriptor sd;
	sd.global_color_table_size = 7;
	TEST_ASSERT_EQUAL(get_global_table_size(&sd), 768);
}

rgb * create_global_color_table(gif_screen_descriptor * sd) {
	return new rgb[get_global_table_size(sd)];
}

rgb * create_local_color_table(gif_image_descriptor * id) {
	return new rgb[get_local_table_size(id)];
}

void print_screen_descriptor_info(gif_screen_descriptor * sd) {
	cout << "width = " << sd->width << ", height = " << sd->height << endl;
	cout << "Global table " << (sd->global_table_enabled == 1 ? "enabled" : "disabled") << endl;
	cout << "No of color bits " << sd->color_resolution + 1 << endl;
	cout << "Sorting " << (sd->sorted == 1 ? "enabled" : "disabled") << endl;
	cout << "Global table size " << (int)sd->global_color_table_size << endl;
}

void print_image_descriptor_info(gif_image_descriptor * id) {
	cout << "===== Image Descriptor =====" << endl;
	cout << "width = " << id->width << ", height = " << id->height << endl;
	cout << "Local color table " << (id->table_enabled == 1 ? "enabled" : "disabled") << endl;
	cout << "Sorting " << (id->sorted == 1 ? "enabled" : "disabled") << endl;
	cout << "Local table size " << (int)id->color_table_size + 1 << endl;
}

int main() {
	FILE * file = fopen("icon.gif", "rb");
	char header[7];
	rgb * global_table = NULL;
	rgb * local_table = NULL;
	gif_screen_descriptor sd;
	cout << "data subblock size : " << sizeof(gif_data_block) << endl;
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
				if (sd.global_table_enabled) {
					global_table = create_global_color_table(&sd);
					fread(global_table, 1, get_global_table_size(&sd), file);
					cout << "file read size : " << ftell(file) << endl;
				}
				fread(identifier, 1, 1, file);
				if (identifier[0] == 0x21) {
					fread(identifier, 1, 1, file);
					if (identifier[0] == (char)0xff) {
						gif_app_ext ext;
						char datablock[257];
						gif_data_block * block = (gif_data_block *)datablock;
						cout << "application extension detected" << endl;
						fread(&ext, 1, sizeof(ext), file);
						fread(block, 1, sizeof(gif_data_block), file);
						while(block->size != 0) {
							fread(block->data, 1, block->size, file);
							fread(block, 1, sizeof(gif_data_block), file);
						}
					}
				}
				fread(identifier, 1, 1, file);
				if (identifier[0] == (char)0x21) {
					fread(identifier, 1, 1, file);
					if (identifier[0] == (char)0xf9) {
						gif_graphics_ctrl_ext ext;
						cout << "graphics control extension detected" << endl;
						fread(&ext, 1, sizeof(ext), file);
					}
				}
				fread(identifier, 1, 1, file);
				if (identifier[0] == (char)0x2c) {
					gif_image_descriptor des;
					cout << "image descriptor found" << endl;
					fread(&des, 1, sizeof(des), file);
					print_image_descriptor_info(&des);
					if (des.table_enabled == 1) {
						local_table = create_local_color_table(&des);
						fread(local_table, 1, get_local_table_size(&des), file);
					}
					fread(identifier, 1, 1, file);
					cout << "LWZ compression bit is : " << (int) identifier[0] << endl;
				}

				if (global_table != NULL)
					delete [] global_table;
				if (local_table != NULL)
					delete [] local_table;
			}
		}
		fclose(file);
	}

	cout << endl;
	UNITY_BEGIN();
	RUN_TEST(test_get_global_table_size);
	UNITY_END();
	return 0;
}
