#ifndef FIGURAS_H
#define FIGURAS_H

#include<iostream>
#include<vector>
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

#define disc 28

#define linf 0
#define lsup 1
#define lder 2
#define lizq 3
#define lfro 4
#define ltra 5

const unsigned int width = 800;
const unsigned int height = 800;

//definiciones de David
std::vector <GLfloat> bas = { 0.0f, 0.70f, 0.44f, 1.0f, 1.0f };
const unsigned int puntos_prim = 4 * disc * disc - 2 * disc;
const unsigned int triangulos_prim = 2 * disc * disc - 2 * disc;

std::vector<GLfloat> normal, puntos, vertice;
std::vector<GLuint> indice;
GLuint inicio = 0;
GLfloat vertices[10 * 11 * (4 * disc * disc - 2 * disc)];
GLuint indices[10 * 3 * (2 * disc * disc - 2 * disc)];


std::vector<GLfloat> calcularNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);

void cleanBuffer();

void meterTriangulo(unsigned int a, unsigned int b, unsigned int c);

void meterTrapezoide(unsigned int a, unsigned int b, unsigned int c, unsigned int d);

class Primitiva {
public:
	std::vector<GLfloat> ppuntos;
	std::vector<GLuint> pindices;
	unsigned int cantidad_puntos = 0, cantidad_triangulos = 0;
	glm::vec3 origen = glm::vec3(0.0f, 0.0f, 0.0f);
	unsigned int pidx = 0, tidx = 0;
	std::vector<GLuint> caja = {
		0, 4 * disc * disc - 2 * disc - 1, //limites inf y superior
		2 * disc * disc - disc, 2 * disc * disc + disc, //der e izq
		2 * disc * disc - 3, 2 * disc * disc + 2 * disc - 3
	};
	glm::vec3 axX = glm::vec3(1.0f, 0.0f, 0.0f),
		axY = glm::vec3(0.0f, 1.0f, 0.0f),
		axZ = glm::vec3(0.0f, 0.0f, 1.0f);

	// los puntos de la caja son los 6 extremos limites de la elipse, los usare para generar
	// los ejes de rotacion en en origen que pueden tener las primitivas
	// 0 3 delimitan el eje de rotacion
	// 6 9 delimitan la afirmacion
	// 12 15 delimitan la negacion
	// hay que pensarlo como si fuera una mano 
	//
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
	void impEjes() {
		std::cout << "EJES DE LA PRIMITVA" << std::endl;
		std::cout << "Eje X: (" << axX.x << ", " <<
			axX.y << ", " << axX.z << ")" << std::endl;
		std::cout << "Eje Y: (" << axY.x << ", " <<
			axY.y << ", " << axY.z << ")" << std::endl;
		std::cout << "Eje Z: (" << axZ.x << ", " <<
			axZ.y << ", " << axZ.z << ")" << std::endl;
	}
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
	void rotate(glm::vec3 RotationAxis, float RotationAngle) {
		for (int i = 0; i < cantidad_puntos; i++) {
			glm::vec3 point = glm::vec3(ppuntos[11 * i], ppuntos[11 * i + 1], ppuntos[11 * i + 2]);
			rotatePoint(RotationAxis, RotationAngle, point);
			ppuntos[11 * i] = point.x;
			ppuntos[11 * i + 1] = point.y;
			ppuntos[11 * i + 2] = point.z;
		}
	}
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
	void generarReflejos() {
		for (int i = 0; i < cantidad_triangulos; i++) {
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
	void buffering() {
		generarReflejos();
		//std::cout << " Cargando al buffer desde: " << 11*pidx << " y " <<  3*tidx << std::endl;
		//std::cout << "  Origen en : (" << origen.x << ", " << origen.y << ", " << origen.z << ")" << std::endl;
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
	void printTriangulos() {
		for (int i = 0; i < cantidad_triangulos; i++) {
			std::cout << pindices[3 * i] << "-"
				<< pindices[3 * i + 1] << "-"
				<< pindices[3 * i + 2] << "-" << std::endl;
		}
	}
	void trasladarOrigen(glm::vec3 nuevoOrigen) {
		origen = nuevoOrigen;
		buffering();
	}
};

Primitiva create(GLfloat x, GLfloat y, GLfloat z, float a, float b, float c, unsigned int np, unsigned int nt);

class Cabeza {
	//las Primitivas individuales
	Primitiva craneo, nariz;
	//uniones hacia el cuello
	GLfloat* u_cuello_x, u_cuello_y, u_cuello_z;
	//uniones hacia la nariz distintiva
	GLfloat* u_nariz_x, u_nariz_y, u_nariz_z;
	//vectores de rotacion
	void afirmacion(GLfloat angulo) {
		//rotamos en angulo de [-45,45]
		//negativo = favor de la palma
		//positivo = contra de la palma

	}
	void negacion(GLfloat angulo) {
		//rotamos en angulo de [-45,45]
		//positivo = contra del pulgar

	}
	void rotacion(GLfloat angulo) {
		//rotamos en angulo de [-90,90]
		//negativo = horario
		//positivo = antihorario
	}
};

class Antebrazo {
public:
	Primitiva antebrazo, mano;
	GLfloat u_muneca_x = 0.0, u_muneca_y = 0.0, u_muneca_z = 0.0;
	glm::vec3 val_rot = glm::vec3(0.0, 0.0, 0.0);
	std::vector<GLfloat> lim_rot = {
		-90.0f, 0.0f,
		-90.0f, 90.0f
	};

	Antebrazo() {

	}

	Antebrazo(GLfloat x, GLfloat y, GLfloat z, float a, float b, float c, unsigned int np, unsigned int nt) {
		antebrazo = create(x, y, z, a, b, c, np, nt);
		updArticulacion();

		np += antebrazo.cantidad_puntos;
		nt += antebrazo.cantidad_triangulos;
		mano = create(u_muneca_x, u_muneca_y, u_muneca_z, a / 2.5, b / 1.5, c / 2, np, nt);
	}

	void cargar(GLfloat x, GLfloat y, GLfloat z, float a, float b, float c, unsigned int np, unsigned int nt) {
		antebrazo = create(x, y, z, a, b, c, np, nt);
		updArticulacion();

		np += antebrazo.cantidad_puntos;
		nt += antebrazo.cantidad_triangulos;
		mano = create(u_muneca_x, u_muneca_y, u_muneca_z, a / 2.5, b / 1.5, c / 2, np, nt);
	}

	void updArticulacion() {
		GLuint muneca = 11 * antebrazo.caja[lsup];
		u_muneca_x = antebrazo.origen.x + antebrazo.ppuntos[muneca];
		u_muneca_y = antebrazo.origen.y + antebrazo.ppuntos[muneca + 1];
		u_muneca_z = antebrazo.origen.z + antebrazo.ppuntos[muneca + 2];
	}

	void buffering() {
		antebrazo.buffering();
		mano.buffering();
	}

	void trasladar(glm::vec3 nuevoOrigen) {
		antebrazo.trasladarOrigen(nuevoOrigen);
		updArticulacion();
		mano.trasladarOrigen(glm::vec3(u_muneca_x, u_muneca_y, u_muneca_z));
	}

	void rotate(glm::vec3 RotationAxis, float RotationAngle) {
		antebrazo.rotate(RotationAxis, RotationAngle);
		mano.trasladarOrigen(antebrazo.origen);
		mano.rotate(antebrazo.axX, RotationAngle);
		updArticulacion();
		mano.trasladarOrigen(glm::vec3(u_muneca_x, u_muneca_y, u_muneca_z));
	}

	void flexion(GLfloat ang, Primitiva brazo) {
		//flexionamos en angulo de [-90,0]
		//negativo = jalon
		GLfloat nval_rot = val_rot.x + ang;

		if (lim_rot[0] > nval_rot)
			ang = lim_rot[0] - val_rot.x;
		if (lim_rot[1] < nval_rot)
			ang = lim_rot[1] - val_rot.x;

		val_rot.x += ang;
		antebrazo.rot('X', ang, brazo);

		mano.trasladarOrigen(antebrazo.origen);
		mano.rotate(antebrazo.axX, ang);

		updArticulacion();
		mano.trasladarOrigen(glm::vec3(u_muneca_x, u_muneca_y, u_muneca_z));
	}

	void rotacion(GLfloat ang, Primitiva brazo) {
		//rotamos en angulo de [-90,90]
		//negativo = horario
		//positivo = antihorario
		GLfloat nval_rot = val_rot.y + ang;

		if (lim_rot[2] > nval_rot)
			ang = lim_rot[2] - val_rot.x;
		if (lim_rot[3] < nval_rot)
			ang = lim_rot[3] - val_rot.x;

		val_rot.y += ang;
		std::cout << "Flexion a " << ang << "grados" << std::endl;
		antebrazo.impEjes();
		antebrazo.rot('Y', ang, brazo);
		mano.trasladarOrigen(antebrazo.origen);
		mano.rotate(antebrazo.axY, ang);

		updArticulacion();
		antebrazo.impEjes();

		mano.trasladarOrigen(glm::vec3(u_muneca_x, u_muneca_y, u_muneca_z));
	}


};

class Brazo : public Antebrazo {
public:
	//las Primitivas individuales
	Antebrazo antebrazo;
	Primitiva brazo;
	//uniones hacia el hombro
	//GLfloat* u_hombro_x, u_hombro_y, u_hombro_z;
	//uniones hacia el antebrazo
	GLfloat u_codo_x = 0.0, u_codo_y = 0.0, u_codo_z = 0.0;
	std::vector<GLfloat> lim_rot = {
		-110.0f, 10.0f,
		0.0f, 110.0f,
		-20.0f, 90.0f
	};

	Brazo() {

	}

	Brazo(GLfloat x, GLfloat y, GLfloat z, float a, float b, float c, unsigned int np, unsigned int nt) {
		brazo = create(x, y, z, a, b, c, np, nt);
		updArticulacion();

		np += brazo.cantidad_puntos;
		nt += brazo.cantidad_triangulos;
		antebrazo.cargar(u_codo_x, u_codo_y, u_codo_z, a * 1.001, b, c, np, nt);
	}

	void cargar(GLfloat x, GLfloat y, GLfloat z, float a, float b, float c, unsigned int np, unsigned int nt) {
		brazo = create(x, y, z, a, b, c, np, nt);
		updArticulacion();

		np += brazo.cantidad_puntos;
		nt += brazo.cantidad_triangulos;
		antebrazo.cargar(u_muneca_x, u_muneca_y, u_muneca_z, a, b, c, np, nt);
	}

	void buffering() {
		brazo.buffering();
		antebrazo.buffering();
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

	void flexion(GLfloat ang, Primitiva hombro) {
		//flexionamos en angulo de [-110,10]
		//negativo = jalon
		//positivo = empujon
		GLfloat nval_rot = val_rot.x + ang;

		if (lim_rot[0] > nval_rot)
			ang = lim_rot[0] - val_rot.x;
		if (lim_rot[1] < nval_rot)
			ang = lim_rot[1] - val_rot.x;

		val_rot.z += ang;
		//en realidad va a ser hombro
		brazo.rot('X', ang, hombro);
		antebrazo.trasladar(brazo.origen);
		antebrazo.rotate(brazo.axX, ang);

		updArticulacion();

		antebrazo.trasladar(glm::vec3(u_codo_x, u_codo_y, u_codo_z));
	}

	void alzar(GLfloat ang, Primitiva hombro) {
		//rotamos en angulo de [0,110]
		//positivo = levantar
		GLfloat nval_rot = val_rot.y + ang;

		if (lim_rot[4] > nval_rot)
			ang = lim_rot[4] - val_rot.y;
		if (lim_rot[5] < nval_rot)
			ang = lim_rot[5] - val_rot.y;

		val_rot.y += ang;
		//en realidad va a ser hombro
		brazo.rot('Y', ang, hombro);
		antebrazo.trasladar(brazo.origen);
		antebrazo.rotate(brazo.axY, ang);

		updArticulacion();

		antebrazo.trasladar(glm::vec3(u_codo_x, u_codo_y, u_codo_z));
	}

	void rotacion(GLfloat ang, Primitiva hombro) {
		//rotamos en angulo de [-20,90]
		//negativo = pulgar abajo
		//positivo = pulgar arriba

		GLfloat nval_rot = val_rot.z + ang;

		if (lim_rot[2] > nval_rot)
			ang = lim_rot[2] - val_rot.z;
		if (lim_rot[3] < nval_rot)
			ang = lim_rot[3] - val_rot.z;

		val_rot.x += ang;
		//en realidad va a ser hombro
		brazo.rot('Z', ang, hombro);
		antebrazo.trasladar(brazo.origen);
		antebrazo.rotate(brazo.axZ, ang);

		updArticulacion();

		antebrazo.trasladar(glm::vec3(u_codo_x, u_codo_y, u_codo_z));
	}
};

class Pie {
	Primitiva planta;
	GLfloat* u_talon_x, u_talon_y, u_talon_z;

	void afirmacion(GLfloat angulo) {
		//rotamos en angulo de [-45,90]
		//negativo = contra ballerina
		//positivo = ballerina

	}
	void flexion(GLfloat angulo) {
		//rotamos en angulo de [-10,10]
		//negativo = izquierda
		//positivo = derecha

	}
	void rotacion(GLfloat angulo) {
		//rotamos en angulo de [-30,30]
		//negativo = izquierda
		//positivo = derecha

	}
};

class Chamorro {
	Primitiva pantorrilla;
	Pie pie;
	GLfloat* u_rodilla_x, u_rodilla_y, u_rodilla_z;
	GLfloat* u_talon_x, u_talon_y, u_talon_z;
	void flexion(GLfloat ang) {
		//flexionamos en angulo de [-90,0]
		//negativo = flexion
	}
};

class Pierna {
	//las Primitivas individuales
	Primitiva muslo;
	Chamorro chamorro;
	GLfloat* u_cadera_x, u_cadera_y, u_cadera_z;
	GLfloat* u_rodilla_x, u_rodilla_y, u_rodilla_z;
	void alzar(GLfloat angulo) {
		//rotamos en angulo de [-100,10]
		//negativo = levantar
		//positivo = bajar

	}
	void apertura(GLfloat angulo) {
		//rotamos en angulo de [-10,90]
		//negativo = izquierda
		//positivo = derecha

	}
	void rotacion(GLfloat angulo) {
		//rotamos en angulo de [0,20]
		//positivo = hacia afuera

	}
};

class Marioneta {
	//las Primitivas individuales
	Primitiva cuello, abdomen, hombro, cadera;
	Pierna ipierna, dpierna;
	Brazo ibrazo, dbrazo;
	Cabeza cabeza;
	//uniones hacia la cabeza
	GLfloat* u_cabeza_x, u_cabeza_y, u_cabeza_z;
	//uniones hacia el pecho
	GLfloat* u_pecho_x, u_pecho_y, u_pecho_z;
	//uniones hacia los hombros
	GLfloat* u_dhombro_x, u_dhombro_y, u_dhombro_z;
	GLfloat* u_ihombro_x, u_ihombro_y, u_ihombro_z;
	//uniones hacia las piernas
	GLfloat* u_dcadera_x, u_dcadera_y, u_dcadera_z;
	GLfloat* u_icadera_x, u_icadera_y, u_icadera_z;
	//uniones hacia el coxis
	GLfloat* u_pelvis_x, u_pelvis_y, u_pelvis_z;
};






#endif
