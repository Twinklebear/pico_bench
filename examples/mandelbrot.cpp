#include <fstream>
#include <array>
#include <vector>
#include <iostream>
#include <pico_bench.h>

/*
 * Convenient wrapper for BMP header information for a 32bpp BMP
 */
#pragma pack(1)
struct BMPHeader {
	const uint8_t header[2] = {'B', 'M'};
	const uint32_t file_size;
	//4 reserved bytes we don't care about
	const uint32_t dont_care = 0;
	//Offset in the file to the pixel array
	const uint32_t px_array = 54;
	const uint32_t header_size = 40;
	const std::array<int32_t, 2> dims;
	const uint16_t color_planes = 1;
	const uint16_t bpp = 24;
	const uint32_t compression = 0;
	const uint32_t img_size;
	const int32_t res[2] = {2835, 2835};
	const uint32_t color_palette = 0;
	const uint32_t important_colors = 0;

	BMPHeader(uint32_t img_size, int32_t w, int32_t h)
		: file_size(54 + img_size), dims{w, h}, img_size(img_size)
	{}
};

bool save_bmp(const std::string &file, size_t w, size_t h, const uint8_t *data);
void mandelbrot(size_t w, size_t h, std::vector<uint8_t> &data);

int main(int, char**){
	const size_t w = 128;
	const size_t h = 128;
	std::vector<uint8_t> image(w * h * 3, 0);
	pico_bench::Benchmarker bencher{100, std::chrono::seconds{5}};
	const auto stats = bencher([&image](){
		mandelbrot(w, h, image);
	});
	std::cout << "Mandelbrot " << stats << "\n";
	save_bmp("mandelbrot.bmp", w, h, image.data());
	return 0;
}

void mandelbrot(size_t w, size_t h, std::vector<uint8_t> &data){
	const size_t MAX_ITER = 100;
	for (size_t p = 0; p < w * h; ++p){
		size_t x = p / w;
		size_t y = p % h;
		float z_real = 0;
		float z_imag = 0;
		float c_real = (x - w / 1.4f) / (w / 2.f);
		float c_imag = (y - h / 2.f) / (h / 2.f);
		size_t k = 0;
		float length_sq = 0;
		do {
			float tmp = std::pow(z_real, 2.f) - std::pow(z_imag, 2.f) + c_real;
			z_imag = 2.f * z_real * z_imag + c_imag;
			z_real = tmp;
			length_sq = std::pow(z_real, 2.f) - std::pow(z_imag, 2.f);
			++k;
		}
		while (length_sq < 4.f && k < MAX_ITER);

		if (k == MAX_ITER){
			k = 0;
		}
		float intensity = k / static_cast<float>(MAX_ITER);
		uint8_t color = static_cast<uint8_t>(intensity * 255.f);
		data[y * w * 3 + x * 3] = color;
		data[y * w * 3 + x * 3 + 1] = color;
		data[y * w * 3 + x * 3 + 2] = color;
	}
}
bool save_bmp(const std::string &file, size_t width, size_t height, const uint8_t *data){
	FILE *fp = fopen(file.c_str(), "wb");
	if (!fp){
		std::cerr << "RenderTarget::save_bmp Error: failed to open file "
			<< file << std::endl;
		return false;
	}
	uint32_t w = width, h = height;
	BMPHeader bmp_header{4 * w * h, static_cast<int32_t>(w),
		static_cast<int32_t>(h)};
	if (fwrite(&bmp_header, sizeof(BMPHeader), 1, fp) != 1){
		fclose(fp);
		return false;
	}
	// Write each row follwed by any necessary padding
	size_t padding = (w * 3) % 4;
	for (size_t r = 0; r < h; ++r){
		if (fwrite(data + 3 * w * r, 1, 3 * w, fp) != 3 * w){
			fclose(fp);
			return false;
		}
		if (padding != 0){
			if (fwrite(data, 1, padding, fp) != padding){
				fclose(fp);
				return false;
			}
		}

	}
	fclose(fp);
	return true;
}

