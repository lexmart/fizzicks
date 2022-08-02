
extern "C" float cosf(float);
extern "C" float sinf(float);
extern "C" float sqrtf(float);
extern "C" void drawBox(float, float, float, float, float, float, float, float);

#include "fizzicks.cpp"

State state = {};

extern "C" void keydown() {
	state.shoot = true;
}

extern "C" void setPartyTime(float t) {
	if(state.partyTime <= 0) {
		state.partyTime = t;
	}
}

extern "C" void fizzicks(float dt) {
	//drawBox(0, 5.0f*cosf(t), 3, 6, xx, xy, yx, yy);

	tick(&state, dt);

	for(int i = 0; i < state.boxCount; i++) {
		Box *box = state.boxes + i;
		drawBox(box->p.x, box->p.y, 2*box->hdim.x, 2*box->hdim.y, box->ax.x, box->ax.y, box->ay.x, box->ay.y);
	}
}


