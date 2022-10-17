#include "interface.h"

/**
* Конструктор каркаса приложения
* Создаются верхнее меню, тул бар, инициализируется область с изображением
**/
Interface::Interface(QWidget *parent) : QMainWindow(parent)
{
	createMenu();
	addToolBar(Qt::LeftToolBarArea, createTools());

	scroll = new QScrollArea(this);
	scroll->setWidgetResizable(true);
	scroll->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	drawedImg = new QLabel(this);
	drawedImg->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	scroll->setWidget(drawedImg);
	setCentralWidget(scroll);

	statusBar()->showMessage("Готово");
}

/**
* Создание верхнего меню
**/
void Interface::createMenu()
{
	// Создание вкладки "Файл"
	topMenuFile = menuBar()->addMenu("&Файл");
	// Добавление "действий"
	actionOpen = topMenuFile->addAction("&Открыть");
	actionSave = topMenuFile->addAction("&Сохранить Как");
	topMenuFile->addSeparator();
	actionQuit = topMenuFile->addAction("&Выход");
	// Привязка "действий" к событию triggered
	connect(actionOpen, &QAction::triggered, this, &Interface::OpenFile);
	connect(actionSave, &QAction::triggered, this, &Interface::SaveFile);
	connect(actionQuit, static_cast<void (QAction::*)(bool)>(&QAction::triggered), this, [&](){ Interface::onQuit(true); });
	// Добавление кнопки "Изображение"
	actionProperties = menuBar()->addAction("&Изображение");
	connect(actionProperties, &QAction::triggered, this, &Interface::Properties);
	// Добавление кнопки "О программе"
	actionAbout = menuBar()->addAction("&О программе");
	connect(actionAbout, &QAction::triggered, this, &Interface::About);
}

/**
* Обработчик события выхода из приложения
**/
void Interface::exitHandler()
{
	this->onQuit(false);
}

/**
* Обработка выхода из приложения
* Аргумент служит для проверки, закрывалось приложение через меню или через кнопку крестика
**/
void Interface::onQuit(bool exit_from_menu)
{
	if (img && has_changes)
	{
		QMessageBox msgBox;
        msgBox.setText("Вы собираетесь закрыть программу");
        msgBox.setInformativeText("Вы хотите сохранить изменения?");
        msgBox.setWindowTitle("Изменения могут быть потеряны");
        if (exit_from_menu)
        	msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        else
        	msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
        msgBox.setDefaultButton(QMessageBox::Save);

        switch (msgBox.exec())
        {
        	case QMessageBox::Cancel:
        		return;
        	case QMessageBox::Save:
        		SaveFile();
        		has_changes = false;
        		QApplication::quit();
        		break;
        	default:
        		has_changes = false;
        		QApplication::quit();
        		break;
        }
	}
	QApplication::quit();
}

/**
* Окно со справкой
**/
void Interface::About()
{
	QMessageBox info;
	info.setWindowTitle("О программе");
	info.setIcon(QMessageBox::Information);
	info.setInformativeText("Данная программа служит для обработки изображений в формате PNG.\n"
							"С помощью меню \"Файл\" можно открыть и сохранить изображение.\n"
							"С помощью меню \"Изображение\" можно узнать информацию об изображении.\n"
							"Для начала взаимодействия необходимо открыть изображение.\n"
							"Чтобы использовать функции обработки нужно:\n"
							"1) Выделить необходимую область\n"
							"2) Настроить дополнительные параметры и цвет линий и заливки\n"
							"3) Нажать кнопку с необходимой функцией (при наведении выведется подсказка)");
	info.setDetailedText("Программа поддерживает 4 функции обработки изображения:\n"
						"1) Рисование пентаграммы\n"
						"2) Отражение область относительно осей X и Y\n"
						"3) Рисование правильного шестиугольника (может быть закрашен)\n"
						"4) Рисование прямоугольника (может быть закрашен)");
	info.setText("Краткая инструкция к программе");

	info.exec();
}

/**
* Свойства изображения
**/
void Interface::Properties()
{
	QDialog propertiesDialog;
	propertiesDialog.setMinimumSize(400, 340);
	propertiesDialog.setWindowTitle("Свойства изображения");

	QGridLayout propertiesLayout;
	propertiesLayout.setSpacing(5);

	QString cType = "Неизвестно";
	if (img)
	{
		if (img->color_type == PNG_COLOR_TYPE_GRAY)
			cType = "Grayscale";
		else if (img->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
			cType = "Grayscale Alpha";
		else if (img->color_type == PNG_COLOR_TYPE_RGB)
			cType = "RGB";
		else if (img->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
			cType = "RGB Alpha";
	}	

	QLabel lbiSize("<b>Размер в пикселях: </b>");
	QLabel lbfName("<b>Имя файла: </b>");
	QLabel lbfSize("<b>Размер файла: </b>");
	QLabel lbColorType("<b>Цветовая схема: </b>");
	QLabel lbBitDepth("<b>Глубина цвета: </b>");
	QLabel lbLastAccess("<b>Последнее открытие: </b>");
	QLabel lbLastModified("<b>Последнее изменение: </b>");

	QLabel outiSize(cType);
	QLabel outfName(cType);
	QLabel outfSize(cType);
	QLabel outColorType(cType);
	QLabel outBitDepth(cType);
	QLabel outLastAccess(cType);
	QLabel outLastModified(cType);
	if (img)
	{
		outiSize.setText(QString::number(img->width)+"x"+QString::number(img->height)+" px");
		outfName.setText(this->_filename);
		outfSize.setText(QString::number(img->file_info.st_size)+" байтов");
		outColorType.setText(cType);
		outBitDepth.setText(QString::number(img->bit_depth));
		outLastAccess.setText(QString(ctime(&img->file_info.st_atime)));
		outLastModified.setText(QString(ctime(&img->file_info.st_mtime)));
	}

	propertiesLayout.addWidget(&lbfName, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	propertiesLayout.addWidget(&lbiSize, 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	propertiesLayout.addWidget(&lbfSize, 2, 0, Qt::AlignRight | Qt::AlignVCenter);
	propertiesLayout.addWidget(&lbColorType, 3, 0, Qt::AlignRight | Qt::AlignVCenter);
	propertiesLayout.addWidget(&lbBitDepth, 4, 0, Qt::AlignRight | Qt::AlignVCenter);
	propertiesLayout.addWidget(&lbLastAccess, 5, 0, Qt::AlignRight | Qt::AlignVCenter);
	propertiesLayout.addWidget(&lbLastModified, 6, 0, Qt::AlignRight | Qt::AlignVCenter);

	propertiesLayout.addWidget(&outfName, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);
	propertiesLayout.addWidget(&outiSize, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
	propertiesLayout.addWidget(&outfSize, 2, 1, Qt::AlignLeft | Qt::AlignVCenter);
	propertiesLayout.addWidget(&outColorType, 3, 1, Qt::AlignLeft | Qt::AlignVCenter);
	propertiesLayout.addWidget(&outBitDepth, 4, 1, Qt::AlignLeft | Qt::AlignVCenter);
	propertiesLayout.addWidget(&outLastAccess, 5, 1, Qt::AlignLeft | Qt::AlignVCenter);
	propertiesLayout.addWidget(&outLastModified, 6, 1, Qt::AlignLeft | Qt::AlignVCenter);

	propertiesDialog.setLayout(&propertiesLayout);
	if (!propertiesDialog.exec())
		return;
}
/**
* Вызов диалогового окна с текстом ошибки
**/
void Interface::showErrorMessage(QString errorMessage)
{
	QMessageBox message;
	message.setWindowTitle("Ошибка!");
	message.setIcon(QMessageBox::Warning);
	message.setText("В процессе исполнения возникла ошибка:\n" + errorMessage);

	message.exec();
}

/**
* Создание тул бара
**/
QToolBar *Interface::createTools()
{
	// Иконки для функций
	QPixmap drPtg("./icons/pentagram.png");
	QPixmap mir("./icons/mirror.png");
	QPixmap drHex("./icons/hexagon.png");
	QPixmap drRect("./icons/rectangle.png");
	QPixmap set("./icons/settings.png");
	QPixmap thck("./icons/thickness.png");
	QPixmap aset("./icons/other.png");
	QPixmap zIn("./icons/zoomIn.png");
	QPixmap zOut("./icons/zoomOut.png");
	//QPixmap rsrv("./icons/reserved.png");

	tools = new QToolBar(this);
	//tools->setFixedWidth(50);
	settings = tools->addAction(QIcon(set), "&Выделенная область");
	tools->addSeparator();

	drawPentagram = tools->addAction(QIcon(drPtg), "&Нарисовать пентаграмму");
	mirrorArea = tools->addAction(QIcon(mir), "&Отразить область");
	drawHexagon = tools->addAction(QIcon(drHex), "&Нарисовать шестиугольник");
	drawRectangle = tools->addAction(QIcon(drRect), "&Нарисовать прямоугольник");
	//reserved = tools->addAction(QIcon(rsrv), "&Зарезервировано");
	tools->addSeparator();

	additionalSettings = tools->addAction(QIcon(aset), "&Дополнительные настройки");
	chThickness = tools->addAction(QIcon(thck), "&Установить толщину");

	// Установка цветов по умолчанию для линий и заливки
	primaryColor.setRgb(0, 0, 0, 255);
	secondaryColor.setRgb(255, 255, 255, 255);
	QPixmap primaryColorPixmap(50, 50);
	QPixmap secondaryColorPixmap(50, 50);
	primaryColorPixmap.fill(primaryColor);
	secondaryColorPixmap.fill(secondaryColor);
	// Изменение иконки выбора цвета
	QPainter pcp(&primaryColorPixmap);
	pcp.setPen(QPen(Qt::black, 1));
	pcp.drawRoundedRect(QRectF(0, 0, 50, 50), 5, 0);

	QPainter scp(&secondaryColorPixmap);
	scp.setPen(QPen(Qt::black, 1));
	scp.drawRoundedRect(QRectF(0, 0, 50, 50), 5, 0);

	primaryColorPeek = tools->addAction(QIcon(primaryColorPixmap), "&Установить цвет линий");
	secondaryColorPeek = tools->addAction(QIcon(secondaryColorPixmap), "&Установить цвет заливки");
	tools->addSeparator();

	zoomIn = tools->addAction(QIcon(zIn), "&Увеличить на 25%");
	zoomOut = tools->addAction(QIcon(zOut), "&Уменьшить на 10%");
	tools->addSeparator();

	connect(settings, &QAction::triggered, this, &Interface::callSettings);
	connect(drawPentagram, &QAction::triggered, this, &Interface::callDrawPentagram);
	connect(mirrorArea, &QAction::triggered, this, &Interface::callMirrorArea);
	connect(drawHexagon, &QAction::triggered, this, &Interface::callDrawHexagon);
	connect(drawRectangle, &QAction::triggered, this, &Interface::callDrawRectangle);
	//connect(reserved, &QAction::triggered, this, &Interface::callReserved);

	connect(additionalSettings, &QAction::triggered, this, &Interface::callSetAdditionalSettings);
	connect(chThickness, &QAction::triggered, this, &Interface::setThickness);

	connect(primaryColorPeek, &QAction::triggered, this, &Interface::callSetPrimaryColor);
	connect(secondaryColorPeek, &QAction::triggered, this, &Interface::callSetSecondaryColor);

	connect(zoomIn, &QAction::triggered, this, &Interface::UpScale);
	connect(zoomOut, &QAction::triggered, this, &Interface::DownScale);

	return tools;
}

/*void Interface::callReserved()
{
	std::cout << "reserved" << std::endl;
}*/

Interface::~Interface()
{
	delete topMenuFile;
	delete actionAbout;
	delete tools;
	delete tmp;
	delete img;
	delete drawedImg;
	delete scroll;
}

/**
* Обработка открытия изображения
**/
void Interface::OpenFile()
{
	if (img && has_changes)
	{
		QMessageBox msgBox;
        msgBox.setText("Вы собираетесь закрыть изображение");
        msgBox.setInformativeText("Все несохраненные изменения будут потеряны");
        msgBox.setWindowTitle("Изменения могут быть потеряны");
        msgBox.setStandardButtons(QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Discard);
        if (msgBox.exec() == QMessageBox::Cancel)
            return;
	}
	QString filename = QFileDialog::getOpenFileName(this, tr("Открыть *.png файл"), QDir::home().dirName(), tr("Изображение (*.png)"));
	if (!filename.isEmpty())
	{
		delete tmp;
		delete img;
		tmp = new QTemporaryFile();
		tmp->open();

		img = new PNGImage();
		try
		{
			img->read_image(filename.toLocal8Bit().data());
			img->write_image(tmp->fileName().toLocal8Bit().data());
			if (x1 == 0 && y1 == 0)
			{
				x1 = img->width;
				y1 = img->height;
			}
			if (x1 > img->width)
				x1 = img->width;
			if (y1 > img->height)
				y1 = img->height;
			scale = 1.0;
			drawImg();
			statusBar()->showMessage("Изображение успешно открыто");
			this->_filename = filename;
			has_changes = false;
		} catch (exception& ex)
		{
			statusBar()->showMessage("Произошла ошибка при открытии изображения!");
			showErrorMessage("Не удалось открыть изображение!");
		}
	}
}

/**
* Отрисовка изображения
**/
void Interface::drawImg()
{
	if (!tmp) return;
	imgPm = QPixmap::fromImage(QImage(tmp->fileName()));
	drawedImg->setPixmap(imgPm.scaled(QSize(img->width * scale, img->height * scale)));
}

/**
* Сохранение изображения в файл
**/
void Interface::SaveFile()
{
	if (!tmp)
	{
		statusBar()->showMessage("Нечего сохранять!");
		showErrorMessage("Сперва необходимо открыть изображение!");
		return;
	}
	QString filename = QFileDialog::getSaveFileName(this, tr("Сохранить *.png файл"), "Безымянный.png", tr("Изображение (*.png)"));
	if (!filename.isEmpty())
	{
		try
		{
			img->write_image(filename.toLocal8Bit().data());
			img->get_file_info(filename.toLocal8Bit().data());
			statusBar()->showMessage("Изображение успешно сохранено");
			this->_filename = filename;
			has_changes = false;
		} catch (exception& ex)
		{
			statusBar()->showMessage("Произошла ошибка при сохранении изображения!");
			showErrorMessage("Не удалось сохранить изображение!");
		}
	}
}

/**
* Увеличение изображения на 25% (относительно изначальных размеров)
**/
void Interface::UpScale()
{
	if (scale <= 5.0)
		scale += 0.25;
	drawImg();
}

/**
* Уменьшение изображения на 10% (относительно изначальных размеров)
**/
void Interface::DownScale()
{
	if (scale >= 0.2)
		scale -= 0.1;
	drawImg();
}

/**
* Вызов меню настройки выделенной области
**/
void Interface::callSettings()
{
	QDialog settingsDialog;
	settingsDialog.setFixedSize(320, 180);
	settingsDialog.setWindowTitle("Настройки выделения");

	QGridLayout settingsLayout;
	settingsLayout.setSpacing(1);

	QLabel lb1("Левый верхний угол области");
	QLabel lb2("Правый нижний угол области");
	QLabel params("");
	settingsLayout.addWidget(&lb1, 0, 0);
	settingsLayout.addWidget(&lb2, 2, 0);

	QSpinBox x0input;
	QSpinBox y0input;
	QSpinBox x1input;
	QSpinBox y1input;

	if (img)
	{
		x0input.setMaximum(img->width);
		y0input.setMaximum(img->height);
		x1input.setMaximum(img->width);
		y1input.setMaximum(img->height);
	}

	x0input.setFixedHeight(30);
	x0input.setValue(x0);
	y0input.setMinimum(0);

	y0input.setFixedHeight(30);
	y0input.setValue(y0);
	x0input.setMinimum(0);

	x1input.setFixedHeight(30);
	x1input.setValue(x1);
	x1input.setMinimum(0);

	y1input.setFixedHeight(30);
	y1input.setValue(y1);
	y1input.setMinimum(0);

	settingsLayout.addWidget(&x0input, 1, 0);
	settingsLayout.addWidget(&y0input, 1, 1);
	settingsLayout.addWidget(&x1input, 3, 0);
	settingsLayout.addWidget(&y1input, 3, 1);

	if (img != nullptr)
	{	
		params.setText(QString("Размеры изображения: %1x%2 px").arg(QString::number(img->width), QString::number(img->height)));
		settingsLayout.addWidget(&params, 4, 0, 1, 2);
	}

	connect(&x0input, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [&](int x) { this->x0 = x; });
	connect(&y0input, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [&](int y) { this->y0 = y; });
	connect(&x1input, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [&](int x) { this->x1 = x; });
	connect(&y1input, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [&](int y) { this->y1 = y; });
		
	settingsDialog.setLayout(&settingsLayout);
	if (!settingsDialog.exec())
		return;
}

/**
* Вызов функции рисования пентаграммы
**/
void Interface::callDrawPentagram()
{
	if (!img)
	{
		statusBar()->showMessage("Сперва необходимо открыть изображение!");
		return;
	}
	try
	{
		PixelPainted color;
		color.r = primaryColor.red();
		color.g = primaryColor.green();
		color.b = primaryColor.blue();
		color.a = primaryColor.alpha();
		img->draw_pentagram(x0, y0, x1, y1, thickness, &color);
		img->write_image(tmp->fileName().toLocal8Bit().data());

		drawImg();
		statusBar()->showMessage("Пентаграмма успешно нарисована");
		has_changes = true;
	} catch (exception& ex)
	{
		statusBar()->showMessage("Возникла ошибка при рисовании!");
		showErrorMessage("Не удалось нарисовать пентаграмму!");
	}
}

/**
* Вызов функции отражения области
**/
void Interface::callMirrorArea()
{
	if (!img)
	{
		statusBar()->showMessage("Сперва необходимо открыть изображение!");
		return;
	}
	try
	{
		img->mirror(mirrorAxis, x0, y0, x1, y1);
		img->write_image(tmp->fileName().toLocal8Bit().data());

		drawImg();
		statusBar()->showMessage("Отражено успешно");
		has_changes = true;
	} catch (exception& ex)
	{
		statusBar()->showMessage("Возникла ошибка при отражении!");
		showErrorMessage("Не удалось отразить область!");
	}
}

/**
* Вызов функции рисования правильного шестиугольника
**/
void Interface::callDrawHexagon()
{
	if (!img)
	{
		statusBar()->showMessage("Сперва необходимо открыть изображение!");
		return;
	}
	try
	{
		PixelPainted color1;
		color1.r = primaryColor.red();
		color1.g = primaryColor.green();
		color1.b = primaryColor.blue();
		color1.a = primaryColor.alpha();

		PixelPainted color2;
		color2.r = secondaryColor.red();
		color2.g = secondaryColor.green();
		color2.b = secondaryColor.blue();
		color2.a = secondaryColor.alpha();

		if (useFill)
			img->draw_hexagon(x0, y0, x1, y1, thickness, &color1, &color2);
		else
			img->draw_hexagon(x0, y0, x1, y1, thickness, &color1);
		img->write_image(tmp->fileName().toLocal8Bit().data());

		drawImg();
		statusBar()->showMessage("Шестиугольник успешно нарисован");
		has_changes = true;
	} catch (exception& ex)
	{
		statusBar()->showMessage("Возникла ошибка при рисовании!");
		showErrorMessage("Не удалось нарисовать шестиугольник!");
	}
}

/**
* Вызов функции рисования прямоугольника
**/
void Interface::callDrawRectangle()
{
	if (!img)
	{
		statusBar()->showMessage("Сперва необходимо открыть изображение!");
		return;
	}
	try
	{
		PixelPainted color1;
		color1.r = primaryColor.red();
		color1.g = primaryColor.green();
		color1.b = primaryColor.blue();
		color1.a = primaryColor.alpha();

		PixelPainted color2;
		color2.r = secondaryColor.red();
		color2.g = secondaryColor.green();
		color2.b = secondaryColor.blue();
		color2.a = secondaryColor.alpha();

		if (useFill)
			img->draw_rectangle(x0, y0, x1, y1, thickness, &color1, &color2);
		else
			img->draw_rectangle(x0, y0, x1, y1, thickness, &color1);
		img->write_image(tmp->fileName().toLocal8Bit().data());

		drawImg();
		statusBar()->showMessage("Прямоугольник успешно нарисован");
		has_changes = true;
	} catch (exception& ex)
	{
		statusBar()->showMessage("Возникла ошибка при рисовании!");
		showErrorMessage("Не удалось нарисовать прямоугольник!");
	}
}

/**
* Вызов окна дополнительных настроек
**/
void Interface::callSetAdditionalSettings()
{
	QDialog additionalDialog;
	additionalDialog.setFixedSize(320, 120);
	additionalDialog.setWindowTitle("Дополнительные настройки");

	QGridLayout additionalLayout;
	additionalLayout.setSpacing(1);

	QLabel lb("Ось отражения области");

	QRadioButton radio1("&Ось X", this);
	QRadioButton radio2("&Ось Y", this);
	QCheckBox filling("&Использовать заливку", this);

	filling.setChecked(useFill);
	if (mirrorAxis) radio1.setChecked(true);
	else radio2.setChecked(true);

	connect(&filling, static_cast<void (QCheckBox::*)(bool)>(&QCheckBox::clicked), [&](){ this->useFill = filling.isChecked(); });
	connect(&radio1, static_cast<void (QRadioButton::*)(bool)>(&QRadioButton::clicked), [&](){ this->mirrorAxis = mirrorAxisX; });
	connect(&radio2, static_cast<void (QRadioButton::*)(bool)>(&QRadioButton::clicked), [&](){ this->mirrorAxis = mirrorAxisY; });
	
	additionalLayout.addWidget(&filling, 0, 0);
	additionalLayout.addWidget(&lb, 1, 0);
	additionalLayout.addWidget(&radio1, 2, 0);
	additionalLayout.addWidget(&radio2, 2, 1);

	additionalDialog.setLayout(&additionalLayout);
	if (!additionalDialog.exec())
		return;
}

/**
* Вызов окна настройки толщины линии
**/
void Interface::setThickness()
{
	QDialog thicknessDialog;
	thicknessDialog.setFixedSize(320, 120);
	thicknessDialog.setWindowTitle("Настройки толщины");
	
	QGridLayout thicknessLayout;
	thicknessLayout.setSpacing(1);

	QSlider setter(Qt::Horizontal, this);
	setter.setMinimum(1);
	setter.setMaximum(50);
	setter.setValue(thickness);

	QLabel lbl("Текущая толщина");
	QLabel val(QString::number(thickness));
	lbl.setFixedHeight(30);
	val.setFixedSize(20, 30);

	connect(&setter, static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged), [&](int th) { 
		this->thickness = th; 
		val.setNum(th);
	});

	thicknessLayout.addWidget(&lbl, 0, 0);
	thicknessLayout.addWidget(&val, 0, 1);
	thicknessLayout.addWidget(&setter, 1, 0);
	thicknessDialog.setLayout(&thicknessLayout);

	if (!thicknessDialog.exec())
		return;
}

/**
* Вызов окна выбора цвета линий
**/
void Interface::callSetPrimaryColor()
{
	QColor peek = QColorDialog::getColor(primaryColor, nullptr, "Выберите цвет линий", QColorDialog::ShowAlphaChannel | QColorDialog::NoButtons);
	if (peek.isValid())
	{
		primaryColor = peek;

		QPixmap primaryColorPixmap(50, 50);
		primaryColorPixmap.fill(primaryColor);

		QPainter pcp(&primaryColorPixmap);
		pcp.setPen(QPen(Qt::black, 1));
		pcp.drawRoundedRect(QRectF(0, 0, 50, 50), 5, 0);

		primaryColorPeek->setIcon(QIcon(primaryColorPixmap));
	}
}

/**
* Вызов окна выбора цвета заливки
**/
void Interface::callSetSecondaryColor()
{
	QColor peek = QColorDialog::getColor(secondaryColor, nullptr, "Выберите цвет заливки", QColorDialog::ShowAlphaChannel | QColorDialog::NoButtons);
	if (peek.isValid())
	{
		secondaryColor = peek;

		QPixmap secondaryColorPixmap(50, 50);
		secondaryColorPixmap.fill(secondaryColor);

		QPainter scp(&secondaryColorPixmap);
		scp.setPen(QPen(Qt::black, 1));
		scp.drawRoundedRect(QRectF(0, 0, 50, 50), 5, 0);

		secondaryColorPeek->setIcon(QIcon(secondaryColorPixmap));
	}
}

/**
* Функции обработки выделения области
**/
void Interface::mousePressEvent(QMouseEvent *event)
{
	if (!img) return;
	sPoint = drawedImg->mapFromGlobal(this->mapToGlobal(event->pos()));
	if (drawedImg->height() > img->height * scale)
	{
		y0 = (png_uint_32)((sPoint.y() - (drawedImg->height()-(img->height * scale))/2)/scale);
	} else y0 = (png_uint_32)(sPoint.y()/scale);
	if (drawedImg->width() > img->width * scale)
	{
		x0 = (png_uint_32)((sPoint.x() - (drawedImg->width()-(img->width * scale))/2)/scale);
	} else x0 = (png_uint_32)(sPoint.x()/scale);

	select = new QRubberBand(QRubberBand::Rectangle, drawedImg);
	select->setGeometry(QRect(sPoint, sPoint));
	select->show();
}

void Interface::mouseMoveEvent(QMouseEvent *event)
{
	if (!img) return;
	QPoint point = drawedImg->mapFromGlobal(this->mapToGlobal(event->pos()));
	select->setGeometry(QRect(sPoint, point).normalized());
}

void Interface::mouseReleaseEvent(QMouseEvent *event)
{
	if (!img) return;
	QPoint point = drawedImg->mapFromGlobal(this->mapToGlobal(event->pos()));

	if (drawedImg->height() > img->height * scale)
	{
		y1 = (png_uint_32)((point.y() - (drawedImg->height()-(img->height * scale))/2)/scale);
	} else y1 = (png_uint_32)(point.y()/scale);
	if (drawedImg->width() > img->width * scale)
	{
		x1 = (png_uint_32)((point.x() - (drawedImg->width()-(img->width * scale))/2)/scale);
	} else x1 = (png_uint_32)(point.x()/scale);
	select->hide();
	select->clearMask();
}
