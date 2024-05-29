#include "Render.h"
#include <sstream>
#include <iostream>
#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include "MyOGL.h"
#include "Camera.h"
#include "Light.h"
#include "Primitives.h"
#include "GUItextRectangle.h"
#include <cmath>
#define PI 3.14159265

bool textureMode = true;
bool lightMode = true;
bool textureReplace = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}

	if (key == 'E')
	{
		textureReplace = !textureReplace;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;
GLuint texId2;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	{RGBTRIPLE* texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	//массив трехбайтных элементов  (R G B)
	{RGBTRIPLE* texarray2;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray2;
	int texW2, texH2;
	OpenGL::LoadBMP("texture2.bmp", &texW2, &texH2, &texarray2);
	OpenGL::RGBtoChar(texarray2, texW2, texH2, &texCharArray2);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId2);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId2);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW2, texH2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray2);

	//отчистка памяти
	free(texCharArray2);
	free(texarray2);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}





double N_Vector_X(double A[], double B[], double C[], double height) {




	//Счиатем А и В по входным точкам
	double Vector_AB[3] = { B[0] - A[0], B[1] - A[1], B[2] - A[2] };
	double Vector_AC[3] = { C[0] - A[0], C[1] - A[1], C[2] - A[2] };

	//Создаем вектор N
	double N_X = Vector_AB[1] * Vector_AC[2] - Vector_AC[1] * Vector_AB[2];
	double N_Y = -Vector_AB[0] * Vector_AC[2] + Vector_AC[0] * Vector_AB[2];
	double N_Z = Vector_AB[0] * Vector_AC[1] - Vector_AC[0] * Vector_AB[1];

	double N_Vector[] = { N_X,N_Y,N_Z };
	double Abs_Vector = sqrt(N_X * N_X + N_Y * N_Y + N_Z * N_Z);

	N_Vector[0] = N_X / Abs_Vector;
	N_Vector[1] = N_Y / Abs_Vector;
	N_Vector[2] = N_Z / Abs_Vector;
	return N_Vector[0];
}

double N_Vector_Y(double A[], double B[], double C[], double height) {




	//Счиатем А и В по входным точкам
	double Vector_AB[3] = { B[0] - A[0], B[1] - A[1], B[2] - A[2] };
	double Vector_AC[3] = { C[0] - A[0], C[1] - A[1], C[2] - A[2] };

	//Создаем вектор N
	double N_X = Vector_AB[1] * Vector_AC[2] - Vector_AC[1] * Vector_AB[2];
	double N_Y = -Vector_AB[0] * Vector_AC[2] + Vector_AC[0] * Vector_AB[2];
	double N_Z = Vector_AB[0] * Vector_AC[1] - Vector_AC[0] * Vector_AB[1];

	double N_Vector[] = { N_X,N_Y,N_Z };
	double Abs_Vector = sqrt(N_X * N_X + N_Y * N_Y + N_Z * N_Z);

	N_Vector[0] = N_X / Abs_Vector;
	N_Vector[1] = N_Y / Abs_Vector;
	N_Vector[2] = N_Z / Abs_Vector;
	return N_Vector[1];
}

double N_Vector_Z(double A[], double B[], double C[], double height) {




	//Счиатем А и В по входным точкам
	double Vector_AB[3] = { B[0] - A[0], B[1] - A[1], B[2] - A[2] };
	double Vector_AC[3] = { C[0] - A[0], C[1] - A[1], C[2] - A[2] };

	//Создаем вектор N
	double N_X = Vector_AB[1] * Vector_AC[2] - Vector_AC[1] * Vector_AB[2];
	double N_Y = -Vector_AB[0] * Vector_AC[2] + Vector_AC[0] * Vector_AB[2];
	double N_Z = Vector_AB[0] * Vector_AC[1] - Vector_AC[0] * Vector_AB[1];

	double N_Vector[] = { N_X,N_Y,N_Z };
	double Abs_Vector = sqrt(N_X * N_X + N_Y * N_Y + N_Z * N_Z);

	N_Vector[0] = N_X / Abs_Vector;
	N_Vector[1] = N_Y / Abs_Vector;
	N_Vector[2] = N_Z / Abs_Vector;
	return N_Vector[2];
}




//Прототипы
void Convexity_1(double);
void Convexity_1_Alpha_Triangles(double);
void Convexity_2(double);
void Convexity_2_Alpha_Triangles(double);
void Concavity(double);
void Concavity_Alpha_Triangles(double);

//Для закрашивания через треугольники основания
void Triangle_Down_Light(double A[], double B[], double C[]) {

	glBegin(GL_TRIANGLES);
	glColor3f(0.5f, 0.0f, 0.5f);
	glNormal3f(0, 0, -1);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glEnd();
}

void Triangle_Up_Light(double A[], double B[], double C[]) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_TRIANGLES);
	glColor4f(0.5f, 0.0f, 0.5f, 0.7f);//4 параметр - кэф прозрачности
	glNormal3f(0, 0, 1);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glEnd();
	glDisable(GL_BLEND);
}



//Для закрашивания через четырёхугольники боковые грани
void Quad(double A[], double B[], double C[], double D[], double height) {
	double N_X = N_Vector_X(A, B, D, height);
	double N_Y = N_Vector_Y(A, B, D, height);
	double N_Z = N_Vector_Z(A, B, D, height);
	//glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_QUADS);
	glColor3d(0.5f, 0.5f, 0.5f);
	glNormal3f(N_X, N_Y, N_Z);
	glTexCoord2d(0, 0);
	glVertex3dv(A);
	glTexCoord2d(0, 1);
	glVertex3dv(B);
	glTexCoord2d(1, 1);
	glVertex3dv(C);
	glTexCoord2d(1, 0);
	glVertex3dv(D);
	glEnd();

}


//Построение самой фигуры
void Figure(double height) {


	double Aold[3] = { 2,0,0 };
	double Bold[3] = { 0,5,0 };
	double Cold[3] = { 4,5,0 };
	double Dold[3] = { 5,10,0 };
	double Eold[3] = { 9,11,0 };
	double Fold[3] = { 6,4,0 };
	double Gold[3] = { 10,1,0 };
	double Hold[3] = { 5,3,0 };

	double NewPrismCenterX = double(49916) / double(9375); //Центр фигуры по координате Х. Посчитал руками. 
	double NewPrismCenterY = double(29889) / double(6250); //Центр фигуры по координате Y. Посчитал руками. 


	double A[3] = { 2 - NewPrismCenterX,0 - NewPrismCenterY,0 };//A: {-3.32437, - 4.78224, 0}
	double B[3] = { 0 - NewPrismCenterX,5 - NewPrismCenterY,0 };//B: {-5.32437, 0.21776, 0}
	double C[3] = { 4 - NewPrismCenterX,5 - NewPrismCenterY,0 };//C: {-1.32437, 0.21776, 0}
	double D[3] = { 5 - NewPrismCenterX,10 - NewPrismCenterY,0 };//D: {-0.324373, 5.21776, 0}
	double E[3] = { 9 - NewPrismCenterX,11 - NewPrismCenterY,0 };//E: {3.67563, 6.21776, 0}
	double F[3] = { 6 - NewPrismCenterX,4 - NewPrismCenterY,0 };//F: {0.675627, - 0.78224, 0}
	double G[3] = { 10 - NewPrismCenterX,1 - NewPrismCenterY,0 };//G: {4.67563, - 3.78224, 0}
	double H[3] = { 5 - NewPrismCenterX,3 - NewPrismCenterY,0 };//H: {-0.324373, - 1.78224, 0}

	double A1[3] = { 2 - NewPrismCenterX,0 - NewPrismCenterY,height };//A: {-3.32437, - 4.78224, 0}
	double B1[3] = { 0 - NewPrismCenterX,5 - NewPrismCenterY,height };//B: {-5.32437, 0.21776, 0}
	double C1[3] = { 4 - NewPrismCenterX,5 - NewPrismCenterY,height };//C: {-1.32437, 0.21776, 0}
	double D1[3] = { 5 - NewPrismCenterX,10 - NewPrismCenterY,height };//D: {-0.324373, 5.21776, 0}
	double E1[3] = { 9 - NewPrismCenterX,11 - NewPrismCenterY,height };//E: {3.67563, 6.21776, 0}
	double F1[3] = { 6 - NewPrismCenterX,4 - NewPrismCenterY,height };//F: {0.675627, - 0.78224, 0}
	double G1[3] = { 10 - NewPrismCenterX,1 - NewPrismCenterY,height };//G: {4.67563, - 3.78224, 0}
	double H1[3] = { 5 - NewPrismCenterX,3 - NewPrismCenterY,height };//H: {-0.324373, - 1.78224, 0}




	//БОКОВЫЕ ГРАНИ:
	//Quad(A, A1, B1, B);
	Quad(B, B1, C1, C, height);
	Quad(C, C1, D1, D, height);
	//Quad(D, D1, E1, E, height);
	Quad(E, E1, F1, F, height);
	Quad(F, F1, G1, G, height);
	Quad(G, G1, H1, H, height);
	Quad(H, H1, A1, A, height);



	//Нижнее основание:
	//Triangle(A, B, H);
	//Triangle(B, C, H);




	//1:
	double First_1[] = { 3-NewPrismCenterX, 3 - NewPrismCenterY ,0 };
	double second_1[] = { 3 - NewPrismCenterX, 5 - NewPrismCenterY,0 };
	double third_1[] = { 5 - NewPrismCenterX, 5 - NewPrismCenterY ,0 };
	double forth_1[] = { 5 - NewPrismCenterX, 3 - NewPrismCenterY,0 };


	//2:
	double First_2[] = { 4 - NewPrismCenterX, 5 - NewPrismCenterY ,0 };
	double second_2[] = { 4.8 - NewPrismCenterX, 9 - NewPrismCenterY,0 };
	double third_2[] = { 8.2 - NewPrismCenterX, 9 - NewPrismCenterY ,0 };
	double forth_2[] = { 6.4 - NewPrismCenterX, 5 - NewPrismCenterY,0 };
	
	//3:
	double First_3[] = { 5 - NewPrismCenterX, 4 - NewPrismCenterY ,0 };
	double second_3[] = { 5 - NewPrismCenterX, 5 - NewPrismCenterY,0 };
	double third_3[] = { 6.4 - NewPrismCenterX, 5 - NewPrismCenterY ,0 };
	double forth_3[] = { 6 - NewPrismCenterX, 4 - NewPrismCenterY,0 };

	//4:
	double First_4[] = { 5 - NewPrismCenterX, 3 - NewPrismCenterY ,0 };
	double second_4[] = { 5 - NewPrismCenterX, 4 - NewPrismCenterY,0 };
	double third_4[] = { 6 - NewPrismCenterX, 4 - NewPrismCenterY ,0 };
	double forth_4[] = { 7.3 - NewPrismCenterX, 3 - NewPrismCenterY,0 };

	//1:
	glBegin(GL_QUADS);
	glColor3d(0.5f, 0, 0.5f);
	glTexCoord2d(0, 0);
	glVertex3dv(First_1);
	glTexCoord2d(0, 0.33);
	glVertex3dv(second_1);
	glTexCoord2d(0.33, 0.33);
	glVertex3dv(third_1);
	glTexCoord2d(0.33, 0);
	glVertex3dv(forth_1);
	glEnd();

	//2:
	glBegin(GL_QUADS);
	glColor3d(0.5f, 0, 0.5f);
	glTexCoord2d(0.165, 0.33);
	glVertex3dv(First_2);
	glTexCoord2d(0.3, 1);
	glVertex3dv(second_2);
	glTexCoord2d(0.85, 1);
	glVertex3dv(third_2);
	glTexCoord2d(0.58, 0.33);
	glVertex3dv(forth_2);
	glEnd();

	//3:
	glBegin(GL_QUADS);
	glColor3d(0.5f, 0, 0.5f);
	glTexCoord2d(0.33, 0.16);
	glVertex3dv(First_3);
	glTexCoord2d(0.33, 0.33);
	glVertex3dv(second_3);
	glTexCoord2d(0.55, 0.33);
	glVertex3dv(third_3);
	glTexCoord2d(0.5, 0.16);
	glVertex3dv(forth_3);
	glEnd();

	//4:
	glBegin(GL_QUADS);
	glColor3d(0.5f, 0, 0.5f);
	glTexCoord2d(0.33, 0);
	glVertex3dv(First_4);
	glTexCoord2d(0.33, 0.16);
	glVertex3dv(second_4);
	glTexCoord2d(0.5, 0.16);
	glVertex3dv(third_4);
	glTexCoord2d(0.71, 0);
	glVertex3dv(forth_4);
	glEnd();
	
	//
	Triangle_Down_Light(C, F, H);
	Triangle_Down_Light(C, D, F);
	Triangle_Down_Light(F, D, E);
	Triangle_Down_Light(F, H, G);





	//Верхнее основание: 
	//Triangle(A1, B1, H1);
	//Triangle(B1, C1, H1);
	/*Triangle_Up_Light(C1, F1, H1);
	Triangle_Up_Light(C1, D1, F1);
	Triangle_Up_Light(F1, D1, E1);
	Triangle_Up_Light(F1, H1, G1);*/





	//Построение выпуклости и вогнутости
	Convexity_1(height);
	Convexity_2(height);
	Concavity(height);
	Convexity_1_Alpha_Triangles(height);
	Convexity_2_Alpha_Triangles(height);
	Concavity_Alpha_Triangles(height);

	Triangle_Up_Light(C1, F1, H1);
	Triangle_Up_Light(C1, D1, F1);
	Triangle_Up_Light(F1, D1, E1);
	Triangle_Up_Light(F1, H1, G1);
}


//Выпуклость
void Convexity_1(double height) {


	int k = 240;
	double NewPrismCenterX = 7.29 - double(49916) / double(9375); //Центр фигуры по координате Х. Посчитал руками. 
	double NewPrismCenterY = 9.4 - double(29889) / double(6250); //Центр фигуры по координате Y. Посчитал руками. 
	double const pi = 3.14159;
	double beta = 2 * pi / k; //angle beta
	double radius = sqrt(22) / 2;; //radius of the circle


	//Нижнее основание:
	double x_center = NewPrismCenterX;
	double y_center = NewPrismCenterY;
	double x = x_center;
	double y = y_center + radius;
	double x_next;
	double y_next;



	//Верхнее основание:
	double x_top_center = NewPrismCenterX;
	double y_top_center = NewPrismCenterY;
	double x_top = x_top_center;
	double y_top = y_top_center + radius;
	double x_next_top;
	double y_next_top;



	double AngleCoefficient_1 = 1.25;


	double N_Vector[3];
	double A[3];
	double A1[3];
	double B[3];
	double B1[3];
	double N_X;
	double N_Y;
	double N_Z;

	//glBindTexture(GL_TEXTURE_2D, texId);


	double tx0 = 0;
	double ty0 = 0;

	for (int i = 0; i <= k;i++) {



		//Нижний круг
		x_next = x_center + radius * sin(beta * i);
		y_next = y_center + radius * cos(beta * i);
		if (i >= k / AngleCoefficient_1 && i <= k) {
			
			glBegin(GL_TRIANGLES);
			glColor3d(0.5f, 0.0f, 0.5f);
			glNormal3f(0, 0, -1);
			glVertex3d(NewPrismCenterX, NewPrismCenterY, 0);
			glVertex3d(x, y, 0);
			glVertex3d(x_next, y_next, 0);
			glEnd();
			
		}



		//Верхний круг
		x_next_top = x_top_center + radius * sin(beta * i);
		y_next_top = y_top_center + radius * cos(beta * i);
		if (i >= k / AngleCoefficient_1 && i <= k) {
			/*glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBegin(GL_TRIANGLES);
			glNormal3f(0, 0, 1);
			glColor4f(0.5f, 0.0f, 0.5f, 0.7f);
			glVertex3d(NewPrismCenterX, NewPrismCenterY, height);
			glVertex3d(x_top, y_top, height);
			glVertex3d(x_next_top, y_next_top, height);
			glEnd();
			glDisable(GL_BLEND);*/
		}

		A[0] = x;
		A[1] = y;
		A[2] = 0;
		A1[0] = x_top;
		A1[1] = y_top;
		A1[2] = height;
		B1[0] = x_next_top;
		B1[1] = y_next_top;
		B1[2] = height;
		B[0] = x_next;
		B[1] = y_next;
		B[2] = 0;

		N_X = N_Vector_X(A, A1, B, height);
		N_Y = N_Vector_Y(A, A1, B, height);
		N_Z = N_Vector_Z(A, A1, B, height);

		
		//Четырёхугольники
		if (i >= 190 && i <= 240) {
			double tx = tx0 + 1.0 / 83.0;
			double ty = ty0 + 1.0 / 83.0;
			glBegin(GL_QUADS);
			glColor3f(1, 1, 0);
			glNormal3f(N_X, N_Y, N_Z);
			glTexCoord2d(tx0, 0);
			glVertex3d(x, y, 0);
			glTexCoord2d(ty0, 1);
			glVertex3d(x_top, y_top, height);
			glTexCoord2d(ty, 1);
			glVertex3d(x_next_top, y_next_top, height);
			glTexCoord2d(tx, 0);
			glVertex3d(x_next, y_next, 0);
			glEnd();
			tx0 = tx;
			ty0 = ty;
		}


	
		x = x_next;
		y = y_next;
		x_top = x_next_top;
		y_top = y_next_top;



	}





}

void Convexity_1_Alpha_Triangles(double height) {


	int k = 240;
	double NewPrismCenterX = 7.29 - double(49916) / double(9375); //Центр фигуры по координате Х. Посчитал руками. 
	double NewPrismCenterY = 9.4 - double(29889) / double(6250); //Центр фигуры по координате Y. Посчитал руками. 
	double const pi = 3.14159;
	double beta = 2 * pi / k; //angle beta
	double radius = sqrt(22) / 2;; //radius of the circle


	//Нижнее основание:
	double x_center = NewPrismCenterX;
	double y_center = NewPrismCenterY;
	double x = x_center;
	double y = y_center + radius;
	double x_next;
	double y_next;



	//Верхнее основание:
	double x_top_center = NewPrismCenterX;
	double y_top_center = NewPrismCenterY;
	double x_top = x_top_center;
	double y_top = y_top_center + radius;
	double x_next_top;
	double y_next_top;



	double AngleCoefficient_1 = 1.25;


	double N_Vector[3];
	double A[3];
	double A1[3];
	double B[3];
	double B1[3];
	double N_X;
	double N_Y;
	double N_Z;

	//glBindTexture(GL_TEXTURE_2D, texId);


	for (int i = 0; i <= k;i++) {



		//Нижний круг
		x_next = x_center + radius * sin(beta * i);
		y_next = y_center + radius * cos(beta * i);
		if (i >= k / AngleCoefficient_1 && i <= k) {

			/*glBegin(GL_TRIANGLES);
			glColor3d(0.5f, 0.0f, 0.5f);
			glNormal3f(0, 0, -1);
			glVertex3d(NewPrismCenterX, NewPrismCenterY, 0);
			glVertex3d(x, y, 0);
			glVertex3d(x_next, y_next, 0);
			glEnd();*/

		}



		//Верхний круг
		x_next_top = x_top_center + radius * sin(beta * i);
		y_next_top = y_top_center + radius * cos(beta * i);
		if (i >= k / AngleCoefficient_1 && i <= k) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBegin(GL_TRIANGLES);
			glNormal3f(0, 0, 1);
			glColor4f(0.5f, 0.0f, 0.5f, 0.7f);
			glVertex3d(NewPrismCenterX, NewPrismCenterY, height);
			glVertex3d(x_top, y_top, height);
			glVertex3d(x_next_top, y_next_top, height);
			glEnd();
			glDisable(GL_BLEND);
		}

		A[0] = x;
		A[1] = y;
		A[2] = 0;
		A1[0] = x_top;
		A1[1] = y_top;
		A1[2] = height;
		B1[0] = x_next_top;
		B1[1] = y_next_top;
		B1[2] = height;
		B[0] = x_next;
		B[1] = y_next;
		B[2] = 0;

		N_X = N_Vector_X(A, A1, B, height);
		N_Y = N_Vector_Y(A, A1, B, height);
		N_Z = N_Vector_Z(A, A1, B, height);


		//Четырёхугольники
		if (i >= k / AngleCoefficient_1 && i <= k) {
			/*glBegin(GL_QUADS);
			glColor3f(0, 1, 0);
			glNormal3f(N_X, N_Y, N_Z);
			glVertex3d(x, y, 0);
			glVertex3d(x_top, y_top, height);
			glVertex3d(x_next_top, y_next_top, height);
			glVertex3d(x_next, y_next, 0);
			glEnd();*/
		}



		x = x_next;
		y = y_next;
		x_top = x_next_top;
		y_top = y_next_top;



	}





}


//Вторая часть выпуклости
void Convexity_2(double height) {


	int k = 240;
	double NewPrismCenterX = 7.29 - double(49916) / double(9375); //Центр фигуры по координате Х. Посчитал руками. 
	double NewPrismCenterY = 9.4 - double(29889) / double(6250); //Центр фигуры по координате Y. Посчитал руками. 
	double const pi = 3.14159;
	double beta = 2 * pi / k; //angle beta
	double radius = sqrt(22) / 2;; //radius of the circle



	//Нижнее основание:
	double x_center = NewPrismCenterX;
	double y_center = NewPrismCenterY;
	double x = x_center;
	double y = y_center + radius;
	double x_next;
	double y_next;



	//Верхнее основание:
	double x_top_center = NewPrismCenterX;
	double y_top_center = NewPrismCenterY;
	double x_top = x_top_center;
	double y_top = y_top_center + radius;
	double x_next_top;
	double y_next_top;

	double AngleCoefficient_1 = 1.32;

	double A[3];
	double A1[3];
	double B[3];
	double B1[3];
	double N_X;
	double N_Y;
	double N_Z;

	double tx0 = 51.0 / 83.0;
	double ty0 = 51.0 / 83.0;

	for (int i = 0; i <= k;i++) {



		//Нижний круг
		x_next = x_center + radius * sin(beta * i);
		y_next = y_center + radius * cos(beta * i);
		if (i >= k / 240 && i <= k / 7.5) {
			glBegin(GL_TRIANGLES);
			glColor3d(0.5f, 0.0f, 0.5f);
			glNormal3f(0, 0, -1);
			glVertex3d(NewPrismCenterX, NewPrismCenterY, 0);
			glVertex3d(x, y, 0);
			glVertex3d(x_next, y_next, 0);
			glEnd();
		}



		//Верхний круг
		x_next_top = x_top_center + radius * sin(beta * i);
		y_next_top = y_top_center + radius * cos(beta * i);
		if (i >= k / 240 && i <= k / 7.5) {
			/*glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBegin(GL_TRIANGLES);
			glColor4f(0.5f, 0.0f, 0.5f, 0.7f);
			glNormal3f(0, 0, 1);
			glVertex3d(NewPrismCenterX, NewPrismCenterY, height);
			glVertex3d(x_top, y_top, height);
			glVertex3d(x_next_top, y_next_top, height);
			glEnd();
			glDisable(GL_BLEND);*/
		}

		A[0] = x;
		A[1] = y;
		A[2] = 0;
		A1[0] = x_top;
		A1[1] = y_top;
		A1[2] = height;
		B1[0] = x_next_top;
		B1[1] = y_next_top;
		B1[2] = height;
		B[0] = x_next;
		B[1] = y_next;
		B[2] = 0;

		N_X = N_Vector_X(A, A1, B, height);
		N_Y = N_Vector_Y(A, A1, B, height);
		N_Z = N_Vector_Z(A, A1, B, height);

		//Четырёхугольники
		if (i >= k / 240 && i <= k / 7.5) {
			double tx = tx0 + 1.0 / 81.0;
			double ty = ty0 + 1.0 / 81.0;
			glBegin(GL_QUADS);
			glColor3f(1, 1, 0);
			glNormal3f(N_X, N_Y, N_Z);
			glTexCoord2d(tx0, 0);
			glVertex3d(x, y, 0);
			glTexCoord2d(ty0, 1);
			glVertex3d(x_top, y_top, height);
			glTexCoord2d(ty, 1);
			glVertex3d(x_next_top, y_next_top, height);
			glTexCoord2d(tx, 0);
			glVertex3d(x_next, y_next, 0);
			glEnd();
			tx0 = tx;
			ty0 = ty;
		}



		x = x_next;
		y = y_next;
		x_top = x_next_top;
		y_top = y_next_top;



	}




}

void Convexity_2_Alpha_Triangles(double height) {


	int k = 240;
	double NewPrismCenterX = 7.29 - double(49916) / double(9375); //Центр фигуры по координате Х. Посчитал руками. 
	double NewPrismCenterY = 9.4 - double(29889) / double(6250); //Центр фигуры по координате Y. Посчитал руками. 
	double const pi = 3.14159;
	double beta = 2 * pi / k; //angle beta
	double radius = sqrt(22) / 2;; //radius of the circle



	//Нижнее основание:
	double x_center = NewPrismCenterX;
	double y_center = NewPrismCenterY;
	double x = x_center;
	double y = y_center + radius;
	double x_next;
	double y_next;



	//Верхнее основание:
	double x_top_center = NewPrismCenterX;
	double y_top_center = NewPrismCenterY;
	double x_top = x_top_center;
	double y_top = y_top_center + radius;
	double x_next_top;
	double y_next_top;

	double AngleCoefficient_1 = 1.32;

	double A[3];
	double A1[3];
	double B[3];
	double B1[3];
	double N_X;
	double N_Y;
	double N_Z;

	for (int i = 0; i <= k;i++) {



		//Нижний круг
		x_next = x_center + radius * sin(beta * i);
		y_next = y_center + radius * cos(beta * i);
		if (i >= k / 240 && i <= k / 7.5) {
			/*glBegin(GL_TRIANGLES);
			glColor3d(0.5f, 0.0f, 0.5f);
			glNormal3f(0, 0, -1);
			glVertex3d(NewPrismCenterX, NewPrismCenterY, 0);
			glVertex3d(x, y, 0);
			glVertex3d(x_next, y_next, 0);
			glEnd();*/
		}



		//Верхний круг
		x_next_top = x_top_center + radius * sin(beta * i);
		y_next_top = y_top_center + radius * cos(beta * i);
		if (i >= k / 240 && i <= k / 7.5) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBegin(GL_TRIANGLES);
			glColor4f(0.5f, 0.0f, 0.5f, 0.7f);
			glNormal3f(0, 0, 1);
			glVertex3d(NewPrismCenterX, NewPrismCenterY, height);
			glVertex3d(x_top, y_top, height);
			glVertex3d(x_next_top, y_next_top, height);
			glEnd();
			glDisable(GL_BLEND);
		}

		A[0] = x;
		A[1] = y;
		A[2] = 0;
		A1[0] = x_top;
		A1[1] = y_top;
		A1[2] = height;
		B1[0] = x_next_top;
		B1[1] = y_next_top;
		B1[2] = height;
		B[0] = x_next;
		B[1] = y_next;
		B[2] = 0;

		N_X = N_Vector_X(A, A1, B, height);
		N_Y = N_Vector_Y(A, A1, B, height);
		N_Z = N_Vector_Z(A, A1, B, height);

		//Четырёхугольники
		if (i >= k / 240 && i <= k / 7.5) {
			/*glBegin(GL_QUADS);
			glColor3f(0, 1, 0);
			glNormal3f(N_X, N_Y, N_Z);
			glVertex3d(x, y, 0);
			glVertex3d(x_top, y_top, height);
			glVertex3d(x_next_top, y_next_top, height);
			glVertex3d(x_next, y_next, 0);
			glEnd();*/
		}



		x = x_next;
		y = y_next;
		x_top = x_next_top;
		y_top = y_next_top;



	}




}

//Вогнутость
void Concavity(double height) {

	int k = 240;
	double NewPrismCenterX = 0 - double(49916) / double(9375); //Центр фигуры по координате Х. Посчитал руками. 
	double NewPrismCenterY = 2.1 - double(29889) / double(6250); //Центр фигуры по координате Y. Посчитал руками. 
	double const pi = 3.14159;
	double beta = 2 * pi / k; //angle beta
	double radius = sqrt(33.1) / 2;; //radius of the circle

	//Нижнее основание:
	double x_center = NewPrismCenterX;
	double y_center = NewPrismCenterY;
	double x = x_center;
	double y = y_center + radius;
	double x_next;
	double y_next;

	//Верхнее основание:
	double x_top_center = NewPrismCenterX;
	double y_top_center = NewPrismCenterY;
	double x_top = x_top_center;
	double y_top = y_top_center + radius;
	double x_next_top;
	double y_next_top;


	double A[3];
	double A1[3];
	double B[3];
	double B1[3];
	double N_X;
	double N_Y;
	double N_Z;


	double tx0 = 0;
	double ty0 = 0;

	//glBindTexture(GL_TEXTURE_2D, texId);

	for (int i = 0; i <= k;i++) {



		x_next = x_center + radius * sin(beta * i);
		y_next = y_center + radius * cos(beta * i);
		x_next_top = x_top_center + radius * sin(beta * i);
		y_next_top = y_top_center + radius * cos(beta * i);


		A[0] = x;
		A[1] = y;
		A[2] = 0;
		A1[0] = x_top;
		A1[1] = y_top;
		A1[2] = height;
		B1[0] = x_next_top;
		B1[1] = y_next_top;
		B1[2] = height;
		B[0] = x_next;
		B[1] = y_next;
		B[2] = 0;

		N_X = N_Vector_X(A, A1, B, height);
		N_Y = N_Vector_Y(A, A1, B, height);
		N_Z = N_Vector_Z(A, A1, B, height);
		

		
		//Четырёхугольники
		if (i >= 4 && i <= 92) { 
			double tx = tx0 + 1.0 / 88;
			double ty = ty0 + 1.0 / 88;
			glBegin(GL_QUADS);
			glColor3f(1, 1, 0);
			glNormal3f(-N_X, -N_Y, -N_Z);
			glTexCoord2d(tx0, 0);
			glVertex3d(x, y, 0);
			glTexCoord2d(ty0, 1);
			glVertex3d(x_top, y_top, height);
			glTexCoord2d(ty, 1);
			glVertex3d(x_next_top, y_next_top, height);
			glTexCoord2d(tx, 0);
			glVertex3d(x_next, y_next, 0);
			glEnd();

			if (i >= k / 60 && i <= k / 5) {
				/*glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_TRIANGLES);
				glColor3d(0.5f, 0.0f, 0.5f);
				glNormal3f(0, 0, 1);
				glVertex3d(x, y, height);
				glVertex3d(x_next, y_next, height);
				glVertex3d(0, 0.28, height);
				glEnd();
				glDisable(GL_BLEND);*/
			}

			if (i >= k / 60 && i <= k / 5) {
				glBegin(GL_TRIANGLES);
				glColor3d(0.5f, 0.0f, 0.5f);
				glNormal3f(0, 0, -1);
				glVertex3d(x, y, 0);
				glVertex3d(x_next, y_next, 0);
				glVertex3d(0, 0.28, 0);
				glEnd();
			}

			if (i >= k / 15 && i <= k / 1.3) {
				/*glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_TRIANGLES);
				glColor4f(0.5f, 0.0f, 0.5f, 0.7f);
				glNormal3f(0, 0, 1);
				glVertex3d(x, y, height);
				glVertex3d(x_next, y_next, height);
				glVertex3d(0, -1.5, height);
				glEnd();
				glDisable(GL_BLEND);*/
			}

			if (i >= k / 15 && i <= k / 1.3) {
				glBegin(GL_TRIANGLES);
				glColor3d(0.5f, 0.0f, 0.5f);
				glNormal3f(0, 0, -1);
				glVertex3d(x, y, 0);
				glVertex3d(x_next, y_next, 0);
				glVertex3d(0, -1.5, 0);
				glEnd();
			}

			if (i >= k / 15 && i <= k / 1.3) {
				/*glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_TRIANGLES);
				glColor4f(0.5f, 0.0f, 0.5f, 0.7f);
				glNormal3f(0, 0, 1);
				glVertex3d(x, y, height);
				glVertex3d(x_next, y_next, height);
				glVertex3d(0, -0.6, height);
				glEnd();
				glDisable(GL_BLEND);*/
			}

			if (i >= k / 15 && i <= k / 1.3) {
				glBegin(GL_TRIANGLES);
				glColor3d(0.5f, 0.0f, 0.5f);
				glNormal3f(0, 0, -1);
				glVertex3d(x, y, 0);
				glVertex3d(x_next, y_next, 0);
				glVertex3d(0, -0.6, 0);
				glEnd();
			}
			tx0 = tx;
			ty0 = ty;
		}

		
		x = x_next;
		y = y_next;
		x_top = x_next_top;
		y_top = y_next_top;
		



	}





}

void Concavity_Alpha_Triangles(double height) {

	int k = 240;
	double NewPrismCenterX = 0 - double(49916) / double(9375); //Центр фигуры по координате Х. Посчитал руками. 
	double NewPrismCenterY = 2.1 - double(29889) / double(6250); //Центр фигуры по координате Y. Посчитал руками. 
	double const pi = 3.14159;
	double beta = 2 * pi / k; //angle beta
	double radius = sqrt(33.1) / 2;; //radius of the circle

	//Нижнее основание:
	double x_center = NewPrismCenterX;
	double y_center = NewPrismCenterY;
	double x = x_center;
	double y = y_center + radius;
	double x_next;
	double y_next;

	//Верхнее основание:
	double x_top_center = NewPrismCenterX;
	double y_top_center = NewPrismCenterY;
	double x_top = x_top_center;
	double y_top = y_top_center + radius;
	double x_next_top;
	double y_next_top;


	double A[3];
	double A1[3];
	double B[3];
	double B1[3];
	double N_X;
	double N_Y;
	double N_Z;


	double tx0 = 0;
	double ty0 = 0;

	//glBindTexture(GL_TEXTURE_2D, texId);

	for (int i = 0; i <= k;i++) {



		x_next = x_center + radius * sin(beta * i);
		y_next = y_center + radius * cos(beta * i);
		x_next_top = x_top_center + radius * sin(beta * i);
		y_next_top = y_top_center + radius * cos(beta * i);


		A[0] = x;
		A[1] = y;
		A[2] = 0;
		A1[0] = x_top;
		A1[1] = y_top;
		A1[2] = height;
		B1[0] = x_next_top;
		B1[1] = y_next_top;
		B1[2] = height;
		B[0] = x_next;
		B[1] = y_next;
		B[2] = 0;

		N_X = N_Vector_X(A, A1, B, height);
		N_Y = N_Vector_Y(A, A1, B, height);
		N_Z = N_Vector_Z(A, A1, B, height);

		double tx0 = 0;
		double ty0 = 0;
		double tx = tx0 + 1.0 / 88;
		double ty = ty0 + 1.0 / 88;
		//Четырёхугольники
		if (i >= k / 60 && i <= k / 2.6) {
			glBegin(GL_QUADS);
			glColor3f(0, 0, 1);
			glNormal3f(-N_X, -N_Y, -N_Z);
			glTexCoord2d(tx0, 0);
			glVertex3d(x, y, 0);
			glTexCoord2d(tx, 0);
			glVertex3d(x_top, y_top, height);
			glTexCoord2d(ty, 1);
			glVertex3d(x_next_top, y_next_top, height);
			glTexCoord2d(ty0, 1);
			glVertex3d(x_next, y_next, 0);
			glEnd();

			if (i >= k / 60 && i <= k / 5) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_TRIANGLES);
				glColor4f(0.5f, 0.0f, 0.5f,0.7f);
				glNormal3f(0, 0, 1);
				glVertex3d(x, y, height);
				glVertex3d(x_next, y_next, height);
				glVertex3d(0, 0.28, height);
				glEnd();
				glDisable(GL_BLEND);
			}

			if (i >= k / 60 && i <= k / 5) {
				glBegin(GL_TRIANGLES);
				glColor3d(0.5f, 0.0f, 0.5f);
				glNormal3f(0, 0, -1);
				glVertex3d(x, y, 0);
				glVertex3d(x_next, y_next, 0);
				glVertex3d(0, 0.28, 0);
				glEnd();
			}

			if (i >= k / 15 && i <= k / 1.3) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_TRIANGLES);
				glColor4f(0.5f, 0.0f, 0.5f, 0.7f);
				glNormal3f(0, 0, 1);
				glVertex3d(x, y, height);
				glVertex3d(x_next, y_next, height);
				glVertex3d(0, -1.5, height);
				glEnd();
				glDisable(GL_BLEND);
			}

			if (i >= k / 15 && i <= k / 1.3) {
				glBegin(GL_TRIANGLES);
				glColor3d(0.5f, 0.0f, 0.5f);
				glNormal3f(0, 0, -1);
				glVertex3d(x, y, 0);
				glVertex3d(x_next, y_next, 0);
				glVertex3d(0, -1.5, 0);
				glEnd();
			}

			if (i >= k / 15 && i <= k / 1.3) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_TRIANGLES);
				glColor4f(0.5f, 0.0f, 0.5f, 0.7f);
				glNormal3f(0, 0, 1);
				glVertex3d(x, y, height);
				glVertex3d(x_next, y_next, height);
				glVertex3d(0, -0.6, height);
				glEnd();
				glDisable(GL_BLEND);
			}

			if (i >= k / 15 && i <= k / 1.3) {
				glBegin(GL_TRIANGLES);
				glColor3d(0.5f, 0.0f, 0.5f);
				glNormal3f(0, 0, -1);
				glVertex3d(x, y, 0);
				glVertex3d(x_next, y_next, 0);
				glVertex3d(0, -0.6, 0);
				glEnd();
			}

		}
		tx0 = tx;
		ty0 = ty;
		x = x_next;
		y = y_next;
		x_top = x_next_top;
		y_top = y_next_top;




	}





}





void circle() {
	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_POLYGON);
	for (double i = 0; i <= 2; i += 0.01)
	{
		double x = 9 * cos(i * 3.141593);
		double y = 9 * sin(i * 3.141593);

		double tx = cos(i * 3.141593) * 0.5 + 0.5;
		double ty = sin(i * 3.141593) * 0.5 + 0.5;

		glColor3d(0.5f, 0.5f, 0.5f);
		glNormal3d(0, 0, 1);
		glTexCoord2d(tx, ty);
		glVertex3d(x, y, 0);

	}
	glEnd();
}



void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	if (textureReplace) {
		glBindTexture(GL_TEXTURE_2D, texId);
	}
	if(!textureReplace){
		glBindTexture(GL_TEXTURE_2D, texId2);
	}


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут 
	
		

	Figure(3);
	//circle();
	


	//Начало рисования квадратика станкина
	/*double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };

	glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.6, 0.6, 0.6);
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);

	glEnd();*/
	//конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "E - сменить текстуру" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}

