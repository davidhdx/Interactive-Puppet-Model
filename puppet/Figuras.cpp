#include "Figuras.h"



std::vector<GLfloat> calcularNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {

	glm::vec3 U = glm::vec3(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
	glm::vec3 V = glm::vec3(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);

	std::vector<GLfloat> resultado = { -(U.y * V.z - U.z * V.y), -(U.z * V.x - U.x * V.z), -(U.x * V.y - U.y * V.x) };

	return resultado;
}

void cleanBuffer() {
	for (int i = 0; i < 10 * 11 * puntos_prim; i++)
		vertices[i] = 0.0;
	for (int i = 0; i < 10 * 3 * triangulos_prim; i++)
		indices[i] = 0;
}

void meterTriangulo(unsigned int a, unsigned int b, unsigned int c) {
	a *= 3;
	b *= 3;
	c *= 3;

	glm::vec3
		p1 = glm::vec3(puntos[a], puntos[a + 1], puntos[a + 2]),
		p2 = glm::vec3(puntos[b], puntos[b + 1], puntos[b + 2]),
		p3 = glm::vec3(puntos[c], puntos[c + 1], puntos[c + 2]);

	normal = calcularNormal(p1, p2, p3);

	unsigned int n = vertice.size() / 11;
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

Primitiva create(GLfloat x, GLfloat y, GLfloat z, float a, float b, float c, unsigned int np, unsigned int nt) {
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
	return resultado;
}



/*|AQUI TERMINA LO QUE DAVID SE PUSO A HACER FAVOR DE MODIFICAR NO SEAN CULXS| */
