// To compile with gcc:  (tested on Ubuntu 14.04 64bit):
//	 g++ sdl2_opengl.cpp -lSDL2 -lGL
// To compile with msvc: (tested on Windows 7 64bit)
//   cl sdl2_opengl.cpp /I C:\sdl2path\include /link C:\path\SDL2.lib C:\path\SDL2main.lib /SUBSYSTEM:CONSOLE /NODEFAULTLIB:libcmtd.lib opengl32.lib


#define SDL_MAIN_HANDLED
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_log.h>
#include <GL/gl.h>
#include <stdint.h>
#include "platform.h"

FILETIME getFileWriteTime(char *file) {
	FILETIME result;
	HANDLE handle = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, 0);
	assert(handle != INVALID_HANDLE_VALUE);
	assert(GetFileTime(handle, 0, 0, &result));
	CloseHandle(handle);
	return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	int width = 1000;
	int height = 1000;

	int numFramesSinceCheckUpdate = 0;
	CopyFile("fizzicks.dll", "fizzicks_hot.dll", 0);
	HMODULE fizzicksDLL = LoadLibrary("fizzicks_hot.dll");
	assert(fizzicksDLL != 0);
	TickFunction *tick = (TickFunction *)GetProcAddress(fizzicksDLL, "tick");
	assert(tick != 0);
	FILETIME lastTickWriteTime = getFileWriteTime("fizzicks.dll");

	bool fullscreen = false;
	int flags = SDL_WINDOW_OPENGL;
	if(fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	SDL_Window *window = SDL_CreateWindow("fizzicks", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
	SDL_GetWindowSize(window, &width, &height);

	SDL_GLContext context = SDL_GL_CreateContext(window);

	State state = {};

	float dt = 1.0f/60.0f;

	bool running = true;
	while(running) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_KEYDOWN) {
				int key = event.key.keysym.sym;
				if(key == SDLK_ESCAPE) {
					running = false;
					break;
				} else if(key == SDLK_SPACE) {
					state.paused = !state.paused;
				} else if(key == 'w' || key == 'W') {
					state.shoot = true;
				}
			}
			else if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
				running = false; 
				break;
			}
		}

		if(numFramesSinceCheckUpdate++ > 10) {
			if(GetFileAttributesA("lock.tmp") == INVALID_FILE_ATTRIBUTES) {
				FILETIME tickWriteTime = getFileWriteTime("fizzicks.dll");
				if(tickWriteTime.dwHighDateTime != lastTickWriteTime.dwHighDateTime || tickWriteTime.dwLowDateTime != lastTickWriteTime.dwLowDateTime) {
					FreeLibrary(fizzicksDLL);
					CopyFile("fizzicks.dll", "fizzicks_hot.dll", 0);
					fizzicksDLL = LoadLibrary("fizzicks_hot.dll");
					assert(fizzicksDLL != 0);
					tick = (TickFunction *)GetProcAddress(fizzicksDLL, "tick");
					assert(tick != 0);
					lastTickWriteTime = tickWriteTime;
				}
				numFramesSinceCheckUpdate = 0;
			}
		}

		tick(&state, dt);

		glViewport(0, 0, width, height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(
			state.camP.x - 0.5f*width/state.pixelsPerMeter,
			state.camP.x + 0.5f*width/state.pixelsPerMeter,
			state.camP.y - 0.5f*height/state.pixelsPerMeter, 
			state.camP.y + 0.5f*height/state.pixelsPerMeter,
			1.0f,
			-1.0f);

		glLineWidth(2.0f);
		for(int i = 0; i < state.boxCount; i++) {
			glBegin(GL_LINES);
			Box *box = state.boxes + i;
			V2 a = box->p + box->hdim.x*box->ax - box->hdim.y*box->ay;
			V2 b = box->p - box->hdim.x*box->ax - box->hdim.y*box->ay;
			V2 c = box->p + box->hdim.x*box->ax + box->hdim.y*box->ay;
			V2 d = box->p - box->hdim.x*box->ax + box->hdim.y*box->ay;

			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex2f(a.x,a.y);
			glVertex2f(b.x,b.y);
			glVertex2f(c.x,c.y);
			glVertex2f(d.x,d.y);
			glVertex2f(a.x,a.y);
			glVertex2f(c.x,c.y);
			glVertex2f(b.x,b.y);
			glVertex2f(d.x,d.y);

			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex2f(box->p.x, box->p.y);
			glVertex2f(box->p.x + 0.25*box->hdim.x*box->ax.x, box->p.y + 0.25*box->hdim.y*box->ax.y);
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex2f(box->p.x, box->p.y);
			glVertex2f(box->p.x + 0.25*box->hdim.x*box->ay.x, box->p.y + 0.25*box->hdim.y*box->ay.y);


			glEnd();
		}

		glColor3f(1.0f, 1.0f, 1.0f);
		glLineWidth(3.0f);
		glBegin(GL_LINES);
		for(int i = 0; i < state.lineCount; i++) {
			Line *line = state.lines + i;
			glColor3f(line->r, line->g, line->b);
			glVertex2f(line->start.x, line->start.y);
			glVertex2f(line->end.x, line->end.y);
		}
		glEnd();

		glColor3f(1.0f, 0.0f, 1.0f);
		glPointSize(6.0f);
		glBegin(GL_POINTS);
		for(int i = 0; i < state.contactCount; i++) {
			Contact contact = state.contacts[i];
			//glVertex2f(contact.p.x, contact.p.y);
		}
		glEnd();

		SDL_GL_SwapWindow(window);
	}
}



