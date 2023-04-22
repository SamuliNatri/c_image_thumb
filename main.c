
// Image resizing with stb_image_resize.h

// Note: this is a basic test. Works only for one uploaded PNG. Multipart data is not handled (boundary etc).

// Usage:
// cc -g main.c -lm && ./a.out 
// curl -F "file=@image.png" localhost:8080

#include "web.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

void upload_image(Request *request) {

	// Find image data start position.
	
	char *header_start_string = "Content-Type: image/png";
	char *header_start = strstr(request->value, header_start_string);

	if(header_start != NULL) {
		
		char *image_start = header_start;
		image_start = image_start + (strlen(header_start_string) + strlen("\r\n\r\n"));

		// Find image data end.

		char *image_end = image_start;
		char image_end_bytes[] = "IEND";

		// Hacky thing for finding PNG end bytes.
		// We can't use strstr() because there might be null bytes.
		// memmem might not be available either.
		// image_end = memmem(request->value, request->size, image_end_bytes, strlen(image_end_bytes));  //image_end += strlen(image_end_bytes);		
		while(!(
					*image_end == 'I' &&
					*(image_end+1) == 'E' &&
					*(image_end+2) == 'N' &&
					*(image_end+3) == 'D')
				) ++image_end;
		image_end += 4;

		size_t image_size = image_end - image_start;

		// Load image data from the request buffer.
		
		int image_width = 0;
		int image_height = 0;
		int image_channels = 0;

		unsigned char *image_data = 
			stbi_load_from_memory((unsigned char*)image_start, image_size, &image_width, &image_height, &image_channels, 0);

		if (image_data == NULL) {
			printf("Error loading image: %s\n", 
					stbi_failure_reason());
		} else {
			printf("Loading image was successful.\n");

			// Write it as a JPEG to disk.
			
			stbi_write_jpg("output.jpg", image_width, image_height, image_channels, image_data, 100);

			// Create a smaller image.
			
			int new_width = image_width / 2;
			int new_height = image_height / 2;
			int new_channels = image_channels;
			unsigned char* output_thumb = malloc(new_width * new_height * new_channels);
			
			stbir_resize_uint8(image_data, image_width, image_height, 0, output_thumb, new_width, new_height, 0, image_channels);
			
			// Write the smaller image to disk.

			stbi_write_jpg("output_thumb.jpg", new_width, new_height, new_channels, output_thumb, 100);
		}

		// Store the original uploaded PNG as PNG to disk.

		FILE *output_orig = fopen("output_orig.png", "wb");
		size_t bytes_written = fwrite(image_start, sizeof(char), image_size, output_orig);

		if(bytes_written != image_size) {
			printf("Error saving file.\n");
		} else {
			printf("File saved successfully.\n");
		}
			
	fclose(output_orig);

	} // if header_start != NULL

}

void w_handle_request(Request *request) {
	printf("Handling request.\n");
	printf("\n\n** request.value: \n\n[%s]\n\n", request->value);
	upload_image(request);
}

void w_build_response(char *response) {
	char resp_temp[] = 
		"HTTP/1.1 200 OK\r\n"
		"ContentlType: text/html\r\n"
		"Content-Length: 4\r\n\r\n"
		"Yes!";
	memcpy(response, resp_temp, strlen(resp_temp));
}
