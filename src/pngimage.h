#pragma once

#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <png.h>
#include <sys/stat.h>

using namespace std;

#define SIGNATURE_SIZE 8
#define mirrorAxisX 1
#define mirrorAxisY 0

struct PixelPainted
{
	// Цвет окраски пикселя в формате RGBA
	png_byte r, g, b, a;
};


/**
* Класс для считывания, записи и обработки png изображения
**/
class PNGImage
{
public:
	png_int_32 width = 0; // Ширина изображения
	png_int_32 height = 0; // Высота изображения
	png_byte color_type = 0; // Цветовая схема изображения
	png_byte bit_depth = 0;	 // Глубина цвета изображения
	int number_of_passes = 0; // Количество проходов
	bool trd_added = false; // Флаг, что добавлены мусорные данные
	struct stat file_info; // Свойства изображения

	void read_image(const char *filename); // Функция считывания png изображения из файла
	void write_image(const char *filename); // Функция записи png изображения в файл
	void get_file_info(const char *filename); // Функция получения информации о файле
	~PNGImage();

	// Функции обработки изображения
	/* Отражение заданной области */
	void mirror(int axis, png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1);
	/* Рисование пентаграммы */
	void draw_pentagram(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1, int thickness, PixelPainted *color);
	void draw_pentagram(png_int_32 cx, png_int_32 cy, int r, int thickness, PixelPainted *color);
	/* Рисование прямоугольника */
	void draw_rectangle(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1, int thickness, PixelPainted *color);
	void draw_rectangle(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1, int thickness, PixelPainted *line_color, PixelPainted *fill_color);
	/* Рисование правильного шестиугольника */
	void draw_hexagon(png_int_32 cx, png_int_32 cy, int r, int thickness, PixelPainted *color);
	void draw_hexagon(png_int_32 cx, png_int_32 cy, int r, int thickness, PixelPainted *line_color, PixelPainted *fill_color);
	void draw_hexagon(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1, int thickness, PixelPainted *color);
	void draw_hexagon(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1, int thickness, PixelPainted *line_color, PixelPainted *fill_color);

	// void reserved(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1);
private:
	// Структуры для считывания, записи и хранения png изображения
	png_structp png_ptr = nullptr;
	png_infop info_ptr = nullptr;
	png_infop end_ptr = nullptr;
	png_bytep *rows = nullptr; // Массив байтов пикселей
	int bytes_per_pixel = 3; // Количество байтов, соотвествующих одному пикселю, по умолчанию 3 (соответствует формату RGB)

	// Вспомогательные функции для считывания и записи png изображения
	void create_structs(FILE *file);
	void init_io(FILE *file);
	void __read_image(FILE *file);
	void __write_image(FILE *file);
	void __add_trash_data();

	// Вспомогательные функции для обработки изображения
	void _draw_line(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1, int thickness, PixelPainted *color);
	void _normalize_coords(png_int_32 *x0, png_int_32 *y0, png_int_32 *x1, png_int_32 *y1);
	void _set_pixel(png_int_32 x, png_int_32 y, PixelPainted *color);
	void _draw_circle(png_int_32 cx, png_int_32 cy, int r, int thickness, PixelPainted *color);
	void _draw_circle_thickness(png_int_32 cx, png_int_32 cy, int r, int thickness, PixelPainted *color);
};
