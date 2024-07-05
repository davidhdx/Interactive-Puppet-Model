#include<filesystem>
namespace fs = std::filesystem;

#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<stb/stb_image.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/quaternion.hpp> 
#include<glm/gtx/quaternion.hpp>
#include<cmath>
#include<glm/gtc/constants.hpp>

#include"Texture.h"
#include"shaderClass.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include"Camera.h"


#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

//definimos el nivel de discretizacion de nuestra elipse
#define disc 4

//usamos macros para no tener que recordar los indices de los datos de cada parte del cuerpo
#define linf 0
#define lsup 1
#define lder 2
#define lizq 3
#define lfro 4
#define ltra 5

#define cabezaX 0
#define cabezaY 1
#define cabezaZ 2

#define brazoderX 3
#define brazoderY 4
#define brazoderZ 5

#define brazoizqX 6
#define brazoizqY 7
#define brazoizqZ 8

#define antederX 9
#define antederY 10

#define anteizqX 11
#define anteizqY 12

#define manoderX 13
#define manoderZ 14

#define manoizqX 15
#define manoizqZ 16

#define piernaderX 17
#define piernaderY 18
#define piernaderZ 19

#define piernaizqX 20
#define piernaizqY 21
#define piernaizqZ 22

#define pantoderX 23

#define pantoizqX 24

#define piederX 25
#define piederZ 26

#define pieizqX 27
#define pieizqZ 28

#define n_piezas 18

//definiciones de David
const unsigned int width = 1200;
const unsigned int height = 600;

std::vector <GLfloat> bas = { 0.0f, 0.70f, 0.44f, 1.0f, 1.0f };
const unsigned int puntos_prim = 4 * disc * disc - 2 * disc;
const unsigned int triangulos_prim = 2 * disc * disc - 2 * disc;

/*|AQUI COMIENZA LO QUE DAVID SE PUSO A HACER SIENDO MAS CAFE QUE SER HUMANO| */

std::vector<GLfloat> normal, puntos, vertice;
std::vector<GLuint> indice;
GLuint inicio = 0, cabezaind = 0, cabezaver = 0;
GLfloat vertices[n_piezas * 11 * (4 * disc * disc - 2 * disc)];
GLuint indices[n_piezas * 3 * (2 * disc * disc - 2 * disc)];

//defino los angulos limite de mis extremidades
GLfloat ang_lims[] = {
	//limite de mano
	-90.0f, 90.0f, //limite de movimiento de eje Z
	-45.0f, 45.0f, //limite de movimiento de eje X
	//limite de antebrazo
	0.0f, 135.0f, //limite de movimiento de eje X
	0.0f, 60.f,   //limite de movimiento de eje Y
	//limite de brazo
	-45.0f, 100.0f,//limite de movimiento de eje X
	0.0f, 100.0f,  //limite de movimiento de eje Z
	0.0f, 50.0f,   //limite de movimiento de eje Y

	//limite de pie
	0.0f, 45.0f,   //limite de movimiento de eje X
	-10.0f, 10.0f, //limite de movimiento de eje Z
	//limite de pantorrilla
	0.0f, 120.0f, //limite de movimiento de eje X
	//limite de pierna
	0.0f, 80.0f,  //limite de movimiento de eje X
	0.0f, 45.0f,//limite de movimiento de eje Z
	0.0f, 80.0f   //limite de movimiento de eje Y
};

//calculo el vector normal dado tres puntos
std::vector<GLfloat> calcularNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {

	glm::vec3 U = glm::vec3(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
	glm::vec3 V = glm::vec3(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);

	std::vector<GLfloat> resultado = { -(U.y * V.z - U.z * V.y), -(U.z * V.x - U.x * V.z), -(U.x * V.y - U.y * V.x) };

	return resultado;
}

//con esto puedo desaparecer al muñeco
void cleanBuffer() {
	for (int i = 0; i < n_piezas * 11 * puntos_prim; i++)
		vertices[i] = 0.0;
	for (int i = 0; i < n_piezas * 3 * triangulos_prim; i++)
		indices[i] = 0;
}

//dado un trangulo puedo cargar a mi vector de puntos de la primitiva y a mi vector de indices de la primitiva
void meterTriangulo(unsigned int a, unsigned int b, unsigned int c) {
	a *= 3;
	b *= 3;
	c *= 3;

	glm::vec3
		p1 = glm::vec3(puntos[a], puntos[a + 1], puntos[a + 2]),
		p2 = glm::vec3(puntos[b], puntos[b + 1], puntos[b + 2]),
		p3 = glm::vec3(puntos[c], puntos[c + 1], puntos[c + 2]);

	normal = calcularNormal(p1, p2, p3);

	unsigned int n = (unsigned int)vertice.size() / 11;
	indice.push_back(inicio + n);
	indice.push_back(inicio + n + 1);
	indice.push_back(inicio + n + 2);

	for (int i = 0; i < 3; i++)
		vertice.push_back(puntos[a + i]);
	for (int i = 0; i < bas.size(); i++)
		vertice.push_back(bas[i]);
	for (int i = 0; i < 3; i++)
		vertice.push_back(normal[i]);

	for (int i = 0; i < 3; i++)
		vertice.push_back(puntos[b + i]);
	for (int i = 0; i < bas.size(); i++)
		vertice.push_back(bas[i]);
	for (int i = 0; i < 3; i++)
		vertice.push_back(normal[i]);

	for (int i = 0; i < 3; i++)
		vertice.push_back(puntos[c + i]);
	for (int i = 0; i < bas.size(); i++)
		vertice.push_back(bas[i]);
	for (int i = 0; i < 3; i++)
		vertice.push_back(normal[i]);
}

//con esta funcion meto mi trapezoide al ppuntos y al pindices 
void meterTrapezoide(unsigned int a, unsigned int b, unsigned int c, unsigned int d) {

	meterTriangulo(a, b, c);
	d = 3 * d;

	unsigned int n = vertice.size() / 11;
	indice.push_back(inicio + n - 2);
	indice.push_back(inicio + n - 1);
	indice.push_back(inicio + n);

	//al salir de meterTriangulo ya tenemos precargado el vector normal asi que no hay que reclacularlo
	for (int i = 0; i < 3; i++)
		vertice.push_back(puntos[d + i]);
	for (int i = 0; i < bas.size(); i++)
		vertice.push_back(bas[i]);
	for (int i = 0; i < 3; i++)
		vertice.push_back(normal[i]);


}

//defino cada extremidad con una clase para poder unsar los metodos
class Primitiva {
public:
	//vector orgen de autoreferencia
	glm::vec3 origen = glm::vec3(0.0f, 0.0f, 0.0f);
	//vector propio de los puntos con referencia la variable origen definida
	std::vector<GLfloat> ppuntos;
	std::vector<GLuint> pindices;
	//cantidad de puntos para evitar salirme de los arreglos
	unsigned int cantidad_puntos = 4 * disc * disc - 2 * disc, cantidad_triangulos = 0;
	//cantidad de puntos que debo saltarme del buffer original
	unsigned int pidx = 0, tidx = 0;
	//utilizo una caja (de las que evitan colisiones) para generar los ejes de rotacion
	std::vector<GLuint> caja = {
		0, 4 * disc * disc - 2 * disc - 1, //limites inf y superior
		2 * disc * disc - disc, 2 * disc * disc + disc, //der e izq
		2 * disc * disc - 3, 2 * disc * disc + 2 * disc - 3
	};
	//los ejes
	glm::vec3 axX = glm::vec3(1.0f, 0.0f, 0.0f),
		axY = glm::vec3(0.0f, 1.0f, 0.0f),
		axZ = glm::vec3(0.0f, 0.0f, 1.0f);

	//con esto genero los ejes con respecto a otra primitiva, por ejemplo por mucho que rote mi antebrazo solo lo puedo flexionar con
	//respecto al brazo
	void cargarEjes(Primitiva conexion) {
		GLuint a = caja[lder];
		GLuint b = caja[lizq];
		axX = glm::vec3(
			conexion.ppuntos[11 * a] - conexion.ppuntos[11 * b],
			conexion.ppuntos[11 * a + 1] - conexion.ppuntos[11 * b + 1],
			conexion.ppuntos[11 * a + 2] - conexion.ppuntos[11 * b + 2]);
		a = caja[lsup];
		b = caja[linf];
		axY = glm::vec3(
			conexion.ppuntos[11 * a] - conexion.ppuntos[11 * b],
			conexion.ppuntos[11 * a + 1] - conexion.ppuntos[11 * b + 1],
			conexion.ppuntos[11 * a + 2] - conexion.ppuntos[11 * b + 2]);
		a = caja[lfro];
		b = caja[ltra];
		axZ = glm::vec3(
			conexion.ppuntos[11 * a] - conexion.ppuntos[11 * b],
			conexion.ppuntos[11 * a + 1] - conexion.ppuntos[11 * b + 1],
			conexion.ppuntos[11 * a + 2] - conexion.ppuntos[11 * b + 2]);
		axX = glm::normalize(axX);
		axY = glm::normalize(axY);
		axZ = glm::normalize(axZ);
	}
	//cargo los ejes de forma autoreferenciada
	void cargarEjesP() {
		GLuint a = caja[lder];
		GLuint b = caja[lizq];
		axX = glm::vec3(
			ppuntos[11 * a] - ppuntos[11 * b],
			ppuntos[11 * a + 1] - ppuntos[11 * b + 1],
			ppuntos[11 * a + 2] - ppuntos[11 * b + 2]);
		a = caja[lsup];
		b = caja[linf];
		axY = glm::vec3(
			ppuntos[11 * a] - ppuntos[11 * b],
			ppuntos[11 * a + 1] - ppuntos[11 * b + 1],
			ppuntos[11 * a + 2] - ppuntos[11 * b + 2]);
		a = caja[lfro];
		b = caja[ltra];
		axZ = glm::vec3(
			ppuntos[11 * a] - ppuntos[11 * b],
			ppuntos[11 * a + 1] - ppuntos[11 * b + 1],
			ppuntos[11 * a + 2] - ppuntos[11 * b + 2]);
		axX = glm::normalize(axX);
		axY = glm::normalize(axY);
		axZ = glm::normalize(axZ);
	}
	//funcion para rotar un punto sobre un eje de rotacion y un angulo dado
	void rotatePoint(glm::vec3 RotationAxis, float RotationAngle, glm::vec3& point) {
		//desmadre de cuaternion
		RotationAngle *= 2 * glm::pi<float>() / 360;
		RotationAngle /= 2;
		RotationAxis = glm::normalize(RotationAxis);

		glm::quat Q(glm::cos(RotationAngle), RotationAxis.x * glm::sin(RotationAngle),
			RotationAxis.y * glm::sin(RotationAngle), RotationAxis.z * glm::sin(RotationAngle));
		glm::quat Qinv(Q.w, -Q.x, -Q.y, -Q.z);
		glm::quat Qpoint(0.0, point.x, point.y, point.z);
		glm::quat Npoint = Qinv * Qpoint * Q;

		point = glm::vec3(Npoint.x, Npoint.y, Npoint.z);
	}
	//con esto roto todos los puntos de mi primitiva, solamento aplico RotatePoint en cada punto
	void rotate(glm::vec3 RotationAxis, float RotationAngle) {
		for (int i = 0; i < cantidad_puntos; i++) {
			glm::vec3 point = glm::vec3(ppuntos[11 * i], ppuntos[11 * i + 1], ppuntos[11 * i + 2]);

			rotatePoint(RotationAxis, RotationAngle, point);
			ppuntos[11 * i] = point.x;
			ppuntos[11 * i + 1] = point.y;
			ppuntos[11 * i + 2] = point.z;
		}
	}
	//rotaciones basicas (usando los ejes de autorteferencias) utilizo mejor esta funcion por velocidad de
	//escritura de codigo
	void rot(char ch, float RotationAngle, Primitiva conexion) {
		cargarEjes(conexion);
		switch (ch) {
		case 'X':
			rotate(axX, RotationAngle);
			break;
		case 'Y':
			rotate(axY, RotationAngle);
			break;
		case 'Z':
			rotate(axZ, RotationAngle);
			break;
		}
		buffering();
	}
	//con esta funcion puedo calcular el vector normal (hacia afuera) de un triangulo dado
	std::vector<GLfloat> normalTriangulo(unsigned int a, unsigned int b, unsigned int c) {
		a *= 11;
		b *= 11;
		c *= 11;

		glm::vec3
			p1 = glm::vec3(ppuntos[a] + origen.x, ppuntos[a + 1] + origen.y, ppuntos[a + 2] + origen.z),
			p2 = glm::vec3(ppuntos[b] + origen.x, ppuntos[b + 1] + origen.y, ppuntos[b + 2] + origen.z),
			p3 = glm::vec3(ppuntos[c] + origen.x, ppuntos[c + 1] + origen.y, ppuntos[c + 2] + origen.z);

		return calcularNormal(p1, p2, p3);
	}
	//con esta funcion cargo correctamente todos los vectores normales de mi figura y asi ilumincarse
	//correctamente
	void generarReflejos() {
		for (unsigned int i = 0; i < cantidad_triangulos; i++) {
			unsigned int x[3] = { pindices[3 * i], pindices[3 * i + 1], pindices[3 * i + 2] };
			std::vector<GLfloat> normal = normalTriangulo(x[0], x[1], x[2]);

			if (disc <= i && i < 2 * disc * disc - 3 * disc)
				normal = normalTriangulo(x[i % 2], x[1 - i % 2], x[2]);

			for (int j = 0; j < 3; j++) {
				for (int k = 0; k < 3; k++)
					ppuntos[11 * x[j] + 8 + k] = normal[k];
			}
		}
	}
	//con esta funcion cargo al espacio de memoria correspondiente de los arreglos vertices e indices para
	//poder usarlos en mi imagen
	void buffering() {
		generarReflejos();
		for (int i = 0; i < ppuntos.size() && i < 11 * (4 * disc * disc - 2 * disc); i++) {
			if (i % 11 == 0)
				vertices[11 * pidx + i] = ppuntos[i] + origen.x;
			else if (i % 11 == 1)
				vertices[11 * pidx + i] = ppuntos[i] + origen.y;
			else if (i % 11 == 2)
				vertices[11 * pidx + i] = ppuntos[i] + origen.z;
			else
				vertices[11 * pidx + i] = ppuntos[i];
		}
		for (int i = 0; i < pindices.size() && i < 3 * (2 * disc * disc - 2 * disc); i++) {
			indices[3 * tidx + i] = pidx + pindices[i];
		}
	}
	/*funcion auxiliar al momento de debugear
	void printTriangulos() {
		for (unsigned int i = 0; i < cantidad_triangulos; i++) {
			std::cout << pindices[3 * i] << "-"
				<< pindices[3 * i + 1] << "-"
				<< pindices[3 * i + 2] << "-" << std::endl;
		}
	}*/
	//con esta funcion puedo mover el punto de referencia de mi primitiva
	void trasladarOrigen(glm::vec3 nuevoOrigen) {
		origen = nuevoOrigen;
		buffering();
	}
};

//funcion de creacion de una primitiva dado un origen (x, y, z) y las longitudes espaciales (2a, 2b, 2c)
Primitiva create(GLfloat x, GLfloat y, GLfloat z, float a, float b, float c,
	unsigned int& np, unsigned int& nt) {
	Primitiva resultado;
	resultado.origen = glm::vec3(x, y, z);
	resultado.pidx = np;
	resultado.tidx = nt;
	normal.clear();
	puntos.clear();
	vertice.clear();
	indice.clear();
	//esta funcion genera una elipse 3d hacia arriba a partir del punto  de altura y = 2a, 
	//anchura x = 2b z = 2c y discretizada a disc puntos
	puntos.push_back(0.0);
	puntos.push_back(0.0);
	puntos.push_back(0.0);


	for (int i = 1; i < disc; i++) {
		//esto es para cada cada elipse en la rebanada
		//bi = bi(i), donde bi(i) = 2b/disc sqrt(2i*disc - i**2) 
		double bi = (2 * b / disc) * sqrt(i * disc - i * i);
		//bi = bi(i), donde bi(i) = 2b/disc sqrt(2i*disc - i**2) 
		double ci = (2 * c / disc) * sqrt(i * disc - i * i);

		//calculamos a que altura deberia estar el circulo correspondiente al corte 
		GLfloat yi = (float)(2 * a * i / disc);

		for (int j = 0; j < disc; j++) {

			//dividimos el circulo en disc partes
			//float theta = (2 * M_PI * j) / disc;
			//claculamos los valores de x, y
			GLfloat xj, zj;
			if (j < disc / 2) {
				xj = bi / disc * (disc - 4 * j);
				zj = (ci / disc) * sqrt(8 * j * disc - 16 * j * j);
			}
			else {
				xj = bi / disc * (disc - 4 * (disc - j));
				zj = -(ci / disc) * sqrt(8 * (disc - j) * disc - 16 * (disc - j) * (disc - j));
			}

			//zi = zc + bi * std::sinf(theta);
			//los XYZ
			puntos.push_back(xj);
			puntos.push_back(yi);
			puntos.push_back(zj);
		}
	}
	//los XYZ
	puntos.push_back(0.0);
	puntos.push_back(2 * a);
	puntos.push_back(0.0);

	resultado.caja.push_back(0.0);
	resultado.caja.push_back(2 * a);
	resultado.caja.push_back(0.0);


	//el arreglo de indices ya tiene n triangulos indexados
	//glm::vec3 p1, p2, p3;
	//vemos que cargamos (disc-2)*disc+2 puntos en la Primitiva
	//en esta funcion unimos los puntos del primer nivel con el nivel cero(x,y,z)
	for (int i = 1; i < disc; i++)
		//metemos el triangulo de los puntos 0, i, i+1
		meterTriangulo(0, i + 1, i);

	//nos falto un ultimo triangulo de esta seccion
	meterTriangulo(0, 1, disc);
	unsigned int des;
	GLuint nivel;
	//en este ciclo vamos a crear los triangulos a partir de los puntos en el nivel [i, i+1]
	for (int i = 1; i < disc - 1; i++) {
		//estamos en el nivel i-esimo y calculamos en que indice comienza dicho nivel
		nivel = (i - 1) * disc;
		for (int j = 1; j < disc; j++) {
			//vamos a introducir los triangulos del trapecio con los puntos
			//nivel+j, nivel+j+1, nivel+disc+j, nivel+disc+j+1
			//esto porque nivel+j y nivel+disc+j estan alineados entre ellos
			//esto ocurre con nivel+j+1, nivel+disc+j+1 tambien

			//metemos el trapecio de los puntos nivel+j, nivel+j+1, nivel+disc+j, nivel+disc+j+1
			des = nivel + j;
			meterTrapezoide(des, des + 1, des + disc, des + disc + 1);
		}
		//introducimos un trapezoide que nos falto en el for anterior
		//es con los puntos nivel + 1, nivel + disc, nivel + disc + 1, nivel + 2 * disc
		meterTrapezoide(nivel + 1, nivel + disc + 1, nivel + disc, nivel + 2 * disc);

	}
	//calculamos que indice tiene el ultimo punto y en que indice empieza el 
	//penultimo nivel
	GLuint ultimo = puntos.size() / 3 - 1;
	nivel = ultimo - disc - 1;
	for (int i = 1; i < disc; i++)
		//metemos el triangulo de los puntos nivel+i, nivel+i+1, ultimo
		meterTriangulo(nivel + i, nivel + i + 1, ultimo);

	//metemos el triangulo de los puntos nivel+1, nivel+disc, ultimo
	meterTriangulo(nivel + disc, nivel + 1, ultimo);

	//guardamos todo en la respuesta del retorno
	resultado.ppuntos = vertice;
	resultado.pindices = indice;
	resultado.cantidad_puntos = vertice.size() / 11;
	resultado.cantidad_triangulos = indice.size() / 3;
	np += resultado.cantidad_puntos;
	nt += resultado.cantidad_triangulos;
	return resultado;
}

//esta funcion evita aplicar una rotacion fuera de los limites de angulos recibidos como parametro
GLfloat rotLimit(GLfloat angleC, GLfloat angleI, GLfloat angleL, GLfloat angleS) {
	angleI -= angleC;
	GLfloat angleN = angleC + angleI;
	if (angleL > angleN)
		angleI = angleL - angleC;
	if (angleS < angleN)
		angleI = angleS - angleC;
	return angleI;
};

/*A continuacion voy a listar las clases correpondientes a:
*cabeza
*antebrazo
*brazo
*chamorro
*pierna
*marioneta
Donde todas poseen (en esencia) los mismo metodos, sin embargo pensarlo de esta manera me facilito
el transferir las rotaciones de una parte del cuerpo a sus extremidades correspondientes de forma
sistematica
**/


//clase cabeza que contiene a un craneo y cabeza para su correcto movimiento
class Cabeza {
public:
	//las Primitivas individuales
	Primitiva craneo, nariz;
	//uniones hacia la nariz distintiva
	GLfloat u_nariz_x = 0.0, u_nariz_y = 0.0, u_nariz_z = 0.0, nariz_tam = 0.0;
	//vectores de rotacion
	glm::vec3 val_rot = glm::vec3(0.0, 0.0, 0.0);
	std::vector<GLfloat> lim_rot = {
		-30.0f, 30.0f, //X
		-30.0f, 30.0f, //Y
		-30.0f, 30.0f  //Z
	};

	Cabeza() {

	}

	Cabeza(GLfloat x, GLfloat y, GLfloat z, float a, float b, float c,
		unsigned int& np, unsigned int& nt) {
		craneo = create(x, y, z, a, b, c, np, nt);
		updArticulacion();
		nariz_tam = 2 * b / 5;
		nariz = create(u_nariz_x, u_nariz_y, u_nariz_z, nariz_tam / 2, nariz_tam / 2, nariz_tam / 2, np, nt);
		nariz.rot('Z', 180, nariz);
	}

	void updArticulacion() {
		GLuint nariz = 11 * craneo.caja[lfro];
		u_nariz_x = craneo.origen.x + craneo.ppuntos[nariz];
		u_nariz_y = craneo.origen.y + craneo.ppuntos[nariz + 1];
		u_nariz_z = craneo.origen.z + craneo.ppuntos[nariz + 2];
	}

	void buffering() {
		nariz.buffering();
		craneo.buffering();
	}

	void trasladar(glm::vec3 nuevoOrigen) {
		craneo.trasladarOrigen(nuevoOrigen);
		updArticulacion();
		nariz.trasladarOrigen(glm::vec3(u_nariz_x, u_nariz_y, u_nariz_z));
	}

	void rotate(glm::vec3 RotationAxis, float RotationAngle) {
		craneo.rotate(RotationAxis, RotationAngle);
		nariz.trasladarOrigen(craneo.origen);
		nariz.rotate(RotationAxis, RotationAngle);
		updArticulacion();
		nariz.trasladarOrigen(glm::vec3(u_nariz_x, u_nariz_y, u_nariz_z));
	}

	void flexion(GLfloat ang, Primitiva cuello) {
		//flexionamos en angulo de [-110,10]
		//negativo = jalon
		//positivo = empujon
		ang = rotLimit(val_rot.x, ang, lim_rot[0], lim_rot[1]);

		val_rot.x += ang;
		//en realidad va a ser hombro
		craneo.rot('X', ang, cuello);
		nariz.trasladarOrigen(craneo.origen);
		nariz.rotate(craneo.axX, ang);

		updArticulacion();

		nariz.trasladarOrigen(glm::vec3(u_nariz_x, u_nariz_y, u_nariz_z));
	}
	void rotacion(GLfloat ang, Primitiva cuello) {
		//flexionamos en angulo de [-110,10]
		//negativo = jalon
		//positivo = empujon
		ang = rotLimit(val_rot.y, ang, lim_rot[2], lim_rot[3]);

		val_rot.y += ang;
		//en realidad va a ser hombro
		craneo.rot('Y', ang, cuello);
		nariz.trasladarOrigen(craneo.origen);
		nariz.rotate(craneo.axY, ang);

		updArticulacion();

		nariz.trasladarOrigen(glm::vec3(u_nariz_x, u_nariz_y, u_nariz_z));
	}
	void negacion(GLfloat ang, Primitiva cuello) {
		//flexionamos en angulo de [-110,10]
		//negativo = jalon
		//positivo = empujon
		ang = rotLimit(val_rot.z, ang, lim_rot[4], lim_rot[5]);

		val_rot.z += ang;
		//en realidad va a ser hombro
		craneo.rot('Z', ang, cuello);
		nariz.trasladarOrigen(craneo.origen);
		nariz.rotate(craneo.axZ, ang);

		updArticulacion();

		nariz.trasladarOrigen(glm::vec3(u_nariz_x, u_nariz_y, u_nariz_z));
	}
};

//el antebrazo domina unicamente a sí mismo y a la mano
class Antebrazo {
public:
	Primitiva antebrazo, mano;
	GLfloat u_muneca_x = 0.0, u_muneca_y = 0.0, u_muneca_z = 0.0;
	glm::vec3 val_rot = glm::vec3(0.0, 0.0, 0.0);
	std::vector<GLfloat> lim_rot = {
		0.0f, 135.0f, //limite de movimiento de eje X
		0.0f, 180.f,
	};

	Antebrazo() {

	}

	Antebrazo(GLfloat x, GLfloat y, GLfloat z, float a, float b, float c,
		unsigned int& np, unsigned int& nt) {
		antebrazo = create(x, y, z, a, b, c, np, nt);
		updArticulacion();

		mano = create(u_muneca_x, u_muneca_y, u_muneca_z, a / 2.5, b, c / 2, np, nt);
	}

	void updArticulacion() {
		GLuint muneca = 11 * antebrazo.caja[lsup];
		u_muneca_x = antebrazo.origen.x + antebrazo.ppuntos[muneca];
		u_muneca_y = antebrazo.origen.y + antebrazo.ppuntos[muneca + 1];
		u_muneca_z = antebrazo.origen.z + antebrazo.ppuntos[muneca + 2];
	}

	void buffering() {
		mano.buffering();
		antebrazo.buffering();
	}

	void trasladar(glm::vec3 nuevoOrigen) {
		antebrazo.trasladarOrigen(nuevoOrigen);
		updArticulacion();
		mano.trasladarOrigen(glm::vec3(u_muneca_x, u_muneca_y, u_muneca_z));
	}

	void rotate(glm::vec3 RotationAxis, float RotationAngle) {
		antebrazo.rotate(RotationAxis, RotationAngle);
		mano.trasladarOrigen(antebrazo.origen);
		mano.rotate(RotationAxis, RotationAngle);
		updArticulacion();
		mano.trasladarOrigen(glm::vec3(u_muneca_x, u_muneca_y, u_muneca_z));
	}

	void flexion(GLfloat ang, Primitiva brazo) {
		//flexionamos en angulo de [-90,0]
		//negativo = jalon
		ang = rotLimit(val_rot.x, ang, lim_rot[0], lim_rot[1]);

		val_rot.x += ang;
		ang = -ang;
		antebrazo.rot('X', ang, brazo);

		mano.trasladarOrigen(antebrazo.origen);
		mano.rotate(antebrazo.axX, ang);

		updArticulacion();
		mano.trasladarOrigen(glm::vec3(u_muneca_x, u_muneca_y, u_muneca_z));
	}

	void rotacion(char ch, GLfloat ang, Primitiva brazo) {
		//rotamos en angulo de [-90,90]
		//negativo = horario
		//positivo = antihorario
		ang = rotLimit(val_rot.y, ang, lim_rot[2], lim_rot[3]);

		val_rot.y += ang;
		if (ch == 'D')
			ang = -ang;
		antebrazo.rot('Y', ang, brazo);
		mano.trasladarOrigen(antebrazo.origen);
		mano.rotate(antebrazo.axY, ang);

		updArticulacion();

		mano.trasladarOrigen(glm::vec3(u_muneca_x, u_muneca_y, u_muneca_z));
	}

	void setAnglesPred() {
		val_rot = glm::vec3(0.0, 0.0, 0.0);
		for (int i = 0; i < 4; i++)
			lim_rot[i] = ang_lims[4 + i];
	}
};

//el brazo domina a si mismo, al antebrazo y a la mano
class Brazo : public Antebrazo {
public:
	//las Primitivas individuales
	Antebrazo antebrazo;
	Primitiva brazo;
	//uniones hacia el hombro
	//GLfloat* u_hombro_x, u_hombro_y, u_hombro_z;
	//uniones hacia el antebrazo
	GLfloat u_codo_x = 0.0, u_codo_y = 0.0, u_codo_z = 0.0;
	glm::vec3 val_rot = glm::vec3(0.0, 0.0, 0.0);
	std::vector<GLfloat> lim_rot = {
		-180.0f, 180.0f,
		-180.0f, 180.0f,
		-180.0f, 180.0f
	};

	Brazo() {

	}

	Brazo(GLfloat x, GLfloat y, GLfloat z, float a, float b, float c,
		unsigned int& np, unsigned int& nt) {
		brazo = create(x, y, z, a, b, c, np, nt);
		updArticulacion();

		antebrazo = Antebrazo(u_codo_x, u_codo_y, u_codo_z, a * 1.001, c, b, np, nt);
	}

	void buffering() {
		antebrazo.buffering();

		brazo.buffering();
	}

	void updArticulacion() {
		GLuint codo = 11 * brazo.caja[lsup];
		u_codo_x = brazo.origen.x + brazo.ppuntos[codo];
		u_codo_y = brazo.origen.y + brazo.ppuntos[codo + 1];
		u_codo_z = brazo.origen.z + brazo.ppuntos[codo + 2];
	}

	void trasladar(glm::vec3 nuevoOrigen) {
		brazo.trasladarOrigen(nuevoOrigen);
		updArticulacion();
		antebrazo.trasladar(glm::vec3(u_codo_x, u_codo_y, u_codo_z));
	}

	void rotate(glm::vec3 RotationAxis, float RotationAngle) {
		brazo.rotate(RotationAxis, RotationAngle);
		antebrazo.trasladar(brazo.origen);
		antebrazo.rotate(RotationAxis, RotationAngle);
		updArticulacion();
		antebrazo.trasladar(glm::vec3(u_codo_x, u_codo_y, u_codo_z));
	}

	void flexion(GLfloat ang, Primitiva hombro) {
		//flexionamos en angulo de [-110,10]
		//negativo = jalon
		//positivo = empujon
		ang = rotLimit(val_rot.x, ang, lim_rot[0], lim_rot[1]);

		val_rot.x += ang;
		//en realidad va a ser hombro
		brazo.rot('X', ang, hombro);
		antebrazo.trasladar(brazo.origen);
		antebrazo.rotate(brazo.axX, ang);

		updArticulacion();

		antebrazo.trasladar(glm::vec3(u_codo_x, u_codo_y, u_codo_z));
	}

	void alzar(char ch, GLfloat ang, Primitiva hombro) {
		//rotamos en angulo de [0,110]
		//positivo = levantar
		ang = rotLimit(val_rot.z, ang, lim_rot[2], lim_rot[3]);

		val_rot.z += ang;
		if (ch == 'D')
			ang = -ang;
		//en realidad va a ser hombro
		brazo.rot('Z', ang, hombro);
		antebrazo.trasladar(brazo.origen);
		antebrazo.rotate(brazo.axZ, ang);
		updArticulacion();

		antebrazo.trasladar(glm::vec3(u_codo_x, u_codo_y, u_codo_z));
	}

	void rotacion(char ch, GLfloat ang, Primitiva hombro) {
		//rotamos en angulo de [-20,90]
		//negativo = pulgar abajo
		//positivo = pulgar arriba
		ang = rotLimit(val_rot.y, ang, lim_rot[4], lim_rot[5]);

		val_rot.y += ang;
		if (ch == 'D')
			ang = -ang;
		//en realidad va a ser hombro
		brazo.rot('Y', ang, hombro);
		antebrazo.trasladar(brazo.origen);
		antebrazo.rotate(brazo.axY, ang);

		updArticulacion();

		antebrazo.trasladar(glm::vec3(u_codo_x, u_codo_y, u_codo_z));
	}

	void setAnglesPred() {
		val_rot = glm::vec3(0.0, 0.0, 0.0);
		for (int i = 0; i < 6; i++)
			lim_rot[i] = ang_lims[8 + i];
	}
};

//el chamorro se domina a si mismo y al pie 
class Chamorro {
public:
	Primitiva pantorrilla, pie;
	GLfloat u_talon_x = 0.0, u_talon_y = 0.0, u_talon_z = 0.0;
	glm::vec3 val_rot = glm::vec3(0.0, 0.0, 0.0);
	std::vector<GLfloat> lim_rot = {
		0.0f, 140.0f
	};

	Chamorro() {

	}

	Chamorro(GLfloat x, GLfloat y, GLfloat z, float a, float b, float c,
		unsigned int& np, unsigned int& nt) {
		pantorrilla = create(x, y, z, a, b, c, np, nt);
		updArticulacion();

		pie = create(u_talon_x, u_talon_y, u_talon_z, a / 2.5, b / 1.5, c / 2, np, nt);
	}

	void updArticulacion() {
		GLuint talon = 11 * pantorrilla.caja[lsup];
		u_talon_x = pantorrilla.origen.x + pantorrilla.ppuntos[talon];
		u_talon_y = pantorrilla.origen.y + pantorrilla.ppuntos[talon + 1];
		u_talon_z = pantorrilla.origen.z + pantorrilla.ppuntos[talon + 2];
	}

	void buffering() {
		pie.buffering();
		pantorrilla.buffering();
	}

	void trasladar(glm::vec3 nuevoOrigen) {
		pantorrilla.trasladarOrigen(nuevoOrigen);
		updArticulacion();
		pie.trasladarOrigen(glm::vec3(u_talon_x, u_talon_y, u_talon_z));
	}

	void rotate(glm::vec3 RotationAxis, float RotationAngle) {
		pantorrilla.rotate(RotationAxis, RotationAngle);
		pie.trasladarOrigen(pantorrilla.origen);
		pie.rotate(RotationAxis, RotationAngle);
		updArticulacion();
		pie.trasladarOrigen(glm::vec3(u_talon_x, u_talon_y, u_talon_z));
	}

	void flexion(GLfloat ang, Primitiva brazo) {
		//flexionamos en angulo de [-90,0]
		//negativo = jalon
		ang = rotLimit(val_rot.x, ang, lim_rot[0], lim_rot[1]);

		val_rot.x += ang;
		pantorrilla.rot('X', ang, brazo);

		pie.trasladarOrigen(pantorrilla.origen);
		pie.rotate(pantorrilla.axX, ang);

		updArticulacion();
		pie.trasladarOrigen(glm::vec3(u_talon_x, u_talon_y, u_talon_z));
	}
	void setAnglesPred() {
		val_rot = glm::vec3(0.0, 0.0, 0.0);
		for (int i = 0; i < 2; i++)
			lim_rot[i] = ang_lims[18 + i];
	}

};

//la pierna se domina a si misma, al chamorro y al pie
class Pierna : public Chamorro {
public:
	//las Primitivas individuales
	Chamorro pantorrilla;
	Primitiva muslo;
	//uniones hacia el pantorrilla
	GLfloat u_rodilla_x = 0.0, u_rodilla_y = 0.0, u_rodilla_z = 0.0;
	glm::vec3 val_rot = glm::vec3(0.0, 0.0, 0.0);
	std::vector<GLfloat> lim_rot = {
		-180.0f, 180.0f,
		-180.0f, 180.0f,
		-180.0f, 180.0f
	};

	Pierna() {

	}

	Pierna(GLfloat x, GLfloat y, GLfloat z, float a, float b, float c,
		unsigned int& np, unsigned int& nt) {
		muslo = create(x, y, z, a, b, c, np, nt);
		updArticulacion();

		pantorrilla = Chamorro(u_rodilla_x, u_rodilla_y, u_rodilla_z, a / 1.001, c, b, np, nt);
	}

	void buffering() {
		pantorrilla.buffering();

		muslo.buffering();
	}

	void updArticulacion() {
		GLuint codo = 11 * muslo.caja[lsup];
		u_rodilla_x = muslo.origen.x + muslo.ppuntos[codo];
		u_rodilla_y = muslo.origen.y + muslo.ppuntos[codo + 1];
		u_rodilla_z = muslo.origen.z + muslo.ppuntos[codo + 2];
	}

	void trasladar(glm::vec3 nuevoOrigen) {
		muslo.trasladarOrigen(nuevoOrigen);
		updArticulacion();
		pantorrilla.trasladar(glm::vec3(u_rodilla_x, u_rodilla_y, u_rodilla_z));
	}

	void rotate(glm::vec3 RotationAxis, float RotationAngle) {
		muslo.rotate(RotationAxis, RotationAngle);
		pantorrilla.trasladar(muslo.origen);
		pantorrilla.rotate(RotationAxis, RotationAngle);
		updArticulacion();
		pantorrilla.trasladar(glm::vec3(u_rodilla_x, u_rodilla_y, u_rodilla_z));
	}

	void flexion(GLfloat ang, Primitiva cadera) {
		//flexionamos en angulo de [-110,10]
		//negativo = jalon
		//positivo = empujon
		ang = rotLimit(val_rot.x, ang, lim_rot[0], lim_rot[1]);

		val_rot.x += ang;
		ang = -ang;
		//en realidad va a ser cadera
		muslo.rot('X', ang, cadera);
		pantorrilla.trasladar(muslo.origen);
		pantorrilla.rotate(muslo.axX, ang);

		updArticulacion();

		pantorrilla.trasladar(glm::vec3(u_rodilla_x, u_rodilla_y, u_rodilla_z));
	}

	void alzar(char ch, GLfloat ang, Primitiva cadera) {
		//rotamos en angulo de [0,110]
		//positivo = levantar
		ang = rotLimit(val_rot.z, ang, lim_rot[2], lim_rot[3]);

		val_rot.z += ang;
		if (ch == 'D')
			ang = -ang;
		//en realidad va a ser cadera
		muslo.rot('Z', ang, cadera);
		pantorrilla.trasladar(muslo.origen);
		pantorrilla.rotate(muslo.axZ, ang);

		updArticulacion();

		pantorrilla.trasladar(glm::vec3(u_rodilla_x, u_rodilla_y, u_rodilla_z));
	}

	void rotacion(char ch, GLfloat ang, Primitiva cadera) {
		//rotamos en angulo de [-20,90]
		//negativo = pulgar abajo
		//positivo = pulgar arriba
		ang = rotLimit(val_rot.y, ang, lim_rot[4], lim_rot[5]);

		val_rot.y += ang;
		if (ch == 'I')
			ang = -ang;
		//en realidad va a ser cadera
		muslo.rot('Y', ang, cadera);
		pantorrilla.trasladar(muslo.origen);
		pantorrilla.rotate(muslo.axY, ang);

		updArticulacion();

		pantorrilla.trasladar(glm::vec3(u_rodilla_x, u_rodilla_y, u_rodilla_z));
	}
	void setAnglesPred() {
		pantorrilla.setAnglesPred();
		val_rot = glm::vec3(0.0, 0.0, 0.0);
		for (int i = 0; i < 6; i++)
			lim_rot[i] = ang_lims[20 + i];
	}
};

//la marioneta domina a todas las extremidades
class Marioneta : public Brazo, Pierna, Cabeza {
public:
	//las Primitivas individuales
	Primitiva hombro, abdomen, cadera, cuello;
	Pierna ipierna, dpierna;
	Brazo ibrazo, dbrazo;
	Cabeza cabeza;
	//uniones hacia el pecho
	GLfloat u_pecho_x = 0.0f, u_pecho_y = 0.0f, u_pecho_z = 0.0f;
	//uniones hacia el pecho
	GLfloat u_clavicula_x = 0.0f, u_clavicula_y = 0.0f, u_clavicula_z = 0.0f;
	//uniones hacia los hombros
	GLfloat u_dhombro_x = 0.0, u_dhombro_y = 0.0, u_dhombro_z = 0.0;
	GLfloat u_ihombro_x = 0.0, u_ihombro_y = 0.0, u_ihombro_z = 0.0;
	//uniones hacia las piernas
	GLfloat u_dcadera_x = 0.0, u_dcadera_y = 0.0, u_dcadera_z = 0.0;
	GLfloat u_icadera_x = 0.0, u_icadera_y = 0.0, u_icadera_z = 0.0;
	//uniones hacia el coxis
	GLfloat u_pelvis_x = 0.0, u_pelvis_y = 0.0, u_pelvis_z = 0.0;
	//uniones hacia el cuello
	GLfloat u_bcuello_x = 0.0, u_bcuello_y = 0.0, u_bcuello_z = 0.0;
	//factor del cuerpo determinado por alto de los hombros
	GLfloat factor = 0.0;
	//

	Marioneta() {

	}

	Marioneta(float ancho, unsigned int& np, unsigned int& nt) {
		factor = ancho / 0.5;

		float alto = 0.15 * factor, grosor = 0.2 * factor;
		cadera = create(0, 0, 0, alto, ancho / 1.75, grosor, np, nt);
		cadera.rot('X', 180, cadera);

		updArticulacion(1);

		abdomen = create(u_pelvis_x, u_pelvis_y, u_pelvis_z, ancho, 2 * grosor, grosor, np, nt);

		dpierna = Pierna(u_dcadera_x, u_dcadera_y, u_dcadera_z, 1.2 * ancho, 0.9 * grosor, 0.9 * grosor, np, nt);
		dpierna.alzar('I', 180, cadera);
		dpierna.pantorrilla.pie.rot('X', -90, dpierna.pantorrilla.pantorrilla);

		ipierna = Pierna(u_icadera_x, u_icadera_y, u_icadera_z, 1.2 * ancho, 0.9 * grosor, 0.9 * grosor, np, nt);
		ipierna.alzar('I', -180, cadera);
		ipierna.pantorrilla.pie.rot('X', -90, ipierna.pantorrilla.pantorrilla);

		updArticulacion(2);

		hombro = create(u_pecho_x, u_pecho_y, u_pecho_z, alto, ancho, grosor, np, nt);

		updArticulacion(3);

		dbrazo = Brazo(u_dhombro_x, u_dhombro_y, u_dhombro_z, ancho / 1.5, 0.6 * grosor, 0.6 * grosor, np, nt);
		dbrazo.alzar('I', 180, hombro);

		ibrazo = Brazo(u_ihombro_x, u_ihombro_y, u_ihombro_z, ancho / 1.5, 0.6 * grosor, 0.6 * grosor, np, nt);
		ibrazo.alzar('I', -180, hombro);

		cuello = create(u_clavicula_x, u_clavicula_y, u_clavicula_z, 1.3 * alto, grosor / 2.1, grosor / 2.1, np, nt);

		updArticulacion(4);

		cabeza = Cabeza(u_bcuello_x, u_bcuello_y, u_bcuello_z, ancho / 1.5, ancho / 1.8, ancho / 1.8, np, nt);

		setAngles();
	}

	void updArticulacion(int i) {
		if (i > 0) {
			GLuint pelvis = 11 * cadera.caja[lsup], ombligo = 11 * cadera.caja[linf];
			u_pelvis_x = cadera.origen.x + (cadera.ppuntos[pelvis] + cadera.ppuntos[ombligo]) / 2;
			u_pelvis_y = cadera.origen.y + (cadera.ppuntos[pelvis + 1] + cadera.ppuntos[ombligo + 1]) / 2;
			u_pelvis_z = cadera.origen.z + (cadera.ppuntos[pelvis + 2] + cadera.ppuntos[ombligo + 2]) / 2;

			GLuint dcadera = 11 * cadera.caja[lder];
			u_dcadera_x = cadera.origen.x + cadera.ppuntos[dcadera];
			u_dcadera_y = cadera.origen.y + cadera.ppuntos[dcadera + 1];
			u_dcadera_z = cadera.origen.z + cadera.ppuntos[dcadera + 2];

			GLuint icadera = 11 * cadera.caja[lizq];
			u_icadera_x = cadera.origen.x + cadera.ppuntos[icadera];
			u_icadera_y = cadera.origen.y + cadera.ppuntos[icadera + 1];
			u_icadera_z = cadera.origen.z + cadera.ppuntos[icadera + 2];
		}
		if (i > 1) {
			GLuint pelvis = 11 * abdomen.caja[linf], pecho = 11 * abdomen.caja[lsup];
			GLfloat a_pelvis_x = abdomen.ppuntos[pelvis],
				a_pelvis_y = abdomen.ppuntos[pelvis + 1],
				a_pelvis_z = abdomen.ppuntos[pelvis + 2];

			GLfloat a_pecho_x = abdomen.ppuntos[pecho],
				a_pecho_y = abdomen.ppuntos[pecho + 1],
				a_pecho_z = abdomen.ppuntos[pecho + 2];

			glm::vec3 V = glm::vec3(a_pelvis_x - a_pecho_x,
				a_pelvis_y - a_pecho_y,
				a_pelvis_z - a_pecho_z);
			GLfloat R = (0.15 * factor) / (glm::sqrt(V.x * V.x + V.y * V.y + V.z * V.z));
			V = glm::vec3(R * V.x, R * V.y, R * V.z);

			u_pecho_x = abdomen.origen.x + a_pecho_x + V.x;
			u_pecho_y = abdomen.origen.y + a_pecho_y + V.y;
			u_pecho_z = abdomen.origen.z + a_pecho_z + V.z;
		}
		if (i > 2) {
			GLuint dhombro = 11 * hombro.caja[lder];
			u_dhombro_x = hombro.origen.x + hombro.ppuntos[dhombro];
			u_dhombro_y = hombro.origen.y + hombro.ppuntos[dhombro + 1];
			u_dhombro_z = hombro.origen.z + hombro.ppuntos[dhombro + 2];

			GLuint ihombro = 11 * hombro.caja[lizq];
			u_ihombro_x = hombro.origen.x + hombro.ppuntos[ihombro];
			u_ihombro_y = hombro.origen.y + hombro.ppuntos[ihombro + 1];
			u_ihombro_z = hombro.origen.z + hombro.ppuntos[ihombro + 2];

			GLuint pecho = 11 * hombro.caja[lsup], esternon = 11 * hombro.caja[linf];
			u_clavicula_x = hombro.origen.x + (hombro.ppuntos[pecho] + hombro.ppuntos[esternon]) / 2;
			u_clavicula_y = hombro.origen.y + (hombro.ppuntos[pecho + 1] + hombro.ppuntos[esternon + 1]) / 2;
			u_clavicula_z = hombro.origen.z + (hombro.ppuntos[pecho + 2] + hombro.ppuntos[esternon + 2]) / 2;
		}
		if (i > 3) {
			GLuint cuelloUp = 11 * cuello.caja[lsup], cuelloDo = 11 * cuello.caja[linf];
			GLfloat a_cuelloup_x = cuello.ppuntos[cuelloUp],
				a_cuelloup_y = cuello.ppuntos[cuelloUp + 1],
				a_cuelloup_z = cuello.ppuntos[cuelloUp + 2];

			GLfloat a_cuellodo_x = cuello.ppuntos[cuelloDo],
				a_cuellodo_y = cuello.ppuntos[cuelloDo + 1],
				a_cuellodo_z = cuello.ppuntos[cuelloDo + 2];

			glm::vec3 V = glm::vec3(a_cuelloup_x - a_cuellodo_x,
				a_cuelloup_y - a_cuellodo_y,
				a_cuelloup_z - a_cuellodo_z);

			GLfloat R = -(2 * factor) / (9 * glm::sqrt(V.x * V.x + V.y * V.y + V.z * V.z));
			V = glm::vec3(R * V.x, R * V.y, R * V.z);

			u_bcuello_x = cuello.origen.x + a_cuelloup_x + V.x;
			u_bcuello_y = cuello.origen.y + a_cuelloup_y + V.y;
			u_bcuello_z = cuello.origen.z + a_cuelloup_z + V.z;
		}
	}

	void buffering() {
		dbrazo.buffering();
		ibrazo.buffering();
		dpierna.buffering();
		ipierna.buffering();
		abdomen.buffering();
		cadera.buffering();
		cuello.buffering();
		cabeza.buffering();

		hombro.buffering();
	}

	void trasladar(glm::vec3 nuevoOrigen) {
		cadera.trasladarOrigen(nuevoOrigen);
		updArticulacion(1);
		abdomen.trasladarOrigen(glm::vec3(u_pelvis_x, u_pelvis_y, u_pelvis_z));
		dpierna.trasladar(glm::vec3(u_dcadera_x, u_dcadera_y, u_dcadera_z));
		ipierna.trasladar(glm::vec3(u_icadera_x, u_icadera_y, u_icadera_z));
		updArticulacion(2);
		hombro.trasladarOrigen(glm::vec3(u_pecho_x, u_pecho_y, u_pecho_z));
		updArticulacion(3);
		cuello.trasladarOrigen(glm::vec3(u_clavicula_x, u_clavicula_y, u_clavicula_z));
		dbrazo.trasladar(glm::vec3(u_dhombro_x, u_dhombro_y, u_dhombro_z));
		ibrazo.trasladar(glm::vec3(u_ihombro_x, u_ihombro_y, u_ihombro_z));
		updArticulacion(4);
		cabeza.trasladar(glm::vec3(u_bcuello_x, u_bcuello_y, u_bcuello_z));

	}

	void setAngles() {
		ipierna.setAnglesPred();
		dpierna.setAnglesPred();
		ibrazo.setAnglesPred();
		dbrazo.setAnglesPred();
		//cabeza.setAnglesPred();
	}

	void rot(char ch, GLfloat ang) {
		glm::vec3 RotationAxis = glm::vec3(0.0f, 0.0f, 0.0f);
		switch (ch) {
		case 'X':
			RotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
			break;
		case 'Y':
			RotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
			break;
		case 'Z':
			RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
			break;
		}
		cadera.rotate(RotationAxis, ang);

		updArticulacion(1);
		//abdomen
		abdomen.trasladarOrigen(cadera.origen);
		abdomen.rotate(RotationAxis, ang);
		abdomen.trasladarOrigen(glm::vec3(u_pelvis_x, u_pelvis_y, u_pelvis_z));
		//pierna derecha
		dpierna.trasladar(cadera.origen);
		dpierna.rotate(RotationAxis, ang);
		dpierna.trasladar(glm::vec3(u_dcadera_x, u_dcadera_y, u_dcadera_z));
		//pierna izquierda
		ipierna.trasladar(cadera.origen);
		ipierna.rotate(RotationAxis, ang);
		ipierna.trasladar(glm::vec3(u_icadera_x, u_icadera_y, u_icadera_z));

		updArticulacion(2);
		//hombro
		hombro.trasladarOrigen(cadera.origen);
		hombro.rotate(RotationAxis, ang);
		hombro.trasladarOrigen(glm::vec3(u_pecho_x, u_pecho_y, u_pecho_z));

		updArticulacion(3);
		//cuello
		cuello.trasladarOrigen(cadera.origen);
		cuello.rotate(RotationAxis, ang);
		cuello.trasladarOrigen(glm::vec3(u_clavicula_x, u_clavicula_y, u_clavicula_z));
		//dbrazo
		dbrazo.trasladar(cadera.origen);
		dbrazo.rotate(RotationAxis, ang);
		dbrazo.trasladar(glm::vec3(u_dhombro_x, u_dhombro_y, u_dhombro_z));
		//brazo izquierdo
		ibrazo.trasladar(cadera.origen);
		ibrazo.rotate(RotationAxis, ang);
		ibrazo.trasladar(glm::vec3(u_ihombro_x, u_ihombro_y, u_ihombro_z));

		updArticulacion(4);
		//cabeza
		cabeza.trasladar(cadera.origen);
		cabeza.rotate(RotationAxis, ang);
		cabeza.trasladar(glm::vec3(u_bcuello_x, u_bcuello_y, u_bcuello_z));
	}

};

//vector del farol para iluminar
GLfloat lightVertices[] =
{ //     COORDINATES     //
	-0.1f, -0.1f,  0.1f,
	-0.1f, -0.1f, -0.1f,
	 0.1f, -0.1f, -0.1f,
	 0.1f, -0.1f,  0.1f,
	-0.1f,  0.1f,  0.1f,
	-0.1f,  0.1f, -0.1f,
	 0.1f,  0.1f, -0.1f,
	 0.1f,  0.1f,  0.1f
};

//indices del mismo
GLuint lightIndices[] =
{
	0, 1, 2,
	0, 2, 3,
	0, 4, 7,
	0, 7, 3,
	3, 7, 6,
	3, 6, 2,
	2, 6, 5,
	2, 5, 1,
	1, 5, 4,
	1, 4, 0,
	4, 5, 6,
	4, 6, 7
};

//con esta variable indico a mi programa si hay que hacer una actualizacion ya que si actulizo en cada
//while loop estare cargando inecesariamente a mi programa
bool actualizacionPendiente = false;

//con esta funcion cambio es estado de la variable anterior, simplemente para reducir el codigo
void Act() {
	if (actualizacionPendiente)
		actualizacionPendiente = false;
	else
		actualizacionPendiente = true;
}

int main()
{
	//creo mi marioneta con un factor de 0.5, al indicar este factor se genera una unica marioneta pues
	//todas las distancias se determinan a traves de ella
	Marioneta objeto(0.5, cabezaver, cabezaind);
	objeto.buffering();
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object of 800 by 800 pixels, naming it "Puppet"
	GLFWwindow* window = glfwCreateWindow(width, height, "Puppet Project", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, width, height);

	// Generates Shader object using shaders default.vert and default.frag
	Shader shaderProgram("default.vert", "default.frag");
	// Generates Vertex Array Object and binds it
	VAO VAO1;
	VAO1.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO1(vertices, sizeof(vertices));
	// Generates Element Buffer Object and links it to indices
	EBO EBO1(indices, sizeof(indices));
	// Links VBO attributes such as coordinates and colors to VAO
	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	VAO1.LinkAttrib(VBO1, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	//cargando los vectores normales

	// Unbind all to prevent accidentally modifying them
	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

	// Shader for light cube
	Shader lightShader("light.vert", "light.frag");
	// Generates Vertex Array Object and binds it
	VAO lightVAO;
	lightVAO.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO lightVBO(lightVertices, sizeof(lightVertices));
	// Generates Element Buffer Object and links it to indices
	EBO lightEBO(lightIndices, sizeof(lightIndices));
	// Links VBO attributes such as coordinates and colors to VAO
	lightVAO.LinkAttrib(lightVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	// Unbind all to prevent accidentally modifying them
	lightVAO.Unbind();
	lightVBO.Unbind();
	lightEBO.Unbind();

	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.0f, 3.0f, 4.0f);
	glm::mat4 lightModel = glm::mat4(1.0f);
	lightModel = glm::translate(lightModel, lightPos);

	glm::vec3 pyramidPos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 pyramidModel = glm::mat4(1.0f);
	pyramidModel = glm::translate(pyramidModel, pyramidPos);

	lightShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
	glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	shaderProgram.Activate();
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(pyramidModel));
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);


	std::string parentDir = (fs::current_path().fs::path::parent_path()).string();
	std::string texPath = "/Resources/";

	// Texture
	Texture brickTex((parentDir + texPath + "brick.png").c_str(), GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
	brickTex.texUnit(shaderProgram, "tex0", 0);
	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);

	// Creates camera object
	Camera camera(width, height, glm::vec3(0.0f, 0.0f, 9.0f));

	// Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Variables to be changed in the ImGUI window
	float angulo[29] = {};

	//limites de angulos para las primitivas de mano y pies pues nunca generamos la clase con
	//dichas variables
	float manos_anglim[4] = {
		-90.0, 90.0, //limite de eje X
		0.0, 30.0   //limite de eje Z
	};
	float manos_angact[4] = {
		0.0, 0.0, //grados de derecha X, Z
		0.0, 0.0  //grados de izquierda X, Z
	};
	float pies_anglim[4] = {
		-20.0, 90.0, //limite de eje X
		-20.0, 20.0   //limite de eje Z
	};
	float pies_angact[4] = {
		0.0, 0.0, //grados de derecha X, Z
		0.0, 0.0  //grados de izquierda X, Z
	};

	// Main while loop
	while (!glfwWindowShouldClose(window))
	{// Specify the color of the background
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		// Clean the back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Tell OpenGL a new frame is about to begin
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Interfaz de usuario
		ImGui::Begin("Movimientos");
		// Text that appears in the window
		ImGui::Text("Modifica:");
		// Checkbox that appears in the window
		ImGui::Text(" Cabeza:");
		ImGui::SliderFloat("Cabeza X", &angulo[cabezaX], objeto.cabeza.lim_rot[0], objeto.cabeza.lim_rot[1]);
		ImGui::SliderFloat("Cabeza Y", &angulo[cabezaY], objeto.cabeza.lim_rot[2], objeto.cabeza.lim_rot[3]);
		ImGui::SliderFloat("Cabeza Z", &angulo[cabezaZ], objeto.cabeza.lim_rot[4], objeto.cabeza.lim_rot[5]);
		ImGui::Text("\n Brazos:");
		ImGui::Text("  Derecho:");
		ImGui::SliderFloat("B. D. X", &angulo[brazoderX], objeto.dbrazo.lim_rot[0], objeto.dbrazo.lim_rot[1]);
		ImGui::SliderFloat("B. D. Y", &angulo[brazoderY], objeto.dbrazo.lim_rot[4], objeto.dbrazo.lim_rot[5]);
		ImGui::SliderFloat("B. D. Z", &angulo[brazoderZ], objeto.dbrazo.lim_rot[2], objeto.dbrazo.lim_rot[3]);
		ImGui::Text("  Izquierdo:");
		ImGui::SliderFloat("B. I. X", &angulo[brazoizqX], objeto.ibrazo.lim_rot[0], objeto.ibrazo.lim_rot[1]);
		ImGui::SliderFloat("B. I. Y", &angulo[brazoizqY], objeto.ibrazo.lim_rot[4], objeto.ibrazo.lim_rot[5]);
		ImGui::SliderFloat("B. I. Z", &angulo[brazoizqZ], objeto.ibrazo.lim_rot[2], objeto.ibrazo.lim_rot[3]);

		ImGui::Text("\n Antebrazos:");
		ImGui::Text("  Derecho:");
		ImGui::SliderFloat("A. D. X", &angulo[antederX], objeto.dbrazo.antebrazo.lim_rot[0], objeto.dbrazo.antebrazo.lim_rot[1]);
		ImGui::SliderFloat("A. D. Y", &angulo[antederY], objeto.dbrazo.antebrazo.lim_rot[2], objeto.dbrazo.antebrazo.lim_rot[3]);
		ImGui::Text("  Izquierdo:");
		ImGui::SliderFloat("A. I. X", &angulo[anteizqX], objeto.ibrazo.antebrazo.lim_rot[0], objeto.ibrazo.antebrazo.lim_rot[1]);
		ImGui::SliderFloat("A. I. Y", &angulo[anteizqY], objeto.ibrazo.antebrazo.lim_rot[2], objeto.ibrazo.antebrazo.lim_rot[3]);

		ImGui::Text("\n Manos:");
		ImGui::Text("  Derecha:");
		ImGui::SliderFloat("M. D. X", &angulo[manoderX], manos_anglim[0], manos_anglim[1]);
		ImGui::SliderFloat("M. D. Z", &angulo[manoderZ], manos_anglim[2], manos_anglim[3]);
		ImGui::Text("  Izquierda:");
		ImGui::SliderFloat("M. I. X", &angulo[manoizqX], manos_anglim[0], manos_anglim[1]);
		ImGui::SliderFloat("M. I. Z", &angulo[manoizqZ], manos_anglim[2], manos_anglim[3]);

		ImGui::Text("\n Piernas:");
		ImGui::Text("  Derecha:");
		ImGui::SliderFloat("P. D. X", &angulo[piernaderX], objeto.dpierna.lim_rot[0], objeto.dpierna.lim_rot[1]);
		ImGui::SliderFloat("P. D. Y", &angulo[piernaderY], objeto.dpierna.lim_rot[4], objeto.dpierna.lim_rot[5]);
		ImGui::SliderFloat("P. D. Z", &angulo[piernaderZ], objeto.dpierna.lim_rot[2], objeto.dpierna.lim_rot[3]);
		ImGui::Text("  Izquierda:");
		ImGui::SliderFloat("P. I. X", &angulo[piernaizqX], objeto.ipierna.lim_rot[0], objeto.ipierna.lim_rot[1]);
		ImGui::SliderFloat("P. I. Y", &angulo[piernaizqY], objeto.ipierna.lim_rot[4], objeto.ipierna.lim_rot[5]);
		ImGui::SliderFloat("P. I. Z", &angulo[piernaizqZ], objeto.ipierna.lim_rot[2], objeto.ipierna.lim_rot[3]);

		ImGui::Text("\n Pantorrillas:");
		ImGui::Text("  Derecha:");
		ImGui::SliderFloat("Pa. D.", &angulo[pantoderX], objeto.dpierna.pantorrilla.lim_rot[0], objeto.dpierna.pantorrilla.lim_rot[1]);
		ImGui::Text("  Izquierda:");
		ImGui::SliderFloat("Pa. I.", &angulo[pantoizqX], objeto.ipierna.pantorrilla.lim_rot[0], objeto.dpierna.pantorrilla.lim_rot[1]);

		ImGui::Text("\n Pies:");
		ImGui::Text("  Derecho:");
		ImGui::SliderFloat("Pi. D. X", &angulo[piederX], pies_anglim[0], pies_anglim[1]);
		ImGui::SliderFloat("Pi. D. Z", &angulo[piederZ], pies_anglim[2], pies_anglim[3]);
		ImGui::Text("  Izquierdo:");
		ImGui::SliderFloat("Pi. I. X", &angulo[pieizqX], pies_anglim[0], pies_anglim[1]);
		ImGui::SliderFloat("Pi. I. Z", &angulo[pieizqZ], pies_anglim[2], pies_anglim[3]);

		// Slider that appears in the window

		// Ends the window
		ImGui::End();

		//verificamos si en alguna variable exisitio algun cambio en el angulo
		if (angulo[cabezaX] != objeto.cabeza.val_rot.x) {
			Act();
			objeto.cabeza.flexion(angulo[cabezaX], objeto.cuello);
		}
		if (angulo[cabezaY] != objeto.cabeza.val_rot.y) {
			Act();
			objeto.cabeza.rotacion(angulo[cabezaY], objeto.cuello);
		}
		if (angulo[cabezaZ] != objeto.cabeza.val_rot.z) {
			Act();
			objeto.cabeza.negacion(angulo[cabezaZ], objeto.cuello);
		}

		if (angulo[brazoderX] != objeto.dbrazo.val_rot.x) {
			Act();
			objeto.dbrazo.flexion(angulo[brazoderX], objeto.hombro);
		}
		if (angulo[brazoderY] != objeto.dbrazo.val_rot.y) {
			Act();
			objeto.dbrazo.rotacion('D', angulo[brazoderY], objeto.hombro);
		}
		if (angulo[brazoderZ] != objeto.dbrazo.val_rot.z) {
			Act();
			objeto.dbrazo.alzar('D', angulo[brazoderZ], objeto.hombro);
		}

		if (angulo[brazoizqX] != objeto.ibrazo.val_rot.x) {
			Act();
			objeto.ibrazo.flexion(angulo[brazoizqX], objeto.hombro);
		}
		if (angulo[brazoizqY] != objeto.ibrazo.val_rot.y) {
			Act();
			objeto.ibrazo.rotacion('I', angulo[brazoizqY], objeto.hombro);
		}
		if (angulo[brazoizqZ] != objeto.ibrazo.val_rot.z) {
			Act();
			objeto.ibrazo.alzar('I', angulo[brazoizqZ], objeto.hombro);
		}

		if (angulo[antederX] != objeto.dbrazo.antebrazo.val_rot.x) {
			Act();
			objeto.dbrazo.antebrazo.flexion(angulo[antederX], objeto.dbrazo.brazo);
		}
		if (angulo[antederY] != objeto.dbrazo.antebrazo.val_rot.y) {
			Act();
			objeto.dbrazo.antebrazo.rotacion('D', angulo[antederY], objeto.dbrazo.antebrazo.antebrazo);
		}

		if (angulo[anteizqX] != objeto.ibrazo.antebrazo.val_rot.x) {
			Act();
			objeto.ibrazo.antebrazo.flexion(angulo[anteizqX], objeto.ibrazo.brazo);
		}
		if (angulo[anteizqY] != objeto.ibrazo.antebrazo.val_rot.y) {
			Act();
			objeto.ibrazo.antebrazo.rotacion('I', angulo[anteizqY], objeto.ibrazo.antebrazo.antebrazo);
		}

		if (angulo[manoderX] != manos_angact[0]) {
			float ang = rotLimit(manos_angact[0], angulo[manoderX], manos_anglim[0], manos_anglim[1]);
			manos_angact[0] += ang;
			Act();
			objeto.dbrazo.antebrazo.mano.rot('X', ang, objeto.dbrazo.antebrazo.antebrazo);
		}
		if (angulo[manoderZ] != manos_angact[1]) {
			float ang = rotLimit(manos_angact[1], angulo[manoderZ], manos_anglim[2], manos_anglim[3]);
			manos_angact[1] += ang;
			Act();
			objeto.dbrazo.antebrazo.mano.rot('Z', -ang, objeto.dbrazo.antebrazo.antebrazo);
		}

		if (angulo[manoizqX] != manos_angact[2]) {
			float ang = rotLimit(manos_angact[2], angulo[manoizqX], manos_anglim[0], manos_anglim[1]);
			manos_angact[2] += ang;
			Act();
			objeto.ibrazo.antebrazo.mano.rot('X', ang, objeto.ibrazo.antebrazo.antebrazo);
		}
		if (angulo[manoizqZ] != manos_angact[3]) {
			float ang = rotLimit(manos_angact[3], angulo[manoizqZ], manos_anglim[2], manos_anglim[3]);
			manos_angact[3] += ang;
			Act();
			objeto.ibrazo.antebrazo.mano.rot('Z', ang, objeto.ibrazo.antebrazo.antebrazo);
		}

		if (angulo[piernaderX] != objeto.dpierna.val_rot.x) {
			Act();
			objeto.dpierna.flexion(angulo[piernaderX], objeto.dpierna.muslo);
		}
		if (angulo[piernaderY] != objeto.dpierna.val_rot.y) {
			Act();
			objeto.dpierna.rotacion('D', angulo[piernaderY], objeto.dpierna.muslo);
		}
		if (angulo[piernaderZ] != objeto.dpierna.val_rot.z) {
			Act();
			objeto.dpierna.alzar('D', angulo[piernaderZ], objeto.dpierna.muslo);
		}

		if (angulo[piernaizqX] != objeto.ipierna.val_rot.x) {
			Act();
			objeto.ipierna.flexion(angulo[piernaizqX], objeto.ipierna.muslo);
		}
		if (angulo[piernaizqY] != objeto.ipierna.val_rot.y) {
			Act();
			objeto.ipierna.rotacion('I', angulo[piernaizqY], objeto.ipierna.muslo);
		}
		if (angulo[piernaizqZ] != objeto.ipierna.val_rot.z) {
			Act();
			objeto.ipierna.alzar('I', angulo[piernaizqZ], objeto.ipierna.muslo);
		}

		if (angulo[pantoderX] != objeto.dpierna.pantorrilla.val_rot.x) {
			Act();
			objeto.dpierna.pantorrilla.flexion(angulo[pantoderX], objeto.dpierna.muslo);
		}
		if (angulo[pantoizqX] != objeto.ipierna.pantorrilla.val_rot.x) {
			Act();
			objeto.ipierna.pantorrilla.flexion(angulo[pantoizqX], objeto.ipierna.muslo);
		}

		if (angulo[piederX] != pies_angact[0]) {
			float ang = rotLimit(pies_angact[0], angulo[piederX], pies_anglim[0], pies_anglim[1]);
			pies_angact[0] += ang;
			Act();
			objeto.dpierna.pantorrilla.pie.rot('X', ang, objeto.dpierna.pantorrilla.pantorrilla);
		}
		if (angulo[piederZ] != pies_angact[1]) {
			float ang = rotLimit(pies_angact[1], angulo[piederZ], pies_anglim[2], pies_anglim[3]);
			pies_angact[1] += ang;
			Act();
			objeto.dpierna.pantorrilla.pie.rot('Z', -ang, objeto.dpierna.pantorrilla.pie);
		}

		if (angulo[pieizqX] != pies_angact[2]) {
			float ang = rotLimit(pies_angact[2], angulo[pieizqX], pies_anglim[0], pies_anglim[1]);
			pies_angact[2] += ang;
			Act();
			objeto.ipierna.pantorrilla.pie.rot('X', ang, objeto.ipierna.pantorrilla.pantorrilla);
		}
		if (angulo[pieizqZ] != pies_angact[3]) {
			float ang = rotLimit(pies_angact[3], angulo[pieizqZ], pies_anglim[2], pies_anglim[3]);
			pies_angact[3] += ang;
			Act();
			objeto.ipierna.pantorrilla.pie.rot('Z', ang, objeto.ipierna.pantorrilla.pie);
		}

		//funciones a traves del teclado que indican el movimiento de la marioneta
		//rota a arriba
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			Act();
			objeto.rot('X', 1);
		}
		//rota hacia abajo
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			Act();
			objeto.rot('X', -1);
		}
		//rota a la iquierda
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			Act();
			objeto.rot('Y', 1);
		}
		//rota hacia la derecha
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			Act();
			objeto.rot('Y', -1);
		}
		//rota a la iquierda
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{

			Act();
			objeto.rot('Z', 1);
		}
		//rota hacia la derecha
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			Act();
			objeto.rot('Z', -1);
		}
		//reinicia la posicion
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			Act();
			for (int i = 0; i < 29; i++)
				angulo[i] = 0.0;
			cabezaver = 0;
			cabezaind = 0;
			objeto = Marioneta(0.5, cabezaver, cabezaind);
			objeto.buffering();
		}
		//oculta la marioneta
		if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
			Act();
			// Generates Vertex Buffer Object and links it to vertices
			cleanBuffer();
		}
		//para trasladar utilizamos una entrada por teclado a la consola
		if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
			Act();
			glm::vec3 post = glm::vec3(0.0, 0.0, 0.0);

			std::cin >> post.x >> post.y >> post.z;

			objeto.trasladar(post);
		}

		//con esta variable realizamos la carga de los nuevos datos al VAO y VBO          
		if (actualizacionPendiente) {
			Act();
			VAO1.Bind();

			objeto.buffering();
			VBO1 = VBO(vertices, sizeof(vertices));

			VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0);
			VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float)));
			VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float)));
			VAO1.LinkAttrib(VBO1, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float)));
		}

		// Updates and exports the camera matrix to the Vertex Shader
		camera.updateMatrix(45.0f, 0.1f, 100.0f);

		// Tells OpenGL which Shader Program we want to use
		shaderProgram.Activate();
		// Exports the camera Position to the Fragment Shader for specular lighting
		glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		// Export the camMatrix to the Vertex Shader of the pyramid
		camera.Matrix(shaderProgram, "camMatrix");
		// Binds texture so that is appears in rendering
		brickTex.Bind();



		// Bind the VAO so OpenGL knows to use it
		VAO1.Bind();

		// Draw primitives, number of indices, datatype of indices, index of indices
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(int), GL_UNSIGNED_INT, 0);

		// Tells OpenGL which Shader Program we want to use
		lightShader.Activate();
		// Export the camMatrix to the Vertex Shader of the light cube
		camera.Matrix(lightShader, "camMatrix");
		// Bind the VAO so OpenGL knows to use it
		lightVAO.Bind();
		// Draw primitives, number of indices, datatype of indices, index of indices
		glDrawElements(GL_TRIANGLES, sizeof(lightIndices) / sizeof(int), GL_UNSIGNED_INT, 0);

		// Renders the ImGUI elements
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();

	}

	// Deletes all ImGUI instances
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Delete all the objects we've created
	VAO1.Delete();
	VBO1.Delete();
	EBO1.Delete();
	brickTex.Delete();
	shaderProgram.Delete();
	lightVAO.Delete();
	lightVBO.Delete();
	lightEBO.Delete();
	lightShader.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}

