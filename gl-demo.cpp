#ifdef USE_EGL
#include <EGL/egl.h>
#else
#include <GL/glx.h>
#endif
#include <GL/gl.h>
#include <stdio.h>
#include <iostream>
#include "opencv2/opencv.hpp"
#include <omp.h>

const int width=640, height=480;

void drawUnitSquare()
{
	glBegin( GL_TRIANGLES );
	glNormal3f(0,0,1);

	glTexCoord2f(1,1);
	glVertex3f(0.5f,0.5f,0);
	glTexCoord2f(0,1);
	glVertex3f(-0.5f,0.5f,0);
	glTexCoord2f(0,0);
	glVertex3f(-0.5f,-0.5f,0);

	glTexCoord2f(0,0);
	glVertex3f(-0.5f,-0.5f,0);
	glTexCoord2f(1,0);
	glVertex3f(0.5f,-0.5f,0);
	glTexCoord2f(1,1);
	glVertex3f(0.5f,0.5f,0);
	glEnd();
}

void draw_and_save()
{
	glViewport(0,0,width,height);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for(int i=0; i<5; ++i){
		double t = omp_get_wtime();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor4f(1,0,0, 0.5f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(i*10, 1,1,0);
			drawUnitSquare();
		glFinish();
		std::cout << (omp_get_wtime()-t)*1000 << "ms.\n";
	}

	cv::Mat rgb_out(height, width, CV_8UC3);
	glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, rgb_out.data);
	cv::imwrite("img.png", rgb_out);
	std::cout << "img.png saved!\n";
}

int main()
{
#ifdef USE_EGL
	EGLDisplay d = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if( d )
		printf("eglGetDisplay success!\n");
	else
		printf("eglGetDisplay failed!\n");
	
	int maj, min;
	if( eglInitialize(d, &maj, &min) )
		printf("eglInitialize success!\n");
	else
		printf("eglInitialize failed!\n");
	printf("EGL version = %d.%d\n", maj, min);
	printf("EGL_VENDOR = %s\n", eglQueryString(d, EGL_VENDOR));
	
	EGLConfig first_config;
	EGLint numConfigs;
	eglGetConfigs(d, &first_config, 1, &numConfigs);
	
	if( eglBindAPI(EGL_OPENGL_API) )
		printf("eglBindAPI success!\n");
	else
		printf("eglBindAPI failed!\n");
	
	EGLContext ctx = eglCreateContext(d, first_config, EGL_NO_CONTEXT, NULL);
	if (ctx != EGL_NO_CONTEXT)
		printf("create context success!\n");
	else
		printf("create context failed!\n");

	const EGLint pbufAttribs[] = {
		EGL_WIDTH, width,
		EGL_HEIGHT, height,
		EGL_NONE
	};
	EGLSurface pbuffer = eglCreatePbufferSurface(d, first_config, pbufAttribs);
	if (pbuffer == EGL_NO_SURFACE) {
		printf("failed to create pbuffer!\n");
		return 0;
	}

	EGLBoolean b = eglMakeCurrent(d, pbuffer, pbuffer, ctx);
	if (b)
		printf("make current success!\n");
	else
		printf("make current failed!\n");
#else
	typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
	typedef Bool (*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);
	static glXCreateContextAttribsARBProc
		glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *)"glXCreateContextAttribsARB");
	static glXMakeContextCurrentARBProc
		glXMakeContextCurrentARB   = (glXMakeContextCurrentARBProc)   glXGetProcAddressARB( (const GLubyte *)"glXMakeContextCurrent");
	if(!glXCreateContextAttribsARB)
		std::cout << "glXGetProcAddressARB \"glXCreateContextAttribsARB\" failed!\n";
	if(!glXMakeContextCurrentARB)
		std::cout << "glXGetProcAddressARB \"glXMakeContextCurrentARB\" failed!\n";

	const char *displayName = NULL;
	Display* display = XOpenDisplay( displayName );

	if(!display)
		std::cout << "XOpenDisplay failed!\n";

	static int visualAttribs[] = { None };
	int numberOfFramebufferConfigurations = 0;
	GLXFBConfig* fbConfigs = glXChooseFBConfig( display, DefaultScreen(display), visualAttribs, &numberOfFramebufferConfigurations );

	if(!fbConfigs)
		std::cout << "glXChooseFBConfig failed!\n";

	int context_attribs[] = {
	    None
	};

	GLXContext openGLContext = glXCreateContextAttribsARB( display, fbConfigs[0], 0, True, context_attribs);

	if(!openGLContext)
		std::cout << "glXCreateContextAttribsARB failed!\n";

	int pbufferAttribs[] = {
	    GLX_PBUFFER_WIDTH,  width,
	    GLX_PBUFFER_HEIGHT, height,
	    None
	};
	GLXPbuffer pbuffer = glXCreatePbuffer( display, fbConfigs[0], pbufferAttribs );

	// clean up:
	XFree( fbConfigs );
	XSync( display, False );

	if ( !glXMakeContextCurrent( display, pbuffer, pbuffer, openGLContext ) ){
	    std::cout << "glXMakeContextCurrent failed!\n";
	}
#endif

	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << ".\n";

	draw_and_save();

#ifdef USE_EGL
	eglDestroySurface(d, pbuffer);
	eglDestroyContext(d, ctx);
	eglTerminate(d);
#else
	glXDestroyPbuffer( display, pbuffer );
	glXDestroyContext( display, openGLContext );
#endif

	return 0;
}
