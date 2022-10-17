#include "pngimage.h"
#include <cmath>
#include <cstring>


/**
* Функция считывания изображения из файла
**/
void PNGImage::read_image(const char *filename)
{
	FILE *file = fopen(filename, "rb"); // Открытие файла на запись в бинарном режиме
	if (!file)
		throw runtime_error{"cannot open the file"};

	// Вызов функций для считывания изображения
	create_structs(file);
	init_io(file);
	__read_image(file);

	png_destroy_read_struct(&png_ptr, NULL, NULL);
	png_ptr = nullptr;
	get_file_info(filename);

	fclose(file);
}

/**
* Функция сохранения изображения в файл
**/
void PNGImage::write_image(const char *filename)
{
	FILE *file = fopen(filename, "wb+"); // Открытие файла на запись в бинарном режиме
	if (!file)
		throw runtime_error{"cannot open the file"};

	// Запись изображения и заголовков в файл
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		fclose(file);
		throw runtime_error{"png structure was not allocated"};
	}
	if (setjmp(png_jmpbuf(png_ptr))) // Обработка ошибок
	{
		png_destroy_write_struct(&png_ptr, NULL);
		throw runtime_error{"I/O setup failed"};
	}
	png_init_io(png_ptr, file);	
	__write_image(file);

	png_destroy_write_struct(&png_ptr, NULL);
	png_ptr = nullptr;

	fclose(file);
}

/**
* Проверка файла на соответствие формату png
* Подготовка структур для заголовков изображения
**/
void PNGImage::create_structs(FILE *file)
{
	png_byte buffer[SIGNATURE_SIZE] = {0};
	fread(buffer, 1, SIGNATURE_SIZE, file);  // Считывание сигнатуры
	if (png_sig_cmp(buffer, 0, SIGNATURE_SIZE)) // Проверка соответствия сигнатуры сигнатуре PNG файла
	{
		fclose(file);
		throw runtime_error{"not a png file"};
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL); // Инициализация структуры для считывания изображения
	if (!png_ptr) // Проверка инициализации структуры
	{
		fclose(file);
		throw runtime_error{"png structure was not allocated"};
	}

	info_ptr = png_create_info_struct(png_ptr); // Инициализация структуры для считывания заголовка IHDR
	if (!info_ptr) // Проверка инициализации структуры
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(file);
		throw runtime_error{"info structure was not allocated"};
	}

	end_ptr = png_create_info_struct(png_ptr); // Инициализация структуры для считывания заголовка IEND
	if (!end_ptr) // Проверка инициализации структуры
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(file);
		throw runtime_error{"info structure was not allocated"};
	}
}


/**
* Подготовка и считывание заголовков изображения
* Изменение некоторых заголовков для поддержки обработки формата
* Установка необходимых для работы значений (bytes_per_pixel)
**/
void PNGImage::init_io(FILE *file)
{
	if (setjmp(png_jmpbuf(png_ptr))) // Обработка ошибок
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		throw runtime_error{"I/O setup error"};
	}

	// Инициализация I/O и считывание заголовка IHDR
	png_init_io(png_ptr, file);
	png_set_sig_bytes(png_ptr, SIGNATURE_SIZE);
	png_read_info(png_ptr, info_ptr);

	// Получение параметров изображения
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	// Корректировка свойств изображения и установка небходимых значений
	if (color_type == PNG_COLOR_TYPE_PALETTE) // Конвертация схемы palette в rgb
		png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) // Принудительное увеличение глубины цвета монохромного изображения
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) // Преобразование схемы rgb в rgba при наличии чанка tRNS
		png_set_tRNS_to_alpha(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY) // Если изображение монохромное без альфа-канала, на один пиксель отводится 1 байт
		bytes_per_pixel = 1;
	if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) // Если изображение монохромное с альфа-каналом, на один пиксель отводится 2 байта
		bytes_per_pixel = 2;
	if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) // Если цветовая схема rgba, на один пиксель отводится 4 байта
		bytes_per_pixel = 4;
	if (bit_depth == 16) // Уменьшение глубины цвета до 8
		png_set_strip_16(png_ptr);
	else if (bit_depth < 8) // Увеличение глубины цвета до 8
		png_set_packing(png_ptr);
	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr); // Обновление структур и применение преобразований
}		

/**
* Считывание заголовков изображения после их обновления
* Считывание байтов, отвечающих за пиксели
* Сохранение параметров изображения (ширина, высота, тип цветовой схемы, глубина цвета) 
**/
void PNGImage::__read_image(FILE *file)
{
	// Получение информации о параметрах изображения
	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	if (setjmp(png_jmpbuf(png_ptr))) // обработка ошибок
	{
		fclose(file);
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		throw runtime_error{"read error"};
	}

	rows = new png_bytep[height];
	for (int i = 0; i < height; i++)
		rows[i] = new png_byte[png_get_rowbytes(png_ptr, info_ptr)];

	png_read_image(png_ptr, rows); // считывание байтов пикселей
	png_read_end(png_ptr, end_ptr); // считывание заголовка IEND
}

/**
* Запись в файл изображения и его заголовков
**/
void PNGImage::__write_image(FILE *file)
{
	if (setjmp(png_jmpbuf(png_ptr))) // Обработка ошибок
	{
		fclose(file);
		png_destroy_write_struct(&png_ptr, NULL);
		throw runtime_error{"Info header write failed"};
	}
	png_write_info(png_ptr, info_ptr); // Запись в файл заголовка IHDR

	if (setjmp(png_jmpbuf(png_ptr))) // Обработка ошибок
	{
		fclose(file);
		png_destroy_write_struct(&png_ptr, NULL);
		throw runtime_error{"Image data write failed"};
	}
	__add_trash_data();
	png_write_image(png_ptr, rows); // Запись в файл байтов пикселей

	if (setjmp(png_jmpbuf(png_ptr))) // Обработка ошибок
	{
		fclose(file);
		png_destroy_write_struct(&png_ptr, NULL);
		throw runtime_error{"End data write failed"};
	}
	png_write_end(png_ptr, end_ptr); // Запись в файл заголовка IEND
}

/**
* Запись в строки мусорных данных, в случае необходимости
**/
void PNGImage::__add_trash_data()
{
	if (trd_added) return; // Проверка, не записывались ли мусорные данные ранее

	int trd = width * bytes_per_pixel % 4;
	if (trd == 0) // Проверка необходимости записи мусорных данных
	{
		trd_added = true;
		return;
	}

	int row_sz = width * bytes_per_pixel + (4 - trd);
	int fixed_sz = width * bytes_per_pixel;

	for (png_int_32 i = 0; i < height; i++)
	{
		// Перевыделение памяти и копирование имеющихся данных
		png_bytep _row = new png_byte[row_sz]; 
		memcpy(_row, rows[i], fixed_sz * sizeof(png_byte));
		for (int j = 0; j < (4 - trd); j++)
			_row[fixed_sz + j + 1] = 0;
		delete [] rows[i];
		rows[i] = _row;
	}
	trd_added = true;
}

/**
* Получение данных о файле
**/
void PNGImage::get_file_info(const char *filename)
{
	stat(filename, &file_info);
}

PNGImage::~PNGImage()
{
	// Очистка памяти под структуры и байты пикселей
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	for (int i = 0; i < height; i++)
		delete [] rows[i];
	delete [] rows;
}

/**
* Функция преобразует координаты углов прямоугольника в координаты его левого верхнего и правого нижнего углов
**/
void PNGImage::_normalize_coords(png_int_32 *x0, png_int_32 *y0, png_int_32 *x1, png_int_32 *y1)
{
	png_int_32 swp;
	if (*x0 > *x1)
	{
		swp = *x1;
		*x1 = *x0;
		*x0 = swp;
	}
	if (*y0 > *y1)
	{
		swp = *y1;
		*y1 = *y0;
		*y0 = *y1;
	}
}

/**
* Функция установки определенного цвета для пикселя
* В зависимости от типа цветовой схемы (количества байтов на пиксель) происходит окраска пикселя
* Для монохромных изображений используется окраска исходя из среднего значения каналов цвета, в который окрашивается пиксель
**/
void PNGImage::_set_pixel(png_int_32 x, png_int_32 y, PixelPainted *color)
{
	if (x < 0 || y < 0 || x >= width || y >= height) return;
	switch (bytes_per_pixel)
	{
		case 1: // Монохромное изображение без альфа-канала
			rows[y][x * bytes_per_pixel + 0] = (png_byte)((color->r + color->g + color->b) / 3);
			break;
		case 2: // Монохромное изображение с альфа-каналом
			rows[y][x * bytes_per_pixel + 0] = (png_byte)((color->r + color->g + color->b) / 3);
			rows[y][x * bytes_per_pixel + 1] = color->a;
			break;
		case 3: // Цветовая схема RGB
			rows[y][x * bytes_per_pixel + 0] = color->r;
			rows[y][x * bytes_per_pixel + 1] = color->g;
			rows[y][x * bytes_per_pixel + 2] = color->b;
			break;
		case 4: // Цветовая схема RGBA
			rows[y][x * bytes_per_pixel + 0] = color->r;
			rows[y][x * bytes_per_pixel + 1] = color->g;
			rows[y][x * bytes_per_pixel + 2] = color->b;
			rows[y][x * bytes_per_pixel + 3] = color->a;
			break;
	}
}

/**
* Функция для отражения заданной области относительно заданной оси
**/
void PNGImage::mirror(int axis = mirrorAxisX, png_int_32 x0 = 0, png_int_32 y0 = 0, png_int_32 x1 = 0, png_int_32 y1 = 0)
{
	_normalize_coords(&x0, &y0, &x1, &y1); // нормализация координат
	png_int_32 x, y, i;
	png_byte tmp;
	if (x1 == 0 && y1 == 0)
	{
		x1 = width;
		y1 = height;
	}
	if (x0 < 0 || x0 > width) x0 = 0;
	if (y0 < 0 || y0 > height) y0 = 0;
	if (x1 > width || x1 < 0) x1 = width;
	if (y1 > height || y1 < 0) y1 = height;
	if (axis == mirrorAxisX) // Отражение относительно оси X
	{
		for (y = y0; y < (y0 + y1) / 2; y++)
		{
			for (x = x0; x < x1; x++)
			{
				for (i = 0; i < bytes_per_pixel; i++) // Смещение всех байтов, относящихся к данным пикселям
				{
					tmp = rows[y][x * bytes_per_pixel + i];
					rows[y][x * bytes_per_pixel + i] = rows[y1+y0-y-1][x * bytes_per_pixel + i];
					rows[y1+y0-y-1][x * bytes_per_pixel + i] = tmp;
				}
			}
		}
	} else if (axis == mirrorAxisY)
	{
		for (y = y0; y < y1; y++)
		{
			for (x = x0; x < (x0 + x1) / 2; x++)
			{
				for (i = 0; i < bytes_per_pixel; i++) // Смещение всех байтов, относящихся к данным пикселям
				{
					tmp = rows[y][x * bytes_per_pixel + i];
					rows[y][x * bytes_per_pixel + i] = rows[y][(x1+x0-x-1) * bytes_per_pixel + i];
					rows[y][(x1+x0-x-1) * bytes_per_pixel + i] = tmp;
				}
			}
		}
	}
}

/**
* Функция для рисования пентаграммы
* Принимает координаты центра окружности пентаграммы, ее радиус, толщину линий и цвет линий
**/
void PNGImage::draw_pentagram(png_int_32 cx, png_int_32 cy, int r, int thickness, PixelPainted *color)
{
	_draw_circle(cx, cy, r, thickness, color); // Рисование окружности пентаграммы

	float side = r/sqrt(0.5 + sqrt(5)/10); // Длина линий пентаграммы
	int halfSide = round(side/2); // Округленная половина длины линии пентаграммы
	int fSide = round(side * 1.55 / 2); // Вспомогательная линия
	int thOffset = (int) thickness / 2.5; // Оступ исходя из толщины линии
	int offsetY = round(sqrt(4*r*r - side*side)/2); // Оступ по y для определения положения горизонтальной линии пентаграммы
	// Рисование линий по вычисленным координатам
	_draw_line(cx, cy-r, cx+halfSide, cy+offsetY, thickness, color);
	_draw_line(cx, cy-r, cx-halfSide, cy+offsetY, thickness, color);
	
	_draw_line(cx-fSide-thOffset, cy-(halfSide*2-offsetY)-thOffset, cx+halfSide, cy+offsetY, thickness, color);
	_draw_line(cx+fSide+thOffset, cy-(halfSide*2-offsetY)-thOffset, cx-halfSide, cy+offsetY, thickness, color);
	_draw_line(cx+fSide, cy-(halfSide*2-offsetY), cx-fSide, cy-(halfSide*2-offsetY), (int)thickness*1.8, color);
}

/**
* Функция для рисования пентаграммы
* Принимает координаты прямоугольной области, в которую вписана пентаграмма, толщину линий и цвет линий
* В случае, если одна из сторон длиннее другой, пентаграмма вписывается в левый верхний угол области
**/
void PNGImage::draw_pentagram(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1, int thickness, PixelPainted *color)
{
	_normalize_coords(&x0, &y0, &x1, &y1); // Нормализация координат
	int side; // Длина стороны квадрата, в который вписана пентаграмма
	int radius; // Радиус окружности пентаграммы
	if (x1 - x0 > y1 - y0)
		side = y1 - y0;
	else 
		side = x1 - x0;
	radius = (int) side / 2;
	draw_pentagram(x0+radius, y0+radius, radius, thickness, color); // Отрисовка пентаграммы
}

/**
* Функция рисования прямоугольника (без заливки)
* Принимает координаты прямоугольника, толщину линий и цвет линий
**/
void PNGImage::draw_rectangle(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1, int thickness, PixelPainted *color)
{
	_normalize_coords(&x0, &y0, &x1, &y1); // Нормализация координат
	int thOffset = (int) (thickness + 1) / 2; // Отступ исходя из толщины линий
	// Рисование линий
	_draw_line(x0, y0-thOffset+1, x0, y1, thickness, color);
	_draw_line(x0, y0, x1, y0, thickness, color);
	_draw_line(x0, y1, x1, y1, thickness, color);
	_draw_line(x1, y0, x1, y1, thickness, color);
}

/**
* Функция рисования прямоугольника (с заливкой)
* Принимает координаты прямоугольника, толщину линий и цвет линий, цвет заливки
**/
void PNGImage::draw_rectangle(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1, int thickness, PixelPainted *line_color, PixelPainted *fill_color)
{
	_normalize_coords(&x0, &y0, &x1, &y1); // Нормализация координат
	png_int_32 x, y;
	int thOffset = (int) (thickness + 1) / 2; // Отступ исходя из толщины линий
	// Закраска области прямоугольника
	for (y = y0; y < y1-thOffset+1; y++)
	{
		for (x = x0; x < x1-thOffset+1; x++)
			_set_pixel(x, y, fill_color);
	}
	draw_rectangle(x0, y0, x1, y1, thickness, line_color); // Рисование линий прямоугольника
}

/**
* Функция рисования правильного шестиугольника (без заливки)
* Принимает координаты центра окружности, в которую вписан шестиугольник, ее радиус, толщину линий и цвет линий
**/
void PNGImage::draw_hexagon(png_int_32 cx, png_int_32 cy, int r, int thickness, PixelPainted *color)
{
	int halfR = (int) r / 2; // Половина радиуса описанной окружности
	int offsetY = (int) round(r * sqrt(3) / 2); // Оступ по y для определения положения линий

	int thOffset = (int) thickness / 5.2; // Отступ исходя из толщины линий
	// Рисование линий
	_draw_line(cx-halfR, cy-offsetY+thOffset, cx+halfR, cy-offsetY+thOffset, round(thickness*2.4), color);
	_draw_line(cx-halfR, cy+offsetY+thickness, cx+halfR, cy+offsetY+thickness, round(thickness*2.4), color);

	_draw_line(cx-halfR, cy-offsetY-thickness, cx-r, cy, thickness, color);
	_draw_line(cx+halfR, cy-offsetY-thickness, cx+r, cy, thickness, color);
	_draw_line(cx-halfR, cy+offsetY+thickness, cx-r, cy, thickness, color);
	_draw_line(cx+halfR, cy+offsetY+thickness, cx+r, cy, thickness, color);
}

/**
* Функция рисования правильного шестиугольника (без заливки)
* Принимает координаты прямоугольной области, в которую вписан шестиугольник, толщину линий и цвет линий
**/
void PNGImage::draw_hexagon(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1, int thickness, PixelPainted *color)
{
	_normalize_coords(&x0, &y0, &x1, &y1); // Нормализация координат
	int side; // Длина стороны квадрата, в который вписан шестиугольник
	int radius; // Радиус окружности
	if (x1 - x0 > y1 - y0)
		side = y1 - y0;
	else 
		side = x1 - x0;
	radius = (int) side / 2;
	draw_hexagon(x0+radius, y0+radius, radius, thickness, color); // Рисование шестиугольника
}

/**
* Функция рисования правильного шестиугольника (с заливкой)
* Принимает координаты центра окружности, в которую вписан шестиугольник, ее радиус, толщину линий и цвет линий, цвет заливки
**/
void PNGImage::draw_hexagon(png_int_32 cx, png_int_32 cy, int r, int thickness, PixelPainted *line_color, PixelPainted *fill_color)
{
	png_int_32 x, y, startX; 
	int offsetY = (int) round(r * sqrt(3) / 2); // Отступ по y для определения положения линий
	for (y = cy-offsetY; y < cy+1; y++)
	{
		startX = round(cx-r/2)-round((y-(cy-offsetY))/sqrt(3)); // Начальная координата зависит от позиции по высоте
		// Так как фигура симметрична, закрашиваем четвертями
		for (x = startX; x < (png_int_32)(cx+1); x++)
		{
			_set_pixel(x, y, fill_color);
			_set_pixel(2*(cx)-x, y, fill_color);
			_set_pixel(2*(cx)-x, 2*(cy)-y, fill_color);
			_set_pixel(x, 2*(cy)-y, fill_color);
		}
	}	

	draw_hexagon(cx, cy, r, thickness, line_color); // Отрисовывание шестиугольника
}

/**
* Функция рисования правильного шестиугольника (с заливкой)
* Принимает координаты прямоугольной области, в которую вписан шестиугольник, толщину линий и цвет линий, цвет заливки
**/
void PNGImage::draw_hexagon(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1, int thickness, PixelPainted *line_color, PixelPainted *fill_color)
{
	_normalize_coords(&x0, &y0, &x1, &y1); // Нормализация координат
	int side; // Длина стороны квадрата, в который вписан шестиугольник
	int radius; // Радиус окружности
	if (x1 - x0 > y1 - y0)
		side = y1 - y0;
	else 
		side = x1 - x0;
	radius = (int) side / 2;
	
	png_int_32 x, y, startX;
	int offsetY = (int) round(radius * sqrt(3) / 2); // Отступ по y для определения положения линий
	for (y = y0+radius-offsetY; y < y0+radius+1; y++)
	{
		startX = round(x0+radius/2)-round((y-(y0+radius-offsetY))/sqrt(3)); // Начальная координата зависит от позиции по высоте
		// Так как фигура симметрична, закрашиваем четвертями
		for (x = startX; x < (png_int_32)(x0+radius)+1; x++)
		{
			_set_pixel(x, y, fill_color);
			_set_pixel(2*(x0+radius)-x, y, fill_color);
			_set_pixel(2*(x0+radius)-x, 2*(y0+radius)-y, fill_color);
			_set_pixel(x, 2*(y0+radius)-y, fill_color);
		}
	}

	draw_hexagon(x0, y0, x1, y1, thickness, line_color); // Отрисовывание шестиугольника
}

/*void PNGImage::reserved(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1)
{
	_normalize_coords(&x0, &y0, &x1, &y1);
	trd_added = false;
}*/

/**
* Вспомогательная функция для рисования линий
* Функция работает на основе алгоритма Брезенхема
* Принимает координаты начала и конца линии, ее толщину и цвет
**/
void PNGImage::_draw_line(png_int_32 x0, png_int_32 y0, png_int_32 x1, png_int_32 y1, int thickness, PixelPainted *color)
{
	int dx = abs((int)(x1-x0)), sx = x0 < x1 ? 1 : -1; // Величина смещения по X и определение: отрисовка вправо или влево
	int dy = abs((int)(y1-y0)), sy = y0 < y1 ? 1 : -1; // Величина смещения по Y и определение: отрисовка вверх или вниз
	int err = dx-dy, e2; // Величины ошибки
	int x2, y2;

	// error value e_xy
	float ed = dx+dy == 0 ? 1 : sqrt((float)dx * dx + (float)dy * dy); // Величина ошибки по диагонали
	for (thickness = (thickness + 1) / 2; ;)
	{
		_set_pixel(x0, y0, color); // Отрисовка основной линии
		e2 = err;
		x2 = x0;
		if (2 * e2 >= -1*dx)
		{
			for (e2 += dy, y2 = y0; e2 < ed * thickness && (y1 != y2 || dx > dy); e2 += dx)
				_set_pixel(x0, y2 += sy, color);
			if (x0 == x1) break;
			e2 = err;
			err -= dy;
			x0 += sx;
		}
		if (2 * e2 <= dy)
		{
			for (e2 = dx-e2; e2 < ed*thickness && (x1 != x2 || dx < dy); e2 += dy)
				_set_pixel(x2 += sx, y0, color);
			if (y0 == y1) break;
			err += dx;
			y0 += sy;
		}
	}
}

/**
* Вспомогательная функция для рисования окружностей
* Функция работает на основе алгоритма Брезенхема
* Принимает координаты центра окружности, ее радиус, толщину и цвет линий
**/
void PNGImage::_draw_circle(png_int_32 cx, png_int_32 cy, int r, int thickness, PixelPainted *color)
{
	png_int_32 x = -1*r, y = 0, err = 2-2*r;
	int rc = r; // Сохранение радиуса окружности
	do {
		// Так как фигура симметрична, отрисовываем четвертями
		_set_pixel(cx - x, cy + y, color);
		_set_pixel(cx - y, cy - x, color);
		_set_pixel(cx + x, cy - y, color);
		_set_pixel(cx + y, cy + x, color);
		rc = err;
		if (rc <= y) 
			err += ++y*2 + 1;
		if (rc > x || err > y) 
			err += ++x*2 + 1;
	} while (x < 0);
	if (thickness > 1)
	{
		// Отрисовка дополнительной окружности для создания толщины
		_draw_circle(cx, cy, r+thickness-1, 1, color);
		if (thickness > 2)
			_draw_circle_thickness(cx, cy, r, thickness, color); // Закрашивание области между окружностями
	}
}

/**
* Вспомогательная функция для отрисовки толщины окружности
* Принимает координаты центра окружности, ее радиус, толщину и цвет линий
**/
void PNGImage::_draw_circle_thickness(png_int_32 cx, png_int_32 cy, int r, int thickness, PixelPainted *color)
{
	int x, y;
	float rad;
	for (y = cy-r-thickness; y < cy+1; y++)
	{
		for (x = cx; x < cx+r+thickness; x++)
		{
			rad = sqrt((x - cx)*(x - cx) + (y - cy)*(y - cy));
			if (rad > r && rad < (r + thickness - 1))
			{
				// Так как фигура симметрична, закрашиваем четвертями
				_set_pixel(x, y, color); // I Четверть
				_set_pixel(2*cx-x, y, color); // II Четверть
				_set_pixel(2*cx-x, 2*cy-y, color); // III Четверть
				_set_pixel(x, 2*cy-y, color);  // IV Четверть
			}
		}
	}
}
