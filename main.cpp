// Raytracer
// Author: Jorge Ribeiro
// Course: Graphic Computing
// Reads a file and draws the scene based on the content of the file

// Reading a file:
// Ns Nl
// Ns: Number of spheres
// X Y Z R: X, Y, Z and Radius of the sphere
// Ir Ig Ib Is: Color
// Nl: Number of light sources
// X Y Z: Position
// R G B s: Color

#include "Util.h"

int main (int argc, char *argv[]) {
	// Read the file to load data
	ifstream file(argv[1]);

	int k;
	vector<int> quant;
	string line;
	getline(file, line);
	istringstream iss(line);
	while(iss >> k) {
		quant.push_back(k);
	}

	// Get spheres data
	double m;
	vector<vector<double> > spheres(quant[0] * 2);
	for(int i = 0; i < quant[0] * 2; i++) {
		getline(file, line);
		istringstream iss(line);
		while(iss >> m) {
			spheres[i].push_back(m);
		}
	}

	// Get light sources data
	vector<vector<double> > lightsources(quant[1] * 2);
	for(int i = 0; i < quant[1] * 2; i++) {
		getline(file, line);
		istringstream iss(line);
		while(iss >> m) {
			lightsources[i].push_back(m);
		}
	}

	cout << "Calculating time:" << endl;
	
	clock_t t1, t2;
	t1 = clock();
	
	int dpi = 72;
	int width = 640, height = 480;
	int n = width * height;
	RGBType *pixels = new RGBType[n];
	
	int aadepth = 1;
	double aathreshold = 0.1;
	double aspectratio = (double)width/(double)height;
	double ambientlight = 0.2;
	double accuracy = 0.00000001;
	
	Vect O (0,0,0);
	Vect X (1,0,0);
	Vect Y (0,1,0);
	Vect Z (0,0,1);
	
	Vect new_sphere_location (1.75, -0.25, 0);
	
	Vect campos (0, 2, -9);
	
	Vect look_at (0, 0, 0);
	Vect diff_btw (campos.getVectX() - look_at.getVectX(), campos.getVectY() - look_at.getVectY(), campos.getVectZ() - look_at.getVectZ());
	
	Vect camdir = diff_btw.negative().normalize();
	Vect camright = Y.crossProduct(camdir).normalize();
	Vect camdown = camright.crossProduct(camdir);
	Camera scene_cam (campos, camdir, camright, camdown);
	
	Color white_light (1.0, 1.0, 1.0, 0);
	Color pretty_green (0.5, 1.0, 0.5, 0.3);
	Color maroon (0.5, 0.25, 0.25, 0);
	Color tile_floor (0, 0, 3, 2);
	Color gray (0.5, 0.5, 0.5, 0);
	Color black (0.0, 0.0, 0.0, 0);
	
	vector<Object*> scene_objects;
	Plane scene_plane (Y, -1, tile_floor);
	scene_objects.push_back(dynamic_cast<Object*>(&scene_plane));
	// Create spheres
	for(int i = 0; i < quant[0] * 2; i++) {
		Vect center (spheres[i][0], spheres[i][1], spheres[i][2]);
		double radius = spheres[i][3];
		i++;
		Color color (spheres[i][0], spheres[i][1], spheres[i][2], spheres[i][3]);
		scene_objects.push_back(dynamic_cast<Object*>(new Sphere (center, radius, color)));
	}

	// Create light sources
	vector<Source*> light_sources;
	for(int i = 0; i < quant[1] * 2; i++) {
		Vect position (lightsources[i][0], lightsources[i][1], lightsources[i][2]);
		i++;
		Color color (lightsources[i][0], lightsources[i][1], lightsources[i][2], lightsources[i][3]);
		light_sources.push_back(dynamic_cast<Source*>(new Light (position, color)));
	}
	
	int thisone, aa_index;
	double xamnt, yamnt;
	double tempRed, tempGreen, tempBlue;
	
	// Run through each pixel
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			thisone = y*width + x;
			
			// start with a blank pixel
			double tempRed[aadepth*aadepth];
			double tempGreen[aadepth*aadepth];
			double tempBlue[aadepth*aadepth];
			
			for (int aax = 0; aax < aadepth; aax++) {
				for (int aay = 0; aay < aadepth; aay++) {
			
					aa_index = aay*aadepth + aax;
					
					srand(time(0));
					
					// create the ray from the camera to this pixel
					if (aadepth == 1) {
					
						// start with no anti-aliasing
						if (width > height) {
							// the image is wider than it is tall
							xamnt = ((x+0.5)/width)*aspectratio - (((width-height)/(double)height)/2);
							yamnt = ((height - y) + 0.5)/height;
						}
						else if (height > width) {
							// the imager is taller than it is wide
							xamnt = (x + 0.5)/ width;
							yamnt = (((height - y) + 0.5)/height)/aspectratio - (((height - width)/(double)width)/2);
						}
						else {
							// the image is square
							xamnt = (x + 0.5)/width;
							yamnt = ((height - y) + 0.5)/height;
						}
					}
					else {
						// anti-aliasing
						if (width > height) {
							// the image is wider than it is tall
							xamnt = ((x + (double)aax/((double)aadepth - 1))/width)*aspectratio - (((width-height)/(double)height)/2);
							yamnt = ((height - y) + (double)aax/((double)aadepth - 1))/height;
						}
						else if (height > width) {
							// the imager is taller than it is wide
							xamnt = (x + (double)aax/((double)aadepth - 1))/ width;
							yamnt = (((height - y) + (double)aax/((double)aadepth - 1))/height)/aspectratio - (((height - width)/(double)width)/2);
						}
						else {
							// the image is square
							xamnt = (x + (double)aax/((double)aadepth - 1))/width;
							yamnt = ((height - y) + (double)aax/((double)aadepth - 1))/height;
						}
					}
					
					Vect cam_ray_origin = scene_cam.getCameraPosition();
					Vect cam_ray_direction = camdir.vectAdd(camright.vectMult(xamnt - 0.5).vectAdd(camdown.vectMult(yamnt - 0.5))).normalize();
					
					Ray cam_ray (cam_ray_origin, cam_ray_direction);
					
					vector<double> intersections;
					
					for (int index = 0; index < scene_objects.size(); index++) {
						intersections.push_back(scene_objects.at(index)->findIntersection(cam_ray));
					}
					
					int index_of_winning_object = winningObjectIndex(intersections);
					
					if (index_of_winning_object == -1) {
						// set the backgroung black
						tempRed[aa_index] = 0;
						tempGreen[aa_index] = 0;
						tempBlue[aa_index] = 0;
					}
					else{
						// index coresponds to an object in our scene
						if (intersections.at(index_of_winning_object) > accuracy) {
							// determine the position and direction vectors at the point of intersection
							
							Vect intersection_position = cam_ray_origin.vectAdd(cam_ray_direction.vectMult(intersections.at(index_of_winning_object)));
							Vect intersecting_ray_direction = cam_ray_direction;
		
							Color intersection_color = getColorAt(intersection_position, intersecting_ray_direction, scene_objects, index_of_winning_object, light_sources, accuracy, ambientlight);
							
							tempRed[aa_index] = intersection_color.getColorRed();
							tempGreen[aa_index] = intersection_color.getColorGreen();
							tempBlue[aa_index] = intersection_color.getColorBlue();
						}
					}
				}
			}
			
			// average the pixel color
			double totalRed = 0;
			double totalGreen = 0;
			double totalBlue = 0;
			
			for (int iRed = 0; iRed < aadepth*aadepth; iRed++) {
				totalRed = totalRed + tempRed[iRed];
			}
			for (int iGreen = 0; iGreen < aadepth*aadepth; iGreen++) {
				totalGreen = totalGreen + tempGreen[iGreen];
			}
			for (int iBlue = 0; iBlue < aadepth*aadepth; iBlue++) {
				totalBlue = totalBlue + tempBlue[iBlue];
			}
			
			double avgRed = totalRed/(aadepth*aadepth);
			double avgGreen = totalGreen/(aadepth*aadepth);
			double avgBlue = totalBlue/(aadepth*aadepth);
			
			pixels[thisone].r = avgRed;
			pixels[thisone].g = avgGreen;
			pixels[thisone].b = avgBlue;
		}
	}
	
	saveBMP("image.bmp",width,height,dpi,pixels);
	
	delete pixels, tempRed, tempGreen, tempBlue;
	
	t2 = clock();
	float diff = ((float)t2 - (float)t1)/1000;
	
	cout << diff << " seconds" << endl;
	
	return 0;
}
