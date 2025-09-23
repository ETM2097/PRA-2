#ifndef BRAZOROBOTICO_H
#define BRAZOROBOTICO_H

class BrazoRobotico{
	private:
		double x, y, z;
		bool sujetando;

	public:
		// Constructor
		BrazoRobotico();
		// Consultores
		const double get_x();
		const double get_y();
		const double get_z();
		const bool get_sujetando();
		// Métodos
		bool coger();
		bool soltar();
		void mover();
};

#endif
