#include "BrazoRobotico.h"
#include <iostream>

BrazoRobotico::BrazoRobotico() : x(0), y(0), z(0), sujetando(false) {};

const double BrazoRobotico::get_x(){
	return x;
}

const double BrazoRobotico::get_y(){
	return y;
}

const double BrazoRobotico::get_z(){
	return z;
}

const bool BrazoRobotico::get_sujetando(){
	return sujetando;
}

bool BrazoRobotico::coger(){
	if (!sujetando){
		sujetando = true;
		return true;
	}
	else return false;
	return false;
}

bool BrazoRobotico::soltar(){
	if(sujetando){
		sujetando = false;
		return true;
	}
	else return false;
	return false;
}

void BrazoRobotico::mover(double x_, double y_, double z_){
	x = x_;
	y = y_;
	z = z_;
	std::cout << "Done" << std::endl;
}


