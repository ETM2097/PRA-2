#include "BrazoRobotico.h"
#include <iostream>

using namespace std;

int main(){
	BrazoRobotico Brazo1 = new BrazoRobotico();
	Brazo1->mover(1.0, 2.0, 3.0);
	cout << Brazo1->get_x() << " " << Brazo1->get_y() << " " 
		<< Brazo1->get_z() << endl;

	return 0;
}
	
