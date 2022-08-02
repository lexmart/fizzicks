
#ifndef __wasm__
	#include <math.h>
#endif

struct V2 {
	float x, y;
};

inline V2 operator+(V2 a, V2 b) {
	V2 result = {a.x + b.x, a.y + b.y};
	return result;
}

inline V2 operator-(V2 a, V2 b) {
	V2 result = {a.x - b.x, a.y - b.y};
	return result;
}

inline V2 operator-(V2 a) {
	V2 result = {-a.x, -a.y};
	return result;
}

inline V2 operator*(float a, V2 b) {
	V2 result = {a*b.x, a*b.y};
	return result;
}

inline V2 rotate90c(V2 v) {
	V2 result = {v.y, -v.x};
	return result;
}

inline V2 rotate90cc(V2 v) {
	V2 result = {-v.y, v.x};
	return result;
}

inline float dot(V2 a, V2 b) {
	return a.x*b.x + a.y*b.y;
}

inline float lengthSq(V2 v) {
	return dot(v, v);
}

inline float length(V2 v) {
	return sqrtf(lengthSq(v));
}

inline V2 normalize(V2 v) {
	return (1.0f/length(v))*v;
}

inline bool operator==(V2 a, V2 b) {
	return a.x == b.x && a.y == b.y;
}

inline float cross(V2 a, V2 b) {
	return a.x*b.y - a.y*b.x;
}

// <0,0,w> cross <rx,ry,rz> = <-w*ry,w*rx,0>
inline V2 cross(float w, V2 r) {
	V2 result = {-w*r.y, w*r.x};
	return result;
}

struct Box {
	V2 p;
	float rot;
	V2 hdim;
	V2 ax;
	V2 ay;
	float mass;
	float I;

	V2 v;
	float w;

	bool gravity;

	Box() {}

	Box(float x, float y, float w, float h, float rot, float mass, bool gravity) {
		p.x = x;
		p.y = y;
		hdim.x = 0.5f*w;
		hdim.y = 0.5f*h;
		this->rot = rot;
		rotate(rot);
		this->mass = mass;
		I = (mass/3.0f)*(w*w + h*h);

		v = {};
		this->w = 0.0f;
		this->gravity = gravity;
	}

	void rotate(float tetha) {
		rot += tetha;
		ax.x = cosf(rot);
		ax.y = sinf(rot);
		ay.x = -ax.y;
		ay.y = ax.x;
	}
};

struct Line {
	V2 start;
	V2 end;
	float r, g, b;
};

struct Contact {
	V2 p;
	V2 n;
	float pen;
	Box *A;
	Box *B;
	float accP;
	float accPT;
};

struct State {
	bool init;
	float t;
	Box boxes[4096];
	int boxCount;

	Contact contacts[4096];
	int contactCount;

	Line lines[128];
	int lineCount;

	float pixelsPerMeter;
	V2 camP;

	bool paused;
	bool shoot;

	float partyTime;
};



typedef void TickFunction(State *state, float dt);


