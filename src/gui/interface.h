#pragma once

#include <ctime>
#include <QWidget>
#include <QMainWindow>
#include <QApplication>
#include <QToolBar>
#include <QIcon>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QFile>
#include <QScrollArea>
#include <QDialog>
#include <QGridLayout>
#include <QSpinBox>
#include <QSlider>
#include <QColor>
#include <QPainter>
#include <QColorDialog>
#include <QMouseEvent>
#include <QRadioButton>
#include <QCheckBox>
#include <QRubberBand>

#include "../pngimage.h"

class Interface : public QMainWindow
{
	Q_OBJECT
public:
	// Координаты области выделения
	png_int_32 x0 = 0;
	png_int_32 y0 = 0;
	png_int_32 x1 = 0;
	png_int_32 y1 = 0;
	// Толщина линий
	png_int_32 thickness = 3;
	// Дополнительный настройки рисования
	int mirrorAxis = mirrorAxisX;
	bool useFill = false;
	QColor primaryColor;
	QColor secondaryColor;

	Interface(QWidget *parent = 0);
	~Interface();
private:
	QTemporaryFile *tmp = nullptr;
	QString _filename = "";
	PNGImage *img = nullptr;
	QPixmap imgPm;
	bool has_changes = false;

	QMenu *topMenuFile = nullptr;
	QAction *actionAbout = nullptr;
	QAction *actionProperties = nullptr;
	QToolBar *tools = nullptr;
	QAction *actionOpen = nullptr;
	QAction *actionSave = nullptr;
	QAction *actionQuit = nullptr;

	QScrollArea *scroll = nullptr;
	QLabel *drawedImg = nullptr;

	QAction *settings = nullptr;
	QAction *drawPentagram = nullptr;
	QAction *mirrorArea = nullptr;
	QAction *drawHexagon = nullptr;
	QAction *drawRectangle = nullptr;
	QAction *chThickness = nullptr;
	//QAction *reserved = nullptr;

	QAction *additionalSettings = nullptr;
	QAction *primaryColorPeek = nullptr;
	QAction *secondaryColorPeek = nullptr;
	QAction *zoomIn = nullptr;
	QAction *zoomOut = nullptr;

	float scale = 1.0;

	QRubberBand *select = nullptr;
	QPoint sPoint;

	void createMenu();
	void drawImg();
	QToolBar *createTools();

	// Обработка выделения области
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	void showErrorMessage(QString errorMessage);
public slots:
	void OpenFile();
	void SaveFile();
	void About();
	void Properties();
	void onQuit(bool exit_from_menu = true);
	void callSettings();
	void callDrawPentagram();
	void callMirrorArea();
	void callDrawHexagon();
	void callDrawRectangle();
	//void callReserved();
	void callSetAdditionalSettings();
	void setThickness();
	void callSetPrimaryColor();
	void callSetSecondaryColor();
	void UpScale();
	void DownScale();

	void exitHandler();
};
