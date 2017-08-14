#include "particle-system.hpp"
#include "utilities.hpp"
#include <glm\glm.hpp>
#include <glm\common.hpp>

using namespace Utilities;
// constants		////
const double eps = 0.0001;
const double mass = 0.02;
const double restDensity = 2.773 * 1000;
const float separation = 0.0173f;
const float mOverRho = mass / restDensity;
const int gridsz = 32;
const float H = 1.0 / gridsz;
const float HH = H * H;
const double _poly6 = 315.0 / (64 * PI * glm::pow(H, 9));
const float _spiky = -45.0 / (PI * glm::pow(H, 6));
const double DT = 1.0 / 60.0;
const int maxZidx = 229377;
const glm::vec3 vec0 = glm::vec3(0);
const glm::vec3 maxAABB = glm::vec3(1);

// state			////
const int solverIterations = 4;
double time;
const int stepsPerFrame = 4;
int stepsTillSort = 0; // modulo 100

// data structures	////
struct Handle
{
	unsigned int zidx;
	Particle *particle;
};

struct Cell
{
	bool empty;
	int firstHandlePos;
};

std::vector<Cell> cells(maxZidx);
std::vector<std::vector<Particle*>> neighborhoods(maxZidx);
std::vector<Handle> handles;
std::vector<glm::vec3> ncellVector(27);



/* Initialize Particle System */
void ParticleSystem::init()
{
	glm::vec3 g = glm::vec3(0, -9.8, 0);
	forces.push_back(g);

	ncellVector =
	{
		glm::vec3(0, 0, 0),	 glm::vec3(-1,0,0),  glm::vec3(1,0,0),
		glm::vec3(0, 0, -1), glm::vec3(-1,0,-1), glm::vec3(1,0,-1),
		glm::vec3(0, 0, 1),  glm::vec3(-1,0,1),  glm::vec3(1,0,1),
		glm::vec3(0, -1, 0), glm::vec3(-1,-1,0), glm::vec3(1,-1,0),
		glm::vec3(0, -1, -1), glm::vec3(-1,-1,-1), glm::vec3(1,-1,-1),
		glm::vec3(0, -1, 1), glm::vec3(-1,-1,1), glm::vec3(1,-1,1),
		glm::vec3(0, 1, 0),  glm::vec3(-1,1,0),  glm::vec3(1,1,0),
		glm::vec3(0, 1, -1), glm::vec3(-1,1,-1), glm::vec3(1,1,-1),
		glm::vec3(0, 1, 1),  glm::vec3(-1,1,1),  glm::vec3(1,1,1),
	};

	for (int i = 0; i < ncellVector.size(); i++)
	{
		ncellVector[i] = ncellVector[i] * H;
	}
}

/* get z-order index from position */
unsigned int ParticleSystem::getZidx(glm::vec3 pos)
{
	pos /= H;
	if (pos.x < 0) return -1;
	if (pos.y < 0) return -1;
	if (pos.z < 0) return -1;
	unsigned char x = (unsigned int)pos.x;
	unsigned char y = (unsigned int)pos.y;
	unsigned char z = (unsigned int)pos.z;
	unsigned int index = 0;
	index = Morton[x] | Morton[y] << 1 | Morton[z] << 2;
	return index;
}

glm::vec3 ParticleSystem::decodeIdx(unsigned int idx)
{
	unsigned int x = Compact1By2(idx);
	unsigned int y = Compact1By2(idx >> 1);
	unsigned int z = Compact1By2(idx >> 2);
	return glm::vec3(x, y, z) / (float)gridsz;
}

/* store all cell neighbors for each cell */
void ParticleSystem::fillNeighborhoods()
{
	unsigned int idmax = maxZidx;
	for (unsigned int id = 0; id < idmax; id++)
	{
		neighborhoods[id].clear();
		glm::vec3 pos = glm::vec3(decodeIdx(id)) + 1.0f/(2*gridsz);
		for (int i = 0; i < 27; i++)
		{
			glm::vec3 cellPos = pos + ncellVector[i];
			unsigned int ind = getZidx(cellPos);
			if (ind < cells.size() && !cells[ind].empty)
			{
				unsigned int ppos = cells[ind].firstHandlePos;
				while ((ppos) < particles.size() && handles[ppos].zidx == ind)
				{
					neighborhoods[id].push_back(handles[ppos].particle);
					ppos++;
				}
			}
		}
	}
}

/* Sort Handle vector (and update with correct index values) */
void updateHandles()
{
	for (unsigned int i = 1; i < handles.size(); i++)
	{
		handles[i].zidx = handles[i].particle->zidx;
		Handle handle = handles[i];
		unsigned int ind = handles[i].particle->zidx;
		
		unsigned int pos = i;
		while (pos > 0 && handles[pos - 1].zidx > ind)
		{
			handles[pos] = handles[pos - 1];
			pos -= 1;
		}
		if(pos != i) handles[pos] = handle;
	}
}

/* regenerate handles (needed after particles sort) */
void ParticleSystem::resetHandles()
{
	if (particles.size() > handles.size()) handles.resize(particles.size());
	for (unsigned int i = 1; i < handles.size(); i++)
	{
		handles[i].particle = &particles[i];
		handles[i].zidx = particles[i].zidx;
	}
}

/* update cells with position to first Particle in particles vector */
void updateCells()
{
	for (unsigned int i = 0; i < cells.size(); i++)
	{
		cells[i].empty = true;
		cells[i].firstHandlePos = 0;
	}
	unsigned int ind = 0;
	for (unsigned int i = 0; i < handles.size(); i++)
	{
		while (handles[i].zidx > ind && ind < cells.size())
		{
			ind++;
		}
		if (cells[ind].empty) 
		{
			cells[ind].firstHandlePos = i; // order handles != order partcls
			cells[ind].empty = false;
		}
	}
}

/* Index sort on particle vector */
void ParticleSystem::sortParticles()
{
	for (unsigned int i = 1; i < particles.size(); i++)
	{
		Particle particle = particles[i];
		unsigned int ind = particles[i].zidx;
		unsigned int pos = i;
		while (pos > 0 && particles[pos - 1].zidx > ind)
		{
			particles[pos] = particles[pos - 1];
			pos -= 1;
		}
		if (pos != i) particles[pos] = particle;
	}
}

double weightFunction(double rr)
{
	double hhrr = HH - rr;
	double hhrr3 = hhrr * hhrr * hhrr;
	return (rr < HH && 0 < rr) ?_poly6 * hhrr3 : 0;
}

glm::vec3 weightGradient(const glm::vec3 &pipj)
{
	float r = glm::length(pipj);
	float hr2 = (H - r) * (H - r);
	return (r < H && 0 < r) ? _spiky * hr2 * (pipj / r) : glm::vec3(0);
}

double densityConstraint(const Particle &pi, const std::vector<Particle*> &hood)
{
	double estimator = 0;
	for (auto &pj : hood)
	{
		glm::vec3 pipj = pi.pred - (*pj).pred;
		estimator += weightFunction(glm::dot(pipj, pipj)) * mass;
	}
	return (estimator/ restDensity) - 1;
}

double gradientConstraintSums(const Particle &pi, const std::vector<Particle*> &hood)
{
	glm::vec3 sum1 = vec0;
	double sum2 = 0;
	double sum1s = 0;
	for (auto &pj : hood)
	{
		glm::vec3 val = weightGradient(pi.pred - (*pj).pred) * mOverRho;
		sum1 += val;
		sum2 += glm::dot(val, val);
	}
	sum1s = glm::dot(sum1, sum1);
	return (sum1s + sum2);
}

void ParticleSystem::createParticle(glm::vec3 pos)
{
	glm::vec3 o = glm::vec3(0); //empty
	unsigned int ind = getZidx(pos);
	Particle p = { pos, ind, o, 0, o, 0 };
	particles.push_back(p);
	Handle h = { ind, &particles.back() };
	handles.push_back(h);
}

void ParticleSystem::createTower()
{
	int x = int(0.2 * gridsz);
	int y = int(0.7 * gridsz);
	int z = int(0.9 * gridsz);
	particles.reserve(x * y * z * 2);
	for (int i = 0; i < x; i++) 
	{
		for (int j = 0; j < y; j++) 
		{
			for (int k = 0; k < z; k++)  
			{
				createParticle(glm::vec3(i, j, k) * separation);
			}
		}
	}
}

void ParticleSystem::reset()
{
	time = 0;
	particles.clear(); 
}

void ParticleSystem::advanceTime(double dt)
{
	if (stepsTillSort == 0)
	{
		sortParticles();
		resetHandles();
		updateCells();
	} else
	{
		updateHandles();
		updateCells();
	}
	stepsTillSort = (stepsTillSort + 1) % 100;
	
	for (Particle &p : particles)
	{
		// apply forces :
		p.vel += (float)dt * forces[0];  // forces[0] is technically g accel.
		// predict position :
		p.pred = p.pos + (float)dt * p.vel;
	}
	fillNeighborhoods();
	int iter = 0;
	while (iter < solverIterations)
	{
		//int lastIDX = -1;
		//double lastGC = 0;
		for(Particle &p : particles)
		{
			std::vector<Particle*> &hood = neighborhoods[p.zidx];
			// calculate lambda
			double dc = -densityConstraint(p, hood);
			//double gc = lastIDX == p.zidx ? lastGC : gradientConstraintSums(p, hood);
			double gc  = gradientConstraintSums(p, hood);
			p.lambda = dc / (gc + 1000);
			//lastIDX = p.zidx;
			//lastGC = gc;
		}
		for (Particle &p : particles)
		{
			std::vector<Particle*> &hood = neighborhoods[p.zidx];
			// calculate delta_p
			p.delta = vec0;
			for (Particle *pj : hood)
			{
				glm::vec3 pipj = p.pred - (*pj).pred;
				double scale = weightFunction(glm::dot(pipj, pipj))
					/ weightFunction(HH * 0.01);
				scale = scale * scale;
				double scorr = -0.00001 * scale * scale;
				double multiplier = p.lambda + (*pj).lambda + scorr;
				glm::vec3 wg = weightGradient(pipj);
				p.delta += (float)multiplier * wg;
			}
			p.delta *= mass/restDensity;
			p.pred += p.delta;
			p.pred = glm::clamp(p.pred, vec0, maxAABB);
		}
		iter++;
	}
	for (Particle &p : particles)
	{
		// update velocity
		p.vel = (float)(1.0 / dt) * (p.pred - p.pos);
		// apply vorticity confinement and XSPHD viscosity
		// update position
		p.pos = p.pred;
		p.zidx = getZidx(glm::vec3(p.pos));
	}
}

void ParticleSystem::bufferParticles()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle),
		&particles[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), nullptr);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle),
		(const GLvoid*)(sizeof(float) * 4));
	glEnableVertexAttribArray(0);
	glPointSize(5);
}

void ParticleSystem::updateVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle),
		&particles[0], GL_STATIC_DRAW);
}

void ParticleSystem::update()
{
	for (int i = 0; i < stepsPerFrame; i++)
	{
		advanceTime(DT / stepsPerFrame);
	}
}

void ParticleSystem::draw()
{
	glBindVertexArray(VAO);
	updateVBO();
	glDrawArrays(GL_POINTS, 0, particles.size());
}