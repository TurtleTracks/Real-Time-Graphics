#include "utilities.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace Utilities {

	// Uniform Samplers written with help of PBRT, Second Edition /////////////

	/**
	* Uniformly sample a vector on a 2D disk with radius 1, centered around the origin
	* @param sample a 2D point in [0,1]^2
	* @param output the receiver of the sampled vector
	*/
	void squareToUniformDisk(const glm::vec2 sample, glm::vec2 &output) {
		double r = glm::sqrt(sample.x);
		double theta = 2 * PI * sample.y;
		output.x = r * glm::cos(theta);
		output.y = r * glm::sin(theta);

	}

	/**
	* Probability density of squareToUniformDisk()
	* @param p the sampled point
	* @return the probability density of squareToUniformDisk() to sample p
	*/
	double squareToUniformDiskPdf(const glm::vec2 p) {
		if (p.x * p.x + p.y * p.y > 1 + EPSILON)
			return 0;
		return 1 / (PI);
	}

	/**
	* Uniformly sample a vector on the unit hemisphere around the pole (0,0,1) 
	* with respect to solid angles
	* (fast implementation)
	* @param sample a point uniformly sampled from [0,1]^2
	* @param output the receiver of the sampled vector
	*/
	void squareToUniformHemisphere(const glm::vec2 sample, glm::vec3 &output) {
		double z = 1 - sample.x;
		double r = glm::sqrt(glm::max(0.0, 1 - z * z));
		double phi = 2 * PI * sample.y;
		double x = r * glm::cos(phi);
		double y = r * glm::sin(phi);
		output = glm::vec3(x, y, z);
	}

	/**
	* Probability density of squareToUniformHemisphere()
	* @param v the sampled vector
	* @return the probability density of squareToUniformSphere() to sample v
	*/
	double squareToUniformHemispherePdf(const glm::vec3 v) {
		// TODO: Implement this method.
		if (v.z < 0 || v.x*v.x + v.y*v.y + v.z*v.z > 1 + EPSILON)
			return 0;
		return 1 / (2 * PI);
	}

	/**
	* Uniformly sample a vector on the unit hemisphere around the pole (0,0,1) 
	* with respect to projected solid angles
	* @param sample a point uniformly sampled from in [0,1]^2
	* @param output the receiver of the sampled vector
	*/
	void squareToCosineHemisphere(const glm::vec2 sample, glm::vec3 &output) {
		glm::vec2 pre = glm::vec2();
		squareToUniformDisk(sample, pre);
		double z = glm::sqrt(
			glm::max(0.0f, 1 - pre.x * pre.x - pre.y * pre.y));
		output.x = pre.x;
		output.y = pre.y;
		output.z = z;
	}

	/**
	* Probability density of squareToCosineHemisphere()
	* @param v the sampled vector
	* @return the probability density of squareToCosineHemisphere() to sample v
	*/
	double squareToCosineHemispherePdf(const glm::vec3 v) {
		if (v.z < 0 || v.x*v.x + v.y*v.y + v.z*v.z > 1 + EPSILON)
			return 0;
		return v.z / PI;
	}

	// Strings and IO /////////////////////////////////////////////////////////

	/* Print a vec4 */
	void printVec(const glm::vec4 p)
	{
		std::cout << "< "
			<< p.x << ", "
			<< p.y << ", "
			<< p.z << ", "
			<< p.w << ", "
			<< " >" << std::endl;
	}

	/* Print a vec3 */
	void printVec(const glm::vec3 p)
	{
		std::cout << "< "
			<< p.x << ", "
			<< p.y << ", "
			<< p.z << ", "
			<< " >" << std::endl;
	}

	/* Print a vec2 */
	void printVec(const glm::vec2 p)
	{
		std::cout << "< "
			<< p.x << ", "
			<< p.y << ", "
			<< " >" << std::endl;
	}
	
	/** Load a file from the correct directory */
	void loadFile(std::string file, std::ifstream &stream)
	{
		stream.open(DATA_FOLDER + file);
	}

	/**
	* Converts string of format "float, float, float"
	* into glm::vec3 
	*/
	glm::vec3 strToVec3(std::string str)
	{
		std::replace(str.begin(), str.end(), ',', ' ');
		glm::vec3 v;
		std::stringstream ss(str);
		float n;
		int i = 0;
		while (ss >> n && i < 3) {
			v[i] = n;
			i++;
		}
		return v;
	}


	// Frames /////////////////////////////////////////////////////////////////

	/**
	 * Create a coordinate frame for the input vector,
	 * input vector must be normalized.
	 */
	Frame::Frame(glm::vec3 norm)
	{
		z = norm;
		float invLen = 1.0 / glm::sqrt(norm.x * norm.x + norm.y * norm.y);
		x = glm::vec3(0, norm.z, -norm.y) * invLen;
		if (glm::abs(norm.x) > glm::abs(norm.y))	
			x = glm::vec3(-norm.z, 0, norm.x) * invLen;
		y = glm::cross(z, x);
	}

	/* Frame to world transform */
	void Frame::frameToWorld(const glm::vec3 v, glm::vec3 & output)
	{
		glm::mat3 FI = glm::mat3(x, y, z);
		FI = glm::inverse(FI);
		output = FI * v;
	}

	/* world to frame transform */
	void Frame::worldToFrame(const glm::vec3 v, glm::vec3 & output)
	{
		glm::mat3 F = glm::mat3(x, y, z);
		output = F * v;
	}

	/* helper function for reverse morton
	 * compacts input binary string 
	 */
	uint32_t Compact1By2(uint32_t x)
	{
		x &= 0x09249249;
		x = (x ^ (x >> 2)) & 0x030c30c3;
		x = (x ^ (x >> 4)) & 0x0300f00f;
		x = (x ^ (x >> 8)) & 0xff0000ff;
		x = (x ^ (x >> 16)) & 0x000003ff;
		return x;
	}

	///////////////////////////////////////////////////////////////////////////
	/* random helper function, might not be useful */
	glm::vec3 RayOBBIntersect(glm::dvec3 o, glm::dvec3 d)
	{
		glm::dvec3 t = glm::dvec3(-1, -INFINITY, INFINITY);
		glm::dvec3 halfv = glm::dvec3(0.5f, 0.5f, 0.5f);
		glm::dvec3 ac = halfv;
		glm::dvec3 p = ac - o;
		double h[3] = { halfv.x, halfv.y, halfv.z };
		glm::dvec3 a[3] = { glm::dvec3(1, 0, 0), glm::dvec3(0, 1, 0), glm::dvec3(0, 0, 1) };

		for (int i = 0; i < 3; i++) 
		{
			double e = glm::dot(a[i], p);
			double f = glm::dot(a[i], d);
			if (glm::abs(f) > EPSILON) 
			{
				double t1 = (e + h[i]) / f;
				double t2 = (e - h[i]) / f;
				if (t1 > t2) 
				{
					double temp = t1;
					t1 = t2;
					t2 = temp;
				}
				if (t1 > t.y)
				{
					t.y = t1;
				}
				if (t2 < t.z) 
				{
					t.z = t2;
				}
				if (t.y > t.z || t.z < 0) 
				{
					return t;
				}
			}
			else if (-EPSILON - h[i] > 0 || -EPSILON + h[i] < 0) 
			{
				return t;
			}
		}
		t.x = 0;
		return t;
	}
}