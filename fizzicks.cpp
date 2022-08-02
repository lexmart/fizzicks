#include "platform.h"

#define assert(x) if(!(x)) { *((int *)0) = 0; }

inline float min(float a, float b) {
	return (a < b) ? a : b;
}

inline float max(float a, float b) {
	return (a > b) ? a : b;
}

inline float clamp(float x, float min, float max) {
	if(x < min) {
		return min;
	} else if(x > max) {
		return max;
	} else {
		return x;
	}
}

inline float abs(float x) {
	return (x < 0) ? -x : x;
}

inline float testAxis(Box *boxA, Box *boxB, V2 &axis, V2 a1, V2 a2, V2 a3, V2 a4, V2 b1, V2 b2, V2 b3, V2 b4, bool *flipped) {
	V2 BA = boxB->p - boxA->p;
	if(dot(BA, axis) < 0) {
		axis = -axis;
		*flipped = true;
	} else {
		*flipped = false;
	}

	float ra = max(max(dot(a1, axis), dot(a2, axis)), max(dot(a3, axis), dot(a4, axis))) - dot(boxA->p, axis);
	float rb = dot(boxB->p, axis) - min(min(dot(b1, axis), dot(b2, axis)), min(dot(b3, axis), dot(b4, axis)));
	float centersDist = dot(BA, axis);

	float result = centersDist - ra - rb;
	return -result;
}

bool findLineLineIntersection(V2 p1, V2 d1, V2 p2, V2 d2, V2 *hit) {
	bool result = false;

	V2 n = rotate90cc(d2);
	float denom = dot(d1,n);
	if(denom != 0.0f) {
		float t = dot(p2-p1, n)/denom;
		*hit = p1 + t*d1;
		result = true;
	}

	return result;
}

// dir must be a unit vector
void clipEdge(V2 &edge1, V2 &edge2, V2 leftP, V2 rightP, V2 dir) {
	V2 n = edge2 - edge1;
	n = rotate90cc(n);

	V2 edgeDir = edge2 - edge1;

	V2 hit;
	if(findLineLineIntersection(rightP, dir, edge1, edgeDir, &hit)) {

		V2 n = rotate90c(dir);
		if(dot(edge1 - hit, n) > 0) {
			edge1 = hit;
		}

		if(dot(edge2 - hit, n) > 0) {
			edge2 = hit;
		}
	}

	if(findLineLineIntersection(leftP, dir, edge1, edgeDir, &hit)) {
		V2 n = rotate90cc(dir);
		if(dot(edge1 - hit, n) > 0) {
			edge1 = hit;
		}

		if(dot(edge2 - hit, n) > 0) {
			edge2 = hit;
		}
	}	
}

int sat(State *state, Box *boxA, Box *boxB, Contact contacts[2]) {
	int contactCount = 0;

	V2 a1 = boxA->p - boxA->hdim.x*boxA->ax - boxA->hdim.y*boxA->ay;
	V2 a2 = boxA->p + boxA->hdim.x*boxA->ax - boxA->hdim.y*boxA->ay;
	V2 a3 = boxA->p - boxA->hdim.x*boxA->ax + boxA->hdim.y*boxA->ay;
	V2 a4 = boxA->p + boxA->hdim.x*boxA->ax + boxA->hdim.y*boxA->ay;
	V2 b1 = boxB->p - boxB->hdim.x*boxB->ax - boxB->hdim.y*boxB->ay;
	V2 b2 = boxB->p + boxB->hdim.x*boxB->ax - boxB->hdim.y*boxB->ay;
	V2 b3 = boxB->p - boxB->hdim.x*boxB->ax + boxB->hdim.y*boxB->ay;
	V2 b4 = boxB->p + boxB->hdim.x*boxB->ax + boxB->hdim.y*boxB->ay;

	/*
		a3------a4
	   	|		|
	   	|		|
	   	|		|
		a1------a2	
	*/

	// Step1: Find the best seperating axis, early out if seperated.
	// The edge whose normal is the best seperating axis is the reference edge.

	bool flipped;
	V2 refEdge1, refEdge2;
	bool refIsA = true;
	V2 bestSepAxis = boxA->ax;
	float bestPen = testAxis(boxA, boxB, bestSepAxis, a1, a2, a3, a4, b1, b2, b3, b4, &flipped);
	if(bestPen <= 0) {
		return contactCount;
	}
	if(flipped) {
		refEdge1 = a1;
		refEdge2 = a3;
	} else {
		refEdge1 = a4;
		refEdge2 = a2;
	}

	V2 testSepAxis = boxA->ay;
	float testPen = testAxis(boxA, boxB, testSepAxis, a1, a2, a3, a4, b1, b2, b3, b4, &flipped);
	if(testPen <= 0) {
		return contactCount;
	} else if(testPen < bestPen) {
		bestSepAxis = testSepAxis;
		bestPen = testPen;
		if(flipped) {
			refEdge1 = a2;
			refEdge2 = a1;
		} else {
			refEdge1 = a3;
			refEdge2 = a4;
		}
	}

	testSepAxis = boxB->ax;
	testPen = testAxis(boxB, boxA, testSepAxis, b1, b2, b3, b4, a1, a2, a3, a4, &flipped);
	if(testPen <= 0) {
		return contactCount;
	} else if(testPen < bestPen) {
		bestSepAxis = testSepAxis;
		bestPen = testPen;
		refIsA = false;
		if(flipped) {
			refEdge1 = b1;
			refEdge2 = b3;
		} else {
			refEdge1 = b4;
			refEdge2 = b2;
		}
	}

	testSepAxis = boxB->ay;
	testPen = testAxis(boxB, boxA, testSepAxis, b1, b2, b3, b4, a1, a2, a3, a4, &flipped);
	if(testPen <= 0) {
		return contactCount;
	} else if(testPen < bestPen) {
		bestSepAxis = testSepAxis;
		bestPen = testPen;
		refIsA = false;
		if(flipped) {
			refEdge1 = b2;
			refEdge2 = b1;
		} else {
			refEdge1 = b3;
			refEdge2 = b4;
		}
	}

	// Step2: Find the incident edge on the other box that is the most parallel to the reference edge AND has a vertex inside the reference box.

	V2 i1, i2, i3, i4;
	if(refIsA) {
		i1 = b1;
		i2 = b2;
		i3 = b3;
		i4 = b4;
	} else {
		i1 = a1;
		i2 = a2;
		i3 = a3;
		i4 = a4;
	}

	V2 incEdge1, incEdge2;
	V2 refEdge = refEdge2 - refEdge1;
	float bestIncEdge = 0.0f;
	bool incEdgeSet = false;

	float eps = 1e-7;

	float testIncEdge = abs(dot(i2 - i1, refEdge));
	bool under = dot(i1 - refEdge1, bestSepAxis) < eps || dot(i2 - refEdge1, bestSepAxis) < eps;
	if(under && testIncEdge > bestIncEdge) {
		bestIncEdge = testIncEdge;
		incEdge1 = i1;
		incEdge2 = i2;
		incEdgeSet = true;
	}
	
	testIncEdge = abs(dot(i4 - i2, refEdge));
	under = dot(i2 - refEdge1, bestSepAxis) < eps || dot(i4 - refEdge1, bestSepAxis) < eps;
	if(under && testIncEdge > bestIncEdge) {
		bestIncEdge = testIncEdge;
		incEdge1 = i2;
		incEdge2 = i4;
		incEdgeSet = true;
	}

	testIncEdge = abs(dot(i3 - i4, refEdge));
	under = dot(i3 - refEdge1, bestSepAxis) < eps || dot(i4 - refEdge1, bestSepAxis) < eps;
	if(under && testIncEdge > bestIncEdge) {
		bestIncEdge = testIncEdge;
		incEdge1 = i3;
		incEdge2 = i4;
		incEdgeSet = true;
	}

	testIncEdge = abs(dot(i1 - i3, refEdge));
	under = dot(i1 - refEdge1, bestSepAxis) < eps || dot(i3 - refEdge1, bestSepAxis) < eps;
	if(under && testIncEdge > bestIncEdge) {
		bestIncEdge = testIncEdge;
		incEdge1 = i1;
		incEdge2 = i3;
		incEdgeSet = true;
	}

	// Step3: Clip the incidient edge along the sides of the reference edge.

	if(incEdgeSet) {
		clipEdge(incEdge1, incEdge2, refEdge1, refEdge2, bestSepAxis);

		// Step4: Check if 1 or 2 points are penetrating the reference edge

		Box *ref;
		Box *inc;
		if(refIsA) {
			ref = boxA;
			inc = boxB;
		} else {
			ref = boxB;
			inc = boxA;
		}

		float foo = dot(bestSepAxis, incEdge1 - refEdge1);
		float bar = dot(bestSepAxis, incEdge2 - refEdge2);

		if(bar > eps || foo > eps) {
			int baz = 3;
		}

		if(incEdge1 == refEdge1 || incEdge1 == refEdge2 || dot(bestSepAxis, incEdge1 - refEdge1) <= eps) {
			contacts[contactCount++] = {incEdge1, bestSepAxis, bestPen, ref, inc};
		}
		if(incEdge2 == refEdge1 || incEdge2 == refEdge2 || dot(bestSepAxis, incEdge2 - refEdge2) <= eps) {
			contacts[contactCount++] = {incEdge2, bestSepAxis, bestPen, ref, inc};
		}
	} else {
		int here = 3;
	}

	if(contactCount != 2){
		int baz = 3;
	}
	
	return contactCount;
}

void solveContacts(State *state, float dt) {

	int nloops = 10;
	for(int i = 0; i < nloops; i++ ) {
		for(int j = 0; j < state->contactCount; j++) {
			Contact *contact = state->contacts + j;
			Box *A = contact->A;
			Box *B = contact->B;

			V2 ra = contact->p - A->p;
			V2 rb = contact->p - B->p;
			V2 n = contact->n;

			float numer = dot(A->v + cross(A->w, ra) - B->v - cross(B->w, rb), n);
			float denom = 1/A->mass + 1/B->mass + dot((1.0f/A->I)*cross(cross(ra, n), ra) + (1.0f/B->I)*cross(cross(rb, n), rb), n);
			float slop = 0.00f;
			float baumgarte = 0.05*max(contact->pen - slop, 0)/dt;
			float newTotalP = max((numer + baumgarte)/denom + contact->accP, 0);
			float P = newTotalP - contact->accP;

			A->v = A->v - (P/A->mass)*n;
			B->v = B->v + (P/B->mass)*n;
			A->w = A->w - cross(ra, (P/A->I)*n);
			B->w = B->w + cross(rb, (P/B->I)*n);
			contact->accP += P;
		}
			
		for(int j = 0; j < state->contactCount; j++) {
			Contact *contact = state->contacts + j;
			Box *A = contact->A;
			Box *B = contact->B;

			V2 ra = contact->p - A->p;
			V2 rb = contact->p - B->p;
			V2 n = contact->n;

			V2 t = rotate90c(n);
			float numer = dot(A->v + cross(A->w, ra) - B->v - cross(B->w, rb), t);
			float denom = 1/A->mass + 1/B->mass + dot((1.0f/A->I)*cross(cross(ra, t), ra) + (1.0f/B->I)*cross(cross(rb, t), rb), t);
			float coefFric = 0.3f;
			float newTotalPT = clamp(numer/denom + contact->accPT, -coefFric*contact->accP, coefFric*contact->accP);
			float PT = newTotalPT - contact->accPT;

			A->v = A->v - (PT/A->mass)*t;
			B->v = B->v + (PT/B->mass)*t;
			A->w = A->w - cross(ra, (PT/A->I)*t);
			B->w = B->w + cross(rb, (PT/B->I)*t);
			contact->accPT += PT;
		}
	}

}

extern "C" void tick(State *state, float dt) {
	if(!state->init) {
		state->boxes[state->boxCount++] = Box(0.0f, 2.0f, 20, 1, 0, 1000000000.0f, false);
		state->boxes[state->boxCount++] = Box(-15.0f, -3.0f, 15, 1, -0.5f, 1000000000.0f, false);
		state->boxes[state->boxCount++] = Box(-0.5f, -11.0f, 20, 1, -0.1f, 1000000000.0f, false);
		state->boxes[state->boxCount++] = Box(-15.0f, 7.0f, 15, 1, 0.2f, 1000000000.0f, false);

		/*state->boxes[state->boxCount++] = Box(0.9f, 26.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 24.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 22.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 20.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 18.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 16.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 14.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 12.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 10.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 8.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 6.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 4.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 2.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, 0.0f, 1, 1, 0, 1, true);
		state->boxes[state->boxCount++] = Box(0.9f, -2.0f, 1, 1, 0, 1, true);*/

		for(int i = 0; i < 10; i++) {
			for(int j = 0; j < i; j++) {
				float x = -7.0f + 1.2f*j + 0.6*(10-i);
				float y = 8.0f + 2.0f*(10-i);
				state->boxes[state->boxCount++] = Box(x, y, 1, 1, 0, 1, true);
			}
		}

		state->init = true;
		state->t = 0;

	}
	state->pixelsPerMeter = 20.0f;
	state->camP = { 0.0, 0.0f };

	if(state->shoot) {
		Box shootBox = Box(20.0f, 27.0f, 1, 1, 0, 1, true);
		shootBox.v = {-20.0f, -20.0f};
		shootBox.w = 8.0f;
		state->boxes[state->boxCount++] = shootBox;
		state->shoot = false;
	}

	if(state->partyTime > 0.0f) {
		for(int i = 0; i < state->boxCount; i++) {
			Box *box = state->boxes + i;
			if(box->gravity) {
				float random = (2.7*box->p.y) - (int)(2.7*box->p.y) + (11.5*box->p.x) - (int)(11.5*box->p.x);
				box->v.y += 3.0f*dt;//min(1.5f*dt + 2.0f*random*dt, 2.0f);
				//box->v.x -= 1.0f*random*dt;
			}
		}
		state->partyTime -= dt;
	}

	Contact newContacts[4096];
	int newContactCount = 0;
	Contact contacts[2];
	for(int i = 0; i < state->boxCount; i++) {
		for(int j = i + 1; j < state->boxCount; j++) {
			int contactCount = sat(state, state->boxes + i, state->boxes + j, contacts);
			for(int k = 0; k < contactCount; k++) {
				newContacts[newContactCount++] = contacts[k];
			}
		}
	}

	// TODO: Hash table
	int numWarmStarted = 0;
	for(int i = 0; i < newContactCount; i++) {
		for(int j = 0; j < state->contactCount; j++) {
			Contact *newContact = newContacts + i;
			Contact *oldContact = state->contacts + j;
			if(newContact->A == oldContact->A && newContact->B == oldContact->B) {
				float pDif = lengthSq(newContact->p - oldContact->p);
				float nDif = lengthSq(newContact->n - oldContact->n);
				if(pDif < 1e-4 && nDif < 1e-4) {
					float warmStartFrac = 0.5f;
					newContact->accP = warmStartFrac*oldContact->accP;
					newContact->accPT = warmStartFrac*oldContact->accPT;
					numWarmStarted++;
				}
			}
		}
	}
	state->contactCount = newContactCount;
	for(int i = 0; i < newContactCount; i++) {
		state->contacts[i] = newContacts[i];
	}
	float pctWarmStarted = (float)numWarmStarted/newContactCount;

	if(!state->paused) {
		for(int i = 0; i < state->boxCount; i++) {
			Box *box = state->boxes + i;
			if(box->gravity) {
				box->v.y -= 1.0f*dt;
			}
		}	

		solveContacts(state, dt);

		for(int i = 0; i < state->boxCount; i++) {
			Box *box = state->boxes + i;
			box->p = box->p + dt*box->v;
			box->rotate(dt*box->w);
		}
	}
}


